/*************************************************************************
 *
 *  $RCSfile: measure.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


// include ---------------------------------------------------------------

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#pragma hdrstop

#include "dialogs.hrc"

#define _SVX_MEASURE_CXX

#include "svdomeas.hxx"
#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdview.hxx"

#include "measctrl.hxx"
#include "measure.hxx"
#include "measure.hrc"
#include "dialmgr.hxx"
#include "dlgutil.hxx"
#include "strarray.hxx"


static USHORT pRanges[] =
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
        SfxSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_MEASURE, FALSE )
{
    SvxMeasurePage* pPage = new SvxMeasurePage( this, rInAttrs );

    pPage->SetView( pSdrView );
    pPage->Construct();

    SetTabPage( pPage );
    SetText( pPage->GetText() );
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
                SvxTabPage      ( pWindow, ResId( RID_SVXPAGE_MEASURE, DIALOG_MGR() ),
                                  rInAttrs ),

        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() ),

        aFtLineDist             ( this, ResId( FT_LINE_DIST ) ),
        aMtrFldLineDist         ( this, ResId( MTR_LINE_DIST ) ),
        aFtHelplineOverhang     ( this, ResId( FT_HELPLINE_OVERHANG ) ),
        aMtrFldHelplineOverhang ( this, ResId( MTR_FLD_HELPLINE_OVERHANG ) ),
        aFtHelplineDist         ( this, ResId( FT_HELPLINE_DIST ) ),
        aMtrFldHelplineDist     ( this, ResId( MTR_FLD_HELPLINE_DIST ) ),
        aFtHelpline1Len         ( this, ResId( FT_HELPLINE1_LEN ) ),
        aMtrFldHelpline1Len     ( this, ResId( MTR_FLD_HELPLINE1_LEN ) ),
        aFtHelpline2Len         ( this, ResId( FT_HELPLINE2_LEN ) ),
        aMtrFldHelpline2Len     ( this, ResId( MTR_FLD_HELPLINE2_LEN ) ),
        aTsbBelowRefEdge        ( this, ResId( TSB_BELOW_REF_EDGE ) ),
        aTsbParallel            ( this, ResId( TSB_PARALLEL ) ),
        aTsbShowUnit            ( this, ResId( TSB_SHOW_UNIT ) ),
        aLbUnit                 ( this, ResId( LB_UNIT ) ),
        aGrpAttributes          ( this, ResId( GRP_ATTRIBUTES ) ),

        aCtlPosition            ( this, ResId( CTL_POSITION ),
                                        RP_RM, 200, 100, CS_LINE ),
        aTsbAutoPosV            ( this, ResId( TSB_AUTOPOSV ) ),
        aTsbAutoPosH            ( this, ResId( TSB_AUTOPOSH ) ),
        aGrpPosition            ( this, ResId( GRP_POSITION ) ),

        aCtlPreview             ( this, ResId( CTL_PREVIEW ), rInAttrs ),
        aGrpPreview             ( this, ResId( GRP_PREVIEW ) ),
        bPositionModified       ( FALSE )
{
    FillUnitLB();

    FreeResource();

    FieldUnit eFUnit;
    GET_MODULE_FIELDUNIT( eFUnit );
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

    Link aLink( LINK( this, SvxMeasurePage, ChangeAttrHdl_Impl ) );
    aMtrFldLineDist.SetModifyHdl( aLink );
    aMtrFldHelplineOverhang.SetModifyHdl( aLink );
    aMtrFldHelplineDist.SetModifyHdl( aLink );
    aMtrFldHelpline1Len.SetModifyHdl( aLink );
    aMtrFldHelpline2Len.SetModifyHdl( aLink );
    aTsbBelowRefEdge.SetClickHdl( aLink );
    aTsbParallel.SetClickHdl( aLink );
    aTsbShowUnit.SetClickHdl( aLink );
    aLbUnit.SetSelectHdl( aLink );
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

void __EXPORT SvxMeasurePage::Reset( const SfxItemSet& rAttrs )
{
    SfxItemPool* pPool = rAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_MEASURELINEDIST );

    const SfxPoolItem* pItem = GetItem( rAttrs, SDRATTR_MEASURELINEDIST );

    // SdrMeasureLineDistItem
    if( pItem || (pItem = &pPool->GetDefaultItem( SDRATTR_MEASURELINEDIST )) )
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
    if( pItem || (pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEOVERHANG )))
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
    if( pItem || (pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINEDIST )))
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
    if( pItem || (pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE1LEN )))
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
    if( pItem || (pItem = &pPool->GetDefaultItem( SDRATTR_MEASUREHELPLINE2LEN )))
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
        aTsbBelowRefEdge.EnableTriState( FALSE );
    }
    else
    {
        aTsbBelowRefEdge.SetState( STATE_DONTKNOW );
    }
    aTsbBelowRefEdge.SaveValue();

    // SdrMeasureTextRota90Item
    // Attention: negate !
    if( rAttrs.GetItemState( SDRATTR_MEASURETEXTROTA90 ) != SFX_ITEM_DONTCARE )
    {
        aTsbParallel.SetState( ( ( const SdrMeasureTextRota90Item& )rAttrs.Get( SDRATTR_MEASURETEXTROTA90 ) ).
                        GetValue() ? STATE_NOCHECK : STATE_CHECK );
        aTsbParallel.EnableTriState( FALSE );
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
        aTsbShowUnit.EnableTriState( FALSE );
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

        for( USHORT i = 0; i < aLbUnit.GetEntryCount(); ++i )
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
                aTsbAutoPosV.EnableTriState( FALSE );
                aTsbAutoPosH.EnableTriState( FALSE );

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

    bPositionModified = FALSE;
}

/*************************************************************************
|*
|* Fill the delivered Item-Set with dialogbox-attributes
|*
\************************************************************************/

BOOL SvxMeasurePage::FillItemSet( SfxItemSet& rAttrs)
{
    BOOL     bModified = FALSE;
    INT32    nValue;
    TriState eState;

    if( aMtrFldLineDist.GetText() != aMtrFldLineDist.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLineDist, eUnit );
        rAttrs.Put( SdrMeasureLineDistItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldHelplineOverhang.GetText() != aMtrFldHelplineOverhang.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelplineOverhang, eUnit );
        rAttrs.Put( SdrMeasureHelplineOverhangItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldHelplineDist.GetText() != aMtrFldHelplineDist.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelplineDist, eUnit );
        rAttrs.Put( SdrMeasureHelplineDistItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldHelpline1Len.GetText() != aMtrFldHelpline1Len.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelpline1Len, eUnit );
        rAttrs.Put( SdrMeasureHelpline1LenItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldHelpline2Len.GetText() != aMtrFldHelpline2Len.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHelpline2Len, eUnit );
        rAttrs.Put( SdrMeasureHelpline2LenItem( nValue ) );
        bModified = TRUE;
    }

    eState = aTsbBelowRefEdge.GetState();
    if( eState != aTsbBelowRefEdge.GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureBelowRefEdgeItem( (BOOL) STATE_CHECK == eState ) );
        bModified = TRUE;
    }

    eState = aTsbParallel.GetState();
    if( eState != aTsbParallel.GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureTextRota90Item( (BOOL) STATE_NOCHECK == eState ) );
        bModified = TRUE;
    }

    eState = aTsbShowUnit.GetState();
    if( eState != aTsbShowUnit.GetSavedValue() )
    {
        rAttrs.Put( SdrMeasureShowUnitItem( (BOOL) STATE_CHECK == eState ) );
        bModified = TRUE;
    }

    USHORT nPos = aLbUnit.GetSelectEntryPos();
    if( nPos != aLbUnit.GetSavedValue() )
    {
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            USHORT nFieldUnit = (USHORT)(long)aLbUnit.GetEntryData( nPos );
            FieldUnit eUnit = (FieldUnit) nFieldUnit;
            rAttrs.Put( SdrMeasureUnitItem( eUnit ) );
            bModified = TRUE;
        }
    }

//enum SdrMeasureTextHPos {SDRMEASURE_TEXTHAUTO,SDRMEASURE_TEXTLEFTOUTSIDE,SDRMEASURE_TEXTINSIDE,SDRMEASURE_TEXTRIGHTOUTSIDE};
//enum SdrMeasureTextVPos {SDRMEASURE_TEXTVAUTO,SDRMEASURE_ABOVE,SDRMEASURETEXT_VERTICALCENTERED,SDRMEASURE_BELOW};

    if( bPositionModified )
    {
        // Position
        SdrMeasureTextVPos eVPos, eOldVPos;
        SdrMeasureTextHPos eHPos, eOldHPos;

        RECT_POINT eRP = aCtlPosition.GetActualRP();
        switch( eRP )
        {
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
                bModified = TRUE;
            }
        }
        else
        {
            rAttrs.Put( SdrMeasureTextVPosItem( eVPos ) );
            bModified = TRUE;
        }

        if ( rAttrs.GetItemState( SDRATTR_MEASURETEXTHPOS ) != SFX_ITEM_DONTCARE )
        {
            eOldHPos = (SdrMeasureTextHPos)
                        ( ( const SdrMeasureTextHPosItem& )rOutAttrs.Get( SDRATTR_MEASURETEXTHPOS ) ).GetValue();
            if( eOldHPos != eHPos )
            {
                rAttrs.Put( SdrMeasureTextHPosItem( eHPos ) );
                bModified = TRUE;
            }
        }
        else
        {
            rAttrs.Put( SdrMeasureTextHPosItem( eHPos ) );
            bModified = TRUE;
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

/*************************************************************************
|*
|*
|*
\************************************************************************/

USHORT* SvxMeasurePage::GetRanges()
{
    return( pRanges );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxMeasurePage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    ChangeAttrHdl_Impl( pWindow );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

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
        }
    }
    ChangeAttrHdl_Impl( p );

    return( 0L );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxMeasurePage, ChangeAttrHdl_Impl, void *, p )
{

    if( p == &aMtrFldLineDist )
    {
        INT32 nValue = GetCoreValue( aMtrFldLineDist, eUnit );
        aAttrSet.Put( SdrMeasureLineDistItem( nValue ) );
    }

    if( p == &aMtrFldHelplineOverhang )
    {
        INT32 nValue = GetCoreValue( aMtrFldHelplineOverhang, eUnit );
        aAttrSet.Put( SdrMeasureHelplineOverhangItem( nValue) );
    }

    if( p == &aMtrFldHelplineDist )
    {
        INT32 nValue = GetCoreValue( aMtrFldHelplineDist, eUnit );
        aAttrSet.Put( SdrMeasureHelplineDistItem( nValue) );
    }

    if( p == &aMtrFldHelpline1Len )
    {
        INT32 nValue = GetCoreValue( aMtrFldHelpline1Len, eUnit );
        aAttrSet.Put( SdrMeasureHelpline1LenItem( nValue ) );
    }

    if( p == &aMtrFldHelpline2Len )
    {
        INT32 nValue = GetCoreValue( aMtrFldHelpline2Len, eUnit );
        aAttrSet.Put( SdrMeasureHelpline2LenItem( nValue ) );
    }

    if( p == &aTsbBelowRefEdge )
    {
        TriState eState = aTsbBelowRefEdge.GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureBelowRefEdgeItem( (BOOL) STATE_CHECK == eState ) );
    }

    if( p == &aTsbParallel )
    {
        TriState eState = aTsbParallel.GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureTextRota90Item( (BOOL) !STATE_CHECK == eState ) );
    }

    if( p == &aTsbShowUnit )
    {
        TriState eState = aTsbShowUnit.GetState();
        if( eState != STATE_DONTKNOW )
            aAttrSet.Put( SdrMeasureShowUnitItem( (BOOL) STATE_CHECK == eState ) );
    }

    if( p == &aLbUnit )
    {
        USHORT nPos = aLbUnit.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            USHORT nFieldUnit = (USHORT)(long)aLbUnit.GetEntryData( nPos );
            FieldUnit eUnit = (FieldUnit) nFieldUnit;
            aAttrSet.Put( SdrMeasureUnitItem( eUnit ) );
        }
    }

    if( p == &aTsbAutoPosV || p == &aTsbAutoPosH || p == &aCtlPosition )
    {
        bPositionModified = TRUE;

        // Position
        RECT_POINT eRP = aCtlPosition.GetActualRP();
        SdrMeasureTextVPos eVPos;
        SdrMeasureTextHPos eHPos;

        switch( eRP )
        {
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

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxMeasurePage::FillUnitLB()
{
    // fill ListBox with metrics
    SvxStringArray aMetricArr( RID_SVXSTR_FIELDUNIT_TABLE );

    long nUnit = FUNIT_NONE;
    String aStrMetric( ResId( STR_MEASURE_AUTOMATIC ) );
    USHORT nPos = aLbUnit.InsertEntry( aStrMetric );
    aLbUnit.SetEntryData( nPos, (void*)nUnit );

    for( USHORT i = 0; i < aMetricArr.Count(); ++i )
    {
        aStrMetric = aMetricArr.GetStringByPos( i );
        nUnit = aMetricArr.GetValue( i );
        nPos = aLbUnit.InsertEntry( aStrMetric );
        aLbUnit.SetEntryData( nPos, (void*)nUnit );
    }
}


