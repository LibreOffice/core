/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "dpoutput.hxx"
#include "dptabsrc.hxx"
#include "dpcachetable.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "markdata.hxx"
#include "attrib.hxx"
#include "formula/errorcodes.hxx"       // errNoValue
#include "miscuno.hxx"
#include "globstr.hrc"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "scresid.hxx"
#include "unonames.hxx"
#include "sc.hrc"
#include "stringutil.hxx"
#include "dputil.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTableResultData.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>

#include <vector>

using namespace com::sun::star;
using ::std::vector;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::sheet::DataPilotTablePositionData;
using ::com::sun::star::sheet::DataPilotTableResultData;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

#define SC_DP_FRAME_INNER_BOLD      20
#define SC_DP_FRAME_OUTER_BOLD      40

#define SC_DP_FRAME_COLOR           Color(0,0,0) //( 0x20, 0x40, 0x68 )

#define SC_DPOUT_MAXLEVELS  256

struct ScDPOutLevelData
{
    long                                nDim;
    long                                nHier;
    long                                nLevel;
    long                                nDimPos;
    uno::Sequence<sheet::MemberResult>  aResult;
    rtl::OUString                       maName;   /// Name is the internal field name.
    rtl::OUString                       maCaption; /// Caption is the name visible in the output table.
    bool                                mbHasHiddenMember:1;
    bool                                mbDataLayout:1;

    ScDPOutLevelData() :
        nDim(-1), nHier(-1), nLevel(-1), nDimPos(-1), mbHasHiddenMember(false), mbDataLayout(false)
    {}

    bool operator<(const ScDPOutLevelData& r) const
        { return nDimPos<r.nDimPos || ( nDimPos==r.nDimPos && nHier<r.nHier ) ||
            ( nDimPos==r.nDimPos && nHier==r.nHier && nLevel<r.nLevel ); }

    void Swap(ScDPOutLevelData& r)
        { ScDPOutLevelData aTemp; aTemp = r; r = *this; *this = aTemp; }

    //! bug (73840) in uno::Sequence - copy and then assign doesn't work!
};

namespace {

bool lcl_compareColfuc ( SCCOL i,  SCCOL j) { return (i<j); }
bool lcl_compareRowfuc ( SCROW i,  SCROW j) { return (i<j); }

class ScDPOutputImpl
{
    ScDocument*         mpDoc;
    sal_uInt16          mnTab;
    ::std::vector< bool > mbNeedLineCols;
    ::std::vector< SCCOL > mnCols;

    ::std::vector< bool > mbNeedLineRows;
    ::std::vector< SCROW > mnRows;

    SCCOL   mnTabStartCol;
    SCROW   mnTabStartRow;
    SCCOL   mnMemberStartCol;
    SCROW   mnMemberStartRow;

    SCCOL   mnDataStartCol;
    SCROW   mnDataStartRow;
    SCCOL   mnTabEndCol;
    SCROW   mnTabEndRow;

public:
    ScDPOutputImpl( ScDocument* pDoc, sal_uInt16 nTab,
        SCCOL   nTabStartCol,
        SCROW   nTabStartRow,
        SCCOL   nMemberStartCol,
        SCROW   nMemberStartRow,
        SCCOL nDataStartCol,
        SCROW nDataStartRow,
        SCCOL nTabEndCol,
        SCROW nTabEndRow );
    bool AddRow( SCROW nRow );
    bool AddCol( SCCOL nCol );

    void OutputDataArea();
    void OutputBlockFrame ( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, bool bHori = false );

};

void ScDPOutputImpl::OutputDataArea()
{
    AddRow( mnDataStartRow );
    AddCol( mnDataStartCol );

    mnCols.push_back( mnTabEndCol+1); //set last row bottom
    mnRows.push_back( mnTabEndRow+1); //set last col bottom

    bool bAllRows = ( ( mnTabEndRow - mnDataStartRow + 2 ) == (SCROW) mnRows.size() );

    std::sort( mnCols.begin(), mnCols.end(), lcl_compareColfuc );
    std::sort( mnRows.begin(), mnRows.end(), lcl_compareRowfuc );

    for( SCCOL nCol = 0; nCol < (SCCOL)mnCols.size()-1; nCol ++ )
    {
        if ( !bAllRows )
        {
            if ( nCol < (SCCOL)mnCols.size()-2)
            {
                for ( SCROW i = nCol%2; i < (SCROW)mnRows.size()-2; i +=2 )
                    OutputBlockFrame( mnCols[nCol], mnRows[i], mnCols[nCol+1]-1, mnRows[i+1]-1 );
                if ( mnRows.size()>=2 )
                    OutputBlockFrame(  mnCols[nCol], mnRows[mnRows.size()-2], mnCols[nCol+1]-1, mnRows[mnRows.size()-1]-1 );
            }
            else
            {
                for ( SCROW i = 0 ; i < (SCROW)mnRows.size()-1; i++ )
                    OutputBlockFrame(  mnCols[nCol], mnRows[i], mnCols[nCol+1]-1,  mnRows[i+1]-1 );
            }
        }
        else
            OutputBlockFrame( mnCols[nCol], mnRows.front(), mnCols[nCol+1]-1, mnRows.back()-1, bAllRows );
    }
    //out put rows area outer framer
    if ( mnTabStartCol != mnDataStartCol )
    {
        if ( mnTabStartRow != mnDataStartRow )
            OutputBlockFrame( mnTabStartCol, mnTabStartRow, mnDataStartCol-1, mnDataStartRow-1 );
        OutputBlockFrame( mnTabStartCol, mnDataStartRow, mnDataStartCol-1, mnTabEndRow );
    }
    //out put cols area outer framer
    OutputBlockFrame( mnDataStartCol, mnTabStartRow, mnTabEndCol, mnDataStartRow-1 );
}

ScDPOutputImpl::ScDPOutputImpl( ScDocument* pDoc, sal_uInt16 nTab,
        SCCOL   nTabStartCol,
        SCROW   nTabStartRow,
        SCCOL   nMemberStartCol,
        SCROW   nMemberStartRow,
        SCCOL nDataStartCol,
        SCROW nDataStartRow,
        SCCOL nTabEndCol,
        SCROW nTabEndRow ):
    mpDoc( pDoc ),
    mnTab( nTab ),
    mnTabStartCol( nTabStartCol ),
    mnTabStartRow( nTabStartRow ),
    mnMemberStartCol( nMemberStartCol),
    mnMemberStartRow( nMemberStartRow),
    mnDataStartCol ( nDataStartCol ),
    mnDataStartRow ( nDataStartRow ),
    mnTabEndCol(  nTabEndCol ),
    mnTabEndRow(  nTabEndRow )
{
    mbNeedLineCols.resize( nTabEndCol-nDataStartCol+1, false );
    mbNeedLineRows.resize( nTabEndRow-nDataStartRow+1, false );

}

bool ScDPOutputImpl::AddRow( SCROW nRow )
{
    if ( !mbNeedLineRows[ nRow - mnDataStartRow ] )
    {
        mbNeedLineRows[ nRow - mnDataStartRow ] = true;
        mnRows.push_back( nRow );
        return true;
    }
    else
        return false;
}

bool ScDPOutputImpl::AddCol( SCCOL nCol )
{

    if ( !mbNeedLineCols[ nCol - mnDataStartCol ] )
    {
        mbNeedLineCols[ nCol - mnDataStartCol ] = true;
        mnCols.push_back( nCol );
        return true;
    }
    else
        return false;
}

void ScDPOutputImpl::OutputBlockFrame ( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, bool bHori )
{
    Color color = SC_DP_FRAME_COLOR;
    ::editeng::SvxBorderLine aLine( &color, SC_DP_FRAME_INNER_BOLD );
    ::editeng::SvxBorderLine aOutLine( &color, SC_DP_FRAME_OUTER_BOLD );

    SvxBoxItem aBox( ATTR_BORDER );

    if ( nStartCol == mnTabStartCol )
        aBox.SetLine(&aOutLine, BOX_LINE_LEFT);
    else
        aBox.SetLine(&aLine, BOX_LINE_LEFT);

    if ( nStartRow == mnTabStartRow )
        aBox.SetLine(&aOutLine, BOX_LINE_TOP);
    else
        aBox.SetLine(&aLine, BOX_LINE_TOP);

    if ( nEndCol == mnTabEndCol ) //bottom row
        aBox.SetLine(&aOutLine, BOX_LINE_RIGHT);
    else
        aBox.SetLine(&aLine,  BOX_LINE_RIGHT);

     if ( nEndRow == mnTabEndRow ) //bottom
        aBox.SetLine(&aOutLine,  BOX_LINE_BOTTOM);
    else
        aBox.SetLine(&aLine,  BOX_LINE_BOTTOM);


    SvxBoxInfoItem aBoxInfo( ATTR_BORDER_INNER );
    aBoxInfo.SetValid(VALID_VERT,false );
    if ( bHori )
    {
        aBoxInfo.SetValid(VALID_HORI,true);
        aBoxInfo.SetLine( &aLine, BOXINFO_LINE_HORI );
    }
    else
        aBoxInfo.SetValid(VALID_HORI,false );

    aBoxInfo.SetValid(VALID_DISTANCE,false);

    mpDoc->ApplyFrameAreaTab( ScRange(  nStartCol, nStartRow, mnTab, nEndCol, nEndRow , mnTab ), &aBox, &aBoxInfo );

}

void lcl_SetStyleById( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uInt16 nStrId )
{
    if ( nCol1 > nCol2 || nRow1 > nRow2 )
    {
        OSL_FAIL("SetStyleById: invalid range");
        return;
    }

    rtl::OUString aStyleName = ScGlobal::GetRscString( nStrId );
    ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
    ScStyleSheet* pStyle = (ScStyleSheet*) pStlPool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if (!pStyle)
    {
        //  create new style (was in ScPivot::SetStyle)

        pStyle = (ScStyleSheet*) &pStlPool->Make( aStyleName, SFX_STYLE_FAMILY_PARA,
                                                    SFXSTYLEBIT_USERDEF );
        pStyle->SetParent( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
        SfxItemSet& rSet = pStyle->GetItemSet();
        if ( nStrId==STR_PIVOT_STYLE_RESULT || nStrId==STR_PIVOT_STYLE_TITLE )
            rSet.Put( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
        if ( nStrId==STR_PIVOT_STYLE_CATEGORY || nStrId==STR_PIVOT_STYLE_TITLE )
            rSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY ) );
    }

    pDoc->ApplyStyleAreaTab( nCol1, nRow1, nCol2, nRow2, nTab, *pStyle );
}

void lcl_SetFrame( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uInt16 nWidth )
{
    ::editeng::SvxBorderLine aLine(0, nWidth, table::BorderLineStyle::SOLID);
    SvxBoxItem aBox( ATTR_BORDER );
    aBox.SetLine(&aLine, BOX_LINE_LEFT);
    aBox.SetLine(&aLine, BOX_LINE_TOP);
    aBox.SetLine(&aLine, BOX_LINE_RIGHT);
    aBox.SetLine(&aLine, BOX_LINE_BOTTOM);
    SvxBoxInfoItem aBoxInfo( ATTR_BORDER_INNER );
    aBoxInfo.SetValid(VALID_HORI,false);
    aBoxInfo.SetValid(VALID_VERT,false);
    aBoxInfo.SetValid(VALID_DISTANCE,false);

    pDoc->ApplyFrameAreaTab( ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab ), &aBox, &aBoxInfo );
}

// -----------------------------------------------------------------------

void lcl_FillNumberFormats( sal_uInt32*& rFormats, long& rCount,
                            const uno::Reference<sheet::XDataPilotMemberResults>& xLevRes,
                            const uno::Reference<container::XIndexAccess>& xDims )
{
    if ( rFormats )
        return;                         // already set

    //  xLevRes is from the data layout dimension
    //! use result sequence from ScDPOutLevelData!

    uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();

    long nSize = aResult.getLength();
    if (nSize)
    {
        //  get names/formats for all data dimensions
        //! merge this with the loop to collect ScDPOutLevelData?

        rtl::OUString aDataNames[SC_DPOUT_MAXLEVELS];
        sal_uInt32 nDataFormats[SC_DPOUT_MAXLEVELS];
        long nDataCount = 0;
        bool bAnySet = false;

        long nDimCount = xDims->getCount();
        for (long nDim=0; nDim<nDimCount; nDim++)
        {
            uno::Reference<uno::XInterface> xDim =
                    ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
            uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
            if ( xDimProp.is() && xDimName.is() )
            {
                sheet::DataPilotFieldOrientation eDimOrient =
                    (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                        xDimProp, rtl::OUString(SC_UNO_DP_ORIENTATION),
                        sheet::DataPilotFieldOrientation_HIDDEN );
                if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
                {
                    aDataNames[nDataCount] = xDimName->getName();
                    long nFormat = ScUnoHelpFunctions::GetLongProperty(
                                            xDimProp,
                                            rtl::OUString(SC_UNONAME_NUMFMT) );
                    nDataFormats[nDataCount] = nFormat;
                    if ( nFormat != 0 )
                        bAnySet = true;
                    ++nDataCount;
                }
            }
        }

        if ( bAnySet )      // forget everything if all formats are 0 (or no data dimensions)
        {
            const sheet::MemberResult* pArray = aResult.getConstArray();

            rtl::OUString aName;
            sal_uInt32* pNumFmt = new sal_uInt32[nSize];
            if (nDataCount == 1)
            {
                //  only one data dimension -> use its numberformat everywhere
                long nFormat = nDataFormats[0];
                for (long nPos=0; nPos<nSize; nPos++)
                    pNumFmt[nPos] = nFormat;
            }
            else
            {
                for (long nPos=0; nPos<nSize; nPos++)
                {
                    //  if CONTINUE bit is set, keep previous name
                    //! keep number format instead!
                    if ( !(pArray[nPos].Flags & sheet::MemberResultFlags::CONTINUE) )
                        aName = pArray[nPos].Name;

                    sal_uInt32 nFormat = 0;
                    for (long i=0; i<nDataCount; i++)
                        if (aName == aDataNames[i])         //! search more efficiently?
                        {
                            nFormat = nDataFormats[i];
                            break;
                        }
                    pNumFmt[nPos] = nFormat;
                }
            }

            rFormats = pNumFmt;
            rCount = nSize;
        }
    }
}

sal_uInt32 lcl_GetFirstNumberFormat( const uno::Reference<container::XIndexAccess>& xDims )
{
    long nDimCount = xDims->getCount();
    for (long nDim=0; nDim<nDimCount; nDim++)
    {
        uno::Reference<uno::XInterface> xDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        if ( xDimProp.is() )
        {
            sheet::DataPilotFieldOrientation eDimOrient =
                (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                    xDimProp, rtl::OUString(SC_UNO_DP_ORIENTATION),
                    sheet::DataPilotFieldOrientation_HIDDEN );
            if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                long nFormat = ScUnoHelpFunctions::GetLongProperty(
                                        xDimProp,
                                        rtl::OUString(SC_UNONAME_NUMFMT) );

                return nFormat;     // use format from first found data dimension
            }
        }
    }

    return 0;       // none found
}

void lcl_SortFields( ScDPOutLevelData* pFields, long nFieldCount )
{
    for (long i=0; i+1<nFieldCount; i++)
    {
        for (long j=0; j+i+1<nFieldCount; j++)
            if ( pFields[j+1] < pFields[j] )
                pFields[j].Swap( pFields[j+1] );
    }
}

bool lcl_MemberEmpty( const uno::Sequence<sheet::MemberResult>& rSeq )
{
    //  used to skip levels that have no members

    long nLen = rSeq.getLength();
    const sheet::MemberResult* pArray = rSeq.getConstArray();
    for (long i=0; i<nLen; i++)
        if (pArray[i].Flags & sheet::MemberResultFlags::HASMEMBER)
            return false;

    return true;    // no member data -> empty
}

uno::Sequence<sheet::MemberResult> lcl_GetSelectedPageAsResult( const uno::Reference<beans::XPropertySet>& xDimProp )
{
    uno::Sequence<sheet::MemberResult> aRet;
    if ( xDimProp.is() )
    {
        try
        {
            //! merge with ScDPDimension::setPropertyValue?

            uno::Any aValue = xDimProp->getPropertyValue( rtl::OUString(SC_UNO_DP_FILTER) );

            uno::Sequence<sheet::TableFilterField> aSeq;
            if (aValue >>= aSeq)
            {
                if ( aSeq.getLength() == 1 )
                {
                    const sheet::TableFilterField& rField = aSeq[0];
                    if ( rField.Field == 0 && rField.Operator == sheet::FilterOperator_EQUAL && !rField.IsNumeric )
                    {
                        rtl::OUString aSelectedPage( rField.StringValue );
                        //! different name/caption string?
                        sheet::MemberResult aResult( aSelectedPage, aSelectedPage, 0 );
                        aRet = uno::Sequence<sheet::MemberResult>( &aResult, 1 );
                    }
                }
                // else return empty sequence
            }
        }
        catch ( uno::Exception& )
        {
            // recent addition - allow source to not handle it (no error)
        }
    }
    return aRet;
}

}

ScDPOutput::ScDPOutput( ScDocument* pD, const uno::Reference<sheet::XDimensionsSupplier>& xSrc,
                        const ScAddress& rPos, bool bFilter ) :
    pDoc( pD ),
    xSource( xSrc ),
    aStartPos( rPos ),
    pColNumFmt( NULL ),
    pRowNumFmt( NULL ),
    nColFmtCount( 0 ),
    nRowFmtCount( 0 ),
    nSingleNumFmt( 0 ),
    bDoFilter(bFilter),
    bResultsError(false),
    mbHasDataLayout(false),
    bSizesValid(false),
    bSizeOverflow(false),
    mbHeaderLayout(false)
{
    nTabStartCol = nMemberStartCol = nDataStartCol = nTabEndCol = 0;
    nTabStartRow = nMemberStartRow = nDataStartRow = nTabEndRow = 0;

    pColFields  = new ScDPOutLevelData[SC_DPOUT_MAXLEVELS];
    pRowFields  = new ScDPOutLevelData[SC_DPOUT_MAXLEVELS];
    pPageFields = new ScDPOutLevelData[SC_DPOUT_MAXLEVELS];
    nColFieldCount = 0;
    nRowFieldCount = 0;
    nPageFieldCount = 0;

    uno::Reference<sheet::XDataPilotResults> xResult( xSource, uno::UNO_QUERY );
    if ( xSource.is() && xResult.is() )
    {
        //  get dimension results:

        uno::Reference<container::XIndexAccess> xDims =
                new ScNameToIndexAccess( xSource->getDimensions() );
        long nDimCount = xDims->getCount();
        for (long nDim=0; nDim<nDimCount; nDim++)
        {
            uno::Reference<uno::XInterface> xDim =
                    ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
            uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
            if ( xDimProp.is() && xDimSupp.is() )
            {
                sheet::DataPilotFieldOrientation eDimOrient =
                    (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                        xDimProp, rtl::OUString(SC_UNO_DP_ORIENTATION),
                        sheet::DataPilotFieldOrientation_HIDDEN );
                long nDimPos = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                        rtl::OUString(SC_UNO_DP_POSITION) );
                bool bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, rtl::OUString(SC_UNO_DP_ISDATALAYOUT));
                bool bHasHiddenMember = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, OUString(SC_UNO_DP_HAS_HIDDEN_MEMBER));

                if ( eDimOrient != sheet::DataPilotFieldOrientation_HIDDEN )
                {
                    uno::Reference<container::XIndexAccess> xHiers =
                            new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                    long nHierarchy = ScUnoHelpFunctions::GetLongProperty(
                                            xDimProp,
                                            rtl::OUString(SC_UNO_DP_USEDHIERARCHY) );
                    if ( nHierarchy >= xHiers->getCount() )
                        nHierarchy = 0;

                    uno::Reference<uno::XInterface> xHier =
                            ScUnoHelpFunctions::AnyToInterface(
                                                xHiers->getByIndex(nHierarchy) );
                    uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
                    if ( xHierSupp.is() )
                    {
                        uno::Reference<container::XIndexAccess> xLevels =
                                new ScNameToIndexAccess( xHierSupp->getLevels() );
                        long nLevCount = xLevels->getCount();
                        for (long nLev=0; nLev<nLevCount; nLev++)
                        {
                            uno::Reference<uno::XInterface> xLevel =
                                        ScUnoHelpFunctions::AnyToInterface(
                                                            xLevels->getByIndex(nLev) );
                            uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                            uno::Reference<sheet::XDataPilotMemberResults> xLevRes(
                                    xLevel, uno::UNO_QUERY );
                            if ( xLevNam.is() && xLevRes.is() )
                            {
                                rtl::OUString aName = xLevNam->getName();
                                Reference<XPropertySet> xPropSet(xLevel, UNO_QUERY);
                                // Caption equals the field name by default.
                                // #i108948# use ScUnoHelpFunctions::GetStringProperty, because
                                // LayoutName is new and may not be present in external implementation
                                OUString aCaption = ScUnoHelpFunctions::GetStringProperty( xPropSet,
                                    OUString(SC_UNO_DP_LAYOUTNAME), aName );

                                bool bRowFieldHasMember = false;
                                switch ( eDimOrient )
                                {
                                    case sheet::DataPilotFieldOrientation_COLUMN:
                                        pColFields[nColFieldCount].nDim    = nDim;
                                        pColFields[nColFieldCount].nHier   = nHierarchy;
                                        pColFields[nColFieldCount].nLevel  = nLev;
                                        pColFields[nColFieldCount].nDimPos = nDimPos;
                                        pColFields[nColFieldCount].aResult = xLevRes->getResults();
                                        pColFields[nColFieldCount].maName  = aName;
                                        pColFields[nColFieldCount].maCaption= aCaption;
                                        pColFields[nColFieldCount].mbHasHiddenMember = bHasHiddenMember;
                                        pColFields[nColFieldCount].mbDataLayout = bIsDataLayout;
                                        if (!lcl_MemberEmpty(pColFields[nColFieldCount].aResult))
                                            ++nColFieldCount;
                                        break;
                                    case sheet::DataPilotFieldOrientation_ROW:
                                        pRowFields[nRowFieldCount].nDim    = nDim;
                                        pRowFields[nRowFieldCount].nHier   = nHierarchy;
                                        pRowFields[nRowFieldCount].nLevel  = nLev;
                                        pRowFields[nRowFieldCount].nDimPos = nDimPos;
                                        pRowFields[nRowFieldCount].aResult = xLevRes->getResults();
                                        pRowFields[nRowFieldCount].maName  = aName;
                                        pRowFields[nRowFieldCount].maCaption= aCaption;
                                        pRowFields[nRowFieldCount].mbHasHiddenMember = bHasHiddenMember;
                                        pRowFields[nRowFieldCount].mbDataLayout = bIsDataLayout;
                                        if (!lcl_MemberEmpty(pRowFields[nRowFieldCount].aResult))
                                        {
                                            ++nRowFieldCount;
                                            bRowFieldHasMember = true;
                                        }
                                        break;
                                    case sheet::DataPilotFieldOrientation_PAGE:
                                        pPageFields[nPageFieldCount].nDim    = nDim;
                                        pPageFields[nPageFieldCount].nHier   = nHierarchy;
                                        pPageFields[nPageFieldCount].nLevel  = nLev;
                                        pPageFields[nPageFieldCount].nDimPos = nDimPos;
                                        pPageFields[nPageFieldCount].aResult = lcl_GetSelectedPageAsResult(xDimProp);
                                        pPageFields[nPageFieldCount].maName  = aName;
                                        pPageFields[nPageFieldCount].maCaption= aCaption;
                                        pPageFields[nPageFieldCount].mbHasHiddenMember = bHasHiddenMember;
                                        // no check on results for page fields
                                        ++nPageFieldCount;
                                        break;
                                    default:
                                    {
                                        // added to avoid warnings
                                    }
                                }

                                // get number formats from data dimensions
                                if ( bIsDataLayout )
                                {
                                    if (bRowFieldHasMember)
                                        mbHasDataLayout = true;

                                    OSL_ENSURE( nLevCount == 1, "data layout: multiple levels?" );
                                    if ( eDimOrient == sheet::DataPilotFieldOrientation_COLUMN )
                                        lcl_FillNumberFormats( pColNumFmt, nColFmtCount, xLevRes, xDims );
                                    else if ( eDimOrient == sheet::DataPilotFieldOrientation_ROW )
                                        lcl_FillNumberFormats( pRowNumFmt, nRowFmtCount, xLevRes, xDims );
                                }
                            }
                        }
                    }
                }
                else if ( bIsDataLayout )
                {
                    // data layout dimension is hidden (allowed if there is only one data dimension)
                    // -> use the number format from the first data dimension for all results

                    nSingleNumFmt = lcl_GetFirstNumberFormat( xDims );
                }
            }
        }
        lcl_SortFields( pColFields, nColFieldCount );
        lcl_SortFields( pRowFields, nRowFieldCount );
        lcl_SortFields( pPageFields, nPageFieldCount );

        //  get data results:

        try
        {
            aData = xResult->getResults();
        }
        catch (uno::RuntimeException&)
        {
            bResultsError = true;
        }
    }

    // get "DataDescription" property (may be missing in external sources)

    uno::Reference<beans::XPropertySet> xSrcProp( xSource, uno::UNO_QUERY );
    if ( xSrcProp.is() )
    {
        try
        {
            uno::Any aAny = xSrcProp->getPropertyValue(
                    rtl::OUString(SC_UNO_DP_DATADESC) );
            rtl::OUString aUStr;
            aAny >>= aUStr;
            aDataDescription = aUStr;
        }
        catch(uno::Exception&)
        {
        }
    }
}

ScDPOutput::~ScDPOutput()
{
    delete[] pColFields;
    delete[] pRowFields;
    delete[] pPageFields;

    delete[] pColNumFmt;
    delete[] pRowNumFmt;
}

void ScDPOutput::SetPosition( const ScAddress& rPos )
{
    aStartPos = rPos;
    bSizesValid = bSizeOverflow = false;
}

void ScDPOutput::DataCell( SCCOL nCol, SCROW nRow, SCTAB nTab, const sheet::DataResult& rData )
{
    long nFlags = rData.Flags;
    if ( nFlags & sheet::DataResultFlags::ERROR )
    {
        pDoc->SetError( nCol, nRow, nTab, errNoValue );
    }
    else if ( nFlags & sheet::DataResultFlags::HASDATA )
    {
        pDoc->SetValue( nCol, nRow, nTab, rData.Value );

        //  use number formats from source

        OSL_ENSURE( bSizesValid, "DataCell: !bSizesValid" );
        sal_uInt32 nFormat = 0;
        if ( pColNumFmt )
        {
            if ( nCol >= nDataStartCol )
            {
                long nIndex = nCol - nDataStartCol;
                if ( nIndex < nColFmtCount )
                    nFormat = pColNumFmt[nIndex];
            }
        }
        else if ( pRowNumFmt )
        {
            if ( nRow >= nDataStartRow )
            {
                long nIndex = nRow - nDataStartRow;
                if ( nIndex < nRowFmtCount )
                    nFormat = pRowNumFmt[nIndex];
            }
        }
        else if ( nSingleNumFmt != 0 )
            nFormat = nSingleNumFmt;        // single format is used everywhere
        if ( nFormat != 0 )
            pDoc->ApplyAttr( nCol, nRow, nTab, SfxUInt32Item( ATTR_VALUE_FORMAT, nFormat ) );
    }
    //  SubTotal formatting is controlled by headers
}

void ScDPOutput::HeaderCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                             const sheet::MemberResult& rData, bool bColHeader, long nLevel )
{
    long nFlags = rData.Flags;

    if ( nFlags & sheet::MemberResultFlags::HASMEMBER )
    {
        bool bNumeric = (nFlags & sheet::MemberResultFlags::NUMERIC) != 0;
        ScSetStringParam aParam;
        if (bNumeric)
        {
            aParam.mbDetectNumberFormat = true;
            aParam.meSetTextNumFormat = ScSetStringParam::Never;
            aParam.mbHandleApostrophe = true;
        }
        else
        {
            aParam.mbDetectNumberFormat = false;
            aParam.meSetTextNumFormat = ScSetStringParam::Always;
            aParam.mbHandleApostrophe = false;
        }
        pDoc->SetString(nCol, nRow, nTab, rData.Caption, &aParam);
    }

    if ( nFlags & sheet::MemberResultFlags::SUBTOTAL )
    {
        ScDPOutputImpl outputimp( pDoc, nTab,
            nTabStartCol, nTabStartRow, nMemberStartCol, nMemberStartRow,
            nDataStartCol, nDataStartRow, nTabEndCol, nTabEndRow );
        //! limit frames to horizontal or vertical?
        if (bColHeader)
        {
            outputimp.OutputBlockFrame( nCol,nMemberStartRow+(SCROW)nLevel, nCol,nDataStartRow-1 );

            lcl_SetStyleById( pDoc,nTab, nCol,nMemberStartRow+(SCROW)nLevel, nCol,nDataStartRow-1,
                                    STR_PIVOT_STYLE_TITLE );
            lcl_SetStyleById( pDoc,nTab, nCol,nDataStartRow, nCol,nTabEndRow,
                                    STR_PIVOT_STYLE_RESULT );
        }
        else
        {
            outputimp.OutputBlockFrame( nMemberStartCol+(SCCOL)nLevel,nRow, nDataStartCol-1,nRow );
            lcl_SetStyleById( pDoc,nTab, nMemberStartCol+(SCCOL)nLevel,nRow, nDataStartCol-1,nRow,
                                    STR_PIVOT_STYLE_TITLE );
            lcl_SetStyleById( pDoc,nTab, nDataStartCol,nRow, nTabEndCol,nRow,
                                    STR_PIVOT_STYLE_RESULT );
        }
    }
}

void ScDPOutput::FieldCell(
    SCCOL nCol, SCROW nRow, SCTAB nTab, const ScDPOutLevelData& rData, bool bInTable)
{
    // Avoid unwanted automatic format detection.
    ScSetStringParam aParam;
    aParam.mbDetectNumberFormat = false;
    aParam.meSetTextNumFormat = ScSetStringParam::Always;
    aParam.mbHandleApostrophe = false;
    pDoc->SetString(nCol, nRow, nTab, rData.maCaption, &aParam);

    if (bInTable)
        lcl_SetFrame( pDoc,nTab, nCol,nRow, nCol,nRow, 20 );

    //  Button
    sal_uInt16 nMergeFlag = SC_MF_BUTTON;
    if (!rData.mbDataLayout)
        nMergeFlag |= SC_MF_BUTTON_POPUP;
    if (rData.mbHasHiddenMember)
        nMergeFlag |= SC_MF_HIDDEN_MEMBER;
    pDoc->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, nMergeFlag);

    lcl_SetStyleById( pDoc,nTab, nCol,nRow, nCol,nRow, STR_PIVOT_STYLE_FIELDNAME );
}

void lcl_DoFilterButton( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    pDoc->SetString( nCol, nRow, nTab, ScGlobal::GetRscString(STR_CELL_FILTER) );
    pDoc->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, SC_MF_BUTTON);
}

void ScDPOutput::CalcSizes()
{
    if (!bSizesValid)
    {
        //  get column size of data from first row
        //! allow different sizes (and clear following areas) ???

        nRowCount = aData.getLength();
        const uno::Sequence<sheet::DataResult>* pRowAry = aData.getConstArray();
        nColCount = nRowCount ? ( pRowAry[0].getLength() ) : 0;

        nHeaderSize = 1;
        if (GetHeaderLayout() && nColFieldCount == 0)
            // Insert an extra header row only when there is no column field.
            nHeaderSize = 2;

        //  calculate output positions and sizes

        long nPageSize = 0;     //! use page fields!
        if ( bDoFilter || nPageFieldCount )
        {
            nPageSize += nPageFieldCount + 1;   // plus one empty row
            if ( bDoFilter )
                ++nPageSize;        //  filter button above the page fields
        }

        if ( aStartPos.Col() + nRowFieldCount + nColCount - 1 > MAXCOL ||
             aStartPos.Row() + nPageSize + nHeaderSize + nColFieldCount + nRowCount > MAXROW )
        {
            bSizeOverflow = true;
        }

        nTabStartCol = aStartPos.Col();
        nTabStartRow = aStartPos.Row() + (SCROW)nPageSize;          // below page fields
        nMemberStartCol = nTabStartCol;
        nMemberStartRow = nTabStartRow + (SCROW) nHeaderSize;
        nDataStartCol = nMemberStartCol + (SCCOL)nRowFieldCount;
        nDataStartRow = nMemberStartRow + (SCROW)nColFieldCount;
        if ( nColCount > 0 )
            nTabEndCol = nDataStartCol + (SCCOL)nColCount - 1;
        else
            nTabEndCol = nDataStartCol;     // single column will remain empty
        // if page fields are involved, include the page selection cells
        if ( nPageFieldCount > 0 && nTabEndCol < nTabStartCol + 1 )
            nTabEndCol = nTabStartCol + 1;
        if ( nRowCount > 0 )
            nTabEndRow = nDataStartRow + (SCROW)nRowCount - 1;
        else
            nTabEndRow = nDataStartRow;     // single row will remain empty
        bSizesValid = true;
    }
}

sal_Int32 ScDPOutput::GetPositionType(const ScAddress& rPos)
{
    using namespace ::com::sun::star::sheet;

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return DataPilotTablePositionType::NOT_IN_TABLE;

    CalcSizes();

    // Make sure the cursor is within the table.
    if (nCol < nTabStartCol || nRow < nTabStartRow || nCol > nTabEndCol || nRow > nTabEndRow)
        return DataPilotTablePositionType::NOT_IN_TABLE;

    // test for result data area.
    if (nCol >= nDataStartCol && nCol <= nTabEndCol && nRow >= nDataStartRow && nRow <= nTabEndRow)
        return DataPilotTablePositionType::RESULT;

    bool bInColHeader = (nRow >= nTabStartRow && nRow < nDataStartRow);
    bool bInRowHeader = (nCol >= nTabStartCol && nCol < nDataStartCol);

    if (bInColHeader && bInRowHeader)
        // probably in that ugly little box at the upper-left corner of the table.
        return DataPilotTablePositionType::OTHER;

    if (bInColHeader)
    {
        if (nRow == nTabStartRow)
            // first row in the column header area is always used for column
            // field buttons.
            return DataPilotTablePositionType::OTHER;

        return DataPilotTablePositionType::COLUMN_HEADER;
    }

    if (bInRowHeader)
        return DataPilotTablePositionType::ROW_HEADER;

    return DataPilotTablePositionType::OTHER;
}

void ScDPOutput::Output()
{
    long nField;
    SCTAB nTab = aStartPos.Tab();
    const uno::Sequence<sheet::DataResult>* pRowAry = aData.getConstArray();

    //  calculate output positions and sizes

    CalcSizes();
    if ( bSizeOverflow || bResultsError )   // does output area exceed sheet limits?
        return;                             // nothing

    //  clear whole (new) output area
    //! when modifying table, clear old area
    //! include IDF_OBJECTS ???
    pDoc->DeleteAreaTab( aStartPos.Col(), aStartPos.Row(), nTabEndCol, nTabEndRow, nTab, IDF_ALL );

    if ( bDoFilter )
        lcl_DoFilterButton( pDoc, aStartPos.Col(), aStartPos.Row(), nTab );

    //  output data results:

    for (long nRow=0; nRow<nRowCount; nRow++)
    {
        SCROW nRowPos = nDataStartRow + (SCROW)nRow;                    //! check for overflow
        const sheet::DataResult* pColAry = pRowAry[nRow].getConstArray();
        long nThisColCount = pRowAry[nRow].getLength();
        OSL_ENSURE( nThisColCount == nColCount, "count mismatch" );     //! ???
        for (long nCol=0; nCol<nThisColCount; nCol++)
        {
            SCCOL nColPos = nDataStartCol + (SCCOL)nCol;                //! check for overflow
            DataCell( nColPos, nRowPos, nTab, pColAry[nCol] );
        }
    }
    //  output page fields:

    for (nField=0; nField<nPageFieldCount; nField++)
    {
        SCCOL nHdrCol = aStartPos.Col();
        SCROW nHdrRow = aStartPos.Row() + nField + ( bDoFilter ? 1 : 0 );
        // draw without frame for consistency with filter button:
        FieldCell(nHdrCol, nHdrRow, nTab, pPageFields[nField], false);
        SCCOL nFldCol = nHdrCol + 1;

        rtl::OUString aPageValue;
        if ( pPageFields[nField].aResult.getLength() == 1 )
            aPageValue = pPageFields[nField].aResult[0].Caption;
        else
            aPageValue = SC_RESSTR(SCSTR_ALL);        //! separate string?

        pDoc->SetString( nFldCol, nHdrRow, nTab, aPageValue );

        lcl_SetFrame( pDoc,nTab, nFldCol,nHdrRow, nFldCol,nHdrRow, 20 );
        pDoc->ApplyAttr( nFldCol, nHdrRow, nTab, ScMergeFlagAttr(SC_MF_AUTO) );
        //! which style?
    }

    //  data description
    //  (may get overwritten by first row field)

    if (aDataDescription.isEmpty())
    {
        //! use default string ("result") ?
    }
    pDoc->SetString(nTabStartCol, nTabStartRow, nTab, aDataDescription);

    //  set STR_PIVOT_STYLE_INNER for whole data area (subtotals are overwritten)

    if ( nDataStartRow > nTabStartRow )
        lcl_SetStyleById( pDoc, nTab, nTabStartCol, nTabStartRow, nTabEndCol, nDataStartRow-1,
                            STR_PIVOT_STYLE_TOP );
    lcl_SetStyleById( pDoc, nTab, nDataStartCol, nDataStartRow, nTabEndCol, nTabEndRow,
                        STR_PIVOT_STYLE_INNER );

    //  output column headers:
    ScDPOutputImpl outputimp( pDoc, nTab,
        nTabStartCol, nTabStartRow, nMemberStartCol, nMemberStartRow,
        nDataStartCol, nDataStartRow, nTabEndCol, nTabEndRow );
    for (nField=0; nField<nColFieldCount; nField++)
    {
        SCCOL nHdrCol = nDataStartCol + (SCCOL)nField;              //! check for overflow
        FieldCell(nHdrCol, nTabStartRow, nTab, pColFields[nField], true);

        SCROW nRowPos = nMemberStartRow + (SCROW)nField;                //! check for overflow
        const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisColCount = rSequence.getLength();
        OSL_ENSURE( nThisColCount == nColCount, "count mismatch" );     //! ???
        for (long nCol=0; nCol<nThisColCount; nCol++)
        {
            SCCOL nColPos = nDataStartCol + (SCCOL)nCol;                //! check for overflow
            HeaderCell( nColPos, nRowPos, nTab, pArray[nCol], true, nField );
            if ( ( pArray[nCol].Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                !( pArray[nCol].Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                long nEnd = nCol;
                while ( nEnd+1 < nThisColCount && ( pArray[nEnd+1].Flags & sheet::MemberResultFlags::CONTINUE ) )
                    ++nEnd;
                SCCOL nEndColPos = nDataStartCol + (SCCOL)nEnd;     //! check for overflow
                if ( nField+1 < nColFieldCount )
                {
                    if ( nField == nColFieldCount - 2 )
                    {
                        outputimp.AddCol( nColPos );
                        if ( nColPos + 1 == nEndColPos  )
                            outputimp.OutputBlockFrame( nColPos,nRowPos, nEndColPos,nRowPos+1, true );
                    }
                    else
                        outputimp.OutputBlockFrame( nColPos,nRowPos, nEndColPos,nRowPos );

                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nEndColPos,nDataStartRow-1, STR_PIVOT_STYLE_CATEGORY );
                }
                else
                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nColPos,nDataStartRow-1, STR_PIVOT_STYLE_CATEGORY );
            }
            else if (  pArray[nCol].Flags & sheet::MemberResultFlags::SUBTOTAL )
                outputimp.AddCol( nColPos );
        }
        if ( nField== 0 && nColFieldCount == 1 )
            outputimp.OutputBlockFrame( nDataStartCol,nTabStartRow, nTabEndCol,nRowPos-1 );
    }

    //  output row headers:
    std::vector<bool> vbSetBorder;
    vbSetBorder.resize( nTabEndRow - nDataStartRow + 1, false );
    for (nField=0; nField<nRowFieldCount; nField++)
    {
        SCCOL nHdrCol = nTabStartCol + (SCCOL)nField;                   //! check for overflow
        SCROW nHdrRow = nDataStartRow - 1;
        FieldCell(nHdrCol, nHdrRow, nTab, pRowFields[nField], true);

        SCCOL nColPos = nMemberStartCol + (SCCOL)nField;                //! check for overflow
        const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisRowCount = rSequence.getLength();
        OSL_ENSURE( nThisRowCount == nRowCount, "count mismatch" );     //! ???
        for (long nRow=0; nRow<nThisRowCount; nRow++)
        {
            SCROW nRowPos = nDataStartRow + (SCROW)nRow;                //! check for overflow
            HeaderCell( nColPos, nRowPos, nTab, pArray[nRow], false, nField );
            if ( ( pArray[nRow].Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                !( pArray[nRow].Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                if ( nField+1 < nRowFieldCount )
                {
                    long nEnd = nRow;
                    while ( nEnd+1 < nThisRowCount && ( pArray[nEnd+1].Flags & sheet::MemberResultFlags::CONTINUE ) )
                        ++nEnd;
                    SCROW nEndRowPos = nDataStartRow + (SCROW)nEnd;     //! check for overflow
                    outputimp.AddRow( nRowPos );
                    if ( vbSetBorder[ nRow ] == false )
                    {
                        outputimp.OutputBlockFrame( nColPos, nRowPos, nTabEndCol, nEndRowPos );
                        vbSetBorder[ nRow ]  = true;
                    }
                    outputimp.OutputBlockFrame( nColPos, nRowPos, nColPos, nEndRowPos );

                    if ( nField == nRowFieldCount - 2 )
                        outputimp.OutputBlockFrame( nColPos+1, nRowPos, nColPos+1, nEndRowPos );

                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nDataStartCol-1,nEndRowPos, STR_PIVOT_STYLE_CATEGORY );
                }
                else
                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nDataStartCol-1,nRowPos, STR_PIVOT_STYLE_CATEGORY );
            }
            else if (  pArray[nRow].Flags & sheet::MemberResultFlags::SUBTOTAL )
                outputimp.AddRow( nRowPos );
        }
    }

    outputimp.OutputDataArea();
}

ScRange ScDPOutput::GetOutputRange( sal_Int32 nRegionType )
{
    using namespace ::com::sun::star::sheet;

    CalcSizes();

    SCTAB nTab = aStartPos.Tab();
    switch (nRegionType)
    {
        case DataPilotOutputRangeType::RESULT:
            return ScRange(nDataStartCol, nDataStartRow, nTab, nTabEndCol, nTabEndRow, nTab);
        case DataPilotOutputRangeType::TABLE:
            return ScRange(aStartPos.Col(), nTabStartRow, nTab, nTabEndCol, nTabEndRow, nTab);
        default:
            OSL_ENSURE(nRegionType == DataPilotOutputRangeType::WHOLE, "ScDPOutput::GetOutputRange: unknown region type");
        break;
    }
    return ScRange(aStartPos.Col(), aStartPos.Row(), nTab, nTabEndCol, nTabEndRow, nTab);
}

bool ScDPOutput::HasError()
{
    CalcSizes();

    return bSizeOverflow || bResultsError;
}

long ScDPOutput::GetHeaderRows()
{
    return nPageFieldCount + ( bDoFilter ? 1 : 0 );
}

void ScDPOutput::GetMemberResultNames(ScDPUniqueStringSet& rNames, long nDimension)
{
    //  Return the list of all member names in a dimension's MemberResults.
    //  Only the dimension has to be compared because this is only used with table data,
    //  where each dimension occurs only once.

    uno::Sequence<sheet::MemberResult> aMemberResults;
    bool bFound = false;
    long nField;

    // look in column fields

    for (nField=0; nField<nColFieldCount && !bFound; nField++)
        if ( pColFields[nField].nDim == nDimension )
        {
            aMemberResults = pColFields[nField].aResult;
            bFound = true;
        }

    // look in row fields

    for (nField=0; nField<nRowFieldCount && !bFound; nField++)
        if ( pRowFields[nField].nDim == nDimension )
        {
            aMemberResults = pRowFields[nField].aResult;
            bFound = true;
        }

    // collect the member names

    if ( bFound )
    {
        const sheet::MemberResult* pArray = aMemberResults.getConstArray();
        long nResultCount = aMemberResults.getLength();

        for (long nItem=0; nItem<nResultCount; nItem++)
        {
            if ( pArray[nItem].Flags & sheet::MemberResultFlags::HASMEMBER )
                rNames.insert(pArray[nItem].Name);
        }
    }
}

void ScDPOutput::SetHeaderLayout(bool bUseGrid)
{
    mbHeaderLayout = bUseGrid;
    bSizesValid = false;
}

bool ScDPOutput::GetHeaderLayout() const
{
    return mbHeaderLayout;
}

namespace {

void lcl_GetTableVars( sal_Int32& rGrandTotalCols, sal_Int32& rGrandTotalRows, sal_Int32& rDataLayoutIndex,
                       std::vector<rtl::OUString>& rDataNames, std::vector<rtl::OUString>& rGivenNames,
                       sheet::DataPilotFieldOrientation& rDataOrient,
                       const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    rDataLayoutIndex = -1;  // invalid
    rGrandTotalCols = 0;
    rGrandTotalRows = 0;
    rDataOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    uno::Reference<beans::XPropertySet> xSrcProp( xSource, uno::UNO_QUERY );
    bool bColGrand = ScUnoHelpFunctions::GetBoolProperty(
        xSrcProp, rtl::OUString(SC_UNO_DP_COLGRAND));
    if ( bColGrand )
        rGrandTotalCols = 1;    // default if data layout not in columns

    bool bRowGrand = ScUnoHelpFunctions::GetBoolProperty(
        xSrcProp, rtl::OUString(SC_UNO_DP_ROWGRAND));
    if ( bRowGrand )
        rGrandTotalRows = 1;    // default if data layout not in rows

    if ( xSource.is() )
    {
        // find index and orientation of "data layout" dimension, count data dimensions

        sal_Int32 nDataCount = 0;

        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xSource->getDimensions() );
        long nDimCount = xDims->getCount();
        for (long nDim=0; nDim<nDimCount; nDim++)
        {
            uno::Reference<uno::XInterface> xDim =
                    ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                sheet::DataPilotFieldOrientation eDimOrient =
                    (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                        xDimProp, rtl::OUString(SC_UNO_DP_ORIENTATION),
                        sheet::DataPilotFieldOrientation_HIDDEN );
                if ( ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                         rtl::OUString(SC_UNO_DP_ISDATALAYOUT) ) )
                {
                    rDataLayoutIndex = nDim;
                    rDataOrient = eDimOrient;
                }
                if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
                {
                    rtl::OUString aSourceName;
                    rtl::OUString aGivenName;
                    ScDPOutput::GetDataDimensionNames( aSourceName, aGivenName, xDim );
                    rDataNames.push_back( aSourceName );
                    rGivenNames.push_back( aGivenName );

                    ++nDataCount;
                }
            }
        }

        if ( ( rDataOrient == sheet::DataPilotFieldOrientation_COLUMN ) && bColGrand )
            rGrandTotalCols = nDataCount;
        else if ( ( rDataOrient == sheet::DataPilotFieldOrientation_ROW ) && bRowGrand )
            rGrandTotalRows = nDataCount;
    }
}

}

void ScDPOutput::GetPositionData(const ScAddress& rPos, DataPilotTablePositionData& rPosData)
{
    using namespace ::com::sun::star::sheet;

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return;                                     // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    rPosData.PositionType = GetPositionType(rPos);
    switch (rPosData.PositionType)
    {
        case DataPilotTablePositionType::RESULT:
        {
            vector<DataPilotFieldFilter> aFilters;
            GetDataResultPositionData(aFilters, rPos);
            sal_Int32 nSize = aFilters.size();

            DataPilotTableResultData aResData;
            aResData.FieldFilters.realloc(nSize);
            for (sal_Int32 i = 0; i < nSize; ++i)
                aResData.FieldFilters[i] = aFilters[i];

            aResData.DataFieldIndex = 0;
            Reference<beans::XPropertySet> xPropSet(xSource, UNO_QUERY);
            if (xPropSet.is())
            {
                sal_Int32 nDataFieldCount = ScUnoHelpFunctions::GetLongProperty( xPropSet,
                                            rtl::OUString(SC_UNO_DP_DATAFIELDCOUNT) );
                if (nDataFieldCount > 0)
                    aResData.DataFieldIndex = (nRow - nDataStartRow) % nDataFieldCount;
            }

            // Copy appropriate DataResult object from the cached sheet::DataResult table.
            if (aData.getLength() > nRow - nDataStartRow &&
                aData[nRow-nDataStartRow].getLength() > nCol-nDataStartCol)
                aResData.Result = aData[nRow-nDataStartRow][nCol-nDataStartCol];

            rPosData.PositionData = makeAny(aResData);
            return;
        }
        case DataPilotTablePositionType::COLUMN_HEADER:
        {
            long nField = nRow - nTabStartRow - 1; // 1st line is used for the buttons
            if (nField < 0)
                break;

            const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nField].aResult;
            if (rSequence.getLength() == 0)
                break;
            const sheet::MemberResult* pArray = rSequence.getConstArray();

            long nItem = nCol - nDataStartCol;
            //  get origin of "continue" fields
            while (nItem > 0 && ( pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
                --nItem;

            if (nItem < 0)
                break;

            DataPilotTableHeaderData aHeaderData;
            aHeaderData.MemberName = OUString(pArray[nItem].Name);
            aHeaderData.Flags = pArray[nItem].Flags;
            aHeaderData.Dimension = static_cast<sal_Int32>(pColFields[nField].nDim);
            aHeaderData.Hierarchy = static_cast<sal_Int32>(pColFields[nField].nHier);
            aHeaderData.Level     = static_cast<sal_Int32>(pColFields[nField].nLevel);

            rPosData.PositionData = makeAny(aHeaderData);
            return;
        }
        case DataPilotTablePositionType::ROW_HEADER:
        {
            long nField = nCol - nTabStartCol;
            if (nField < 0)
                break;

            const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nField].aResult;
            if (rSequence.getLength() == 0)
                break;
            const sheet::MemberResult* pArray = rSequence.getConstArray();

            long nItem = nRow - nDataStartRow;
            //  get origin of "continue" fields
            while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
                --nItem;

            if (nItem < 0)
                break;

            DataPilotTableHeaderData aHeaderData;
            aHeaderData.MemberName = OUString(pArray[nItem].Name);
            aHeaderData.Flags = pArray[nItem].Flags;
            aHeaderData.Dimension = static_cast<sal_Int32>(pRowFields[nField].nDim);
            aHeaderData.Hierarchy = static_cast<sal_Int32>(pRowFields[nField].nHier);
            aHeaderData.Level     = static_cast<sal_Int32>(pRowFields[nField].nLevel);

            rPosData.PositionData = makeAny(aHeaderData);
            return;
        }
    }
}

bool ScDPOutput::GetDataResultPositionData(vector<sheet::DataPilotFieldFilter>& rFilters, const ScAddress& rPos)
{
    // Check to make sure there is at least one data field.
    Reference<beans::XPropertySet> xPropSet(xSource, UNO_QUERY);
    if (!xPropSet.is())
        return false;

    sal_Int32 nDataFieldCount = ScUnoHelpFunctions::GetLongProperty( xPropSet,
                                rtl::OUString(SC_UNO_DP_DATAFIELDCOUNT) );
    if (nDataFieldCount == 0)
        // No data field is present in this datapilot table.
        return false;

    // #i111421# use lcl_GetTableVars for correct size of totals and data layout position
    sal_Int32 nGrandTotalCols;
    sal_Int32 nGrandTotalRows;
    sal_Int32 nDataLayoutIndex;
    std::vector<rtl::OUString> aDataNames;
    std::vector<rtl::OUString> aGivenNames;
    sheet::DataPilotFieldOrientation eDataOrient;
    lcl_GetTableVars( nGrandTotalCols, nGrandTotalRows, nDataLayoutIndex, aDataNames, aGivenNames, eDataOrient, xSource );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return false;                                     // wrong sheet

    CalcSizes();

    // test for data area.
    if (nCol < nDataStartCol || nCol > nTabEndCol || nRow < nDataStartRow || nRow > nTabEndRow)
    {
        // Cell is outside the data field area.
        return false;
    }

    bool bFilterByCol = (nCol <= static_cast<SCCOL>(nTabEndCol - nGrandTotalCols));
    bool bFilterByRow = (nRow <= static_cast<SCROW>(nTabEndRow - nGrandTotalRows));

    // column fields
    for (SCCOL nColField = 0; nColField < nColFieldCount && bFilterByCol; ++nColField)
    {
        if (pColFields[nColField].nDim == nDataLayoutIndex)
            // There is no sense including the data layout field for filtering.
            continue;

        sheet::DataPilotFieldFilter filter;
        filter.FieldName = pColFields[nColField].maName;

        const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nColField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();

        OSL_ENSURE(nDataStartCol + rSequence.getLength() - 1 == nTabEndCol, "ScDPOutput::GetDataFieldCellData: error in geometric assumption");

        long nItem = nCol - nDataStartCol;
                //  get origin of "continue" fields
        while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
            --nItem;

        filter.MatchValue = pArray[nItem].Name;
        rFilters.push_back(filter);
    }

    // row fields
    for (SCROW nRowField = 0; nRowField < nRowFieldCount && bFilterByRow; ++nRowField)
    {
        if (pRowFields[nRowField].nDim == nDataLayoutIndex)
            // There is no sense including the data layout field for filtering.
            continue;

        sheet::DataPilotFieldFilter filter;
        filter.FieldName = pRowFields[nRowField].maName;

        const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nRowField].aResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();

        OSL_ENSURE(nDataStartRow + rSequence.getLength() - 1 == nTabEndRow, "ScDPOutput::GetDataFieldCellData: error in geometric assumption");

        long nItem = nRow - nDataStartRow;
            //  get origin of "continue" fields
        while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
            --nItem;

        filter.MatchValue = pArray[nItem].Name;
        rFilters.push_back(filter);
    }

    return true;
}

namespace {

//
//  helper functions for ScDPOutput::GetPivotData
//

bool lcl_IsNamedDataField( const ScDPGetPivotDataField& rTarget, const rtl::OUString& rSourceName, const rtl::OUString& rGivenName )
{
    // match one of the names, ignoring case
    return ScGlobal::GetpTransliteration()->isEqual( rTarget.maFieldName, rSourceName ) ||
           ScGlobal::GetpTransliteration()->isEqual( rTarget.maFieldName, rGivenName );
}

bool lcl_IsNamedCategoryField( const ScDPGetPivotDataField& rFilter, const ScDPOutLevelData& rField )
{
    return ScGlobal::GetpTransliteration()->isEqual( rFilter.maFieldName, rField.maName );
}

bool lcl_IsCondition( const sheet::MemberResult& rResultEntry, const ScDPGetPivotDataField& rFilter )
{
    //! handle numeric conditions?
    return ScGlobal::GetpTransliteration()->isEqual( rResultEntry.Name, rFilter.maValStr );
}

bool lcl_CheckPageField( const ScDPOutLevelData& rField,
                         const std::vector< ScDPGetPivotDataField >& rFilters,
                         std::vector<bool>& rFilterUsed )
{
    for (SCSIZE nFilterPos = 0; nFilterPos < rFilters.size(); ++nFilterPos)
    {
        if ( lcl_IsNamedCategoryField( rFilters[nFilterPos], rField ) )
        {
            rFilterUsed[nFilterPos] = true;

            // page field result is empty or the selection as single entry (see lcl_GetSelectedPageAsResult)
            if ( rField.aResult.getLength() == 1 &&
                 lcl_IsCondition( rField.aResult[0], rFilters[nFilterPos] ) )
            {
                return true;        // condition matches page selection
            }
            else
            {
                return false;       // no page selection or different entry
            }
        }
    }

    return true;    // valid if the page field doesn't have a filter
}

uno::Sequence<sheet::GeneralFunction> lcl_GetSubTotals(
        const uno::Reference<sheet::XDimensionsSupplier>& xSource, const ScDPOutLevelData& rField )
{
    uno::Sequence<sheet::GeneralFunction> aSubTotals;

    uno::Reference<sheet::XHierarchiesSupplier> xHierSupp;
    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nIntCount = xIntDims->getCount();
    if ( rField.nDim < nIntCount )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
                                    xIntDims->getByIndex( rField.nDim ) );
        xHierSupp = uno::Reference<sheet::XHierarchiesSupplier>( xIntDim, uno::UNO_QUERY );
    }
    OSL_ENSURE( xHierSupp.is(), "dimension not found" );

    sal_Int32 nHierCount = 0;
    uno::Reference<container::XIndexAccess> xHiers;
    if ( xHierSupp.is() )
    {
        uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
        xHiers = new ScNameToIndexAccess( xHiersName );
        nHierCount = xHiers->getCount();
    }
    uno::Reference<uno::XInterface> xHier;
    if ( rField.nHier < nHierCount )
        xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex( rField.nHier ) );
    OSL_ENSURE( xHier.is(), "hierarchy not found" );

    sal_Int32 nLevCount = 0;
    uno::Reference<container::XIndexAccess> xLevels;
    uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
    if ( xLevSupp.is() )
    {
        uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
        xLevels = new ScNameToIndexAccess( xLevsName );
        nLevCount = xLevels->getCount();
    }
    uno::Reference<uno::XInterface> xLevel;
    if ( rField.nLevel < nLevCount )
        xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( rField.nLevel ) );
    OSL_ENSURE( xLevel.is(), "level not found" );

    uno::Reference<beans::XPropertySet> xLevelProp( xLevel, uno::UNO_QUERY );
    if ( xLevelProp.is() )
    {
        try
        {
            uno::Any aValue = xLevelProp->getPropertyValue( rtl::OUString(SC_UNO_DP_SUBTOTAL) );
            aValue >>= aSubTotals;
        }
        catch(uno::Exception&)
        {
        }
    }

    return aSubTotals;
}

void lcl_FilterInclude( std::vector<bool>& rResult, std::vector< sal_Int32 >& rSubtotal,
                        const ScDPOutLevelData& rField,
                        const std::vector< ScDPGetPivotDataField >& rFilters,
                        std::vector<bool>& rFilterUsed,
                        bool& rBeforeDataLayout,
                        sal_Int32 nGrandTotals, sal_Int32 nDataLayoutIndex,
                        const std::vector<rtl::OUString>& rDataNames, const std::vector<rtl::OUString>& rGivenNames,
                        const ScDPGetPivotDataField& rTarget, const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    // returns true if a filter was given for the field

    OSL_ENSURE( rFilters.size() == rFilterUsed.size(), "wrong size" );

    const bool bIsDataLayout = ( rField.nDim == nDataLayoutIndex );
    if (bIsDataLayout)
        rBeforeDataLayout = false;

    bool bHasFilter = false;
    ScDPGetPivotDataField aFilter;
    if ( !bIsDataLayout )          // selection of data field is handled separately
    {
        for (SCSIZE nFilterPos = 0; nFilterPos < rFilters.size() && !bHasFilter; ++nFilterPos)
        {
            if ( lcl_IsNamedCategoryField( rFilters[nFilterPos], rField ) )
            {
                aFilter = rFilters[nFilterPos];
                rFilterUsed[nFilterPos] = true;
                bHasFilter = true;
            }
        }
    }

    bool bHasFunc = bHasFilter && aFilter.meFunction != sheet::GeneralFunction_NONE;

    uno::Sequence<sheet::GeneralFunction> aSubTotals;
    if ( !bIsDataLayout )
        aSubTotals = lcl_GetSubTotals( xSource, rField );
    bool bManualSub = ( aSubTotals.getLength() > 0 && aSubTotals[0] != sheet::GeneralFunction_AUTO );

    const uno::Sequence<sheet::MemberResult>& rSequence = rField.aResult;
    const sheet::MemberResult* pArray = rSequence.getConstArray();
    sal_Int32 nSize = rSequence.getLength();

    OSL_ENSURE( (sal_Int32)rResult.size() == nSize, "Number of fields do not match result count" );

    sal_Int32 nContCount = 0;
    sal_Int32 nSubTotalCount = 0;
    sheet::MemberResult aPrevious;
    for( sal_Int32 j=0; j < nSize; j++ )
    {
        sheet::MemberResult aResultEntry = pArray[j];
        if ( aResultEntry.Flags & sheet::MemberResultFlags::CONTINUE )
        {
            aResultEntry = aPrevious;
            ++nContCount;
        }
        else if ( ( aResultEntry.Flags & sheet::MemberResultFlags::SUBTOTAL ) == 0 )
        {
            // count the CONTINUE entries before a SUBTOTAL
            nContCount = 0;
        }

        if ( j >= nSize - nGrandTotals )
        {
            // mark as subtotal for the preceding data
            if ( ( aResultEntry.Flags & sheet::MemberResultFlags::SUBTOTAL ) != 0 )
            {
                rSubtotal[j] = nSize - nGrandTotals;

                if ( rResult[j] && nGrandTotals > 1 )
                {
                    // grand total is always automatic
                    sal_Int32 nDataPos = j - ( nSize - nGrandTotals );
                    OSL_ENSURE( nDataPos < (sal_Int32)rDataNames.size(), "wrong data count" );
                    rtl::OUString aSourceName( rDataNames[nDataPos] );     // vector contains source names
                    rtl::OUString aGivenName( rGivenNames[nDataPos] );

                    rResult[j] = lcl_IsNamedDataField( rTarget, aSourceName, aGivenName );
                }
            }

            // treat "grand total" columns/rows as empty description, as if they were marked
            // in a previous field

            OSL_ENSURE( ( aResultEntry.Flags &
                            ( sheet::MemberResultFlags::HASMEMBER | sheet::MemberResultFlags::SUBTOTAL ) ) == 0 ||
                        ( aResultEntry.Flags &
                            ( sheet::MemberResultFlags::HASMEMBER | sheet::MemberResultFlags::SUBTOTAL ) ) ==
                                ( sheet::MemberResultFlags::HASMEMBER | sheet::MemberResultFlags::SUBTOTAL ),
                        "non-subtotal member found in grand total result" );
            aResultEntry.Flags = 0;
        }

        // mark subtotals (not grand total) for preceding data (assume CONTINUE is set)
        if ( ( aResultEntry.Flags & sheet::MemberResultFlags::SUBTOTAL ) != 0 )
        {
            rSubtotal[j] = nContCount + 1 + nSubTotalCount;

            if ( rResult[j] )
            {
                if ( bManualSub )
                {
                    if ( rBeforeDataLayout )
                    {
                        // manual subtotals and several data fields

                        sal_Int32 nDataCount = rDataNames.size();
                        sal_Int32 nFuncPos = nSubTotalCount / nDataCount;       // outer order: subtotal functions
                        sal_Int32 nDataPos = nSubTotalCount % nDataCount;       // inner order: data fields

                        rtl::OUString aSourceName( rDataNames[nDataPos] );             // vector contains source names
                        rtl::OUString aGivenName( rGivenNames[nDataPos] );

                        OSL_ENSURE( nFuncPos < aSubTotals.getLength(), "wrong subtotal count" );
                        rResult[j] = lcl_IsNamedDataField( rTarget, aSourceName, aGivenName ) &&
                                     aSubTotals[nFuncPos] == aFilter.meFunction;
                    }
                    else
                    {
                        // manual subtotals for a single data field

                        OSL_ENSURE( nSubTotalCount < aSubTotals.getLength(), "wrong subtotal count" );
                        rResult[j] = ( aSubTotals[nSubTotalCount] == aFilter.meFunction );
                    }
                }
                else    // automatic subtotals
                {
                    if ( rBeforeDataLayout )
                    {
                        OSL_ENSURE( nSubTotalCount < (sal_Int32)rDataNames.size(), "wrong data count" );
                        rtl::OUString aSourceName( rDataNames[nSubTotalCount] );       // vector contains source names
                        rtl::OUString aGivenName( rGivenNames[nSubTotalCount] );

                        rResult[j] = lcl_IsNamedDataField( rTarget, aSourceName, aGivenName );
                    }

                    // if a function was specified, automatic subtotals never match
                    if ( bHasFunc )
                        rResult[j] = false;
                }
            }

            ++nSubTotalCount;
        }
        else
            nSubTotalCount = 0;

        if( rResult[j] )
        {
            if ( bIsDataLayout )
            {
                if ( ( aResultEntry.Flags & sheet::MemberResultFlags::HASMEMBER ) != 0 )
                {
                    // Asterisks are added in ScDPSaveData::WriteToSource to create unique names.
                    //! preserve original name there?
                    rtl::OUString aSourceName = ScDPUtil::getSourceDimensionName(aResultEntry.Name);

                    rResult[j] = lcl_IsNamedDataField(
                        rTarget, aSourceName, aResultEntry.Caption);
                }
            }
            else if ( bHasFilter )
            {
                // name must match (simple value or subtotal)
                rResult[j] = ( ( aResultEntry.Flags & sheet::MemberResultFlags::HASMEMBER ) != 0 ) &&
                             lcl_IsCondition( aResultEntry, aFilter );

                // if a function was specified, simple (non-subtotal) values never match
                if ( bHasFunc && nSubTotalCount == 0 )
                    rResult[j] = false;
            }
            // if no condition is given, keep the columns/rows included
        }
        aPrevious = aResultEntry;
    }
}

void lcl_StripSubTotals( std::vector<bool>& rResult, const std::vector<sal_Int32>& rSubtotal )
{
    sal_Int32 nSize = rResult.size();
    OSL_ENSURE( (sal_Int32)rSubtotal.size() == nSize, "sizes don't match" );

    for (sal_Int32 nPos=0; nPos<nSize; nPos++)
        if ( rResult[nPos] && rSubtotal[nPos] )
        {
            // if a subtotal is included, clear the result flag for the columns/rows that the subtotal includes
            sal_Int32 nStart = nPos - rSubtotal[nPos];
            OSL_ENSURE( nStart >= 0, "invalid subtotal count" );

            for (sal_Int32 nPrev = nStart; nPrev < nPos; nPrev++)
                rResult[nPrev] = false;
        }
}

rtl::OUString lcl_GetDataFieldName( const rtl::OUString& rSourceName, sheet::GeneralFunction eFunc )
{
    sal_uInt16 nStrId = 0;
    switch ( eFunc )
    {
        case sheet::GeneralFunction_SUM:        nStrId = STR_FUN_TEXT_SUM;      break;
        case sheet::GeneralFunction_COUNT:
        case sheet::GeneralFunction_COUNTNUMS:  nStrId = STR_FUN_TEXT_COUNT;    break;
        case sheet::GeneralFunction_AVERAGE:    nStrId = STR_FUN_TEXT_AVG;      break;
        case sheet::GeneralFunction_MAX:        nStrId = STR_FUN_TEXT_MAX;      break;
        case sheet::GeneralFunction_MIN:        nStrId = STR_FUN_TEXT_MIN;      break;
        case sheet::GeneralFunction_PRODUCT:    nStrId = STR_FUN_TEXT_PRODUCT;  break;
        case sheet::GeneralFunction_STDEV:
        case sheet::GeneralFunction_STDEVP:     nStrId = STR_FUN_TEXT_STDDEV;   break;
        case sheet::GeneralFunction_VAR:
        case sheet::GeneralFunction_VARP:       nStrId = STR_FUN_TEXT_VAR;      break;
        case sheet::GeneralFunction_NONE:
        case sheet::GeneralFunction_AUTO:
        default:
        {
            OSL_FAIL("wrong function");
        }
    }
    if ( !nStrId )
        return rtl::OUString();

    rtl::OUStringBuffer aRet( ScGlobal::GetRscString( nStrId ) );
    aRet.appendAscii(RTL_CONSTASCII_STRINGPARAM(" - "));
    aRet.append(rSourceName);
    return aRet.makeStringAndClear();
}

}

void ScDPOutput::GetDataDimensionNames(
    rtl::OUString& rSourceName, rtl::OUString& rGivenName, const uno::Reference<uno::XInterface>& xDim )
{
    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimName.is() )
    {
        // Asterisks are added in ScDPSaveData::WriteToSource to create unique names.
        //! preserve original name there?
        rSourceName = ScDPUtil::getSourceDimensionName(xDimName->getName());

        // Generate "given name" the same way as in dptabres.
        //! Should use a stored name when available

        sheet::GeneralFunction eFunc = (sheet::GeneralFunction)ScUnoHelpFunctions::GetEnumProperty(
                                xDimProp, rtl::OUString(SC_UNO_DP_FUNCTION),
                                sheet::GeneralFunction_NONE );
        rGivenName = lcl_GetDataFieldName( rSourceName, eFunc );
    }
}

bool ScDPOutput::GetPivotData( ScDPGetPivotDataField& rTarget,
                               const std::vector< ScDPGetPivotDataField >& rFilters )
{
    CalcSizes();

    // need to know about grand total columns/rows:
    sal_Int32 nGrandTotalCols;
    sal_Int32 nGrandTotalRows;
    sal_Int32 nDataLayoutIndex;
    std::vector<rtl::OUString> aDataNames;
    std::vector<rtl::OUString> aGivenNames;
    sheet::DataPilotFieldOrientation eDataOrient;
    lcl_GetTableVars( nGrandTotalCols, nGrandTotalRows, nDataLayoutIndex, aDataNames, aGivenNames, eDataOrient, xSource );

    if ( aDataNames.empty() )
        return false;               // incomplete table without data fields -> no result

    if ( eDataOrient == sheet::DataPilotFieldOrientation_HIDDEN )
    {
        // no data layout field -> single data field -> must match the selected field in rTarget

        OSL_ENSURE( aDataNames.size() == 1, "several data fields but no data layout field" );
        if ( !lcl_IsNamedDataField( rTarget, aDataNames[0], aGivenNames[0] ) )
            return false;
    }

    std::vector<bool> aIncludeCol(nColCount, true);
    std::vector< sal_Int32 > aSubtotalCol( nColCount, 0 );
    std::vector<bool> aIncludeRow(nRowCount, true);
    std::vector< sal_Int32 > aSubtotalRow( nRowCount, 0 );

    std::vector<bool> aFilterUsed(rFilters.size(), false);

    long nField;
    long nCol;
    long nRow;
    bool bBeforeDataLayout;

    // look in column fields

    bBeforeDataLayout = ( eDataOrient == sheet::DataPilotFieldOrientation_COLUMN );
    for (nField=0; nField<nColFieldCount; nField++)
        lcl_FilterInclude( aIncludeCol, aSubtotalCol, pColFields[nField], rFilters, aFilterUsed, bBeforeDataLayout,
                           nGrandTotalCols, nDataLayoutIndex, aDataNames, aGivenNames, rTarget, xSource );

    // look in row fields

    bBeforeDataLayout = ( eDataOrient == sheet::DataPilotFieldOrientation_ROW );
    for (nField=0; nField<nRowFieldCount; nField++)
        lcl_FilterInclude( aIncludeRow, aSubtotalRow, pRowFields[nField], rFilters, aFilterUsed, bBeforeDataLayout,
                           nGrandTotalRows, nDataLayoutIndex, aDataNames, aGivenNames, rTarget, xSource );

    // page fields

    for (nField=0; nField<nPageFieldCount; nField++)
        if ( !lcl_CheckPageField( pPageFields[nField], rFilters, aFilterUsed ) )
            return false;

    // all filter fields must be used
    for (SCSIZE nFilter=0; nFilter<aFilterUsed.size(); nFilter++)
        if (!aFilterUsed[nFilter])
            return false;

    lcl_StripSubTotals( aIncludeCol, aSubtotalCol );
    lcl_StripSubTotals( aIncludeRow, aSubtotalRow );

    long nColPos = 0;
    long nColIncluded = 0;
    for (nCol=0; nCol<nColCount; nCol++)
        if (aIncludeCol[nCol])
        {
            nColPos = nCol;
            ++nColIncluded;
        }

    long nRowPos = 0;
    long nRowIncluded = 0;
    for (nRow=0; nRow<nRowCount; nRow++)
        if (aIncludeRow[nRow])
        {
            nRowPos = nRow;
            ++nRowIncluded;
        }

    if ( nColIncluded != 1 || nRowIncluded != 1 )
        return false;

    const uno::Sequence<sheet::DataResult>& rDataRow = aData[nRowPos];
    if ( nColPos >= rDataRow.getLength() )
        return false;

    const sheet::DataResult& rResult = rDataRow[nColPos];
    if ( rResult.Flags & sheet::DataResultFlags::ERROR )
        return false;                                       //! different error?

    rTarget.mbValIsStr = false;
    rTarget.mnValNum = rResult.Value;

    return true;
}

bool ScDPOutput::IsFilterButton( const ScAddress& rPos )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() || !bDoFilter )
        return false;                               // wrong sheet or no button at all

    //  filter button is at top left
    return ( nCol == aStartPos.Col() && nRow == aStartPos.Row() );
}

long ScDPOutput::GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return -1;                                      // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nRow == nTabStartRow && nCol >= nDataStartCol && nCol < nDataStartCol + nColFieldCount )
    {
        rOrient = sheet::DataPilotFieldOrientation_COLUMN;
        long nField = nCol - nDataStartCol;
        return pColFields[nField].nDim;
    }

    //  test for row header

    if ( nRow+1 == nDataStartRow && nCol >= nTabStartCol && nCol < nTabStartCol + nRowFieldCount )
    {
        rOrient = sheet::DataPilotFieldOrientation_ROW;
        long nField = nCol - nTabStartCol;
        return pRowFields[nField].nDim;
    }

    //  test for page field

    SCROW nPageStartRow = aStartPos.Row() + ( bDoFilter ? 1 : 0 );
    if ( nCol == aStartPos.Col() && nRow >= nPageStartRow && nRow < nPageStartRow + nPageFieldCount )
    {
        rOrient = sheet::DataPilotFieldOrientation_PAGE;
        long nField = nRow - nPageStartRow;
        return pPageFields[nField].nDim;
    }

    //! single data field (?)

    rOrient = sheet::DataPilotFieldOrientation_HIDDEN;
    return -1;      // invalid
}

bool ScDPOutput::GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop,
                                long nDragDim,
                                Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos )
{
    //  Rectangle instead of ScRange for rPosRect to allow for negative values

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return false;                                       // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nCol >= nDataStartCol && nCol <= nTabEndCol &&
            nRow + 1 >= nMemberStartRow && nRow < nMemberStartRow + nColFieldCount )
    {
        long nField = nRow - nMemberStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = true;
        }
        //! find start of dimension

        rPosRect = Rectangle( nDataStartCol, nMemberStartRow + nField,
                              nTabEndCol, nMemberStartRow + nField -1 );

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (long nPos=0; nPos<nColFieldCount && !bFound; nPos++)
        {
            if (pColFields[nPos].nDim == nDragDim)
            {
                bFound = true;
                if ( nField < nPos )
                    bBeforeDrag = true;
                else if ( nField > nPos )
                    bAfterDrag = true;
            }
        }

        if ( bFound )
        {
            if (!bBeforeDrag)
            {
                ++rPosRect.Bottom();
                if (bAfterDrag)
                    ++rPosRect.Top();
            }
        }
        else
        {
            if ( !bMouseTop )
            {
                ++rPosRect.Top();
                ++rPosRect.Bottom();
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_COLUMN;
        rDimPos = nField;                       //!...
        return true;
    }

    //  test for row header

    //  special case if no row fields
    bool bSpecial = ( nRow+1 >= nDataStartRow && nRow <= nTabEndRow &&
                        nRowFieldCount == 0 && nCol == nTabStartCol && bMouseLeft );

    if ( bSpecial || ( nRow+1 >= nDataStartRow && nRow <= nTabEndRow &&
                        nCol + 1 >= nTabStartCol && nCol < nTabStartCol + nRowFieldCount ) )
    {
        long nField = nCol - nTabStartCol;
        //! find start of dimension

        rPosRect = Rectangle( nTabStartCol + nField, nDataStartRow - 1,
                              nTabStartCol + nField - 1, nTabEndRow );

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (long nPos=0; nPos<nRowFieldCount && !bFound; nPos++)
        {
            if (pRowFields[nPos].nDim == nDragDim)
            {
                bFound = true;
                if ( nField < nPos )
                    bBeforeDrag = true;
                else if ( nField > nPos )
                    bAfterDrag = true;
            }
        }

        if ( bFound )
        {
            if (!bBeforeDrag)
            {
                ++rPosRect.Right();
                if (bAfterDrag)
                    ++rPosRect.Left();
            }
        }
        else
        {
            if ( !bMouseLeft )
            {
                ++rPosRect.Left();
                ++rPosRect.Right();
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_ROW;
        rDimPos = nField;                       //!...
        return true;
    }

    //  test for page fields

    SCROW nPageStartRow = aStartPos.Row() + ( bDoFilter ? 1 : 0 );
    if ( nCol >= aStartPos.Col() && nCol <= nTabEndCol &&
            nRow + 1 >= nPageStartRow && nRow < nPageStartRow + nPageFieldCount )
    {
        long nField = nRow - nPageStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = true;
        }
        //! find start of dimension

        rPosRect = Rectangle( aStartPos.Col(), nPageStartRow + nField,
                              nTabEndCol, nPageStartRow + nField - 1 );

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (long nPos=0; nPos<nPageFieldCount && !bFound; nPos++)
        {
            if (pPageFields[nPos].nDim == nDragDim)
            {
                bFound = true;
                if ( nField < nPos )
                    bBeforeDrag = true;
                else if ( nField > nPos )
                    bAfterDrag = true;
            }
        }

        if ( bFound )
        {
            if (!bBeforeDrag)
            {
                ++rPosRect.Bottom();
                if (bAfterDrag)
                    ++rPosRect.Top();
            }
        }
        else
        {
            if ( !bMouseTop )
            {
                ++rPosRect.Top();
                ++rPosRect.Bottom();
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_PAGE;
        rDimPos = nField;                       //!...
        return true;
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
