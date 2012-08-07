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

#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <tools/shl.hxx>

#include <cuires.hrc>

#define _SVX_MEASURE_CXX

#include <svx/svdomeas.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdview.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "svx/measctrl.hxx"
#include "measure.hxx"
#include "measure.hrc"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/strarray.hxx>
#include <sfx2/request.hxx>
#include "svx/ofaitem.hxx"

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
        SfxSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_MEASURE )
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
                SvxTabPage      ( pWindow, CUI_RES( RID_SVXPAGE_MEASURE ),
                                  rInAttrs ),

        aFlLine                 ( this, CUI_RES( FL_LINE ) ),
        aFtLineDist             ( this, CUI_RES( FT_LINE_DIST ) ),
        aMtrFldLineDist         ( this, CUI_RES( MTR_LINE_DIST ) ),
        aFtHelplineOverhang     ( this, CUI_RES( FT_HELPLINE_OVERHANG ) ),
        aMtrFldHelplineOverhang ( this, CUI_RES( MTR_FLD_HELPLINE_OVERHANG ) ),
        aFtHelplineDist         ( this, CUI_RES( FT_HELPLINE_DIST ) ),
        aMtrFldHelplineDist     ( this, CUI_RES( MTR_FLD_HELPLINE_DIST ) ),
        aFtHelpline1Len         ( this, CUI_RES( FT_HELPLINE1_LEN ) ),
        aMtrFldHelpline1Len     ( this, CUI_RES( MTR_FLD_HELPLINE1_LEN ) ),
        aFtHelpline2Len         ( this, CUI_RES( FT_HELPLINE2_LEN ) ),
        aMtrFldHelpline2Len     ( this, CUI_RES( MTR_FLD_HELPLINE2_LEN ) ),
        aTsbBelowRefEdge        ( this, CUI_RES( TSB_BELOW_REF_EDGE ) ),
        aFtDecimalPlaces        ( this, CUI_RES( FT_DECIMALPLACES ) ),
        aMtrFldDecimalPlaces    ( this, CUI_RES( MTR_FLD_DECIMALPLACES ) ),

        aFlLabel                ( this, CUI_RES( FL_LABEL ) ),
        aFtPosition             ( this, CUI_RES( FT_POSITION ) ),
        aCtlPosition            ( this, CUI_RES( CTL_POSITION ) ),
        aTsbAutoPosV            ( this, CUI_RES( TSB_AUTOPOSV ) ),
        aTsbAutoPosH            ( this, CUI_RES( TSB_AUTOPOSH ) ),
        aTsbShowUnit            ( this, CUI_RES( TSB_SHOW_UNIT ) ),
        aLbUnit                 ( this, CUI_RES( LB_UNIT ) ),
        aTsbParallel            ( this, CUI_RES( TSB_PARALLEL ) ),
        aCtlPreview             ( this, CUI_RES( CTL_PREVIEW ), rInAttrs ),

        aFlVert                 ( this, CUI_RES( FL_VERT ) ),
        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() ),
        pView( 0 ),

        bPositionModified       ( sal_False )
{
    aCtlPreview.SetAccessibleName(aCtlPreview.GetHelpText());
    FillUnitLB();

    FreeResource();

    const FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( aMtrFldLineDist, eFUnit );
    SetFieldUnit( aMtrFldHelplineOverhang, eFUnit );
    SetFieldUnit( aMtrFldHelplineDist, eFUnit );
    SetFieldUnit( aMtrFldHelpline1Len, eFUnit );
    SetFieldUnit( aMtrFldHelpline2Len, eFUnit );
    if( eFUnit == FUNIT_MM )
    {
        aMtrFldLineDist.SetSpinSize( 50 );
        aMtrFldHelplineOverhang.SetSpinSize( 50 );
        aMtrFldHelplineDist.SetSpinSize( 50 );
        aMtrFldHelpline1Len.SetSpinSize( 50 );
        aMtrFldHelpline2Len.SetSpinSize( 50 );
    }

    aTsbAutoPosV.SetClickHdl( LINK( this, SvxMeasurePage, ClickAutoPosHdl_Impl ) );
    aTsbAutoPosH.SetClickHdl( LINK( this, SvxMeasurePage, ClickAutoPosHdl_Impl ) );

    // set background and border of iconchoicectrl
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    aCtlPreview.SetBackground ( rStyles.GetWindowColor() );
    aCtlPreview.SetBorderStyle(WINDOW_BORDER_MONO);

    Link aLink( LINK( this, SvxMeasurePage, ChangeAttrHdl_Impl ) );
    aMtrFldLineDist.SetModifyHdl( aLink );
    aMtrFldHelplineOverhang.SetModifyHdl( aLink );
    aMtrFldHelplineDist.SetModifyHdl( aLink );
    aMtrFldHelpline1Len.SetModifyHdl( aLink );
    aMtrFldHelpline2Len.SetModifyHdl( aLink );
    aMtrFldDecimalPlaces.SetModifyHdl( aLink );
    aTsbBelowRefEdge.SetClickHdl( aLink );
    aTsbParallel.SetClickHdl( aLink );
    aTsbShowUnit.SetClickHdl( aLink );
    aLbUnit.SetSelectHdl( aLink );
    aLbUnit.SetAccessibleName(GetNonMnemonicString(aTsbShowUnit.GetText()));
    aCtlPosition.SetAccessibleRelationMemberOf( &aFlLabel );
    aLbUnit.SetAccessibleRelationLabeledBy( &aTsbShowUnit );
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
        SetMetricValue( aMtrFldLineDist, nValue, eUnit );
    }
    else
    {
        aMtrFldLineDist.SetText( String() );
    }
    aMtrFldLineDist.SaveValue();

    // SdrMeasureHelplineOverhangItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEOVERHANG );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelplineOverhangItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldHelplineOverhang, nValue, eUnit );
    }
    else
    {
        aMtrFldHelplineOverhang.SetText( String() );
    }
    aMtrFldHelplineOverhang.SaveValue();

    // SdrMeasureHelplineDistItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINEDIST );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelplineDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldHelplineDist, nValue, eUnit );
    }
    else
    {
        aMtrFldHelplineDist.SetText( String() );
    }
    aMtrFldHelplineDist.SaveValue();

    // SdrMeasureHelpline1LenItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE1LEN );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelpline1LenItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldHelpline1Len, nValue, eUnit );
    }
    else
    {
        aMtrFldHelpline1Len.SetText( String() );
    }
    aMtrFldHelpline1Len.SaveValue();

    // SdrMeasureHelpline2LenItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE2LEN );
    if( pItem )
    {
        long nValue = ( ( const SdrMeasureHelpline2LenItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldHelpline2Len, nValue, eUnit );
    }
    else
    {
        aMtrFldHelpline2Len.SetText( String() );
    }
    aMtrFldHelpline2Len.SaveValue();

    // SdrMeasureBelowRefEdgeItem
    if( rAttrs.GetItemState( SDRATTR_MEASUREBELOWREFEDGE ) != SFX_ITEM_DONTCARE )
    {
        aTsbBelowRefEdge.SetState( ( ( const SdrMeasureBelowRefEdgeItem& )rAttrs.Get( SDRATTR_MEASUREBELOWREFEDGE ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbBelowRefEdge.EnableTriState( sal_False );
    }
    else
    {
        aTsbBelowRefEdge.SetState( STATE_DONTKNOW );
    }
    aTsbBelowRefEdge.SaveValue();

    // SdrMeasureDecimalPlacesItem
    pItem = GetItem( rAttrs, SDRATTR_MEASUREDECIMALPLACES );
    if( pItem == NULL )
        pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREDECIMALPLACES );
    if( pItem )
    {
        sal_Int16 nValue = ( ( const SdrMeasureDecimalPlacesItem* )pItem )->GetValue();
        aMtrFldDecimalPlaces.SetValue( nValue );
    }
    else
    {
        aMtrFldDecimalPlaces.SetText( String() );
    }
    aMtrFldDecimalPlaces.SaveValue();

    // SdrMeasureTextRota90Item
    // Attention: negate !
    if( rAttrs.GetItemState( SDRATTR_MEASURETEXTROTA90 ) != SFX_ITEM_DONTCARE )
    {
        aTsbParallel.SetState( ( ( const SdrMeasureTextRota90Item& )rAttrs.Get( SDRATTR_MEASURETEXTROTA90 ) ).
                        GetValue() ? STATE_NOCHECK : STATE_CHECK );
        aTsbParallel.EnableTriState( sal_False );
    }
    else
    {
        aTsbParallel.SetState( STATE_DONTKNOW );
    }
    aTsbParallel.SaveValue();

    // SdrMeasureShowUnitItem
    if( rAttrs.GetItemState( SDRATTR_MEASURESHOWUNIT ) != SFX_ITEM_DONTCARE )
    {
        aTsbShowUnit.SetState( ( ( const SdrMeasureShowUnitItem& )rAttrs.Get( SDRATTR_MEASURESHOWUNIT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbShowUnit.EnableTriState( sal_False );
    }
    else
    {
        aTsbShowUnit.SetState( STATE_DONTKNOW );
    }
    aTsbShowUnit.SaveValue();

    // SdrMeasureUnitItem
    if( rAttrs.GetItemState( SDRATTR_MEASUREUNIT ) != SFX_ITEM_DONTCARE )
    {
        long nFieldUnit = (long) ( ( const SdrMeasureUnitItem& )rAttrs.
                                    Get( SDRATTR_MEASUREUNIT ) ).GetValue();

        for( sal_uInt16 i = 0; i < aLbUnit.GetEntryCount(); ++i )
        {
            if ( (long)aLbUnit.GetEntryData( i ) == nFieldUnit )
            {
                aLbUnit.SelectEntryPos( i );
                break;
            }
        }
    }
    else
    {
        aLbUnit.SetNoSelection();
    }
    aLbUnit.SaveValue();

    // Position
    if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTVPOS ) != SFX_ITEM_DONTCARE )
    {
        SdrMeasureTextVPos eVPos = (SdrMeasureTextVPos)
                    ( ( const SdrMeasureTextVPosItem& )rAttrs.Get( SDRATTR_MEASURETEXTVPOS ) ).GetValue();
        {
            if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTHPOS ) != SFX_ITEM_DONTCARE )
            {
                aTsbAutoPosV.EnableTriState( sal_False );
                aTsbAutoPosH.EnableTriState( sal_False );

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
                    aTsbAutoPosH.SetState( STATE_CHECK );
                    nState = CS_NOHORZ;
                }

                if( eVPos == SDRMEASURE_TEXTVAUTO )
                {
                    aTsbAutoPosV.SetState( STATE_CHECK );
                    nState |= CS_NOVERT;
                }

                aCtlPosition.SetState( nState );
                aCtlPosition.SetActualRP( eRP );
            }
        }
    }
    else
    {
        aCtlPosition.Reset();
        aTsbAutoPosV.SetState( STATE_DONTKNOW );
        aTsbAutoPosH.SetState( STATE_DONTKNOW );
    }

    // put the attributes to the preview-control,
    // otherwise the control don't know about
    // the settings of the dialog (#67930)
    ChangeAttrHdl_Impl( &aTsbShowUnit );
    aCtlPreview.SetAttributes( rAttrs );

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

    if( aMtrFldLineDist.GetText() != aMtrFldLineDist.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLineDist, eUnit );
        rAttrs.Put( SdrMeasureLineDistItem( nValue ) );
        bModified = sal_True;
    }

    if( aMtrFldHelplineOverhang.GetText() != aMtrFldHelplineOverhang.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelplineOverhang, eUnit );
        rAttrs.Put( SdrMeasureHelplineOverhangItem( nValue ) );
        bModified = sal_True;
    }

    if( aMtrFldHelplineDist.GetText() != aMtrFldHelplineDist.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelplineDist, eUnit );
        rAttrs.Put( SdrMeasureHelplineDistItem( nValue ) );
        bModified = sal_True;
    }

    if( aMtrFldHelpline1Len.GetText() != aMtrFldHelpline1Len.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelpline1Len, eUnit );
        rAttrs.Put( SdrMeasureHelpline1LenItem( nValue ) );
        bModified = sal_True;
    }

    if( aMtrFldHelpline2Len.GetText() != aMtrFldHelpline2Len.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelpline2Len, eUnit );
        rAttrs.Put( SdrMeasureHelpline2LenItem( nValue ) );
        bModified = sal_True;
    }

    eState = aTsbBelowRefEdge.GetState();
    if( eState != aTsbBelowRefEdge.GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureBelowRefEdgeItem( (sal_Bool) STATE_CHECK == eState ) );
        bModified = sal_True;
    }

    if( aMtrFldDecimalPlaces.GetText() != aMtrFldDecimalPlaces.GetSavedValue() )
    {
        nValue = static_cast<sal_Int32>(aMtrFldDecimalPlaces.GetValue());
        rAttrs.Put(
            SdrMeasureDecimalPlacesItem(
                sal::static_int_cast< sal_Int16 >( nValue ) ) );
        bModified = sal_True;
    }

    eState = aTsbParallel.GetState();
    if( eState != aTsbParallel.GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureTextRota90Item( (sal_Bool) STATE_NOCHECK == eState ) );
        bModified = sal_True;
    }

    eState = aTsbShowUnit.GetState();
    if( eState != aTsbShowUnit.GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureShowUnitItem( (sal_Bool) STATE_CHECK == eState ) );
        bModified = sal_True;
    }

    sal_uInt16 nPos = aLbUnit.GetSelectEntryPos();
    if( nPos != aLbUnit.GetSavedValue() )
    {
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nFieldUnit = (sal_uInt16)(long)aLbUnit.GetEntryData( nPos );
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

        RECT_POINT eRP = aCtlPosition.GetActualRP();
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
        if( aTsbAutoPosH.GetState() == STATE_CHECK )
            eHPos = SDRMEASURE_TEXTHAUTO;

        if( aTsbAutoPosV.GetState() == STATE_CHECK )
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

    aCtlPreview.pMeasureObj->SetModel( pView->GetModel() );
    aCtlPreview.Invalidate();
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
    if( aTsbAutoPosH.GetState() == STATE_CHECK )
    {
        switch( aCtlPosition.GetActualRP() )
        {
            case RP_LT:
            case RP_RT:
                aCtlPosition.SetActualRP( RP_MT );
            break;

            case RP_LM:
            case RP_RM:
                aCtlPosition.SetActualRP( RP_MM );
            break;

            case RP_LB:
            case RP_RB:
                aCtlPosition.SetActualRP( RP_MB );
            break;
            default: ;//prevent warning
        }
    }
    if( aTsbAutoPosV.GetState() == STATE_CHECK )
    {
        switch( aCtlPosition.GetActualRP() )
        {
            case RP_LT:
            case RP_LB:
                aCtlPosition.SetActualRP( RP_LM );
            break;

            case RP_MT:
            case RP_MB:
                aCtlPosition.SetActualRP( RP_MM );
            break;

            case RP_RT:
            case RP_RB:
                aCtlPosition.SetActualRP( RP_RM );
            break;
            default: ;//prevent warning
        }
    }
    ChangeAttrHdl_Impl( p );

    return( 0L );
}

IMPL_LINK( SvxMeasurePage, ChangeAttrHdl_Impl, void *, p )
{

    if( p == &aMtrFldLineDist )
    {
        sal_Int32 nValue = GetCoreValue( aMtrFldLineDist, eUnit );
        aAttrSet.Put( SdrMeasureLineDistItem( nValue ) );
    }

    if( p == &aMtrFldHelplineOverhang )
    {
        sal_Int32 nValue = GetCoreValue( aMtrFldHelplineOverhang, eUnit );
        aAttrSet.Put( SdrMeasureHelplineOverhangItem( nValue) );
    }

    if( p == &aMtrFldHelplineDist )
    {
        sal_Int32 nValue = GetCoreValue( aMtrFldHelplineDist, eUnit );
        aAttrSet.Put( SdrMeasureHelplineDistItem( nValue) );
    }

    if( p == &aMtrFldHelpline1Len )
    {
        sal_Int32 nValue = GetCoreValue( aMtrFldHelpline1Len, eUnit );
        aAttrSet.Put( SdrMeasureHelpline1LenItem( nValue ) );
    }

    if( p == &aMtrFldHelpline2Len )
    {
        sal_Int32 nValue = GetCoreValue( aMtrFldHelpline2Len, eUnit );
        aAttrSet.Put( SdrMeasureHelpline2LenItem( nValue ) );
    }

    if( p == &aTsbBelowRefEdge )
    {
        TriState eState = aTsbBelowRefEdge.GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureBelowRefEdgeItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    if( p == &aMtrFldDecimalPlaces )
    {
        sal_Int16 nValue = sal::static_int_cast< sal_Int16 >(
            aMtrFldDecimalPlaces.GetValue() );
        aAttrSet.Put( SdrMeasureDecimalPlacesItem( nValue ) );
    }

    if( p == &aTsbParallel )
    {
        TriState eState = aTsbParallel.GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureTextRota90Item( (sal_Bool) !STATE_CHECK == eState ) );
    }

    if( p == &aTsbShowUnit )
    {
        TriState eState = aTsbShowUnit.GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureShowUnitItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    if( p == &aLbUnit )
    {
        sal_uInt16 nPos = aLbUnit.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nFieldUnit = (sal_uInt16)(long)aLbUnit.GetEntryData( nPos );
            FieldUnit _eUnit = (FieldUnit) nFieldUnit;
            aAttrSet.Put( SdrMeasureUnitItem( _eUnit ) );
        }
    }

    if( p == &aTsbAutoPosV || p == &aTsbAutoPosH || p == &aCtlPosition )
    {
        bPositionModified = sal_True;

        // Position
        RECT_POINT eRP = aCtlPosition.GetActualRP();
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

        if( aTsbAutoPosH.GetState() == STATE_CHECK )
        {
            eHPos = SDRMEASURE_TEXTHAUTO;
            nState = CS_NOHORZ;
        }

        if( aTsbAutoPosV.GetState() == STATE_CHECK )
        {
            eVPos = SDRMEASURE_TEXTVAUTO;
            nState |= CS_NOVERT;
        }

        if( p == &aTsbAutoPosV || p == &aTsbAutoPosH )
            aCtlPosition.SetState( nState );

        aAttrSet.Put( SdrMeasureTextVPosItem( eVPos ) );
        aAttrSet.Put( SdrMeasureTextHPosItem( eHPos ) );
    }

    aCtlPreview.SetAttributes( aAttrSet );
    aCtlPreview.Invalidate();

    return( 0L );
}

void SvxMeasurePage::FillUnitLB()
{
    // fill ListBox with metrics
    SvxStringArray aMetricArr( SVX_RES( RID_SVXSTR_FIELDUNIT_TABLE ) );

    long nUnit = FUNIT_NONE;
    String aStrMetric( CUI_RES( STR_MEASURE_AUTOMATIC ) );
    sal_uInt16 nPos = aLbUnit.InsertEntry( aStrMetric );
    aLbUnit.SetEntryData( nPos, (void*)nUnit );

    for( sal_uInt16 i = 0; i < aMetricArr.Count(); ++i )
    {
        aStrMetric = aMetricArr.GetStringByPos( i );
        nUnit = aMetricArr.GetValue( i );
        nPos = aLbUnit.InsertEntry( aStrMetric );
        aLbUnit.SetEntryData( nPos, (void*)nUnit );
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
