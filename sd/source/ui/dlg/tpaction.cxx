/*************************************************************************
 *
 *  $RCSfile: tpaction.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:00:20 $
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


#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XVNDSUNSTARSCRIPTURL_HPP_
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "sdattr.hxx"

#define ITEMID_COLOR            ATTR_ANIMATION_COLOR
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <sfx2/app.hxx>
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdresid.hxx"
#include "tpaction.hxx"
#include "tpaction.hrc"
#include "strmname.h"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "filedlg.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

static USHORT pActionRanges[] =
{
    ATTR_ANIMATION_TRANSPCOLOR,
    ATTR_ANIMATION_TRANSPCOLOR,
    ATTR_ACTION_START,
    ATTR_ACTION_END,
    0
};

static USHORT pEffectRanges[] =
{
    ATTR_ANIMATION_START,
    ATTR_ANIMATION_END,
    0
};


#define DOCUMENT_TOKEN (sal_Unicode('#'))


/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdActionDlg::SdActionDlg (
    ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView ) :
        SfxSingleTabDialog  ( pParent, *pAttr, TP_ANIMATION_ACTION ),
        rOutAttrs           ( *pAttr )
{
    // FreeResource();
    SfxTabPage* pPage = SdTPAction::Create( this, rOutAttrs );
    DBG_ASSERT( pPage, "Seite konnte nicht erzeugt werden");

    // Ehemals in PageCreated
    ( (SdTPAction*) pPage )->SetView( pView );
    ( (SdTPAction*) pPage )->Construct();

    SetTabPage( pPage );

    String aStr( pPage->GetText() );
    if( aStr.Len() )
        SetText( aStr );
}


/*************************************************************************
|*
|*  Action-TabPage
|*
\************************************************************************/

SdTPAction::SdTPAction( Window* pWindow, const SfxItemSet& rInAttrs ) :
        SfxTabPage      ( pWindow, SdResId( TP_ANIMATION ), rInAttrs ),

        aFtAction       ( this, SdResId( FT_ACTION ) ),
        aLbAction       ( this, SdResId( LB_ACTION ) ),
        aFlEffect       ( this, SdResId( FL_EFFECT ) ),
        aFtAnimation    ( this, SdResId( FT_ANIMATION ) ),
        aLbEffect       ( this, SdResId( LB_EFFECT ) ),
        aFtSpeed        ( this, SdResId( FT_SPEED ) ),
        aRbtSlow        ( this, SdResId( RBT_SLOW ) ),
        aRbtMedium      ( this, SdResId( RBT_MEDIUM ) ),
        aRbtFast        ( this, SdResId( RBT_FAST ) ),
        aTsbSound       ( this, SdResId( TSB_SOUND ) ),
        aEdtSound       ( this, SdResId( EDT_SOUND ) ),
        aBtnSearch      ( this, SdResId( BTN_SEARCH ) ),
        aTsbPlayFull    ( this, SdResId( TSB_PLAY_FULL ) ),
        aFtTree         ( this, SdResId( FT_TREE ) ),
        aLbTree         ( this, SdResId( LB_TREE ) ),
        aLbTreeDocument ( this, SdResId( LB_TREE_DOCUMENT ) ),
        aLbOLEAction    ( this, SdResId( LB_OLE_ACTION ) ),
        aFlSeparator    ( this, SdResId( FL_SEPARATOR ) ),
        aEdtBookmark    ( this, SdResId( EDT_BOOKMARK ) ),
        aEdtDocument    ( this, SdResId( EDT_DOCUMENT ) ),
        aEdtProgram     ( this, SdResId( EDT_PROGRAM ) ),
        aEdtMacro       ( this, SdResId( EDT_MACRO ) ),
        aBtnSeek        ( this, SdResId( BTN_SEEK ) ),

        pView           ( NULL ),
        pDoc            ( NULL ),
        rOutAttrs       ( rInAttrs ),
        bTreeUpdated    ( FALSE )
{
    FreeResource();

    aTsbSound.SetClickHdl( LINK( this, SdTPAction, ClickSoundHdl ) );
    aLbEffect.SetSelectHdl( LINK( this, SdTPAction, ChangeEffectHdl ) );
    aBtnSearch.SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );
    aBtnSeek.SetClickHdl( LINK( this, SdTPAction, ClickSearchHdl ) );

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    pCurrentEffects = new List;
    pCurrentActions = new List;

    aLbAction.SetSelectHdl( LINK( this, SdTPAction, ClickActionHdl ) );
    aLbTree.SetSelectHdl( LINK( this, SdTPAction, SelectTreeHdl ) );
    aEdtDocument.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );
    aEdtMacro.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );

    // Controls enablen
    aFtAction.Show();
    aLbAction.Show();
}

// -----------------------------------------------------------------------

SdTPAction::~SdTPAction()
{
    delete pCurrentEffects;
    delete pCurrentActions;
}

// -----------------------------------------------------------------------

void SdTPAction::SetView( const ::sd::View* pSdView )
{
    pView = pSdView;

    // Holen der ColorTable und Fuellen der ListBox
    ::sd::DrawDocShell* pDocSh =
          static_cast<const ::sd::View*>(pView)->GetDocSh();
    pDoc = pDocSh->GetDoc();
    SfxViewFrame* pFrame = pDocSh->GetViewShell()->GetViewFrame();
    aLbTree.SetViewFrame( pFrame );
    aLbTreeDocument.SetViewFrame( pFrame );

    SvxColorTableItem aItem( *(const SvxColorTableItem*)( pDocSh->GetItem( SID_COLOR_TABLE ) ) );
    pColTab = aItem.GetColorTable();
    DBG_ASSERT( pColTab, "Keine Farbtabelle vorhanden!" );
}

// -----------------------------------------------------------------------

void SdTPAction::Construct()
{
    // OLE-Actionlistbox auffuellen
    SdrOle2Obj* pOleObj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    BOOL        bDisableAll = FALSE;
    BOOL        bOLEAction = FALSE;

    if ( pView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        SdrObject* pObj;

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetObj();

            UINT32 nInv = pObj->GetObjInventor();
            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                pOleObj = (SdrOle2Obj*) pObj;
            }
            else if (nInv == SdrInventor && nSdrObjKind == OBJ_GRAF)
            {
                pGrafObj = (SdrGrafObj*) pObj;
            }
//          // VCXControl ?
//          else if( pObj->IsA( TYPE( VCSbxDrawObject ) ) )
//          {
//              bDisableAll = TRUE;
//          }
        }
    }
    if( pGrafObj )
    {
        bOLEAction = TRUE;

        aVerbVector.push_back( 0 );
        aLbOLEAction.InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( String( SdResId( STR_EDIT_OBJ ) ) ) );
    }
    else if( pOleObj )
    {
        SvInPlaceObjectRef aIPObj = pOleObj->GetObjRef();

        if ( aIPObj.Is() )
        {
            bOLEAction = TRUE;
            const SvVerbList* pList = &aIPObj->GetVerbList();
            for( USHORT i = 0; i < pList->Count() ; i++ )
            {
                const SvVerb& rVerb = pList->GetObject( i );
                if( rVerb.IsOnMenu() )
                {
                    String aTmp( rVerb.GetName() );
                    aVerbVector.push_back( rVerb.GetId() );
                    aLbOLEAction.InsertEntry( MnemonicGenerator::EraseAllMnemonicChars( aTmp ) );
                }
            }
        }
    }

    // auf einer Masterpage ?
    BOOL bOnMaster = FALSE;
    if (pView->GetPageViewPvNum(0)->GetPage()->IsMasterPage())
    {
        bOnMaster = TRUE;
    }

    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_NONE, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_LEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_UPPERLEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_TOP, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_UPPERRIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_RIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_LOWERRIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_BOTTOM, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_LOWERLEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_TO_CENTER, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_FADE_FROM_CENTER, LIST_APPEND);
    //pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_FROM_LEFT, LIST_APPEND);
    //pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_FROM_TOP, LIST_APPEND);
    //pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_FROM_RIGHT, LIST_APPEND);
    //pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_FROM_BOTTOM, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_VERTICAL_STRIPES, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_HORIZONTAL_STRIPES, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_VERTICAL_LINES, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_HORIZONTAL_LINES, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_VERTICAL_CHECKERBOARD, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_HORIZONTAL_CHECKERBOARD, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_CLOCKWISE, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_COUNTERCLOCKWISE, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_CLOSE_VERTICAL, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_CLOSE_HORIZONTAL, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_OPEN_VERTICAL, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_OPEN_HORIZONTAL, LIST_APPEND);
    //pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_PATH
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_SPIRALIN_LEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_SPIRALIN_RIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_SPIRALOUT_LEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_SPIRALOUT_RIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_DISSOLVE, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_WAVYLINE_FROM_LEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_WAVYLINE_FROM_TOP, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_WAVYLINE_FROM_RIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_WAVYLINE_FROM_BOTTOM, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_LEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_UPPERLEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_TOP, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_UPPERRIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_RIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_LOWERRIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_BOTTOM, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_TO_LOWERLEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_LEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_UPPERLEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_TOP, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_RIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_BOTTOM, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_MOVE_SHORT_TO_LOWERLEFT, LIST_APPEND);
    pCurrentEffects->Insert((void*)(ULONG)presentation::AnimationEffect_RANDOM, LIST_APPEND);

    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_NONE, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_PREVPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_NEXTPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_FIRSTPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_LASTPAGE, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_BOOKMARK, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_DOCUMENT, LIST_APPEND);
    if (!bOnMaster)
        pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_INVISIBLE, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_SOUND, LIST_APPEND);
    if (bOLEAction && aLbOLEAction.GetEntryCount())
        pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_VERB, LIST_APPEND);
    if (!bOnMaster)
        pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_VANISH, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_PROGRAM, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_MACRO, LIST_APPEND);
    pCurrentActions->Insert((void*)(ULONG)presentation::ClickAction_STOPPRESENTATION, LIST_APPEND);

    // Action-Listbox fuellen
    for (ULONG nAction = 0; nAction < pCurrentActions->Count(); nAction++)
    {
        USHORT nRId = GetClickActionSdResId((presentation::ClickAction)(ULONG)pCurrentActions->
                                                GetObject(nAction));
        aLbAction.InsertEntry( String( SdResId( nRId ) ) );
    }


    // Effekte-Listbox fuellen
    for (ULONG nEffect = 0; nEffect < pCurrentEffects->Count(); nEffect++)
    {
        presentation::AnimationEffect eEffect = (presentation::AnimationEffect)(ULONG)pCurrentEffects->
                                            GetObject(nEffect);
        USHORT nRId = GetAnimationEffectSdResId(eEffect);
        aLbEffect.InsertEntry( String( SdResId( nRId ) ) );
    }


    if( bDisableAll )
    {
        aFtAction.Disable();
        aLbAction.Disable();
    }
    else
    {
        ChangeEffectHdl( this );
        ClickActionHdl( this );
    }
}

// -----------------------------------------------------------------------

BOOL SdTPAction::FillItemSet( SfxItemSet& rAttrs )
{
    BOOL bModified = FALSE;
    presentation::ClickAction eCA = presentation::ClickAction_NONE;

    if( aLbAction.GetSelectEntryCount() )
        eCA = GetActualClickAction();

    if( aLbAction.GetSavedValue() != aLbAction.GetSelectEntryPos() )
    {
        rAttrs.Put( SfxAllEnumItem( ATTR_ACTION, eCA ) );
        bModified = TRUE;
    }
    else
        rAttrs.InvalidateItem( ATTR_ACTION );

    String aFileName = GetEditText( TRUE );
    if( aFileName.Len() == 0 )
        rAttrs.InvalidateItem( ATTR_ACTION_FILENAME );
    else
    {
        if( eCA == presentation::ClickAction_SOUND ||
            eCA == presentation::ClickAction_VANISH ||
            eCA == presentation::ClickAction_DOCUMENT ||
            eCA == presentation::ClickAction_PROGRAM )
            aFileName = ::URIHelper::SmartRelToAbs( aFileName, FALSE,
                                                    INetURLObject::WAS_ENCODED,
                                                    INetURLObject::DECODE_UNAMBIGUOUS );

        rAttrs.Put( SfxStringItem( ATTR_ACTION_FILENAME, aFileName ) );
        bModified = TRUE;
    }

    if( eCA == presentation::ClickAction_VANISH )
    {
        // Effekt
        if( aLbEffect.GetSelectEntryCount() &&
            aLbEffect.GetSavedValue() != aLbEffect.GetSelectEntryPos() )
        {
            rAttrs.Put( SfxAllEnumItem( ATTR_ACTION_EFFECT, GetActualAnimationEffect() ) );
            bModified = TRUE;
        }
        else
            rAttrs.InvalidateItem( ATTR_ACTION_EFFECT );

        // Speed
        if( aRbtSlow.IsChecked() && aRbtSlow.GetSavedValue() != TRUE )
        {
            rAttrs.Put( SfxAllEnumItem( ATTR_ACTION_EFFECTSPEED, (presentation::AnimationSpeed) presentation::AnimationSpeed_SLOW ) );
            bModified = TRUE;
        }
        else if( aRbtMedium.IsChecked() && aRbtMedium.GetSavedValue() != TRUE )
        {
            rAttrs.Put( SfxAllEnumItem( ATTR_ACTION_EFFECTSPEED, (presentation::AnimationSpeed) presentation::AnimationSpeed_MEDIUM ) );
            bModified = TRUE;
        }
        else if( aRbtFast.IsChecked() && aRbtFast.GetSavedValue() != TRUE )
        {
            rAttrs.Put( SfxAllEnumItem( ATTR_ACTION_EFFECTSPEED, (presentation::AnimationSpeed) presentation::AnimationSpeed_FAST ) );
            bModified = TRUE;
        }
        else
            rAttrs.InvalidateItem( ATTR_ACTION_EFFECTSPEED );

        // Sound On
        TriState eState = aTsbSound.GetState();
        if( eState != aTsbSound.GetSavedValue() )
        {
            if( eState == STATE_DONTKNOW )
                rAttrs.InvalidateItem( ATTR_ACTION_SOUNDON );
            else if( eState == STATE_CHECK )
            {
                rAttrs.Put( SfxBoolItem( ATTR_ACTION_SOUNDON, TRUE ) );
                bModified = TRUE;
            }
            else
            {
                rAttrs.Put( SfxBoolItem( ATTR_ACTION_SOUNDON, FALSE ) );
                bModified = TRUE;
            }
        }
        // vollstaendig abspielen
        eState = aTsbPlayFull.GetState();
        if( eState != aTsbPlayFull.GetSavedValue() )
        {
            if( eState == STATE_DONTKNOW )
                rAttrs.InvalidateItem( ATTR_ACTION_PLAYFULL );
            else if( eState == STATE_CHECK )
            {
                rAttrs.Put( SfxBoolItem( ATTR_ACTION_PLAYFULL, TRUE ) );
                bModified = TRUE;
            }
            else
            {
                rAttrs.Put( SfxBoolItem( ATTR_ACTION_PLAYFULL, FALSE ) );
                bModified = TRUE;
            }
        }
    }

    return( bModified );
}

//------------------------------------------------------------------------

void SdTPAction::Reset( const SfxItemSet& rAttrs )
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    String      aFileName;

    // aLbAction
    if( rAttrs.GetItemState( ATTR_ACTION ) != SFX_ITEM_DONTCARE )
    {
        eCA = (presentation::ClickAction) ( ( const SfxAllEnumItem& ) rAttrs.
                    Get( ATTR_ACTION ) ).GetValue();
        SetActualClickAction( eCA );
    }
    else
        aLbAction.SetNoSelection();

    // aEdtSound
    if( rAttrs.GetItemState( ATTR_ACTION_FILENAME ) != SFX_ITEM_DONTCARE )
    {
        aFileName = ( ( const SfxStringItem& ) rAttrs.
                    Get( ATTR_ACTION_FILENAME ) ).GetValue();
        SetEditText( aFileName );
    }

    // Effekt
    if( rAttrs.GetItemState( ATTR_ACTION_EFFECT ) != SFX_ITEM_DONTCARE )
    {
        presentation::AnimationEffect eAE = (presentation::AnimationEffect) ( ( const SfxAllEnumItem& ) rAttrs.
                    Get( ATTR_ACTION_EFFECT ) ).GetValue();
        SetActualAnimationEffect( eAE );
    }
    else
        aLbEffect.SetNoSelection();

    // Speed
    if( rAttrs.GetItemState( ATTR_ACTION_EFFECTSPEED ) != SFX_ITEM_DONTCARE )
    {
        presentation::AnimationSpeed eAS = (presentation::AnimationSpeed) ( ( const SfxAllEnumItem& ) rAttrs.
                    Get( ATTR_ACTION_EFFECTSPEED ) ).GetValue();

        if( eAS == presentation::AnimationSpeed_SLOW )
            aRbtSlow.Check();
        else if( eAS == presentation::AnimationSpeed_MEDIUM )
            aRbtMedium.Check();
        else if( eAS == presentation::AnimationSpeed_FAST )
            aRbtFast.Check();
    }

    // Sound on
    if( rAttrs.GetItemState( ATTR_ACTION_SOUNDON ) != SFX_ITEM_DONTCARE )
    {
        aTsbSound.EnableTriState( FALSE );
        if( ( ( const SfxBoolItem& ) rAttrs.Get( ATTR_ACTION_SOUNDON ) ).GetValue() )
            aTsbSound.SetState( STATE_CHECK );
        else
            aTsbSound.SetState( STATE_NOCHECK );
    }
    else
        aTsbSound.SetState( STATE_DONTKNOW );

    // vollstaendig abspielen
    if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( ATTR_ACTION_PLAYFULL ) )
    {
        aTsbPlayFull.EnableTriState( FALSE );
        if( ( ( const SfxBoolItem& ) rAttrs.Get( ATTR_ACTION_PLAYFULL ) ).GetValue() )
            aTsbPlayFull.SetState( STATE_CHECK );
        else
            aTsbPlayFull.SetState( STATE_NOCHECK );
    }
    else
        aTsbPlayFull.SetState( STATE_DONTKNOW );

    switch( eCA )
    {
        case presentation::ClickAction_VANISH:
        {
            ChangeEffectHdl( this );
        }
        break;

        case presentation::ClickAction_BOOKMARK:
        {
            if( !aLbTree.SelectEntry( aFileName ) )
                aLbTree.SelectAll( FALSE );
        }
        break;

        case presentation::ClickAction_DOCUMENT:
        {
            if( aFileName.GetTokenCount( DOCUMENT_TOKEN ) == 2 )
                aLbTreeDocument.SelectEntry( aFileName.GetToken( 1, DOCUMENT_TOKEN ) );
        }
        break;

        default:
        break;
    }
    ClickActionHdl( this );

    aLbAction.SaveValue();
    aLbEffect.SaveValue();
    aRbtSlow.SaveValue();
    aRbtMedium.SaveValue();
    aRbtFast.SaveValue();
    aEdtSound.SaveValue();
    aTsbSound.SaveValue();
    aTsbPlayFull.SaveValue();
}

// -----------------------------------------------------------------------

void SdTPAction::ActivatePage( const SfxItemSet& rSet )
{
}

// -----------------------------------------------------------------------

int SdTPAction::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
        FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

SfxTabPage* SdTPAction::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTPAction( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SdTPAction::GetRanges()
{
    return( pActionRanges );
}

//------------------------------------------------------------------------

void SdTPAction::UpdateTree()
{
    if( !bTreeUpdated )
    {
        //aLbTree.Clear();
        aLbTree.Fill( pDoc, TRUE, pDoc->GetDocSh()->GetMedium()->GetName() );
        bTreeUpdated = TRUE;
    }
}

//------------------------------------------------------------------------

void SdTPAction::OpenFileDialog()
{
    // Soundpreview nur fuer Interaktionen mit Sound
    presentation::ClickAction eCA = GetActualClickAction();
    BOOL bSound = ( eCA == presentation::ClickAction_SOUND ||
                    eCA == presentation::ClickAction_VANISH );
    BOOL bPage = ( eCA == presentation::ClickAction_BOOKMARK );
    BOOL bDocument = ( eCA == presentation::ClickAction_DOCUMENT ||
                       eCA == presentation::ClickAction_PROGRAM );
    BOOL bMacro = ( eCA == presentation::ClickAction_MACRO );
    //BOOL bObject = ( eCA == presentation::ClickAction_VERB );

    if( bPage )
    {
        // Es wird in der TreeLB nach dem eingegebenen Objekt gesucht
        aLbTree.SelectEntry( GetEditText() );
    }
    else
    {
        String aFile( GetEditText() );

        if( bSound )
        {
            SdOpenSoundFileDialog   aFileDialog;

            if( !aFile.Len() )
                aFile = SvtPathOptions().GetGraphicPath();

            aFileDialog.SetPath( aFile );

            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
        }
        else if (bMacro)
        {
            Window* pOldWin = Application::GetDefDialogParent();
            Application::SetDefDialogParent( this );

            // choose macro dialog
            ::rtl::OUString aScriptURL = SfxApplication::ChooseScript();

            if ( aScriptURL.getLength() != 0 )
            {
                SetEditText( aScriptURL );
            }

            Application::SetDefDialogParent( pOldWin );
        }
        else
        {
            sfx2::FileDialogHelper aFileDialog(WB_OPEN | WB_3DLOOK | WB_STDMODAL );

            if (bDocument && !aFile.Len())
                aFile = SvtPathOptions().GetWorkPath();

            aFileDialog.SetDisplayDirectory( aFile );

            // The following is a fix for #1008001# and a workarround for
            // #i4306#: The addition of the implicitely existing "all files"
            // filter makes the (Windows system) open file dialog follow
            // links on the desktop to directories.
            aFileDialog.AddFilter (
                String (SdResId (STR_FILTERNAME_ALL)),
                String (RTL_CONSTASCII_USTRINGPARAM("*.*")));


            if( aFileDialog.Execute() == ERRCODE_NONE )
            {
                aFile = aFileDialog.GetPath();
                SetEditText( aFile );
            }
            if( bDocument )
                CheckFileHdl( NULL );
        }
    }
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, ChangeEffectHdl, void *, EMPTYARG )
{
    USHORT nPos = GetActualAnimationEffect();
    if( nPos == presentation::AnimationEffect_NONE &&
        aLbEffect.GetSelectEntryCount() > 0 )
    {
        aRbtSlow.Disable();
        aRbtMedium.Disable();
        aRbtFast.Disable();
    }
    else
    {
        aRbtSlow.Enable();
        aRbtMedium.Enable();
        aRbtFast.Enable();

        // Damit wenigstens ein Radiobutton gechecked wird:
        if( !aRbtSlow.IsChecked() &&
            !aRbtMedium.IsChecked() &&
            !aRbtFast.IsChecked() )
        {
            aRbtSlow.Check();
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, ClickSoundHdl, void *, EMPTYARG )
{
    if( aTsbSound.GetState() == STATE_NOCHECK)
    {
        aEdtSound.Disable();
        aBtnSearch.Disable();
        aTsbPlayFull.Disable();
    }
    else
    {
        aEdtSound.Enable();
        aBtnSearch.Enable();
        aTsbPlayFull.Enable();
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, ClickSearchHdl, void *, EMPTYARG )
{
    OpenFileDialog();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, ClickActionHdl, void *, EMPTYARG )
{
    presentation::ClickAction eCA = GetActualClickAction();

    // hide controls we don't need
    switch( eCA )
    {
        case presentation::ClickAction_NONE:
        case presentation::ClickAction_INVISIBLE:
        case presentation::ClickAction_PREVPAGE:
        case presentation::ClickAction_NEXTPAGE:
        case presentation::ClickAction_FIRSTPAGE:
        case presentation::ClickAction_LASTPAGE:
        case presentation::ClickAction_STOPPRESENTATION:
            aFtTree.Hide();
            aLbTree.Hide();
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aFlEffect.Hide();
            aFtAnimation.Hide();
            aLbEffect.Hide();
            aFtSpeed.Hide();
            aRbtSlow.Hide();
            aRbtMedium.Hide();
            aRbtFast.Hide();
            aTsbPlayFull.Hide();
            aTsbSound.Hide();

            aFlSeparator.Hide();
            aEdtSound.Hide();
            aEdtBookmark.Hide();
            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aBtnSearch.Hide();
            aBtnSeek.Hide();
            break;

        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            aFtTree.Hide();
            aLbTree.Hide();
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aFlEffect.Hide();
            aFtAnimation.Hide();
            aLbEffect.Hide();
            aFtSpeed.Hide();
            aRbtSlow.Hide();
            aRbtMedium.Hide();
            aRbtFast.Hide();
            aTsbPlayFull.Hide();
            aTsbSound.Hide();

            aEdtDocument.Hide();

            if( eCA == presentation::ClickAction_MACRO )
            {
                aEdtSound.Hide();
                aEdtProgram.Hide();
            }
            else if( eCA == presentation::ClickAction_PROGRAM )
            {
                aEdtSound.Hide();
                aEdtMacro.Hide();
            }
            else if( eCA == presentation::ClickAction_SOUND )
            {
                aEdtProgram.Hide();
                aEdtMacro.Hide();
            }

            aBtnSeek.Hide();
            break;


        case presentation::ClickAction_DOCUMENT:
            aLbTree.Hide();
            aLbOLEAction.Hide();

            aFlEffect.Hide();
            aFtAnimation.Hide();
            aLbEffect.Hide();
            aFtSpeed.Hide();
            aRbtSlow.Hide();
            aRbtMedium.Hide();
            aRbtFast.Hide();
            aTsbPlayFull.Hide();
            aEdtSound.Hide();
            aTsbSound.Hide();

            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aEdtBookmark.Hide();
            aBtnSeek.Hide();
            break;

        case presentation::ClickAction_VERB:
            aLbTree.Hide();
            aLbTreeDocument.Hide();

            aFlEffect.Hide();
            aFtAnimation.Hide();
            aLbEffect.Hide();
            aFtSpeed.Hide();
            aRbtSlow.Hide();
            aRbtMedium.Hide();
            aRbtFast.Hide();
            aTsbPlayFull.Hide();
            aEdtSound.Hide();
            aTsbSound.Hide();

            aFlSeparator.Hide();
            aEdtBookmark.Hide();
            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aBtnSearch.Hide();
            aBtnSeek.Hide();
            break;

        case presentation::ClickAction_BOOKMARK:
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aFlEffect.Hide();
            aFtAnimation.Hide();
            aLbEffect.Hide();
            aFtSpeed.Hide();
            aRbtSlow.Hide();
            aRbtMedium.Hide();
            aRbtFast.Hide();
            aTsbPlayFull.Hide();
            aEdtSound.Hide();
            aTsbSound.Hide();

            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aBtnSearch.Hide();
            break;

        case presentation::ClickAction_VANISH:
            aFtTree.Hide();
            aLbTree.Hide();
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aFlSeparator.Hide();
            aEdtBookmark.Hide();
            aEdtDocument.Hide();
            aEdtProgram.Hide();
            aEdtMacro.Hide();
            aBtnSeek.Hide();
            break;
    }

    // show controls we do need
    switch( eCA )
    {
        case presentation::ClickAction_NONE:
        case presentation::ClickAction_INVISIBLE:
        case presentation::ClickAction_PREVPAGE:
        case presentation::ClickAction_NEXTPAGE:
        case presentation::ClickAction_FIRSTPAGE:
        case presentation::ClickAction_LASTPAGE:
        case presentation::ClickAction_STOPPRESENTATION:
            // none
            break;

        case presentation::ClickAction_SOUND:
            aFlSeparator.Show();
            aEdtSound.Show();
            aEdtSound.Enable();
            aBtnSearch.Show();
            aBtnSearch.Enable();
            aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_SOUND ) ) );
            break;

        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            aFlSeparator.Show();
            aBtnSearch.Show();
            aBtnSearch.Enable();
            if( eCA == presentation::ClickAction_MACRO )
            {
                aEdtMacro.Show();
                aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_MACRO ) ) );
            }
            else
            {
                aEdtProgram.Show();
                aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_PROGRAM ) ) );
            }
            break;

        case presentation::ClickAction_DOCUMENT:
            aFtTree.Show();
            aLbTreeDocument.Show();

            aFlSeparator.Show();
            aEdtDocument.Show();
            aBtnSearch.Show();
            aBtnSearch.Enable();

            aFtTree.SetText( String( SdResId( STR_EFFECTDLG_JUMP ) ) );
            aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_DOCUMENT ) ) );

            CheckFileHdl( NULL );
            break;

        case presentation::ClickAction_VERB:
            aFtTree.Show();
            aLbOLEAction.Show();

            aFtTree.SetText( String( SdResId( STR_EFFECTDLG_ACTION ) ) );
            break;

        case presentation::ClickAction_BOOKMARK:
            UpdateTree();

            aFtTree.Show();
            aLbTree.Show();

            aFlSeparator.Show();
            aEdtBookmark.Show();
            aBtnSeek.Show();

            aFtTree.SetText( String( SdResId( STR_EFFECTDLG_JUMP ) ) );
            aFlSeparator.SetText( String( SdResId( STR_EFFECTDLG_PAGE_OBJECT ) ) );
            break;

        case presentation::ClickAction_VANISH:
            aFlEffect.Show();
            aFtAnimation.Show();
            aLbEffect.Show();
            aFtSpeed.Show();
            aRbtSlow.Show();
            aRbtMedium.Show();
            aRbtFast.Show();
            aTsbPlayFull.Show();
            aEdtSound.Show();
            aTsbSound.Show();

            aBtnSearch.Show();

            ClickSoundHdl( NULL );
            ChangeEffectHdl( NULL );
            break;
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, SelectTreeHdl, void *, EMPTYARG )
{
    aEdtBookmark.SetText( aLbTree.GetSelectEntry() );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, CheckFileHdl, void *, EMPTYARG )
{
    String aFile( GetEditText() );

    if( aFile != aLastFile )
    {
        // Ueberpruefen, ob es eine gueltige Draw-Datei ist
        SfxMedium aMedium( aFile,
                    STREAM_READ | STREAM_NOCREATE,
                    TRUE );               // Download

        if( aMedium.IsStorage() )
        {
            WaitObject aWait( GetParent()->GetParent() );

            // ist es eine Draw-Datei?
            // mit READ oeffnen, sonst schreiben die Storages evtl. in die Datei!

            SvStorage* pStorage = aMedium.GetStorage();
            DBG_ASSERT( pStorage, "Kein Storage!" );

            if( pStorage->IsStream( pStarDrawDoc ) ||
                pStorage->IsStream( pStarDrawDoc3 ) ||
                pStorage->IsStream( pStarDrawXMLContent ) ||
                pStorage->IsStream( pStarDrawOldXMLContent ) )
            {
                SdDrawDocument* pBookmarkDoc = pDoc->OpenBookmarkDoc( aFile );

                if( pBookmarkDoc )
                {
                    aLastFile = aFile;

                    aLbTreeDocument.Clear();
                    aLbTreeDocument.Fill( pBookmarkDoc, TRUE, aFile );
                    pDoc->CloseBookmarkDoc();
                    aLbTreeDocument.Show();
                }
                else
                    aLbTreeDocument.Hide();
            }
            else
                aLbTreeDocument.Hide();

        }
        else
            aLbTreeDocument.Hide();
    }

    return( 0L );
}

//------------------------------------------------------------------------

presentation::ClickAction SdTPAction::GetActualClickAction()
{
    presentation::ClickAction eCA = presentation::ClickAction_NONE;
    USHORT nPos = aLbAction.GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        eCA = (presentation::ClickAction)(ULONG)pCurrentActions->GetObject((ULONG)nPos);
    return( eCA );
}

//------------------------------------------------------------------------

void SdTPAction::SetActualClickAction( presentation::ClickAction eCA )
{
    USHORT nPos = (USHORT)pCurrentActions->GetPos((void*)(ULONG)eCA);
    DBG_ASSERT(nPos != LIST_ENTRY_NOTFOUND, "unbekannte Interaktion");
    aLbAction.SelectEntryPos(nPos);
}

//------------------------------------------------------------------------

presentation::AnimationEffect SdTPAction::GetActualAnimationEffect()
{
    presentation::AnimationEffect eAE = presentation::AnimationEffect_NONE;

    USHORT nPos = aLbEffect.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        eAE = (presentation::AnimationEffect)(ULONG)pCurrentEffects->GetObject((ULONG)nPos);
    return( eAE );
}

//------------------------------------------------------------------------

void SdTPAction::SetActualAnimationEffect( presentation::AnimationEffect eAE )
{
    USHORT nPos = (USHORT)pCurrentEffects->GetPos((void*)(ULONG)eAE);
    DBG_ASSERT(nPos != LIST_ENTRY_NOTFOUND, "unbekannter Effekt");
    aLbEffect.SelectEntryPos(nPos);
}

//------------------------------------------------------------------------

void SdTPAction::SetEditText( String const & rStr )
{
    presentation::ClickAction   eCA = GetActualClickAction();
    String                      aText(rStr);

    // possibly convert URI back to system path
    switch( eCA )
    {
        case presentation::ClickAction_DOCUMENT:
            if( rStr.GetTokenCount( DOCUMENT_TOKEN ) == 2 )
                aText = rStr.GetToken( 0, DOCUMENT_TOKEN );

            // fallthrough intended
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_VANISH:
        case presentation::ClickAction_PROGRAM:
            INetURLObject aURL( aText );

            // try to convert to system path
            String aTmpStr(aURL.getFSysPath(INetURLObject::FSYS_DETECT));

            if( aTmpStr.Len() )
                aText = aTmpStr;    // was a system path
            break;
    }

    // set the string to the corresponding control
    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_VANISH:
            aEdtSound.SetText( aText );
            break;
        case presentation::ClickAction_VERB:
        {
            ::std::vector< long >::iterator aFound( ::std::find( aVerbVector.begin(), aVerbVector.end(), rStr.ToInt32() ) );

            if( aFound != aVerbVector.end() )
                aLbOLEAction.SelectEntryPos( static_cast< short >( aFound - aVerbVector.begin() ) );
        }
        break;
        case presentation::ClickAction_PROGRAM:
            aEdtProgram.SetText( aText );
            break;
        case presentation::ClickAction_MACRO:
        {
            aEdtMacro.SetText( aText );
        }
            break;
        case presentation::ClickAction_DOCUMENT:
            aEdtDocument.SetText( aText );
            break;
        case presentation::ClickAction_BOOKMARK:
            aEdtBookmark.SetText( aText );
            break;
    }
}

String SdTPAction::GetMacroName( const String& rMacroPath )
{
    String result = rMacroPath;

    // try to get name by parsing the macro path
    // using the new URI parsing services

    Reference< XMultiServiceFactory > xSMgr =
        ::comphelper::getProcessServiceFactory();

    Reference< com::sun::star::uri::XUriReferenceFactory >
        xFactory( xSMgr->createInstance(
            ::rtl::OUString::createFromAscii(
                "com.sun.star.uri.UriReferenceFactory" ) ), UNO_QUERY );

    if ( xFactory.is() )
    {
        Reference< com::sun::star::uri::XVndSunStarScriptUrl >
            xUrl( xFactory->parse( rMacroPath ), UNO_QUERY );

        if ( xUrl.is() )
        {
            result = xUrl->getName();
        }
    }

    return result;
}

//------------------------------------------------------------------------

String SdTPAction::GetEditText( BOOL bFullDocDestination )
{
    String aStr;
    presentation::ClickAction eCA = GetActualClickAction();

    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_VANISH:
            aStr =  aEdtSound.GetText();
            break;

        case presentation::ClickAction_VERB:
        {
            const USHORT nPos = aLbOLEAction.GetSelectEntryPos();

            if( nPos < aVerbVector.size() )
                aStr = UniString::CreateFromInt32( aVerbVector[ nPos ] );

            return( aStr );
        }

        case presentation::ClickAction_DOCUMENT:
            aStr = aEdtDocument.GetText();
            break;

        case presentation::ClickAction_PROGRAM:
            aStr = aEdtProgram.GetText();
            break;

        case presentation::ClickAction_MACRO:
        {
            return aEdtMacro.GetText();
        }
        break;

        case presentation::ClickAction_BOOKMARK:
            return( aEdtBookmark.GetText() );
    }

    // validate file URI
    INetURLObject aURL( aStr );

    if( aStr.Len() && aURL.GetProtocol() == INET_PROT_NOT_VALID )
        aURL = INetURLObject( ::URIHelper::SmartRelToAbs( aStr ) );

    // get adjusted file name
    aStr = aURL.GetMainURL( INetURLObject::NO_DECODE );

    if( bFullDocDestination &&
        eCA == presentation::ClickAction_DOCUMENT &&
        aLbTreeDocument.Control::IsVisible() &&
        aLbTreeDocument.GetSelectionCount() > 0 )
    {
        String aTmpStr( aLbTreeDocument.GetSelectEntry() );
        if( aTmpStr.Len() )
        {
            aStr.Append( DOCUMENT_TOKEN );
            aStr.Append( aTmpStr );
        }
    }

    return( aStr );
}

//------------------------------------------------------------------------

USHORT SdTPAction::GetClickActionSdResId( presentation::ClickAction eCA )
{
    switch( eCA )
    {
        case presentation::ClickAction_NONE:             return STR_CLICK_ACTION_NONE;
        case presentation::ClickAction_PREVPAGE:         return STR_CLICK_ACTION_PREVPAGE;
        case presentation::ClickAction_NEXTPAGE:         return STR_CLICK_ACTION_NEXTPAGE;
        case presentation::ClickAction_FIRSTPAGE:        return STR_CLICK_ACTION_FIRSTPAGE;
        case presentation::ClickAction_LASTPAGE:         return STR_CLICK_ACTION_LASTPAGE;
        case presentation::ClickAction_BOOKMARK:         return STR_CLICK_ACTION_BOOKMARK;
        case presentation::ClickAction_DOCUMENT:         return STR_CLICK_ACTION_DOCUMENT;
        case presentation::ClickAction_PROGRAM:          return STR_CLICK_ACTION_PROGRAM;
        case presentation::ClickAction_MACRO:            return STR_CLICK_ACTION_MACRO;
        case presentation::ClickAction_INVISIBLE:        return STR_CLICK_ACTION_INVISIBLE;
        case presentation::ClickAction_SOUND:            return STR_CLICK_ACTION_SOUND;
        case presentation::ClickAction_VERB:             return STR_CLICK_ACTION_VERB;
        case presentation::ClickAction_VANISH:           return STR_CLICK_ACTION_VANISH;
        case presentation::ClickAction_STOPPRESENTATION: return STR_CLICK_ACTION_STOPPRESENTATION;
        default: DBG_ERROR( "Keine StringResource fuer ClickAction vorhanden!" );
    }
    return( 0 );
}

//------------------------------------------------------------------------

USHORT SdTPAction::GetAnimationEffectSdResId( presentation::AnimationEffect eAE )
{
    switch( eAE )
    {
        case presentation::AnimationEffect_NONE:                    return STR_EFFECT_NONE;
        case presentation::AnimationEffect_FADE_FROM_LEFT:          return STR_EFFECT_FADE_FROM_L;
        case presentation::AnimationEffect_FADE_FROM_UPPERLEFT:     return STR_EFFECT_FADE_FROM_UL;
        case presentation::AnimationEffect_FADE_FROM_TOP:           return STR_EFFECT_FADE_FROM_T;
        case presentation::AnimationEffect_FADE_FROM_UPPERRIGHT:    return STR_EFFECT_FADE_FROM_UR;
        case presentation::AnimationEffect_FADE_FROM_RIGHT:         return STR_EFFECT_FADE_FROM_R;
        case presentation::AnimationEffect_FADE_FROM_LOWERRIGHT:    return STR_EFFECT_FADE_FROM_LR;
        case presentation::AnimationEffect_FADE_FROM_BOTTOM:        return STR_EFFECT_FADE_FROM_B;
        case presentation::AnimationEffect_FADE_FROM_LOWERLEFT:     return STR_EFFECT_FADE_FROM_LL;
        case presentation::AnimationEffect_MOVE_FROM_LEFT:          return 0;
        case presentation::AnimationEffect_MOVE_FROM_UPPERLEFT:     return 0;
        case presentation::AnimationEffect_MOVE_FROM_TOP:           return 0;
        case presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT:    return 0;
        case presentation::AnimationEffect_MOVE_FROM_RIGHT:         return 0;
        case presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT:    return 0;
        case presentation::AnimationEffect_MOVE_FROM_BOTTOM:        return 0;
        case presentation::AnimationEffect_MOVE_FROM_LOWERLEFT:     return 0;
        case presentation::AnimationEffect_FADE_TO_CENTER:          return STR_EFFECT_FADE_TO_CENTER;
        case presentation::AnimationEffect_FADE_FROM_CENTER:        return STR_EFFECT_FADE_FROM_CENTER;
        case presentation::AnimationEffect_VERTICAL_STRIPES:        return STR_EFFECT_VERTICAL_STRIPES;
        case presentation::AnimationEffect_HORIZONTAL_STRIPES:      return STR_EFFECT_HORIZONTAL_STRIPES;
        case presentation::AnimationEffect_VERTICAL_LINES:          return STR_EFFECT_VERTICAL_LINES;
        case presentation::AnimationEffect_HORIZONTAL_LINES:        return STR_EFFECT_HORIZONTAL_LINES;
        case presentation::AnimationEffect_VERTICAL_CHECKERBOARD:   return STR_EFFECT_VERTICAL_CHECKERBOARD;
        case presentation::AnimationEffect_HORIZONTAL_CHECKERBOARD: return STR_EFFECT_HORIZONTAL_CHECKERBOARD;
        case presentation::AnimationEffect_CLOCKWISE:               return STR_EFFECT_CLOCKWISE;
        case presentation::AnimationEffect_COUNTERCLOCKWISE:        return STR_EFFECT_COUNTERCLOCKWISE;
        case presentation::AnimationEffect_CLOSE_VERTICAL:          return STR_EFFECT_CLOSE_VERTICAL;
        case presentation::AnimationEffect_CLOSE_HORIZONTAL:        return STR_EFFECT_CLOSE_HORIZONTAL;
        case presentation::AnimationEffect_OPEN_VERTICAL:           return STR_EFFECT_OPEN_VERTICAL;
        case presentation::AnimationEffect_OPEN_HORIZONTAL:         return STR_EFFECT_OPEN_HORIZONTAL;
        case presentation::AnimationEffect_PATH:                    return 0;
        case presentation::AnimationEffect_MOVE_TO_LEFT:            return STR_EFFECT_MOVE_TO_L;
        case presentation::AnimationEffect_MOVE_TO_UPPERLEFT:       return STR_EFFECT_MOVE_TO_UL;
        case presentation::AnimationEffect_MOVE_TO_TOP:             return STR_EFFECT_MOVE_TO_T;
        case presentation::AnimationEffect_MOVE_TO_UPPERRIGHT:      return STR_EFFECT_MOVE_TO_UR;
        case presentation::AnimationEffect_MOVE_TO_RIGHT:           return STR_EFFECT_MOVE_TO_R;
        case presentation::AnimationEffect_MOVE_TO_LOWERRIGHT:      return STR_EFFECT_MOVE_TO_LR;
        case presentation::AnimationEffect_MOVE_TO_BOTTOM:          return STR_EFFECT_MOVE_TO_B;
        case presentation::AnimationEffect_MOVE_TO_LOWERLEFT:       return STR_EFFECT_MOVE_TO_LL;
        case presentation::AnimationEffect_MOVE_SHORT_TO_LEFT:      return STR_EFFECT_MOVE_S_TO_L;
        case presentation::AnimationEffect_MOVE_SHORT_TO_UPPERLEFT: return STR_EFFECT_MOVE_S_TO_UL;
        case presentation::AnimationEffect_MOVE_SHORT_TO_TOP:       return STR_EFFECT_MOVE_S_TO_T;
        case presentation::AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT:return STR_EFFECT_MOVE_S_TO_UR;
        case presentation::AnimationEffect_MOVE_SHORT_TO_RIGHT:     return STR_EFFECT_MOVE_S_TO_R;
        case presentation::AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT:return STR_EFFECT_MOVE_S_TO_LR;
        case presentation::AnimationEffect_MOVE_SHORT_TO_BOTTOM:    return STR_EFFECT_MOVE_S_TO_B;
        case presentation::AnimationEffect_MOVE_SHORT_TO_LOWERLEFT: return STR_EFFECT_MOVE_S_TO_LL;
        case presentation::AnimationEffect_SPIRALIN_LEFT:           return STR_EFFECT_SPIRALIN_L;
        case presentation::AnimationEffect_SPIRALIN_RIGHT:          return STR_EFFECT_SPIRALIN_R;
        case presentation::AnimationEffect_SPIRALOUT_LEFT:          return STR_EFFECT_SPIRALOUT_L;
        case presentation::AnimationEffect_SPIRALOUT_RIGHT:         return STR_EFFECT_SPIRALOUT_R;
        case presentation::AnimationEffect_DISSOLVE:                return STR_EFFECT_DISSOLVE;
        case presentation::AnimationEffect_WAVYLINE_FROM_LEFT:      return STR_EFFECT_WAVYLINE_FROM_L;
        case presentation::AnimationEffect_WAVYLINE_FROM_RIGHT:     return STR_EFFECT_WAVYLINE_FROM_R;
        case presentation::AnimationEffect_WAVYLINE_FROM_TOP:       return STR_EFFECT_WAVYLINE_FROM_T;
        case presentation::AnimationEffect_WAVYLINE_FROM_BOTTOM:    return STR_EFFECT_WAVYLINE_FROM_B;
        case presentation::AnimationEffect_RANDOM:                  return STR_EFFECT_RANDOM;
        case presentation::AnimationEffect_LASER_FROM_LEFT:         return 0;
        case presentation::AnimationEffect_LASER_FROM_TOP:          return 0;
        case presentation::AnimationEffect_LASER_FROM_RIGHT:        return 0;
        case presentation::AnimationEffect_LASER_FROM_BOTTOM:       return 0;
        case presentation::AnimationEffect_LASER_FROM_UPPERLEFT:    return 0;
        case presentation::AnimationEffect_LASER_FROM_UPPERRIGHT:   return 0;
        case presentation::AnimationEffect_LASER_FROM_LOWERLEFT:    return 0;
        case presentation::AnimationEffect_LASER_FROM_LOWERRIGHT:   return 0;

        default:
            DBG_ERROR( "Keine StringResource fuer AnimationEffect vorhanden!" );
    }
    return( 0 );
}


