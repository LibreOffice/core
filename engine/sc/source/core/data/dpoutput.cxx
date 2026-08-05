/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/sequence.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <svl/itemset.hxx>

#include <dpoutput.hxx>
#include <dpobject.hxx>
#include <dociter.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <formula/errorcodes.hxx>
#include <miscuno.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <unonames.hxx>
#include <strings.hrc>
#include <stringutil.hxx>
#include <dputil.hxx>
#include <pivot/DPOutLevelData.hxx>
#include <pivot/StyleOutput.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTableResultData.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/GeneralFunction2.hpp>
#include <com/sun/star/sheet/MemberResult.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersAccess.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>

#include <limits>
#include <string_view>
#include <utility>
#include <vector>
#include <iostream>

using namespace com::sun::star;

namespace
{
struct ScDPOutLevelDataComparator
{
    bool operator()(const ScDPOutLevelData & rA, const ScDPOutLevelData & rB)
    {
        return rA.mnDimPos<rB.mnDimPos || ( rA.mnDimPos==rB.mnDimPos && rA.mnHier<rB.mnHier ) ||
        ( rA.mnDimPos==rB.mnDimPos && rA.mnHier==rB.mnHier && rA.mnLevel<rB.mnLevel );
    }
};
} // end anonymous namespace

namespace
{

void lcl_FillNumberFormats( std::unique_ptr<sal_uInt32[]>& rFormats, sal_Int32& rCount,
                            const uno::Reference<sheet::XDataPilotMemberResults>& xLevRes,
                            const uno::Reference<container::XIndexAccess>& xDims )
{
    if ( rFormats )
        return;                         // already set

    //  xLevRes is from the data layout dimension
    //TODO: use result sequence from ScDPOutLevelData!

    cpo::uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();

    tools::Long nSize = aResult.getLength();
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
        uno::Reference<uno::XInterface> xDim(xDims->getByIndex(nDim), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
        if ( xDimProp.is() && xDimName.is() )
        {
            sheet::DataPilotFieldOrientation eDimOrient =
                ScUnoHelpFunctions::GetEnumProperty(
                    xDimProp, SC_UNO_DP_ORIENTATION,
                    sheet::DataPilotFieldOrientation_HIDDEN );
            if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                aDataNames.push_back(xDimName->getName());
                tools::Long nFormat = ScUnoHelpFunctions::GetLongProperty(
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
        tools::Long nFormat = aDataFormats[0];
        for (tools::Long nPos=0; nPos<nSize; nPos++)
            pNumFmt[nPos] = nFormat;
    }
    else
    {
        for (tools::Long nPos=0; nPos<nSize; nPos++)
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
    tools::Long nDimCount = xDims->getCount();
    for (tools::Long nDim=0; nDim<nDimCount; nDim++)
    {
        uno::Reference<beans::XPropertySet> xDimProp(xDims->getByIndex(nDim), uno::UNO_QUERY);
        if ( xDimProp.is() )
        {
            sheet::DataPilotFieldOrientation eDimOrient =
                ScUnoHelpFunctions::GetEnumProperty(
                    xDimProp, SC_UNO_DP_ORIENTATION,
                    sheet::DataPilotFieldOrientation_HIDDEN );
            if ( eDimOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                tools::Long nFormat = ScUnoHelpFunctions::GetLongProperty(
                                        xDimProp,
                                        SC_UNONAME_NUMFMT );

                return nFormat;     // use format from first found data dimension
            }
        }
    }

    return 0;       // none found
}

bool lcl_MemberEmpty( const cpo::uno::Sequence<sheet::MemberResult>& rSeq )
{
    //  used to skip levels that have no members

    return std::none_of(rSeq.begin(), rSeq.end(),
        [](const sheet::MemberResult& rMem) {
            return rMem.Flags & sheet::MemberResultFlags::HASMEMBER; });
}

/**
 * Get visible page dimension members as results, except that, if all
 * members are visible, then this function returns empty result.
 */
cpo::uno::Sequence<sheet::MemberResult> getVisiblePageMembersAsResults( const uno::Reference<uno::XInterface>& xLevel )
{
    if (!xLevel.is())
        return cpo::uno::Sequence<sheet::MemberResult>();

    uno::Reference<sheet::XMembersSupplier> xMSupplier(xLevel, uno::UNO_QUERY);
    if (!xMSupplier.is())
        return cpo::uno::Sequence<sheet::MemberResult>();

    uno::Reference<sheet::XMembersAccess> xNA = xMSupplier->getMembers();
    if (!xNA.is())
        return cpo::uno::Sequence<sheet::MemberResult>();

    std::vector<sheet::MemberResult> aRes;
    const cpo::uno::Sequence<OUString> aNames = xNA->getElementNames();
    for (const OUString& rName : aNames)
    {
        xNA->getByName(rName);

        uno::Reference<beans::XPropertySet> xMemPS(xNA->getByName(rName), uno::UNO_QUERY);
        if (!xMemPS.is())
            continue;

        OUString aCaption = ScUnoHelpFunctions::GetStringProperty(xMemPS, SC_UNO_DP_LAYOUTNAME, OUString());
        if (aCaption.isEmpty())
            aCaption = rName;

        bool bVisible = ScUnoHelpFunctions::GetBoolProperty(xMemPS, SC_UNO_DP_ISVISIBLE);

        if (bVisible)
        {
            /* TODO: any numeric value to obtain? */
            aRes.emplace_back(rName, aCaption, 0, std::numeric_limits<double>::quiet_NaN());
        }
    }

    if (o3tl::make_unsigned(aNames.getLength()) == aRes.size())
        // All members are visible.  Return empty result.
        return cpo::uno::Sequence<sheet::MemberResult>();

    return comphelper::containerToSequence(aRes);
}

} // end anonymous namespace

ScDPOutput::ScDPOutput(ScDocument* pDocument, uno::Reference<sheet::XDimensionsSupplier> xSource,
                       const ScAddress& rPosition, bool bFilter, bool bExpandCollapse, ScDPObject& rObject, bool bHideHeader)
    : mpDocument(pDocument)
    , maFormatOutput(rObject)
    , mpStyleOutput(std::make_unique<sc::pivot::StyleOutput>(*pDocument))
    , mxSource(std::move(xSource))
    , maStartPos(rPosition)
    , mnColFormatCount(0)
    , mnRowFormatCount(0)
    , mnSingleNumberFormat(0)
    , mnRowDims(0)
    , mnColCount(0)
    , mnRowCount(0)
    , mnHeaderSize(0)
    , mbDoFilter(bFilter)
    , mbResultsError(false)
    , mbSizesValid(false)
    , mbSizeOverflow(false)
    , mbHeaderLayout(false)
    , mbHasCompactRowField(false)
    , mbExpandCollapse(bExpandCollapse)
    , mbHideHeader(bHideHeader)
{
    mnTabStartCol = mnMemberStartCol = mnDataStartCol = mnTabEndCol = 0;
    mnTabStartRow = mnMemberStartRow = mnDataStartRow = mnTabEndRow = 0;

    uno::Reference<sheet::XDataPilotResults> xResult(mxSource, uno::UNO_QUERY);
    if (mxSource.is() && xResult.is())
    {
        //  get dimension results:

        uno::Reference<container::XIndexAccess> xDims =
                new ScNameToIndexAccess(mxSource->getDimensions());
        tools::Long nDimCount = xDims->getCount();
        for (tools::Long nDim=0; nDim<nDimCount; nDim++)
        {
            uno::Reference<uno::XInterface> xDim(xDims->getByIndex(nDim), uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
            uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
            if ( xDimProp.is() && xDimSupp.is() )
            {
                sheet::DataPilotFieldOrientation eDimOrient =
                    ScUnoHelpFunctions::GetEnumProperty(
                        xDimProp, SC_UNO_DP_ORIENTATION,
                        sheet::DataPilotFieldOrientation_HIDDEN );
                tools::Long nDimPos = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                        SC_UNO_DP_POSITION );
                bool bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, SC_UNO_DP_ISDATALAYOUT);
                // TODO: wire up calculated field info to output
                bool bIsCalculatedField = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, SC_UNO_DP_CALCULATEDFIELD);
                std::optional<OUString> aCalculation;
                if (bIsCalculatedField)
                    aCalculation = ScUnoHelpFunctions::GetStringProperty(
                        xDimProp, SC_UNO_DP_CALCULATION, u""_ustr);
                bool bHasHiddenMember = ScUnoHelpFunctions::GetBoolProperty(
                    xDimProp, SC_UNO_DP_HAS_HIDDEN_MEMBER);
                sal_Int32 nNumFmt = ScUnoHelpFunctions::GetLongProperty(
                    xDimProp, SC_UNO_DP_NUMBERFO);

                if ( eDimOrient != sheet::DataPilotFieldOrientation_HIDDEN )
                {
                    uno::Reference<container::XIndexAccess> xHiers =
                            new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                    tools::Long nHierarchy = ScUnoHelpFunctions::GetLongProperty(
                                            xDimProp,
                                            SC_UNO_DP_USEDHIERARCHY );
                    if ( nHierarchy >= xHiers->getCount() )
                        nHierarchy = 0;

                    uno::Reference<sheet::XLevelsSupplier> xHierSupp(xHiers->getByIndex(nHierarchy),
                                                                     uno::UNO_QUERY);
                    if ( xHierSupp.is() )
                    {
                        uno::Reference<container::XIndexAccess> xLevels =
                                new ScNameToIndexAccess( xHierSupp->getLevels() );
                        tools::Long nLevCount = xLevels->getCount();
                        for (tools::Long nLev=0; nLev<nLevCount; nLev++)
                        {
                            uno::Reference<uno::XInterface> xLevel(xLevels->getByIndex(nLev),
                                                                   uno::UNO_QUERY);
                            uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                            uno::Reference<sheet::XDataPilotMemberResults> xLevRes(
                                    xLevel, uno::UNO_QUERY );
                            if ( xLevNam.is() && xLevRes.is() )
                            {
                                OUString aName = xLevNam->getName();
                                uno::Reference<beans::XPropertySet> xPropSet(xLevel, uno::UNO_QUERY);
                                // Caption equals the field name by default.
                                // #i108948# use ScUnoHelpFunctions::GetStringProperty, because
                                // LayoutName is new and may not be present in external implementation
                                OUString aCaption = ScUnoHelpFunctions::GetStringProperty( xPropSet,
                                    SC_UNO_DP_LAYOUTNAME, aName );

                                switch ( eDimOrient )
                                {
                                    case sheet::DataPilotFieldOrientation_COLUMN:
                                    {
                                        cpo::uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();
                                        if (!lcl_MemberEmpty(aResult))
                                        {
                                            mpColFields.emplace_back(nDim, nHierarchy, nLev, nDimPos, nNumFmt, aResult, aName,
                                                                     aCaption, bHasHiddenMember, bIsDataLayout, false);
                                        }
                                    }
                                    break;
                                    case sheet::DataPilotFieldOrientation_ROW:
                                    {
                                        cpo::uno::Sequence<sheet::MemberResult> aResult = xLevRes->getResults();
                                        ++mnRowDims;
                                        // We want only to remove the DATA column if it is empty
                                        // and not any other empty columns (to still show the
                                        // header columns)
                                        bool bSkip = lcl_MemberEmpty(aResult) && bIsDataLayout;
                                        if (!bSkip)
                                        {
                                            bool bFieldCompact = false;
                                            try
                                            {
                                                sheet::DataPilotFieldLayoutInfo aLayoutInfo;
                                                xPropSet->getPropertyValue( SC_UNO_DP_LAYOUT ) >>= aLayoutInfo;
                                                bFieldCompact = (aLayoutInfo.LayoutMode == sheet::DataPilotFieldLayoutMode::COMPACT_LAYOUT);
                                            }
                                            catch (cpo::uno::Exception&)
                                            {
                                            }
                                            mpRowFields.emplace_back(nDim, nHierarchy, nLev, nDimPos, nNumFmt, aResult, aName,
                                                                     aCaption, bHasHiddenMember, bIsDataLayout, false);
                                            maRowCompactFlags.push_back(bFieldCompact);
                                            mbHasCompactRowField |= bFieldCompact;
                                        }

                                    }
                                    break;
                                    case sheet::DataPilotFieldOrientation_PAGE:
                                    {
                                        cpo::uno::Sequence<sheet::MemberResult> aResult = getVisiblePageMembersAsResults(xLevel);
                                        // no check on results for page fields
                                        mpPageFields.emplace_back(nDim, nHierarchy, nLev, nDimPos, nNumFmt, aResult, aName,
                                                                  aCaption, bHasHiddenMember, false, true);
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
                                        lcl_FillNumberFormats(mpColNumberFormat, mnColFormatCount, xLevRes, xDims);
                                    else if ( eDimOrient == sheet::DataPilotFieldOrientation_ROW )
                                        lcl_FillNumberFormats(mpRowNumberFormat, mnRowFormatCount, xLevRes, xDims);
                                }
                            }
                        }
                    }
                }
                else if ( bIsDataLayout )
                {
                    // data layout dimension is hidden (allowed if there is only one data dimension)
                    // -> use the number format from the first data dimension for all results

                    mnSingleNumberFormat = lcl_GetFirstNumberFormat( xDims );
                }
            }
        }
        std::sort(mpColFields.begin(), mpColFields.end(), ScDPOutLevelDataComparator());
        std::sort(mpRowFields.begin(), mpRowFields.end(), ScDPOutLevelDataComparator());
        std::sort(mpPageFields.begin(), mpPageFields.end(), ScDPOutLevelDataComparator());

        //  get data results:

        try
        {
            maData = xResult->getResults();
        }
        catch (const uno::RuntimeException&)
        {
            mbResultsError = true;
        }
    }

    // get "DataDescription" property (may be missing in external sources)

    uno::Reference<beans::XPropertySet> xSrcProp(mxSource, uno::UNO_QUERY);
    if ( !xSrcProp.is() )
        return;

    try
    {
        cpo::uno::Any aAny = xSrcProp->getPropertyValue( SC_UNO_DP_DATADESC );
        OUString aUStr;
        aAny >>= aUStr;
        maDataDescription = aUStr;
    }
    catch(const cpo::uno::Exception&)
    {
    }
}

ScDPOutput::~ScDPOutput()
{
}

void ScDPOutput::SetPosition(const ScAddress& rPosition)
{
    maStartPos = rPosition;
    mbSizesValid = mbSizeOverflow = false;
}

void ScDPOutput::DataCell( SCCOL nCol, SCROW nRow, SCTAB nTab, const sheet::DataResult& rData )
{
    tools::Long nFlags = rData.Flags;
    if ( nFlags & sheet::DataResultFlags::ERROR )
    {
        mpDocument->SetError( nCol, nRow, nTab, FormulaError::NoValue );
    }
    else if ( nFlags & sheet::DataResultFlags::HASDATA )
    {
        mpDocument->SetValue( nCol, nRow, nTab, rData.Value );

        //  use number formats from source

        OSL_ENSURE(mbSizesValid, "DataCell: !bSizesValid");
        sal_uInt32 nFormat = 0;
        bool bApplyFormat = false;
        if (mpColNumberFormat)
        {
            if (nCol >= mnDataStartCol)
            {
                tools::Long nIndex = nCol - mnDataStartCol;
                if (nIndex < mnColFormatCount)
                {
                    nFormat = mpColNumberFormat[nIndex];
                    bApplyFormat = true;
                }
            }
        }
        else if (mpRowNumberFormat)
        {
            if (nRow >= mnDataStartRow)
            {
                tools::Long nIndex = nRow - mnDataStartRow;
                if (nIndex < mnRowFormatCount)
                {
                    nFormat = mpRowNumberFormat[nIndex];
                    bApplyFormat = true;
                }
            }
        }
        else if (mnSingleNumberFormat != 0)
        {
            nFormat = mnSingleNumberFormat;        // single format is used everywhere
            bApplyFormat = true;
        }

        if (bApplyFormat)
            mpDocument->ApplyAttr(nCol, nRow, nTab, SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    }
    //  SubTotal formatting is controlled by headers
}

void ScDPOutput::HeaderCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                             const sheet::MemberResult& rData, bool bColHeader, tools::Long nLevel )
{
    tools::Long nFlags = rData.Flags;

    if ( nFlags & sheet::MemberResultFlags::HASMEMBER )
    {
        bool bNumeric = (nFlags & sheet::MemberResultFlags::NUMERIC) != 0;
        if (bNumeric && std::isfinite( rData.Value))
        {
            mpDocument->SetValue( nCol, nRow, nTab, rData.Value);
        }
        else
        {
            ScSetStringParam aParam;
            if (bNumeric)
                aParam.setNumericInput();
            else
                aParam.setTextInput();

            mpDocument->SetString(nCol, nRow, nTab, rData.Caption, &aParam);
        }
    }

    if ( !(nFlags & sheet::MemberResultFlags::SUBTOTAL) )
        return;

    bool bGrandTotal = (nFlags & sheet::MemberResultFlags::GRANDTOTAL) != 0;
    if (bColHeader)
        mpStyleOutput->addSubtotalColumn(nCol, mnMemberStartRow + SCROW(nLevel), bGrandTotal);
    else
        mpStyleOutput->addSubtotalRow(nRow, mnMemberStartCol + SCCOL(nLevel), bGrandTotal);
}

void ScDPOutput::MultiFieldCell(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bRowField)
{
    mpDocument->SetString(nCol, nRow, nTab, ScResId(bRowField ? STR_PIVOT_ROW_LABELS : STR_PIVOT_COL_LABELS));

    ScMF nMergeFlag = ScMF::Button;
    for (auto& rData : mpRowFields)
    {
        if (rData.mbHasHiddenMember)
        {
            nMergeFlag |= ScMF::HiddenMember;
            break;
        }
    }

    nMergeFlag |= ScMF::ButtonPopup2;

    mpDocument->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, nMergeFlag);
    mpStyleOutput->addFieldCell(nCol, nRow, /*bFrame*/ false);
}

void ScDPOutput::FieldCell(
    SCCOL nCol, SCROW nRow, SCTAB nTab, const ScDPOutLevelData& rData, bool bInTable)
{
    // Avoid unwanted automatic format detection.
    ScSetStringParam aParam;
    aParam.mbDetectNumberFormat = false;
    aParam.meSetTextNumFormat = ScSetStringParam::Always;
    aParam.mbHandleApostrophe = false;
    mpDocument->SetString(nCol, nRow, nTab, rData.maCaption, &aParam);

    // For field button drawing
    ScMF nMergeFlag = ScMF::NONE;
    if (rData.mbHasHiddenMember)
        nMergeFlag |= ScMF::HiddenMember;

    if (rData.mbPageDim)
    {
        nMergeFlag |= ScMF::ButtonPopup;
        mpDocument->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, ScMF::Button);
        mpDocument->ApplyFlagsTab(nCol+1, nRow, nCol+1, nRow, nTab, nMergeFlag);
    }
    else
    {
        nMergeFlag |= ScMF::Button;
        if (!rData.mbDataLayout)
            nMergeFlag |= ScMF::ButtonPopup;
        mpDocument->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, nMergeFlag);
    }

    mpStyleOutput->addFieldCell(nCol, nRow, bInTable);
}

static void lcl_DoFilterButton( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    pDoc->SetString( nCol, nRow, nTab, ScResId(STR_CELL_FILTER) );
    pDoc->ApplyFlagsTab(nCol, nRow, nCol, nRow, nTab, ScMF::Button);
}

SCCOL ScDPOutput::GetColumnsForRowFields() const
{
    if (!mbHasCompactRowField)
        return static_cast<SCCOL>(mpRowFields.size());

    SCCOL nNum = 0;
    for (const auto bCompact: maRowCompactFlags)
        if (!bCompact)
            ++nNum;

    if (maRowCompactFlags.back())
        ++nNum;

    return nNum;
}

void ScDPOutput::CalcSizes()
{
    if (mbSizesValid)
        return;

    //  get column size of data from first row
    //TODO: allow different sizes (and clear following areas) ???

    mnRowCount = maData.getLength();
    const cpo::uno::Sequence<sheet::DataResult>* pRowAry = maData.getConstArray();
    mnColCount = mnRowCount ? ( pRowAry[0].getLength() ) : 0;

    mnHeaderSize = 1;
    if (mbHideHeader)
        mnHeaderSize = 0;
    else if (GetHeaderLayout() && mpColFields.empty())
        // Insert an extra header row only when there is no column field.
        mnHeaderSize = 2;

    //  calculate output positions and sizes

    tools::Long nPageSize = 0;     // use page fields!
    if (mbDoFilter || !mpPageFields.empty())
    {
        nPageSize += mpPageFields.size() + 1;   // plus one empty row
        if (mbDoFilter)
            ++nPageSize;        //  filter button above the page fields
    }

    if (maStartPos.Col() + static_cast<tools::Long>(mpRowFields.size()) + mnColCount - 1 > mpDocument->MaxCol() ||
        maStartPos.Row() + nPageSize + mnHeaderSize + static_cast<tools::Long>(mpColFields.size()) + mnRowCount > mpDocument->MaxRow())
    {
        mbSizeOverflow = true;
    }

    mnTabStartCol = maStartPos.Col();
    mnTabStartRow = maStartPos.Row() + static_cast<SCROW>(nPageSize);          // below page fields
    mnMemberStartCol = mnTabStartCol;
    mnMemberStartRow = mnTabStartRow + static_cast<SCROW>(mnHeaderSize);
    mnDataStartCol = mnMemberStartCol + GetColumnsForRowFields();
    mnDataStartRow = mnMemberStartRow + static_cast<SCROW>(mpColFields.size());
    if (mnColCount > 0)
        mnTabEndCol = mnDataStartCol + static_cast<SCCOL>(mnColCount) - 1;
    else
        mnTabEndCol = mnDataStartCol;     // single column will remain empty
    // if page fields are involved, include the page selection cells
    if (!mpPageFields.empty() && mnTabEndCol < mnTabStartCol + 1)
        mnTabEndCol = mnTabStartCol + 1;
    if (mnRowCount > 0)
        mnTabEndRow = mnDataStartRow + static_cast<SCROW>(mnRowCount) - 1;
    else
        mnTabEndRow = mnDataStartRow;     // single row will remain empty
    mbSizesValid = true;
}

sal_Int32 ScDPOutput::GetPositionType(const ScAddress& rPos)
{
    using namespace ::com::sun::star::sheet;

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != maStartPos.Tab() )
        return DataPilotTablePositionType::NOT_IN_TABLE;

    CalcSizes();

    // Make sure the cursor is within the table.
    if (nCol < mnTabStartCol || nRow < mnTabStartRow || nCol > mnTabEndCol || nRow > mnTabEndRow)
        return DataPilotTablePositionType::NOT_IN_TABLE;

    // test for result data area.
    if (nCol >= mnDataStartCol && nCol <= mnTabEndCol && nRow >= mnDataStartRow && nRow <= mnTabEndRow)
        return DataPilotTablePositionType::RESULT;

    bool bInColHeader = (nRow >= mnTabStartRow && nRow < mnDataStartRow);
    bool bInRowHeader = (nCol >= mnTabStartCol && nCol < mnDataStartCol);

    if (bInColHeader && bInRowHeader)
        // probably in that ugly little box at the upper-left corner of the table.
        return DataPilotTablePositionType::OTHER;

    if (bInColHeader)
    {
        if (nRow == mnTabStartRow)
            // first row in the column header area is always used for column
            // field buttons.
            return DataPilotTablePositionType::OTHER;

        return DataPilotTablePositionType::COLUMN_HEADER;
    }

    if (bInRowHeader)
        return DataPilotTablePositionType::ROW_HEADER;

    return DataPilotTablePositionType::OTHER;
}

void ScDPOutput::outputPageFields(SCTAB nTab)
{
    for (size_t nField = 0; nField < mpPageFields.size(); ++nField)
    {
        SCCOL nHeaderCol = maStartPos.Col();
        SCROW nHeaderRow = maStartPos.Row() + nField + (mbDoFilter ? 1 : 0);
        // draw without frame for consistency with filter button:
        FieldCell(nHeaderCol, nHeaderRow, nTab, mpPageFields[nField], false);
        SCCOL nFieldCol = nHeaderCol + 1;

        OUString aPageValue = ScResId(SCSTR_ALL);
        const cpo::uno::Sequence<sheet::MemberResult>& rRes = mpPageFields[nField].maResult;
        sal_Int32 n = rRes.getLength();
        if (n == 1)
        {
            if (rRes[0].Caption.isEmpty())
                aPageValue = ScResId(STR_EMPTYDATA);
            else
                aPageValue = rRes[0].Caption;
        }
        else if (n > 1)
        {
            aPageValue = ScResId(SCSTR_MULTIPLE);
        }

        ScSetStringParam aParam;
        aParam.setTextInput();
        mpDocument->SetString(nFieldCol, nHeaderRow, nTab, aPageValue, &aParam);

        mpStyleOutput->addPageFieldValueCell(nFieldCol, nHeaderRow);
    }
}

void ScDPOutput::outputColumnHeaders(SCTAB nTab)
{
    size_t nNumColFields = mpColFields.size();

    for (size_t nField = 0; nField < nNumColFields; nField++)
    {
        SCCOL nHeaderCol = mnDataStartCol + SCCOL(nField); //TODO: check for overflow

        if (mnMemberStartRow > mnTabStartRow)
        {
            if (!mbHasCompactRowField || nNumColFields == 1)
                FieldCell(nHeaderCol, mnTabStartRow, nTab, mpColFields[nField], true);
            else if (!nField)
                MultiFieldCell(nHeaderCol, mnTabStartRow, nTab, false /* bRowField */);
        }

        SCROW nRowPos = mnMemberStartRow + SCROW(nField); //TODO: check for overflow
        const cpo::uno::Sequence<sheet::MemberResult> rMemberSequence = mpColFields[nField].maResult;
        const sheet::MemberResult* pMemberArray = rMemberSequence.getConstArray();
        tools::Long nThisColCount = rMemberSequence.getLength();
        OSL_ENSURE(nThisColCount == mnColCount, "count mismatch"); //TODO: ???

        for (tools::Long nColumn = 0; nColumn < nThisColCount; nColumn++)
        {
            sheet::MemberResult const& rMember = rMemberSequence[nColumn];

            SCCOL nColPos = mnDataStartCol + SCCOL(nColumn); //TODO: check for overflow

            HeaderCell(nColPos, nRowPos, nTab, rMember, true, nField);

            if ((rMember.Flags & sheet::MemberResultFlags::HASMEMBER) &&
               !(rMember.Flags & sheet::MemberResultFlags::SUBTOTAL))
            {
                // Check the number of columns this spreads
                tools::Long nEnd = nColumn;
                while (nEnd + 1 < nThisColCount && (pMemberArray[nEnd + 1].Flags & sheet::MemberResultFlags::CONTINUE))
                    ++nEnd;

                SCCOL nEndColPos = mnDataStartCol + SCCOL(nEnd); //TODO: check for overflow
                mpStyleOutput->addColumnMemberSpan(nField, nColPos, nEndColPos, nRowPos);
            }

            // Resolve formats
            maFormatOutput.insertFieldMember(nField, mpColFields[nField], nColumn, rMember, nColPos, nRowPos, sc::FormatResultDirection::COLUMN);

            // Apply the same number format as in data source.
            mpDocument->ApplyAttr(nColPos, nRowPos, nTab, SfxUInt32Item(ATTR_VALUE_FORMAT, mpColFields[nField].mnSrcNumFmt));
        }
    }
}

void ScDPOutput::outputRowHeader(SCTAB nTab)
{
    size_t nFieldColOffset = 0;
    size_t nFieldIndentLevel = 0; // To calculate indent level for fields packed in a column.
    size_t nNumRowFields = mpRowFields.size();
    for (size_t nField = 0; nField < nNumRowFields; nField++)
    {
        const bool bCompactField = maRowCompactFlags[nField];
        SCCOL nHdrCol = mnTabStartCol + SCCOL(nField); //TODO: check for overflow
        SCROW nHdrRow = mnDataStartRow - 1;
        if (!mbHasCompactRowField || nNumRowFields == 1)
            FieldCell(nHdrCol, nHdrRow, nTab, mpRowFields[nField], true);
        else if (!nField)
            MultiFieldCell(nHdrCol, nHdrRow, nTab, true /* bRowField */);

        SCCOL nColPos = mnMemberStartCol + SCCOL(nFieldColOffset); //TODO: check for overflow
        const cpo::uno::Sequence<sheet::MemberResult> rMemberSequence = mpRowFields[nField].maResult;
        const sheet::MemberResult* pMemberArray = rMemberSequence.getConstArray();
        sal_Int32 nThisRowCount = rMemberSequence.getLength();
        OSL_ENSURE(nThisRowCount == mnRowCount, "count mismatch");     //TODO: ???
        for (sal_Int32 nRow = 0; nRow < nThisRowCount; nRow++)
        {
            sheet::MemberResult const& rMember = rMemberSequence[nRow];
            const sheet::MemberResult& rData = rMember;
            const bool bHasMember = rData.Flags & sheet::MemberResultFlags::HASMEMBER;
            const bool bSubtotal = rData.Flags & sheet::MemberResultFlags::SUBTOTAL;
            SCROW nRowPos = mnDataStartRow + SCROW(nRow); //TODO: check for overflow
            HeaderCell( nColPos, nRowPos, nTab, rData, false, nFieldColOffset );
            if (bHasMember && !bSubtotal)
            {
                tools::Long nEnd = nRow;
                while (nEnd + 1 < nThisRowCount && (pMemberArray[nEnd + 1].Flags & sheet::MemberResultFlags::CONTINUE))
                {
                    ++nEnd;
                }
                SCROW nEndRowPos = mnDataStartRow + SCROW(nEnd); //TODO: check for overflow
                mpStyleOutput->addRowMemberSpan(nField, nColPos, nRowPos, nEndRowPos);

                // Set flags for collapse/expand buttons and indent field header text
                {
                    bool bLast = mnRowDims == (nField + 1);
                    size_t nMinIndentLevel = mbExpandCollapse ? 1 : 0;
                    tools::Long nIndent = o3tl::convert(13 * (bLast ? nFieldIndentLevel : nMinIndentLevel + nFieldIndentLevel), o3tl::Length::px, o3tl::Length::twip);
                    bool bHasContinue = !bLast && nRow + 1 < nThisRowCount && (pMemberArray[nRow + 1].Flags & sheet::MemberResultFlags::CONTINUE);
                    if (nIndent)
                        mpStyleOutput->addIndent(nColPos, nRowPos, nIndent);
                    // A repeated item label is not the header of the group, so it has no button
                    const bool bRepeatedMember = rData.Flags & sheet::MemberResultFlags::CONTINUE;
                    if (mbExpandCollapse && !bLast && !bRepeatedMember)
                    {
                        mpStyleOutput->addExpander(nColPos, nRowPos,
                            bHasContinue ? ScMF::DpCollapse : ScMF::DpExpand);
                    }
                }
            }

            // Resolve formats
            maFormatOutput.insertFieldMember(nField, mpRowFields[nField], nRow, rMember, nColPos, nRowPos, sc::FormatResultDirection::ROW);

            // Apply the same number format as in data source.
            mpDocument->ApplyAttr(nColPos, nRowPos, nTab, SfxUInt32Item(ATTR_VALUE_FORMAT, mpRowFields[nField].mnSrcNumFmt));
        }

        if (!bCompactField)
        {
            // Next field should be placed in next column only if current field has a non-compact layout.
            ++nFieldColOffset;
            nFieldIndentLevel = 0; // Reset indent level.
        }
        else
        {
            ++nFieldIndentLevel;
        }
    }
}

void ScDPOutput::outputDataResults(SCTAB nTab)
{
    const cpo::uno::Sequence<sheet::DataResult>* pRowAry = maData.getConstArray();

    for (sal_Int32 nRow = 0; nRow < mnRowCount; nRow++)
    {
        SCROW nRowPos = mnDataStartRow + SCROW(nRow); //TODO: check for overflow
        const sheet::DataResult* pColAry = pRowAry[nRow].getConstArray();
        sal_Int32 nThisColCount = pRowAry[nRow].getLength();
        OSL_ENSURE(nThisColCount == mnColCount, "count mismatch"); //TODO: ???
        for (sal_Int32 nCol = 0; nCol < nThisColCount; nCol++)
        {
            SCCOL nColPos = mnDataStartCol + SCCOL(nCol); //TODO: check for overflow
            DataCell(nColPos, nRowPos, nTab, pColAry[nCol]);
        }
    }

    maFormatOutput.apply(*mpDocument);
}

void ScDPOutput::Output()
{
    SCTAB nTab = maStartPos.Tab();

    //  calculate output positions and sizes
    CalcSizes();

    if (mbSizeOverflow || mbResultsError)   // does output area exceed sheet limits?
        return;                             // nothing

    // Collect the pivot table sections while the header content is written, so all the visual
    // formatting can be drawn in one pass.
    mpStyleOutput->clear();
    {
        sc::pivot::Geometry aGeometry;
        aGeometry.mnTab = nTab;
        aGeometry.mnTabStartCol = mnTabStartCol;
        aGeometry.mnTabStartRow = mnTabStartRow;
        aGeometry.mnMemberStartCol = mnMemberStartCol;
        aGeometry.mnMemberStartRow = mnMemberStartRow;
        aGeometry.mnDataStartCol = mnDataStartCol;
        aGeometry.mnDataStartRow = mnDataStartRow;
        aGeometry.mnTabEndCol = mnTabEndCol;
        aGeometry.mnTabEndRow = mnTabEndRow;
        aGeometry.mnColumnFieldCount = mpColFields.size();
        aGeometry.mnRowFieldCount = mpRowFields.size();
        mpStyleOutput->setGeometry(aGeometry);
    }

    // Prepare format output
    bool bColumnFieldIsDataOnly = mnColCount == 1 && mnRowCount > 0 && mpColFields.empty();
    maFormatOutput.prepare(nTab, mpColFields, mpRowFields, bColumnFieldIsDataOnly);

    // Set grand total positions and data area for format output matching
    {
        bool bColumnGrand = false;
        bool bRowGrand = false;
        uno::Reference<beans::XPropertySet> xSourcePropertySet(mxSource, uno::UNO_QUERY);
        if (xSourcePropertySet.is())
        {
            bColumnGrand = ScUnoHelpFunctions::GetBoolProperty(xSourcePropertySet, SC_UNO_DP_COLGRAND);
            bRowGrand = ScUnoHelpFunctions::GetBoolProperty(xSourcePropertySet, SC_UNO_DP_ROWGRAND);
        }
        SCROW nGrandTotalRow = bRowGrand ? mnTabEndRow : -1;
        SCCOL nGrandTotalColumn = bColumnGrand ? mnTabEndCol : -1;
        maFormatOutput.setGrandTotalPositions(nGrandTotalRow, nGrandTotalColumn);
        maFormatOutput.setDataArea(maStartPos.Col(), maStartPos.Row(), mnDataStartCol, mnDataStartRow, mnTabEndCol, mnTabEndRow);
    }

    //  clear whole (new) output area
    // when modifying table, clear old area !
    //TODO: include InsertDeleteFlags::OBJECTS ???
    mpDocument->DeleteAreaTab(maStartPos.Col(), maStartPos.Row(), mnTabEndCol, mnTabEndRow, nTab, InsertDeleteFlags::ALL );

    if (mbDoFilter)
        lcl_DoFilterButton(mpDocument, maStartPos.Col(), maStartPos.Row(), nTab);

    outputPageFields(nTab);

    //  data description
    //  (may get overwritten by first row field)

    if (maDataDescription.isEmpty())
    {
        //TODO: use default string ("result") ?
    }
    mpDocument->SetString(mnTabStartCol, mnTabStartRow, nTab, maDataDescription);

    outputColumnHeaders(nTab);

    outputRowHeader(nTab);

    if (bColumnFieldIsDataOnly)
    {
        // the table contains exactly one data field and no column fields.
        // Display data description at top right corner. The classic layout
        // has it in the header row above, so label the column as a total.
        ScSetStringParam aParam;
        aParam.setTextInput();
        SCCOL nCol = mnDataStartCol;
        SCROW nRow = mnDataStartRow - 1;
        // more than one header row means the description has its own row above
        const bool bOwnHeaderRow = mnHeaderSize > 1;
        const OUString aHeader
            = bOwnHeaderRow ? ScResId(STR_PIVOT_TOTAL) : maDataDescription;
        mpDocument->SetString(nCol, nRow, nTab, aHeader, &aParam);
        maFormatOutput.insertEmptyDataColumn(nCol, nRow);
    }

    // Draw the visual formatting of the table before the results are written. The results carry
    // the number formats and the pivot table formats of the document, and those come out on top
    // of the frames, the bold weight and the alignment that are drawn here.
    mpStyleOutput->apply();

    outputDataResults(nTab);
}

void ScDPOutput::Output(const ScRange& rOldRange, bool bCheckForSpill)
{
    mbSpillError = false;

    if (!bCheckForSpill)
    {
        Output();
        return;
    }

    SCTAB nTab = maStartPos.Tab();

    CalcSizes();

    if (mbSizeOverflow || mbResultsError)
        return;

    // Check if the new output range has non-empty cells outside the old pivot table area.
    ScRange aNewRange(maStartPos.Col(), maStartPos.Row(), nTab, mnTabEndCol, mnTabEndRow, nTab);
    ScCellIterator aIterator(*mpDocument, aNewRange);
    for (bool bHasCell = aIterator.first(); bHasCell; bHasCell = aIterator.next())
    {
        if (!aIterator.isEmpty() && !rOldRange.Contains(aIterator.GetPos()))
        {
            mbSpillError = true;
            return;
        }
    }

    // No spill detected
}

ScRange ScDPOutput::GetOutputRange( sal_Int32 nRegionType )
{
    using namespace ::com::sun::star::sheet;

    CalcSizes();

    SCTAB nTab = maStartPos.Tab();
    switch (nRegionType)
    {
        case DataPilotOutputRangeType::RESULT:
            return ScRange(mnDataStartCol, mnDataStartRow, nTab, mnTabEndCol, mnTabEndRow, nTab);
        case DataPilotOutputRangeType::TABLE:
            return ScRange(maStartPos.Col(), mnTabStartRow, nTab, mnTabEndCol, mnTabEndRow, nTab);
        default:
            OSL_ENSURE(nRegionType == DataPilotOutputRangeType::WHOLE, "ScDPOutput::GetOutputRange: unknown region type");
        break;
    }
    return ScRange(maStartPos.Col(), maStartPos.Row(), nTab, mnTabEndCol, mnTabEndRow, nTab);
}

bool ScDPOutput::HasError()
{
    CalcSizes();

    return mbSizeOverflow || mbResultsError;
}

sal_Int32 ScDPOutput::GetHeaderRows() const
{
    return mpPageFields.size() + (mbDoFilter ? 1 : 0);
}

namespace
{
    void insertNames(ScDPUniqueStringSet& rNames, const cpo::uno::Sequence<sheet::MemberResult>& rMemberResults)
    {
        for (const sheet::MemberResult& rMemberResult : rMemberResults)
        {
            if (rMemberResult.Flags & sheet::MemberResultFlags::HASMEMBER)
                rNames.insert(rMemberResult.Name);
        }
    }
}

void ScDPOutput::GetMemberResultNames(ScDPUniqueStringSet& rNames, tools::Long nDimension)
{
    //  Return the list of all member names in a dimension's MemberResults.
    //  Only the dimension has to be compared because this is only used with table data,
    //  where each dimension occurs only once.

    auto lFindDimension = [nDimension](const ScDPOutLevelData& rField) { return rField.mnDim == nDimension; };

    // look in column fields
    auto colit = std::find_if(mpColFields.begin(), mpColFields.end(), lFindDimension);
    if (colit != mpColFields.end())
    {
        // collect the member names
        insertNames(rNames, colit->maResult);
        return;
    }

    // look in row fields
    auto rowit = std::find_if(mpRowFields.begin(), mpRowFields.end(), lFindDimension);
    if (rowit != mpRowFields.end())
    {
        // collect the member names
        insertNames(rNames, rowit->maResult);
    }
}

void ScDPOutput::SetHeaderLayout(bool bUseGrid)
{
    mbHeaderLayout = bUseGrid;
    mbSizesValid = false;
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

    if ( !xSource.is() )
        return;

    // find index and orientation of "data layout" dimension, count data dimensions

    sal_Int32 nDataCount = 0;

    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xSource->getDimensions() );
    tools::Long nDimCount = xDims->getCount();
    for (tools::Long nDim=0; nDim<nDimCount; nDim++)
    {
        uno::Reference<uno::XInterface> xDim(xDims->getByIndex(nDim), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        if ( xDimProp.is() )
        {
            sheet::DataPilotFieldOrientation eDimOrient =
                ScUnoHelpFunctions::GetEnumProperty(
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
                    cpo::uno::Any aValue = xDimProp->getPropertyValue( SC_UNO_DP_LAYOUTNAME );

                    if( aValue.hasValue() )
                    {
                        OUString strLayoutName;

                        if( ( aValue >>= strLayoutName ) && !strLayoutName.isEmpty() )
                            aGivenName = strLayoutName;
                    }
                }
                catch(const cpo::uno::Exception&)
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

void ScDPOutput::GetRowFieldRange(SCCOL nCol, sal_Int32& nRowFieldStart, sal_Int32& nRowFieldEnd) const
{
    if (!mbHasCompactRowField)
    {
        nRowFieldStart = nCol;
        nRowFieldEnd = nCol + 1;
        return;
    }

    if (nCol >= static_cast<SCCOL>(maRowCompactFlags.size()))
    {
        nRowFieldStart = nRowFieldEnd = 0;
        return;
    }

    nRowFieldStart = -1;
    nRowFieldEnd = -1;
    SCCOL nCurCol = 0;
    sal_Int32 nField = 0;

    for (const auto bCompact: maRowCompactFlags)
    {
        if (nCurCol == nCol && nRowFieldStart == -1)
            nRowFieldStart = nField;

        if (!bCompact)
            ++nCurCol;

        ++nField;

        if (nCurCol == (nCol + 1) && nRowFieldStart != -1 && nRowFieldEnd == -1)
        {
            nRowFieldEnd = nField;
            break;
        }
    }

    if (nRowFieldStart != -1 && nRowFieldEnd == -1 && nCurCol == nCol)
        nRowFieldEnd = static_cast<sal_Int32>(maRowCompactFlags.size());

    if (nRowFieldStart == -1 || nRowFieldEnd == -1)
    {
        SAL_WARN("sc.core", "ScDPOutput::GetRowFieldRange : unable to find field range for nCol = " << nCol);
        nRowFieldStart = nRowFieldEnd = 0;
    }
}

sal_Int32 ScDPOutput::GetRowFieldCompact(SCCOL nColQuery, SCROW nRowQuery) const
{
    if (!mbHasCompactRowField)
        return nColQuery - mnTabStartCol;

    SCCOL nCol = nColQuery - mnTabStartCol;
    sal_Int32 nStartField = 0;
    sal_Int32 nEndField = 0;
    GetRowFieldRange(nCol, nStartField, nEndField);

    for (sal_Int32 nField = nEndField - 1; nField >= nStartField; --nField)
    {
        const cpo::uno::Sequence<sheet::MemberResult> rSequence = mpRowFields[nField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();
        sal_Int32 nThisRowCount = rSequence.getLength();
        SCROW nRow = nRowQuery - mnDataStartRow;
        if (nRow >= 0 && nRow < nThisRowCount)
        {
            const sheet::MemberResult& rData = pArray[nRow];
            if ((rData.Flags & sheet::MemberResultFlags::HASMEMBER)
                && !(rData.Flags & sheet::MemberResultFlags::SUBTOTAL))
            {
                return nField;
            }
        }
    }

    return -1;
}

void ScDPOutput::GetPositionData(const ScAddress& rPos, sheet::DataPilotTablePositionData& rPosData)
{
    using namespace ::com::sun::star::sheet;

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if (nTab != maStartPos.Tab())
        return; // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    rPosData.PositionType = GetPositionType(rPos);
    switch (rPosData.PositionType)
    {
        case DataPilotTablePositionType::RESULT:
        {
            std::vector<DataPilotFieldFilter> aFilters;
            GetDataResultPositionData(aFilters, rPos);

            sheet::DataPilotTableResultData aResData;
            aResData.FieldFilters = comphelper::containerToSequence(aFilters);
            aResData.DataFieldIndex = 0;
            uno::Reference<beans::XPropertySet> xPropSet(mxSource, uno::UNO_QUERY);
            if (xPropSet.is())
            {
                sal_Int32 nDataFieldCount = ScUnoHelpFunctions::GetLongProperty( xPropSet,
                                            SC_UNO_DP_DATAFIELDCOUNT );
                if (nDataFieldCount > 0)
                    aResData.DataFieldIndex = (nRow - mnDataStartRow) % nDataFieldCount;
            }

            // Copy appropriate DataResult object from the cached sheet::DataResult table.
            if (maData.getLength() > nRow - mnDataStartRow &&
                maData[nRow - mnDataStartRow].getLength() > nCol - mnDataStartCol)
                aResData.Result = maData[nRow - mnDataStartRow][nCol - mnDataStartCol];

            rPosData.PositionData <<= aResData;
            return;
        }
        case DataPilotTablePositionType::COLUMN_HEADER:
        {
            tools::Long nField = nRow - mnTabStartRow - 1; // 1st line is used for the buttons
            if (nField < 0)
                break;

            if (mpColFields.size() < o3tl::make_unsigned(nField) + 1 )
                break;
            const cpo::uno::Sequence<sheet::MemberResult> rSequence = mpColFields[nField].maResult;
            if (!rSequence.hasElements())
                break;
            const sheet::MemberResult* pArray = rSequence.getConstArray();

            tools::Long nItem = nCol - mnDataStartCol;
            //  get origin of "continue" fields
            while (nItem > 0 && ( pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
                --nItem;

            if (nItem < 0)
                break;

            DataPilotTableHeaderData aHeaderData;
            aHeaderData.MemberName = pArray[nItem].Name;
            aHeaderData.Flags = pArray[nItem].Flags;
            aHeaderData.Dimension = static_cast<sal_Int32>(mpColFields[nField].mnDim);
            aHeaderData.Hierarchy = static_cast<sal_Int32>(mpColFields[nField].mnHier);
            aHeaderData.Level     = static_cast<sal_Int32>(mpColFields[nField].mnLevel);

            rPosData.PositionData <<= aHeaderData;
            return;
        }
        case DataPilotTablePositionType::ROW_HEADER:
        {
            tools::Long nField = GetRowFieldCompact(nCol, nRow);
            if (nField < 0)
                break;

            if (mpRowFields.size() < o3tl::make_unsigned(nField) + 1 )
                break;
            const cpo::uno::Sequence<sheet::MemberResult> rSequence = mpRowFields[nField].maResult;
            if (!rSequence.hasElements())
                break;
            const sheet::MemberResult* pArray = rSequence.getConstArray();

            tools::Long nItem = nRow - mnDataStartRow;
            //  get origin of "continue" fields
            while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
                --nItem;

            if (nItem < 0)
                break;

            DataPilotTableHeaderData aHeaderData;
            aHeaderData.MemberName = pArray[nItem].Name;
            aHeaderData.Flags = pArray[nItem].Flags;
            aHeaderData.Dimension = static_cast<sal_Int32>(mpRowFields[nField].mnDim);
            aHeaderData.Hierarchy = static_cast<sal_Int32>(mpRowFields[nField].mnHier);
            aHeaderData.Level     = static_cast<sal_Int32>(mpRowFields[nField].mnLevel);

            rPosData.PositionData <<= aHeaderData;
            return;
        }
    }
}

bool ScDPOutput::GetDataResultPositionData(std::vector<sheet::DataPilotFieldFilter>& rFilters, const ScAddress& rPos)
{
    // Check to make sure there is at least one data field.
    uno::Reference<beans::XPropertySet> xPropSet(mxSource, uno::UNO_QUERY);
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
    lcl_GetTableVars( nGrandTotalCols, nGrandTotalRows, nDataLayoutIndex, aDataNames, aGivenNames, eDataOrient, mxSource);

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if (nTab != maStartPos.Tab())
        return false; // wrong sheet

    CalcSizes();

    // test for data area.
    if (nCol < mnDataStartCol || nCol > mnTabEndCol || nRow < mnDataStartRow || nRow > mnTabEndRow)
    {
        // Cell is outside the data field area.
        return false;
    }

    bool bFilterByCol = (nCol <= static_cast<SCCOL>(mnTabEndCol - nGrandTotalCols));
    bool bFilterByRow = (nRow <= static_cast<SCROW>(mnTabEndRow - nGrandTotalRows));

    // column fields
    for (size_t nColField = 0; nColField < mpColFields.size() && bFilterByCol; ++nColField)
    {
        if (mpColFields[nColField].mnDim == nDataLayoutIndex)
            // There is no sense including the data layout field for filtering.
            continue;

        sheet::DataPilotFieldFilter filter;
        filter.FieldName = mpColFields[nColField].maName;

        const cpo::uno::Sequence<sheet::MemberResult> rSequence = mpColFields[nColField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();

        OSL_ENSURE(mnDataStartCol + rSequence.getLength() - 1 == mnTabEndCol, "ScDPOutput::GetDataFieldCellData: error in geometric assumption");

        tools::Long nItem = nCol - mnDataStartCol;
                //  get origin of "continue" fields
        while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
            --nItem;

        filter.MatchValueName = pArray[nItem].Name;
        rFilters.push_back(filter);
    }

    // row fields
    for (size_t nRowField = 0; nRowField < mpRowFields.size() && bFilterByRow; ++nRowField)
    {
        if (mpRowFields[nRowField].mnDim == nDataLayoutIndex)
            // There is no sense including the data layout field for filtering.
            continue;

        sheet::DataPilotFieldFilter filter;
        filter.FieldName = mpRowFields[nRowField].maName;

        const cpo::uno::Sequence<sheet::MemberResult> rSequence = mpRowFields[nRowField].maResult;
        const sheet::MemberResult* pArray = rSequence.getConstArray();

        OSL_ENSURE(mnDataStartRow + rSequence.getLength() - 1 == mnTabEndRow, "ScDPOutput::GetDataFieldCellData: error in geometric assumption");

        tools::Long nItem = nRow - mnDataStartRow;
            //  get origin of "continue" fields
        while ( nItem > 0 && (pArray[nItem].Flags & sheet::MemberResultFlags::CONTINUE) )
            --nItem;

        filter.MatchValueName = pArray[nItem].Name;
        rFilters.push_back(filter);
    }

    return true;
}

namespace {

OUString lcl_GetDataFieldName( std::u16string_view rSourceName, sal_Int16 eFunc )
{
    TranslateId pStrId;
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

    return ScResId(pStrId) + " - " + rSourceName;
}

}

void ScDPOutput::GetDataDimensionNames(
    OUString& rSourceName, OUString& rGivenName, const uno::Reference<uno::XInterface>& xDim )
{
    uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
    uno::Reference<container::XNamed> xDimName( xDim, uno::UNO_QUERY );
    if ( !(xDimProp.is() && xDimName.is()) )
        return;

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

bool ScDPOutput::IsFilterButton( const ScAddress& rPos )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if (nTab != maStartPos.Tab() || !mbDoFilter)
        return false;                               // wrong sheet or no button at all

    //  filter button is at top left
    return nCol == maStartPos.Col() && nRow == maStartPos.Row();
}

tools::Long ScDPOutput::GetHeaderDim( const ScAddress& rPos, sheet::DataPilotFieldOrientation& rOrient )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if (nTab != maStartPos.Tab())
        return -1;                                      // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nRow == mnTabStartRow && nCol >= mnDataStartCol && o3tl::make_unsigned(nCol) < mnDataStartCol + mpColFields.size())
    {
        rOrient = sheet::DataPilotFieldOrientation_COLUMN;
        tools::Long nField = nCol - mnDataStartCol;
        return mpColFields[nField].mnDim;
    }

    //  test for row header

    if ( nRow+1 == mnDataStartRow && nCol >= mnTabStartCol && o3tl::make_unsigned(nCol) < mnTabStartCol + mpRowFields.size() )
    {
        rOrient = sheet::DataPilotFieldOrientation_ROW;
        tools::Long nField = nCol - mnTabStartCol;
        return mpRowFields[nField].mnDim;
    }

    //  test for page field

    SCROW nPageStartRow = maStartPos.Row() + (mbDoFilter ? 1 : 0);
    if ( nCol == maStartPos.Col() && nRow >= nPageStartRow && o3tl::make_unsigned(nRow) < nPageStartRow + mpPageFields.size() )
    {
        rOrient = sheet::DataPilotFieldOrientation_PAGE;
        tools::Long nField = nRow - nPageStartRow;
        return mpPageFields[nField].mnDim;
    }

    //TODO: single data field (?)

    rOrient = sheet::DataPilotFieldOrientation_HIDDEN;
    return -1;      // invalid
}

bool ScDPOutput::GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop,
                                tools::Long nDragDim,
                                tools::Rectangle& rPosRect, sheet::DataPilotFieldOrientation& rOrient, tools::Long& rDimPos )
{
    //  Rectangle instead of ScRange for rPosRect to allow for negative values

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    if ( nTab != maStartPos.Tab() )
        return false;                                       // wrong sheet

    //  calculate output positions and sizes

    CalcSizes();

    //  test for column header

    if ( nCol >= mnDataStartCol && nCol <= mnTabEndCol &&
            nRow + 1 >= mnMemberStartRow && o3tl::make_unsigned(nRow) < mnMemberStartRow + mpColFields.size())
    {
        tools::Long nField = nRow - mnMemberStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = true;
        }
        //TODO: find start of dimension

        rPosRect = tools::Rectangle(mnDataStartCol, mnMemberStartRow + nField,
                                    mnTabEndCol, mnMemberStartRow + nField - 1);

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (tools::Long nPos=0; o3tl::make_unsigned(nPos)<mpColFields.size() && !bFound; nPos++)
        {
            if (mpColFields[nPos].mnDim == nDragDim)
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
                rPosRect.AdjustBottom( 1 );
                if (bAfterDrag)
                    rPosRect.AdjustTop( 1 );
            }
        }
        else
        {
            if ( !bMouseTop )
            {
                rPosRect.AdjustTop( 1 );
                rPosRect.AdjustBottom( 1 );
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_COLUMN;
        rDimPos = nField;                       //!...
        return true;
    }

    //  test for row header

    //  special case if no row fields
    bool bSpecial = ( nRow+1 >= mnDataStartRow && nRow <= mnTabEndRow &&
                        mpRowFields.empty() && nCol == mnTabStartCol && bMouseLeft );

    if ( bSpecial || ( nRow+1 >= mnDataStartRow && nRow <= mnTabEndRow &&
                        nCol + 1 >= mnTabStartCol && o3tl::make_unsigned(nCol) < mnTabStartCol + mpRowFields.size() ) )
    {
        tools::Long nField = nCol - mnTabStartCol;
        //TODO: find start of dimension

        rPosRect = tools::Rectangle(mnTabStartCol + nField, mnDataStartRow - 1,
                              mnTabStartCol + nField - 1, mnTabEndRow);

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (tools::Long nPos = 0; o3tl::make_unsigned(nPos) < mpRowFields.size() && !bFound; nPos++)
        {
            if (mpRowFields[nPos].mnDim == nDragDim)
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
                rPosRect.AdjustRight( 1 );
                if (bAfterDrag)
                    rPosRect.AdjustLeft( 1 );
            }
        }
        else
        {
            if ( !bMouseLeft )
            {
                rPosRect.AdjustLeft( 1 );
                rPosRect.AdjustRight( 1 );
                ++nField;
            }
        }

        rOrient = sheet::DataPilotFieldOrientation_ROW;
        rDimPos = nField;                       //!...
        return true;
    }

    //  test for page fields

    SCROW nPageStartRow = maStartPos.Row() + (mbDoFilter ? 1 : 0);
    if (nCol >= maStartPos.Col() && nCol <= mnTabEndCol &&
            nRow + 1 >= nPageStartRow && o3tl::make_unsigned(nRow) < nPageStartRow + mpPageFields.size())
    {
        tools::Long nField = nRow - nPageStartRow;
        if (nField < 0)
        {
            nField = 0;
            bMouseTop = true;
        }
        //TODO: find start of dimension

        rPosRect = tools::Rectangle(maStartPos.Col(), nPageStartRow + nField,
                                    mnTabEndCol, nPageStartRow + nField - 1);

        bool bFound = false;            // is this within the same orientation?
        bool bBeforeDrag = false;
        bool bAfterDrag = false;
        for (tools::Long nPos = 0; o3tl::make_unsigned(nPos) < mpPageFields.size() && !bFound; nPos++)
        {
            if (mpPageFields[nPos].mnDim == nDragDim)
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
                rPosRect.AdjustBottom( 1 );
                if (bAfterDrag)
                    rPosRect.AdjustTop( 1 );
            }
        }
        else
        {
            if ( !bMouseTop )
            {
                rPosRect.AdjustTop( 1 );
                rPosRect.AdjustBottom( 1 );
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
