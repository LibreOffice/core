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

#include <cuires.hrc>
#include <dialmgr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/request.hxx>
#include <vcl/settings.hxx>

#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/dlgutil.hxx>
#include <svx/measctrl.hxx>
#include <svx/ofaitem.hxx>
#include <svx/strarray.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdview.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmfsitm.hxx>
#include <svx/sxmlhitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmuitm.hxx>

#include "measure.hxx"

const sal_uInt16 SvxMeasurePage::pRanges[] =
{
    SDRATTR_MEASURE_FIRST,
    SDRATTR_MEASURE_LAST,
    0
};

/*************************************************************************
|*
|* Dialog to change measure-attributes
|*
\************************************************************************/

SvxMeasureDialog::SvxMeasureDialog( vcl::Window* pParent, const SfxItemSet& rInAttrs,
                                const SdrView* pSdrView )
    : SfxSingleTabDialog(pParent, rInAttrs)
{
    VclPtrInstance<SvxMeasurePage> _pPage( get_content_area(), rInAttrs );

    _pPage->SetView( pSdrView );
    _pPage->Construct();

    SetTabPage(_pPage );
    SetText(CUI_RESSTR(RID_SVXSTR_DIMENSION_LINE));
}

/*************************************************************************
|*
|* Tabpage for changing measure-attributes
|*
\************************************************************************/

SvxMeasurePage::SvxMeasurePage( vcl::Window* pWindow, const SfxItemSet& rInAttrs ) :
                SvxTabPage      ( pWindow
                                 ,"DimensionLinesTabPage"
                                 ,"cui/ui/dimensionlinestabpage.ui"
                                 ,rInAttrs ),
        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() ),
        pView( nullptr ),
        eUnit( SFX_MAPUNIT_100TH_MM ),
        bPositionModified       ( false )
{
    get(m_pMtrFldLineDist, "MTR_LINE_DIST");
    get(m_pMtrFldHelplineOverhang, "MTR_FLD_HELPLINE_OVERHANG");
    get(m_pMtrFldHelplineDist, "MTR_FLD_HELPLINE_DIST");
    get(m_pMtrFldHelpline1Len, "MTR_FLD_HELPLINE1_LEN");
    get(m_pMtrFldHelpline2Len, "MTR_FLD_HELPLINE2_LEN");
    get(m_pTsbBelowRefEdge, "TSB_BELOW_REF_EDGE");
    get(m_pMtrFldDecimalPlaces, "MTR_FLD_DECIMALPLACES");

    get(m_pCtlPosition, "CTL_POSITION");
    get(m_pTsbAutoPosV, "TSB_AUTOPOSV");
    get(m_pTsbAutoPosH, "TSB_AUTOPOSH");
    get(m_pTsbShowUnit, "TSB_SHOW_UNIT");
    get(m_pLbUnit, "LB_UNIT");
    get(m_pTsbParallel, "TSB_PARALLEL");

    get(m_pCtlPreview, "CTL_PREVIEW");
    m_pCtlPreview->SetAttributes(rInAttrs);

    get(m_pFtAutomatic,"STR_MEASURE_AUTOMATIC");

    FillUnitLB();

    const FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_pMtrFldLineDist, eFUnit );
    SetFieldUnit( *m_pMtrFldHelplineOverhang, eFUnit );
    SetFieldUnit( *m_pMtrFldHelplineDist, eFUnit );
    SetFieldUnit( *m_pMtrFldHelpline1Len, eFUnit );
    SetFieldUnit( *m_pMtrFldHelpline2Len, eFUnit );
    if( eFUnit == FUNIT_MM )
    {
        m_pMtrFldLineDist->SetSpinSize( 50 );
        m_pMtrFldHelplineOverhang->SetSpinSize( 50 );
        m_pMtrFldHelplineDist->SetSpinSize( 50 );
        m_pMtrFldHelpline1Len->SetSpinSize( 50 );
        m_pMtrFldHelpline2Len->SetSpinSize( 50 );
    }

    m_pTsbAutoPosV->SetClickHdl( LINK( this, SvxMeasurePage, ClickAutoPosHdl_Impl ) );
    m_pTsbAutoPosH->SetClickHdl( LINK( this, SvxMeasurePage, ClickAutoPosHdl_Impl ) );

    // set background and border of iconchoicectrl
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    m_pCtlPreview->SetBackground ( rStyles.GetWindowColor() );
    m_pCtlPreview->SetBorderStyle(WindowBorderStyle::MONO);

    Link<Edit&,void> aLink( LINK( this, SvxMeasurePage, ChangeAttrEditHdl_Impl ) );
    m_pMtrFldLineDist->SetModifyHdl( aLink );
    m_pMtrFldHelplineOverhang->SetModifyHdl( aLink );
    m_pMtrFldHelplineDist->SetModifyHdl( aLink );
    m_pMtrFldHelpline1Len->SetModifyHdl( aLink );
    m_pMtrFldHelpline2Len->SetModifyHdl( aLink );
    m_pMtrFldDecimalPlaces->SetModifyHdl( aLink );
    m_pTsbBelowRefEdge->SetClickHdl( LINK( this, SvxMeasurePage, ChangeAttrClickHdl_Impl ) );
    m_pTsbParallel->SetClickHdl( LINK( this, SvxMeasurePage, ChangeAttrClickHdl_Impl ) );
    m_pTsbShowUnit->SetClickHdl( LINK( this, SvxMeasurePage, ChangeAttrClickHdl_Impl ) );
    m_pLbUnit->SetSelectHdl( LINK( this, SvxMeasurePage, ChangeAttrListBoxHdl_Impl ) );
}

SvxMeasurePage::~SvxMeasurePage()
{
    disposeOnce();
}

void SvxMeasurePage::dispose()
{
    m_pMtrFldLineDist.clear();
    m_pMtrFldHelplineOverhang.clear();
    m_pMtrFldHelplineDist.clear();
    m_pMtrFldHelpline1Len.clear();
    m_pMtrFldHelpline2Len.clear();
    m_pTsbBelowRefEdge.clear();
    m_pMtrFldDecimalPlaces.clear();
    m_pCtlPosition.clear();
    m_pTsbAutoPosV.clear();
    m_pTsbAutoPosH.clear();
    m_pTsbShowUnit.clear();
    m_pLbUnit.clear();
    m_pTsbParallel.clear();
    m_pFtAutomatic.clear();
    m_pCtlPreview.clear();
    SvxTabPage::dispose();
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void SvxMeasurePage::Reset( const SfxItemSet* rAttrs )
{
    SfxItemPool* pPool = rAttrs->GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_MEASURELINEDIST );

    const SfxPoolItem* pItem = GetItem( *rAttrs, SDRATTR_MEASURELINEDIST );

    // SdrMeasureLineDistItem
    if( pItem == nullptr )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASURELINEDIST );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLineDist, nValue, eUnit );
    }
    else
    {
        m_pMtrFldLineDist->SetText( OUString() );
    }
    m_pMtrFldLineDist->SaveValue();

    // SdrMeasureHelplineOverhangItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem == nullptr )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelplineOverhang, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelplineOverhang->SetText( OUString() );
    }
    m_pMtrFldHelplineOverhang->SaveValue();

    // SdrMeasureHelplineDistItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINEDIST );
    if( pItem == nullptr )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEDIST );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelplineDist, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelplineDist->SetText( OUString() );
    }
    m_pMtrFldHelplineDist->SaveValue();

    // SdrMeasureHelpline1LenItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem == nullptr )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelpline1Len, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelpline1Len->SetText( OUString() );
    }
    m_pMtrFldHelpline1Len->SaveValue();

    // SdrMeasureHelpline2LenItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem == nullptr )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelpline2Len, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelpline2Len->SetText( OUString() );
    }
    m_pMtrFldHelpline2Len->SaveValue();

    // SdrMeasureBelowRefEdgeItem
    if( rAttrs->GetItemState( SDRATTR_MEASUREBELOWREFEDGE ) != SfxItemState::DONTCARE )
    {
        m_pTsbBelowRefEdge->SetState( static_cast<const SdrMeasureBelowRefEdgeItem&>( rAttrs->Get( SDRATTR_MEASUREBELOWREFEDGE ) ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbBelowRefEdge->EnableTriState( false );
    }
    else
    {
        m_pTsbBelowRefEdge->SetState( TRISTATE_INDET );
    }
    m_pTsbBelowRefEdge->SaveValue();

    // SdrMeasureDecimalPlacesItem
    pItem = GetItem( *rAttrs, SDRATTR_MEASUREDECIMALPLACES );
    if( pItem == nullptr )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREDECIMALPLACES );
    if( pItem )
    {
        sal_Int16 nValue = static_cast<const SdrMeasureDecimalPlacesItem*>( pItem )->GetValue();
        m_pMtrFldDecimalPlaces->SetValue( nValue );
    }
    else
    {
        m_pMtrFldDecimalPlaces->SetText( OUString() );
    }
    m_pMtrFldDecimalPlaces->SaveValue();

    // SdrMeasureTextRota90Item
    // Attention: negate !
    if( rAttrs->GetItemState( SDRATTR_MEASURETEXTROTA90 ) != SfxItemState::DONTCARE )
    {
        m_pTsbParallel->SetState( static_cast<const SdrMeasureTextRota90Item&>( rAttrs->Get( SDRATTR_MEASURETEXTROTA90 ) ).
                        GetValue() ? TRISTATE_FALSE : TRISTATE_TRUE );
        m_pTsbParallel->EnableTriState( false );
    }
    else
    {
        m_pTsbParallel->SetState( TRISTATE_INDET );
    }
    m_pTsbParallel->SaveValue();

    // SdrMeasureShowUnitItem
    if( rAttrs->GetItemState( SDRATTR_MEASURESHOWUNIT ) != SfxItemState::DONTCARE )
    {
        m_pTsbShowUnit->SetState( static_cast<const SdrYesNoItem&>( rAttrs->Get( SDRATTR_MEASURESHOWUNIT ) ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbShowUnit->EnableTriState( false );
    }
    else
    {
        m_pTsbShowUnit->SetState( TRISTATE_INDET );
    }
    m_pTsbShowUnit->SaveValue();

    // SdrMeasureUnitItem
    if( rAttrs->GetItemState( SDRATTR_MEASUREUNIT ) != SfxItemState::DONTCARE )
    {
        long nFieldUnit = (long) static_cast<const SdrMeasureUnitItem&>( rAttrs->
                                    Get( SDRATTR_MEASUREUNIT ) ).GetValue();

        for( sal_Int32 i = 0; i < m_pLbUnit->GetEntryCount(); ++i )
        {
            if ( reinterpret_cast<sal_IntPtr>(m_pLbUnit->GetEntryData( i )) == nFieldUnit )
            {
                m_pLbUnit->SelectEntryPos( i );
                break;
            }
        }
    }
    else
    {
        m_pLbUnit->SetNoSelection();
    }
    m_pLbUnit->SaveValue();

    // Position
    if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTVPOS ) != SfxItemState::DONTCARE )
    {
        SdrMeasureTextVPos eVPos = (SdrMeasureTextVPos)
                    static_cast<const SdrMeasureTextVPosItem&>( rAttrs->Get( SDRATTR_MEASURETEXTVPOS ) ).GetValue();
        {
            if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTHPOS ) != SfxItemState::DONTCARE )
            {
                m_pTsbAutoPosV->EnableTriState( false );
                m_pTsbAutoPosH->EnableTriState( false );

                SdrMeasureTextHPos eHPos = (SdrMeasureTextHPos)
                            static_cast<const SdrMeasureTextHPosItem&>( rAttrs->Get( SDRATTR_MEASURETEXTHPOS ) ).GetValue();
                RECT_POINT eRP = RP_MM;
                switch( eVPos )
                {
                case SDRMEASURE_ABOVE:
                    switch( eHPos )
                    {
                    case SDRMEASURE_TEXTLEFTOUTSIDE:    eRP = RP_LT; break;
                    case SDRMEASURE_TEXTINSIDE:         eRP = RP_MT; break;
                    case SDRMEASURE_TEXTRIGHTOUTSIDE:   eRP = RP_RT; break;
                    case SDRMEASURE_TEXTHAUTO:          eRP = RP_MT; break;
                    }
                    break;
                case SDRMEASURETEXT_VERTICALCENTERED:
                    switch( eHPos )
                    {
                    case SDRMEASURE_TEXTLEFTOUTSIDE:    eRP = RP_LM; break;
                    case SDRMEASURE_TEXTINSIDE:         eRP = RP_MM; break;
                    case SDRMEASURE_TEXTRIGHTOUTSIDE:   eRP = RP_RM; break;
                    case SDRMEASURE_TEXTHAUTO:          eRP = RP_MM; break;
                    }
                    break;
                case SDRMEASURE_BELOW:
                    switch( eHPos )
                    {
                    case SDRMEASURE_TEXTLEFTOUTSIDE:    eRP = RP_LB; break;
                    case SDRMEASURE_TEXTINSIDE:         eRP = RP_MB; break;
                    case SDRMEASURE_TEXTRIGHTOUTSIDE:   eRP = RP_RB; break;
                    case SDRMEASURE_TEXTHAUTO:          eRP = RP_MB; break;
                    }
                    break;
                case SDRMEASURE_TEXTVAUTO:
                    switch( eHPos )
                    {
                    case SDRMEASURE_TEXTLEFTOUTSIDE:    eRP = RP_LM; break;
                    case SDRMEASURE_TEXTINSIDE:         eRP = RP_MM; break;
                    case SDRMEASURE_TEXTRIGHTOUTSIDE:   eRP = RP_RM; break;
                    case SDRMEASURE_TEXTHAUTO:          eRP = RP_MM; break;
                    }
                    break;
                 default: ;//prevent warning
                }

                CTL_STATE nState = CTL_STATE::NONE;

                if( eHPos == SDRMEASURE_TEXTHAUTO )
                {
                    m_pTsbAutoPosH->SetState( TRISTATE_TRUE );
                    nState = CTL_STATE::NOHORZ;
                }

                if( eVPos == SDRMEASURE_TEXTVAUTO )
                {
                    m_pTsbAutoPosV->SetState( TRISTATE_TRUE );
                    nState |= CTL_STATE::NOVERT;
                }

                m_pCtlPosition->SetState( nState );
                m_pCtlPosition->SetActualRP( eRP );
            }
        }
    }
    else
    {
        m_pCtlPosition->Reset();
        m_pTsbAutoPosV->SetState( TRISTATE_INDET );
        m_pTsbAutoPosH->SetState( TRISTATE_INDET );
    }

    // put the attributes to the preview-control,
    // otherwise the control don't know about
    // the settings of the dialog (#67930)
    ChangeAttrHdl_Impl( m_pTsbShowUnit );
    m_pCtlPreview->SetAttributes( *rAttrs );

    bPositionModified = false;
}

/*************************************************************************
|*
|* Fill the delivered Item-Set with dialogbox-attributes
|*
\************************************************************************/

bool SvxMeasurePage::FillItemSet( SfxItemSet* rAttrs)
{
    bool     bModified = false;
    sal_Int32    nValue;
    TriState eState;

    if( m_pMtrFldLineDist->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldLineDist, eUnit );
        rAttrs->Put( makeSdrMeasureLineDistItem( nValue ) );
        bModified = true;
    }

    if( m_pMtrFldHelplineOverhang->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelplineOverhang, eUnit );
        rAttrs->Put( makeSdrMeasureHelplineOverhangItem( nValue ) );
        bModified = true;
    }

    if( m_pMtrFldHelplineDist->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelplineDist, eUnit );
        rAttrs->Put( makeSdrMeasureHelplineDistItem( nValue ) );
        bModified = true;
    }

    if( m_pMtrFldHelpline1Len->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelpline1Len, eUnit );
        rAttrs->Put( makeSdrMeasureHelpline1LenItem( nValue ) );
        bModified = true;
    }

    if( m_pMtrFldHelpline2Len->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelpline2Len, eUnit );
        rAttrs->Put( makeSdrMeasureHelpline2LenItem( nValue ) );
        bModified = true;
    }

    eState = m_pTsbBelowRefEdge->GetState();
    if( m_pTsbBelowRefEdge->IsValueChangedFromSaved() )
    {
        rAttrs->Put( SdrMeasureBelowRefEdgeItem( TRISTATE_TRUE == eState ) );
        bModified = true;
    }

    if( m_pMtrFldDecimalPlaces->IsValueChangedFromSaved() )
    {
        nValue = static_cast<sal_Int32>(m_pMtrFldDecimalPlaces->GetValue());
        rAttrs->Put(
            SdrMeasureDecimalPlacesItem(
                sal::static_int_cast< sal_Int16 >( nValue ) ) );
        bModified = true;
    }

    eState = m_pTsbParallel->GetState();
    if( m_pTsbParallel->IsValueChangedFromSaved() )
    {
        rAttrs->Put( SdrMeasureTextRota90Item( TRISTATE_FALSE == eState ) );
        bModified = true;
    }

    eState = m_pTsbShowUnit->GetState();
    if( m_pTsbShowUnit->IsValueChangedFromSaved() )
    {
        rAttrs->Put( SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, TRISTATE_TRUE == eState ) );
        bModified = true;
    }

    sal_Int32 nPos = m_pLbUnit->GetSelectEntryPos();
    if( m_pLbUnit->IsValueChangedFromSaved() )
    {
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nFieldUnit = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pLbUnit->GetEntryData( nPos ));
            FieldUnit _eUnit = (FieldUnit) nFieldUnit;
            rAttrs->Put( SdrMeasureUnitItem( _eUnit ) );
            bModified = true;
        }
    }

    if( bPositionModified )
    {
        // Position
        SdrMeasureTextVPos eVPos, eOldVPos;
        SdrMeasureTextHPos eHPos, eOldHPos;

        RECT_POINT eRP = m_pCtlPosition->GetActualRP();
        switch( eRP )
        {
            default:
            case RP_LT: eVPos = SDRMEASURE_ABOVE;
                        eHPos = SDRMEASURE_TEXTLEFTOUTSIDE; break;
            case RP_LM: eVPos = SDRMEASURETEXT_VERTICALCENTERED;
                        eHPos = SDRMEASURE_TEXTLEFTOUTSIDE; break;
            case RP_LB: eVPos = SDRMEASURE_BELOW;
                        eHPos = SDRMEASURE_TEXTLEFTOUTSIDE; break;
            case RP_MT: eVPos = SDRMEASURE_ABOVE;
                        eHPos = SDRMEASURE_TEXTINSIDE; break;
            case RP_MM: eVPos = SDRMEASURETEXT_VERTICALCENTERED;
                        eHPos = SDRMEASURE_TEXTINSIDE; break;
            case RP_MB: eVPos = SDRMEASURE_BELOW;
                        eHPos = SDRMEASURE_TEXTINSIDE; break;
            case RP_RT: eVPos = SDRMEASURE_ABOVE;
                        eHPos = SDRMEASURE_TEXTRIGHTOUTSIDE; break;
            case RP_RM: eVPos = SDRMEASURETEXT_VERTICALCENTERED;
                        eHPos = SDRMEASURE_TEXTRIGHTOUTSIDE; break;
            case RP_RB: eVPos = SDRMEASURE_BELOW;
                        eHPos = SDRMEASURE_TEXTRIGHTOUTSIDE; break;
        }
        if( m_pTsbAutoPosH->GetState() == TRISTATE_TRUE )
            eHPos = SDRMEASURE_TEXTHAUTO;

        if( m_pTsbAutoPosV->GetState() == TRISTATE_TRUE )
            eVPos = SDRMEASURE_TEXTVAUTO;

        if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTVPOS ) != SfxItemState::DONTCARE )
        {
            eOldVPos = (SdrMeasureTextVPos)
                        static_cast<const SdrMeasureTextVPosItem&>( rOutAttrs.Get( SDRATTR_MEASURETEXTVPOS ) ).GetValue();
            if( eOldVPos != eVPos )
            {
                rAttrs->Put( SdrMeasureTextVPosItem( eVPos ) );
                bModified = true;
            }
        }
        else
        {
            rAttrs->Put( SdrMeasureTextVPosItem( eVPos ) );
            bModified = true;
        }

        if ( rAttrs->GetItemState( SDRATTR_MEASURETEXTHPOS ) != SfxItemState::DONTCARE )
        {
            eOldHPos = (SdrMeasureTextHPos)
                        static_cast<const SdrMeasureTextHPosItem&>( rOutAttrs.Get( SDRATTR_MEASURETEXTHPOS ) ).GetValue();
            if( eOldHPos != eHPos )
            {
                rAttrs->Put( SdrMeasureTextHPosItem( eHPos ) );
                bModified = true;
            }
        }
        else
        {
            rAttrs->Put( SdrMeasureTextHPosItem( eHPos ) );
            bModified = true;
        }
    }

    return bModified;
}

/*************************************************************************
|*
|* The View have to set at the measure-object to be able to notify
|* unit and floatingpoint-values
|*
\************************************************************************/

void SvxMeasurePage::Construct()
{
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    m_pCtlPreview->pMeasureObj->SetModel( pView->GetModel() );
    m_pCtlPreview->Invalidate();
}

VclPtr<SfxTabPage> SvxMeasurePage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxMeasurePage>::Create( pWindow, *rAttrs );
}

void SvxMeasurePage::PointChanged( vcl::Window* pWindow, RECT_POINT /*eRP*/ )
{
    ChangeAttrHdl_Impl( pWindow );
}

IMPL_LINK_TYPED( SvxMeasurePage, ClickAutoPosHdl_Impl, Button*, p, void )
{
    if( m_pTsbAutoPosH->GetState() == TRISTATE_TRUE )
    {
        switch( m_pCtlPosition->GetActualRP() )
        {
            case RP_LT:
            case RP_RT:
                m_pCtlPosition->SetActualRP( RP_MT );
            break;

            case RP_LM:
            case RP_RM:
                m_pCtlPosition->SetActualRP( RP_MM );
            break;

            case RP_LB:
            case RP_RB:
                m_pCtlPosition->SetActualRP( RP_MB );
            break;
            default: ;//prevent warning
        }
    }
    if( m_pTsbAutoPosV->GetState() == TRISTATE_TRUE )
    {
        switch( m_pCtlPosition->GetActualRP() )
        {
            case RP_LT:
            case RP_LB:
                m_pCtlPosition->SetActualRP( RP_LM );
            break;

            case RP_MT:
            case RP_MB:
                m_pCtlPosition->SetActualRP( RP_MM );
            break;

            case RP_RT:
            case RP_RB:
                m_pCtlPosition->SetActualRP( RP_RM );
            break;
            default: ;//prevent warning
        }
    }
    ChangeAttrHdl_Impl( p );
}

IMPL_LINK_TYPED( SvxMeasurePage, ChangeAttrClickHdl_Impl, Button*, p, void )
{
    ChangeAttrHdl_Impl(p);
}
IMPL_LINK_TYPED( SvxMeasurePage, ChangeAttrListBoxHdl_Impl, ListBox&, rBox, void )
{
    ChangeAttrHdl_Impl(&rBox);
}
IMPL_LINK_TYPED( SvxMeasurePage, ChangeAttrEditHdl_Impl, Edit&, rBox, void )
{
    ChangeAttrHdl_Impl(&rBox);
}
void SvxMeasurePage::ChangeAttrHdl_Impl( void* p )
{
    if( p == m_pMtrFldLineDist )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldLineDist, eUnit );
        aAttrSet.Put( makeSdrMeasureLineDistItem( nValue ) );
    }

    if( p == m_pMtrFldHelplineOverhang )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelplineOverhang, eUnit );
        aAttrSet.Put( makeSdrMeasureHelplineOverhangItem( nValue) );
    }

    if( p == m_pMtrFldHelplineDist )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelplineDist, eUnit );
        aAttrSet.Put( makeSdrMeasureHelplineDistItem( nValue) );
    }

    if( p == m_pMtrFldHelpline1Len )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelpline1Len, eUnit );
        aAttrSet.Put( makeSdrMeasureHelpline1LenItem( nValue ) );
    }

    if( p == m_pMtrFldHelpline2Len )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelpline2Len, eUnit );
        aAttrSet.Put( makeSdrMeasureHelpline2LenItem( nValue ) );
    }

    if( p == m_pTsbBelowRefEdge )
    {
        TriState eState = m_pTsbBelowRefEdge->GetState();
        if( eState != TRISTATE_INDET )
            aAttrSet.Put( SdrMeasureBelowRefEdgeItem( TRISTATE_TRUE == eState ) );
    }

    if( p == m_pMtrFldDecimalPlaces )
    {
        sal_Int16 nValue = sal::static_int_cast< sal_Int16 >(
            m_pMtrFldDecimalPlaces->GetValue() );
        aAttrSet.Put( SdrMeasureDecimalPlacesItem( nValue ) );
    }

    if( p == m_pTsbParallel )
    {
        TriState eState = m_pTsbParallel->GetState();
        if( eState != TRISTATE_INDET )
            aAttrSet.Put( SdrMeasureTextRota90Item( TRISTATE_FALSE == eState ) );
    }

    if( p == m_pTsbShowUnit )
    {
        TriState eState = m_pTsbShowUnit->GetState();
        if( eState != TRISTATE_INDET )
            aAttrSet.Put( SdrYesNoItem( SDRATTR_MEASURESHOWUNIT, TRISTATE_TRUE == eState ) );
    }

    if( p == m_pLbUnit )
    {
        sal_Int32 nPos = m_pLbUnit->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nFieldUnit = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pLbUnit->GetEntryData( nPos ));
            FieldUnit _eUnit = (FieldUnit) nFieldUnit;
            aAttrSet.Put( SdrMeasureUnitItem( _eUnit ) );
        }
    }

    if( p == m_pTsbAutoPosV || p == m_pTsbAutoPosH || p == m_pCtlPosition )
    {
        bPositionModified = true;

        // Position
        RECT_POINT eRP = m_pCtlPosition->GetActualRP();
        SdrMeasureTextVPos eVPos;
        SdrMeasureTextHPos eHPos;

        switch( eRP )
        {
            default:
            case RP_LT: eVPos = SDRMEASURE_ABOVE;
                        eHPos = SDRMEASURE_TEXTLEFTOUTSIDE; break;
            case RP_LM: eVPos = SDRMEASURETEXT_VERTICALCENTERED;
                        eHPos = SDRMEASURE_TEXTLEFTOUTSIDE; break;
            case RP_LB: eVPos = SDRMEASURE_BELOW;
                        eHPos = SDRMEASURE_TEXTLEFTOUTSIDE; break;
            case RP_MT: eVPos = SDRMEASURE_ABOVE;
                        eHPos = SDRMEASURE_TEXTINSIDE; break;
            case RP_MM: eVPos = SDRMEASURETEXT_VERTICALCENTERED;
                        eHPos = SDRMEASURE_TEXTINSIDE; break;
            case RP_MB: eVPos = SDRMEASURE_BELOW;
                        eHPos = SDRMEASURE_TEXTINSIDE; break;
            case RP_RT: eVPos = SDRMEASURE_ABOVE;
                        eHPos = SDRMEASURE_TEXTRIGHTOUTSIDE; break;
            case RP_RM: eVPos = SDRMEASURETEXT_VERTICALCENTERED;
                        eHPos = SDRMEASURE_TEXTRIGHTOUTSIDE; break;
            case RP_RB: eVPos = SDRMEASURE_BELOW;
                        eHPos = SDRMEASURE_TEXTRIGHTOUTSIDE; break;
        }

        CTL_STATE nState = CTL_STATE::NONE;

        if( m_pTsbAutoPosH->GetState() == TRISTATE_TRUE )
        {
            eHPos = SDRMEASURE_TEXTHAUTO;
            nState = CTL_STATE::NOHORZ;
        }

        if( m_pTsbAutoPosV->GetState() == TRISTATE_TRUE )
        {
            eVPos = SDRMEASURE_TEXTVAUTO;
            nState |= CTL_STATE::NOVERT;
        }

        if( p == m_pTsbAutoPosV || p == m_pTsbAutoPosH )
            m_pCtlPosition->SetState( nState );

        aAttrSet.Put( SdrMeasureTextVPosItem( eVPos ) );
        aAttrSet.Put( SdrMeasureTextHPosItem( eHPos ) );
    }

    m_pCtlPreview->SetAttributes( aAttrSet );
    m_pCtlPreview->Invalidate();
}

void SvxMeasurePage::FillUnitLB()
{
    // fill ListBox with metrics
    SvxStringArray aMetricArr( SVX_RES( RID_SVXSTR_FIELDUNIT_TABLE ) );

    sal_IntPtr nUnit = FUNIT_NONE;
    OUString aStrMetric( m_pFtAutomatic->GetText());
    sal_Int32 nPos = m_pLbUnit->InsertEntry( aStrMetric );
    m_pLbUnit->SetEntryData( nPos, reinterpret_cast<void*>(nUnit) );

    for( sal_uInt32 i = 0; i < aMetricArr.Count(); ++i )
    {
        aStrMetric = aMetricArr.GetStringByPos( i );
        nUnit = aMetricArr.GetValue( i );
        nPos = m_pLbUnit->InsertEntry( aStrMetric );
        m_pLbUnit->SetEntryData( nPos, reinterpret_cast<void*>(nUnit) );
    }
}
void SvxMeasurePage::PageCreated(const SfxAllItemSet& aSet)
{
    const OfaPtrItem* pOfaPtrItem = aSet.GetItem<OfaPtrItem>(SID_OBJECT_LIST, false);

    if (pOfaPtrItem)
        SetView( static_cast<SdrView *>(pOfaPtrItem->GetValue()));

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
