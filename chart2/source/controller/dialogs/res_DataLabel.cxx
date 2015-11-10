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

#include "chartview/ChartSfxItemIds.hxx"
#include "dlg_NumberFormat.hxx"

#include <svx/numinf.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/ilstitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svxids.hrc>
#include <svl/zforlist.hxx>
#include <svtools/controldims.hrc>

namespace chart
{

namespace
{

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

void lcl_setBoolItemToCheckBox( const SfxItemSet& rInAttrs, sal_uInt16 nWhichId, CheckBox& rCheckbox )
{
    rCheckbox.EnableTriState( false );

    const SfxPoolItem *pPoolItem = nullptr;
    if( rInAttrs.GetItemState(nWhichId, true, &pPoolItem) == SfxItemState::SET )
        rCheckbox.Check( static_cast<const SfxBoolItem*>(pPoolItem)->GetValue() );
    else
    {
        rCheckbox.EnableTriState();
        rCheckbox.SetState( TRISTATE_INDET );
    }
}

}//end anonymous namespace

DataLabelResources::DataLabelResources(VclBuilderContainer* pWindow, vcl::Window* pParent, const SfxItemSet& rInAttrs )
    :
    m_pNumberFormatter(nullptr),
    m_bNumberFormatMixedState(true),
    m_bPercentFormatMixedState(true),
    m_nNumberFormatForValue(0),
    m_nNumberFormatForPercent(11),
    m_bSourceFormatMixedState(true),
    m_bPercentSourceMixedState(true),
    m_bSourceFormatForValue(true),
    m_bSourceFormatForPercent(true),
    m_pWindow(pParent),
    m_pPool(rInAttrs.GetPool())
{
    pWindow->get(m_pCBNumber, "CB_VALUE_AS_NUMBER");
    pWindow->get(m_pPB_NumberFormatForValue, "PB_NUMBERFORMAT");
    pWindow->get(m_pCBPercent, "CB_VALUE_AS_PERCENTAGE");
    pWindow->get(m_pPB_NumberFormatForPercent, "PB_PERCENT_NUMBERFORMAT");
    pWindow->get(m_pFT_NumberFormatForPercent,"STR_DLG_NUMBERFORMAT_FOR_PERCENTAGE_VALUE");
    pWindow->get(m_pCBCategory, "CB_CATEGORY");
    pWindow->get(m_pCBSymbol, "CB_SYMBOL");
    pWindow->get(m_pCBWrapText, "CB_WRAP_TEXT");

    pWindow->get(m_pBxLabelPlacement, "boxPLACEMENT");
    pWindow->get(m_pLB_LabelPlacement, "LB_LABEL_PLACEMENT");

    pWindow->get(m_pDC_Dial, "CT_DIAL");
    pWindow->get(m_pFT_Dial, "CT_LABEL_DIAL");

    m_pDC_Dial->SetText(m_pFT_Dial->GetText());

    pWindow->get(m_pNF_Degrees, "NF_LABEL_DEGREES");
    pWindow->get(m_pLB_TextDirection, "LB_LABEL_TEXTDIR");
    pWindow->get(m_pBxTextDirection, "boxTXT_DIRECTION");
    pWindow->get(m_pBxOrientation, "boxORIENTATION");

    pWindow->get(m_pSeparatorResources, "boxSEPARATOR");
    pWindow->get(m_pLB_Separator, "LB_TEXT_SEPARATOR");

    m_aEntryMap[0] = " ";
    m_aEntryMap[1] = ", ";
    m_aEntryMap[2] = "; ";
    m_aEntryMap[3] = "\n" ;

    //fill label placement list
    std::map< sal_Int32, OUString > aPlacementToStringMap;
    for( sal_Int32 nEnum=0; nEnum<m_pLB_LabelPlacement->GetEntryCount(); ++nEnum )
        aPlacementToStringMap[nEnum] = m_pLB_LabelPlacement->GetEntry(static_cast<sal_uInt16>(nEnum));

    ::com::sun::star::uno::Sequence < sal_Int32 > aAvailabelPlacementList;
    const SfxPoolItem *pPoolItem = nullptr;
    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, true, &pPoolItem) == SfxItemState::SET )
        aAvailabelPlacementList = static_cast<const SfxIntegerListItem*>(pPoolItem)->GetConstSequence();

    m_pLB_LabelPlacement->Clear();
    for( sal_Int32 nN=0; nN<aAvailabelPlacementList.getLength(); ++nN )
    {
        sal_uInt16 nListBoxPos = static_cast<sal_uInt16>( nN );
        sal_Int32 nPlacement = aAvailabelPlacementList[nN];
        m_aPlacementToListBoxMap[nPlacement]=nListBoxPos;
        m_aListBoxToPlacementMap[nListBoxPos]=nPlacement;
        m_pLB_LabelPlacement->InsertEntry( aPlacementToStringMap[nPlacement] );
    }
    m_pLB_LabelPlacement->SetDropDownLineCount(m_pLB_LabelPlacement->GetEntryCount());

    //some click handler
    m_pPB_NumberFormatForValue->SetClickHdl( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_pPB_NumberFormatForPercent->SetClickHdl( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_pCBNumber->SetClickHdl( LINK( this, DataLabelResources, CheckHdl ));
    m_pCBPercent->SetClickHdl( LINK( this, DataLabelResources, CheckHdl ));
    m_pCBCategory->SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));
    m_pCBSymbol->SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));
    m_pCBWrapText->SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent , m_bPercentSourceMixedState);

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_NO_PERCENTVALUE, true, &pPoolItem) == SfxItemState::SET )
    {
        bool bForbidPercentValue = (static_cast< const SfxBoolItem & >( rInAttrs.Get( SCHATTR_DATADESCR_NO_PERCENTVALUE )).GetValue() );
        if( bForbidPercentValue )
            m_pCBPercent->Enable(false);
    }

    m_pDC_Dial->SetLinkedField( m_pNF_Degrees );
}

DataLabelResources::~DataLabelResources()
{
}

void DataLabelResources::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumberFormatter = pFormatter;
}

IMPL_LINK_TYPED( DataLabelResources, NumberFormatDialogHdl, Button *, pButton, void )
{
    if( !m_pPool || !m_pNumberFormatter )
    {
        OSL_FAIL("Missing item pool or number formatter");
        return;
    }

    if( pButton == m_pPB_NumberFormatForValue && !m_pCBNumber->IsChecked())
        m_pCBNumber->Check();
    else if( pButton == m_pPB_NumberFormatForPercent && !m_pCBPercent->IsChecked())
        m_pCBPercent->Check();

    SfxItemSet aNumberSet = NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( *m_pPool );
    aNumberSet.Put (SvxNumberInfoItem( m_pNumberFormatter, (const sal_uInt16)SID_ATTR_NUMBERFORMAT_INFO));

    bool bPercent = ( pButton == m_pPB_NumberFormatForPercent );

    sal_uLong& rnFormatKey = bPercent ? m_nNumberFormatForPercent : m_nNumberFormatForValue;
    bool& rUseSourceFormat = bPercent ? m_bSourceFormatForPercent : m_bSourceFormatForValue;
    bool& rbMixedState = bPercent ? m_bPercentFormatMixedState : m_bNumberFormatMixedState;
    bool& rbSourceMixedState = bPercent ? m_bPercentSourceMixedState : m_bSourceFormatMixedState;

    if(!rbMixedState)
        aNumberSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, rnFormatKey ));
    aNumberSet.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, rUseSourceFormat ));

    ScopedVclPtrInstance< NumberFormatDialog > aDlg(m_pWindow, aNumberSet);
    if( bPercent )
        aDlg->SetText( m_pFT_NumberFormatForPercent->GetText());
    if( RET_OK == aDlg->Execute() )
    {
        const SfxItemSet* pResult = aDlg->GetOutputItemSet();
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
}

IMPL_LINK_TYPED( DataLabelResources, CheckHdl, Button*, pBox, void )
{
    if( pBox )
        static_cast<CheckBox*>(pBox)->EnableTriState( false );
    EnableControls();
}

void DataLabelResources::EnableControls()
{
    m_pCBSymbol->Enable( m_pCBNumber->IsChecked() || (m_pCBPercent->IsChecked() && m_pCBPercent->IsEnabled())
    || m_pCBCategory->IsChecked() );

    m_pCBWrapText->Enable( m_pCBNumber->IsChecked() || (m_pCBPercent->IsChecked() && m_pCBPercent->IsEnabled())
    || m_pCBCategory->IsChecked() );

    // Enable or disable separator, placement and direction based on the check
    // box states. Note that the check boxes are tri-state.
    {
        long nNumberOfCheckedLabelParts = 0;
        if (m_pCBNumber->GetState() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;
        if (m_pCBPercent->GetState() != TRISTATE_FALSE && m_pCBPercent->IsEnabled())
            ++nNumberOfCheckedLabelParts;
        if (m_pCBCategory->GetState() != TRISTATE_FALSE)
            ++nNumberOfCheckedLabelParts;

        m_pSeparatorResources->Enable( nNumberOfCheckedLabelParts > 1 );

        bool bEnableTextDir = nNumberOfCheckedLabelParts > 0;
        m_pBxTextDirection->Enable( bEnableTextDir );
        bool bEnablePlacement = nNumberOfCheckedLabelParts > 0 && m_pLB_LabelPlacement->GetEntryCount()>1;
        m_pBxLabelPlacement->Enable( bEnablePlacement );
    }

    m_pPB_NumberFormatForValue->Enable( m_pNumberFormatter && m_pCBNumber->IsChecked() );
    m_pPB_NumberFormatForPercent->Enable( m_pNumberFormatter && m_pCBPercent->IsChecked() && m_pCBPercent->IsEnabled() );

    bool bEnableRotation = ( m_pCBNumber->IsChecked() || m_pCBPercent->IsChecked() || m_pCBCategory->IsChecked() );
    m_pBxOrientation->Enable( bEnableRotation );
}

bool DataLabelResources::FillItemSet( SfxItemSet* rOutAttrs ) const
{
    if( m_pCBNumber->IsChecked() )
    {
        if( !m_bNumberFormatMixedState )
            rOutAttrs->Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, m_nNumberFormatForValue ));
        if( !m_bSourceFormatMixedState )
            rOutAttrs->Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, m_bSourceFormatForValue ));
    }
    if( m_pCBPercent->IsChecked() )
    {
        if( !m_bPercentFormatMixedState )
            rOutAttrs->Put( SfxUInt32Item( SCHATTR_PERCENT_NUMBERFORMAT_VALUE, m_nNumberFormatForPercent ));
        if( !m_bPercentSourceMixedState )
            rOutAttrs->Put( SfxBoolItem( SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_bSourceFormatForPercent ));
    }

    if( m_pCBNumber->GetState()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_NUMBER, m_pCBNumber->IsChecked() ) );
    if( m_pCBPercent->GetState()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_PERCENTAGE, m_pCBPercent->IsChecked() ) );
    if( m_pCBCategory->GetState()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_CATEGORY, m_pCBCategory->IsChecked() ) );
    if( m_pCBSymbol->GetState()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYMBOL, m_pCBSymbol->IsChecked()) );
    if( m_pCBWrapText->GetState()!= TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_DATADESCR_WRAP_TEXT, m_pCBWrapText->IsChecked()) );

    OUString aSep = m_aEntryMap[m_pLB_Separator->GetSelectEntryPos()];
    rOutAttrs->Put( SfxStringItem( SCHATTR_DATADESCR_SEPARATOR, aSep) );

    ::std::map< sal_uInt16, sal_Int32 >::const_iterator aIt( m_aListBoxToPlacementMap.find(m_pLB_LabelPlacement->GetSelectEntryPos()) );
    if(aIt!=m_aListBoxToPlacementMap.end())
    {
        sal_Int32 nValue = aIt->second;
        rOutAttrs->Put( SfxInt32Item( SCHATTR_DATADESCR_PLACEMENT, nValue ) );
    }

    if( m_pLB_TextDirection->GetSelectEntryCount() > 0 )
        rOutAttrs->Put( SvxFrameDirectionItem( m_pLB_TextDirection->GetSelectEntryValue(), EE_PARA_WRITINGDIR ) );

    if( m_pDC_Dial->IsVisible() )
    {
        sal_Int32 nDegrees = m_pDC_Dial->GetRotation();
        rOutAttrs->Put(SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }

    return true;
}

void DataLabelResources::Reset(const SfxItemSet& rInAttrs)
{
    // default state
    m_pCBSymbol->Enable( false );

    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_NUMBER, *m_pCBNumber );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_PERCENTAGE, *m_pCBPercent );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_CATEGORY, *m_pCBCategory );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_SYMBOL, *m_pCBSymbol );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_WRAP_TEXT, *m_pCBWrapText );

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent ,  m_bPercentSourceMixedState);

    const SfxPoolItem *pPoolItem = nullptr;
    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_SEPARATOR, true, &pPoolItem) == SfxItemState::SET )
       for(sal_Int32 i=0; i < NUMBER_SEPARATORS; ++i )
       {
          if( m_aEntryMap[i] == static_cast<const SfxStringItem*>(pPoolItem)->GetValue())
              m_pLB_Separator->SelectEntryPos( i );
       }
    else
        m_pLB_Separator->SelectEntryPos( 0 );

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_PLACEMENT, true, &pPoolItem) == SfxItemState::SET )
    {
        sal_Int32 nPlacement = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        ::std::map< sal_Int32, sal_uInt16 >::const_iterator aIt( m_aPlacementToListBoxMap.find(nPlacement) );
        if(aIt!=m_aPlacementToListBoxMap.end())
        {
            sal_uInt16 nPos = aIt->second;
            m_pLB_LabelPlacement->SelectEntryPos( nPos );
        }
        else
            m_pLB_LabelPlacement->SetNoSelection();
    }
    else
        m_pLB_LabelPlacement->SetNoSelection();

    if( rInAttrs.GetItemState(EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_pLB_TextDirection->SelectEntryValue( SvxFrameDirection(static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue()) );

    if( rInAttrs.GetItemState( SCHATTR_TEXT_DEGREES, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nDegrees = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_pDC_Dial->SetRotation( nDegrees );
    }
    else
        m_pDC_Dial->SetRotation( 0 );

    EnableControls();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
