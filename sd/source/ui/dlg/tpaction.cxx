/*************************************************************************
 *
 *  $RCSfile: tpaction.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-01 17:26:33 $
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

#include "sdattr.hxx"

#define ITEMID_COLOR            ATTR_ANIMATION_COLOR
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#include <sfx2/app.hxx>
#include <tools/urlobj.hxx>
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SFXFILEDLG_HXX //autogen
#include <sfx2/iodlg.hxx>
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
#ifndef _BASOBJ_HXX //autogen
#include <basctl/basobj.hxx>
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#include "tpeffect.hxx"
#include "strmname.h"
#include "sdview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "strings.hrc"
#include "res_bmp.hrc"

using namespace ::com::sun::star;

static USHORT pActionRanges[] =
{
    ATTR_ANIMATION_TRANSPCOLOR,
    ATTR_ANIMATION_TRANSPCOLOR,
    ATTR_ACTION_START,
    ATTR_ACTION_END,
    0
};


#define DOCUMENT_TOKEN (sal_Unicode('#'))


/*************************************************************************
|*
|*  Action-TabPage
|*
\************************************************************************/

SdTPAction::SdTPAction( Window* pWindow, const SfxItemSet& rInAttrs ) :
        SdTPAnimation   ( pWindow, rInAttrs ),
        bTreeUpdated    ( FALSE )
//      aLastFile       ( "XTendedEddy" ) I think nobody need this !!
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    pCurrentEffects = new List;
    pCurrentActions = new List;

    aLbAction.SetSelectHdl( LINK( this, SdTPAction, ClickActionHdl ) );
    aLbTree.SetSelectHdl( LINK( this, SdTPAction, SelectTreeHdl ) );
    aEdtDocument.SetLoseFocusHdl( LINK( this, SdTPAction, CheckFileHdl ) );

    // Controls enablen
    aFtAction.Show();
    aLbAction.Show();
    //aTsbSound.Show();
    aEdtPage.Show();
    aBtnSearch.Show();
    aGrpSound.Show();

}

// -----------------------------------------------------------------------

SdTPAction::~SdTPAction()
{
    delete pCurrentEffects;
    delete pCurrentActions;
}

// -----------------------------------------------------------------------

void SdTPAction::Construct()
{
    // OLE-Actionlistbox auffuellen
    SdrOle2Obj* pOleObj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    BOOL        bDisableAll = FALSE;
    BOOL        bOLEAction = FALSE;

    if ( pView->HasMarkedObj() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkList();
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
        aLbOLEAction.InsertEntry( String( SdResId( STR_EDIT_OBJ ) ).EraseAllChars('~') );
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
                    String aTmp = rVerb.GetName();
                    aLbOLEAction.InsertEntry( aTmp.EraseAllChars('~') );
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
    if (bOLEAction)
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

BOOL __EXPORT SdTPAction::FillItemSet( SfxItemSet& rAttrs )
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
        if( //eCA == presentation::ClickAction_SOUND ||
            //eCA == presentation::ClickAction_VANISH ||
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
/* Sollte schon bearbeitet worden sein
        // Sound (FileName)
        String aTmpStr( aEdtSound.GetText() );
        if( aTmpStr == aEdtSound.GetSavedValue() )
            rAttrs.InvalidateItem( ATTR_ACTION_FILENAME );
        else
        {
            aTmpStr = INetURLObject::SmartRelToAbs( aTmpStr, FALSE,
                                                    INetURLObject::WAS_ENCODED,
                                                    INetURLObject::DECODE_UNAMBIGUOUS );
            rAttrs.Put( SfxStringItem( ATTR_ACTION_FILENAME, aTmpStr ) );
            bModified = TRUE;
        }
*/
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

        // Transparenzfarbe
        if( aLbTranspColor.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND &&
            aLbTranspColor.GetSavedValue() != aLbTranspColor.GetSelectEntryPos() )
        {
            SvxColorItem aColorItem( aLbTranspColor.GetSelectEntryColor(),
                                     ATTR_ANIMATION_TRANSPCOLOR );
            rAttrs.Put( aColorItem );
        }

    }

    return( bModified );
}

//------------------------------------------------------------------------

void __EXPORT SdTPAction::Reset( const SfxItemSet& rAttrs )
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
        if( eCA == presentation::ClickAction_MACRO )
        {
            sal_Unicode cToken = '.';
            String aStr( aFileName.GetToken( 2, cToken ) );
            aStr.Append( cToken );
            aStr.Append( aFileName.GetToken( 1, cToken ) );
            aStr.Append( cToken );
            aStr.Append( aFileName.GetToken( 0, cToken ) );
            aFileName = aStr;
        }
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

    // Transparenzfarbe
    const SfxPoolItem* pPoolItem;
    if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( ATTR_ANIMATION_TRANSPCOLOR, FALSE, &pPoolItem ) )
    {
        Color aColor = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
        aLbTranspColor.SelectEntry( aColor );
    }

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
    aLbTranspColor.SaveValue();
    aTsbSound.SaveValue();
    aTsbPlayFull.SaveValue();
}

// -----------------------------------------------------------------------

void __EXPORT SdTPAction::ActivatePage( const SfxItemSet& rSet )
{
    const SvxColorItem* pColorItem;
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_ANIMATION_TRANSPCOLOR, FALSE,
                                    (const SfxPoolItem**) &pColorItem ) )
    {
        Color aColor = pColorItem->GetValue();
        aLbTranspColor.SelectEntry( aColor );
    }
}

// -----------------------------------------------------------------------

int __EXPORT SdTPAction::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet &&
        GetActualClickAction() == presentation::ClickAction_VANISH )
    {
        SvxColorItem aColorItem( aLbTranspColor.GetSelectEntryColor(),
                                    ATTR_ANIMATION_TRANSPCOLOR );
        pSet->Put( aColorItem );
    }

    if( pSet )
        FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT SdTPAction::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTPAction( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

USHORT* __EXPORT SdTPAction::GetRanges()
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
            pFileDialog = new SfxFileDialog ( this, WB_OPEN | WB_3DLOOK | WB_STDMODAL );
            pBtnPreview = new PushButton( pFileDialog, SdResId( RID_PREVIEW_BUTTON ) );
            pFileDialog->AddControl( pBtnPreview );
            pBtnPreview->SetClickHdl( LINK( this, SdTPAnimation, ClickPreviewHdl ) );
            pBtnPreview->Show();

#ifdef MAC
            String aDescr1;
            aDescr1 = String(SdResId(STR_SOUNDFILE1));
            pFileDialog->AddFilter (aDescr1, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.snd" ) ), "sfil0");

            String aDescr2;
            aDescr2 = String(SdResId(STR_SOUNDFILE2));
            pFileDialog->AddFilter (aDescr2, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.aif" ) ), "AIFF0");

            pFileDialog->SetDefaultExt("snd");
#else
            String aDescr;
            aDescr = String(SdResId(STR_WAV_FILE));
            pFileDialog->AddFilter (aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.wav" ) ) );
            aDescr = String(SdResId(STR_MIDI_FILE));
            pFileDialog->AddFilter (aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.mid" ) ) );

            pFileDialog->SetDefaultExt( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "wav" ) ) );
#endif

            if( !aFile.Len() )
                aFile = SvtPathOptions().GetGraphicPath();

            pFileDialog->SetPath( aFile );

            if ( pFileDialog->Execute() )
            {
                aFile = pFileDialog->GetPath();
                SetEditText( aFile );
            }

            if( aSound.IsPlaying() )
                aSound.Stop();

            delete pFileDialog;
            pBtnPreview = NULL;
        }
        else if (bMacro)
        {
            Window* pOldWin = Application::GetDefDialogParent();
            Application::SetDefDialogParent( this );

            SbMethod* pMethod = BasicIDE::ChooseMacro(FALSE, TRUE);

            if (pMethod)
            {
                SbModule* pModule = pMethod->GetModule();
                SbxObject* pObject = pModule->GetParent();
                DBG_ASSERT(pObject->IsA(TYPE(StarBASIC)), "Kein Basic gefunden!");

                // the format of "aMacro" has to be internally as following (because of file-format)
                // "Macroname.Modulname.Libname.Documentname" or
                // "Macroname.Modulname.Libname.Applicationsname"

                // But for the UI we need this format:
                // "Libname.Modulname.Macroname"
                String aMacro( pObject->GetName() );
                aMacro.Append( sal_Unicode('.') );
                aMacro.Append( pModule->GetName() );
                aMacro.Append( sal_Unicode('.') );
                aMacro.Append( pMethod->GetName() );

                SetEditText(aMacro);

            }
            Application::SetDefDialogParent( pOldWin );
        }
        else
        {
            SfxFileDialog* pSfxFileDlg = new SfxFileDialog ( this, WB_OPEN | WB_3DLOOK | WB_STDMODAL );

            if (bDocument && !aFile.Len())
                aFile = SvtPathOptions().GetWorkPath();

            pSfxFileDlg->SetPath( aFile );

            if( pSfxFileDlg->Execute() )
            {
                aFile = pSfxFileDlg->GetPath();
                SetEditText( aFile );
            }
            if( bDocument )
                CheckFileHdl( NULL );

            delete pSfxFileDlg;
        }
    }
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, ClickActionHdl, void *, EMPTYARG )
{
    presentation::ClickAction eCA = GetActualClickAction();

    switch( eCA )
    {
        case presentation::ClickAction_NONE:
        case presentation::ClickAction_INVISIBLE:
        case presentation::ClickAction_PREVPAGE:
        case presentation::ClickAction_NEXTPAGE:
        case presentation::ClickAction_FIRSTPAGE:
        case presentation::ClickAction_LASTPAGE:
        case presentation::ClickAction_VERB:
        case presentation::ClickAction_STOPPRESENTATION:
            aEdtSound.Disable();
            aEdtPage.Disable();
            aEdtDocument.Disable();
            aEdtObject.Disable();
            aBtnSearch.Disable();
            aBtnSeek.Disable();
            // kein break !
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
            aLbTreeDocument.Hide();
        case presentation::ClickAction_DOCUMENT:
            aLbTree.Hide();
            // kein break !
        case presentation::ClickAction_BOOKMARK:
            aLbOLEAction.Hide();
            aLbEffect.Hide();
            aFtTranspColor.Hide();
            aLbTranspColor.Hide();
            aRbtSlow.Hide();
            aRbtMedium.Hide();
            aRbtFast.Hide();
            aGrpEffect.Hide();
            aTsbPlayFull.Hide();
            aTsbSound.Hide();
            aGrpSound.Disable();

            if( eCA == presentation::ClickAction_SOUND )
            {
                aEdtPage.Hide();
                aEdtDocument.Hide();
                aEdtObject.Hide();
                aEdtSound.Enable();
                aEdtSound.Show();

                aBtnSeek.Hide();
                aBtnSearch.Enable();
                aBtnSearch.Show();

                aGrpSound.Enable();
                aGrpSound.SetText( String( SdResId( STR_EFFECTDLG_SOUND ) ) );
            }
            else if( eCA == presentation::ClickAction_DOCUMENT ||
                     eCA == presentation::ClickAction_MACRO ||
                     eCA == presentation::ClickAction_PROGRAM )
            {
                aEdtPage.Hide();
                aEdtObject.Hide();
                aEdtSound.Hide();
                aEdtDocument.Enable();
                aEdtDocument.Show();

                aBtnSeek.Hide();
                aBtnSearch.Enable();
                aBtnSearch.Show();

                aGrpSound.Enable();

                if (eCA == presentation::ClickAction_DOCUMENT)
                    aGrpSound.SetText( String( SdResId( STR_EFFECTDLG_DOCUMENT ) ) );
                else if (eCA == presentation::ClickAction_MACRO)
                    aGrpSound.SetText( String( SdResId( STR_EFFECTDLG_MACRO ) ) );
                else
                    aGrpSound.SetText( String( SdResId( STR_EFFECTDLG_PROGRAM ) ) );

                if( eCA == presentation::ClickAction_DOCUMENT )
                    CheckFileHdl( NULL );
            }
            else if( eCA == presentation::ClickAction_VERB )
            {
                aLbOLEAction.Show();
            }
            else if( eCA == presentation::ClickAction_BOOKMARK )
            {
                UpdateTree();

                aLbTreeDocument.Hide();
                aEdtSound.Hide();
                aEdtDocument.Hide();
                aEdtObject.Hide();
                aEdtPage.Enable();
                aEdtPage.Show();
                aGrpSound.Enable();

                aBtnSeek.Enable();
                aBtnSeek.Show();
                aBtnSearch.Hide();

                //aLbTree.Enable();
                aLbTree.Show();
                aGrpSound.SetText( String( SdResId( STR_EFFECTDLG_PAGE_OBJECT ) ) );
            }
        break;

        case presentation::ClickAction_VANISH:
        {
            aLbTree.Hide();
            aLbTreeDocument.Hide();
            aLbOLEAction.Hide();

            aLbEffect.Enable();
            aFtTranspColor.Enable();
            aLbTranspColor.Enable();
            aRbtSlow.Enable();
            aRbtMedium.Enable();
            aRbtFast.Enable();
            aGrpEffect.Enable();
            aTsbSound.Enable();
            aGrpSound.Enable();

            aLbEffect.Show();
            aFtTranspColor.Show();
            aLbTranspColor.Show();
            aRbtSlow.Show();
            aRbtMedium.Show();
            aRbtFast.Show();
            aGrpEffect.Show();
            aTsbPlayFull.Show();
            aTsbSound.Show();
            aGrpSound.Show();
            aGrpSound.SetText( String( SdResId( STR_EFFECTDLG_SOUND ) ) );

            aBtnSeek.Hide();
            aBtnSearch.Show();

            aEdtPage.Hide();
            aEdtDocument.Hide();
            aEdtObject.Hide();
            aEdtSound.Show();

            ClickSoundHdl( NULL );
            ChangeEffectHdl( NULL );
        }
        break;

        default:
        break;
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, SelectTreeHdl, void *, EMPTYARG )
{
    aEdtPage.SetText( aLbTree.GetSelectEntry() );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SdTPAction, CheckFileHdl, void *, EMPTYARG )
{
    String aFile( aEdtDocument.GetText() );
    String aStrTmp = aFile.ToLowerAscii();

    if (aFile != aLastFile && aStrTmp.SearchAscii(".sdd") != STRING_NOTFOUND)
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
    else
        aLbTreeDocument.Show();

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

void SdTPAction::SetEditText( String& rStr )
{
    presentation::ClickAction eCA = GetActualClickAction();

    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_VANISH:
            aEdtSound.SetText( rStr );
            break;
        case presentation::ClickAction_VERB:
            aLbOLEAction.SelectEntryPos( (USHORT)rStr.ToInt32() );
            // aEdtObject.SetText( rStr ); <-- kann raus !?!
            break;
        case presentation::ClickAction_DOCUMENT:
        case presentation::ClickAction_PROGRAM:
        case presentation::ClickAction_MACRO:
        {
            if( rStr.GetTokenCount( DOCUMENT_TOKEN ) == 2 )
                aEdtDocument.SetText( rStr.GetToken( 0, DOCUMENT_TOKEN ) );
            else
                aEdtDocument.SetText( rStr );
        }
            break;
        case presentation::ClickAction_BOOKMARK:
            aEdtPage.SetText( rStr );
            break;
    }
}

//------------------------------------------------------------------------

String SdTPAction::GetEditText( BOOL bURL )
{
    String aStr;
    presentation::ClickAction eCA = GetActualClickAction();

    switch( eCA )
    {
        case presentation::ClickAction_SOUND:
        case presentation::ClickAction_VANISH:
            return( aEdtSound.GetText() );
        case presentation::ClickAction_VERB:
            return( UniString::CreateFromInt32( aLbOLEAction.GetSelectEntryPos() ) );
            // return( aEdtObject.GetText() ); <-- kann raus !?!
        case presentation::ClickAction_DOCUMENT:
        {
            aStr = aEdtDocument.GetText();
            if( bURL && aLbTreeDocument.Control::IsVisible() &&
                        aLbTreeDocument.GetSelectionCount() > 0 )
            {
                String aTmpStr( aLbTreeDocument.GetSelectEntry() );
                if( aTmpStr.Len() )
                {
                    aStr.Append( DOCUMENT_TOKEN );
                    aStr.Append( aTmpStr );
                }
            }
        }
        break;

        case presentation::ClickAction_PROGRAM:
        {
            aStr = aEdtDocument.GetText();
        }
        break;

        case presentation::ClickAction_MACRO:
        {
            String aTmpStr = aEdtDocument.GetText();
            // Currently, the macro has got following format:
            // "Libname.Modulname.Macroname"
            // But "aMacro" Have to be following format (because of file-format )
            // "Macroname.Modulname.Libname.BASIC"
            sal_Unicode cToken = '.';
            aStr = aTmpStr.GetToken( 2, cToken );
            aStr.Append( cToken );
            aStr.Append( aTmpStr.GetToken( 1, cToken ) );
            aStr.Append( cToken );
            aStr.Append( aTmpStr.GetToken( 0, cToken ) );
            aStr.Append( cToken );
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "BASIC" ) );    // Name ist egal, wird nur wegen synt. Reihenfolge gebraucht
        }
        break;

        case presentation::ClickAction_BOOKMARK:
            return( aEdtPage.GetText() );
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


