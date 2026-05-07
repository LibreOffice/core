/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <PivotTableFormat.hxx>
#include <pivottablebuffer.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <pivot/PivotTableFormats.hxx>
#include <dpsave.hxx>
#include <dpobject.hxx>

namespace oox::xls
{
PivotTableFormat::PivotTableFormat(PivotTable& rPivotTable)
    : WorkbookHelper(rPivotTable)
    , mrPivotTable(rPivotTable)
{
}

void PivotTableFormat::importFormat(const oox::AttributeList& rAttribs)
{
    mnDxfId = rAttribs.getInteger(XML_dxfId, -1);
}

void PivotTableFormat::importPivotArea(const oox::AttributeList& rAttribs)
{
    mnField = rAttribs.getInteger(XML_field);

    auto oType = rAttribs.getToken(XML_type);
    if (oType)
    {
        switch (*oType)
        {
            case XML_none:
                meType = PivotAreaType::None;
                break;
            case XML_data:
                meType = PivotAreaType::Data;
                break;
            case XML_all:
                meType = PivotAreaType::All;
                break;
            case XML_origin:
                meType = PivotAreaType::Origin;
                break;
            case XML_button:
                meType = PivotAreaType::Button;
                break;
            case XML_topRight:
                meType = PivotAreaType::TopRight;
                break;
            default:
            case XML_normal:
                meType = PivotAreaType::Normal;
                break;
        }
    }

    moField = rAttribs.getUnsigned(XML_field);
    mbDataOnly = rAttribs.getBool(XML_dataOnly, true);
    mbLabelOnly = rAttribs.getBool(XML_labelOnly, false);
    mbGrandRow = rAttribs.getBool(XML_grandRow, false);
    mbGrandCol = rAttribs.getBool(XML_grandCol, false);
    mbCacheIndex = rAttribs.getBool(XML_cacheIndex, false);
    mbOutline = rAttribs.getBool(XML_outline, true);
    moOffset = rAttribs.getXString(XML_offset);
    mbCollapsedLevelsAreSubtotals = rAttribs.getBool(XML_collapsedLevelsAreSubtotals, false);
    moFieldPosition = rAttribs.getUnsigned(XML_fieldPosition);
}

void PivotTableFormat::finalizeImport()
{
    auto pPattern = std::make_shared<ScPatternAttr>(getScDocument().getCellAttributeHelper());

    if (DxfRef pDxf = getStyles().getDxf(mnDxfId))
    {
        // bSkipPoolDefs=true so dxf items at pool default (e.g.
        // ScLineBreakCell(false) for a dxf carrying alignment but no explicit
        // wrapText) don't silently overwrite values set by an earlier dxf in
        // the apply chain. Without this, multi-format buttons end up with
        // wrap=false because the LAST applied alignment-bearing dxf with
        // implicit-default wrap wins.
        pDxf->fillToItemSet(pPattern->GetItemSetWritable(), true);
    }

    ScDPObject* pDPObj = mrPivotTable.getDPObject();
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();

    sc::PivotTableFormats aFormats;

    if (pSaveData->hasFormats())
        aFormats = pSaveData->getFormats();

    // Resolve references - TODO

    sc::PivotTableFormat aFormat;
    // type='button' formats target a field's button cell — distinct from data-
    // area or member-label cells. Currently, ECMA-376 says button formats
    // typically have dataOnly=0 labelOnly=0; without this branch they would
    // collapse to FormatType::None and silently drop. Classify them as
    // FormatType::Button so the pivot output applies the dxf to the field-
    // button cell explicitly.
    if (meType == PivotAreaType::Button)
    {
        aFormat.eType = sc::FormatType::Button;
        aFormat.oField = mnField;
    }
    else if (mbDataOnly)
        aFormat.eType = sc::FormatType::Data;
    else if (mbLabelOnly)
        aFormat.eType = sc::FormatType::Label;

    aFormat.bDataOnly = mbDataOnly;
    aFormat.bLabelOnly = mbLabelOnly;
    aFormat.bOutline = mbOutline;
    aFormat.oFieldPosition = moFieldPosition;

    aFormat.pPattern = std::move(pPattern);
    for (auto& rReference : maReferences)
    {
        if (rReference->mnField)
        {
            aFormat.aSelections.push_back(
                sc::Selection{ .bSelected = rReference->mbSelected,
                               .nField = sal_Int32(*rReference->mnField),
                               .nIndices = rReference->maFieldItemsIndices });
        }
    }
    aFormats.add(aFormat);

    pSaveData->setFormats(aFormats);
}

#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
PivotTableReference& PivotTableFormat::createReference()
{
    auto xReference = std::make_shared<PivotTableReference>(*this);
    maReferences.push_back(xReference);
    return *xReference;
}
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 16
#pragma GCC diagnostic pop
#endif

PivotTableReference::PivotTableReference(const PivotTableFormat& rFormat)
    : WorkbookHelper(rFormat)
{
}
void PivotTableReference::importReference(const oox::AttributeList& rAttribs)
{
    mnField = rAttribs.getUnsigned(XML_field);
    mnCount = rAttribs.getUnsigned(XML_count);
    mbSelected = rAttribs.getBool(XML_selected, true);
    mbByPosition = rAttribs.getBool(XML_byPosition, false);
    mbRelative = rAttribs.getBool(XML_relative, false);
    mbDefaultSubtotal = rAttribs.getBool(XML_defaultSubtotal, false);
    mbSumSubtotal = rAttribs.getBool(XML_sumSubtotal, false);
    mbCountASubtotal = rAttribs.getBool(XML_countASubtotal, false);
    mbAvgSubtotal = rAttribs.getBool(XML_avgSubtotal, false);
    mbMaxSubtotal = rAttribs.getBool(XML_maxSubtotal, false);
    mbMinSubtotal = rAttribs.getBool(XML_minSubtotal, false);
    mbProductSubtotal = rAttribs.getBool(XML_productSubtotal, false);
    mbCountSubtotal = rAttribs.getBool(XML_countSubtotal, false);
    mbStdDevSubtotal = rAttribs.getBool(XML_stdDevSubtotal, false);
    mbStdDevPSubtotal = rAttribs.getBool(XML_stdDevPSubtotal, false);
    mbVarSubtotal = rAttribs.getBool(XML_varSubtotal, false);
    mbVarPSubtotal = rAttribs.getBool(XML_varPSubtotal, false);
}

void PivotTableReference::addFieldItem(const oox::AttributeList& rAttribs)
{
    auto oSharedItemsIndex = rAttribs.getUnsigned(XML_v); // XML_v - shared items index
    if (oSharedItemsIndex)
    {
        maFieldItemsIndices.push_back(*oSharedItemsIndex);
    }
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
