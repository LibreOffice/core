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
#include <tools/shl.hxx>

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
#include <svx/sxmsuitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmuitm.hxx>

#include "measure.hxx"

static sal_uInt16 pRanges[] =
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

SvxMeasureDialog::SvxMeasureDialog( Window* pParent, const SfxItemSet& rInAttrs,
                                const SdrView* pSdrView ) :
        SfxNoLayoutSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_MEASURE )
{
    SvxMeasurePage* _pPage = new SvxMeasurePage( this, rInAttrs );

    _pPage->SetView( pSdrView );
    _pPage->Construct();

    SetTabPage( _pPage );
    SetText( _pPage->GetText() );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxMeasureDialog::~SvxMeasureDialog()
{
}

/*************************************************************************
|*
|* Tabpage for changing measure-attributes
|*
\************************************************************************/

SvxMeasurePage::SvxMeasurePage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SvxTabPage      ( pWindow
                                 ,"DimensionLines"
                                 ,"cui/ui/dimensionlines.ui"
                                 ,rInAttrs ),

//         aCtlPreview             ( this, CUI_RES( CTL_PREVIEW ), rInAttrs ),

        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() ),
        pView( 0 ),
        bPositionModified       ( sal_False )
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
    m_pCtlPreview->SetBorderStyle(WINDOW_BORDER_MONO);

    Link aLink( LINK( this, SvxMeasurePage, ChangeAttrHdl_Impl ) );
    m_pMtrFldLineDist->SetModifyHdl( aLink );
    m_pMtrFldHelplineOverhang->SetModifyHdl( aLink );
    m_pMtrFldHelplineDist->SetModifyHdl( aLink );
    m_pMtrFldHelpline1Len->SetModifyHdl( aLink );
    m_pMtrFldHelpline2Len->SetModifyHdl( aLink );
    m_pMtrFldDecimalPlaces->SetModifyHdl( aLink );
    m_pTsbBelowRefEdge->SetClickHdl( aLink );
    m_pTsbParallel->SetClickHdl( aLink );
    m_pTsbShowUnit->SetClickHdl( aLink );
    m_pLbUnit->SetSelectHdl( aLink );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxMeasurePage::~SvxMeasurePage()
{
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void SvxMeasurePage::Reset( const SfxItemSet& rAttrs )
{
    SfxItemPool* pPool = rAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_MEASURELINEDIST );

    const SfxPoolItem* pItem = GetItem( rAttrs, SDRATTR_MEASURELINEDIST );

    // SdrMeasureLineDistItem
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASURELINEDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureLineDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLineDist, nValue, eUnit );
    }
    else
    {
        m_pMtrFldLineDist->SetText( String() );
    }
    m_pMtrFldLineDist->SaveValue();

    // SdrMeasureHelplineOverhangItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelplineOverhangItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelplineOverhang, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelplineOverhang->SetText( String() );
    }
    m_pMtrFldHelplineOverhang->SaveValue();

    // SdrMeasureHelplineDistItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINEDIST );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelplineDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelplineDist, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelplineDist->SetText( String() );
    }
    m_pMtrFldHelplineDist->SaveValue();

    // SdrMeasureHelpline1LenItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelpline1LenItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelpline1Len, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelpline1Len->SetText( String() );
    }
    m_pMtrFldHelpline1Len->SaveValue();

    // SdrMeasureHelpline2LenItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelpline2LenItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHelpline2Len, nValue, eUnit );
    }
    else
    {
        m_pMtrFldHelpline2Len->SetText( String() );
    }
    m_pMtrFldHelpline2Len->SaveValue();

    // SdrMeasureBelowRefEdgeItem
    if( rAttrs.GetItemState( SDRATTR_MEASUREBELOWREFEDGE ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbBelowRefEdge->SetState( ( ( const SdrMeasureBelowRefEdgeItem& )rAttrs.Get( SDRATTR_MEASUREBELOWREFEDGE ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbBelowRefEdge->EnableTriState( sal_False );
    }
    else
    {
        m_pTsbBelowRefEdge->SetState( STATE_DONTKNOW );
    }
    m_pTsbBelowRefEdge->SaveValue();

    // SdrMeasureDecimalPlacesItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREDECIMALPLACES );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREDECIMALPLACES );
    if( pItem )
    {
        sal_Int16 nValue = ( ( const SdrMeasureDecimalPlacesItem* )pItem )->GetValue();
        m_pMtrFldDecimalPlaces->SetValue( nValue );
    }
    else
    {
        m_pMtrFldDecimalPlaces->SetText( String() );
    }
    m_pMtrFldDecimalPlaces->SaveValue();

    // SdrMeasureTextRota90Item
    // Attention: negate !
    if( rAttrs.GetItemState( SDRATTR_MEASURETEXTROTA90 ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbParallel->SetState( ( ( const SdrMeasureTextRota90Item& )rAttrs.Get( SDRATTR_MEASURETEXTROTA90 ) ).
                        GetValue() ? STATE_NOCHECK : STATE_CHECK );
        m_pTsbParallel->EnableTriState( sal_False );
    }
    else
    {
        m_pTsbParallel->SetState( STATE_DONTKNOW );
    }
    m_pTsbParallel->SaveValue();

    // SdrMeasureShowUnitItem
    if( rAttrs.GetItemState( SDRATTR_MEASURESHOWUNIT ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbShowUnit->SetState( ( ( const SdrMeasureShowUnitItem& )rAttrs.Get( SDRATTR_MEASURESHOWUNIT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbShowUnit->EnableTriState( sal_False );
    }
    else
    {
        m_pTsbShowUnit->SetState( STATE_DONTKNOW );
    }
    m_pTsbShowUnit->SaveValue();

    // SdrMeasureUnitItem
    if( rAttrs.GetItemState( SDRATTR_MEASUREUNIT ) != SFX_ITEM_DONTCARE )
    {
        long nFieldUnit = (long) ( ( const SdrMeasureUnitItem& )rAttrs.
                                    Get( SDRATTR_MEASUREUNIT ) ).GetValue();

        for( sal_uInt16 i = 0; i < m_pLbUnit->GetEntryCount(); ++i )
        {
            if ( (sal_IntPtr)m_pLbUnit->GetEntryData( i ) == nFieldUnit )
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
    if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTVPOS ) != SFX_ITEM_DONTCARE )
    {
        SdrMeasureTextVPos eVPos = (SdrMeasureTextVPos)
                    ( ( const SdrMeasureTextVPosItem& )rAttrs.Get( SDRATTR_MEASURETEXTVPOS ) ).GetValue();
        {
            if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTHPOS ) != SFX_ITEM_DONTCARE )
            {
                m_pTsbAutoPosV->EnableTriState( sal_False );
                m_pTsbAutoPosH->EnableTriState( sal_False );

                SdrMeasureTextHPos eHPos = (SdrMeasureTextHPos)
                            ( ( const SdrMeasureTextHPosItem& )rAttrs.Get( SDRATTR_MEASURETEXTHPOS ) ).GetValue();
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

                CTL_STATE nState = 0;

                if( eHPos == SDRMEASURE_TEXTHAUTO )
                {
                    m_pTsbAutoPosH->SetState( STATE_CHECK );
                    nState = CS_NOHORZ;
                }

                if( eVPos == SDRMEASURE_TEXTVAUTO )
                {
                    m_pTsbAutoPosV->SetState( STATE_CHECK );
                    nState |= CS_NOVERT;
                }

                m_pCtlPosition->SetState( nState );
                m_pCtlPosition->SetActualRP( eRP );
            }
        }
    }
    else
    {
        m_pCtlPosition->Reset();
        m_pTsbAutoPosV->SetState( STATE_DONTKNOW );
        m_pTsbAutoPosH->SetState( STATE_DONTKNOW );
    }

    // put the attributes to the preview-control,
    // otherwise the control don't know about
    // the settings of the dialog (#67930)
    ChangeAttrHdl_Impl( m_pTsbShowUnit );
    m_pCtlPreview->SetAttributes( rAttrs );

    bPositionModified = sal_False;
}

/*************************************************************************
|*
|* Fill the delivered Item-Set with dialogbox-attributes
|*
\************************************************************************/

sal_Bool SvxMeasurePage::FillItemSet( SfxItemSet& rAttrs)
{
    sal_Bool     bModified = sal_False;
    sal_Int32    nValue;
    TriState eState;

    if( m_pMtrFldLineDist->GetText() != m_pMtrFldLineDist->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldLineDist, eUnit );
        rAttrs.Put( SdrMeasureLineDistItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldHelplineOverhang->GetText() != m_pMtrFldHelplineOverhang->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelplineOverhang, eUnit );
        rAttrs.Put( SdrMeasureHelplineOverhangItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldHelplineDist->GetText() != m_pMtrFldHelplineDist->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelplineDist, eUnit );
        rAttrs.Put( SdrMeasureHelplineDistItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldHelpline1Len->GetText() != m_pMtrFldHelpline1Len->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelpline1Len, eUnit );
        rAttrs.Put( SdrMeasureHelpline1LenItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldHelpline2Len->GetText() != m_pMtrFldHelpline2Len->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldHelpline2Len, eUnit );
        rAttrs.Put( SdrMeasureHelpline2LenItem( nValue ) );
        bModified = sal_True;
    }

    eState = m_pTsbBelowRefEdge->GetState();
    if( eState != m_pTsbBelowRefEdge->GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureBelowRefEdgeItem( (sal_Bool) STATE_CHECK == eState ) );
        bModified = sal_True;
    }

    if( m_pMtrFldDecimalPlaces->GetText() != m_pMtrFldDecimalPlaces->GetSavedValue() )
    {
        nValue = static_cast<sal_Int32>(m_pMtrFldDecimalPlaces->GetValue());
        rAttrs.Put(
            SdrMeasureDecimalPlacesItem(
                sal::static_int_cast< sal_Int16 >( nValue ) ) );
        bModified = sal_True;
    }

    eState = m_pTsbParallel->GetState();
    if( eState != m_pTsbParallel->GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureTextRota90Item( (sal_Bool) STATE_NOCHECK == eState ) );
        bModified = sal_True;
    }

    eState = m_pTsbShowUnit->GetState();
    if( eState != m_pTsbShowUnit->GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureShowUnitItem( (sal_Bool) STATE_CHECK == eState ) );
        bModified = sal_True;
    }

    sal_uInt16 nPos = m_pLbUnit->GetSelectEntryPos();
    if( nPos != m_pLbUnit->GetSavedValue() )
    {
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pLbUnit->GetEntryData( nPos );
            FieldUnit _eUnit = (FieldUnit) nFieldUnit;
            rAttrs.Put( SdrMeasureUnitItem( _eUnit ) );
            bModified = sal_True;
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
        if( m_pTsbAutoPosH->GetState() == STATE_CHECK )
            eHPos = SDRMEASURE_TEXTHAUTO;

        if( m_pTsbAutoPosV->GetState() == STATE_CHECK )
            eVPos = SDRMEASURE_TEXTVAUTO;

        if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTVPOS ) != SFX_ITEM_DONTCARE )
        {
            eOldVPos = (SdrMeasureTextVPos)
                        ( ( const SdrMeasureTextVPosItem& )rOutAttrs.Get( SDRATTR_MEASURETEXTVPOS ) ).GetValue();
            if( eOldVPos != eVPos )
            {
                rAttrs.Put( SdrMeasureTextVPosItem( eVPos ) );
                bModified = sal_True;
            }
        }
        else
        {
            rAttrs.Put( SdrMeasureTextVPosItem( eVPos ) );
            bModified = sal_True;
        }

        if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTHPOS ) != SFX_ITEM_DONTCARE )
        {
            eOldHPos = (SdrMeasureTextHPos)
                        ( ( const SdrMeasureTextHPosItem& )rOutAttrs.Get( SDRATTR_MEASURETEXTHPOS ) ).GetValue();
            if( eOldHPos != eHPos )
            {
                rAttrs.Put( SdrMeasureTextHPosItem( eHPos ) );
                bModified = sal_True;
            }
        }
        else
        {
            rAttrs.Put( SdrMeasureTextHPosItem( eHPos ) );
            bModified = sal_True;
        }
    }

    return( bModified );
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

/*************************************************************************
|*
|* create the tabpage
|*
\************************************************************************/

SfxTabPage* SvxMeasurePage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxMeasurePage( pWindow, rAttrs ) );
}

sal_uInt16* SvxMeasurePage::GetRanges()
{
    return( pRanges );
}

void SvxMeasurePage::PointChanged( Window* pWindow, RECT_POINT /*eRP*/ )
{
    ChangeAttrHdl_Impl( pWindow );
}

IMPL_LINK( SvxMeasurePage, ClickAutoPosHdl_Impl, void *, p )
{
    if( m_pTsbAutoPosH->GetState() == STATE_CHECK )
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
    if( m_pTsbAutoPosV->GetState() == STATE_CHECK )
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

    return( 0L );
}

IMPL_LINK( SvxMeasurePage, ChangeAttrHdl_Impl, void *, p )
{

    if( p == m_pMtrFldLineDist )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldLineDist, eUnit );
        aAttrSet.Put( SdrMeasureLineDistItem( nValue ) );
    }

    if( p == m_pMtrFldHelplineOverhang )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelplineOverhang, eUnit );
        aAttrSet.Put( SdrMeasureHelplineOverhangItem( nValue) );
    }

    if( p == m_pMtrFldHelplineDist )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelplineDist, eUnit );
        aAttrSet.Put( SdrMeasureHelplineDistItem( nValue) );
    }

    if( p == m_pMtrFldHelpline1Len )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelpline1Len, eUnit );
        aAttrSet.Put( SdrMeasureHelpline1LenItem( nValue ) );
    }

    if( p == m_pMtrFldHelpline2Len )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHelpline2Len, eUnit );
        aAttrSet.Put( SdrMeasureHelpline2LenItem( nValue ) );
    }

    if( p == m_pTsbBelowRefEdge )
    {
        TriState eState = m_pTsbBelowRefEdge->GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureBelowRefEdgeItem( (sal_Bool) STATE_CHECK == eState ) );
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
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureTextRota90Item( (sal_Bool) !STATE_CHECK == eState ) );
    }

    if( p == m_pTsbShowUnit )
    {
        TriState eState = m_pTsbShowUnit->GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureShowUnitItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    if( p == m_pLbUnit )
    {
        sal_uInt16 nPos = m_pLbUnit->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pLbUnit->GetEntryData( nPos );
            FieldUnit _eUnit = (FieldUnit) nFieldUnit;
            aAttrSet.Put( SdrMeasureUnitItem( _eUnit ) );
        }
    }

    if( p == m_pTsbAutoPosV || p == m_pTsbAutoPosH || p == m_pCtlPosition )
    {
        bPositionModified = sal_True;

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

        CTL_STATE nState = 0;

        if( m_pTsbAutoPosH->GetState() == STATE_CHECK )
        {
            eHPos = SDRMEASURE_TEXTHAUTO;
            nState = CS_NOHORZ;
        }

        if( m_pTsbAutoPosV->GetState() == STATE_CHECK )
        {
            eVPos = SDRMEASURE_TEXTVAUTO;
            nState |= CS_NOVERT;
        }

        if( p == m_pTsbAutoPosV || p == m_pTsbAutoPosH )
            m_pCtlPosition->SetState( nState );

        aAttrSet.Put( SdrMeasureTextVPosItem( eVPos ) );
        aAttrSet.Put( SdrMeasureTextHPosItem( eHPos ) );
    }

    m_pCtlPreview->SetAttributes( aAttrSet );
    m_pCtlPreview->Invalidate();

    return( 0L );
}

void SvxMeasurePage::FillUnitLB()
{
    // fill ListBox with metrics
    SvxStringArray aMetricArr( SVX_RES( RID_SVXSTR_FIELDUNIT_TABLE ) );

    sal_IntPtr nUnit = FUNIT_NONE;
    OUString aStrMetric( m_pFtAutomatic->GetText());
    sal_uInt16 nPos = m_pLbUnit->InsertEntry( aStrMetric );
    m_pLbUnit->SetEntryData( nPos, (void*)nUnit );

    for( sal_uInt16 i = 0; i < aMetricArr.Count(); ++i )
    {
        aStrMetric = aMetricArr.GetStringByPos( i );
        nUnit = aMetricArr.GetValue( i );
        nPos = m_pLbUnit->InsertEntry( aStrMetric );
        m_pLbUnit->SetEntryData( nPos, (void*)nUnit );
    }
}
void SvxMeasurePage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pOfaPtrItem,OfaPtrItem,SID_OBJECT_LIST,sal_False);

    if (pOfaPtrItem)
        SetView( static_cast<SdrView *>(pOfaPtrItem->GetValue()));

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
