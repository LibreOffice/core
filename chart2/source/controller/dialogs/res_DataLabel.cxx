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
#include "res_DataLabel_IDs.hrc"

#include "Strings.hrc"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "dlg_NumberFormat.hxx"

// header for class SvxNumberInfoItem
#include <svx/numinf.hxx>
#include <svl/eitem.hxx>
// for SfxUInt32Item
#include <svl/intitem.hxx>
// header for class SfxStringItem
#include <svl/stritem.hxx>
//SfxIntegerListItem
#include <svl/ilstitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

// header for define RET_OK
#include <vcl/msgbox.hxx>
#include <svx/svxids.hrc>
// header for SvNumberFormatter
#include <svl/zforlist.hxx>
#include <svtools/controldims.hrc>


//.............................................................................
namespace chart
{
//.............................................................................

namespace
{

bool lcl_ReadNumberFormatFromItemSet( const SfxItemSet& rSet, sal_uInt16 nValueWhich, sal_uInt16 nSourceFormatWhich, sal_uLong& rnFormatKeyOut, bool& rbSourceFormatOut, bool& rbSourceFormatMixedStateOut )
{
    bool bSet = false;
    const SfxPoolItem *pItem1 = NULL;
    if( rSet.GetItemState( nValueWhich, sal_True, &pItem1 ) == SFX_ITEM_SET )
    {
        const SfxUInt32Item * pNumItem = dynamic_cast< const SfxUInt32Item * >( pItem1 );
        if( pNumItem )
        {
            rnFormatKeyOut = pNumItem->GetValue();
            bSet = true;
        }
    }

    rbSourceFormatMixedStateOut=true;
    const SfxPoolItem *pItem2 = NULL;
    if( rSet.GetItemState( nSourceFormatWhich, sal_True, &pItem2 ) == SFX_ITEM_SET )
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
    rCheckbox.EnableTriState( sal_False );

    const SfxPoolItem *pPoolItem = NULL;
    if( rInAttrs.GetItemState(nWhichId, sal_True, &pPoolItem) == SFX_ITEM_SET )
        rCheckbox.Check( ((const SfxBoolItem*)pPoolItem)->GetValue() );
    else
    {
        rCheckbox.EnableTriState( sal_True );
        rCheckbox.SetState( STATE_DONTKNOW );
    }
}

}//end anonymous namespace

DataLabelResources::DataLabelResources( Window* pWindow, const SfxItemSet& rInAttrs )
    : m_aCBNumber(pWindow, SchResId(CB_VALUE_AS_NUMBER)),
    m_aPB_NumberFormatForValue(pWindow, SchResId(PB_NUMBERFORMAT)),
    m_aCBPercent(pWindow, SchResId(CB_VALUE_AS_PERCENTAGE)),
    m_aPB_NumberFormatForPercent(pWindow, SchResId(PB_PERCENT_NUMBERFORMAT)),
    m_aCBCategory(pWindow, SchResId(CB_CATEGORY)),
    m_aCBSymbol(pWindow, SchResId(CB_SYMBOL)),
    m_aSeparatorResources(pWindow),
    m_aFT_LabelPlacement(pWindow, SchResId(FT_LABEL_PLACEMENT)),
    m_aLB_LabelPlacement(pWindow, SchResId(LB_LABEL_PLACEMENT)),
    m_aFL_Rotate(pWindow, SchResId(FL_LABEL_ROTATE)),
    m_aDC_Dial(pWindow, SchResId(CT_LABEL_DIAL)),
    m_aFT_Degrees(pWindow, SchResId(FT_LABEL_DEGREES)),
    m_aNF_Degrees(pWindow, SchResId(NF_LABEL_DEGREES)),
    m_aFT_TextDirection(pWindow, SchResId(FT_LABEL_TEXTDIR)),
    m_aLB_TextDirection(pWindow, SchResId(LB_LABEL_TEXTDIR), &m_aFT_TextDirection),
    m_pNumberFormatter(0),
    m_bNumberFormatMixedState(true),
    m_bPercentFormatMixedState(true),
    m_nNumberFormatForValue(0),
    m_nNumberFormatForPercent(11),
    m_bSourceFormatMixedState(true),
    m_bPercentSourceMixedState(true),
    m_bSourceFormatForValue(true),
    m_bSourceFormatForPercent(true),
    m_pWindow(pWindow),
    m_pPool(rInAttrs.GetPool())
{
    //fill label placement list
    std::map< sal_Int32, OUString > aPlacementToStringMap;
    for( sal_Int32 nEnum=0; nEnum<m_aLB_LabelPlacement.GetEntryCount(); ++nEnum )
        aPlacementToStringMap[nEnum] = m_aLB_LabelPlacement.GetEntry(static_cast<sal_uInt16>(nEnum));

    ::com::sun::star::uno::Sequence < sal_Int32 > aAvailabelPlacementList;
    const SfxPoolItem *pPoolItem = NULL;
    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, sal_True, &pPoolItem) == SFX_ITEM_SET )
        aAvailabelPlacementList =((const SfxIntegerListItem*)pPoolItem)->GetConstSequence();

    m_aLB_LabelPlacement.Clear();
    for( sal_Int32 nN=0; nN<aAvailabelPlacementList.getLength(); ++nN )
    {
        sal_uInt16 nListBoxPos = static_cast<sal_uInt16>( nN );
        sal_Int32 nPlacement = aAvailabelPlacementList[nN];
        m_aPlacementToListBoxMap[nPlacement]=nListBoxPos;
        m_aListBoxToPlacementMap[nListBoxPos]=nPlacement;
        m_aLB_LabelPlacement.InsertEntry( aPlacementToStringMap[nPlacement] );
    }
    m_aLB_LabelPlacement.SetDropDownLineCount(m_aLB_LabelPlacement.GetEntryCount());

    //replace&resize push buttons and resize checkboxes
    Size aControlDistance( pWindow->LogicToPixel( Size(RSC_SP_CTRL_DESC_X,RSC_SP_CTRL_GROUP_Y), MapMode(MAP_APPFONT) ) );
    long nPBWidth = ::std::max( m_aPB_NumberFormatForPercent.CalcMinimumSize().getWidth(), m_aPB_NumberFormatForValue.CalcMinimumSize().getWidth() )
        + 20; //the min with is to small to fit, hm... so add alittle
    long nCBXWidth = ::std::max( m_aCBNumber.CalcMinimumSize().getWidth(), m_aCBPercent.CalcMinimumSize().getWidth() );
    long nNewPBXPos = m_aCBNumber.GetPosPixel().X() + nCBXWidth + aControlDistance.Width();
    long nPageWidth = pWindow->LogicToPixel( Size(260,185), MapMode(MAP_APPFONT) ).getWidth();
    if( nNewPBXPos + nPBWidth > nPageWidth )
    {
        if( nPBWidth > nPageWidth/2 )
            nPBWidth = nPageWidth/2;
        nNewPBXPos = nPageWidth-nPBWidth;
        nCBXWidth = nPageWidth-m_aCBNumber.GetPosPixel().X()-nPBWidth-aControlDistance.Width();
    }
    m_aPB_NumberFormatForValue.SetPosPixel( Point( nNewPBXPos, m_aPB_NumberFormatForValue.GetPosPixel().Y() ) );
    m_aPB_NumberFormatForPercent.SetPosPixel( Point( nNewPBXPos, m_aPB_NumberFormatForPercent.GetPosPixel().Y() ) );

    Size aPBSize( m_aPB_NumberFormatForPercent.GetSizePixel() );
    aPBSize.setWidth( nPBWidth );
    m_aPB_NumberFormatForValue.SetSizePixel( aPBSize );
    m_aPB_NumberFormatForPercent.SetSizePixel( aPBSize );

    Size aCBSize( m_aCBNumber.GetSizePixel() );
    aCBSize.setWidth(nCBXWidth);
    m_aCBNumber.SetSizePixel( aCBSize );
    m_aCBPercent.SetSizePixel( aCBSize );

    //place and align separator and label placement listboxes
    Size aSize( m_aFT_LabelPlacement.GetSizePixel() );
    aSize.setWidth( m_aFT_LabelPlacement.CalcMinimumSize().getWidth() );
    m_aFT_LabelPlacement.SetSizePixel(aSize);

    long nWantedMinLeftBorder = m_aFT_LabelPlacement.GetPosPixel().X() + aSize.getWidth () + aControlDistance.Width();

    m_aSeparatorResources.PositionBelowControl(m_aCBSymbol);
    m_aSeparatorResources.AlignListBoxWidthAndXPos( nWantedMinLeftBorder, -1, m_aLB_LabelPlacement.CalcMinimumSize().getWidth() );
    m_aSeparatorResources.Show(true);

    aSize = m_aLB_LabelPlacement.GetSizePixel();
    aSize.setWidth( m_aSeparatorResources.GetCurrentListBoxSize().getWidth() );
    m_aLB_LabelPlacement.SetSizePixel(aSize);
    m_aLB_LabelPlacement.SetPosPixel( Point( m_aSeparatorResources.GetCurrentListBoxPosition().X(), m_aLB_LabelPlacement.GetPosPixel().Y() ) );

    //some click handler
    m_aPB_NumberFormatForValue.SetClickHdl( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_aPB_NumberFormatForPercent.SetClickHdl( LINK( this, DataLabelResources, NumberFormatDialogHdl ) );
    m_aCBNumber.SetClickHdl( LINK( this, DataLabelResources, CheckHdl ));
    m_aCBPercent.SetClickHdl( LINK( this, DataLabelResources, CheckHdl ));
    m_aCBCategory.SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));
    m_aCBSymbol.SetClickHdl(  LINK( this, DataLabelResources, CheckHdl ));

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent , m_bPercentSourceMixedState);

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_NO_PERCENTVALUE, sal_True, &pPoolItem) == SFX_ITEM_SET )
    {
        bool bForbidPercentValue = (static_cast< const SfxBoolItem & >( rInAttrs.Get( SCHATTR_DATADESCR_NO_PERCENTVALUE )).GetValue() );
        if( bForbidPercentValue )
            m_aCBPercent.Enable(false);
    }

    m_aDC_Dial.SetLinkedField( &m_aNF_Degrees );
}

DataLabelResources::~DataLabelResources()
{
}

void DataLabelResources::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumberFormatter = pFormatter;
}

IMPL_LINK( DataLabelResources, NumberFormatDialogHdl, PushButton *, pButton )
{
    if( !m_pPool || !m_pNumberFormatter )
    {
        OSL_FAIL("Missing item pool or number formatter");
        return 1;
    }

    if( pButton == &m_aPB_NumberFormatForValue && !m_aCBNumber.IsChecked())
        m_aCBNumber.Check();
    else if( pButton == &m_aPB_NumberFormatForPercent && !m_aCBPercent.IsChecked())
        m_aCBPercent.Check();

    SfxItemSet aNumberSet = NumberFormatDialog::CreateEmptyItemSetForNumberFormatDialog( *m_pPool );
    aNumberSet.Put (SvxNumberInfoItem( m_pNumberFormatter, (const sal_uInt16)SID_ATTR_NUMBERFORMAT_INFO));

    bool bPercent = ( pButton == &m_aPB_NumberFormatForPercent );

    sal_uLong& rnFormatKey = bPercent ? m_nNumberFormatForPercent : m_nNumberFormatForValue;
    bool& rUseSourceFormat = bPercent ? m_bSourceFormatForPercent : m_bSourceFormatForValue;
    bool& rbMixedState = bPercent ? m_bPercentFormatMixedState : m_bNumberFormatMixedState;
    bool& rbSourceMixedState = bPercent ? m_bPercentSourceMixedState : m_bSourceFormatMixedState;

    if(!rbMixedState)
        aNumberSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, rnFormatKey ));
    aNumberSet.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, rUseSourceFormat ));

    NumberFormatDialog aDlg(m_pWindow, aNumberSet);
    if( bPercent )
        aDlg.SetText( SCH_RESSTR( STR_DLG_NUMBERFORMAT_FOR_PERCENTAGE_VALUE ) );
    if( RET_OK == aDlg.Execute() )
    {
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
    return 0;
}

IMPL_LINK( DataLabelResources, CheckHdl, CheckBox*, pBox )
{
    if( pBox )
        pBox->EnableTriState( sal_False );
    EnableControls();
    return 0;
}

void DataLabelResources::EnableControls()
{
    m_aCBSymbol.Enable( m_aCBNumber.IsChecked() || (m_aCBPercent.IsChecked() && m_aCBPercent.IsEnabled()) || m_aCBCategory.IsChecked() );

    // Enable or disable separator, placement and direction based on the check
    // box states. Note that the check boxes are tri-state.
    {
        long nNumberOfCheckedLabelParts = 0;
        if (m_aCBNumber.GetState() != STATE_NOCHECK)
            ++nNumberOfCheckedLabelParts;
        if (m_aCBPercent.GetState() != STATE_NOCHECK && m_aCBPercent.IsEnabled())
            ++nNumberOfCheckedLabelParts;
        if (m_aCBCategory.GetState() != STATE_NOCHECK)
            ++nNumberOfCheckedLabelParts;
        m_aSeparatorResources.Enable( nNumberOfCheckedLabelParts > 1 );
        bool bEnableTextDir = nNumberOfCheckedLabelParts > 0;
        m_aFT_TextDirection.Enable( bEnableTextDir );
        m_aLB_TextDirection.Enable( bEnableTextDir );
        bool bEnablePlacement = nNumberOfCheckedLabelParts > 0 && m_aLB_LabelPlacement.GetEntryCount()>1;
        m_aFT_LabelPlacement.Enable( bEnablePlacement );
        m_aLB_LabelPlacement.Enable( bEnablePlacement );
    }

    m_aPB_NumberFormatForValue.Enable( m_pNumberFormatter && m_aCBNumber.IsChecked() );
    m_aPB_NumberFormatForPercent.Enable( m_pNumberFormatter && m_aCBPercent.IsChecked() && m_aCBPercent.IsEnabled() );

    bool bEnableRotation = ( m_aCBNumber.IsChecked() || m_aCBPercent.IsChecked() || m_aCBCategory.IsChecked() );
    m_aFL_Rotate.Enable( bEnableRotation );
    m_aDC_Dial.Enable( bEnableRotation );
    m_aFT_Degrees.Enable( bEnableRotation );
    m_aNF_Degrees.Enable( bEnableRotation );
}

sal_Bool DataLabelResources::FillItemSet( SfxItemSet& rOutAttrs ) const
{
    if( m_aCBNumber.IsChecked() )
    {
        if( !m_bNumberFormatMixedState )
            rOutAttrs.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, m_nNumberFormatForValue ));
        if( !m_bSourceFormatMixedState )
            rOutAttrs.Put( SfxBoolItem( SID_ATTR_NUMBERFORMAT_SOURCE, m_bSourceFormatForValue ));
    }
    if( m_aCBPercent.IsChecked() )
    {
        if( !m_bPercentFormatMixedState )
            rOutAttrs.Put( SfxUInt32Item( SCHATTR_PERCENT_NUMBERFORMAT_VALUE, m_nNumberFormatForPercent ));
        if( !m_bPercentSourceMixedState )
            rOutAttrs.Put( SfxBoolItem( SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_bSourceFormatForPercent ));
    }

    if( m_aCBNumber.GetState()!= STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_NUMBER, m_aCBNumber.IsChecked() ) );
    if( m_aCBPercent.GetState()!= STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_PERCENTAGE, m_aCBPercent.IsChecked() ) );
    if( m_aCBCategory.GetState()!= STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_CATEGORY, m_aCBCategory.IsChecked() ) );
    if( m_aCBSymbol.GetState()!= STATE_DONTKNOW )
        rOutAttrs.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYMBOL, m_aCBSymbol.IsChecked()) );

    rOutAttrs.Put( SfxStringItem( SCHATTR_DATADESCR_SEPARATOR, m_aSeparatorResources.GetValue() ) );
    ::std::map< sal_uInt16, sal_Int32 >::const_iterator aIt( m_aListBoxToPlacementMap.find(m_aLB_LabelPlacement.GetSelectEntryPos()) );
    if(aIt!=m_aListBoxToPlacementMap.end())
    {
        sal_Int32 nValue = aIt->second;
        rOutAttrs.Put( SfxInt32Item( SCHATTR_DATADESCR_PLACEMENT, nValue ) );
    }

    if( m_aLB_TextDirection.GetSelectEntryCount() > 0 )
        rOutAttrs.Put( SfxInt32Item( EE_PARA_WRITINGDIR, m_aLB_TextDirection.GetSelectEntryValue() ) );

    if( m_aDC_Dial.IsVisible() )
    {
        sal_Int32 nDegrees = m_aDC_Dial.GetRotation();
        rOutAttrs.Put(SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }

    return sal_True;
}

void DataLabelResources::Reset(const SfxItemSet& rInAttrs)
{
    // default state
    m_aCBSymbol.Enable( sal_False );

    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_NUMBER, m_aCBNumber );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_PERCENTAGE, m_aCBPercent );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_CATEGORY, m_aCBCategory );
    lcl_setBoolItemToCheckBox( rInAttrs, SCHATTR_DATADESCR_SHOW_SYMBOL, m_aCBSymbol );

    m_bNumberFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_SOURCE, m_nNumberFormatForValue, m_bSourceFormatForValue, m_bSourceFormatMixedState );
    m_bPercentFormatMixedState = !lcl_ReadNumberFormatFromItemSet( rInAttrs, SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, m_nNumberFormatForPercent, m_bSourceFormatForPercent ,  m_bPercentSourceMixedState);

    const SfxPoolItem *pPoolItem = NULL;
    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_SEPARATOR, sal_True, &pPoolItem) == SFX_ITEM_SET )
        m_aSeparatorResources.SetValue( ((const SfxStringItem*)pPoolItem)->GetValue() );
    else
        m_aSeparatorResources.SetDefault();

    if( rInAttrs.GetItemState(SCHATTR_DATADESCR_PLACEMENT, sal_True, &pPoolItem) == SFX_ITEM_SET )
    {
        sal_Int32 nPlacement = ((const SfxInt32Item*)pPoolItem)->GetValue();
        ::std::map< sal_Int32, sal_uInt16 >::const_iterator aIt( m_aPlacementToListBoxMap.find(nPlacement) );
        if(aIt!=m_aPlacementToListBoxMap.end())
        {
            sal_uInt16 nPos = aIt->second;
            m_aLB_LabelPlacement.SelectEntryPos( nPos );
        }
        else
            m_aLB_LabelPlacement.SetNoSelection();
    }
    else
        m_aLB_LabelPlacement.SetNoSelection();

    if( rInAttrs.GetItemState(EE_PARA_WRITINGDIR, sal_True, &pPoolItem ) == SFX_ITEM_SET )
        m_aLB_TextDirection.SelectEntryValue( SvxFrameDirection(((const SvxFrameDirectionItem*)pPoolItem)->GetValue()) );

    if( rInAttrs.GetItemState( SCHATTR_TEXT_DEGREES, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_Int32 nDegrees = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_aDC_Dial.SetRotation( nDegrees );
    }
    else
        m_aDC_Dial.SetRotation( 0 );

    EnableControls();
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
