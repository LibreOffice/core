/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <markdata.hxx>
#include "../../source/core/data/markarr.cxx"
#include "../../source/core/data/markmulti.cxx"
#include "../../source/core/data/segmenttree.cxx"

#include <utility>

struct MarkTestData  // To represent a single rectangle part of a multiselection
{
    ScRange aRange;
    bool bMark;

    // Cumulative Test data follows
    std::vector<ScAddress> aInsideAddresses;
    std::vector<ScAddress> aOutsideAddresses;

    std::vector<SCCOL> aColumnsWithFullMarks;
    std::vector<SCCOL> aColumnsWithoutFullMarks;

    std::vector<SCROW> aRowsWithFullMarks;
    std::vector<SCROW> aRowsWithoutFullMarks;

    std::vector<ScRange> aRangesWithFullMarks;
    std::vector<ScRange> aRangesWithoutFullMarks;

    // To test ScMarkData::GetNextMarked()
    // Encoding : EndRow is the next marked row in the column = StartCol after StartRow.
    // EndCol = 0 means search up, 1 means to search down
    std::vector<ScRange> aNextMarked;

    std::vector<SCCOL> aColumnsWithAtLeastOneMark;
    std::vector<SCCOL> aColumnsWithoutAnyMarks;

};

struct MarkArrayTestData
{
    SCCOL nCol;
    std::vector<std::pair<SCROW,SCROW>> aMarkedRowSegs;
};

struct MultiMarkTestData
{
    std::vector<MarkTestData> aMarks;
    ScRange aSelectionCover;
    ScRangeList aLeftEnvelope;
    ScRangeList aRightEnvelope;
    ScRangeList aTopEnvelope;
    ScRangeList aBottomEnvelope;
    std::vector<MarkArrayTestData> aMarkArrays;

    // To test ScMultiSel::HasOneMark()
    // Encoding : StartCol is the column to test, StartRow is the beginning of the one mark,
    // EndRow is the end of the one mark, EndCol is not used
    std::vector<ScRange> aColsWithOneMark;
    std::vector<SCCOL> aColsWithoutOneMark;

    // To test ScMultiSel::IsAllMarked()
    // Encoding StartCol is the column to be queried, [StartRow,EndRow] is the range to test.
    std::vector<ScRange> aColsAllMarked;
    std::vector<ScRange> aColsNotAllMarked;

    // To test ScMultiSel::HasEqualRowsMarked()
    std::vector<std::pair<SCCOL,SCCOL>> aColsWithEqualMarksList;
    std::vector<std::pair<SCCOL,SCCOL>> aColsWithUnequalMarksList;
};

class Test : public CppUnit::TestFixture
{
public:
    void testSimpleMark( const ScRange& rRange, const ScRange& rSelectionCover,
                         const ScRangeList& rLeftEnvelope, const ScRangeList& rRightEnvelope,
                         const ScRangeList& rTopEnvelope, const ScRangeList& rBottomEnvelope );
    void testSimpleMark_Simple();
    void testSimpleMark_Column();
    void testSimpleMark_Row();

    void testMultiMark( const MultiMarkTestData& rData );
    void testMultiMark_FourRanges();
    void testMultiMark_NegativeMarking();

    void testInsertTabBeforeSelected();
    void testInsertTabAfterSelected();
    void testDeleteTabBeforeSelected();
    void testDeleteTabAfterSelected();

    void testScMarkArraySearch();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSimpleMark_Simple);
    CPPUNIT_TEST(testSimpleMark_Column);
    CPPUNIT_TEST(testSimpleMark_Row);
    CPPUNIT_TEST(testMultiMark_FourRanges);
    CPPUNIT_TEST(testMultiMark_NegativeMarking);
    CPPUNIT_TEST(testInsertTabBeforeSelected);
    CPPUNIT_TEST(testInsertTabAfterSelected);
    CPPUNIT_TEST(testDeleteTabBeforeSelected);
    CPPUNIT_TEST(testDeleteTabAfterSelected);
    CPPUNIT_TEST(testScMarkArraySearch);
    CPPUNIT_TEST_SUITE_END();

private:
    void testScMarkArraySearch_check(const ScMarkArray & ar, SCROW nRow, bool expectStatus, SCSIZE nIndexExpect);
};

static void lcl_GetSortedRanges( const ScRangeList& rRangeList, ScRangeList& rRangeListOut )
{
    rRangeListOut.RemoveAll();
    std::vector<ScRange> aRanges;
    size_t nSize = rRangeList.size();
    aRanges.reserve( nSize );
    for ( size_t nIdx = 0; nIdx < nSize; ++nIdx )
        aRanges.push_back( rRangeList[nIdx] );
    std::sort( aRanges.begin(), aRanges.end() );
    for ( size_t nIdx = 0; nIdx < nSize; ++nIdx )
        rRangeListOut.push_back( aRanges[nIdx] );
}

void Test::testSimpleMark( const ScRange& rRange, const ScRange& rSelectionCover,
                           const ScRangeList& rLeftEnvelope, const ScRangeList& rRightEnvelope,
                           const ScRangeList& rTopEnvelope, const ScRangeList& rBottomEnvelope )
{
    ScMarkData aMark;
    CPPUNIT_ASSERT( !aMark.IsMarked() && !aMark.IsMultiMarked() );

    aMark.SetMarkArea( rRange );
    CPPUNIT_ASSERT( aMark.IsMarked() && !aMark.IsMultiMarked() );

    ScRange aRangeResult;
    aMark.GetMarkArea( aRangeResult );
    CPPUNIT_ASSERT_EQUAL( rRange, aRangeResult );

    SCROW nMidRow = ( rRange.aStart.Row() + rRange.aEnd.Row() ) / 2;
    SCCOL nMidCol = ( rRange.aStart.Col() + rRange.aEnd.Col() ) / 2;
    SCROW nOutRow1 = rRange.aEnd.Row() + 1;
    SCCOL nOutCol1 = rRange.aEnd.Col() + 1;
    SCROW nOutRow2 = rRange.aStart.Row() - 1;
    SCCOL nOutCol2 = rRange.aStart.Col() - 1;

    CPPUNIT_ASSERT( aMark.IsCellMarked( nMidCol, nMidRow ) );
    if ( ValidCol( nOutCol1 ) && ValidRow( nOutRow1 ) )
        CPPUNIT_ASSERT( !aMark.IsCellMarked( nOutCol1, nOutRow1 ) );

    if ( ValidCol( nOutCol2 ) && ValidRow( nOutRow2 ) )
        CPPUNIT_ASSERT( !aMark.IsCellMarked( nOutCol2, nOutRow2 ) );

    if ( ValidRow( nOutRow1 ) )
        CPPUNIT_ASSERT( !aMark.IsCellMarked( nMidCol, nOutRow1 ) );

    if ( ValidCol( nOutCol1 ) )
        CPPUNIT_ASSERT( !aMark.IsCellMarked( nOutCol1, nMidRow ) );

    if ( ValidRow( nOutRow2 ) )
        CPPUNIT_ASSERT( !aMark.IsCellMarked( nMidCol, nOutRow2 ) );

    if ( ValidCol( nOutCol2 ) )
        CPPUNIT_ASSERT( !aMark.IsCellMarked( nOutCol2, nMidRow ) );

    if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
        CPPUNIT_ASSERT( aMark.IsColumnMarked( nMidCol ) );
    else
        CPPUNIT_ASSERT( !aMark.IsColumnMarked( nMidCol ) );
    if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL )
        CPPUNIT_ASSERT( aMark.IsRowMarked( nMidRow ) );
    else
        CPPUNIT_ASSERT( !aMark.IsRowMarked( nMidRow ) );

    ScRange aSelectionCoverOutput;
    aMark.GetSelectionCover( aSelectionCoverOutput );
    CPPUNIT_ASSERT_EQUAL( rSelectionCover, aSelectionCoverOutput );
    CPPUNIT_ASSERT_EQUAL( rLeftEnvelope, aMark.GetLeftEnvelope() );
    CPPUNIT_ASSERT_EQUAL( rRightEnvelope, aMark.GetRightEnvelope() );
    CPPUNIT_ASSERT_EQUAL( rTopEnvelope, aMark.GetTopEnvelope() );
    CPPUNIT_ASSERT_EQUAL( rBottomEnvelope, aMark.GetBottomEnvelope() );
}

void Test::testSimpleMark_Simple()
{
    testSimpleMark( ScRange( 10, 15, 0, 20, 30, 0 ),                  // Simple range
                    ScRange( 9, 14, 0, 21, 31, 0 ),                   // Cover
                    ScRangeList( ScRange( 9, 15, 0, 9, 30, 0 ) ),     // Left envelope
                    ScRangeList( ScRange( 21, 15, 0, 21, 30, 0 ) ),   // Right envelope
                    ScRangeList( ScRange( 10, 14, 0, 20, 14, 0 ) ),   // Top envelope
                    ScRangeList( ScRange( 10, 31, 0, 20, 31, 0 ) ) ); // Bottom envelope
}

void Test::testSimpleMark_Column()
{
    // Column 10, rows from 15 to 30
    testSimpleMark( ScRange( 10, 15, 0, 10, 30, 0 ),                  // Simple range
                    ScRange( 9, 14, 0, 11, 31, 0 ),                   // Cover
                    ScRangeList( ScRange( 9, 15, 0, 9, 30, 0 ) ),     // Left envelope
                    ScRangeList( ScRange( 11, 15, 0, 11, 30, 0 ) ),   // Right envelope
                    ScRangeList( ScRange( 10, 14, 0, 10, 14, 0 ) ),   // Top envelope
                    ScRangeList( ScRange( 10, 31, 0, 10, 31, 0 ) ) ); // Bottom envelope

    // Full Column 10
    testSimpleMark( ScRange( 10, 0, 0, 10, MAXROW, 0 ),                 // Simple range
                    ScRange( 9, 0, 0, 11, MAXROW, 0 ),                  // Cover
                    ScRangeList( ScRange( 9, 0, 0, 9, MAXROW, 0 ) ),    // Left envelope
                    ScRangeList( ScRange( 11, 0, 0, 11, MAXROW, 0 ) ),  // Right envelope
                    ScRangeList(),                                      // Top envelope
                    ScRangeList());                                     // Bottom envelope
}

void Test::testSimpleMark_Row()
{
    // Row 15, cols from 10 to 20
    testSimpleMark( ScRange( 10, 15, 0, 20, 15, 0 ),                  // Simple range
                    ScRange( 9, 14, 0, 21, 16, 0 ),                   // Cover
                    ScRangeList( ScRange( 9, 15, 0, 9, 15, 0 ) ),     // Left envelope
                    ScRangeList( ScRange( 21, 15, 0, 21, 15, 0 ) ),   // Right envelope
                    ScRangeList( ScRange( 10, 14, 0, 20, 14, 0 ) ),   // Top envelope
                    ScRangeList( ScRange( 10, 16, 0, 20, 16, 0 ) ) ); // Bottom envelope

    // Full Row 15
    testSimpleMark( ScRange( 0, 15, 0, MAXCOL, 15, 0 ),                    // Simple range
                    ScRange( 0, 14, 0, MAXCOL, 16, 0 ),                    // Cover
                    ScRangeList(),                                         // Left envelope
                    ScRangeList(),                                         // Right envelope
                    ScRangeList( ScRange( 0, 14, 0, MAXCOL, 14, 0 ) ),     // Top envelope
                    ScRangeList( ScRange( 0, 16, 0, MAXCOL, 16, 0 ) ) );   // Bottom envelope
}

void Test::testMultiMark( const MultiMarkTestData& rMarksData )
{

    ScMarkData aMark;
    ScMultiSel aMultiSel;
    CPPUNIT_ASSERT( !aMark.IsMarked() && !aMark.IsMultiMarked() );
    CPPUNIT_ASSERT_EQUAL( SCCOL(0), aMultiSel.size() );
    CPPUNIT_ASSERT( !aMultiSel.HasAnyMarks() );

    for ( const auto& rAreaTestData : rMarksData.aMarks )
    {
        aMultiSel.SetMarkArea( rAreaTestData.aRange.aStart.Col(), rAreaTestData.aRange.aEnd.Col(),
                               rAreaTestData.aRange.aStart.Row(), rAreaTestData.aRange.aEnd.Row(),
                               rAreaTestData.bMark );
        aMark.SetMultiMarkArea( rAreaTestData.aRange, rAreaTestData.bMark );
        CPPUNIT_ASSERT( aMark.IsMultiMarked() );

        for ( const auto& rMarkedAddress : rAreaTestData.aInsideAddresses )
        {
            CPPUNIT_ASSERT( aMultiSel.GetMark( rMarkedAddress.Col(), rMarkedAddress.Row() ) );
            CPPUNIT_ASSERT( aMark.IsCellMarked( rMarkedAddress.Col(), rMarkedAddress.Row() ) );
        }

        for ( const auto& rUnMarkedAddress : rAreaTestData.aOutsideAddresses )
        {
            CPPUNIT_ASSERT( !aMark.IsCellMarked( rUnMarkedAddress.Col(), rUnMarkedAddress.Row() ) );
            CPPUNIT_ASSERT( !aMark.IsCellMarked( rUnMarkedAddress.Col(), rUnMarkedAddress.Row() ) );
        }

        for ( const auto& rCol : rAreaTestData.aColumnsWithFullMarks )
        {
            CPPUNIT_ASSERT( aMark.IsColumnMarked( rCol ) );
            CPPUNIT_ASSERT( aMultiSel.IsAllMarked( rCol, 0, MAXROW ) );
        }

        for ( const auto& rCol : rAreaTestData.aColumnsWithoutFullMarks )
        {
            CPPUNIT_ASSERT( !aMark.IsColumnMarked( rCol ) );
            CPPUNIT_ASSERT( !aMultiSel.IsAllMarked( rCol, 0, MAXROW ) );
        }

        for ( const auto& rRow : rAreaTestData.aRowsWithFullMarks )
        {
            CPPUNIT_ASSERT( aMark.IsRowMarked( rRow ) );
            CPPUNIT_ASSERT( aMultiSel.IsRowMarked( rRow ) );
        }

        for ( const auto& rRow : rAreaTestData.aRowsWithoutFullMarks )
        {
            CPPUNIT_ASSERT( !aMark.IsRowMarked( rRow ) );
            CPPUNIT_ASSERT( !aMultiSel.IsRowMarked( rRow ) );
        }

        for ( const auto& rRange : rAreaTestData.aRangesWithFullMarks )
        {
            CPPUNIT_ASSERT( aMark.IsAllMarked( rRange ) );
            SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
            SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
            for ( SCROW nColIter = nCol1; nColIter <= nCol2; ++nColIter )
                CPPUNIT_ASSERT( aMultiSel.IsAllMarked( nColIter, nRow1, nRow2 ) );
        }

        for ( const auto& rRange : rAreaTestData.aRangesWithoutFullMarks )
            CPPUNIT_ASSERT( !aMark.IsAllMarked( rRange ) );

        for ( const auto& rRange : rAreaTestData.aNextMarked )
        {
            SCROW nNextRow1 = aMark.GetNextMarked( rRange.aStart.Col(), rRange.aStart.Row(), !static_cast<bool>(rRange.aEnd.Col()) );
            CPPUNIT_ASSERT_EQUAL( rRange.aEnd.Row(), nNextRow1 );
            SCROW nNextRow2 = aMultiSel.GetNextMarked( rRange.aStart.Col(), rRange.aStart.Row(), !static_cast<bool>(rRange.aEnd.Col()) );
            CPPUNIT_ASSERT_EQUAL( rRange.aEnd.Row(), nNextRow2 );
        }

        for ( const auto& rCol : rAreaTestData.aColumnsWithAtLeastOneMark )
        {
            CPPUNIT_ASSERT( aMark.HasMultiMarks( rCol ) );
            CPPUNIT_ASSERT( aMultiSel.HasMarks( rCol ) );
        }

        for ( const auto& rCol : rAreaTestData.aColumnsWithoutAnyMarks )
        {
            CPPUNIT_ASSERT( !aMark.HasMultiMarks( rCol ) );
            CPPUNIT_ASSERT( !aMultiSel.HasMarks( rCol ) );
        }

    }

    ScRange aSelectionCoverOutput;
    aMark.GetSelectionCover( aSelectionCoverOutput );
    CPPUNIT_ASSERT_EQUAL( rMarksData.aSelectionCover, aSelectionCoverOutput );

    ScRangeList aRangesExpected, aRangesActual;
    lcl_GetSortedRanges( rMarksData.aLeftEnvelope, aRangesExpected );
    lcl_GetSortedRanges( aMark.GetLeftEnvelope(), aRangesActual );
    CPPUNIT_ASSERT_EQUAL( aRangesExpected, aRangesActual );
    lcl_GetSortedRanges( rMarksData.aRightEnvelope, aRangesExpected );
    lcl_GetSortedRanges( aMark.GetRightEnvelope(), aRangesActual );
    CPPUNIT_ASSERT_EQUAL( aRangesExpected, aRangesActual );
    lcl_GetSortedRanges( rMarksData.aTopEnvelope, aRangesExpected );
    lcl_GetSortedRanges( aMark.GetTopEnvelope(), aRangesActual );
    CPPUNIT_ASSERT_EQUAL( aRangesExpected, aRangesActual );
    lcl_GetSortedRanges( rMarksData.aBottomEnvelope, aRangesExpected );
    lcl_GetSortedRanges( aMark.GetBottomEnvelope(), aRangesActual );
    CPPUNIT_ASSERT_EQUAL( aRangesExpected, aRangesActual );

    for ( const auto& rMarkArrayTestData : rMarksData.aMarkArrays )
    {
        ScMarkArray aArray( aMark.GetMarkArray( rMarkArrayTestData.nCol ) );
        std::vector<std::pair<SCROW, SCROW>> aMarkedRowSegs;
        ScMarkArrayIter aIter( &aArray );
        SCROW nStart, nEnd;
        while ( aIter.Next( nStart, nEnd ) )
            aMarkedRowSegs.emplace_back( nStart, nEnd );

        CPPUNIT_ASSERT_EQUAL( rMarkArrayTestData.aMarkedRowSegs.size(), aMarkedRowSegs.size() );
        size_t nIdx = 0;
        for ( const auto& rPair : rMarkArrayTestData.aMarkedRowSegs )
        {
            CPPUNIT_ASSERT_EQUAL( rPair.first, aMarkedRowSegs[nIdx].first );
            CPPUNIT_ASSERT_EQUAL( rPair.second, aMarkedRowSegs[nIdx].second );
            ++nIdx;
        }
    }

    for ( const auto& rColWithOneMark : rMarksData.aColsWithOneMark )
    {
        SCROW nRow1 = -1, nRow2 = -1;
        CPPUNIT_ASSERT( aMultiSel.HasOneMark( rColWithOneMark.aStart.Col(), nRow1, nRow2 ) );
        CPPUNIT_ASSERT_EQUAL( rColWithOneMark.aStart.Row(), nRow1 );
        CPPUNIT_ASSERT_EQUAL( rColWithOneMark.aEnd.Row(), nRow2 );
    }

    {
        SCROW nRow1 = -1, nRow2 = -1;
        for ( const SCCOL& rCol : rMarksData.aColsWithoutOneMark )
            CPPUNIT_ASSERT( !aMultiSel.HasOneMark( rCol, nRow1, nRow2 ) );
    }

    for ( const auto& rColAllMarked : rMarksData.aColsAllMarked )
        CPPUNIT_ASSERT( aMultiSel.IsAllMarked( rColAllMarked.aStart.Col(),
                                               rColAllMarked.aStart.Row(),
                                               rColAllMarked.aEnd.Row() ) );

    for ( const auto& rColNotAllMarked : rMarksData.aColsNotAllMarked )
        CPPUNIT_ASSERT( !aMultiSel.IsAllMarked( rColNotAllMarked.aStart.Col(),
                                                rColNotAllMarked.aStart.Row(),
                                                rColNotAllMarked.aEnd.Row() ) );

    for ( const auto& rColsWithEqualMarks : rMarksData.aColsWithEqualMarksList )
        CPPUNIT_ASSERT( aMultiSel.HasEqualRowsMarked( rColsWithEqualMarks.first, rColsWithEqualMarks.second ) );

    for ( const auto& rColsWithUnequalMarks : rMarksData.aColsWithUnequalMarksList )
        CPPUNIT_ASSERT( !aMultiSel.HasEqualRowsMarked( rColsWithUnequalMarks.first, rColsWithUnequalMarks.second ) );

    aMultiSel.Clear();
    CPPUNIT_ASSERT_EQUAL( SCCOL(0), aMultiSel.size() );
    CPPUNIT_ASSERT( !aMultiSel.HasAnyMarks() );
}

void Test::testMultiMark_FourRanges()
{
    MultiMarkTestData aData;
    MarkTestData aSingle1;

    // Create rectangle ( 10, 5, 20, 10 )
    aSingle1.aRange = ScRange( 10, 5, 0, 20, 10, 0 );
    aSingle1.bMark = true;

    aSingle1.aInsideAddresses.emplace_back( 15, 6, 0 );
    aSingle1.aInsideAddresses.emplace_back( 10, 5, 0 );
    aSingle1.aInsideAddresses.emplace_back( 20, 5, 0 );
    aSingle1.aInsideAddresses.emplace_back( 10, 10, 0 );
    aSingle1.aInsideAddresses.emplace_back( 20, 10, 0 );

    aSingle1.aOutsideAddresses.emplace_back( 15, 4, 0 );
    aSingle1.aOutsideAddresses.emplace_back( 15, 11, 0 );
    aSingle1.aOutsideAddresses.emplace_back( 9, 6, 0 );
    aSingle1.aOutsideAddresses.emplace_back( 21, 6, 0 );
    aSingle1.aOutsideAddresses.emplace_back( 26, 10, 0 );

    aSingle1.aColumnsWithoutFullMarks.push_back( 16 );
    aSingle1.aColumnsWithoutFullMarks.push_back( 21 );

    aSingle1.aRowsWithoutFullMarks.push_back( 7 );
    aSingle1.aRowsWithoutFullMarks.push_back( 11 );

    aSingle1.aRangesWithFullMarks.emplace_back( 10, 5, 0, 20, 10, 0 );
    aSingle1.aRangesWithFullMarks.emplace_back( 11, 6, 0, 19, 8, 0 );

    aSingle1.aRangesWithoutFullMarks.emplace_back( 9, 4, 0, 21, 11, 0 );
    aSingle1.aRangesWithoutFullMarks.emplace_back( 25, 7, 0, 30, 15, 0 );

    aSingle1.aNextMarked.emplace_back( 15, 1, 0, 1, 5, 0 ); // Search down
    aSingle1.aNextMarked.emplace_back( 15, 15, 0, 0, 10, 0 ); // Search up

    aSingle1.aNextMarked.emplace_back( 15, 15, 0, 1, MAXROWCOUNT, 0 ); // Search down fail
    aSingle1.aNextMarked.emplace_back( 15, 4, 0, 0, -1, 0 ); // Search up fail

    aSingle1.aColumnsWithAtLeastOneMark.push_back( 10 );
    aSingle1.aColumnsWithAtLeastOneMark.push_back( 15 );
    aSingle1.aColumnsWithAtLeastOneMark.push_back( 20 );

    aSingle1.aColumnsWithoutAnyMarks.push_back( 21 );
    aSingle1.aColumnsWithoutAnyMarks.push_back( 9 );

    // Create rectangle ( 25, 7, 30, 15 )
    MarkTestData aSingle2;
    aSingle2.aRange = ScRange( 25, 7, 0, 30, 15, 0 );
    aSingle2.bMark = true;

    aSingle2.aInsideAddresses = aSingle1.aInsideAddresses;
    aSingle2.aInsideAddresses.emplace_back( 27, 10, 0 );
    aSingle2.aInsideAddresses.emplace_back( 25, 7, 0 );
    aSingle2.aInsideAddresses.emplace_back( 30, 7, 0 );
    aSingle2.aInsideAddresses.emplace_back( 25, 15, 0 );
    aSingle2.aInsideAddresses.emplace_back( 30, 15, 0 );

    aSingle2.aOutsideAddresses.emplace_back( 15, 4, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 15, 11, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 9, 6, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 21, 6, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 26, 6, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 26, 16, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 24, 8, 0 );
    aSingle2.aOutsideAddresses.emplace_back( 31, 11, 0 );

    aSingle2.aColumnsWithoutFullMarks = aSingle1.aColumnsWithoutAnyMarks;

    aSingle2.aRowsWithoutFullMarks = aSingle1.aRowsWithoutFullMarks;

    aSingle2.aRangesWithFullMarks = aSingle1.aRangesWithFullMarks;
    aSingle2.aRangesWithFullMarks.emplace_back( 25, 7, 0, 30, 15, 0 );
    aSingle2.aRangesWithFullMarks.emplace_back( 26, 8, 0, 29, 14, 0 );

    aSingle2.aRangesWithoutFullMarks.emplace_back( 9, 4, 0, 21, 11, 0 );
    aSingle2.aRangesWithoutFullMarks.emplace_back( 24, 6, 0, 31, 16, 0 );
    aSingle2.aRangesWithoutFullMarks.emplace_back( 10, 5, 0, 30, 15, 0 );

    aSingle2.aNextMarked.emplace_back( 27, 16, 0, 0, 15, 0 );  // up ok
    aSingle2.aNextMarked.emplace_back( 27, 4, 0, 1, 7, 0 );  // down ok
    aSingle2.aNextMarked.emplace_back( 27, 4, 0, 0, -1, 0 );  // up fail
    aSingle2.aNextMarked.emplace_back( 27, 16, 0, 1, MAXROWCOUNT, 0 );  // down fail

    aSingle2.aColumnsWithAtLeastOneMark = aSingle1.aColumnsWithAtLeastOneMark;
    aSingle2.aColumnsWithAtLeastOneMark.push_back( 25 );
    aSingle2.aColumnsWithAtLeastOneMark.push_back( 27 );
    aSingle2.aColumnsWithAtLeastOneMark.push_back( 30 );

    aSingle2.aColumnsWithoutAnyMarks.push_back( 9 );
    aSingle2.aColumnsWithoutAnyMarks.push_back( 21 );
    aSingle2.aColumnsWithoutAnyMarks.push_back( 24 );
    aSingle2.aColumnsWithoutAnyMarks.push_back( 31 );

    // Full row = 20
    MarkTestData aSingle3;

    aSingle3.aRange = ScRange( 0, 20, 0, MAXCOL, 20, 0 );
    aSingle3.bMark = true;

    aSingle3.aInsideAddresses = aSingle2.aInsideAddresses;
    aSingle3.aInsideAddresses.emplace_back( 5, 20, 0 );
    aSingle3.aInsideAddresses.emplace_back( 100, 20, 0 );

    aSingle3.aOutsideAddresses.emplace_back( 0, 21, 0 );
    aSingle3.aOutsideAddresses.emplace_back( 27, 19, 0 );

    aSingle3.aColumnsWithoutFullMarks = aSingle2.aColumnsWithoutAnyMarks;

    aSingle3.aRowsWithFullMarks.push_back( 20 );

    aSingle3.aRangesWithFullMarks = aSingle2.aRangesWithFullMarks;
    aSingle3.aRangesWithFullMarks.emplace_back( 0, 20, 0, MAXCOL, 20, 0 );
    aSingle3.aRangesWithFullMarks.emplace_back( 15, 20, 0, 55, 20, 0 );

    aSingle3.aNextMarked.emplace_back( 15, 16, 0, 0, 10, 0 );  // up ok
    aSingle3.aNextMarked.emplace_back( 15, 16, 0, 1, 20, 0 );  // down ok
    aSingle3.aNextMarked.emplace_back( 22, 15, 0, 0, -1, 0 );  // up fail
    aSingle3.aNextMarked.emplace_back( 22, 25, 0, 1, MAXROWCOUNT, 0 );  // down fail

    aSingle3.aColumnsWithAtLeastOneMark = aSingle2.aColumnsWithAtLeastOneMark;
    aSingle3.aColumnsWithAtLeastOneMark.push_back( 39 );

    // Full col = 35
    MarkTestData aSingle4;

    aSingle4.aRange = ScRange( 35, 0, 0, 35, MAXROW, 0 );
    aSingle4.bMark = true;

    aSingle4.aInsideAddresses = aSingle3.aInsideAddresses;
    aSingle4.aInsideAddresses.emplace_back( 35, 10, 0 );
    aSingle4.aInsideAddresses.emplace_back( 35, 25, 0 );

    aSingle4.aOutsideAddresses.emplace_back( 33, 10, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 39, 10, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 33, 25, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 39, 25, 0 );

    aSingle4.aColumnsWithFullMarks.push_back( 35 );

    aSingle4.aRowsWithFullMarks.push_back( 20 );

    aSingle4.aRangesWithFullMarks = aSingle3.aRangesWithFullMarks;
    aSingle4.aRangesWithFullMarks.emplace_back( 35, 0, 0, 35, MAXROW, 0 );
    aSingle4.aRangesWithFullMarks.emplace_back( 35, 10, 0, 35, 25, 0 );

    // Add the rectangle data to aData
    aData.aMarks.push_back( aSingle1 );
    aData.aMarks.push_back( aSingle2 );
    aData.aMarks.push_back( aSingle3 );
    aData.aMarks.push_back( aSingle4 );

    aData.aSelectionCover = ScRange( 0, 0, 0, MAXCOL, MAXROW, 0 );
    aData.aLeftEnvelope.push_back( ScRange( 9, 5, 0, 9, 10, 0 ) );
    aData.aLeftEnvelope.push_back( ScRange( 24, 7, 0, 24, 15, 0 ) );
    aData.aLeftEnvelope.push_back( ScRange( 34, 0, 0, 34, 19, 0 ) );
    aData.aLeftEnvelope.push_back( ScRange( 34, 21, 0, 34, MAXROW, 0 ) );

    aData.aRightEnvelope.push_back( ScRange( 21, 5, 0, 21, 10, 0 ) );
    aData.aRightEnvelope.push_back( ScRange( 31, 7, 0, 31, 15, 0 ) );
    aData.aRightEnvelope.push_back( ScRange( 36, 0, 0, 36, 19, 0 ) );
    aData.aRightEnvelope.push_back( ScRange( 36, 21, 0, 36, MAXROW, 0 ) );

    aData.aTopEnvelope.push_back( ScRange( 10, 4, 0, 20, 4, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 25, 6, 0, 30, 6, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 0, 19, 0, 34, 19, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 36, 19, 0, MAXCOL, 19, 0 ) );

    aData.aBottomEnvelope.push_back( ScRange( 10, 11, 0, 20, 11, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 25, 16, 0, 30, 16, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 0, 21, 0, 34, 21, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 36, 21, 0, MAXCOL, 21, 0 ) );

    MarkArrayTestData aMarkArrayTestData1;
    aMarkArrayTestData1.nCol = 5;
    aMarkArrayTestData1.aMarkedRowSegs.emplace_back( 20, 20 );

    MarkArrayTestData aMarkArrayTestData2;
    aMarkArrayTestData2.nCol = 15;
    aMarkArrayTestData2.aMarkedRowSegs.emplace_back( 5, 10 );
    aMarkArrayTestData2.aMarkedRowSegs.emplace_back( 20, 20 );

    MarkArrayTestData aMarkArrayTestData3;
    aMarkArrayTestData3.nCol = 22;
    aMarkArrayTestData3.aMarkedRowSegs.emplace_back( 20, 20 );

    MarkArrayTestData aMarkArrayTestData4;
    aMarkArrayTestData4.nCol = 27;
    aMarkArrayTestData4.aMarkedRowSegs.emplace_back( 7, 15 );
    aMarkArrayTestData4.aMarkedRowSegs.emplace_back( 20, 20 );

    MarkArrayTestData aMarkArrayTestData5;
    aMarkArrayTestData5.nCol = 33;
    aMarkArrayTestData5.aMarkedRowSegs.emplace_back( 20, 20 );

    MarkArrayTestData aMarkArrayTestData6;
    aMarkArrayTestData6.nCol = 35;
    aMarkArrayTestData6.aMarkedRowSegs.emplace_back( 0, MAXROW );

    MarkArrayTestData aMarkArrayTestData7;
    aMarkArrayTestData7.nCol = 40;
    aMarkArrayTestData7.aMarkedRowSegs.emplace_back( 20, 20 );

    aData.aMarkArrays.push_back( aMarkArrayTestData1 );
    aData.aMarkArrays.push_back( aMarkArrayTestData2 );
    aData.aMarkArrays.push_back( aMarkArrayTestData3 );
    aData.aMarkArrays.push_back( aMarkArrayTestData4 );
    aData.aMarkArrays.push_back( aMarkArrayTestData5 );
    aData.aMarkArrays.push_back( aMarkArrayTestData6 );
    aData.aMarkArrays.push_back( aMarkArrayTestData7 );

    aData.aColsWithOneMark.emplace_back( 5, 20, 0, 0, 20, 0 );
    aData.aColsWithOneMark.emplace_back( 22, 20, 0, 0, 20, 0 );
    aData.aColsWithOneMark.emplace_back( 32, 20, 0, 0, 20, 0 );
    aData.aColsWithOneMark.emplace_back( 35, 0, 0, 0, MAXROW, 0 );
    aData.aColsWithOneMark.emplace_back( 50, 20, 0, 0, 20, 0 );

    aData.aColsWithoutOneMark.push_back( 10 );
    aData.aColsWithoutOneMark.push_back( 15 );
    aData.aColsWithoutOneMark.push_back( 20 );
    aData.aColsWithoutOneMark.push_back( 25 );
    aData.aColsWithoutOneMark.push_back( 30 );

    aData.aColsAllMarked.emplace_back( 10, 5, 0, 0, 10, 0 );
    aData.aColsAllMarked.emplace_back( 15, 5, 0, 0, 10, 0 );
    aData.aColsAllMarked.emplace_back( 5, 20, 0, 0, 20, 0 );
    aData.aColsAllMarked.emplace_back( 10, 20, 0, 0, 20, 0 );
    aData.aColsAllMarked.emplace_back( 25, 7, 0, 0, 15, 0 );
    aData.aColsAllMarked.emplace_back( 30, 7, 0, 0, 15, 0 );
    aData.aColsAllMarked.emplace_back( 35, 0, 0, 0, MAXROW, 0 );
    aData.aColsAllMarked.emplace_back( 100, 20, 0, 0, 20, 0 );

    aData.aColsNotAllMarked.emplace_back( 5, 5, 0, 0, 25, 0 );
    aData.aColsNotAllMarked.emplace_back( 15, 5, 0, 0, 25, 0 );
    aData.aColsNotAllMarked.emplace_back( 22, 15, 0, 0, 25, 0 );
    aData.aColsNotAllMarked.emplace_back( 27, 7, 0, 0, 20, 0 );
    aData.aColsNotAllMarked.emplace_back( 100, 19, 0, 0, 21, 0 );

    aData.aColsWithEqualMarksList.emplace_back( 0, 9 );
    aData.aColsWithEqualMarksList.emplace_back( 10, 20 );
    aData.aColsWithEqualMarksList.emplace_back( 21, 24 );
    aData.aColsWithEqualMarksList.emplace_back( 25, 30 );
    aData.aColsWithEqualMarksList.emplace_back( 31, 34 );
    aData.aColsWithEqualMarksList.emplace_back( 36, 100 );
    aData.aColsWithEqualMarksList.emplace_back( 0, 22 );
    aData.aColsWithEqualMarksList.emplace_back( 0, 34 );

    aData.aColsWithUnequalMarksList.emplace_back( 0, 10 );
    aData.aColsWithUnequalMarksList.emplace_back( 0, 20 );
    aData.aColsWithUnequalMarksList.emplace_back( 10, 21 );
    aData.aColsWithUnequalMarksList.emplace_back( 20, 25 );
    aData.aColsWithUnequalMarksList.emplace_back( 20, 30 );
    aData.aColsWithUnequalMarksList.emplace_back( 24, 30 );
    aData.aColsWithUnequalMarksList.emplace_back( 30, 31 );
    aData.aColsWithUnequalMarksList.emplace_back( 30, 34 );
    aData.aColsWithUnequalMarksList.emplace_back( 30, 35 );
    aData.aColsWithUnequalMarksList.emplace_back( 35, 100 );

    testMultiMark( aData );
}

void Test::testMultiMark_NegativeMarking()
{
    MultiMarkTestData aData;

    // Create full row = 5
    MarkTestData aSingle1;
    aSingle1.aRange = ScRange( 0, 5, 0, MAXCOL, 5, 0 );
    aSingle1.bMark = true;

    // Create rectangle ( 10, 8, 25, 20 )
    MarkTestData aSingle2;
    aSingle2.aRange = ScRange( 10, 8, 0, 25, 20, 0 );
    aSingle2.bMark = true;

    // Create full row = 12
    MarkTestData aSingle3;
    aSingle3.aRange = ScRange( 0, 12, 0, MAXCOL, 12, 0 );
    aSingle3.bMark = true;

    // Create deselection rectangle ( 17, 5, 20, 5 )
    MarkTestData aSingle4;
    aSingle4.aRange = ScRange( 17, 5, 0, 20, 5, 0 );
    aSingle4.bMark = false;

    aSingle4.aInsideAddresses.emplace_back( 6, 5, 0 );
    aSingle4.aInsideAddresses.emplace_back( 30, 5, 0 );
    aSingle4.aInsideAddresses.emplace_back( 6, 12, 0 );
    aSingle4.aInsideAddresses.emplace_back( 30, 12, 0 );
    aSingle4.aInsideAddresses.emplace_back( 13, 14, 0 );
    aSingle4.aInsideAddresses.emplace_back( 16, 12, 0 );

    aSingle4.aOutsideAddresses.emplace_back( 5, 2, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 18, 5, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 17, 5, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 20, 5, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 18, 7, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 5, 10, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 30, 10, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 5, 14, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 30, 14, 0 );
    aSingle4.aOutsideAddresses.emplace_back( 18, 25, 0 );

    aSingle4.aRowsWithFullMarks.push_back( 12 );

    aSingle4.aRowsWithoutFullMarks.push_back( 5 );

    aSingle4.aRangesWithFullMarks.emplace_back( 5, 5, 0, 16, 5, 0 );
    aSingle4.aRangesWithFullMarks.emplace_back( 21, 5, 0, 30, 5, 0 );
    aSingle4.aRangesWithFullMarks.emplace_back( 5, 12, 0, 9, 12, 0 );
    aSingle4.aRangesWithFullMarks.emplace_back( 26, 12, 0, 30, 12, 0 );
    aSingle4.aRangesWithFullMarks.emplace_back( 10, 8, 0, 25, 20, 0 );

    aSingle4.aRangesWithoutFullMarks.emplace_back( 10, 5, 0, 25, 5, 0 );

    aSingle4.aNextMarked.emplace_back( 18, 7, 0, 0, -1, 0 ); // up fail
    aSingle4.aNextMarked.emplace_back( 18, 4, 0, 1, 8, 0 ); // down ok

    // Create deselection rectangle ( 15, 10, 18, 14 )
    MarkTestData aSingle5;
    aSingle5.aRange = ScRange( 15, 10, 0, 18, 14, 0 );
    aSingle5.bMark = false;

    aSingle5.aInsideAddresses.emplace_back( 6, 5, 0 );
    aSingle5.aInsideAddresses.emplace_back( 30, 5, 0 );
    aSingle5.aInsideAddresses.emplace_back( 6, 12, 0 );
    aSingle5.aInsideAddresses.emplace_back( 30, 12, 0 );
    aSingle5.aInsideAddresses.emplace_back( 13, 14, 0 );

    aSingle5.aOutsideAddresses = aSingle4.aOutsideAddresses;
    aSingle5.aOutsideAddresses.emplace_back( 17, 12, 0 );
    aSingle5.aOutsideAddresses.emplace_back( 15, 10, 0 );
    aSingle5.aOutsideAddresses.emplace_back( 18, 10, 0 );
    aSingle5.aOutsideAddresses.emplace_back( 15, 14, 0 );
    aSingle5.aOutsideAddresses.emplace_back( 18, 14, 0 );

    aSingle5.aRowsWithoutFullMarks.push_back( 12 );
    aSingle5.aRowsWithoutFullMarks.push_back( 5 );

    aSingle5.aRangesWithoutFullMarks.emplace_back( 10, 8, 0, 25, 20, 0 );

    aSingle5.aNextMarked = aSingle4.aNextMarked;
    aSingle5.aNextMarked.emplace_back( 17, 12, 0, 0, 9, 0 ); // up ok
    aSingle5.aNextMarked.emplace_back( 17, 12, 0, 1, 15, 0 ); // down ok

    // Add the rectangle data to aData
    aData.aMarks.push_back( aSingle1 );
    aData.aMarks.push_back( aSingle2 );
    aData.aMarks.push_back( aSingle3 );
    aData.aMarks.push_back( aSingle4 );
    aData.aMarks.push_back( aSingle5 );

    aData.aSelectionCover = ScRange( 0, 4, 0, MAXCOL, 21, 0 );
    aData.aLeftEnvelope.push_back( ScRange( 9, 8, 0, 9, 11, 0 ) );
    aData.aLeftEnvelope.push_back( ScRange( 9, 13, 0, 9, 20, 0 ) );
    aData.aLeftEnvelope.push_back( ScRange( 18, 10, 0, 18, 14, 0 ) );
    aData.aLeftEnvelope.push_back( ScRange( 20, 5, 0, 20, 5, 0 ) );

    aData.aRightEnvelope.push_back( ScRange( 17, 5, 0, 17, 5, 0 ) );
    aData.aRightEnvelope.push_back( ScRange( 15, 10, 0, 15, 14, 0 ) );
    aData.aRightEnvelope.push_back( ScRange( 26, 8, 0, 26, 11, 0 ) );
    aData.aRightEnvelope.push_back( ScRange( 26, 13, 0, 26, 20, 0 ) );

    aData.aTopEnvelope.push_back( ScRange( 0, 4, 0, 16, 4, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 21, 4, 0, MAXCOL, 4, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 10, 7, 0, 25, 7, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 0, 11, 0, 9, 11, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 26, 11, 0, MAXCOL, 11, 0 ) );
    aData.aTopEnvelope.push_back( ScRange( 15, 14, 0, 18, 14, 0 ) );

    aData.aBottomEnvelope.push_back( ScRange( 0, 6, 0, 16, 6, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 21, 6, 0, MAXCOL, 6, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 15, 10, 0, 18, 10, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 0, 13, 0, 9, 13, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 26, 13, 0, MAXCOL, 13, 0 ) );
    aData.aBottomEnvelope.push_back( ScRange( 10, 21, 0, 25, 21, 0 ) );

    aData.aColsWithOneMark.emplace_back( 19, 8, 0, 0, 20, 0 );
    aData.aColsWithOneMark.emplace_back( 20, 8, 0, 0, 20, 0 );

    aData.aColsWithoutOneMark.push_back( 5 );
    aData.aColsWithoutOneMark.push_back( 10 );
    aData.aColsWithoutOneMark.push_back( 12 );
    aData.aColsWithoutOneMark.push_back( 16 );
    aData.aColsWithoutOneMark.push_back( 17 );
    aData.aColsWithoutOneMark.push_back( 24 );
    aData.aColsWithoutOneMark.push_back( 100 );

    aData.aColsAllMarked.emplace_back( 10, 8, 0, 0, 20, 0 );
    aData.aColsAllMarked.emplace_back( 17, 8, 0, 0, 9, 0 );
    aData.aColsAllMarked.emplace_back( 20, 8, 0, 0, 20, 0 );
    aData.aColsAllMarked.emplace_back( 100, 5, 0, 0, 5, 0 );
    aData.aColsAllMarked.emplace_back( 100, 12, 0, 0, 12, 0 );

    aData.aColsNotAllMarked.emplace_back( 5, 5, 0, 0, 12, 0 );
    aData.aColsNotAllMarked.emplace_back( 10, 5, 0, 0, 20, 0 );
    aData.aColsNotAllMarked.emplace_back( 15, 8, 0, 0, 20, 0 );
    aData.aColsNotAllMarked.emplace_back( 18, 8, 0, 0, 20, 0 );
    aData.aColsNotAllMarked.emplace_back( 25, 5, 0, 0, 20, 0 );

    aData.aColsWithEqualMarksList.emplace_back( 0, 9 );
    aData.aColsWithEqualMarksList.emplace_back( 10, 14 );
    aData.aColsWithEqualMarksList.emplace_back( 15, 16 );
    aData.aColsWithEqualMarksList.emplace_back( 17, 18 );
    aData.aColsWithEqualMarksList.emplace_back( 19, 20 );
    aData.aColsWithEqualMarksList.emplace_back( 21, 25 );
    aData.aColsWithEqualMarksList.emplace_back( 26, 100 );
    aData.aColsWithEqualMarksList.emplace_back( 0, 100 );

    aData.aColsWithUnequalMarksList.emplace_back( 0, 10 );
    aData.aColsWithUnequalMarksList.emplace_back( 10, 15 );
    aData.aColsWithUnequalMarksList.emplace_back( 15, 17 );
    aData.aColsWithUnequalMarksList.emplace_back( 17, 19 );
    aData.aColsWithUnequalMarksList.emplace_back( 19, 21 );
    aData.aColsWithUnequalMarksList.emplace_back( 21, 26 );

    testMultiMark( aData );
}

void Test::testInsertTabBeforeSelected()
{
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    aMark.InsertTab(0);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), aMark.GetSelectCount());
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), aMark.GetFirstSelected());
}

void Test::testInsertTabAfterSelected()
{
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    aMark.InsertTab(1);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), aMark.GetSelectCount());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), aMark.GetFirstSelected());
}

void Test::testDeleteTabBeforeSelected()
{
    ScMarkData aMark;
    aMark.SelectOneTable(1);
    aMark.DeleteTab(0);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), aMark.GetSelectCount());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), aMark.GetFirstSelected());
}

void Test::testDeleteTabAfterSelected()
{
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    aMark.DeleteTab(1);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), aMark.GetSelectCount());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), aMark.GetFirstSelected());
}

void Test::testScMarkArraySearch_check(const ScMarkArray & ar, SCROW nRow, bool expectStatus, SCSIZE nIndexExpect)
{
    SCSIZE nIndex = 0;
    bool status = ar.Search(nRow, nIndex);
    CPPUNIT_ASSERT_EQUAL(expectStatus, status);
    CPPUNIT_ASSERT_EQUAL(nIndexExpect, nIndex);
}

void Test::testScMarkArraySearch()
{
    // empty
    {
        ScMarkArray ar;
        testScMarkArraySearch_check(ar, -1, false, 0);
        testScMarkArraySearch_check(ar, 100, false, 0);
    }

    // one range
    {
        ScMarkArray ar;
        ar.SetMarkArea(10, 20, true);

        // 0-9,10-20,21+

        testScMarkArraySearch_check(ar, -100, true, 0);
        testScMarkArraySearch_check(ar, -1, true, 0);

        testScMarkArraySearch_check(ar, 0,  true, 0);
        testScMarkArraySearch_check(ar, 5,  true, 0);
        testScMarkArraySearch_check(ar, 9,  true, 0);
        testScMarkArraySearch_check(ar, 10, true, 1);
        testScMarkArraySearch_check(ar, 11, true, 1);
        testScMarkArraySearch_check(ar, 19, true, 1);
        testScMarkArraySearch_check(ar, 20, true, 1);
        testScMarkArraySearch_check(ar, 21, true, 2);
        testScMarkArraySearch_check(ar, 22, true, 2);
    }

    // three ranges
    {
        ScMarkArray ar;
        ar.SetMarkArea(10, 20, true);
        ar.SetMarkArea(21, 30, true);
        ar.SetMarkArea(50, 100, true);

        // 0-9,10-30,31-49,50-100,101+

        testScMarkArraySearch_check(ar, -100, true, 0);
        testScMarkArraySearch_check(ar, -1, true, 0);

        testScMarkArraySearch_check(ar, 5,  true, 0);
        testScMarkArraySearch_check(ar, 15, true, 1);
        testScMarkArraySearch_check(ar, 25, true, 1);
        testScMarkArraySearch_check(ar, 35, true, 2);
        testScMarkArraySearch_check(ar, 55, true, 3);
        testScMarkArraySearch_check(ar, 20, true, 1);
        testScMarkArraySearch_check(ar, 21, true, 1);
    }

    // three single-row ranges
    {
        ScMarkArray ar;
        ar.SetMarkArea(4, 4, true);
        ar.SetMarkArea(6, 6, true);
        ar.SetMarkArea(8, 8, true);

        testScMarkArraySearch_check(ar, -100, true, 0);
        testScMarkArraySearch_check(ar, -1, true, 0);

        testScMarkArraySearch_check(ar, 3,  true, 0);
        testScMarkArraySearch_check(ar, 4, true, 1);
        testScMarkArraySearch_check(ar, 5, true, 2);
        testScMarkArraySearch_check(ar, 6, true, 3);
        testScMarkArraySearch_check(ar, 7, true, 4);
        testScMarkArraySearch_check(ar, 8, true, 5);
        testScMarkArraySearch_check(ar, 9, true, 6);
        testScMarkArraySearch_check(ar, 10, true, 6);
    }

    // one range
    {
        ScMarkArray ar;
        ar.SetMarkArea(10, MAXROW, true);

        // 0-10,11+

        testScMarkArraySearch_check(ar, -100, true, 0);
        testScMarkArraySearch_check(ar, -1, true, 0);

        testScMarkArraySearch_check(ar, 0,  true, 0);
        testScMarkArraySearch_check(ar, 5,  true, 0);
        testScMarkArraySearch_check(ar, 9,  true, 0);
        testScMarkArraySearch_check(ar, 10, true, 1);
        testScMarkArraySearch_check(ar, 11, true, 1);
        testScMarkArraySearch_check(ar, 12, true, 1);
        testScMarkArraySearch_check(ar, 200, true, 1);
        testScMarkArraySearch_check(ar, MAXROW, true, 1);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
