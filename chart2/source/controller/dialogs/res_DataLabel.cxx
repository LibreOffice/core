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
#include <svx/svxids.hrc>
#include <osl/diagnose.h>

namespace chart
{

namespace
{

const OUStringLiteral our_aLBEntryMap[] = {u" ", u", ", u"; ", u"\n", u". "};

bool lcl_ReadNumberFormatFromItemSet( const SfxItemSet& rSet, sal_uInt16 nValueWhich, sal_uInt16 nSourceFormatWhich, sal_uLong& rnFormatKeyOut, bool& rbSourceFormatOut, bool& rbSourceFormatMixedStateOut )
{
    bool bSet = false;
    const SfxPoolItem *pItem1 = nullptr;
    if( rSet.GetItemState( nValueWhich, true, &pItem1 ) == SfxItemState::SET )
    {
        const SfxUInt32Item * pNumItem = dynamic_cast< const SfxUInt32Item * >( pItem1 );
        if( pNumItem )
        {
            rnFormatKeyOut = pNumItem->GetValue();
            bSet = true;
        }
    }

    rbSourceFormatMixedStateOut=true;
    const SfxPoolItem *pItem2 = nullptr;
    if( rSet.GetItemState( nSourceFormatWhich, true, &pItem2 ) == SfxItemState::SET )
    {
        const SfxBoolItem * pBoolItem = dynamic_cast< const SfxBoolItem * >( pItem2 );
        if( pBoolItem )
        {
            rbSourceFormatOut = pBoolItem->GetValue();
            rbSourceFormatMixedStateOut=false;
        }
    }
    return bSet;
}

void lcl_setBoolItemToCheckBox(const SfxItemSet& rInAttrs, sal_uInt16 nWhichId, weld::CheckButton& rCheckbox)
{
    const SfxPoolItem *pPoolItem = nullptr;
    if( rInAttrs.GetItemState(nWhichId, true, &pPoolItem) == SfxItemState::SET )
        rCheckbox.set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    else
        rCheckbox.set_state(TRISTATE_INDET);
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
    , m_xCBNumber(pBuilder->weld_check_button("CB_VALUE_AS_NUMBER"))
    , m_xPB_NumberFormatForValue(pBuilder->weld_button("PB_NUMBERFORMAT"))
    , m_xCBPercent(pBuilder->weld_check_button("CB_VALUE_AS_PERCENTAGE"))
    , m_xPB_NumberFormatForPercent(pBuilder->weld_button("PB_PERCENT_NUMBERFORMAT"))
    , m_xFT_NumberFormatForPercent(pBuilder->weld_label("STR_DLG_NUMBERFORMAT_FOR_PERCENTAGE_VALUE"))
    , m_xCBCategory(pBuilder->weld_check_button("CB_CATEGORY"))
    , m_xCBSymbol(pBuilder->weld_check_button("CB_SYMBOL"))
    , m_xCBWrapText(pBuilder->weld_check_button("CB_WRAP_TEXT"))
    , m_xSeparatorResources(pBuilder->weld_widget("boxSEPARATOR"))
    , m_xLB_Separator(pBuilder->weld_combo_box("LB_TEXT_SEPARATOR"))
    , m_xBxLabelPlacement(pBuilder->weld_widget("boxPLACEMENT"))
    , m_xLB_LabelPlacement(pBuilder->weld_combo_box("LB_LABEL_PLACEMENT"))
    , m_xBxOrientation(pBuilder->weld_widget("boxORIENTATION"))
    , m_xFT_Dial(pBuilder->weld_label("CT_LABEL_DIAL"))
    , m_xNF_Degrees(pBuilder->weld_metric_spin_button("NF_LABEL_DEGREES", FieldUnit::DEGREE))
    , m_xBxTextDirection(pBuilder->weld_widget("boxTXT_DIRECTION"))
    , m_xLB_TextDirection(new TextDirectionListBox(pBuilder->weld_combo_box("LB_LABEL_TEXTDIR")))
    , m_xDC_Dial(new svx::DialControl)
    , m_xDC_DialWin(new weld::CustomWeld(*pBuilder, "CT_DIAL", *m_xDC_Dial))
{
    m_xDC_Dial->SetText(m_xFT_Dial->get_label());

    //fill label placement list
    std::map< sal_Int32, OUString > aPlacementToStringMap;
    for( sal_Int32 nEnum=0; nEnum<m_xLB_LabelPlacement->get_count(); ++nEnum )
        aPlacementToStringMap[nEnum] = m_xLB_LabelPlacement->get_text(static_cast<sal_uInt16>(nEnum));


    std::vector< sal_Int32 > aAvailablePlacementList;
    const SfxPoolItem *pPoolItem = nullptr;
    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, true, &pPoolItem) == SfxItemState::SET )
        aAvailablePlacementList = static_cast<const SfxIntegerListItem*>(pPoolItem)->GetList();

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
    m_xCBWrapText->connect_toggled(  LINK( this, DataLabelResources, CheckHdl ));

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent , m_bPercentSourceMixedState);

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_NO_PERCENTVALUE, true, &pPoolItem) == SfxItemState::SET )
    {
        bool bForbidPercentValue = rInAttrs.Get( SCHATTR_DATADESCR_NO_PERCENTVALUE ).GetValue();
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
        m_xCBNumber->set_active(true);
    else if (&rButton == m_xPB_NumberFormatForPercent.get() && !m_xCBPercent->get_active())
        m_xCBPercent->set_active(true);

    SfxItemSet aNumberSet = NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( *m_pPool );
    aNumberSet.Put (SvxNumberInfoItem( m_pNumberFormatter, static_cast<sal_uInt16>(SID_ATTR_NUMBERFORMAT_INFO)));

    bool bPercent = (&rButton == m_xPB_NumberFormatForPercent.get());

    sal_uLong& rnFormatKey = bPercent ? m_nNumberFormatForPercent : m_nNumberFormatForValue;
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
        sal_uLong nOldFormat = rnFormatKey;
        bool bOldMixedState = rbMixedState || rbSourceMixedState;

        rbMixedState = !lcl_ReadNumberFormatFromItemSet( *pResult, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, rnFormatKey, rUseSourceFormat, rbSourceMixedState );

        //todo this maybe can be removed when the numberformatter dialog does handle mixed state for source format correctly
        if( bOldMixedState && bOldSource == rUseSourceFormat && nOldFormat == rnFormatKey )
            rbMixedState = rbSourceMixedState = true;
    }
}

IMPL_LINK_NOARG(DataLabelResources, CheckHdl, weld::ToggleButton&, void)
{
    EnableControls();
}

void DataLabelResources::EnableControls()
{
    m_xCBSymbol->set_sensitive( m_xCBNumber->get_active() || (m_xCBPercent->get_active() && m_xCBPercent->get_sensitive())
    || m_xCBCategory->get_active() );

    m_xCBWrapText->set_sensitive( m_xCBNumber->get_active() || (m_xCBPercent->get_active() && m_xCBPercent->get_sensitive())
    || m_xCBCategory->get_active() );

    // Enable or disable separator, placement and direction based on the check
    // box states. Note that the check boxes are tri-state.
    {
        long nNumberOfCheckedLabelParts = 0;
        if (m_xCBNumber->get_state() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;
        if (m_xCBPercent->get_state() != TRISTATE_FALSE && m_xCBPercent->get_sensitive())
            ++nNumberOfCheckedLabelParts;
        if (m_xCBCategory->get_state() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;

        m_xSeparatorResources->set_sensitive( nNumberOfCheckedLabelParts > 1 );

        bool bEnableTextDir = nNumberOfCheckedLabelParts > 0;
        m_xBxTextDirection->set_sensitive( bEnableTextDir );
        bool bEnablePlacement = nNumberOfCheckedLabelParts > 0 && m_xLB_LabelPlacement->get_count()>1;
        m_xBxLabelPlacement->set_sensitive( bEnablePlacement );
    }

    m_xPB_NumberFormatForValue->set_sensitive( m_pNumberFormatter && m_xCBNumber->get_active() );
    m_xPB_NumberFormatForPercent->set_sensitive( m_pNumberFormatter && m_xCBPercent->get_active() && m_xCBPercent->get_sensitive() );

    bool bEnableRotation = ( m_xCBNumber->get_active() || m_xCBPercent->get_active() || m_xCBCategory->get_active() );
    m_xBxOrientation->set_sensitive( bEnableRotation );
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
    if( m_xCBWrapText->get_state()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_WRAP_TEXT, m_xCBWrapText->get_active()) );

    auto const aSep = our_aLBEntryMap[m_xLB_Separator->get_active()];
    rOutAttrs->Put( SfxStringItem( SCHATTR_DATADESCR_SEPARATOR, aSep) );

    std::map< sal_uInt16, sal_Int32 >::const_iterator aIt( m_aListBoxToPlacementMap.find(m_xLB_LabelPlacement->get_active()) );
    if(aIt!=m_aListBoxToPlacementMap.end())
    {
        sal_Int32 nValue = aIt->second;
        rOutAttrs->Put( SfxInt32Item( SCHATTR_DATADESCR_PLACEMENT, nValue ) );
    }

    if (m_xLB_TextDirection->get_active() != -1)
        rOutAttrs->Put( SvxFrameDirectionItem( m_xLB_TextDirection->get_active_id(), EE_PARA_WRITINGDIR ) );

    if( m_xDC_Dial->IsVisible() )
    {
        sal_Int32 nDegrees = m_xDC_Dial->GetRotation();
        rOutAttrs->Put(SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }
}

void DataLabelResources::Reset(const SfxItemSet& rInAttrs)
{
    // default state
    m_xCBSymbol->set_sensitive( false );

    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_NUMBER, *m_xCBNumber );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_PERCENTAGE, *m_xCBPercent );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_CATEGORY, *m_xCBCategory );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_SYMBOL, *m_xCBSymbol );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_WRAP_TEXT, *m_xCBWrapText );

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent ,  m_bPercentSourceMixedState);

    const SfxPoolItem *pPoolItem = nullptr;
    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_SEPARATOR, true, &pPoolItem) == SfxItemState::SET )
       for(size_t i=0; i < SAL_N_ELEMENTS(our_aLBEntryMap); ++i )
       {
          if( our_aLBEntryMap[i] == static_cast<const SfxStringItem*>(pPoolItem)->GetValue())
              m_xLB_Separator->set_active( i );
       }
    else
        m_xLB_Separator->set_active( 0 );

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_PLACEMENT, true, &pPoolItem) == SfxItemState::SET )
    {
        sal_Int32 nPlacement = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
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

    if( rInAttrs.GetItemState(EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_xLB_TextDirection->set_active_id( static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue() );

    if( rInAttrs.GetItemState( SCHATTR_TEXT_DEGREES, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nDegrees = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_xDC_Dial->SetRotation( nDegrees );
    }
    else
        m_xDC_Dial->SetRotation( 0 );

    EnableControls();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
