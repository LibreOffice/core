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

#include <sal/config.h>

#include <string_view>

#include "res_DataLabel.hxx"

#include <TextDirectionListBox.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include "dlg_NumberFormat.hxx"

#include <svx/numinf.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/ilstitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/sdangitm.hxx>
#include <svx/svxids.hrc>
#include <osl/diagnose.h>

namespace chart
{

namespace
{

const std::u16string_view our_aLBEntryMap[] = {u" ", u", ", u"; ", u"\n", u". "};

bool lcl_ReadNumberFormatFromItemSet( const SfxItemSet& rSet, TypedWhichId<SfxUInt32Item> nValueWhich, TypedWhichId<SfxBoolItem> nSourceFormatWhich, sal_uInt32& rnFormatKeyOut, bool& rbSourceFormatOut, bool& rbSourceFormatMixedStateOut )
{
    bool bSet = false;
    if( const SfxUInt32Item* pNumItem = rSet.GetItemIfSet( nValueWhich ) )
    {
        rnFormatKeyOut = pNumItem->GetValue();
        bSet = true;
    }

    rbSourceFormatMixedStateOut=true;
    if( const SfxBoolItem * pBoolItem = rSet.GetItemIfSet( nSourceFormatWhich ) )
    {
        rbSourceFormatOut = pBoolItem->GetValue();
        rbSourceFormatMixedStateOut=false;
    }
    return bSet;
}

void lcl_setBoolItemToCheckBox(const SfxItemSet& rInAttrs, TypedWhichId<SfxBoolItem> nWhichId, weld::CheckButton& rCheckbox, weld::TriStateEnabled& rTriState)
{
    if( const SfxBoolItem* pPoolItem = rInAttrs.GetItemIfSet(nWhichId) )
    {
        rCheckbox.set_active(pPoolItem->GetValue());
        rTriState.bTriStateEnabled = false;
    }
    else
    {
        rCheckbox.set_state(TRISTATE_INDET);
        rTriState.bTriStateEnabled = true;
    }
}

}//end anonymous namespace

DataLabelResources::DataLabelResources(weld::Builder* pBuilder, weld::Window* pParent, const SfxItemSet& rInAttrs)
    : m_pNumberFormatter(nullptr)
    , m_bNumberFormatMixedState(true)
    , m_bPercentFormatMixedState(true)
    , m_nNumberFormatForValue(0)
    , m_nNumberFormatForPercent(11)
    , m_bSourceFormatMixedState(true)
    , m_bPercentSourceMixedState(true)
    , m_bSourceFormatForValue(true)
    , m_bSourceFormatForPercent(true)
    , m_pWindow(pParent)
    , m_pPool(rInAttrs.GetPool())
    , m_xCBNumber(pBuilder->weld_check_button(u"CB_VALUE_AS_NUMBER"_ustr))
    , m_xPB_NumberFormatForValue(pBuilder->weld_button(u"PB_NUMBERFORMAT"_ustr))
    , m_xCBPercent(pBuilder->weld_check_button(u"CB_VALUE_AS_PERCENTAGE"_ustr))
    , m_xPB_NumberFormatForPercent(pBuilder->weld_button(u"PB_PERCENT_NUMBERFORMAT"_ustr))
    , m_xFT_NumberFormatForPercent(pBuilder->weld_label(u"STR_DLG_NUMBERFORMAT_FOR_PERCENTAGE_VALUE"_ustr))
    , m_xCBCategory(pBuilder->weld_check_button(u"CB_CATEGORY"_ustr))
    , m_xCBSymbol(pBuilder->weld_check_button(u"CB_SYMBOL"_ustr))
    , m_xCBDataSeries(pBuilder->weld_check_button(u"CB_DATA_SERIES_NAME"_ustr))
    , m_xCBWrapText(pBuilder->weld_check_button(u"CB_WRAP_TEXT"_ustr))
    , m_xLB_Separator(pBuilder->weld_combo_box(u"LB_TEXT_SEPARATOR"_ustr))
    , m_xLB_LabelPlacement(pBuilder->weld_combo_box(u"LB_LABEL_PLACEMENT"_ustr))
    , m_xBxOrientation(pBuilder->weld_widget(u"boxORIENTATION"_ustr))
    , m_xFT_Dial(pBuilder->weld_label(u"CT_LABEL_DIAL"_ustr))
    , m_xNF_Degrees(pBuilder->weld_metric_spin_button(u"NF_LABEL_DEGREES"_ustr, FieldUnit::DEGREE))
    , m_xBxTextDirection(pBuilder->weld_widget(u"boxTXT_DIRECTION"_ustr))
    , m_aLB_TextDirection(pBuilder->weld_combo_box(u"LB_LABEL_TEXTDIR"_ustr))
    , m_xDC_Dial(new svx::DialControl)
    , m_xDC_DialWin(new weld::CustomWeld(*pBuilder, u"CT_DIAL"_ustr, *m_xDC_Dial))
    , m_xCBCustomLeaderLines(pBuilder->weld_check_button(u"CB_CUSTOM_LEADER_LINES"_ustr))
{
    m_xDC_Dial->SetText(m_xFT_Dial->get_label());

    //fill label placement list
    std::map< sal_Int32, OUString > aPlacementToStringMap;
    for( sal_Int32 nEnum=0; nEnum<m_xLB_LabelPlacement->get_count(); ++nEnum )
        aPlacementToStringMap[nEnum] = m_xLB_LabelPlacement->get_text(static_cast<sal_uInt16>(nEnum));


    std::vector< sal_Int32 > aAvailablePlacementList;
    if( const SfxIntegerListItem* pPlacementsItem = rInAttrs.GetItemIfSet(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS) )
        aAvailablePlacementList = pPlacementsItem->GetList();

    m_xLB_LabelPlacement->clear();
    for( size_t nN=0; nN<aAvailablePlacementList.size(); ++nN )
    {
        sal_uInt16 nListBoxPos = static_cast<sal_uInt16>( nN );
        sal_Int32 nPlacement = aAvailablePlacementList[nN];
        m_aPlacementToListBoxMap[nPlacement]=nListBoxPos;
        m_aListBoxToPlacementMap[nListBoxPos]=nPlacement;
        m_xLB_LabelPlacement->append_text( aPlacementToStringMap[nPlacement] );
    }

    //some click handler
    m_xPB_NumberFormatForValue->connect_clicked( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_xPB_NumberFormatForPercent->connect_clicked( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_xCBNumber->connect_toggled( LINK( this, DataLabelResources, CheckHdl ));
    m_xCBPercent->connect_toggled( LINK( this, DataLabelResources, CheckHdl ));
    m_xCBCategory->connect_toggled(  LINK( this, DataLabelResources, CheckHdl ));
    m_xCBSymbol->connect_toggled(  LINK( this, DataLabelResources, CheckHdl ));
    m_xCBDataSeries->connect_toggled(  LINK( this, DataLabelResources, CheckHdl ));
    m_xCBWrapText->connect_toggled(  LINK( this, DataLabelResources, CheckHdl ));
    m_xCBCustomLeaderLines->connect_toggled( LINK( this, DataLabelResources, CheckHdl ));

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent , m_bPercentSourceMixedState);

    if( const SfxBoolItem* pNoPercentValueItem = rInAttrs.GetItemIfSet(SCHATTR_DATADESCR_NO_PERCENTVALUE) )
    {
        bool bForbidPercentValue = pNoPercentValueItem->GetValue();
        if( bForbidPercentValue )
            m_xCBPercent->set_sensitive(false);
    }

    m_xDC_Dial->SetLinkedField(m_xNF_Degrees.get());
}

DataLabelResources::~DataLabelResources()
{
}

void DataLabelResources::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumberFormatter = pFormatter;
}

IMPL_LINK(DataLabelResources, NumberFormatDialogHdl, weld::Button&, rButton, void)
{
    if( !m_pPool || !m_pNumberFormatter )
    {
        OSL_FAIL("Missing item pool or number formatter");
        return;
    }

    if (&rButton == m_xPB_NumberFormatForValue.get() && !m_xCBNumber->get_active())
    {
        m_xCBNumber->set_active(true);
        m_aNumberState.bTriStateEnabled = false;
    }
    else if (&rButton == m_xPB_NumberFormatForPercent.get() && !m_xCBPercent->get_active())
    {
        m_xCBPercent->set_active(true);
        m_aPercentState.bTriStateEnabled = false;
    }

    SfxItemSet aNumberSet = NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( *m_pPool );
    aNumberSet.Put (SvxNumberInfoItem( m_pNumberFormatter, SID_ATTR_NUMBERFORMAT_INFO));

    bool bPercent = (&rButton == m_xPB_NumberFormatForPercent.get());

    sal_uInt32& rnFormatKey = bPercent ? m_nNumberFormatForPercent : m_nNumberFormatForValue;
    bool& rUseSourceFormat = bPercent ? m_bSourceFormatForPercent : m_bSourceFormatForValue;
    bool& rbMixedState = bPercent ? m_bPercentFormatMixedState : m_bNumberFormatMixedState;
    bool& rbSourceMixedState = bPercent ? m_bPercentSourceMixedState : m_bSourceFormatMixedState;

    if(!rbMixedState)
        aNumberSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, rnFormatKey ));
    aNumberSet.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, rUseSourceFormat ));

    NumberFormatDialog aDlg(m_pWindow, aNumberSet);
    if( bPercent )
        aDlg.set_title(m_xFT_NumberFormatForPercent->get_label());
    if (aDlg.run() != RET_OK)
        return;

    const SfxItemSet* pResult = aDlg.GetOutputItemSet();
    if( pResult )
    {
        bool bOldSource = rUseSourceFormat;
        sal_uInt32 nOldFormat = rnFormatKey;
        bool bOldMixedState = rbMixedState || rbSourceMixedState;

        rbMixedState = !lcl_ReadNumberFormatFromItemSet( *pResult, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, rnFormatKey, rUseSourceFormat, rbSourceMixedState );

        //todo this maybe can be removed when the numberformatter dialog does handle mixed state for source format correctly
        if( bOldMixedState && bOldSource == rUseSourceFormat && nOldFormat == rnFormatKey )
            rbMixedState = rbSourceMixedState = true;
    }
}

IMPL_LINK(DataLabelResources, CheckHdl, weld::Toggleable&, rToggle, void)
{
    if (&rToggle == m_xCBNumber.get())
        m_aNumberState.ButtonToggled(rToggle);
    else if (&rToggle == m_xCBPercent.get())
        m_aPercentState.ButtonToggled(rToggle);
    else if (&rToggle == m_xCBCategory.get())
        m_aCategoryState.ButtonToggled(rToggle);
    else if (&rToggle == m_xCBSymbol.get())
        m_aSymbolState.ButtonToggled(rToggle);
    else if (&rToggle == m_xCBDataSeries.get())
        m_aDataSeriesState.ButtonToggled(rToggle);
    else if (&rToggle == m_xCBWrapText.get())
        m_aWrapTextState.ButtonToggled(rToggle);
    else if (&rToggle == m_xCBCustomLeaderLines.get())
        m_aCustomLeaderLinesState.ButtonToggled(rToggle);
    EnableControls();
}

void DataLabelResources::EnableControls()
{
    m_xCBSymbol->set_sensitive( m_xCBNumber->get_active() || (m_xCBPercent->get_active() && m_xCBPercent->get_sensitive())
    || m_xCBCategory->get_active() || m_xCBDataSeries->get_active());

    m_xCBWrapText->set_sensitive( m_xCBNumber->get_active() || (m_xCBPercent->get_active() && m_xCBPercent->get_sensitive())
    || m_xCBCategory->get_active() || m_xCBDataSeries->get_active() );

    // Enable or disable separator, placement and direction based on the check
    // box states. Note that the check boxes are tri-state.
    {
        tools::Long nNumberOfCheckedLabelParts = 0;
        if (m_xCBNumber->get_state() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;
        if (m_xCBPercent->get_state() != TRISTATE_FALSE && m_xCBPercent->get_sensitive())
            ++nNumberOfCheckedLabelParts;
        if (m_xCBCategory->get_state() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;
        if (m_xCBDataSeries->get_state() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;

        m_xLB_Separator->set_sensitive( nNumberOfCheckedLabelParts > 1 );

        bool bEnableTextDir = nNumberOfCheckedLabelParts > 0;
        m_xBxTextDirection->set_sensitive( bEnableTextDir );
        bool bEnablePlacement = nNumberOfCheckedLabelParts > 0 && m_xLB_LabelPlacement->get_count()>1;
        m_xLB_LabelPlacement->set_sensitive( bEnablePlacement );
    }

    m_xPB_NumberFormatForValue->set_sensitive( m_pNumberFormatter && m_xCBNumber->get_active() );
    m_xPB_NumberFormatForPercent->set_sensitive( m_pNumberFormatter && m_xCBPercent->get_active() && m_xCBPercent->get_sensitive() );

    bool bEnableRotation = (m_xCBNumber->get_active() || m_xCBPercent->get_active()
                            || m_xCBCategory->get_active() || m_xCBDataSeries->get_active());
    m_xBxOrientation->set_sensitive(bEnableRotation);
}

void DataLabelResources::FillItemSet( SfxItemSet* rOutAttrs ) const
{
    if( m_xCBNumber->get_active() )
    {
        if( !m_bNumberFormatMixedState )
            rOutAttrs->Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, m_nNumberFormatForValue ));
        if( !m_bSourceFormatMixedState )
            rOutAttrs->Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, m_bSourceFormatForValue ));
    }
    if( m_xCBPercent->get_active() )
    {
        if( !m_bPercentFormatMixedState )
            rOutAttrs->Put( SfxUInt32Item( SCHATTR_PERCENT_NUMBERFORMAT_VALUE, m_nNumberFormatForPercent ));
        if( !m_bPercentSourceMixedState )
            rOutAttrs->Put( SfxBoolItem( SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_bSourceFormatForPercent ));
    }

    if( m_xCBNumber->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_NUMBER, m_xCBNumber->get_active() ) );
    if( m_xCBPercent->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_PERCENTAGE, m_xCBPercent->get_active() ) );
    if( m_xCBCategory->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_CATEGORY, m_xCBCategory->get_active() ) );
    if( m_xCBSymbol->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYMBOL, m_xCBSymbol->get_active()) );
    if( m_xCBDataSeries->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME, m_xCBDataSeries->get_active()) );
    if( m_xCBWrapText->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_WRAP_TEXT, m_xCBWrapText->get_active()) );
    if( m_xCBCustomLeaderLines->get_state() != TRISTATE_INDET )
        rOutAttrs->Put(SfxBoolItem( SCHATTR_DATADESCR_CUSTOM_LEADER_LINES, m_xCBCustomLeaderLines->get_active()) );

    auto const aSep = our_aLBEntryMap[m_xLB_Separator->get_active()];
    rOutAttrs->Put( SfxStringItem( SCHATTR_DATADESCR_SEPARATOR, OUString(aSep)) );

    std::map< sal_uInt16, sal_Int32 >::const_iterator aIt( m_aListBoxToPlacementMap.find(m_xLB_LabelPlacement->get_active()) );
    if(aIt!=m_aListBoxToPlacementMap.end())
    {
        sal_Int32 nValue = aIt->second;
        rOutAttrs->Put( SfxInt32Item( SCHATTR_DATADESCR_PLACEMENT, nValue ) );
    }

    if (m_aLB_TextDirection.get_active() != -1)
        rOutAttrs->Put( SvxFrameDirectionItem( m_aLB_TextDirection.get_active_id(), EE_PARA_WRITINGDIR ) );

    if( m_xDC_Dial->IsVisible() )
    {
        Degree100 nDegrees = m_xDC_Dial->GetRotation();
        rOutAttrs->Put(SdrAngleItem( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }
}

void DataLabelResources::Reset(const SfxItemSet& rInAttrs)
{
    // default state
    m_xCBSymbol->set_sensitive( false );

    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_NUMBER, *m_xCBNumber, m_aNumberState );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_PERCENTAGE, *m_xCBPercent, m_aPercentState );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_CATEGORY, *m_xCBCategory, m_aCategoryState );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_SYMBOL, *m_xCBSymbol, m_aSymbolState );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME, *m_xCBDataSeries, m_aDataSeriesState );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_WRAP_TEXT, *m_xCBWrapText, m_aWrapTextState );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_CUSTOM_LEADER_LINES, *m_xCBCustomLeaderLines, m_aCustomLeaderLinesState );

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent ,  m_bPercentSourceMixedState);

    if( const SfxStringItem* pSeparatorItem = rInAttrs.GetItemIfSet(SCHATTR_DATADESCR_SEPARATOR) )
       for(size_t i=0; i < std::size(our_aLBEntryMap); ++i )
       {
          if( our_aLBEntryMap[i] == pSeparatorItem->GetValue())
              m_xLB_Separator->set_active( i );
       }
    else
        m_xLB_Separator->set_active( 0 );

    if( const SfxInt32Item* pPlacementItem = rInAttrs.GetItemIfSet(SCHATTR_DATADESCR_PLACEMENT) )
    {
        sal_Int32 nPlacement = pPlacementItem->GetValue();
        std::map< sal_Int32, sal_uInt16 >::const_iterator aIt( m_aPlacementToListBoxMap.find(nPlacement) );
        if(aIt!=m_aPlacementToListBoxMap.end())
        {
            sal_uInt16 nPos = aIt->second;
            m_xLB_LabelPlacement->set_active( nPos );
        }
        else
            m_xLB_LabelPlacement->set_active(-1);
    }
    else
        m_xLB_LabelPlacement->set_active(-1);

    if( const SvxFrameDirectionItem* pDirectionItem = rInAttrs.GetItemIfSet(EE_PARA_WRITINGDIR) )
        m_aLB_TextDirection.set_active_id( pDirectionItem->GetValue() );

    if( const SdrAngleItem* pAngleItem = rInAttrs.GetItemIfSet( SCHATTR_TEXT_DEGREES ) )
    {
        Degree100 nDegrees = pAngleItem->GetValue();
        m_xDC_Dial->SetRotation( nDegrees );
    }
    else
        m_xDC_Dial->SetRotation( 0_deg100 );

    EnableControls();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
