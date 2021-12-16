/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>
#include <string_view>

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <osl/file.hxx>
#include <config_features.h>

#include <vcl/svapp.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <svl/stritem.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svx/svdograf.hxx>
#include <svx/svxids.hrc>

#include <drwlayer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <dbdata.hxx>
#include <validat.hxx>
#include <formulacell.hxx>
#include <formulaopt.hxx>
#include <userdat.hxx>
#include <stlsheet.hxx>
#include <docfunc.hxx>
#include <markdata.hxx>
#include <colorscale.hxx>
#include <olinetab.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <docsh.hxx>
#include <editutil.hxx>
#include <cellvalue.hxx>
#include <attrib.hxx>
#include <fillinfo.hxx>
#include <scopetools.hxx>
#include <columnspanset.hxx>
#include <tokenstringcontext.hxx>
#include <formula/errorcodes.hxx>
#include <externalrefmgr.hxx>
#include <stlpool.hxx>
#include <hints.hxx>
#include <detfunc.hxx>
#include <cellmergeoption.hxx>
#include <undoblk.hxx>

#include <orcusfilters.hxx>
#include <filter.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

#include <comphelper/scopeguard.hxx>
#include <tools/UnitConversion.hxx>
#include <unotools/syslocaleoptions.hxx>
#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScFiltersTest2 : public test::FiltersTest, public ScBootstrapFixture
{
public:
    ScFiltersTest2();

    virtual bool load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                      SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                      unsigned int nFilterVersion) override;

    virtual void setUp() override;
    virtual void tearDown() override;

    void testMiscRowHeights();
    void testOptimalHeightReset();
    void testCustomNumFormatHybridCellODS();
    void testTdf121040();
    void testTdf118086();
    void testTdf118624();
    void testTdf124454();

    void testPrintRangeODS();
    void testOutlineODS();

    void testColumnStyleXLSX();
    void testColumnStyleAutoFilterXLSX();

    void testSharedFormulaHorizontalXLS();
    void testSharedFormulaWrappedRefsXLS();
    void testSharedFormulaBIFF5();
    void testSharedFormulaXLSB();
    void testSharedFormulaXLS();
    void testSharedFormulaColumnLabelsODS();
    void testSharedFormulaColumnRowLabelsODS();
    void testExternalRefCacheXLSX();
    void testExternalRefCacheODS();
    void testHybridSharedStringODS();
    void testCopyMergedNumberFormats();
    void testVBAUserFunctionXLSM();
    void testEmbeddedImageXLS();
    void testErrorOnExternalReferences();
    void testTdf145054();
    void testTdf84762();
    void testTdf44076();
    void testEditEngStrikeThroughXLSX();
    void testRefStringXLSX();
    void testHiddenSheetsXLSX();
    void testRelFormulaValidationXLS();
    void testTdf130132();
    void testTdf133327();
    void testColumnStyle2XLSX();
    void testAutofilterXLSX();

    void testBnc762542();

    void testTdf136364();
    void testTdf103734();
    void testTdf126116();
    void testTdf144209();
    void testTdf98844();
    void testTdf100458();
    void testTdf118561();
    void testTdf125099();
    void testTdf134455();
    void testTdf119533();
    void testTdf127982();
    void testTdf109409();
    void testTdf132105();
    void testTdf131424();
    void testTdf100709XLSX();
    void testTdf97598XLSX();
    void testTdf110440XLSX();
    void testTdf83672XLSX();

    void testCondFormatXLSB();
    void testPageScalingXLSX();
    void testActiveXCheckboxXLSX();
    void testtdf120301_xmlSpaceParsingXLSX();
    void testUnicodeFileNameGnumeric();
    void testCondFormatFormulaListenerXLSX();

    void testMergedCellsXLSXML();
    void testBackgroundColorStandardXLSXML();
    void testTdf131536();
    void testTdf130583();
    void testTdf85617();
    void testTdf134234();
    void testTdf42481();
    void testNamedExpressionsXLSXML();
    void testEmptyRowsXLSXML();
    void testBorderDirectionsXLSXML();
    void testBorderColorsXLSXML();
    void testHiddenRowsColumnsXLSXML();
    void testColumnWidthRowHeightXLSXML();
    void testCharacterSetXLSXML();
    void testTdf137091();
    void testTdf62268();
    void testTdf137453();
    void testTdf112780();
    void testTdf72470();
    void testTdf35636();
    void testTdf98481();
    void testTdf115022();
    void testVBAMacroFunctionODS();
    void testAutoheight2Rows();
    void testXLSDefColWidth();
    void testPreviewMissingObjLink();
    void testShapeRotationImport();
    void testShapeDisplacementOnRotationImport();
    void testTextBoxBodyUpright();
    void testTextBoxBodyRotateAngle();
    void testTextLengthDataValidityXLSX();
    void testDeleteCircles();
    void testDrawCircleInMergeCells();
    void testDeleteCirclesInRowAndCol();
    void testTdf129940();
    void testTdf139612();
    void testTdf144740();
    void testTdf139763ShapeAnchor();
    void testAutofilterNamedRangesXLSX();
    void testInvalidBareBiff5();

    CPPUNIT_TEST_SUITE(ScFiltersTest2);

    CPPUNIT_TEST(testMiscRowHeights);
    CPPUNIT_TEST(testOptimalHeightReset);
    CPPUNIT_TEST(testCustomNumFormatHybridCellODS);
    CPPUNIT_TEST(testTdf121040);
    CPPUNIT_TEST(testTdf118086);
    CPPUNIT_TEST(testTdf118624);
    CPPUNIT_TEST(testTdf124454);
    CPPUNIT_TEST(testPrintRangeODS);
    CPPUNIT_TEST(testOutlineODS);
    CPPUNIT_TEST(testColumnStyleXLSX);
    CPPUNIT_TEST(testColumnStyleAutoFilterXLSX);
    CPPUNIT_TEST(testSharedFormulaHorizontalXLS);
    CPPUNIT_TEST(testSharedFormulaWrappedRefsXLS);
    CPPUNIT_TEST(testSharedFormulaBIFF5);
    CPPUNIT_TEST(testSharedFormulaXLSB);
    CPPUNIT_TEST(testSharedFormulaXLS);
    CPPUNIT_TEST(testSharedFormulaColumnLabelsODS);
    CPPUNIT_TEST(testSharedFormulaColumnRowLabelsODS);
    CPPUNIT_TEST(testExternalRefCacheXLSX);
    CPPUNIT_TEST(testExternalRefCacheODS);
    CPPUNIT_TEST(testHybridSharedStringODS);
    CPPUNIT_TEST(testCopyMergedNumberFormats);
    CPPUNIT_TEST(testVBAUserFunctionXLSM);
    CPPUNIT_TEST(testEmbeddedImageXLS);
    CPPUNIT_TEST(testErrorOnExternalReferences);
    CPPUNIT_TEST(testTdf145054);
    CPPUNIT_TEST(testTdf84762);
    CPPUNIT_TEST(testTdf44076);
    CPPUNIT_TEST(testEditEngStrikeThroughXLSX);
    CPPUNIT_TEST(testRefStringXLSX);
    CPPUNIT_TEST(testRelFormulaValidationXLS);
    CPPUNIT_TEST(testTdf130132);
    CPPUNIT_TEST(testTdf133327);
    CPPUNIT_TEST(testColumnStyle2XLSX);
    CPPUNIT_TEST(testAutofilterXLSX);

    CPPUNIT_TEST(testBnc762542);

    CPPUNIT_TEST(testHiddenSheetsXLSX);

    CPPUNIT_TEST(testTdf136364);
    CPPUNIT_TEST(testTdf103734);
    CPPUNIT_TEST(testTdf126116);
    CPPUNIT_TEST(testTdf144209);
    CPPUNIT_TEST(testTdf98844);
    CPPUNIT_TEST(testTdf100458);
    CPPUNIT_TEST(testTdf118561);
    CPPUNIT_TEST(testTdf125099);
    CPPUNIT_TEST(testTdf134455);
    CPPUNIT_TEST(testTdf119533);
    CPPUNIT_TEST(testTdf127982);
    CPPUNIT_TEST(testTdf109409);
    CPPUNIT_TEST(testTdf132105);
    CPPUNIT_TEST(testTdf131424);
    CPPUNIT_TEST(testTdf100709XLSX);
    CPPUNIT_TEST(testTdf97598XLSX);
    CPPUNIT_TEST(testTdf110440XLSX);
    CPPUNIT_TEST(testTdf83672XLSX);

    CPPUNIT_TEST(testCondFormatXLSB);
    CPPUNIT_TEST(testPageScalingXLSX);
    CPPUNIT_TEST(testActiveXCheckboxXLSX);
    CPPUNIT_TEST(testtdf120301_xmlSpaceParsingXLSX);
    CPPUNIT_TEST(testUnicodeFileNameGnumeric);
    CPPUNIT_TEST(testMergedCellsXLSXML);
    CPPUNIT_TEST(testBackgroundColorStandardXLSXML);
    CPPUNIT_TEST(testTdf131536);
    CPPUNIT_TEST(testTdf130583);
    CPPUNIT_TEST(testTdf85617);
    CPPUNIT_TEST(testTdf134234);
    CPPUNIT_TEST(testTdf42481);
    CPPUNIT_TEST(testNamedExpressionsXLSXML);
    CPPUNIT_TEST(testEmptyRowsXLSXML);
    CPPUNIT_TEST(testBorderDirectionsXLSXML);
    CPPUNIT_TEST(testBorderColorsXLSXML);
    CPPUNIT_TEST(testHiddenRowsColumnsXLSXML);
    CPPUNIT_TEST(testColumnWidthRowHeightXLSXML);
    CPPUNIT_TEST(testCharacterSetXLSXML);
    CPPUNIT_TEST(testCondFormatFormulaListenerXLSX);
    CPPUNIT_TEST(testTdf137091);
    CPPUNIT_TEST(testTdf62268);
    CPPUNIT_TEST(testTdf137453);
    CPPUNIT_TEST(testTdf112780);
    CPPUNIT_TEST(testTdf72470);
    CPPUNIT_TEST(testTdf35636);
    CPPUNIT_TEST(testTdf98481);
    CPPUNIT_TEST(testTdf115022);
    CPPUNIT_TEST(testVBAMacroFunctionODS);
    CPPUNIT_TEST(testAutoheight2Rows);
    CPPUNIT_TEST(testXLSDefColWidth);
    CPPUNIT_TEST(testPreviewMissingObjLink);
    CPPUNIT_TEST(testShapeRotationImport);
    CPPUNIT_TEST(testShapeDisplacementOnRotationImport);
    CPPUNIT_TEST(testTextBoxBodyUpright);
    CPPUNIT_TEST(testTextBoxBodyRotateAngle);
    CPPUNIT_TEST(testTextLengthDataValidityXLSX);
    CPPUNIT_TEST(testDeleteCircles);
    CPPUNIT_TEST(testDrawCircleInMergeCells);
    CPPUNIT_TEST(testDeleteCirclesInRowAndCol);
    CPPUNIT_TEST(testTdf129940);
    CPPUNIT_TEST(testTdf139612);
    CPPUNIT_TEST(testTdf144740);
    CPPUNIT_TEST(testTdf139763ShapeAnchor);
    CPPUNIT_TEST(testAutofilterNamedRangesXLSX);
    CPPUNIT_TEST(testInvalidBareBiff5);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

ScFiltersTest2::ScFiltersTest2()
    : ScBootstrapFixture("sc/qa/unit/data")
{
}

void ScFiltersTest2::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScFiltersTest2::tearDown()
{
    uno::Reference<lang::XComponent>(m_xCalcComponent, UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();
}

bool ScFiltersTest2::load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                          SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                          unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData, OUString(),
                                                       nFilterFlags, nClipboardID, nFilterVersion);
    bool bLoaded = xDocShRef.is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScFiltersTest2::testMiscRowHeights()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    static const TestParam::RowData DfltRowData[] = {
        // check rows at the beginning and end of document
        // and make sure they are reported as the default row
        // height ( indicated by -1 )
        { 2, 4, 0, -1, 0, false },
        { 1048573, 1048575, 0, -1, 0, false },
    };

    static const TestParam::RowData MultiLineOptData[] = {
        // Row 0 is 12.63 mm and optimal flag is set => 12.36 mm
        { 0, 0, 0, 1236, CHECK_OPTIMAL, true },
        // Row 1 is 11.99 mm and optimal flag is NOT set
        { 1, 1, 0, 1199, CHECK_OPTIMAL, false },
    };

    TestParam aTestValues[] = {
        /* Checks that a document saved to ods with default rows does indeed
           have default row heights ( there was a problem where the optimal
           height was being calculated after import if no hard height )
        */
        { "alldefaultheights.", FORMAT_ODS, -1, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        /* Checks the imported height of some multiline input, additionally checks
           that the optimal height flag is set ( or not )
        */
        { "multilineoptimal.", FORMAT_ODS, -1, SAL_N_ELEMENTS(MultiLineOptData), MultiLineOptData },
    };
    miscRowHeightsTest(aTestValues, SAL_N_ELEMENTS(aTestValues));
}

// regression test at least fdo#59193
// what we want to test here is that when cell contents are deleted
// and the optimal flag is set for that row that the row is actually resized

void ScFiltersTest2::testOptimalHeightReset()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    ScDocShellRef xDocSh = loadDoc(u"multilineoptimal.", FORMAT_ODS, true);
    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument& rDoc = xDocSh->GetDocument();
    // open document in read/write mode ( otherwise optimal height stuff won't
    // be triggered ) *and* you can't delete cell contents.
    int nHeight = convertTwipToMm100(rDoc.GetRowHeight(nRow, nTab, false));
    CPPUNIT_ASSERT_EQUAL(1236, nHeight);

    ScDocFunc& rFunc = xDocSh->GetDocFunc();

    // delete content of A1
    ScRange aDelRange(0, 0, 0, 0, 0, 0);
    ScMarkData aMark(rDoc.GetSheetLimits());
    aMark.SetMarkArea(aDelRange);
    bool bRet = rFunc.DeleteContents(aMark, InsertDeleteFlags::ALL, false, true);
    CPPUNIT_ASSERT_MESSAGE("DeleteContents failed", bRet);

    // get the new height of A1
    nHeight = convertTwipToMm100(rDoc.GetRowHeight(nRow, nTab, false));

    // set optimal height for empty row 2
    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(2, 2));
    rFunc.SetWidthOrHeight(false, aRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true);

    // retrieve optimal height
    int nOptimalHeight = convertTwipToMm100(rDoc.GetRowHeight(aRowArr[0].mnStart, nTab, false));

    // check if the new height of A1 ( after delete ) is now the optimal height of an empty cell
    CPPUNIT_ASSERT_EQUAL(nOptimalHeight, nHeight);
    xDocSh->DoClose();
}

void ScFiltersTest2::testCustomNumFormatHybridCellODS()
{
    ScDocShellRef xDocSh = loadDoc(u"custom-numfmt-hybrid-cell.", FORMAT_ODS, false);
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.SetAutoCalc(true);

    // All of B14, B16 and B18 should be displaying empty strings by virtue
    // of the custom number format being set on those cells.

    for (SCROW nRow : { 13, 15, 17 })
    {
        ScAddress aPos(1, nRow, 0);
        OUString aStr = rDoc.GetString(aPos);
        CPPUNIT_ASSERT(aStr.isEmpty());
    }

    // Now, set value of 1 to B15.  This should trigger re-calc on B18 and B18
    // should now show a value of 1.
    rDoc.SetValue(ScAddress(1, 15, 0), 1.0);

    OUString aStr = rDoc.GetString(ScAddress(1, 17, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), aStr);

    // Make sure the cell doesn't have an error value.
    ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(1, 17, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NONE, pFC->GetErrCode());

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf121040()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf121040.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf121040.ods", xDocSh.is());

    const SCTAB nTab = 0;
    ScDocument& rDoc = xDocSh->GetDocument();

    // The first 9 rows should have the same height
    const sal_uInt16 nHeight = rDoc.GetRowHeight(0, nTab, false);
    for (SCTAB nRow = 1; nRow < 9; nRow++)
    {
        CPPUNIT_ASSERT_EQUAL(nHeight, rDoc.GetRowHeight(nRow, nTab, false));
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf118086()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf118086.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf118086.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(477), rDoc.GetRowHeight(2, static_cast<SCTAB>(0), false));

    // Without the fix in place, this test would have failed with
    // - Expected: 256
    // - Actual  : 477
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(256), rDoc.GetRowHeight(2, static_cast<SCTAB>(1), false));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(256), rDoc.GetRowHeight(2, static_cast<SCTAB>(2), false));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf118624()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf118624.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("RAND() in array/matrix mode shouldn't return the same value",
                           rDoc.GetString(ScAddress(0, 0, 0))
                               != rDoc.GetString(ScAddress(0, 1, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf124454()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf124454.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(2, 0, 0)));
    // Without the fix in place, double negation with text in array
    // would have returned -1
    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(3, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testPrintRangeODS()
{
    ScDocShellRef xDocSh = loadDoc(u"print-range.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    const ScRange* pRange = rDoc.GetRepeatRowRange(0);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 1, 0), *pRange);

    pRange = rDoc.GetRepeatRowRange(1);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 2, 0, 0, 4, 0), *pRange);

    xDocSh->DoClose();
}

void ScFiltersTest2::testOutlineODS()
{
    ScDocShellRef xDocSh = loadDoc(u"outline.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    const ScOutlineTable* pTable = rDoc.GetOutlineTable(0);
    CPPUNIT_ASSERT(pTable);

    const ScOutlineArray& rArr = pTable->GetRowArray();
    size_t nDepth = rArr.GetDepth();
    CPPUNIT_ASSERT_EQUAL(size_t(4), nDepth);

    for (size_t i = 0; i < nDepth; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(size_t(1), rArr.GetCount(i));
    }

    struct OutlineData
    {
        SCCOLROW nStart;
        SCCOLROW nEnd;
        bool bHidden;
        bool bVisible;

        size_t nDepth;
        size_t nIndex;
    };

    static const OutlineData aRow[] = { { 1, 29, false, true, 0, 0 },
                                        { 2, 26, false, true, 1, 0 },
                                        { 4, 23, false, true, 2, 0 },
                                        { 6, 20, true, true, 3, 0 } };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aRow); ++i)
    {
        const ScOutlineEntry* pEntry = rArr.GetEntry(aRow[i].nDepth, aRow[i].nIndex);
        SCCOLROW nStart = pEntry->GetStart();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nStart, nStart);

        SCCOLROW nEnd = pEntry->GetEnd();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nEnd, nEnd);

        bool bHidden = pEntry->IsHidden();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bHidden, bHidden);

        bool bVisible = pEntry->IsVisible();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bVisible, bVisible);
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testColumnStyleXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"column-style.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    const ScPatternAttr* pPattern = rDoc.GetPattern(0, 0, 0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttr = pPattern->GetItem(ATTR_PROTECTION);
    CPPUNIT_ASSERT(rAttr.GetProtection());

    pPattern = rDoc.GetPattern(0, 1, 0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttrNew = pPattern->GetItem(ATTR_PROTECTION);
    CPPUNIT_ASSERT(!rAttrNew.GetProtection());

    xDocSh->DoClose();
}

void ScFiltersTest2::testColumnStyleAutoFilterXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"column-style-autofilter.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    const ScPatternAttr* pPattern = rDoc.GetPattern(0, 10, 18);
    CPPUNIT_ASSERT(pPattern);

    const ScMergeFlagAttr& rAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
    CPPUNIT_ASSERT(!rAttr.HasAutoFilter());

    xDocSh->DoClose();
}

void ScFiltersTest2::testSharedFormulaHorizontalXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/horizontal.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Make sure K2:S2 on the 2nd sheet are all formula cells.
    ScAddress aPos(0, 1, 1);
    for (SCCOL nCol = 10; nCol <= 18; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell is expected here.", CELLTYPE_FORMULA,
                                     rDoc.GetCellType(aPos));
    }

    // Likewise, B3:J9 all should be formula cells.
    for (SCCOL nCol = 1; nCol <= 9; ++nCol)
    {
        aPos.SetCol(nCol);
        for (SCROW nRow = 2; nRow <= 8; ++nRow)
        {
            aPos.SetRow(nRow);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell is expected here.", CELLTYPE_FORMULA,
                                         rDoc.GetCellType(aPos));
        }
    }

    // B2:I2 too.
    aPos.SetRow(1);
    for (SCCOL nCol = 1; nCol <= 8; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell is expected here.", CELLTYPE_FORMULA,
                                     rDoc.GetCellType(aPos));
    }

    // J2 has a string of "MW".
    aPos.SetCol(9);
    CPPUNIT_ASSERT_EQUAL(OUString("MW"), rDoc.GetString(aPos));

    xDocSh->DoClose();
}

void ScFiltersTest2::testSharedFormulaWrappedRefsXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/wrapped-refs.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // Check the values of H7:H10.
    CPPUNIT_ASSERT_EQUAL(7.0, rDoc.GetValue(ScAddress(7, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(8.0, rDoc.GetValue(ScAddress(7, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(9.0, rDoc.GetValue(ScAddress(7, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(7, 9, 0)));

    // EM7:EM10 should reference H7:H10.
    CPPUNIT_ASSERT_EQUAL(7.0, rDoc.GetValue(ScAddress(142, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(8.0, rDoc.GetValue(ScAddress(142, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(9.0, rDoc.GetValue(ScAddress(142, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(142, 9, 0)));

    // Make sure EM7:EM10 are grouped.
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(142, 6, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());

    xDocSh->DoClose();
}

void ScFiltersTest2::testSharedFormulaBIFF5()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/biff5.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // E6:E376 should be all formulas, and they should belong to the same group.
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(4, 5, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(371), pFC->GetSharedLength());

    xDocSh->DoClose();
}

void ScFiltersTest2::testSharedFormulaXLSB()
{
    ScDocShellRef xDocSh = loadDoc(u"shared_formula.", FORMAT_XLSB);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // A1:A30 should be all formulas, and they should belong to the same group.
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(30), pFC->GetSharedLength());

    for (SCROW nRow = 0; nRow < 30; ++nRow)
    {
        ASSERT_DOUBLES_EQUAL(3.0, rDoc.GetValue(0, nRow, 0));
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testSharedFormulaXLS()
{
    {
        // fdo#80091
        ScDocShellRef xDocSh = loadDoc(u"shared-formula/relative-refs1.", FORMAT_XLS);
        CPPUNIT_ASSERT(xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();
        rDoc.CalcAll();

        // A1:A30 should be all formulas, and they should belong to the same group.
        const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0, 1, 0));
        CPPUNIT_ASSERT(pFC);
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(29), pFC->GetSharedLength());

        for (SCROW nRow = 0; nRow < 30; ++nRow)
        {
            ASSERT_DOUBLES_EQUAL(double(nRow + 1), rDoc.GetValue(0, nRow, 0));
        }

        xDocSh->DoClose();
    }

    {
        // fdo#84556 and some related tests
        ScDocShellRef xDocSh = loadDoc(u"shared-formula/relative-refs2.", FORMAT_XLS);
        CPPUNIT_ASSERT(xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();
        rDoc.CalcAll();

        {
            const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(2, 1, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = rDoc.GetFormulaCell(ScAddress(2, 10, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula = rDoc.GetFormula(2, 1, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B9:D9)"), aFormula);

            aFormula = rDoc.GetFormula(2, 10, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B18:D18)"), aFormula);
        }

        {
            const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(4, 8, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = rDoc.GetFormulaCell(ScAddress(4, 17, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula = rDoc.GetFormula(4, 8, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(G9:EY9)"), aFormula);

            aFormula = rDoc.GetFormula(4, 17, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(G18:EY18)"), aFormula);
        }

        {
            const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(6, 15, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = rDoc.GetFormulaCell(ScAddress(6, 24, 0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula = rDoc.GetFormula(6, 15, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A16:A40000)"), aFormula);

            aFormula = rDoc.GetFormula(6, 24, 0);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A25:A40009)"), aFormula);
        }

        xDocSh->DoClose();
    }
}

void ScFiltersTest2::testSharedFormulaColumnLabelsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/column-labels.", FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    CPPUNIT_ASSERT_EQUAL(5.0, rDoc.GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(15.0, rDoc.GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(28.0, rDoc.GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(48.0, rDoc.GetValue(ScAddress(2, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(50.0, rDoc.GetValue(ScAddress(3, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(144.0, rDoc.GetValue(ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(147.0, rDoc.GetValue(ScAddress(3, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(320.0, rDoc.GetValue(ScAddress(3, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(ScAddress(4, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(5.0, rDoc.GetValue(ScAddress(4, 5, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testSharedFormulaColumnRowLabelsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/column-row-labels.", FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // Expected output in each of the three ranges.
    //
    // +---+---+---+
    // | 1 | 4 | 7 |
    // +---+---+---+
    // | 2 | 5 | 8 |
    // +---+---+---+
    // | 3 | 6 | 9 |
    // +---+---+---+

    auto aCheckFunc = [&](SCCOL nStartCol, SCROW nStartRow) {
        double fExpected = 1.0;
        for (SCCOL nCol = 0; nCol <= 2; ++nCol)
        {
            for (SCROW nRow = 0; nRow <= 2; ++nRow)
            {
                ScAddress aPos(nStartCol + nCol, nStartRow + nRow, 0);
                CPPUNIT_ASSERT_EQUAL(fExpected, rDoc.GetValue(aPos));
                fExpected += 1.0;
            }
        }
    };

    aCheckFunc(5, 1); // F2:H4
    aCheckFunc(9, 1); // J2:L4
    aCheckFunc(1, 6); // B7:D9

    xDocSh->DoClose();
}

void ScFiltersTest2::testExternalRefCacheXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"external-refs.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // These string values are cached external cell values.
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), rDoc.GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Andy"), rDoc.GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Bruce"), rDoc.GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie"), rDoc.GetString(ScAddress(0, 3, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testExternalRefCacheODS()
{
    ScDocShellRef xDocSh = loadDoc(u"external-ref-cache.", FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Cells B2:B4 have VLOOKUP with external references which should all show "text".
    CPPUNIT_ASSERT_EQUAL(OUString("text"), rDoc.GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("text"), rDoc.GetString(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("text"), rDoc.GetString(ScAddress(1, 3, 0)));

    // Both cells A6 and A7 should be registered with scExternalRefManager properly
    CPPUNIT_ASSERT_EQUAL(
        true, rDoc.GetExternalRefManager()->hasCellExternalReference(ScAddress(0, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(
        true, rDoc.GetExternalRefManager()->hasCellExternalReference(ScAddress(0, 6, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testHybridSharedStringODS()
{
    ScDocShellRef xDocSh = loadDoc(u"hybrid-shared-string.", FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // A2 contains formula with MATCH function.  The result must be 2, not #N/A!
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0, 1, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testCopyMergedNumberFormats()
{
    ScDocShellRef xDocSh = loadDoc(u"copy-merged-number-formats.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Cells B1, C1 and D1 are formatted as dates.
    OUString aStrB1 = rDoc.GetString(ScAddress(1, 0, 0));
    OUString aStrC1 = rDoc.GetString(ScAddress(2, 0, 0));
    OUString aStrD1 = rDoc.GetString(ScAddress(3, 0, 0));

    ScDocument aCopyDoc;
    aCopyDoc.InsertTab(0, "CopyHere");
    rDoc.CopyStaticToDocument(ScRange(1, 0, 0, 3, 0, 0), 0, aCopyDoc);

    // Make sure the date formats are copied to the new document.
    CPPUNIT_ASSERT_EQUAL(aStrB1, aCopyDoc.GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(aStrC1, aCopyDoc.GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(aStrD1, aCopyDoc.GetString(ScAddress(3, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testVBAUserFunctionXLSM()
{
    ScDocShellRef xDocSh = loadDoc(u"vba-user-function.", FORMAT_XLSM);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // A1 contains formula with user-defined function, and the function is defined in VBA.
    ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);

    sc::CompileFormulaContext aCxt(rDoc);
    OUString aFormula = pFC->GetFormula(aCxt);

    CPPUNIT_ASSERT_EQUAL(OUString("=MYFUNC()"), aFormula);

    // Check the formula state after the load.
    FormulaError nErrCode = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(nErrCode));

    // Check the result.
    CPPUNIT_ASSERT_EQUAL(42.0, rDoc.GetValue(ScAddress(0, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testEmbeddedImageXLS()
{
    // The document has one embedded image on the first sheet.  Make sure it's
    // imported properly.

    ScDocShellRef xDocSh = loadDoc(u"file-with-png-image.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    ScDrawLayer* pDL = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDL);
    const SdrPage* pPage = pDL->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    const SdrGrafObj* pImageObj = dynamic_cast<const SdrGrafObj*>(pObj);
    CPPUNIT_ASSERT(pImageObj);
    const Graphic& rGrf = pImageObj->GetGraphic();
    BitmapEx aBMP = rGrf.GetBitmapEx();
    CPPUNIT_ASSERT_MESSAGE(
        "Bitmap content should not be empty if the image has been properly imported.",
        !aBMP.IsEmpty());

    xDocSh->DoClose();
}

void ScFiltersTest2::testErrorOnExternalReferences()
{
    ScDocShellRef xDocSh = loadDoc(u"blank.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open empty doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Test tdf#89330
    rDoc.SetString(ScAddress(0, 0, 0), "='file:///Path/To/FileA.ods'#$Sheet1.A1A");

    ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(int(FormulaError::NoName), static_cast<int>(pFC->GetErrCode()));

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0, 0, 0), "'file:///Path/To/FileA.ods'#$Sheet1.A1A",
                         "Formula changed");

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf145054()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf145054.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Copy sheet
    rDoc.CopyTab(0, 1);
    CPPUNIT_ASSERT_EQUAL(SCTAB(2), rDoc.GetTableCount());

    // Make sure named DB was copied
    ScDBData* pDBData = rDoc.GetDBCollection()->getNamedDBs().findByName("__Anonymous_Sheet_DB__1");
    CPPUNIT_ASSERT(pDBData);
}

void ScFiltersTest2::testTdf84762()
{
    ScDocShellRef xDocSh = loadDoc(u"blank.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open empty doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    rDoc.SetString(ScAddress(0, 0, 0), "=RAND()");
    rDoc.SetString(ScAddress(0, 1, 0), "=RAND()");
    rDoc.SetString(ScAddress(1, 0, 0), "=RAND()*A1");
    rDoc.SetString(ScAddress(1, 1, 0), "=RAND()*B1");

    double nValA1, nValB1, nValA2, nValB2;

    // Without the fix in place, some cells wouldn't have been updated
    // after using F9 a few times
    for (sal_Int16 i = 0; i < 10; ++i)
    {
        nValA1 = rDoc.GetValue(0, 0, 0);
        nValB1 = rDoc.GetValue(0, 1, 0);
        nValA2 = rDoc.GetValue(1, 0, 0);
        nValB2 = rDoc.GetValue(1, 1, 0);

        xDocSh->DoRecalc(false);

        CPPUNIT_ASSERT(nValA1 != rDoc.GetValue(0, 0, 0));
        CPPUNIT_ASSERT(nValA2 != rDoc.GetValue(0, 1, 0));
        CPPUNIT_ASSERT(nValB1 != rDoc.GetValue(1, 0, 0));
        CPPUNIT_ASSERT(nValB2 != rDoc.GetValue(1, 1, 0));
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf44076()
{
    ScDocShellRef xDocSh = loadDoc(u"blank.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open empty doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    rDoc.SetString(ScAddress(0, 0, 0), "=(-8)^(1/3)");

    CPPUNIT_ASSERT_EQUAL(-2.0, rDoc.GetValue(ScAddress(0, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testEditEngStrikeThroughXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"strike-through.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    const EditTextObject* pObj = rDoc.GetEditText(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pObj->GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(OUString("this is strike through  this not"), pObj->GetText(0));

    std::vector<EECharAttrib> aAttribs;
    pObj->GetCharAttribs(0, aAttribs);
    for (const auto& rAttrib : aAttribs)
    {
        if (rAttrib.pAttr->Which() == EE_CHAR_STRIKEOUT)
        {
            const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>(*rAttrib.pAttr);
            if (rAttrib.nStart == 0)
            {
                CPPUNIT_ASSERT(rItem.GetStrikeout() != STRIKEOUT_NONE);
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(STRIKEOUT_NONE, rItem.GetStrikeout());
            }
        }
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testRefStringXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"ref_string.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    double nVal = rDoc.GetValue(2, 2, 0);
    ASSERT_DOUBLES_EQUAL(3.0, nVal);

    const ScCalcConfig& rCalcConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, rCalcConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf130132()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf130132.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScPatternAttr* pAttr = rDoc.GetPattern(434, 0, 0);

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        const Color& rColor = rBackground.GetColor();
        // background colour is yellow
        CPPUNIT_ASSERT_EQUAL(Color(255, 255, 0), rColor);
    }
}

void ScFiltersTest2::testTdf133327()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf133327.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    const ScPatternAttr* pAttr = rDoc.GetPattern(250, 1, 0);

    const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
    const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
    const Color& rColor = rBackground.GetColor();

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:255 G:255 B: 0
    // - Actual  : Color: R:255 G:255 B: 255
    CPPUNIT_ASSERT_EQUAL(Color(255, 255, 0), rColor);
}

void ScFiltersTest2::testColumnStyle2XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"column_style.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScPatternAttr* pAttr = rDoc.GetPattern(1, 1, 0);

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        const Color& rColor = rBackground.GetColor();
        CPPUNIT_ASSERT_EQUAL(Color(255, 51, 51), rColor);
    }

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        const SvxHorJustifyItem& rJustify = static_cast<const SvxHorJustifyItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Center, rJustify.GetValue());
    }

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_FONT_HEIGHT);
        const SvxFontHeightItem& rFontHeight = static_cast<const SvxFontHeightItem&>(rItem);
        sal_uInt16 nHeight = rFontHeight.GetHeight();
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(240), nHeight);
    }

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_FONT);
        const SvxFontItem& rFont = static_cast<const SvxFontItem&>(rItem);
        OUString aName = rFont.GetFamilyName();
        CPPUNIT_ASSERT_EQUAL(OUString("Linux Biolinum G"), aName);
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf110440XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf110440.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    uno::Reference<frame::XModel> xModel = xDocSh->GetModel();
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 uno::UNO_QUERY_THROW);
    xIA.set(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    bool bVisible = true;
    xShape->getPropertyValue("Visible") >>= bVisible;
    // This failed: group shape's hidden property was lost on import.
    CPPUNIT_ASSERT(!bVisible);

    xDocSh->DoClose();
}

void ScFiltersTest2::testBnc762542()
{
    ScDocShellRef xDocSh = loadDoc(u"bnc762542.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 10 shapes.", static_cast<size_t>(10), nCount);

    // previously, some of the shapes were (incorrectly) rotated by 90 degrees
    for (size_t i : { 1, 2, 4, 5, 7, 9 })
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

        tools::Rectangle aRect(pObj->GetCurrentBoundRect());
        CPPUNIT_ASSERT_MESSAGE("Drawing object shouldn't be rotated.",
                               aRect.GetWidth() > aRect.GetHeight());
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testHiddenSheetsXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"hidden_sheets.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1st sheet should be hidden", false, rDoc.IsVisible(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet should be visible", true, rDoc.IsVisible(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("3rd sheet should be hidden", false, rDoc.IsVisible(2));

    xDocSh->DoClose();
}

void ScFiltersTest2::testAutofilterXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"autofilter.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScDBData* pData = rDoc.GetDBCollection()->GetDBNearCursor(0, 0, 0);
    CPPUNIT_ASSERT(pData);
    ScRange aRange;
    pData->GetArea(aRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 4, 0), aRange);

    xDocSh->DoClose();
}

namespace
{
void checkValidationFormula(const ScAddress& rPos, const ScDocument& rDoc,
                            const OUString& rExpectedFormula)
{
    const SfxUInt32Item* pItem = rDoc.GetAttr(rPos, ATTR_VALIDDATA);
    CPPUNIT_ASSERT(pItem);
    sal_uLong nKey = pItem->GetValue();
    const ScValidationData* pData = rDoc.GetValidationEntry(nKey);
    CPPUNIT_ASSERT(pData);

    OUString aFormula = pData->GetExpression(rPos, 0);
    CPPUNIT_ASSERT_EQUAL(rExpectedFormula, aFormula);
}
}

void ScFiltersTest2::testRelFormulaValidationXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"validation.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    checkValidationFormula(ScAddress(3, 4, 0), rDoc, "C5");
    checkValidationFormula(ScAddress(5, 8, 0), rDoc, "D7");

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf136364()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf136364.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, it would have failed with
    // - Expected: =SUM((B2:B3~C4:C5~D6:D7))
    // - Actual  : =SUM((B2:B3~C4:C5,D6:D7))
    OUString aFormula = rDoc.GetFormula(4, 0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM((B2:B3~C4:C5~D6:D7))"), aFormula);
    CPPUNIT_ASSERT_EQUAL(27.0, rDoc.GetValue(ScAddress(4, 0, 0)));

    // - Expected: =SUM((B2~C4~D6))
    // - Actual  : =SUM((B2~C4,D6))
    aFormula = rDoc.GetFormula(4, 1, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM((B2~C4~D6))"), aFormula);
    CPPUNIT_ASSERT_EQUAL(12.0, rDoc.GetValue(ScAddress(4, 1, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf103734()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf103734.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, MAX() would have returned -1.8E+308
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), rDoc.GetString(ScAddress(2, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf126116()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf126116.", FORMAT_FODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("02/02/21"), rDoc.GetString(ScAddress(0, 0, 0)));

    rDoc.SetString(ScAddress(0, 0, 0), "03/03");

    sal_uInt32 nNumberFormat = rDoc.GetNumberFormat(0, 0, 0);
    const SvNumberformat* pNumberFormat = rDoc.GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();

    // Without the fix in place, this test would have failed with
    // - Expected: MM/DD/YY
    // - Actual  : MM/DD/YYYY
    CPPUNIT_ASSERT_EQUAL(OUString("MM/DD/YY"), rFormatStr);

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf144209()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf144209.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("AA 0"), rDoc.GetString(ScAddress(0, 0, 0)));

    xDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: AA 33263342642.5385
    // - Actual  : AA 0
    CPPUNIT_ASSERT_EQUAL(OUString("AA 33263342642.5385"), rDoc.GetString(ScAddress(0, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf98844()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf98844.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(47.6227, rDoc.GetValue(ScAddress(0, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(48.0, rDoc.GetValue(ScAddress(0, 8, 0)));

    xDocSh->DoHardRecalc();

    // Without the fix in place, SUM() wouldn't have been updated when
    // Precision as shown is enabled
    CPPUNIT_ASSERT_EQUAL(48.0, rDoc.GetValue(ScAddress(0, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(48.0, rDoc.GetValue(ScAddress(0, 8, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf100458()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf100458_lost_zero_value.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(rDoc.HasValueData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(0, 0, 0));
    CPPUNIT_ASSERT(!rDoc.HasStringData(0, 0, 0));
    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf118561()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf118561.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    //Without the fix in place, it would have failed with
    //- Expected: apple
    //- Actual  : Err:502
    CPPUNIT_ASSERT_EQUAL(OUString("apple"), rDoc.GetString(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("apple"), rDoc.GetString(ScAddress(2, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), rDoc.GetString(ScAddress(3, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("fruits"), rDoc.GetString(ScAddress(4, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("apple"), rDoc.GetString(ScAddress(5, 1, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("hat"), rDoc.GetString(ScAddress(6, 1, 1)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf125099()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf125099.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("03:53:46"), rDoc.GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("03:23:59"), rDoc.GetString(ScAddress(0, 1, 0)));

    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_EQUAL(OUString("03:53:46"), rDoc.GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this would have failed with
    // - Expected: 03:24:00
    // - Actual  : 03:23:59
    CPPUNIT_ASSERT_EQUAL(OUString("03:24:00"), rDoc.GetString(ScAddress(0, 1, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf134455()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf134455.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("00:05"), rDoc.GetString(ScAddress(3, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("00:10"), rDoc.GetString(ScAddress(3, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("00:59"), rDoc.GetString(ScAddress(3, 6, 0)));

    // Without the fix in place, TIMEVALUE would have returned Err:502 for values
    // greater than 59
    CPPUNIT_ASSERT_EQUAL(OUString("01:05"), rDoc.GetString(ScAddress(3, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("04:00"), rDoc.GetString(ScAddress(3, 8, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf119533()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf119533.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without fix in place, this test would have failed with
    // - Expected: 0.5
    // - Actual  : 0.483333333333333
    CPPUNIT_ASSERT_EQUAL(OUString("0.5"), rDoc.GetString(ScAddress(4, 0, 0)));

    // Without fix in place, this test would have failed with
    // - Expected: 9.5
    // - Actual  : 9.51666666666667
    CPPUNIT_ASSERT_EQUAL(OUString("9.5"), rDoc.GetString(ScAddress(5, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf127982()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf127982.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, these cells would be empty
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), rDoc.GetString(ScAddress(3, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R6"), rDoc.GetString(ScAddress(3, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R7"), rDoc.GetString(ScAddress(3, 7, 0)));

    CPPUNIT_ASSERT_EQUAL(OUString("R1"), rDoc.GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R6"), rDoc.GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R7"), rDoc.GetString(ScAddress(4, 7, 0)));

    // Without the fix in place, these cells would be empty
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), rDoc.GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R6"), rDoc.GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("R7"), rDoc.GetString(ScAddress(4, 7, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf109409()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf109409.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // TEXTJOIN
    CPPUNIT_ASSERT_EQUAL(OUString("A1;B1;A2;B2;A3;B3"), rDoc.GetString(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1;B1;A2;B2;A3;B3"), rDoc.GetString(ScAddress(3, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1;A2;A3;B1;B2;B3"), rDoc.GetString(ScAddress(3, 4, 0)));

    // Without the fix in place, it would have failed with
    //- Expected: A1;B1;A2;B2;A3;B3
    //- Actual  : A1;A2;A3;B1;B2;B3
    CPPUNIT_ASSERT_EQUAL(OUString("A1;B1;A2;B2;A3;B3"), rDoc.GetString(ScAddress(3, 5, 0)));

    // CONCAT
    CPPUNIT_ASSERT_EQUAL(OUString("A1B1A2B2A3B3"), rDoc.GetString(ScAddress(6, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1B1A2B2A3B3"), rDoc.GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A1A2A3B1B2B3"), rDoc.GetString(ScAddress(6, 4, 0)));

    // Without the fix in place, it would have failed with
    //- Expected: A1B1A2B2A3B3
    //- Actual  : A1A2A3B1B2B3
    CPPUNIT_ASSERT_EQUAL(OUString("A1B1A2B2A3B3"), rDoc.GetString(ScAddress(6, 5, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf132105()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf132105.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // MATCH
    CPPUNIT_ASSERT_EQUAL(OUString("5"), rDoc.GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), rDoc.GetString(ScAddress(1, 1, 0)));

    // COUNT
    CPPUNIT_ASSERT_EQUAL(OUString("0"), rDoc.GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("20"), rDoc.GetString(ScAddress(1, 2, 0)));

    // COUNTA
    CPPUNIT_ASSERT_EQUAL(OUString("20"), rDoc.GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("20"), rDoc.GetString(ScAddress(1, 3, 0)));

    // COUNTBLANK
    // Without the fix in place, it would have failed with
    // - Expected: 0
    //- Actual  : Err:504
    CPPUNIT_ASSERT_EQUAL(OUString("0"), rDoc.GetString(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0"), rDoc.GetString(ScAddress(1, 4, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf131424()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf131424.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, table reference would have failed
    CPPUNIT_ASSERT_EQUAL(35.0, rDoc.GetValue(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(58.0, rDoc.GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(81.0, rDoc.GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(104.0, rDoc.GetValue(ScAddress(2, 4, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf100709XLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"tdf100709.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf100709.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B52 should not be formatted with a $", OUString("218"),
                                 rDoc.GetString(1, 51, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A75 should not be formatted as a date", OUString("218"),
                                 rDoc.GetString(0, 74, 0));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf97598XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf97598_scenarios.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf97598_scenarios.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aStr = rDoc.GetString(0, 0, 0); // A1
    CPPUNIT_ASSERT_EQUAL(OUString("Cell A1"), aStr);

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf83672XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf83672.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf83672.xlsx", xDocSh.is());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocSh->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    sal_Int32 nRotate = 0;
    xShapeProperties->getPropertyValue("RotateAngle") >>= nRotate;
    CPPUNIT_ASSERT(nRotate != 0);
    xDocSh->DoClose();
}

void ScFiltersTest2::testUnicodeFileNameGnumeric()
{
    // Mapping the LO-internal URL
    // <file:///.../sc/qa/unit/data/gnumeric/t%C3%A4%C3%9Ft.gnumeric> to the
    // repo's file sc/qa/unit/data/gnumeric/t\303\244\303\237t.gnumeric only
    // works when the system encoding is UTF-8:
    if (osl_getThreadTextEncoding() != RTL_TEXTENCODING_UTF8)
    {
        return;
    }
    ScDocShellRef xDocSh = loadDoc(u"t\u00E4\u00DFt.", FORMAT_GNUMERIC);
    CPPUNIT_ASSERT(xDocSh.is());

    xDocSh->DoClose();
}

void ScFiltersTest2::testMergedCellsXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"merged-cells.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load merged-cells.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // B1:C1 is merged.
    ScRange aMergedRange(1, 0, 0); // B1
    rDoc.ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 0, 0, 2, 0, 0), aMergedRange);

    // D1:F1 is merged.
    aMergedRange = ScRange(3, 0, 0); // D1
    rDoc.ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(3, 0, 0, 5, 0, 0), aMergedRange);

    // A2:A3 is merged.
    aMergedRange = ScRange(0, 1, 0); // A2
    rDoc.ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 0, 2, 0), aMergedRange);

    // A4:A6 is merged.
    aMergedRange = ScRange(0, 3, 0); // A4
    rDoc.ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 3, 0, 0, 5, 0), aMergedRange);

    // C3:F6 is merged.
    aMergedRange = ScRange(2, 2, 0); // C3
    rDoc.ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(2, 2, 0, 5, 5, 0), aMergedRange);

    xDocSh->DoClose();
}

void ScFiltersTest2::testBackgroundColorStandardXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"background-color-standard.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load background-color-standard.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    struct Check
    {
        OUString aCellValue;
        Color aFontColor;
        Color aBgColor;
    };

    const std::vector<Check> aChecks = {
        { OUString("Background Color"), COL_BLACK, COL_TRANSPARENT },
        { OUString("Dark Red"), COL_WHITE, Color(192, 0, 0) },
        { OUString("Red"), COL_WHITE, Color(255, 0, 0) },
        { OUString("Orange"), COL_WHITE, Color(255, 192, 0) },
        { OUString("Yellow"), COL_WHITE, Color(255, 255, 0) },
        { OUString("Light Green"), COL_WHITE, Color(146, 208, 80) },
        { OUString("Green"), COL_WHITE, Color(0, 176, 80) },
        { OUString("Light Blue"), COL_WHITE, Color(0, 176, 240) },
        { OUString("Blue"), COL_WHITE, Color(0, 112, 192) },
        { OUString("Dark Blue"), COL_WHITE, Color(0, 32, 96) },
        { OUString("Purple"), COL_WHITE, Color(112, 48, 160) },
    };

    for (size_t nRow = 0; nRow < aChecks.size(); ++nRow)
    {
        ScAddress aPos(0, nRow, 0);
        OUString aStr = rDoc.GetString(aPos);
        CPPUNIT_ASSERT_EQUAL(aChecks[nRow].aCellValue, aStr);

        const ScPatternAttr* pPat = rDoc.GetPattern(aPos);
        CPPUNIT_ASSERT(pPat);

        const SvxColorItem& rColor = pPat->GetItem(ATTR_FONT_COLOR);
        CPPUNIT_ASSERT_EQUAL(aChecks[nRow].aFontColor, rColor.GetValue());

        const SvxBrushItem& rBgColor = pPat->GetItem(ATTR_BACKGROUND);
        CPPUNIT_ASSERT_EQUAL(aChecks[nRow].aBgColor, rBgColor.GetColor());
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf131536()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf131536.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aPos(3, 9, 0);
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(aPos));
    ASSERT_FORMULA_EQUAL(
        rDoc, aPos,
        "IF(D$4=\"-\",\"-\",MID(TEXT(INDEX($Comparison.$I:$J,$Comparison.$A5,$Comparison.D$2),\"\")"
        ",2,4)"
        "=RIGHT(TEXT(INDEX($Comparison.$L:$Z,$Comparison.$A5,$Comparison.D$4),\"\"),4))",
        nullptr);

    ScAddress aPos2(4, 9, 0);
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(aPos2));
    ASSERT_FORMULA_EQUAL(
        rDoc, aPos2,
        "IF(D$4=\"-\",\"-\",MID(TEXT(INDEX($Comparison.$I:$J,$Comparison.$A5,$Comparison.D$2),"
        "\"0\"),2,4)"
        "=RIGHT(TEXT(INDEX($Comparison.$L:$Z,$Comparison.$A5,$Comparison.D$4),\"0\"),4))",
        nullptr);
}

void ScFiltersTest2::testTdf130583()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf130583.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("b"), rDoc.GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("c"), rDoc.GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), rDoc.GetString(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("d"), rDoc.GetString(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), rDoc.GetString(ScAddress(1, 4, 0)));

    // Without the fix in place, SWITCH would have returned #VALUE!
    CPPUNIT_ASSERT_EQUAL(OUString("b"), rDoc.GetString(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("c"), rDoc.GetString(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), rDoc.GetString(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("d"), rDoc.GetString(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), rDoc.GetString(ScAddress(4, 4, 0)));
}

void ScFiltersTest2::testTdf85617()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf85617.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aPos(2, 2, 0);
    //Without the fix in place, it would be Err:509
    CPPUNIT_ASSERT_EQUAL(4.5, rDoc.GetValue(aPos));
}

void ScFiltersTest2::testTdf134234()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf134234.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(ScAddress(1, 0, 1)));

    //Without the fix in place, SUMPRODUCT would have returned 0
    CPPUNIT_ASSERT_EQUAL(36.54, rDoc.GetValue(ScAddress(2, 0, 1)));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(833),
                         static_cast<sal_uInt32>(rDoc.GetValue(ScAddress(3, 0, 1))));
}

void ScFiltersTest2::testTdf42481()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf42481.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(3, 9, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: #VALUE!
    // - Actual  : 14
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(3, 10, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("14"), rDoc.GetString(ScAddress(3, 11, 0)));
}

void ScFiltersTest2::testNamedExpressionsXLSXML()
{
    {
        // global named expressions

        ScDocShellRef xDocSh = loadDoc(u"named-exp-global.", FORMAT_XLS_XML);
        CPPUNIT_ASSERT_MESSAGE("Failed to load named-exp-global.xml", xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();

        // A7
        ScAddress aPos(0, 6, 0);
        CPPUNIT_ASSERT_EQUAL(15.0, rDoc.GetValue(aPos));
        ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(MyRange)", nullptr);

        // B7
        aPos.IncCol();
        CPPUNIT_ASSERT_EQUAL(55.0, rDoc.GetValue(aPos));
        ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(MyRange2)", nullptr);

        const ScRangeData* pRD = rDoc.GetRangeName()->findByUpperName("MYRANGE");
        CPPUNIT_ASSERT(pRD);
        pRD = rDoc.GetRangeName()->findByUpperName("MYRANGE2");
        CPPUNIT_ASSERT(pRD);

        xDocSh->DoClose();
    }

    {
        // sheet-local named expressions

        ScDocShellRef xDocSh = loadDoc(u"named-exp-local.", FORMAT_XLS_XML);
        CPPUNIT_ASSERT_MESSAGE("Failed to load named-exp-local.xml", xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();

        // A7 on Sheet1
        ScAddress aPos(0, 6, 0);
        CPPUNIT_ASSERT_EQUAL(27.0, rDoc.GetValue(aPos));
        ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(MyRange)", nullptr);

        // A7 on Sheet2
        aPos.IncTab();
        CPPUNIT_ASSERT_EQUAL(74.0, rDoc.GetValue(aPos));
        ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(MyRange)", nullptr);

        const ScRangeName* pRN = rDoc.GetRangeName(0);
        CPPUNIT_ASSERT(pRN);
        const ScRangeData* pRD = pRN->findByUpperName("MYRANGE");
        CPPUNIT_ASSERT(pRD);
        pRN = rDoc.GetRangeName(1);
        CPPUNIT_ASSERT(pRN);
        pRD = pRN->findByUpperName("MYRANGE");
        CPPUNIT_ASSERT(pRD);

        xDocSh->DoClose();
    }
}

void ScFiltersTest2::testEmptyRowsXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"empty-rows.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load empty-rows.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "Top row, followed by 2 empty rows.", nullptr },
            { nullptr, nullptr },
            { nullptr, nullptr },
            { nullptr, "1" },
            { nullptr, "2" },
            { nullptr, "3" },
            { nullptr, "4" },
            { nullptr, "5" },
            { nullptr, "15" },
        };

        ScRange aDataRange;
        aDataRange.Parse("A1:B9", rDoc);
        bool bSuccess = checkOutput(&rDoc, aDataRange, aOutputCheck, "Expected output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScAddress aPos;
    aPos.Parse("B9", rDoc);
    ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(B4:B8)", nullptr);

    xDocSh->DoClose();
}

void ScFiltersTest2::testBorderDirectionsXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"border-directions.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load border-directions.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    struct Check
    {
        ScAddress aPos;
        bool bTop;
        bool bBottom;
        bool bLeft;
        bool bRight;
        bool bTLtoBR;
        bool bTRtoBL;
    };

    std::vector<Check> aChecks = {
        { { 1, 1, 0 }, true, false, false, false, false, false }, // B2 - top
        { { 1, 3, 0 }, false, false, true, false, false, false }, // B4 - left
        { { 1, 5, 0 }, false, false, false, true, false, false }, // B6 - right
        { { 1, 7, 0 }, false, true, false, false, false, false }, // B8 - bottom
        { { 1, 9, 0 }, false, false, false, false, true, false }, // B10 - tl to br
        { { 1, 11, 0 }, false, false, false, false, false, true }, // B12 - tr to bl
        { { 1, 13, 0 }, false, false, false, false, true, true }, // B14 - cross-diagonal
    };

    auto funcCheckBorder = [](bool bHasBorder, const editeng::SvxBorderLine* pLine) -> bool {
        if (bHasBorder)
        {
            if (!pLine)
            {
                std::cout << "Border was expected, but not found!" << std::endl;
                return false;
            }

            if (SvxBorderLineStyle::SOLID != pLine->GetBorderLineStyle())
            {
                std::cout << "Border type was expected to be of SOLID, but is not." << std::endl;
                return false;
            }

            if (COL_BLACK != pLine->GetColor())
            {
                std::cout << "Border color was expected to be black, but is not." << std::endl;
                return false;
            }
        }
        else
        {
            if (pLine)
            {
                std::cout << "Border was not expected, but is found!" << std::endl;
                return false;
            }
        }

        return true;
    };

    for (const Check& c : aChecks)
    {
        const ScPatternAttr* pPat = rDoc.GetPattern(c.aPos);
        CPPUNIT_ASSERT(pPat);

        const SvxBoxItem& rBox = pPat->GetItem(ATTR_BORDER);

        const editeng::SvxBorderLine* pLine = rBox.GetTop();
        CPPUNIT_ASSERT(funcCheckBorder(c.bTop, pLine));

        pLine = rBox.GetBottom();
        CPPUNIT_ASSERT(funcCheckBorder(c.bBottom, pLine));

        pLine = rBox.GetLeft();
        CPPUNIT_ASSERT(funcCheckBorder(c.bLeft, pLine));

        pLine = rBox.GetRight();
        CPPUNIT_ASSERT(funcCheckBorder(c.bRight, pLine));

        pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
        CPPUNIT_ASSERT(funcCheckBorder(c.bTLtoBR, pLine));

        pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
        CPPUNIT_ASSERT(funcCheckBorder(c.bTRtoBL, pLine));
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testBorderColorsXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"border-colors.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load border-colors.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // B3 - red
    const ScPatternAttr* pPat = rDoc.GetPattern(ScAddress(1, 2, 0));
    CPPUNIT_ASSERT(pPat);
    const editeng::SvxBorderLine* pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(255, 0, 0), pLine->GetColor());

    // B4 - blue
    pPat = rDoc.GetPattern(ScAddress(1, 3, 0));
    CPPUNIT_ASSERT(pPat);
    pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 112, 192), pLine->GetColor());

    // B5 - green
    pPat = rDoc.GetPattern(ScAddress(1, 4, 0));
    CPPUNIT_ASSERT(pPat);
    pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 176, 80), pLine->GetColor());

    // B7 - yellow (left), purple (right), light blue (cross)
    pPat = rDoc.GetPattern(ScAddress(1, 6, 0));
    CPPUNIT_ASSERT(pPat);

    pLine = pPat->GetItem(ATTR_BORDER).GetLeft();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(255, 255, 0), pLine->GetColor()); // yellow

    pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(112, 48, 160), pLine->GetColor()); // purple

    pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 176, 240), pLine->GetColor()); // light blue

    pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 176, 240), pLine->GetColor()); // light blue

    xDocSh->DoClose();
}

void ScFiltersTest2::testHiddenRowsColumnsXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"hidden-rows-columns.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load hidden-rows-columns.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    struct Check
    {
        SCCOLROW nPos1;
        SCCOLROW nPos2;
        bool bVisible;
    };

    std::vector<Check> aRowChecks = {
        { 0, 0, true }, { 1, 2, false }, { 3, 3, true },      { 4, 4, false },
        { 5, 7, true }, { 8, 8, false }, { 9, MAXROW, true },
    };

    for (const Check& c : aRowChecks)
    {
        SCROW nRow1 = -1, nRow2 = -1;
        bool bVisible = !rDoc.RowHidden(c.nPos1, 0, &nRow1, &nRow2);
        CPPUNIT_ASSERT_EQUAL(bVisible, c.bVisible);
        CPPUNIT_ASSERT_EQUAL(c.nPos1, nRow1);
        CPPUNIT_ASSERT_EQUAL(c.nPos2, nRow2);
    }

    std::vector<Check> aColChecks = {
        { 0, 1, true }, { 2, 5, false }, { 6, 9, true }, { 10, 10, false }, { 11, MAXCOL, true },
    };

    for (const Check& c : aColChecks)
    {
        SCCOL nCol1 = -1, nCol2 = -1;
        bool bVisible = !rDoc.ColHidden(c.nPos1, 1, &nCol1, &nCol2);
        CPPUNIT_ASSERT_EQUAL(bVisible, c.bVisible);
        CPPUNIT_ASSERT_EQUAL(c.nPos1, SCCOLROW(nCol1));
        CPPUNIT_ASSERT_EQUAL(c.nPos2, SCCOLROW(nCol2));
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testColumnWidthRowHeightXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"column-width-row-height.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load column-width-row-height.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    struct RowHeight
    {
        SCROW nRow1;
        SCROW nRow2;
        sal_uInt16 nHeight; // in points (1 point == 20 twips)
    };

    std::vector<RowHeight> aRowChecks = {
        { 2, 2, 20 }, { 3, 3, 30 }, { 4, 4, 40 }, { 5, 5, 50 }, { 7, 9, 25 }, { 12, 13, 35 },
    };

    for (const RowHeight& rh : aRowChecks)
    {
        for (SCROW i = rh.nRow1; i <= rh.nRow2; ++i)
        {
            sal_uInt16 nHeight = rDoc.GetRowHeight(i, 0);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(rh.nHeight * 20), nHeight);
        }
    }

    struct ColWidth
    {
        SCCOL nCol1;
        SCCOL nCol2;
        sal_uInt16 nWidth; // in points (1 point == 20 twips
    };

    std::vector<ColWidth> aColChecks = {
        { 1, 1, 56 }, { 2, 2, 83 }, { 3, 3, 109 }, { 5, 7, 67 }, { 10, 11, 119 },
    };

    for (const ColWidth& cw : aColChecks)
    {
        for (SCCOL i = cw.nCol1; i <= cw.nCol2; ++i)
        {
            sal_uInt16 nWidth = rDoc.GetColWidth(i, 0);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(cw.nWidth * 20), nWidth);
        }
    }

    xDocSh->DoClose();
}

void ScFiltersTest2::testCharacterSetXLSXML()
{
    ScDocShellRef xDocSh = loadDoc(u"character-set.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load column-width-row-height.xml", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(SCTAB(1), rDoc.GetTableCount());

    OUString aName;
    rDoc.GetName(0, aName);

    // Check the sheet name.  The values are all Cyrillic letters.
    std::vector<sal_Unicode> aBuf = { 0x041b, 0x0438, 0x0441, 0x0442, 0x0031 };
    OUString aExpected(aBuf.data(), aBuf.size());
    CPPUNIT_ASSERT_EQUAL(aExpected, aName);

    // Check the value of I4
    OUString aVal = rDoc.GetString(ScAddress(8, 3, 0));
    aBuf = { 0x0421, 0x0443, 0x043c, 0x043c, 0x0430 };
    aExpected = OUString(aBuf.data(), aBuf.size());
    CPPUNIT_ASSERT_EQUAL(aExpected, aVal);

    // Check the value of J3
    aVal = rDoc.GetString(ScAddress(9, 2, 0));
    aBuf = { 0x041e, 0x0441, 0x0442, 0x0430, 0x0442, 0x043e, 0x043a };
    aExpected = OUString(aBuf.data(), aBuf.size());
    CPPUNIT_ASSERT_EQUAL(aExpected, aVal);

    xDocSh->DoClose();
}

void ScFiltersTest2::testCondFormatXLSB()
{
    ScDocShellRef xDocSh = loadDoc(u"cond_format.", FORMAT_XLSB);

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());
    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());

    xDocSh->DoClose();
}

void ScFiltersTest2::testPageScalingXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"page_scale.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aStyleName = rDoc.GetPageStyle(0);
    ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find(aStyleName, SfxStyleFamily::Page);
    CPPUNIT_ASSERT(pStyleSheet);

    SfxItemSet& rSet = pStyleSheet->GetItemSet();
    sal_uInt16 nVal = rSet.Get(ATTR_PAGE_SCALE).GetValue();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(90), nVal);

    xDocSh->DoClose();
}

void ScFiltersTest2::testActiveXCheckboxXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"activex_checkbox.", FORMAT_XLSX);
    uno::Reference<frame::XModel> xModel = xDocSh->GetModel();
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));

    // Check custom label
    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), sLabel);

    // Check background color (highlight system color)
    Color nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x316AC5), nColor);

    // Check Text color (active border system color)
    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xD4D0C8), nColor);

    // Check state of the checkbox
    sal_Int16 nState;
    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xDocSh->DoClose();
}

void ScFiltersTest2::testtdf120301_xmlSpaceParsingXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf120301_xmlSpaceParsing.", FORMAT_XLSX);
    uno::Reference<frame::XModel> xModel = xDocSh->GetModel();
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);

    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> XPropSet(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    OUString sCaption;
    XPropSet->getPropertyValue("Label") >>= sCaption;
    CPPUNIT_ASSERT_EQUAL(OUString("Check Box 1"), sCaption);
    xDocSh->DoClose();
}

namespace
{
struct PaintListener : public SfxListener
{
    bool mbCalled = false;
    virtual void Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint) override
    {
        const ScPaintHint* pPaintHint = dynamic_cast<const ScPaintHint*>(&rHint);
        if (pPaintHint)
        {
            if (pPaintHint->GetStartCol() <= 0 && pPaintHint->GetEndCol() >= 0
                && pPaintHint->GetStartRow() <= 9 && pPaintHint->GetEndRow() >= 9)
            {
                mbCalled = true;
            }
        }
    }
};
}

void ScFiltersTest2::testCondFormatFormulaListenerXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"cond_format_formula_listener.", FORMAT_XLSX);
    PaintListener aListener;
    aListener.StartListening(*xDocSh);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());
    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);
    rDoc.SetDocVisible(true);
    rDoc.SetValue(0, 0, 0, 2.0);

    CPPUNIT_ASSERT(aListener.mbCalled);

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf137091()
{
    // Set the system locale to Turkish
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("tr-TR");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    ScDocShellRef xDocSh = loadDoc(u"tdf137091.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 28/4
    // - Actual  : Err:507
    CPPUNIT_ASSERT_EQUAL(OUString("28/4"), rDoc.GetString(ScAddress(2, 1, 0)));
    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf62268()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf62268.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    int nHeight;

    SCTAB nTab = 0;
    nHeight = rDoc.GetRowHeight(0, nTab, false);
    CPPUNIT_ASSERT_LESSEQUAL(3, abs(256 - nHeight));
    nHeight = rDoc.GetRowHeight(1, nTab, false);
    CPPUNIT_ASSERT_LESSEQUAL(19, abs(1905 - nHeight));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf137453()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf137453.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 3333333333/100
    // - Actual  : -961633963/100
    CPPUNIT_ASSERT_EQUAL(OUString("3333333333/100"), rDoc.GetString(ScAddress(0, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf112780()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf112780.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : #VALUE!
    CPPUNIT_ASSERT_EQUAL(OUString(""), rDoc.GetString(ScAddress(3, 5, 0)));

    OUString aFormula = rDoc.GetFormula(3, 5, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=G6+J6+M6"), aFormula);

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf72470()
{
    // Without the fix in place, this test would have hung

    ScDocShellRef xDocSh = loadDoc(u"tdf72470.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("name"), rDoc.GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(u"أسمي walid"), rDoc.GetString(ScAddress(0, 1, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf35636()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf35636.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, SUMIF would have returned 0.0
    // with empty cells in the criteria
    CPPUNIT_ASSERT_EQUAL(50.0, rDoc.GetValue(ScAddress(1, 4, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf98481()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf98481.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(ScAddress(3, 4, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf115022()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf115022.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(6.0, rDoc.GetValue(ScAddress(1, 8, 0)));

    xDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 21
    CPPUNIT_ASSERT_EQUAL(6.0, rDoc.GetValue(ScAddress(1, 8, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testVBAMacroFunctionODS()
{
    ScDocShellRef xDocSh = loadDoc(u"vba_macro_functions.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aFunction = rDoc.GetFormula(2, 0, 0);
    std::cout << aFunction << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, rDoc.GetValue(2, 0, 0), 1e-6);

    xDocSh->DoClose();
}

void ScFiltersTest2::testAutoheight2Rows()
{
    ScDocShellRef xDocSh = loadDoc(u"autoheight2rows.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    SCTAB nTab = 0;
    int nHeight1 = rDoc.GetRowHeight(0, nTab, false);
    int nHeight2 = rDoc.GetRowHeight(1, nTab, false);
    int nHeight4 = rDoc.GetRowHeight(3, nTab, false);
    int nHeight5 = rDoc.GetRowHeight(4, nTab, false);

    // We will do relative comparison, because calculated autoheight
    // can be different on different platforms
    CPPUNIT_ASSERT_MESSAGE("Row #1 and row #4 must have same height after load & auto-adjust",
                           abs(nHeight1 - nHeight4) < 10);
    CPPUNIT_ASSERT_MESSAGE("Row #2 and row #5 must have same height after load & auto-adjust",
                           abs(nHeight2 - nHeight5) < 10);

    xDocSh->DoClose();
}

void ScFiltersTest2::testXLSDefColWidth()
{
    // XLS has only 256 columns; but on import, we need to set default width to all above that limit
    ScDocShellRef xDocSh = loadDoc(u"chartx.", FORMAT_XLS); // just some XLS with narrow columns
    ScDocument& rDoc = xDocSh->GetDocument();

    int nWidth = rDoc.GetColWidth(MAXCOL, 0, false);
    // This was 1280
    CPPUNIT_ASSERT_EQUAL(1005, nWidth);

    xDocSh->DoClose();
}

void ScFiltersTest2::testPreviewMissingObjLink()
{
    ScDocShellRef xDocSh = loadDoc(u"keep-preview-missing-obj-link.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load keep-preview-missing-obj-link.ods.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Retrieve the ole object
    const SdrOle2Obj* pOleObj = getSingleOleObject(rDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve an ole object from the 2nd sheet.", pOleObj);

    const Graphic* pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE(
        "the ole object links to a missing file, but we should retain its preview", pGraphic);

    xDocSh->DoClose();
}

void ScFiltersTest2::testShapeRotationImport()
{
    if (!IsDefaultDPI())
        return;
    // tdf#83593 Incorrectly calculated bounding rectangles caused shapes to appear as if there
    // were extra or missing rotations. Hence, we check the sizes of these rectangles.
    ScDocShellRef xDocSh = loadDoc(u"testShapeRotationImport.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load testShapeRotationImport.xlsx", xDocSh.is());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocSh->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);

    // The expected values are in the map below. Note that some of the angles are outside of the set which contains
    // the value of the wrong angles. This is to check the border cases and one value on both sides.
    std::map<sal_Int32, std::map<std::string, sal_Int32>> aExpectedValues{
        { 4400, { { "x", 6832 }, { "y", 36894 }, { "width", 8898 }, { "height", 1163 } } },
        { 4500, { { "x", 4491 }, { "y", 36400 }, { "width", 8898 }, { "height", 1164 } } },
        { 4600, { { "x", 1674 }, { "y", 36271 }, { "width", 8863 }, { "height", 1143 } } },
        { 13400, { { "x", 13762 }, { "y", 28403 }, { "width", 8863 }, { "height", 1194 } } },
        { 13500, { { "x", 10818 }, { "y", 27951 }, { "width", 8863 }, { "height", 1170 } } },
        { 13600, { { "x", 8450 }, { "y", 28337 }, { "width", 8898 }, { "height", 1163 } } },
        { 22400, { { "x", 14949 }, { "y", 12978 }, { "width", 8898 }, { "height", 1163 } } },
        { 22500, { { "x", 11765 }, { "y", 12834 }, { "width", 8898 }, { "height", 1163 } } },
        { 22600, { { "x", 8253 }, { "y", 12920 }, { "width", 8862 }, { "height", 1170 } } },
        { 31400, { { "x", 8099 }, { "y", 1160 }, { "width", 9815 }, { "height", 1171 } } },
        { 31500, { { "x", 4428 }, { "y", 1274 }, { "width", 10238 }, { "height", 1171 } } },
        { 31600, { { "x", 1964 }, { "y", 1879 }, { "width", 10307 }, { "height", 1163 } } },
    };

    for (sal_Int32 ind = 0; ind < 12; ++ind)
    {
        uno::Reference<drawing::XShape> xShape(xPage->getByIndex(ind), uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Any nRotProp = xShapeProperties->getPropertyValue("RotateAngle");
        sal_Int32 nRot = nRotProp.get<sal_Int32>();
        const OString sNote = "RotateAngle = " + OString::number(nRot);

        awt::Point aPosition = xShape->getPosition();
        awt::Size aSize = xShape->getSize();

        CPPUNIT_ASSERT_MESSAGE(sNote.getStr(), aExpectedValues.find(nRot) != aExpectedValues.end());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["x"], aPosition.X);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["y"], aPosition.Y);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["width"], aSize.Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["height"], aSize.Height);
    }
}

void ScFiltersTest2::testShapeDisplacementOnRotationImport()
{
    // tdf#135918 shape is displaced on rotation if it is placed next to the sheets upper/left edges
    ScDocShellRef xDocSh = loadDoc(u"testShapeDisplacementOnRotationImport.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load testShapeDisplacementOnRotationImport.xlsx",
                           xDocSh.is());

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocSh->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);
    uno::Any aRectProp = xShapeProperties->getPropertyValue("FrameRect");
    awt::Rectangle aRectangle = aRectProp.get<awt::Rectangle>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRectangle.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRectangle.Y);
}

void ScFiltersTest2::testTextBoxBodyUpright()
{
    // tdf#106197 We should import the "upright" attribute of txBody.
    ScDocShellRef xDocSh = loadDoc(u"tdf106197_import_upright.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load testTextBoxBodyUpright.xlsx", xDocSh.is());

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocSh->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);

    // Check that we imported "Upright".
    bool isUpright = false;
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName("InteropGrabBag"))
    {
        uno::Sequence<beans::PropertyValue> aGrabBag;
        xShapeProperties->getPropertyValue("InteropGrabBag") >>= aGrabBag;
        for (auto& aProp : std::as_const(aGrabBag))
        {
            if (aProp.Name == "Upright")
            {
                aProp.Value >>= isUpright;
                break;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(true, isUpright);

    // Check the new textRotateAngle.
    sal_Int32 nAngle;
    uno::Any aGeom = xShapeProperties->getPropertyValue("CustomShapeGeometry");
    auto aGeomSeq = aGeom.get<Sequence<beans::PropertyValue>>();
    for (const auto& aProp : std::as_const(aGeomSeq))
    {
        if (aProp.Name == "TextPreRotateAngle")
        {
            aProp.Value >>= nAngle;
            break;
        }
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(90), nAngle);
}

void ScFiltersTest2::testTextBoxBodyRotateAngle()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf141644.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf141644.xlsx", xDocSh.is());

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocSh->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);

    // Check the new textRotateAngle.
    sal_Int32 nAngle;
    uno::Any aGeom = xShapeProperties->getPropertyValue("CustomShapeGeometry");
    auto aGeomSeq = aGeom.get<Sequence<beans::PropertyValue>>();
    for (const auto& aProp : std::as_const(aGeomSeq))
    {
        if (aProp.Name == "TextPreRotateAngle")
        {
            aProp.Value >>= nAngle;
            break;
        }
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-270), nAngle);
}

void ScFiltersTest2::testTextLengthDataValidityXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"textLengthDataValidity.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load textLengthDataValidity.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    const ScValidationData* pData = rDoc.GetValidationEntry(1);

    ScRefCellValue aCellA1; // A1 = 1234(numeric value)
    ScAddress aValBaseAddrA1(0, 0, 0);
    aCellA1.assign(rDoc, aValBaseAddrA1);
    bool bValidA1 = pData->IsDataValid(aCellA1, aValBaseAddrA1);

    ScRefCellValue aCellA2; // A2 = 1234(numeric value format as text)
    ScAddress aValBaseAddrA2(0, 1, 0);
    aCellA2.assign(rDoc, aValBaseAddrA2);
    bool bValidA2 = pData->IsDataValid(aCellA2, aValBaseAddrA2);

    ScRefCellValue aCellA3; // A3 = 1234.00(numeric value)
    ScAddress aValBaseAddrA3(0, 2, 0);
    aCellA3.assign(rDoc, aValBaseAddrA3);
    bool bValidA3 = pData->IsDataValid(aCellA3, aValBaseAddrA3);

    ScRefCellValue aCellA4; // A4 = 12.3(numeric value)
    ScAddress aValBaseAddrA4(0, 3, 0);
    aCellA4.assign(rDoc, aValBaseAddrA4);
    bool bValidA4 = pData->IsDataValid(aCellA4, aValBaseAddrA4);

    // True if text length = 4
    CPPUNIT_ASSERT_EQUAL(true, bValidA1);
    CPPUNIT_ASSERT_EQUAL(true, bValidA2);
    CPPUNIT_ASSERT_EQUAL(true, bValidA3);
    CPPUNIT_ASSERT_EQUAL(true, bValidA4);

    xDocSh->DoClose();
}

void ScFiltersTest2::testDeleteCircles()
{
    ScDocShellRef xDocSh = loadDoc(u"testDeleteCircles.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load testDeleteCircles.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    ScRefCellValue aCellA1; // A1 = "Hello"
    ScAddress aPosA1(0, 0, 0);
    aCellA1.assign(rDoc, aPosA1);

    // Mark invalid value
    bool bOverflow;
    bool bMarkInvalid = ScDetectiveFunc(rDoc, 0).MarkInvalid(bOverflow);
    CPPUNIT_ASSERT_EQUAL(true, bMarkInvalid);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    // The value of A1 change to Hello1.
    rDoc.SetString(0, 0, 0, "Hello1");

    // Check that the data is valid.(True if text length = 6)
    const ScValidationData* pData = rDoc.GetValidationEntry(1);
    bool bValidA1 = pData->IsDataValid(aCellA1, aPosA1);
    // if valid, delete circle.
    if (bValidA1)
        ScDetectiveFunc(rDoc, 0).DeleteCirclesAt(aPosA1.Col(), aPosA1.Row());

    // There should not be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    xDocSh->DoClose();
}

void ScFiltersTest2::testDrawCircleInMergeCells()
{
    ScDocShellRef xDocSh = loadDoc(u"testDrawCircleInMergeCells.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load testDrawCircleInMergeCells.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // A1:B2 is merged.
    ScRange aMergedRange(0, 0, 0);
    rDoc.ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 1, 0), aMergedRange);

    // Mark invalid value
    bool bOverflow;
    bool bMarkInvalid = ScDetectiveFunc(rDoc, 0).MarkInvalid(bOverflow);
    CPPUNIT_ASSERT_EQUAL(true, bMarkInvalid);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    SdrObject* pObj = pPage->GetObj(0);
    tools::Rectangle aRect(pObj->GetLogicRect());
    Point aStartCircle = aRect.TopLeft();
    Point aEndCircle = aRect.BottomRight();

    tools::Rectangle aCellRect = rDoc.GetMMRect(0, 0, 1, 1, 0);
    aCellRect.AdjustLeft(-250);
    aCellRect.AdjustRight(250);
    aCellRect.AdjustTop(-70);
    aCellRect.AdjustBottom(70);
    Point aStartCell = aCellRect.TopLeft();
    Point aEndCell = aCellRect.BottomRight();

    CPPUNIT_ASSERT_EQUAL(aStartCell.X(), aStartCircle.X());
    CPPUNIT_ASSERT_EQUAL(aEndCell.X(), aEndCircle.X());
    CPPUNIT_ASSERT_EQUAL(aStartCell.Y(), aStartCircle.Y());
    CPPUNIT_ASSERT_EQUAL(aEndCell.Y(), aEndCircle.Y());

    // Change the height of the first row. (556 ~ 1cm)
    rDoc.SetRowHeight(0, 0, 556);
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    pDrawLayer->RecalcPos(pObj, *pData, false, false);

    tools::Rectangle aRecalcRect(pObj->GetLogicRect());
    Point aStartRecalcCircle = aRecalcRect.TopLeft();
    Point aEndRecalcCircle = aRecalcRect.BottomRight();

    tools::Rectangle aRecalcCellRect = rDoc.GetMMRect(0, 0, 1, 1, 0);
    aRecalcCellRect.AdjustLeft(-250);
    aRecalcCellRect.AdjustRight(250);
    aRecalcCellRect.AdjustTop(-70);
    aRecalcCellRect.AdjustBottom(70);
    Point aStartRecalcCell = aRecalcCellRect.TopLeft();
    Point aEndRecalcCell1 = aRecalcCellRect.BottomRight();

    CPPUNIT_ASSERT_EQUAL(aStartRecalcCell.X(), aStartRecalcCircle.X());
    CPPUNIT_ASSERT_EQUAL(aEndRecalcCell1.X(), aEndRecalcCircle.X());
    CPPUNIT_ASSERT_EQUAL(aStartRecalcCell.Y(), aStartRecalcCircle.Y());
    CPPUNIT_ASSERT_EQUAL(aEndRecalcCell1.Y(), aEndRecalcCircle.Y());

    xDocSh->DoClose();
}

void ScFiltersTest2::testDeleteCirclesInRowAndCol()
{
    ScDocShellRef xDocSh = loadDoc(u"deleteCirclesInRowAndCol.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load deleteCirclesInRowAndCol.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // There should be 6 circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), pPage->GetObjCount());

    // Delete first row (1023 = MAXCOLS)
    pDrawLayer->DeleteObjectsInArea(0, 0, 0, 1023, 0, true);

    // There should be 3 circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pPage->GetObjCount());

    // Delete first col (1048575 = MAXROWS)
    pDrawLayer->DeleteObjectsInArea(0, 0, 0, 0, 1048575, true);

    // There should not be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf129940()
{
    // Test pure span elements inside text:ruby-base
    ScDocShellRef xDocSh = loadDoc(u"tdf129940.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf129940.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    // Pure text within text:ruby-base
    OUString aStr = rDoc.GetString(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(u"小笠原"), aStr);
    aStr = rDoc.GetString(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(u"徳彦"), aStr);

    // Multiple text:span within text:ruby-base
    aStr = rDoc.GetString(ScAddress(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(u"注音符號"), aStr);

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf139612()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf139612.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf139612.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    xDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(1, 15, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf144740()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf144740.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf144740.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(1, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest2::testTdf139763ShapeAnchor()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf139763ShapeAnchor.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.xlsx", xDocSh.is());

    // There are two objects on the first sheet, anchored to page by element xdr:absoluteAnchor
    // and anchored to cell by element xdr:oneCellAnchor. Error was, that they were imported as
    // "anchor to cell (resize with cell".
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc.GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    // There should be 2 shapes
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPage->GetObjCount());

    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get page anchored object.", pObj);
    CPPUNIT_ASSERT_MESSAGE("Shape must be page anchored", !ScDrawLayer::IsCellAnchored(*pObj));

    pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Failed to get cell anchored object.", pObj);
    CPPUNIT_ASSERT_MESSAGE("Shape must be anchored to cell.", ScDrawLayer::IsCellAnchored(*pObj));
    CPPUNIT_ASSERT_MESSAGE("Shape must not resize with cell.",
                           !ScDrawLayer::IsResizeWithCell(*pObj));

    xDocSh->DoClose();
}

void ScFiltersTest2::testAutofilterNamedRangesXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"autofilternamedrange.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load autofilternamedrange.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScDBData* pDBData = rDoc.GetDBCollection()->GetDBNearCursor(0, 0, 0);
    CPPUNIT_ASSERT(pDBData);
    ScRange aRange;
    pDBData->GetArea(aRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 3, 0), aRange);
    OUString aPosStr;
    bool bSheetLocal = false;
    // test there is no '_xlnm._FilterDatabase' named range on the filter area
    const ScRangeData* pRData = rDoc.GetRangeAtBlock(aRange, aPosStr, &bSheetLocal);
    CPPUNIT_ASSERT(!pRData);
    CPPUNIT_ASSERT_EQUAL(OUString(), aPosStr);

    xDocSh->DoClose();
}

void ScFiltersTest2::testInvalidBareBiff5()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf144732.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // Check that we import the contents from such file, as Excel does
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), rDoc.GetTableCount());

    // Row 1
    ScAddress aPos(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));

    // Row 2
    aPos = ScAddress(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, rDoc.GetCellType(aPos));
    OUString sFormula = rDoc.GetFormula(aPos.Col(), aPos.Row(), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(OUString("=TRUE()"), sFormula);
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, rDoc.GetCellType(aPos));
    sFormula = rDoc.GetFormula(aPos.Col(), aPos.Row(), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), sFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"sheetjs"), rDoc.GetString(aPos));

    // Row 3
    aPos = ScAddress(0, 2, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"foo    bar"), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"baz"), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(41689.4375, rDoc.GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"0.3"), rDoc.GetString(aPos));

    // Row 4
    aPos = ScAddress(0, 3, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"baz"), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"_"), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(3.14159, rDoc.GetValue(aPos));

    // Row 5
    aPos = ScAddress(0, 4, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"hidden"), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));

    // Row 6
    aPos = ScAddress(0, 5, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(u"visible"), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(aPos));

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
