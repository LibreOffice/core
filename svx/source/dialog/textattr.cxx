/*************************************************************************
 *
 *  $RCSfile: textattr.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:34:25 $
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
#include <sfx2/request.hxx> //add CHINA001
#include <ofaitem.hxx> //add CHINA001
#include <svxids.hrc> //add CHINA001
#ifndef _SVX_WRITINGMODEITEM_HXX
#include <writingmodeitem.hxx>
#endif

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

//CHINA001 SvxTextAttrDialog::SvxTextAttrDialog( Window* pParent, const SfxItemSet& rInAttrs,
//CHINA001                              const SdrView* pSdrView ) :
//CHINA001      SfxSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_TEXTATTR )
//CHINA001 {
//CHINA001 SvxTextAttrPage* pPage = new SvxTextAttrPage( this, rInAttrs );
//CHINA001
//CHINA001 pPage->SetView( pSdrView );
//CHINA001 pPage->Construct();
//CHINA001
//CHINA001 SetTabPage( pPage );
//CHINA001 SetText( pPage->GetText() );
//CHINA001 }
//CHINA001
/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

//CHINA001 SvxTextAttrDialog::~SvxTextAttrDialog()
//CHINA001 {
//CHINA001 }

/*************************************************************************
|*
|* Dialog (Seite) zum Kopieren von Objekten
|*
\************************************************************************/

SvxTextAttrPage::SvxTextAttrPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SvxTabPage      ( pWindow, ResId( RID_SVXPAGE_TEXTATTR, DIALOG_MGR() ),
                                  rInAttrs ),

                aFlText         ( this, ResId( FL_TEXT ) ),
                aTsbAutoGrowWidth  ( this, ResId( TSB_AUTOGROW_WIDTH ) ),
                aTsbAutoGrowHeight ( this, ResId( TSB_AUTOGROW_HEIGHT ) ),
                aTsbFitToSize   ( this, ResId( TSB_FIT_TO_SIZE ) ),
                aTsbContour     ( this, ResId( TSB_CONTOUR ) ),
                aTsbWordWrapText( this, ResId( TSB_WORDWRAP_TEXT ) ),
                aTsbAutoGrowSize( this, ResId( TSB_AUTOGROW_SIZE ) ),
                aFlDistance     ( this, ResId( FL_DISTANCE ) ),
                aFtLeft         ( this, ResId( FT_LEFT ) ),
                aMtrFldLeft     ( this, ResId( MTR_FLD_LEFT ) ),
                aFtRight        ( this, ResId( FT_RIGHT ) ),
                aMtrFldRight    ( this, ResId( MTR_FLD_RIGHT ) ),
                aFtTop          ( this, ResId( FT_TOP ) ),
                aMtrFldTop      ( this, ResId( MTR_FLD_TOP ) ),
                aFtBottom       ( this, ResId( FT_BOTTOM ) ),
                aMtrFldBottom   ( this, ResId( MTR_FLD_BOTTOM ) ),

                aFlSeparator    ( this, ResId( FL_SEPARATOR ) ),

                aFlPosition     ( this, ResId( FL_POSITION ) ),
                aCtlPosition    ( this, ResId( CTL_POSITION ),
                                            RP_MM, 240, 100 ),
                aTsbFullWidth   ( this, ResId( TSB_FULL_WIDTH ) ),

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

    // autogrowsize
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWSIZE ) != SFX_ITEM_DONTCARE )
    {
        aTsbAutoGrowSize.SetState( ( ( const SdrTextAutoGrowSizeItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWSIZE ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbAutoGrowSize.EnableTriState( FALSE );
    }
    else
        aTsbAutoGrowSize.SetState( STATE_DONTKNOW );
    aTsbAutoGrowSize.SaveValue();

    // wordwrap text
    if ( rAttrs.GetItemState( SDRATTR_TEXT_WORDWRAP ) != SFX_ITEM_DONTCARE )
    {
        aTsbWordWrapText.SetState( ( ( const SdrTextWordWrapItem& )rAttrs.Get( SDRATTR_TEXT_WORDWRAP ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbWordWrapText.EnableTriState( FALSE );
    }
    else
        aTsbWordWrapText.SetState( STATE_DONTKNOW );
    aTsbWordWrapText.SaveValue();


    // #103516# Do the setup based on states of hor/ver adjust
    // Setup center field and FullWidth
    SfxItemState eVState = rAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

    if(SFX_ITEM_DONTCARE != eVState && SFX_ITEM_DONTCARE != eHState)
    {
        // VertAdjust and HorAdjust are unequivocal, thus
        SdrTextVertAdjust eTVA = (SdrTextVertAdjust)((const SdrTextVertAdjustItem&)rAttrs.Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
        SdrTextHorzAdjust eTHA = (SdrTextHorzAdjust)((const SdrTextHorzAdjustItem&)rAttrs.Get(SDRATTR_TEXT_HORZADJUST)).GetValue();
        RECT_POINT eRP;

        aTsbFullWidth.EnableTriState( FALSE );

        // Translate item values into local anchor position.
        switch (eTVA)
        {
            case SDRTEXTVERTADJUST_TOP:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RP_LT; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RP_MT; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RP_RT; break;
                }
                break;
            }
            case SDRTEXTVERTADJUST_BLOCK:
            case SDRTEXTVERTADJUST_CENTER:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RP_LM; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RP_MM; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RP_RM; break;
                }
                break;
            }
            case SDRTEXTVERTADJUST_BOTTOM:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RP_LB; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RP_MB; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RP_RB; break;
                }
            }
        }

        // See if we have to check the "full width" check button.
        sal_Bool bLeftToRight(IsTextDirectionLeftToRight());

        if((bLeftToRight && (SDRTEXTHORZADJUST_BLOCK == eTHA)) || (!bLeftToRight && (SDRTEXTVERTADJUST_BLOCK == eTVA)))
        {
            // Move anchor to valid position.
            ClickFullWidthHdl_Impl(NULL);
            aTsbFullWidth.SetState(STATE_CHECK);
        }

        aCtlPosition.SetActualRP( eRP );
    }
    else
    {
        // VertAdjust or HorAdjust is not unequivocal
        aCtlPosition.Reset();

        aCtlPosition.SetState(STATE_DONTKNOW);
        aCtlPosition.DoCompletelyDisable(sal_True);

        aTsbFullWidth.SetState(STATE_DONTKNOW);
        aTsbFullWidth.Enable( sal_False );
        aFlPosition.Enable( sal_False );
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

    eState = aTsbAutoGrowSize.GetState();
    if( eState != aTsbAutoGrowSize.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowSizeItem( (BOOL) STATE_CHECK == eState ) );
    }

    eState = aTsbWordWrapText.GetState();
    if( eState != aTsbWordWrapText.GetSavedValue() )
    {
        rAttrs.Put( SdrTextWordWrapItem( (BOOL) STATE_CHECK == eState ) );
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

    // #103516# Do not change values if adjust controls were disabled.
    sal_Bool bIsDisabled(aCtlPosition.IsCompletelyDisabled());

    if(!bIsDisabled)
    {
        if( aTsbFullWidth.GetState() == STATE_CHECK )
            if (IsTextDirectionLeftToRight())
                eTHA = SDRTEXTHORZADJUST_BLOCK;
            else
                eTVA = SDRTEXTVERTADJUST_BLOCK;

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
    }

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

    bFitToSizeEnabled = bContourEnabled = TRUE;
    bWordWrapTextEnabled = bAutoGrowSizeEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = FALSE;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if( pObj->GetObjInventor() == SdrInventor )
        {
            switch( eKind )
            {
                case OBJ_TEXT :
                case OBJ_TITLETEXT :
                case OBJ_OUTLINETEXT :
                case OBJ_CAPTION :
                {
                    if ( ((SdrTextObj*)pObj)->HasText() )
                    {
                        // Konturfluss ist NICHT bei reinen Textobjekten m”glich
                        bContourEnabled = FALSE;

                        // Breite und Hoehe anpassen ist NUR bei reinen Textobjekten m”glich
                        bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = TRUE;
                    }
                }
                break;
                case OBJ_CUSTOMSHAPE :
                {
                    if ( ((SdrTextObj*)pObj)->HasText() )
                    {
                        bFitToSizeEnabled = bContourEnabled = FALSE;
                        bAutoGrowSizeEnabled = TRUE;
                        bWordWrapTextEnabled = TRUE;
                    }
                }
                break;
            }
        }
    }
    aTsbAutoGrowHeight.Enable( bAutoGrowHeightEnabled );
    aTsbAutoGrowWidth.Enable( bAutoGrowWidthEnabled );
    aTsbFitToSize.Enable( bFitToSizeEnabled );
    aTsbContour.Enable( bContourEnabled );
    aTsbAutoGrowSize.Enable( bAutoGrowSizeEnabled );
    aTsbWordWrapText.Enable( bWordWrapTextEnabled );
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

/** Check whether we have to uncheck the "Full width" check box.
*/
void SvxTextAttrPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    if (aTsbFullWidth.GetState() == STATE_CHECK)
    {
        // Depending on write direction and currently checked anchor we have
        // to uncheck the "full width" button.
        if (IsTextDirectionLeftToRight())
            switch( eRP )
            {
                case RP_LT:
                case RP_LM:
                case RP_LB:
                case RP_RT:
                case RP_RM:
                case RP_RB:
                    aTsbFullWidth.SetState( STATE_NOCHECK );
            }
        else
            switch (eRP)
            {
                case RP_LT:
                case RP_MT:
                case RP_RT:
                case RP_LB:
                case RP_MB:
                case RP_RB:
                    aTsbFullWidth.SetState( STATE_NOCHECK );
            }
    }
}

/*************************************************************************
|*
|* Aendert evtl. die Position des Positions-Controls
|*
\************************************************************************/

/** When switching the "full width" check button on the text anchor may have
    to be moved to a valid and adjacent position.  This position depends on
    the current anchor position and the text writing direction.
*/
IMPL_LINK( SvxTextAttrPage, ClickFullWidthHdl_Impl, void *, EMPTYARG )
{
    if( aTsbFullWidth.GetState() == STATE_CHECK )
    {
        if (IsTextDirectionLeftToRight())
        {
            // Move text anchor to horizontal middle axis.
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
        else
        {
            // Move text anchor to vertical middle axis.
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
                        !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                        bContourEnabled );

    aTsbAutoGrowWidth.Enable( !bFitToSize &&
                              !( bContour && bContourEnabled ) &&
                              bAutoGrowWidthEnabled );

    aTsbAutoGrowHeight.Enable( !bFitToSize &&
                               !( bContour && bContourEnabled ) &&
                               bAutoGrowHeightEnabled );

    aTsbFitToSize.Enable( !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                          !( bContour && bContourEnabled ) &&
                          bFitToSizeEnabled );

    // #101901# enable/disable metric fields and decorations dependent of contour
    aMtrFldLeft.Enable(!bContour);
    aMtrFldRight.Enable(!bContour);
    aMtrFldTop.Enable(!bContour);
    aMtrFldBottom.Enable(!bContour);
    aFlDistance.Enable(!bContour);
    aFtLeft.Enable(!bContour);
    aFtRight.Enable(!bContour);
    aFtTop.Enable(!bContour);
    aFtBottom.Enable(!bContour);

    if( bContour && bContourEnabled )
    {
        aMtrFldLeft.SetValue( 0 );
        aMtrFldRight.SetValue( 0 );
        aMtrFldTop.SetValue( 0 );
        aMtrFldBottom.SetValue( 0 );
    }

    // #103516# Do the setup based on states of hor/ver adjust
    SfxItemState eVState = rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );
    sal_Bool bHorAndVer(SFX_ITEM_DONTCARE == eVState || SFX_ITEM_DONTCARE == eHState);

    // #83698# enable/disable text anchoring dependent of contour
    aCtlPosition.Enable(!bContour && !bHorAndVer);
    aTsbFullWidth.Enable(!bContour && !bHorAndVer);
    aFlPosition.Enable(!bContour && !bHorAndVer);

    return( 0L );
}


bool SvxTextAttrPage::IsTextDirectionLeftToRight (void) const
{
    // Determine the text writing direction with left to right as default.
    bool bLeftToRightDirection = true;
    SfxItemState eState = rOutAttrs.GetItemState(SDRATTR_TEXTDIRECTION);

    if(SFX_ITEM_DONTCARE != eState)
    {
        const SvxWritingModeItem& rItem = static_cast<const SvxWritingModeItem&> (
            rOutAttrs.Get (SDRATTR_TEXTDIRECTION));
        if (rItem.GetValue() == com::sun::star::text::WritingMode_TB_RL)
            bLeftToRightDirection = false;
    }
    return bLeftToRightDirection;
}

//add CHINA001 begin
void SvxTextAttrPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pViewItem,OfaPtrItem,SID_SVXTEXTATTRPAGE_VIEW,sal_False);

    if (pViewItem)
        SetView( static_cast<SdrView *>(pViewItem->GetValue()));

    Construct();
}

//end of CHINA001
