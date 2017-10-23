/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <scitems.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <dpoutput.hxx>
#include <dptabsrc.hxx>
#include <dpfilteredcache.hxx>
#include <document.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <markdata.hxx>
#include <attrib.hxx>
#include <formula/errorcodes.hxx>
#include <miscuno.hxx>
#include <globstr.hrc>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <scresid.hxx>
#include <unonames.hxx>
#include <strings.hrc>
#include <stringutil.hxx>
#include <dputil.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTableResultData.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/GeneralFunction2.hpp>

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

#define SC_DP_FRAME_INNER_BOLD      20
#define SC_DP_FRAME_OUTER_BOLD      40

#define SC_DP_FRAME_COLOR           Color(0,0,0) //( 0x20, 0x40, 0x68 )

struct ScDPOutLevelData
{
    long                                mnDim;
    long                                mnHier;
    long                                mnLevel;
    long                                mnDimPos;
    sal_uInt32 mnSrcNumFmt; /// Prevailing number format used in the source data.
    uno::Sequence<sheet::MemberResult>  maResult;
    OUString                            maName;     /// Name is the internal field name.
    OUString                            maCaption;  /// Caption is the name visible in the output table.
    double                              mfValue;    /// Value is the underlying numeric value, if any, or NaN
    bool                                mbHasHiddenMember:1;
    bool                                mbDataLayout:1;
    bool                                mbPageDim:1;

    ScDPOutLevelData(long nDim, long nHier, long nLevel, long nDimPos, sal_uInt32 nSrcNumFmt, const uno::Sequence<sheet::MemberResult>  &aResult,
                       const OUString &aName, const OUString &aCaption, double fValue, bool bHasHiddenMember, bool bDataLayout, bool bPageDim) :
        mnDim(nDim), mnHier(nHier), mnLevel(nLevel), mnDimPos(nDimPos), mnSrcNumFmt(nSrcNumFmt), maResult(aResult),
        maName(aName), maCaption(aCaption), mfValue(fValue), mbHasHiddenMember(bHasHiddenMember), mbDataLayout(bDataLayout),
        mbPageDim(bPageDim)
    {
        rtl::math::setNan(&mfValue);
    }

    // bug (73840) in uno::Sequence - copy and then assign doesn't work!
};



namespace {
    struct ScDPOutLevelDataComparator
    {
        bool operator()(const ScDPOutLevelData & rA, const ScDPOutLevelData & rB)
        {
            return rA.mnDimPos<rB.mnDimPos || ( rA.mnDimPos==rB.mnDimPos && rA.mnHier<rB.mnHier ) ||
            ( rA.mnDimPos==rB.mnDimPos && rA.mnHier==rB.mnHier && rA.mnLevel<rB.mnLevel );
        }
    };


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

    SCCOL   mnDataStartCol;
    SCROW   mnDataStartRow;
    SCCOL   mnTabEndCol;
    SCROW   mnTabEndRow;

public:
    ScDPOutputImpl( ScDocument* pDoc, sal_uInt16 nTab,
        SCCOL   nTabStartCol,
        SCROW   nTabStartRow,
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

    std::sort( mnCols.begin(), mnCols.end());
    std::sort( mnRows.begin(), mnRows.end());

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
        SCCOL nDataStartCol,
        SCROW nDataStartRow,
        SCCOL nTabEndCol,
        SCROW nTabEndRow ):
    mpDoc( pDoc ),
    mnTab( nTab ),
    mnTabStartCol( nTabStartCol ),
    mnTabStartRow( nTabStartRow ),
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
        aBox.SetLine(&aOutLine, SvxBoxItemLine::LEFT);
    else
        aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);

    if ( nStartRow == mnTabStartRow )
        aBox.SetLine(&aOutLine, SvxBoxItemLine::TOP);
    else
        aBox.SetLine(&aLine, SvxBoxItemLine::TOP);

    if ( nEndCol == mnTabEndCol ) //bottom row
        aBox.SetLine(&aOutLine, SvxBoxItemLine::RIGHT);
    else
        aBox.SetLine(&aLine,  SvxBoxItemLine::RIGHT);

     if ( nEndRow == mnTabEndRow ) //bottom
        aBox.SetLine(&aOutLine,  SvxBoxItemLine::BOTTOM);
    else
        aBox.SetLine(&aLine,  SvxBoxItemLine::BOTTOM);

    SvxBoxInfoItem aBoxInfo( ATTR_BORDER_INNER );
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT,false );
    if ( bHori )
    {
        aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI);
        aBoxInfo.SetLine( &aLine, SvxBoxInfoItemLine::HORI );
    }
    else
        aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI,false );

    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE,false);

    mpDoc->ApplyFrameAreaTab(ScRange(nStartCol, nStartRow, mnTab, nEndCol, nEndRow , mnTab), aBox, aBoxInfo);

}

void lcl_SetStyleById(ScDocument* pDoc, SCTAB nTab,
                      SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                      const char* pStrId)
{
    if ( nCol1 > nCol2 || nRow1 > nRow2 )
    {
        OSL_FAIL("SetStyleById: invalid range");
        return;
    }

    OUString aStyleName = ScGlobal::GetRscString(pStrId);
    ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
    ScStyleSheet* pStyle = static_cast<ScStyleSheet*>( pStlPool->Find( aStyleName, SfxStyleFamily::Para ) );
    if (!pStyle)
    {
        //  create new style (was in ScPivot::SetStyle)

        pStyle = static_cast<ScStyleSheet*>( &pStlPool->Make( aStyleName, SfxStyleFamily::Para,
                                                    SFXSTYLEBIT_USERDEF ) );
        pStyle->SetParent( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
        SfxItemSet& rSet = pStyle->GetItemSet();
        if (strcmp(pStrId, STR_PIVOT_STYLE_RESULT) == 0 || strcmp(pStrId, STR_PIVOT_STYLE_TITLE) == 0)
            rSet.Put( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
        if (strcmp(pStrId, STR_PIVOT_STYLE_CATEGORY) == 0 || strcmp(pStrId, STR_PIVOT_STYLE_TITLE) == 0)
            rSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY ) );
    }

    pDoc->ApplyStyleAreaTab( nCol1, nRow1, nCol2, nRow2, nTab, *pStyle );
}

void lcl_SetFrame( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_uInt16 nWidth )
{
    ::editeng::SvxBorderLine aLine(nullptr, nWidth, SvxBorderLineStyle::SOLID);
    SvxBoxItem aBox( ATTR_BORDER );
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
    aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
    SvxBoxInfoItem aBoxInfo( ATTR_BORDER_INNER );
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI,false);
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT,false);
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE,false);

    pDoc->ApplyFrameAreaTab(ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab), aBox, aBoxInfo);
}

void lcl_FillNumberFormats( std::unique_ptr<sal_uInt32[]>& rFormats, long& rCount,
                            const uno::Reference<sheet::XDataPilotMemberResults>& xLevRes,
                            const uno::Reference<container::XIndexAccess>& xDims )
{
    if ( rFormats )
        return;                         // already set

    //  xLevRes is from the data layout dimension
    //TODO: use result sequence from ScDPOutLevelData!

    uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();

    long nSize = aResult.getLength();
    if (!nSize)
        return;

    //  get names/formats for all data dimensions
    //TODO: merge this with the loop to collect ScDPOutLevelData?

    std::vector <OUString> aDataNames;
    std::vector <sal_uInt32> aDataFormats;
    sal_Int32 nDimCount = xDims->getCount();
    sal_Int32 nDim = 0;
    for ( ; nDim < nDimCount ; nDim++)
    {
        uno::Reference<uno::XInterface> xDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
        if ( xDimProp.is() && xDimName.is() )
        {
            sheet::DataPilotFieldOrientation eDimOrient =
                (sheet::DataPilotFieldOrientation) ScUnoHelpFunctions::GetEnumProperty(
                    xDimProp, SC_UNO_DP_ORIENTATION,
                    sheet::DataPilotFieldOrientation_HIDDEN );
            if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                aDataNames.push_back(xDimName->getName());
                long nFormat = ScUnoHelpFunctions::GetLongProperty(
                                        xDimProp,
                                        SC_UNONAME_NUMFMT );
                aDataFormats.push_back(nFormat);
            }
        }
    }

    if (aDataFormats.empty())
        return;

    const sheet::MemberResult* pArray = aResult.getConstArray();

    OUString aName;
    sal_uInt32* pNumFmt = new sal_uInt32[nSize];
    if (aDataFormats.size() == 1)
    {
        //  only one data dimension -> use its numberformat everywhere
        long nFormat = aDataFormats[0];
        for (long nPos=0; nPos<nSize; nPos++)
            pNumFmt[nPos] = nFormat;
    }
    else
    {
        for (long nPos=0; nPos<nSize; nPos++)
        {
            //  if CONTINUE bit is set, keep previous name
            //TODO: keep number format instead!
            if ( !(pArray[nPos].Flags & sheet::MemberResultFlags::CONTINUE) )
                aName = pArray[nPos].Name;

            sal_uInt32 nFormat = 0;
            for (size_t i=0; i<aDataFormats.size(); i++)
                if (aName == aDataNames[i])         //TODO: search more efficiently?
                {
                    nFormat = aDataFormats[i];
                    break;
                }
            pNumFmt[nPos] = nFormat;
        }
    }

    rFormats.reset( pNumFmt );
    rCount = nSize;
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
                    xDimProp, SC_UNO_DP_ORIENTATION,
                    sheet::DataPilotFieldOrientation_HIDDEN );
            if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                long nFormat = ScUnoHelpFunctions::GetLongProperty(
                                        xDimProp,
                                        SC_UNONAME_NUMFMT );

                return nFormat;     // use format from first found data dimension
            }
        }
    }

    return 0;       // none found
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

/**
 * Get visible page dimension members as results, except that, if all
 * members are visible, then this function returns empty result.
 */
uno::Sequence<sheet::MemberResult> getVisiblePageMembersAsResults( const uno::Reference<uno::XInterface>& xLevel )
{
    if (!xLevel.is())
        return uno::Sequence<sheet::MemberResult>();

    uno::Reference<sheet::XMembersSupplier> xMSupplier(xLevel, UNO_QUERY);
    if (!xMSupplier.is())
        return uno::Sequence<sheet::MemberResult>();

    uno::Reference<sheet::XMembersAccess> xNA = xMSupplier->getMembers();
    if (!xNA.is())
        return uno::Sequence<sheet::MemberResult>();

    std::vector<sheet::MemberResult> aRes;
    uno::Sequence<OUString> aNames = xNA->getElementNames();
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
    {
        const OUString& rName = aNames[i];
        xNA->getByName(rName);

        uno::Reference<beans::XPropertySet> xMemPS(xNA->getByName(rName), UNO_QUERY);
        if (!xMemPS.is())
            continue;

        OUString aCaption = ScUnoHelpFunctions::GetStringProperty(xMemPS, SC_UNO_DP_LAYOUTNAME, OUString());
        if (aCaption.isEmpty())
            aCaption = rName;

        bool bVisible = ScUnoHelpFunctions::GetBoolProperty(xMemPS, SC_UNO_DP_ISVISIBLE);

        if (bVisible)
        {
            /* TODO: any numeric value to obtain? */
            double fValue;
            rtl::math::setNan(&fValue);
            aRes.emplace_back(rName, aCaption, 0, fValue);
        }
    }

    if (aNames.getLength() == static_cast<sal_Int32>(aRes.size()))
        // All members are visible.  Return empty result.
        return uno::Sequence<sheet::MemberResult>();

    return ScUnoHelpFunctions::VectorToSequence(aRes);
}

}

ScDPOutput::ScDPOutput( ScDocument* pD, const uno::Reference<sheet::XDimensionsSupplier>& xSrc,
                        const ScAddress& rPos, bool bFilter ) :
    pDoc( pD ),
    xSource( xSrc ),
    aStartPos( rPos ),
    pColNumFmt( nullptr ),
    pRowNumFmt( nullptr ),
    nColFmtCount( 0 ),
    nRowFmtCount( 0 ),
    nSingleNumFmt( 0 ),
    nColCount(0),
    nRowCount(0),
    nHeaderSize(0),
    bDoFilter(bFilter),
    bResultsError(false),
    bSizesValid(false),
    bSizeOverflow(false),
    mbHeaderLayout(false)
{
    nTabStartCol = nMemberStartCol = nDataStartCol = nTabEndCol = 0;
    nTabStartRow = nMemberStartRow = nDataStartRow = nTabEndRow = 0;

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
                        xDimProp, SC_UNO_DP_ORIENTATION,
                        sheet::DataPilotFieldOrientation_HIDDEN );
                long nDimPos = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                        SC_UNO_DP_POSITION );
                bool bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, SC_UNO_DP_ISDATALAYOUT);
                bool bHasHiddenMember = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, SC_UNO_DP_HAS_HIDDEN_MEMBER);
                sal_Int32 nNumFmt = ScUnoHelpFunctions::GetLongProperty(
                    xDimProp, SC_UNO_DP_NUMBERFO);

                if ( eDimOrient != sheet::DataPilotFieldOrientation_HIDDEN )
                {
                    uno::Reference<container::XIndexAccess> xHiers =
                            new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                    long nHierarchy = ScUnoHelpFunctions::GetLongProperty(
                                            xDimProp,
                                            SC_UNO_DP_USEDHIERARCHY );
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
                                OUString aName = xLevNam->getName();
                                Reference<XPropertySet> xPropSet(xLevel, UNO_QUERY);
                                // Caption equals the field name by default.
                                // #i108948# use ScUnoHelpFunctions::GetStringProperty, because
                                // LayoutName is new and may not be present in external implementation
                                OUString aCaption = ScUnoHelpFunctions::GetStringProperty( xPropSet,
                                    SC_UNO_DP_LAYOUTNAME, aName );

                                /* TODO: any numeric value to obtain? */
                                double fValue;
                                rtl::math::setNan(&fValue);

                                switch ( eDimOrient )
                                {
                                    case sheet::DataPilotFieldOrientation_COLUMN:
                                    {
                                        uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();
                                        if (!lcl_MemberEmpty(aResult))
                                        {
                                            ScDPOutLevelData tmp(nDim, nHierarchy, nLev, nDimPos, nNumFmt, aResult, aName,
                                                                   aCaption, fValue, bHasHiddenMember, bIsDataLayout, false);
                                            pColFields.push_back(tmp);
                                        }
                                    }
                                    break;
                                    case sheet::DataPilotFieldOrientation_ROW:
                                    {
                                        uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();
                                        if (!lcl_MemberEmpty(aResult))
                                        {
                                            ScDPOutLevelData tmp(nDim, nHierarchy, nLev, nDimPos, nNumFmt, aResult, aName,
                                                                   aCaption, fValue, bHasHiddenMember, bIsDataLayout, false);
                                            pRowFields.push_back(tmp);
                                        }
                                    }
                                    break;
                                    case sheet::DataPilotFieldOrientation_PAGE:
                                    {
                                        uno::Sequence<sheet::MemberResult> aResult = getVisiblePageMembersAsResults(xLevel);
                                        // no check on results for page fields
                                        ScDPOutLevelData tmp(nDim, nHierarchy, nLev, nDimPos, nNumFmt, aResult, aName,
                                                               aCaption, fValue, bHasHiddenMember, false, true);
                                        pPageFields.push_back(tmp);
                                    }
                                    break;
                                    default:
                                    {
                                        // added to avoid warnings
                                    }
                                }

                                // get number formats from data dimensions
                                if ( bIsDataLayout )
                                {
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
        std::sort(pColFields.begin(), pColFields.end(), ScDPOutLevelDataComparator());
        std::sort(pRowFields.begin(), pRowFields.end(), ScDPOutLevelDataComparator());
        std::sort(pPageFields.begin(), pPageFields.end(), ScDPOutLevelDataComparator());

        //  get data results:

        try
        {
            aData = xResult->getResults();
        }
        catch (const uno::RuntimeException&)
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
            uno::Any aAny = xSrcProp->getPropertyValue( SC_UNO_DP_DATADESC );
            OUString aUStr;
            aAny >>= aUStr;
            aDataDescription = aUStr;
        }
        catch(const uno::Exception&)
        {
        }
    }
}

ScDPOutput::~ScDPOutput()
{
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
        pDoc->SetError( nCol, nRow, nTab, FormulaError::NoValue );
    }
    else if ( nFlags & sheet::DataResultFlags::HASDATA )
    {
        pDoc->SetValue( nCol, nRow, nTab, rData.Value );

        //  use number formats from source

        OSL_ENSURE( bSizesValid, "DataCell: !bSizesValid" );
        sal_uInt32 nFormat = 0;
        bool bApplyFormat = false;
        if ( pColNumFmt )
        {
            if ( nCol >= nDataStartCol )
            {
                long nIndex = nCol - nDataStartCol;
                if ( nIndex < nColFmtCount )
                {
                    nFormat = pColNumFmt[nIndex];
                    bApplyFormat = true;
                }
            }
        }
        else if ( pRowNumFmt )
        {
            if ( nRow >= nDataStartRow )
            {
                long nIndex = nRow - nDataStartRow;
                if ( nIndex < nRowFmtCount )
                {
                    nFormat = pRowNumFmt[nIndex];
                    bApplyFormat = true;
                }
            }
        }
        else if ( nSingleNumFmt != 0 )
        {
            nFormat = nSingleNumFmt;        // single format is used everywhere
            bApplyFormat = true;
        }

        if (bApplyFormat)
            pDoc->ApplyAttr(nCol, nRow, nTab, SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
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
        if (bNumeric && rtl::math::isFinite( rData.Value))
        {
            pDoc->SetValue( nCol, nRow, nTab, rData.Value);
        }
        else
        {
            ScSetStringParam aParam;
            if (bNumeric)
                aParam.setNumericInput();
            else
                aParam.setTextInput();

            pDoc->SetString(nCol, nRow, nTab, rData.Caption, &aParam);
        }
    }

    if ( nFlags & sheet::MemberResultFlags::SUBTOTAL )
    {
        ScDPOutputImpl outputimp( pDoc, nTab,
            nTabStartCol, nTabStartRow,
            nDataStartCol, nDataStartRow, nTabEndCol, nTabEndRow );
        //TODO: limit frames to horizontal or vertical?
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

    // For field button drawing
    ScMF nMergeFlag = ScMF::NONE;
    if (rData.mbHasHiddenMember)
        nMergeFlag |= ScMF::HiddenMember;

    if (rData.mbPageDim)
    {
        nMergeFlag |= ScMF::ButtonPopup;
        pDoc->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, ScMF::Button);
        pDoc->ApplyFlagsTab(nCol+1, nRow, nCol+1, nRow, nTab, nMergeFlag);
    }
    else
    {
        nMergeFlag |= ScMF::Button;
        if (!rData.mbDataLayout)
            nMergeFlag |= ScMF::ButtonPopup;
        pDoc->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, nMergeFlag);
    }

    lcl_SetStyleById( pDoc,nTab, nCol,nRow, nCol,nRow, STR_PIVOT_STYLE_FIELDNAME );
}

static void lcl_DoFilterButton( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    pDoc->SetString( nCol, nRow, nTab, ScGlobal::GetRscString(STR_CELL_FILTER) );
    pDoc->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, ScMF::Button);
}

void ScDPOutput::CalcSizes()
{
    if (!bSizesValid)
    {
        //  get column size of data from first row
        //TODO: allow different sizes (and clear following areas) ???

        nRowCount = aData.getLength();
        const uno::Sequence<sheet::DataResult>* pRowAry = aData.getConstArray();
        nColCount = nRowCount ? ( pRowAry[0].getLength() ) : 0;

        nHeaderSize = 1;
        if (GetHeaderLayout() && pColFields.empty())
            // Insert an extra header row only when there is no column field.
            nHeaderSize = 2;

        //  calculate output positions and sizes

        long nPageSize = 0;     // use page fields!
        if ( bDoFilter || pPageFields.size() )
        {
            nPageSize += pPageFields.size() + 1;   // plus one empty row
            if ( bDoFilter )
                ++nPageSize;        //  filter button above the page fields
        }

        if ( aStartPos.Col() + static_cast<long>(pRowFields.size()) + nColCount - 1 > MAXCOL ||
             aStartPos.Row() + nPageSize + nHeaderSize + pColFields.size() + nRowCount > MAXROW )
        {
            bSizeOverflow = true;
        }

        nTabStartCol = aStartPos.Col();
        nTabStartRow = aStartPos.Row() + (SCROW)nPageSize;          // below page fields
        nMemberStartCol = nTabStartCol;
        nMemberStartRow = nTabStartRow + (SCROW) nHeaderSize;
        nDataStartCol = nMemberStartCol + (SCCOL)pRowFields.size();
        nDataStartRow = nMemberStartRow + (SCROW)pColFields.size();
        if ( nColCount > 0 )
            nTabEndCol = nDataStartCol + (SCCOL)nColCount - 1;
        else
            nTabEndCol = nDataStartCol;     // single column will remain empty
        // if page fields are involved, include the page selection cells
        if ( !pPageFields.empty() && nTabEndCol < nTabStartCol + 1 )
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
    SCTAB nTab = aStartPos.Tab();
    const uno::Sequence<sheet::DataResult>* pRowAry = aData.getConstArray();

    //  calculate output positions and sizes

    CalcSizes();
    if ( bSizeOverflow || bResultsError )   // does output area exceed sheet limits?
        return;                             // nothing

    //  clear whole (new) output area
    // when modifying table, clear old area !
    //TODO: include InsertDeleteFlags::OBJECTS ???
    pDoc->DeleteAreaTab( aStartPos.Col(), aStartPos.Row(), nTabEndCol, nTabEndRow, nTab, InsertDeleteFlags::ALL );

    if ( bDoFilter )
        lcl_DoFilterButton( pDoc, aStartPos.Col(), aStartPos.Row(), nTab );

    //  output page fields:

    for (size_t nField=0; nField<pPageFields.size(); ++nField)
    {
        SCCOL nHdrCol = aStartPos.Col();
        SCROW nHdrRow = aStartPos.Row() + nField + ( bDoFilter ? 1 : 0 );
        // draw without frame for consistency with filter button:
        FieldCell(nHdrCol, nHdrRow, nTab, pPageFields[nField], false);
        SCCOL nFldCol = nHdrCol + 1;

        OUString aPageValue = ScResId(SCSTR_ALL);
        const uno::Sequence<sheet::MemberResult>& rRes = pPageFields[nField].maResult;
        sal_Int32 n = rRes.getLength();
        if (n == 1)
        {
            if (rRes[0].Caption.isEmpty())
                aPageValue = ScGlobal::GetRscString(STR_EMPTYDATA);
            else
                aPageValue = rRes[0].Caption;
        }
        else if (n > 1)
            aPageValue = ScResId(SCSTR_MULTIPLE);

        ScSetStringParam aParam;
        aParam.setTextInput();
        pDoc->SetString(nFldCol, nHdrRow, nTab, aPageValue, &aParam);

        lcl_SetFrame( pDoc,nTab, nFldCol,nHdrRow, nFldCol,nHdrRow, 20 );
    }

    //  data description
    //  (may get overwritten by first row field)

    if (aDataDescription.isEmpty())
    {
        //TODO: use default string ("result") ?
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
        nTabStartCol, nTabStartRow,
        nDataStartCol, nDataStartRow, nTabEndCol, nTabEndRow );
    for (size_t nField=0; nField<pColFields.size(); nField++)
    {
        SCCOL nHdrCol = nDataStartCol + (SCCOL)nField;              //TODO: check for overflow
        FieldCell(nHdrCol, nTabStartRow, nTab, pColFields[nField], true);

        SCROW nRowPos = nMemberStartRow + (SCROW)nField;                //TODO: check for overflow
        const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisColCount = rSequence.getLength();
        OSL_ENSURE( nThisColCount == nColCount, "count mismatch" );     //TODO: ???
        for (long nCol=0; nCol<nThisColCount; nCol++)
        {
            SCCOL nColPos = nDataStartCol + (SCCOL)nCol;                //TODO: check for overflow
            HeaderCell( nColPos, nRowPos, nTab, pArray[nCol], true, nField );
            if ( ( pArray[nCol].Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                !( pArray[nCol].Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                long nEnd = nCol;
                while ( nEnd+1 < nThisColCount && ( pArray[nEnd+1].Flags & sheet::MemberResultFlags::CONTINUE ) )
                    ++nEnd;
                SCCOL nEndColPos = nDataStartCol + (SCCOL)nEnd;     //TODO: check for overflow
                if ( nField+1 < pColFields.size())
                {
                    if ( nField == pColFields.size() - 2 )
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

            // Apply the same number format as in data source.
            pDoc->ApplyAttr(nColPos, nRowPos, nTab, SfxUInt32Item(ATTR_VALUE_FORMAT, pColFields[nField].mnSrcNumFmt));
        }
        if ( nField== 0 && pColFields.size() == 1 )
            outputimp.OutputBlockFrame( nDataStartCol,nTabStartRow, nTabEndCol,nRowPos-1 );
    }

    //  output row headers:
    std::vector<bool> vbSetBorder;
    vbSetBorder.resize( nTabEndRow - nDataStartRow + 1, false );
    for (size_t nField=0; nField<pRowFields.size(); nField++)
    {
        SCCOL nHdrCol = nTabStartCol + (SCCOL)nField;                   //TODO: check for overflow
        SCROW nHdrRow = nDataStartRow - 1;
        FieldCell(nHdrCol, nHdrRow, nTab, pRowFields[nField], true);

        SCCOL nColPos = nMemberStartCol + (SCCOL)nField;                //TODO: check for overflow
        const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        long nThisRowCount = rSequence.getLength();
        OSL_ENSURE( nThisRowCount == nRowCount, "count mismatch" );     //TODO: ???
        for (long nRow=0; nRow<nThisRowCount; nRow++)
        {
            SCROW nRowPos = nDataStartRow + (SCROW)nRow;                //TODO: check for overflow
            HeaderCell( nColPos, nRowPos, nTab, pArray[nRow], false, nField );
            if ( ( pArray[nRow].Flags & sheet::MemberResultFlags::HASMEMBER ) &&
                !( pArray[nRow].Flags & sheet::MemberResultFlags::SUBTOTAL ) )
            {
                if ( nField+1 < pRowFields.size() )
                {
                    long nEnd = nRow;
                    while ( nEnd+1 < nThisRowCount && ( pArray[nEnd+1].Flags & sheet::MemberResultFlags::CONTINUE ) )
                        ++nEnd;
                    SCROW nEndRowPos = nDataStartRow + (SCROW)nEnd;     //TODO: check for overflow
                    outputimp.AddRow( nRowPos );
                    if ( !vbSetBorder[ nRow ] )
                    {
                        outputimp.OutputBlockFrame( nColPos, nRowPos, nTabEndCol, nEndRowPos );
                        vbSetBorder[ nRow ]  = true;
                    }
                    outputimp.OutputBlockFrame( nColPos, nRowPos, nColPos, nEndRowPos );

                    if ( nField == pRowFields.size() - 2 )
                        outputimp.OutputBlockFrame( nColPos+1, nRowPos, nColPos+1, nEndRowPos );

                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nDataStartCol-1,nEndRowPos, STR_PIVOT_STYLE_CATEGORY );
                }
                else
                    lcl_SetStyleById( pDoc, nTab, nColPos,nRowPos, nDataStartCol-1,nRowPos, STR_PIVOT_STYLE_CATEGORY );
            }
            else if (  pArray[nRow].Flags & sheet::MemberResultFlags::SUBTOTAL )
                outputimp.AddRow( nRowPos );

            // Apply the same number format as in data source.
            pDoc->ApplyAttr(nColPos, nRowPos, nTab, SfxUInt32Item(ATTR_VALUE_FORMAT, pRowFields[nField].mnSrcNumFmt));
        }
    }

    if (nColCount == 1 && nRowCount > 0 && pColFields.empty())
    {
        // the table contains exactly one data field and no column fields.
        // Display data description at top right corner.
        ScSetStringParam aParam;
        aParam.setTextInput();
        pDoc->SetString(nDataStartCol, nDataStartRow-1, nTab, aDataDescription, &aParam);
    }

    //  output data results:

    for (long nRow=0; nRow<nRowCount; nRow++)
    {
        SCROW nRowPos = nDataStartRow + (SCROW)nRow;                    //TODO: check for overflow
        const sheet::DataResult* pColAry = pRowAry[nRow].getConstArray();
        long nThisColCount = pRowAry[nRow].getLength();
        OSL_ENSURE( nThisColCount == nColCount, "count mismatch" );     //TODO: ???
        for (long nCol=0; nCol<nThisColCount; nCol++)
        {
            SCCOL nColPos = nDataStartCol + (SCCOL)nCol;                //TODO: check for overflow
            DataCell( nColPos, nRowPos, nTab, pColAry[nCol] );
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
    return pPageFields.size() + ( bDoFilter ? 1 : 0 );
}

void ScDPOutput::GetMemberResultNames(ScDPUniqueStringSet& rNames, long nDimension)
{
    //  Return the list of all member names in a dimension's MemberResults.
    //  Only the dimension has to be compared because this is only used with table data,
    //  where each dimension occurs only once.

    uno::Sequence<sheet::MemberResult> aMemberResults;
    bool bFound = false;

    // look in column fields

    for (size_t nField=0; nField<pColFields.size() && !bFound; nField++)
        if ( pColFields[nField].mnDim == nDimension )
        {
            aMemberResults = pColFields[nField].maResult;
            bFound = true;
        }

    // look in row fields

    for (size_t nField=0; nField<pRowFields.size() && !bFound; nField++)
        if ( pRowFields[nField].mnDim == nDimension )
        {
            aMemberResults = pRowFields[nField].maResult;
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

namespace {

void lcl_GetTableVars( sal_Int32& rGrandTotalCols, sal_Int32& rGrandTotalRows, sal_Int32& rDataLayoutIndex,
                       std::vector<OUString>& rDataNames, std::vector<OUString>& rGivenNames,
                       sheet::DataPilotFieldOrientation& rDataOrient,
                       const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
    rDataLayoutIndex = -1;  // invalid
    rGrandTotalCols = 0;
    rGrandTotalRows = 0;
    rDataOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    uno::Reference<beans::XPropertySet> xSrcProp( xSource, uno::UNO_QUERY );
    bool bColGrand = ScUnoHelpFunctions::GetBoolProperty(
        xSrcProp, SC_UNO_DP_COLGRAND);
    if ( bColGrand )
        rGrandTotalCols = 1;    // default if data layout not in columns

    bool bRowGrand = ScUnoHelpFunctions::GetBoolProperty(
        xSrcProp, SC_UNO_DP_ROWGRAND);
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
                        xDimProp, SC_UNO_DP_ORIENTATION,
                        sheet::DataPilotFieldOrientation_HIDDEN );
                if ( ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                                         SC_UNO_DP_ISDATALAYOUT ) )
                {
                    rDataLayoutIndex = nDim;
                    rDataOrient = eDimOrient;
                }
                if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
                {
                    OUString aSourceName;
                    OUString aGivenName;
                    ScDPOutput::GetDataDimensionNames( aSourceName, aGivenName, xDim );
                    try
                    {
                        uno::Any aValue = xDimProp->getPropertyValue( SC_UNO_DP_LAYOUTNAME );

                        if( aValue.hasValue() )
                        {
                            OUString strLayoutName;

                            if( ( aValue >>= strLayoutName ) && !strLayoutName.isEmpty() )
                                aGivenName = strLayoutName;
                        }
                    }
                    catch(const uno::Exception&)
                    {
                    }
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
                                            SC_UNO_DP_DATAFIELDCOUNT );
                if (nDataFieldCount > 0)
                    aResData.DataFieldIndex = (nRow - nDataStartRow) % nDataFieldCount;
            }

            // Copy appropriate DataResult object from the cached sheet::DataResult table.
            if (aData.getLength() > nRow - nDataStartRow &&
                aData[nRow-nDataStartRow].getLength() > nCol-nDataStartCol)
                aResData.Result = aData[nRow-nDataStartRow][nCol-nDataStartCol];

            rPosData.PositionData <<= aResData;
            return;
        }
        case DataPilotTablePositionType::COLUMN_HEADER:
        {
            long nField = nRow - nTabStartRow - 1; // 1st line is used for the buttons
            if (nField < 0)
                break;

            const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nField].maResult;
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
            aHeaderData.MemberName = pArray[nItem].Name;
            aHeaderData.Flags = pArray[nItem].Flags;
            aHeaderData.Dimension = static_cast<sal_Int32>(pColFields[nField].mnDim);
            aHeaderData.Hierarchy = static_cast<sal_Int32>(pColFields[nField].mnHier);
            aHeaderData.Level     = static_cast<sal_Int32>(pColFields[nField].mnLevel);

            rPosData.PositionData <<= aHeaderData;
            return;
        }
        case DataPilotTablePositionType::ROW_HEADER:
        {
            long nField = nCol - nTabStartCol;
            if (nField < 0)
                break;

            const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nField].maResult;
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
            aHeaderData.MemberName = pArray[nItem].Name;
            aHeaderData.Flags = pArray[nItem].Flags;
            aHeaderData.Dimension = static_cast<sal_Int32>(pRowFields[nField].mnDim);
            aHeaderData.Hierarchy = static_cast<sal_Int32>(pRowFields[nField].mnHier);
            aHeaderData.Level     = static_cast<sal_Int32>(pRowFields[nField].mnLevel);

            rPosData.PositionData <<= aHeaderData;
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
                                SC_UNO_DP_DATAFIELDCOUNT );
    if (nDataFieldCount == 0)
        // No data field is present in this datapilot table.
        return false;

    // #i111421# use lcl_GetTableVars for correct size of totals and data layout position
    sal_Int32 nGrandTotalCols;
    sal_Int32 nGrandTotalRows;
    sal_Int32 nDataLayoutIndex;
    std::vector<OUString> aDataNames;
    std::vector<OUString> aGivenNames;
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
    for (size_t nColField = 0; nColField < pColFields.size() && bFilterByCol; ++nColField)
    {
        if (pColFields[nColField].mnDim == nDataLayoutIndex)
            // There is no sense including the data layout field for filtering.
            continue;

        sheet::DataPilotFieldFilter filter;
        filter.FieldName = pColFields[nColField].maName;

        const uno::Sequence<sheet::MemberResult> rSequence = pColFields[nColField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();

        OSL_ENSURE(nDataStartCol + rSequence.getLength() - 1 == nTabEndCol, "ScDPOutput::GetDataFieldCellData: error in geometric assumption");

        long nItem = nCol - nDataStartCol;
                //  get origin of "continue" fields
        while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
            --nItem;

        filter.MatchValueName = pArray[nItem].Name;
        rFilters.push_back(filter);
    }

    // row fields
    for (size_t nRowField = 0; nRowField < pRowFields.size() && bFilterByRow; ++nRowField)
    {
        if (pRowFields[nRowField].mnDim == nDataLayoutIndex)
            // There is no sense including the data layout field for filtering.
            continue;

        sheet::DataPilotFieldFilter filter;
        filter.FieldName = pRowFields[nRowField].maName;

        const uno::Sequence<sheet::MemberResult> rSequence = pRowFields[nRowField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();

        OSL_ENSURE(nDataStartRow + rSequence.getLength() - 1 == nTabEndRow, "ScDPOutput::GetDataFieldCellData: error in geometric assumption");

        long nItem = nRow - nDataStartRow;
            //  get origin of "continue" fields
        while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
            --nItem;

        filter.MatchValueName = pArray[nItem].Name;
        rFilters.push_back(filter);
    }

    return true;
}

namespace {

OUString lcl_GetDataFieldName( const OUString& rSourceName, sal_Int16 eFunc )
{
    const char* pStrId = nullptr;
    switch ( eFunc )
    {
        case sheet::GeneralFunction2::SUM:        pStrId = STR_FUN_TEXT_SUM;      break;
        case sheet::GeneralFunction2::COUNT:
        case sheet::GeneralFunction2::COUNTNUMS:  pStrId = STR_FUN_TEXT_COUNT;    break;
        case sheet::GeneralFunction2::AVERAGE:    pStrId = STR_FUN_TEXT_AVG;      break;
        case sheet::GeneralFunction2::MEDIAN:     pStrId = STR_FUN_TEXT_MEDIAN;   break;
        case sheet::GeneralFunction2::MAX:        pStrId = STR_FUN_TEXT_MAX;      break;
        case sheet::GeneralFunction2::MIN:        pStrId = STR_FUN_TEXT_MIN;      break;
        case sheet::GeneralFunction2::PRODUCT:    pStrId = STR_FUN_TEXT_PRODUCT;  break;
        case sheet::GeneralFunction2::STDEV:
        case sheet::GeneralFunction2::STDEVP:     pStrId = STR_FUN_TEXT_STDDEV;   break;
        case sheet::GeneralFunction2::VAR:
        case sheet::GeneralFunction2::VARP:       pStrId = STR_FUN_TEXT_VAR;      break;
        case sheet::GeneralFunction2::NONE:
        case sheet::GeneralFunction2::AUTO:                                       break;
        default:
        {
            assert(false);
        }
    }
    if (!pStrId)
        return OUString();

    OUStringBuffer aRet(ScGlobal::GetRscString(pStrId));
    aRet.append(" - ");
    aRet.append(rSourceName);
    return aRet.makeStringAndClear();
}

}

void ScDPOutput::GetDataDimensionNames(
    OUString& rSourceName, OUString& rGivenName, const uno::Reference<uno::XInterface>& xDim )
{
    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
    if ( xDimProp.is() && xDimName.is() )
    {
        // Asterisks are added in ScDPSaveData::WriteToSource to create unique names.
        //TODO: preserve original name there?
        rSourceName = ScDPUtil::getSourceDimensionName(xDimName->getName());

        // Generate "given name" the same way as in dptabres.
        //TODO: Should use a stored name when available

        sal_Int16 eFunc = ScUnoHelpFunctions::GetShortProperty(
                          xDimProp, SC_UNO_DP_FUNCTION2,
                          sheet::GeneralFunction2::NONE );
        rGivenName = lcl_GetDataFieldName( rSourceName, eFunc );
    }
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

long ScDPOutput::GetHeaderDim( const ScAddress& rPos, sheet::DataPilotFieldOrientation& rOrient )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != aStartPos.Tab() )
        return -1;                                      // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nRow == nTabStartRow && nCol >= nDataStartCol && static_cast<size_t>(nCol) < nDataStartCol + pColFields.size())
    {
        rOrient = sheet::DataPilotFieldOrientation_COLUMN;
        long nField = nCol - nDataStartCol;
        return pColFields[nField].mnDim;
    }

    //  test for row header

    if ( nRow+1 == nDataStartRow && nCol >= nTabStartCol && static_cast<size_t>(nCol) < nTabStartCol + pRowFields.size() )
    {
        rOrient = sheet::DataPilotFieldOrientation_ROW;
        long nField = nCol - nTabStartCol;
        return pRowFields[nField].mnDim;
    }

    //  test for page field

    SCROW nPageStartRow = aStartPos.Row() + ( bDoFilter ? 1 : 0 );
    if ( nCol == aStartPos.Col() && nRow >= nPageStartRow && static_cast<size_t>(nRow) < nPageStartRow + pPageFields.size() )
    {
        rOrient = sheet::DataPilotFieldOrientation_PAGE;
        long nField = nRow - nPageStartRow;
        return pPageFields[nField].mnDim;
    }

    //TODO: single data field (?)

    rOrient = sheet::DataPilotFieldOrientation_HIDDEN;
    return -1;      // invalid
}

bool ScDPOutput::GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop,
                                long nDragDim,
                                tools::Rectangle& rPosRect, sheet::DataPilotFieldOrientation& rOrient, long& rDimPos )
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
            nRow + 1 >= nMemberStartRow && static_cast<size_t>(nRow) < nMemberStartRow + pColFields.size())
    {
        long nField = nRow - nMemberStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = true;
        }
        //TODO: find start of dimension

        rPosRect = tools::Rectangle( nDataStartCol, nMemberStartRow + nField,
                              nTabEndCol, nMemberStartRow + nField -1 );

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (long nPos=0; static_cast<size_t>(nPos)<pColFields.size() && !bFound; nPos++)
        {
            if (pColFields[nPos].mnDim == nDragDim)
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
                        pRowFields.empty() && nCol == nTabStartCol && bMouseLeft );

    if ( bSpecial || ( nRow+1 >= nDataStartRow && nRow <= nTabEndRow &&
                        nCol + 1 >= nTabStartCol && static_cast<size_t>(nCol) < nTabStartCol + pRowFields.size() ) )
    {
        long nField = nCol - nTabStartCol;
        //TODO: find start of dimension

        rPosRect = tools::Rectangle( nTabStartCol + nField, nDataStartRow - 1,
                              nTabStartCol + nField - 1, nTabEndRow );

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (long nPos=0; static_cast<size_t>(nPos)<pRowFields.size() && !bFound; nPos++)
        {
            if (pRowFields[nPos].mnDim == nDragDim)
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
            nRow + 1 >= nPageStartRow && static_cast<size_t>(nRow) < nPageStartRow + pPageFields.size() )
    {
        long nField = nRow - nPageStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = true;
        }
        //TODO: find start of dimension

        rPosRect = tools::Rectangle( aStartPos.Col(), nPageStartRow + nField,
                              nTabEndCol, nPageStartRow + nField - 1 );

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (long nPos=0; static_cast<size_t>(nPos)<pPageFields.size() && !bFound; nPos++)
        {
            if (pPageFields[nPos].mnDim == nDragDim)
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
