/*************************************************************************
 *
 *  $RCSfile: textattr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2000-10-24 12:20:14 $
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

#pragma hdrstop

#include "dialogs.hrc"

#ifndef _SVDDEF_HXX //autogen
#include "svddef.hxx"
#endif
#ifndef SDTMITM_HXX //autogen
#include "sdtditm.hxx"
#endif
#ifndef _SDTAGITM_HXX //autogen
#include "sdtagitm.hxx"
#endif
#ifndef _SDTAITM_HXX //autogen
#include "sdtaitm.hxx"
#endif
#ifndef _SDTFSITM_HXX //autogen
#include "sdtfsitm.hxx"
#endif
#ifndef _SDTCFITM_HXX //autogen
#include "sdtcfitm.hxx"
#endif
#ifndef _SVDOBJ_HXX //autogen
#include "svdobj.hxx"
#endif
#ifndef _SVDMARK_HXX //autogen
#include "svdmark.hxx"
#endif
#ifndef _SVDVIEW_HXX //autogen
#include "svdview.hxx"
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include "svdotext.hxx"
#endif


#ifndef _SVX_TEXTATTR_CXX
#define _SVX_TEXTATTR_CXX
#endif
#include "textattr.hxx"
#include "textattr.hrc"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

static USHORT pRanges[] =
{
    SDRATTR_MISC_FIRST,
    SDRATTR_TEXT_HORZADJUST,
    0
};

/*************************************************************************
|*
|* Dialog zum Kopieren von Objekten
|*
\************************************************************************/

SvxTextAttrDialog::SvxTextAttrDialog( Window* pParent, const SfxItemSet& rInAttrs,
                                const SdrView* pSdrView ) :
        SfxSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_TEXTATTR, FALSE )
{
    SvxTextAttrPage* pPage = new SvxTextAttrPage( this, rInAttrs );

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

SvxTextAttrDialog::~SvxTextAttrDialog()
{
}

/*************************************************************************
|*
|* Dialog (Seite) zum Kopieren von Objekten
|*
\************************************************************************/

SvxTextAttrPage::SvxTextAttrPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SvxTabPage      ( pWindow, ResId( RID_SVXPAGE_TEXTATTR, DIALOG_MGR() ),
                                  rInAttrs ),

                aTsbAutoGrowWidth  ( this, ResId( TSB_AUTOGROW_WIDTH ) ),
                aTsbAutoGrowHeight ( this, ResId( TSB_AUTOGROW_HEIGHT ) ),
                aTsbFitToSize   ( this, ResId( TSB_FIT_TO_SIZE ) ),
                aTsbContour     ( this, ResId( TSB_CONTOUR ) ),
                aGrpText        ( this, ResId( GRP_TEXT ) ),

                aFtLeft         ( this, ResId( FT_LEFT ) ),
                aMtrFldLeft     ( this, ResId( MTR_FLD_LEFT ) ),
                aFtRight        ( this, ResId( FT_RIGHT ) ),
                aMtrFldRight    ( this, ResId( MTR_FLD_RIGHT ) ),
                aFtTop          ( this, ResId( FT_TOP ) ),
                aMtrFldTop      ( this, ResId( MTR_FLD_TOP ) ),
                aFtBottom       ( this, ResId( FT_BOTTOM ) ),
                aMtrFldBottom   ( this, ResId( MTR_FLD_BOTTOM ) ),
                aGrpDistance    ( this, ResId( GRP_DISTANCE ) ),

                aCtlPosition    ( this, ResId( CTL_POSITION ),
                                            RP_MM, 240, 100 ),
                aTsbFullWidth   ( this, ResId( TSB_FULL_WIDTH ) ),
                aGrpPosition    ( this, ResId( GRP_POSITION ) ),

                rOutAttrs       ( rInAttrs )
{
    FreeResource();

    FieldUnit eFUnit = GetModuleFieldUnit( &rInAttrs );
    SetFieldUnit( aMtrFldLeft, eFUnit );
    SetFieldUnit( aMtrFldRight, eFUnit );
    SetFieldUnit( aMtrFldTop, eFUnit );
    SetFieldUnit( aMtrFldBottom, eFUnit );

    Link aLink( LINK( this, SvxTextAttrPage, ClickHdl_Impl ) );
    aTsbAutoGrowWidth.SetClickHdl( aLink );
    aTsbAutoGrowHeight.SetClickHdl( aLink );
    aTsbFitToSize.SetClickHdl( aLink );
    aTsbContour.SetClickHdl( aLink );

    aTsbFullWidth.SetClickHdl(
        LINK( this, SvxTextAttrPage, ClickFullWidthHdl_Impl ) );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxTextAttrPage::~SvxTextAttrPage()
{
}

/*************************************************************************
|*
|* Liest uebergebenen Item-Set
|*
\************************************************************************/

void __EXPORT SvxTextAttrPage::Reset( const SfxItemSet& rAttrs )
{
    SfxItemPool* pPool = rAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    // Linker Abstand vom Rahmen
    const SfxPoolItem* pItem = GetItem( rAttrs, SDRATTR_TEXT_LEFTDIST );

    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LEFTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextLeftDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldLeft, nValue, eUnit );
    }
    else
        aMtrFldLeft.SetText( String() );
    aMtrFldLeft.SaveValue();

    // Rechter Abstand vom Rahmen
    pItem = GetItem( rAttrs, SDRATTR_TEXT_RIGHTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_RIGHTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextRightDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldRight, nValue, eUnit );
    }
    else
        aMtrFldRight.SetText( String() );
    aMtrFldRight.SaveValue();

    // Oberer Abstand vom Rahmen
    pItem = GetItem( rAttrs, SDRATTR_TEXT_UPPERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_UPPERDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextUpperDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldTop, nValue, eUnit );
    }
    else
        aMtrFldTop.SetText( String() );
    aMtrFldTop.SaveValue();

    // Unterer Abstand vom Rahmen
    pItem = GetItem( rAttrs, SDRATTR_TEXT_LOWERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LOWERDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextLowerDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldBottom, nValue, eUnit );
    }
    else
        aMtrFldBottom.SetText( String() );
    aMtrFldBottom.SaveValue();

    // An Hoehe anpassen
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) != SFX_ITEM_DONTCARE )
    {
        aTsbAutoGrowHeight.SetState( ( ( const SdrTextAutoGrowHeightItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbAutoGrowHeight.EnableTriState( FALSE );
    }
    else
        aTsbAutoGrowHeight.SetState( STATE_DONTKNOW );
    aTsbAutoGrowHeight.SaveValue();

    // An Breite anpassen
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWWIDTH ) != SFX_ITEM_DONTCARE )
    {
        aTsbAutoGrowWidth.SetState( ( ( const SdrTextAutoGrowWidthItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWWIDTH ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbAutoGrowWidth.EnableTriState( FALSE );
    }
    else
        aTsbAutoGrowWidth.SetState( STATE_DONTKNOW );
    aTsbAutoGrowWidth.SaveValue();

    // zentriert
    if ( rAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST ) != SFX_ITEM_DONTCARE )
    {
        SdrTextVertAdjust eTVA = (SdrTextVertAdjust)
                    ( ( const SdrTextVertAdjustItem& )rAttrs.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue();
        {
            if ( rAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST ) != SFX_ITEM_DONTCARE )
            {
                aTsbFullWidth.EnableTriState( FALSE );

                SdrTextHorzAdjust eTHA = (SdrTextHorzAdjust)
                            ( ( const SdrTextHorzAdjustItem& )rAttrs.Get( SDRATTR_TEXT_HORZADJUST ) ).GetValue();
                RECT_POINT eRP;
                if( eTVA == SDRTEXTVERTADJUST_TOP )
                {
                    if( eTHA == SDRTEXTHORZADJUST_LEFT )   eRP = RP_LT;
                    else if( eTHA == SDRTEXTHORZADJUST_CENTER ) eRP = RP_MT;
                    else if( eTHA == SDRTEXTHORZADJUST_RIGHT )  eRP = RP_RT;
                    else if( eTHA == SDRTEXTHORZADJUST_BLOCK )
                    {
                        eRP = RP_MT;
                        aTsbFullWidth.SetState( STATE_CHECK );
                    }
                }
                else if( eTVA == SDRTEXTVERTADJUST_CENTER )
                {
                    if( eTHA == SDRTEXTHORZADJUST_LEFT )   eRP = RP_LM;
                    else if( eTHA == SDRTEXTHORZADJUST_CENTER ) eRP = RP_MM;
                    else if( eTHA == SDRTEXTHORZADJUST_RIGHT )  eRP = RP_RM;
                    else if( eTHA == SDRTEXTHORZADJUST_BLOCK )
                    {
                        eRP = RP_MM;
                        aTsbFullWidth.SetState( STATE_CHECK );
                    }
                }
                else if( eTVA == SDRTEXTVERTADJUST_BOTTOM )
                {
                    if( eTHA == SDRTEXTHORZADJUST_LEFT )   eRP = RP_LB;
                    else if( eTHA == SDRTEXTHORZADJUST_CENTER ) eRP = RP_MB;
                    else if( eTHA == SDRTEXTHORZADJUST_RIGHT )  eRP = RP_RB;
                    else if( eTHA == SDRTEXTHORZADJUST_BLOCK )
                    {
                        eRP = RP_MB;
                        aTsbFullWidth.SetState( STATE_CHECK );
                    }
                }

                aCtlPosition.SetActualRP( eRP );
            }
        }
    }
    else
    {
        aCtlPosition.Reset();
        aTsbFullWidth.SetState( STATE_DONTKNOW );
    }

    // Am Rahmen anpassen
    if ( rAttrs.GetItemState( SDRATTR_TEXT_FITTOSIZE ) != SFX_ITEM_DONTCARE )
    {
        SdrFitToSizeType eFTS = (SdrFitToSizeType)
                    ( ( const SdrTextFitToSizeTypeItem& )rAttrs.Get( SDRATTR_TEXT_FITTOSIZE ) ).GetValue();
        aTsbFitToSize.SetState( eFTS == SDRTEXTFIT_NONE ? STATE_NOCHECK : STATE_CHECK );
        aTsbFitToSize.EnableTriState( FALSE );
    }
    else
        aTsbFitToSize.SetState( STATE_DONTKNOW );
    aTsbFitToSize.SaveValue();

    // Konturfluss
    if( rAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME ) != SFX_ITEM_DONTCARE )
    {
        BOOL bContour = ( ( const SdrTextContourFrameItem& )rAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
        aTsbContour.SetState( bContour ? STATE_CHECK : STATE_NOCHECK );
        aTsbContour.EnableTriState( FALSE );
    }
    else
        aTsbContour.SetState( STATE_DONTKNOW );
    aTsbContour.SaveValue();

    ClickHdl_Impl( NULL );
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\************************************************************************/

BOOL SvxTextAttrPage::FillItemSet( SfxItemSet& rAttrs)
{
    SfxItemPool* pPool = rAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    INT32    nValue;
    TriState eState;

    if( aMtrFldLeft.GetText() != aMtrFldLeft.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLeft, eUnit );
        rAttrs.Put( SdrTextLeftDistItem( nValue ) );
    }

    if( aMtrFldRight.GetText() != aMtrFldRight.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldRight, eUnit );
        rAttrs.Put( SdrTextRightDistItem( nValue ) );
    }

    if( aMtrFldTop.GetText() != aMtrFldTop.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldTop, eUnit );
        rAttrs.Put( SdrTextUpperDistItem( nValue ) );
    }

    if( aMtrFldBottom.GetText() != aMtrFldBottom.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldBottom, eUnit );
        rAttrs.Put( SdrTextLowerDistItem( nValue ) );
    }

    eState = aTsbAutoGrowHeight.GetState();
    if( eState != aTsbAutoGrowHeight.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowHeightItem( (BOOL) STATE_CHECK == eState ) );
    }

    eState = aTsbAutoGrowWidth.GetState();
    if( eState != aTsbAutoGrowWidth.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowWidthItem( (BOOL) STATE_CHECK == eState ) );
    }

    // Konturfluss
    eState = aTsbContour.GetState();
    if( eState != aTsbContour.GetSavedValue() )
    {
        rAttrs.Put( SdrTextContourFrameItem( (BOOL) STATE_CHECK == eState ) );
    }

    eState = aTsbFitToSize.GetState();
    if( eState != aTsbFitToSize.GetSavedValue() )
    {
        SdrFitToSizeType eFTS;
        switch( eState )
        {
            case STATE_NOCHECK: eFTS = SDRTEXTFIT_NONE; break;
            //case STATE_CHECK: eFTS = SDRTEXTFIT_RESIZEATTR; break;
            case STATE_CHECK: eFTS = SDRTEXTFIT_PROPORTIONAL; break;
        }
        rAttrs.Put( SdrTextFitToSizeTypeItem( eFTS ) );
    }

    // zentriert
    RECT_POINT eRP = aCtlPosition.GetActualRP();
    SdrTextVertAdjust eTVA, eOldTVA;
    SdrTextHorzAdjust eTHA, eOldTHA;

    switch( eRP )
    {
        case RP_LT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RP_LM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RP_LB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RP_MT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RP_MM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RP_MB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RP_RT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
        case RP_RM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
        case RP_RB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
    }
    if( aTsbFullWidth.GetState() == STATE_CHECK )
        eTHA = SDRTEXTHORZADJUST_BLOCK;

    if ( rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST ) != SFX_ITEM_DONTCARE )
    {
        eOldTVA = (SdrTextVertAdjust)
                    ( ( const SdrTextVertAdjustItem& )rOutAttrs.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue();
        if( eOldTVA != eTVA )
            rAttrs.Put( SdrTextVertAdjustItem( eTVA ) );
    }
    else
        rAttrs.Put( SdrTextVertAdjustItem( eTVA ) );

    if ( rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST ) != SFX_ITEM_DONTCARE )
    {
        eOldTHA = (SdrTextHorzAdjust)
                    ( ( const SdrTextHorzAdjustItem& )rOutAttrs.Get( SDRATTR_TEXT_HORZADJUST ) ).GetValue();
        if( eOldTHA != eTHA )
            rAttrs.Put( SdrTextHorzAdjustItem( eTHA ) );
    }
    else
        rAttrs.Put( SdrTextHorzAdjustItem( eTHA ) );

    return( TRUE );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxTextAttrPage::Construct()
{
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    bContourEnabled = TRUE;
    bAutoGrowSizeEnabled = FALSE;

    const SdrMarkList& rMarkList = pView->GetMarkList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if( pObj->GetObjInventor() == SdrInventor &&
            ( eKind==OBJ_TEXT || eKind==OBJ_TITLETEXT || eKind==OBJ_OUTLINETEXT
              || eKind==OBJ_CAPTION ) &&
            ( (SdrTextObj*) pObj )->HasText() )
        {
            // Konturfluss ist NICHT bei reinen Textobjekten m”glich
            bContourEnabled = FALSE;

            // Breite und Hoehe anpassen ist NUR bei reinen Textobjekten m”glich
            bAutoGrowSizeEnabled = TRUE;
        }
    }
    aTsbAutoGrowHeight.Enable( bAutoGrowSizeEnabled );
    aTsbAutoGrowWidth.Enable( bAutoGrowSizeEnabled );
    aTsbContour.Enable( bContourEnabled );
}

/*************************************************************************
|*
|* Erzeugt die Seite
|*
\************************************************************************/

SfxTabPage* SvxTextAttrPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxTextAttrPage( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

USHORT* SvxTextAttrPage::GetRanges()
{
    return( pRanges );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxTextAttrPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    switch( eRP )
    {
        case RP_LT:
        case RP_LM:
        case RP_LB:
        case RP_RT:
        case RP_RM:
        case RP_RB:
            if( aTsbFullWidth.GetState() == STATE_CHECK )
                aTsbFullWidth.SetState( STATE_NOCHECK );
        break;
    }
}

/*************************************************************************
|*
|* Aendert evtl. die Position des Positions-Controls
|*
\************************************************************************/

IMPL_LINK( SvxTextAttrPage, ClickFullWidthHdl_Impl, void *, EMPTYARG )
{
    if( aTsbFullWidth.GetState() == STATE_CHECK )
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
    return( 0L );
}

/*************************************************************************
|*
|* Enabled/Disabled "Groesse an Text" oder "Am Rahmen Anpassen"
|*
\************************************************************************/

IMPL_LINK( SvxTextAttrPage, ClickHdl_Impl, void *, p )
{
    BOOL bAutoGrowWidth  = aTsbAutoGrowWidth.GetState() == STATE_CHECK;
    BOOL bAutoGrowHeight = aTsbAutoGrowHeight.GetState() == STATE_CHECK;
    BOOL bFitToSize      = aTsbFitToSize.GetState() == STATE_CHECK;
    BOOL bContour        = aTsbContour.GetState() == STATE_CHECK;

    aTsbContour.Enable( !bFitToSize &&
                        !( ( bAutoGrowWidth || bAutoGrowHeight ) && bAutoGrowSizeEnabled ) &&
                        bContourEnabled );

    aTsbAutoGrowWidth.Enable( !bFitToSize &&
                              !( bContour && bContourEnabled ) &&
                              bAutoGrowSizeEnabled );

    aTsbAutoGrowHeight.Enable( !bFitToSize &&
                               !( bContour && bContourEnabled ) &&
                               bAutoGrowSizeEnabled );

    aTsbFitToSize.Enable( !( ( bAutoGrowWidth || bAutoGrowHeight ) && bAutoGrowSizeEnabled ) &&
                          !( bContour && bContourEnabled ) );

    aMtrFldLeft.Enable( !(bContour && bContourEnabled) );
    aMtrFldRight.Enable( !(bContour && bContourEnabled) );
    aMtrFldTop.Enable( !(bContour && bContourEnabled) );
    aMtrFldBottom.Enable( !(bContour && bContourEnabled) );

/*
    // Am Rahmen anpassen
    if( bFitToSize )
    {
        aTsbAutoGrowWidth.Enable( FALSE );
        aTsbAutoGrowHeight.Enable( FALSE );
        aTsbContour.Enable( FALSE );
    }
    else
    {
        aTsbAutoGrowWidth.Enable( bAutoGrowSizeEnabled );
        aTsbAutoGrowHeight.Enable( bAutoGrowSizeEnabled );
        aTsbContour.Enable( bContourEnabled );
    }

    // An Hoehe oder Breite anpassen
    if( ( bAutoGrowWidth || bAutoGrowHeight ) && bAutoGrowSizeEnabled )
    {
        aTsbFitToSize.Enable( FALSE );
        aTsbContour.Enable( FALSE );
    }
    else
    {
        aTsbFitToSize.Enable( TRUE );
        aTsbContour.Enable( bContourEnabled );
    }

    // Kontur
    if( bContour && bContourEnabled )
    {
        aTsbAutoGrowWidth.Enable( FALSE );
        aTsbAutoGrowHeight.Enable( FALSE );
        aTsbFitToSize.Enable( FALSE );
    }
    else
    {
        aTsbAutoGrowWidth.Enable( bAutoGrowSizeEnabled && !bFitToSize );
        aTsbAutoGrowHeight.Enable( bAutoGrowSizeEnabled && !bFitToSize );
        aMtrFldLeft.Enable( TRUE );
        aMtrFldRight.Enable( TRUE );
        aMtrFldTop.Enable( TRUE );
        aMtrFldBottom.Enable( TRUE );
        aTsbFitToSize.Enable( TRUE );
    }
*/

//////////
    /*
    if( p == &aTsbFitToSize ||
        p == NULL )
    {
        if( aTsbFitToSize.GetState() == STATE_CHECK )
        {
            aTsbAutoGrowWidth.Enable( FALSE );
            aTsbAutoGrowHeight.Enable( FALSE );
        }
        else if( bAutoGrowSizeEnabled )
        {
            aTsbAutoGrowWidth.Enable( TRUE );
            aTsbAutoGrowHeight.Enable( TRUE );
        }
    }
    if( p == &aTsbAutoGrowWidth ||
        p == &aTsbAutoGrowHeight ||
        p == NULL )
    {
        if( ( aTsbAutoGrowWidth.GetState() == STATE_CHECK && aTsbAutoGrowWidth.IsEnabled() ) ||
            ( aTsbAutoGrowHeight.GetState() == STATE_CHECK && aTsbAutoGrowHeight.IsEnabled() ) )
            aTsbFitToSize.Enable( FALSE );
        else
            aTsbFitToSize.Enable( TRUE );
    }
    if( p == &aTsbFitToSize ||
        p == NULL )
    {
        BOOL bOff = aTsbFitToSize.GetState() == STATE_NOCHECK;
        aTsbContour.Enable( bOff );
    }
    if( p == &aTsbContour ||
        p == NULL )
    {
        BOOL bOff = aTsbContour.GetState() == STATE_NOCHECK;
        aMtrFldLeft.Enable( bOff );
        aMtrFldRight.Enable( bOff );
        aMtrFldTop.Enable( bOff );
        aMtrFldBottom.Enable( bOff );
        aTsbFitToSize.Enable( bOff );
    }
    */
    return( 0L );
}


