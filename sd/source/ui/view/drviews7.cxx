/*************************************************************************
 *
 *  $RCSfile: drviews7.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:26:06 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _E3D_GLOBL3D_HXX
#include <svx/globl3d.hxx>
#endif
#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _FLDITEM_HXX
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_CLIPFMTITEM_HXX
#include <svx/clipfmtitem.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif
#ifndef _SVX_FONTWORK_BAR_HXX
#include <svx/fontworkbar.hxx>
#endif

// #UndoRedo#
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif

#include <svtools/moduleoptions.hxx>
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _PASTEDLG_HXX
#include <so3/pastedlg.hxx>
#endif

#pragma hdrstop

#include <svx/pfiledlg.hxx>
#include <svx/grafctrl.hxx>
#include <svtools/cliplistener.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif

#include "misc.hxx"
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#include "DrawDocShell.hxx"
#include "zoomlist.hxx"
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#ifndef _SDFORMATCLIPBOARD_HXX
#include "formatclipboard.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "LayerTabBar.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

//////////////////////////////////////////////////////////////////////////////
// service routine for Undo/Redo implementation
namespace {
SfxUndoManager* ImpGetUndoManagerFromViewShell (::sd::DrawViewShell& rDViewShell)
{
    ::sd::ViewShell* pViewShell = rDViewShell.GetDocSh()->GetViewShell();
    if(pViewShell)
        return pViewShell->GetViewFrame()->GetDispatcher()->GetShell(0)->GetUndoManager();
    DBG_ASSERT(pViewShell, "ViewShell not found");
    return 0L;
}
}

namespace sd {

IMPL_LINK( DrawViewShell, ClipboardChanged, TransferableDataHelper*, pDataHelper )
{
    if ( pDataHelper )
    {
        bPastePossible = ( pDataHelper->GetFormatCount() != 0 );

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PASTE );
        rBindings.Invalidate( SID_PASTE2 );
        rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    return 0;
}

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void DrawViewShell::GetMenuState( SfxItemSet &rSet )
{
    ViewShell::GetMenuState(rSet);
    BOOL bDisableVerticalText = !SvtLanguageOptions().IsVerticalTextEnabled();

    if (pFuSlideShow)
        rSet.Put(SfxBoolItem(SID_LIVE_PRESENTATION, pFuSlideShow->IsLivePresentation()));

    if ( bDisableVerticalText )
    {
        rSet.DisableItem( SID_DRAW_FONTWORK_VERTICAL );
        rSet.DisableItem( SID_DRAW_CAPTION_VERTICAL );
        rSet.DisableItem( SID_TEXT_FITTOSIZE_VERTICAL );
        rSet.DisableItem( SID_DRAW_TEXT_VERTICAL );
    }

    SfxApplication* pApp = SFX_APP();

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) )
    {
        SfxChildWindow* pPreviewChildWindow = GetViewFrame()->GetChildWindow(
            PreviewChildWindow::GetChildWindowId());
        PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
            pPreviewChildWindow ? pPreviewChildWindow->GetWindow() : NULL );
        FuSlideShow* pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

        if ( (pShow && pShow->IsInputLocked()) ||
             GetDocSh()->IsPreview() )
        {
            rSet.DisableItem( SID_PRESENTATION );
        }
    }

    FASTBOOL bConvertToPathPossible = pDrView->IsConvertToPathObjPossible(FALSE);

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

    //format paintbrush
    {
        SdFormatClipboard* pFormatClipboard = GetDocSh()->pFormatClipboard;
        bool bHasContent = pFormatClipboard && pFormatClipboard->HasContent();
        rSet.Put(SfxBoolItem(SID_FORMATPAINTBRUSH,bHasContent));
        if( ( nMarkCount!=1 && !bHasContent ) || pDrView->IsTextEdit() )
            rSet.DisableItem( SID_FORMATPAINTBRUSH );
        if( !bHasContent && nMarkCount==1 )
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
            if( !pFormatClipboard->CanCopyThisType(pObj->GetObjInventor(),pObj->GetObjIdentifier()) )
                rSet.DisableItem( SID_FORMATPAINTBRUSH );
        }
    }

    // Stati der SfxChild-Windows (Animator, Fontwork etc.)
    SetChildWindowState( rSet );

    // Images der Toolboxen mappen (nur Zoom)
    UpdateToolboxImages( rSet, FALSE );

    if (pFuActual)
    {
        USHORT nSId = pFuActual->GetSlotID();

//      switch ( nSId )
//      {
//          case SID_OBJECT_SELECT:
//              rSet.Put( SfxAllEnumItem( SID_OBJECT_SELECT, pFuActual->GetSlotValue() ) );
//              break;
//          default:
                rSet.Put( SfxBoolItem( nSId, TRUE ) );
//      }

        // Bewirkt ein uncheck eines simulierten Slots
        USHORT nId = GetIdBySubId( nSId );

        // Images der Toolboxen mappen
        UpdateToolboxImages( rSet );

        if( nId != SID_ZOOM_TOOLBOX &&
            nSId != SID_DRAWTBX_INSERT &&
            nSId != SID_POSITION &&
            nSId != SID_OBJECT_ALIGN )
        {
            if( nId != SID_OBJECT_CHOOSE_MODE )
                rSet.Put( TbxImageItem( SID_OBJECT_CHOOSE_MODE, 0 ) );
            if( nId != SID_DRAWTBX_TEXT )
                rSet.Put( TbxImageItem( SID_DRAWTBX_TEXT, 0 ) );
            if( nId != SID_DRAWTBX_RECTANGLES )
                rSet.Put( TbxImageItem( SID_DRAWTBX_RECTANGLES, 0 ) );
            if( nId != SID_DRAWTBX_ELLIPSES )
                rSet.Put( TbxImageItem( SID_DRAWTBX_ELLIPSES, 0 ) );
            if( nId != SID_DRAWTBX_LINES )
                rSet.Put( TbxImageItem( SID_DRAWTBX_LINES, 0 ) );
            if( nId != SID_DRAWTBX_ARROWS )
                rSet.Put( TbxImageItem( SID_DRAWTBX_ARROWS, 0 ) );
            if( nId != SID_DRAWTBX_3D_OBJECTS )
                rSet.Put( TbxImageItem( SID_DRAWTBX_3D_OBJECTS, 0 ) );
            if( nId != SID_DRAWTBX_CONNECTORS )
                rSet.Put( TbxImageItem( SID_DRAWTBX_CONNECTORS, 0 ) );
        }
    }


    SdrPageView* pPageView = pDrView->GetPageViewPvNum( 0 );

    GetMenuStateSel(rSet);

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        BOOL bDisable = TRUE;
        SdPage* pPage = (SdPage*) pPageView->GetPage();

        if ( pPage->GetPageKind() == PK_STANDARD && !pPage->IsMasterPage() )
        {
            SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);

            if (pObj && !pObj->IsEmptyPresObj())
            {
                bDisable = FALSE;
            }
        }

        if (bDisable)
        {
            rSet.DisableItem(SID_EXPAND_PAGE);
        }
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        BOOL bDisable = TRUE;
        SdPage* pPage = (SdPage*) pPageView->GetPage();

        if (pPage->GetPageKind() == PK_STANDARD && !pPage->IsMasterPage() )
        {
            SdrObject* pObj = pPage->GetPresObj(PRESOBJ_TITLE);

            if (pObj && !pObj->IsEmptyPresObj())
            {
                bDisable = FALSE;
            }
        }

        if (bDisable)
        {
            rSet.DisableItem(SID_SUMMARY_PAGE);
        }
    }

    // Starten der Praesentation moeglich?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_REHEARSE_TIMINGS ) )
    {
        BOOL bDisable = TRUE;
        USHORT nCount = GetDoc()->GetSdPageCount( PK_STANDARD );

        for( USHORT i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if( !pPage->IsExcluded() )
                bDisable = FALSE;
        }

        if( bDisable || GetDocSh()->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
            rSet.DisableItem( SID_REHEARSE_TIMINGS );
        }
    }

    // Klebepunkte
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_EDITMODE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_INSERT_POINT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_PERCENT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_ESCDIR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_ESCDIR_LEFT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_ESCDIR_RIGHT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_ESCDIR_TOP ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_ESCDIR_BOTTOM ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_HORZALIGN_CENTER ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_HORZALIGN_LEFT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_HORZALIGN_RIGHT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_VERTALIGN_CENTER ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_VERTALIGN_TOP ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GLUE_VERTALIGN_BOTTOM ) )
    {
        // Prozent
        TRISTATE eState = pDrView->IsMarkedGluePointsPercent();
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_PERCENT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_PERCENT, eState == STATE_CHECK ) );

        // Bei Prozent hat Alignment keinen Effekt
        if( eState == STATE_CHECK )
        {
            rSet.DisableItem( SID_GLUE_HORZALIGN_CENTER );
            rSet.DisableItem( SID_GLUE_HORZALIGN_LEFT );
            rSet.DisableItem( SID_GLUE_HORZALIGN_RIGHT );
            rSet.DisableItem( SID_GLUE_VERTALIGN_CENTER );
            rSet.DisableItem( SID_GLUE_VERTALIGN_TOP );
            rSet.DisableItem( SID_GLUE_VERTALIGN_BOTTOM );
        }
        else
        {
            // Horizontale Ausrichtung
            USHORT nHorz = pDrView->GetMarkedGluePointsAlign( FALSE );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_CENTER, nHorz == SDRHORZALIGN_CENTER ) );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_LEFT,   nHorz == SDRHORZALIGN_LEFT ) );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_RIGHT,  nHorz == SDRHORZALIGN_RIGHT ) );
            // Vertikale Ausrichtung
            USHORT nVert = pDrView->GetMarkedGluePointsAlign( TRUE );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_CENTER, nVert == SDRVERTALIGN_CENTER ) );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_TOP,    nVert == SDRVERTALIGN_TOP ) );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_BOTTOM, nVert == SDRVERTALIGN_BOTTOM ) );
        }

        // Punkt einfuegen
        rSet.Put( SfxBoolItem( SID_GLUE_INSERT_POINT, pDrView->IsInsGluePointMode() ) );

        // Autrittsrichtung
        // Links
        eState = pDrView->IsMarkedGluePointsEscDir( SDRESC_LEFT );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_LEFT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_LEFT, eState == STATE_CHECK ) );
        // Rechts
        eState = pDrView->IsMarkedGluePointsEscDir( SDRESC_RIGHT );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_RIGHT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_RIGHT, eState == STATE_CHECK ) );
        // Oben
        eState = pDrView->IsMarkedGluePointsEscDir( SDRESC_TOP );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_TOP );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_TOP, eState == STATE_CHECK ) );
        // Unten
        eState = pDrView->IsMarkedGluePointsEscDir( SDRESC_BOTTOM );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_BOTTOM );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_BOTTOM, eState == STATE_CHECK ) );
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GRID_FRONT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_HELPLINES_FRONT ) )
    {
        rSet.Put( SfxBoolItem( SID_GRID_FRONT, pDrView->IsGridFront() ) );
        rSet.Put( SfxBoolItem( SID_HELPLINES_FRONT, pDrView->IsHlplFront() ) );
    }

    if (!pDrView->IsFrameDragSingles())
    {
        rSet.Put(SfxBoolItem(SID_BEZIER_EDIT, TRUE));
    }

    if( !pDrView->IsMirrorAllowed( TRUE, TRUE ) )
    {
        rSet.DisableItem( SID_HORIZONTAL );
        rSet.DisableItem( SID_VERTICAL );
    }

    if( !pDrView->IsMirrorAllowed() )
    {
        rSet.DisableItem( SID_OBJECT_MIRROR );
//        rSet.DisableItem( SID_CONVERT_TO_3D_LATHE );
//        rSet.DisableItem( SID_CONVERT_TO_3D_LATHE_FAST );
    }

    // interactive transparence control
    if(!pDrView->IsTransparenceAllowed())
    {
        rSet.DisableItem( SID_OBJECT_TRANSPARENCE );
    }

    // interactive gradient control
    if(!pDrView->IsGradientAllowed())
    {
        rSet.DisableItem( SID_OBJECT_GRADIENT );
    }

    // Morphen ggf. disablen
    if ( !pDrView->IsMorphingAllowed() )
        rSet.DisableItem( SID_POLYGON_MORPHING );

    // Vectorize ggf. disablen
    if ( !pDrView->IsVectorizeAllowed() )
        rSet.DisableItem( SID_VECTORIZE );

    if( !pDrView->IsReverseOrderPossible() )
    {
        rSet.DisableItem( SID_REVERSE_ORDER );
    }

    if ( !bConvertToPathPossible &&
         !pDrView->IsCrookAllowed( pDrView->IsCrookNoContortion() ) )
    {
        // Implizite Wandlung in Kurve nicht moeglich
        rSet.DisableItem(SID_OBJECT_CROOK_ROTATE);
        rSet.DisableItem(SID_OBJECT_CROOK_SLANT);
        rSet.DisableItem(SID_OBJECT_CROOK_STRETCH);
    }

    if ( !pDrView->IsGroupEntered() )
    {
        rSet.DisableItem( SID_LEAVE_GROUP );
        rSet.Put( SfxBoolItem( SID_LEAVE_ALL_GROUPS, FALSE ) );
        rSet.ClearItem( SID_LEAVE_ALL_GROUPS );
        rSet.DisableItem( SID_LEAVE_ALL_GROUPS );
    }
    else
        rSet.Put( SfxBoolItem( SID_LEAVE_ALL_GROUPS, TRUE ) );

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_THESAURUS ) )
    {
        if ( !pDrView->IsTextEdit() )
        {
            rSet.DisableItem( SID_THESAURUS );
        }
        else
        {
            LanguageType            eLang = GetDoc()->GetLanguage( EE_CHAR_LANGUAGE );
            Reference< XThesaurus > xThesaurus( LinguMgr::GetThesaurus() );
            Locale                  aLocale;

            SvxLanguageToLocale( aLocale, eLang );

            if (!xThesaurus.is() || eLang == LANGUAGE_NONE || !xThesaurus->hasLocale(aLocale) )
                rSet.DisableItem( SID_THESAURUS );
        }
    }

    if ( !pDrView->IsTextEdit() )
    {
        rSet.DisableItem( SID_THESAURUS );
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SELECTALL ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SIZE_ALL ) )
    {
        if( pPageView && pPageView->GetObjList()->GetObjCount() == 0 )
        {
            // Sollte disabled sein, wenn kein Objekt auf der Zeichenflaeche ist:
            rSet.DisableItem( SID_SELECTALL );
            rSet.DisableItem( SID_SIZE_ALL );
        }
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONTEXT ) )
        rSet.Put( SfxStringItem( SID_CONTEXT, pDrView->GetStatusText() ) );

    // clipboard (paste)
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PASTE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PASTE2 ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) )
    {
        if ( !pClipEvtLstnr )
        {
            // SSA: #108717# avoid clipboard initialization for read-only presentation views (workaround for NT4.0 clipboard prob...)
            if( !ISA(PresentationViewShell)
                || ( pFuSlideShow && pFuSlideShow->IsLivePresentation())  )
            {
                // create listener
                pClipEvtLstnr = new TransferableClipboardListener( LINK( this, DrawViewShell, ClipboardChanged ) );
                pClipEvtLstnr->acquire();
                pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), TRUE );

                // get initial state
                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
                bPastePossible = ( aDataHelper.GetFormatCount() != 0 );
            }
            else
                bPastePossible = FALSE;
        }

        if( !bPastePossible )
        {
            rSet.DisableItem( SID_PASTE );
            rSet.DisableItem( SID_PASTE2 );
            rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );
        }
        else if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) )
        {
            SvxClipboardFmtItem aItem( SID_CLIPBOARD_FORMAT_ITEMS );
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );

            for( sal_uInt32 i = 0; i < aDataHelper.GetFormatCount(); i++ )
            {
                static SotFormatStringId aSupportedFormats[] =
                {
                    SOT_FORMATSTR_ID_EMBED_SOURCE,
                    SOT_FORMATSTR_ID_LINK_SOURCE,
                    SOT_FORMATSTR_ID_DRAWING,
                    SOT_FORMATSTR_ID_SVXB,
                    FORMAT_GDIMETAFILE,
                    FORMAT_BITMAP,
                    SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK,
                    FORMAT_STRING,
                    SOT_FORMATSTR_ID_HTML,
                    FORMAT_RTF,
                    SOT_FORMATSTR_ID_EDITENGINE
                };

                ::com::sun::star::datatransfer::DataFlavor  aFlavor;
                const SotFormatStringId                     nTestFormat = aDataHelper.GetFormat( i );

                sal_uInt32  j;
                sal_Bool    bDouble = sal_False;
                for ( j = 0; j < i; j++ )
                {
                    const SotFormatStringId nF = aDataHelper.GetFormat( j );
                    if (  nTestFormat == nF )
                    {
                        bDouble = sal_True;
                        break;
                    }
                }
                if ( bDouble == sal_False )
                {
                    for( sal_uInt32 n = 0, nCount = sizeof( aSupportedFormats ) / sizeof( SotFormatStringId ); n < nCount; n++ )
                    {
                        if( nTestFormat == aSupportedFormats[ n ] )
                        {
                            String aName;

                            if( SOT_FORMATSTR_ID_EMBED_SOURCE == nTestFormat )
                            {
                                TransferableObjectDescriptor aDesc;

                                if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc ) )
                                    aName = aDesc.maTypeName;
                            }

                            if( aName.Len() )
                                aItem.AddClipbrdFormat( nTestFormat, aName );
                            else
                                aItem.AddClipbrdFormat( nTestFormat );
                        }
                    }
                }
            }

            SotFormatStringId nFormat;
            bool bHasFormat = false;
            nFormat = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
            bHasFormat = aDataHelper.HasFormat (nFormat);
            if ( ! bHasFormat)
            {
                nFormat = SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE;
                bHasFormat = aDataHelper.HasFormat (nFormat);
            }
            if (bHasFormat)
            {
                String sName,sSource;
                if (SvPasteObjectDialog::GetEmbeddedName (aDataHelper, sName, sSource, nFormat))
                    aItem.AddClipbrdFormat (nFormat, sName);
            }

            rSet.Put( aItem );
        }
    }

    if ( !bConvertToPathPossible )
    {
        rSet.DisableItem(SID_CHANGEBEZIER);
    }

    if( !( pDrView->IsConvertToPolyObjPossible(FALSE) || pDrView->IsVectorizeAllowed() ) )
        rSet.DisableItem(SID_CHANGEPOLYGON);

    if( !( pDrView->IsConvertToPolyObjPossible(FALSE) || pDrView->IsConvertToContourPossible() ) )
        rSet.DisableItem(SID_CONVERT_TO_CONTOUR);

    if ( !pDrView->IsConvertTo3DObjPossible() )
    {
        rSet.DisableItem(SID_CONVERT_TO_3D);
        rSet.DisableItem(SID_CONVERT_TO_3D_LATHE);
        rSet.DisableItem(SID_CONVERT_TO_3D_LATHE_FAST);
    }

    if (pDrView->IsPixelMode())
    {
        rSet.Put(SfxBoolItem(SID_PIXELMODE, TRUE));
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_PIXELMODE, FALSE));
    }

    if (pDrView->GetSlideShow())
    {
        rSet.Put(SfxBoolItem(SID_ANIMATIONMODE, TRUE));
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_ANIMATIONMODE, FALSE));
    }

    if (pDrView->IsActionMode())
    {
        rSet.Put(SfxBoolItem(SID_ACTIONMODE, TRUE));
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_ACTIONMODE, FALSE));
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MANAGE_LINKS ) )
    {
        if ( GetDoc()->GetLinkCount() == 0 )
        {
            rSet.DisableItem(SID_MANAGE_LINKS);
        }
    }

    if (ePageKind == PK_HANDOUT)
    {
        rSet.DisableItem(SID_ANIMATIONMODE);
        rSet.DisableItem(SID_PRESENTATION_LAYOUT);
    }

    if (ePageKind == PK_NOTES)
    {
        rSet.DisableItem(SID_ANIMATIONMODE);
        rSet.DisableItem(SID_INSERTPAGE);
        rSet.DisableItem(SID_RENAMEPAGE);
        rSet.DisableItem(SID_RENAMEPAGE_QUICK);
        rSet.DisableItem(SID_DUPLICATE_PAGE);
        rSet.ClearItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_EFFECTS);

        if (eEditMode == EM_MASTERPAGE)
            rSet.DisableItem(SID_MODIFYPAGE);

        rSet.DisableItem(SID_INSERTLAYER);
        rSet.DisableItem(SID_LAYERMODE);
        rSet.DisableItem(SID_INSERTFILE);
    }
    else if (ePageKind == PK_HANDOUT)
    {
        rSet.DisableItem(SID_INSERTPAGE);
        rSet.DisableItem(SID_DUPLICATE_PAGE);
        rSet.ClearItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_EFFECTS);
        rSet.DisableItem(SID_RENAMEPAGE);
        rSet.DisableItem(SID_RENAMEPAGE_QUICK);
        rSet.DisableItem(SID_INSERTLAYER);
        rSet.DisableItem(SID_MODIFYLAYER);
        rSet.DisableItem(SID_RENAMELAYER);
        rSet.DisableItem(SID_LAYERMODE);
        rSet.DisableItem(SID_INSERTFILE);
        rSet.DisableItem(SID_PAGEMODE);
    }
    else
    {
        if (eEditMode == EM_MASTERPAGE)
        {
            rSet.DisableItem(SID_INSERTPAGE);
            rSet.DisableItem(SID_DUPLICATE_PAGE);
            rSet.DisableItem(SID_MODIFYPAGE);
            rSet.ClearItem(SID_ANIMATION_OBJECTS);
            rSet.DisableItem(SID_ANIMATION_OBJECTS);
        }

        rSet.Put (SfxBoolItem (SID_LAYERMODE, IsLayerModeActive()));
    }

    if ( ! IsLayerModeActive())
    {
        rSet.DisableItem( SID_INSERTLAYER );
        rSet.DisableItem( SID_MODIFYLAYER );
        rSet.DisableItem( SID_DELETE_LAYER );
        rSet.DisableItem( SID_RENAMELAYER );
    }

    if (eEditMode == EM_PAGE)
    {
        /**********************************************************************
        * Seiten-Modus
        **********************************************************************/
        rSet.Put(SfxBoolItem(SID_PAGEMODE, TRUE));
        rSet.Put(SfxBoolItem(SID_MASTERPAGE, FALSE));
        rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, FALSE));
        rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, FALSE));
        rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, FALSE));

        if (ePageKind == PK_STANDARD &&
            rSet.GetItemState(SID_TITLE_MASTERPAGE) == SFX_ITEM_AVAILABLE)
        {
            // Gibt es eine Seite mit dem AutoLayout "Titel"?
            BOOL bDisable = TRUE;
            USHORT i = 0;
            USHORT nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

            while (i < nCount && bDisable)
            {
                SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

                if (pPage->GetAutoLayout() == AUTOLAYOUT_TITLE)
                {
                    bDisable = FALSE;
                }

                i++;
            }

            if (bDisable)
            {
                rSet.DisableItem(SID_TITLE_MASTERPAGE);
            }
            else
            {
                rSet.Put(SfxBoolItem(SID_TITLE_MASTERPAGE, FALSE));
            }
        }
        else
        {
            rSet.DisableItem(SID_TITLE_MASTERPAGE);
        }
    }
    else
    {
        /**********************************************************************
        * Hintergrundseiten-Modus
        **********************************************************************/
        if (ePageKind == PK_STANDARD)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, TRUE));
            rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, FALSE));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, FALSE));

            if (rSet.GetItemState(SID_TITLE_MASTERPAGE) == SFX_ITEM_AVAILABLE)
            {
                SdPage* pMPage = (SdPage*) pPageView->GetPage();

                BOOL bCheck = FALSE;
                BOOL bDisable = TRUE;
                USHORT i = 0;
                USHORT nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

                // Referenziert eine Seite mit dem AutoLayout "Titel" die
                // aktuelle MasterPage?
                while (i < nCount && !bCheck && bDisable)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

                    // Seite referenziert aktuelle MasterPage
                    if (pPage->GetAutoLayout() == AUTOLAYOUT_TITLE)
                    {
                        // Eine Seite hat das AutoLayout "Titel"
                        bDisable = FALSE;

                        SdPage& rRefMPage = (SdPage&)(pPage->TRG_GetMasterPage());

                        if(&rRefMPage == pMPage)
                        {
                            // Eine Seite mit dem AutoLayout "Titel"
                            // referenziert die aktuelle MasterPage
                            bCheck = TRUE;
                        }
                    }

                    i++;
                }

                if (bCheck)
                {
                    rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, FALSE));
                }

                rSet.Put(SfxBoolItem(SID_TITLE_MASTERPAGE, bCheck));

                if (bDisable)
                {
                    rSet.ClearItem(SID_TITLE_MASTERPAGE);
                    rSet.DisableItem(SID_TITLE_MASTERPAGE);
                }
            }
        }
        else if (ePageKind == PK_NOTES)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, FALSE));
            rSet.DisableItem(SID_TITLE_MASTERPAGE);
            rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, TRUE));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, FALSE));
        }
        else if (ePageKind == PK_HANDOUT)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, FALSE));
            rSet.DisableItem(SID_TITLE_MASTERPAGE);
            rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, FALSE));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, TRUE));
        }
    }

    // Status der Lineale setzen
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_RULER ) )
        rSet.Put( SfxBoolItem( SID_RULER, HasRuler() ) );

    // nicht die letzte Seite oder eine Masterpage loeschen
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DELETE_PAGE ) )
    {
        if (aTabControl.GetPageCount() == 1 ||
            eEditMode == EM_MASTERPAGE      ||
            ePageKind == PK_NOTES           ||
            ePageKind == PK_HANDOUT         ||
            IsLayerModeActive())
        {
            rSet.DisableItem(SID_DELETE_PAGE);
        }
    }

    // darf der aktuelle Layer geloescht werden?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DELETE_LAYER ) )
    {
        SdrLayerAdmin& rAdmin = GetDoc()->GetLayerAdmin();

        USHORT        nCurrentLayer = GetLayerTabControl()->GetCurPageId();
        const String& rName         = GetLayerTabControl()->GetPageText(nCurrentLayer);

        BOOL bDisableIt = !IsLayerModeActive();
        bDisableIt |= (rName == String(SdResId(STR_LAYER_LAYOUT)));
        bDisableIt |= (rName == String(SdResId(STR_LAYER_BCKGRND)));
        bDisableIt |= (rName == String(SdResId(STR_LAYER_BCKGRNDOBJ)));
        bDisableIt |= (rName == String(SdResId(STR_LAYER_CONTROLS)));
        bDisableIt |= (rName == String(SdResId(STR_LAYER_MEASURELINES)));

        if (bDisableIt)
        {
            rSet.DisableItem(SID_DELETE_LAYER);
            rSet.DisableItem(SID_RENAMELAYER);
        }
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CUT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_COPY ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTLINE_BULLET ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BULLET ) )
    {
        OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();

        // Sonderbehandlung für SID_OUTLINE_BULLET wenn Objekte
        // mit unterschiedlichen arten von NumBullet Items markiert
        // sind
        BOOL bHasOutliner = FALSE;
        BOOL bHasOther    = FALSE;
        const SdrMarkList& rMarkList = GetView()->GetMarkedObjectList();
        const ULONG nCount = rMarkList.GetMarkCount();
        for(ULONG nNum = 0; nNum < nCount; nNum++)
        {
            SdrObject* pObj = rMarkList.GetMark(nNum)->GetObj();
            if( pObj->GetObjInventor() == SdrInventor )
            {
                if( pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
                {
                    bHasOutliner = TRUE;
                    if(bHasOther)
                        break;
                }
                else
                {
                    bHasOther = TRUE;
                    if(bHasOutliner)
                        break;
                }
            }
        }

        if( bHasOther && bHasOutliner )
            rSet.DisableItem( SID_OUTLINE_BULLET );

        if (pOlView)
        {
            if ( pOlView->GetSelected().Len() == 0 )
            {
                rSet.DisableItem( SID_CUT );
                rSet.DisableItem( SID_COPY );
            }
        }

        // ggfs. Menuepunkt "Aufzaehlungszeichen" bzw. "Sonderzeichen" disablen
        if (! pOlView )
        {
            rSet.DisableItem(SID_BULLET);
        }
        else if (!pDrView->GetTextEditObject())
        {
            rSet.DisableItem(SID_BULLET);
        }
    }

    if ( GetDocSh()->IsUIActive() )
    {
        rSet.DisableItem( SID_INSERT_OBJECT );
        rSet.DisableItem( SID_INSERT_PLUGIN );
        rSet.DisableItem( SID_INSERT_SOUND );
        rSet.DisableItem( SID_INSERT_VIDEO );
        rSet.DisableItem( SID_INSERT_APPLET );
        rSet.DisableItem( SID_INSERT_FLOATINGFRAME );
#ifdef STARIMAGE_AVAILABLE
        rSet.DisableItem( SID_INSERT_IMAGE );
#endif
        rSet.DisableItem( SID_INSERT_MATH );
        rSet.DisableItem( SID_INSERT_DIAGRAM );
        rSet.DisableItem( SID_ATTR_TABLE );
        rSet.DisableItem( SID_SIZE_REAL );
        rSet.DisableItem( SID_SIZE_OPTIMAL );
        rSet.DisableItem( SID_SIZE_ALL );
        rSet.DisableItem( SID_SIZE_PAGE_WIDTH );
        rSet.DisableItem( SID_SIZE_PAGE );
//      rSet.DisableItem( SID_INSERTPAGE );
        rSet.DisableItem( SID_DUPLICATE_PAGE );
        rSet.DisableItem( SID_ZOOM_TOOLBOX );
    }

    // Zoom-Stati
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ZOOM_IN ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ZOOM_OUT )||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ZOOM_PANNING ) )
    {
        if( GetActiveWindow()->GetZoom() <= GetActiveWindow()->GetMinZoom() || GetDocSh()->IsUIActive() )
        {
            rSet.DisableItem( SID_ZOOM_IN );
            rSet.DisableItem( SID_ZOOM_PANNING );
        }
        if( GetActiveWindow()->GetZoom() >= GetActiveWindow()->GetMaxZoom() || GetDocSh()->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_OUT );
    }

    if (!pZoomList->IsNextPossible())
    {
       rSet.DisableItem(SID_ZOOM_NEXT);
    }
    if (!pZoomList->IsPreviousPossible())
    {
       rSet.DisableItem(SID_ZOOM_PREV);
    }

    // EditText aktiv
    if (GetObjectBarManager().GetTopObjectBarId() == RID_DRAW_TEXT_TOOLBOX)
    {
        USHORT nCurrentSId = SID_ATTR_CHAR;

        if (pFuActual)
        {
            nCurrentSId = pFuActual->GetSlotID();
        }
        if( nCurrentSId != SID_TEXT_FITTOSIZE &&
            nCurrentSId != SID_TEXT_FITTOSIZE_VERTICAL &&
            nCurrentSId != SID_ATTR_CHAR_VERTICAL )
            nCurrentSId = SID_ATTR_CHAR;

        rSet.Put( SfxBoolItem( nCurrentSId, TRUE ) );

        // Kurzform von UpdateToolboxImages()
        rSet.Put( TbxImageItem( SID_DRAWTBX_TEXT, nCurrentSId ) );
    }

    if ( GetDocSh()->IsReadOnly() )
    {
        rSet.DisableItem( SID_AUTOSPELL_CHECK );
    }
    else
    {
        if (GetDoc()->GetOnlineSpell())
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, TRUE));
        }
        else
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, FALSE));
        }
    }

    SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
    String aActiveLayer = pDrView->GetActiveLayer();

    if ( ( aActiveLayer.Len() != 0 && pPV && ( pPV->IsLayerLocked(aActiveLayer) ||
          !pPV->IsLayerVisible(aActiveLayer) ) ) ||
          SD_MOD()->GetWaterCan() )
    {
        rSet.DisableItem( SID_PASTE );
        rSet.DisableItem( SID_PASTE2 );
        rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );

        rSet.DisableItem( SID_INSERT_FLD_DATE_FIX );
        rSet.DisableItem( SID_INSERT_FLD_DATE_VAR );
        rSet.DisableItem( SID_INSERT_FLD_TIME_FIX );
        rSet.DisableItem( SID_INSERT_FLD_TIME_VAR );
        rSet.DisableItem( SID_INSERT_FLD_AUTHOR );
        rSet.DisableItem( SID_INSERT_FLD_PAGE );
        rSet.DisableItem( SID_INSERT_FLD_FILE );

        rSet.DisableItem( SID_INSERT_GRAPHIC );
        rSet.DisableItem( SID_INSERT_DIAGRAM );
        rSet.DisableItem( SID_INSERT_OBJECT );
        rSet.DisableItem( SID_INSERT_PLUGIN );
        rSet.DisableItem( SID_INSERT_SOUND );
        rSet.DisableItem( SID_INSERT_VIDEO );
        rSet.DisableItem( SID_INSERT_APPLET );
        rSet.DisableItem( SID_INSERT_FLOATINGFRAME );

#ifdef STARIMAGE_AVAILABLE
        rSet.DisableItem( SID_INSERT_IMAGE );
#endif
        rSet.DisableItem( SID_INSERT_MATH );
        rSet.DisableItem( SID_INSERT_FRAME );
        rSet.DisableItem( SID_INSERTFILE );
        rSet.DisableItem( SID_ATTR_TABLE );
        rSet.DisableItem( SID_COPYOBJECTS );

        rSet.DisableItem( SID_SCAN );
        rSet.DisableItem( SID_TWAIN_SELECT );
        rSet.DisableItem( SID_TWAIN_TRANSFER );

//        rSet.DisableItem( SID_BEZIER_EDIT );
        rSet.DisableItem( SID_GLUE_EDITMODE );
        rSet.DisableItem( SID_OBJECT_ROTATE );
        rSet.DisableItem( SID_OBJECT_SHEAR );
        rSet.DisableItem( SID_OBJECT_MIRROR );
        rSet.DisableItem( SID_OBJECT_TRANSPARENCE );
        rSet.DisableItem( SID_OBJECT_GRADIENT );
        rSet.DisableItem( SID_OBJECT_CROOK_ROTATE );
        rSet.DisableItem( SID_OBJECT_CROOK_SLANT );
        rSet.DisableItem( SID_OBJECT_CROOK_STRETCH );

        // Alle objekterzeugenden Werkzeuge disablen
        rSet.ClearItem( SID_ATTR_CHAR );
        rSet.DisableItem( SID_ATTR_CHAR );
        rSet.ClearItem( SID_ATTR_CHAR_VERTICAL );
        rSet.DisableItem( SID_ATTR_CHAR_VERTICAL );
        rSet.ClearItem(SID_DRAW_LINE);
        rSet.DisableItem(SID_DRAW_LINE);
        rSet.ClearItem(SID_DRAW_MEASURELINE);
        rSet.DisableItem(SID_DRAW_MEASURELINE);
        rSet.ClearItem(SID_DRAW_XLINE);
        rSet.DisableItem(SID_DRAW_XLINE);
        rSet.ClearItem( SID_LINE_ARROW_START );
        rSet.DisableItem( SID_LINE_ARROW_START );
        rSet.ClearItem( SID_LINE_ARROW_END );
        rSet.DisableItem( SID_LINE_ARROW_END );
        rSet.ClearItem( SID_LINE_ARROWS );
        rSet.DisableItem( SID_LINE_ARROWS );
        rSet.ClearItem( SID_LINE_ARROW_CIRCLE );
        rSet.DisableItem( SID_LINE_ARROW_CIRCLE );
        rSet.ClearItem( SID_LINE_CIRCLE_ARROW );
        rSet.DisableItem( SID_LINE_CIRCLE_ARROW );
        rSet.ClearItem( SID_LINE_ARROW_SQUARE );
        rSet.DisableItem( SID_LINE_ARROW_SQUARE );
        rSet.ClearItem( SID_LINE_SQUARE_ARROW );
        rSet.DisableItem( SID_LINE_SQUARE_ARROW );

        rSet.ClearItem(SID_DRAW_RECT);
        rSet.DisableItem(SID_DRAW_RECT);
        rSet.ClearItem(SID_DRAW_RECT_NOFILL);
        rSet.DisableItem(SID_DRAW_RECT_NOFILL);
        rSet.ClearItem(SID_DRAW_RECT_ROUND);
        rSet.DisableItem(SID_DRAW_RECT_ROUND);
        rSet.ClearItem(SID_DRAW_RECT_ROUND_NOFILL);
        rSet.DisableItem(SID_DRAW_RECT_ROUND_NOFILL);
        rSet.ClearItem(SID_DRAW_SQUARE);
        rSet.DisableItem(SID_DRAW_SQUARE);
        rSet.ClearItem(SID_DRAW_SQUARE_NOFILL);
        rSet.DisableItem(SID_DRAW_SQUARE_NOFILL);
        rSet.ClearItem(SID_DRAW_SQUARE_ROUND);
        rSet.DisableItem(SID_DRAW_SQUARE_ROUND);
        rSet.ClearItem(SID_DRAW_SQUARE_ROUND_NOFILL);
        rSet.DisableItem(SID_DRAW_SQUARE_ROUND_NOFILL);
        rSet.ClearItem(SID_DRAW_ELLIPSE);
        rSet.DisableItem(SID_DRAW_ELLIPSE);
        rSet.ClearItem(SID_DRAW_ELLIPSE_NOFILL);
        rSet.DisableItem(SID_DRAW_ELLIPSE_NOFILL);
        rSet.ClearItem(SID_DRAW_CIRCLE);
        rSet.DisableItem(SID_DRAW_CIRCLE);
        rSet.ClearItem(SID_DRAW_CIRCLE_NOFILL);
        rSet.DisableItem(SID_DRAW_CIRCLE_NOFILL);
        rSet.ClearItem(SID_DRAW_CAPTION);
        rSet.DisableItem(SID_DRAW_CAPTION);
        rSet.ClearItem(SID_DRAW_FONTWORK);
        rSet.DisableItem(SID_DRAW_FONTWORK);
        rSet.ClearItem(SID_DRAW_FONTWORK_VERTICAL);
        rSet.DisableItem(SID_DRAW_FONTWORK_VERTICAL);
        rSet.ClearItem(SID_DRAW_CAPTION_VERTICAL);
        rSet.DisableItem(SID_DRAW_CAPTION_VERTICAL);
        rSet.ClearItem(SID_TEXT_FITTOSIZE);
        rSet.DisableItem(SID_TEXT_FITTOSIZE);
        rSet.ClearItem(SID_TEXT_FITTOSIZE_VERTICAL);
        rSet.DisableItem(SID_TEXT_FITTOSIZE_VERTICAL);
        rSet.ClearItem(SID_TOOL_CONNECTOR);
        rSet.DisableItem(SID_TOOL_CONNECTOR);
        rSet.ClearItem(SID_CONNECTOR_ARROW_START);
        rSet.DisableItem(SID_CONNECTOR_ARROW_START);
        rSet.ClearItem(SID_CONNECTOR_ARROW_END);
        rSet.DisableItem(SID_CONNECTOR_ARROW_END);
        rSet.ClearItem(SID_CONNECTOR_ARROWS);
        rSet.DisableItem(SID_CONNECTOR_ARROWS);
        rSet.ClearItem(SID_CONNECTOR_CIRCLE_START);
        rSet.DisableItem(SID_CONNECTOR_CIRCLE_START);
        rSet.ClearItem(SID_CONNECTOR_CIRCLE_END);
        rSet.DisableItem(SID_CONNECTOR_CIRCLE_END);
        rSet.ClearItem(SID_CONNECTOR_CIRCLES);
        rSet.DisableItem(SID_CONNECTOR_CIRCLES);
        rSet.ClearItem(SID_CONNECTOR_LINE);
        rSet.DisableItem(SID_CONNECTOR_LINE);
        rSet.ClearItem(SID_CONNECTOR_LINE_ARROW_START);
        rSet.DisableItem(SID_CONNECTOR_LINE_ARROW_START);
        rSet.ClearItem(SID_CONNECTOR_LINE_ARROW_END);
        rSet.DisableItem(SID_CONNECTOR_LINE_ARROW_END);
        rSet.ClearItem(SID_CONNECTOR_LINE_ARROWS);
        rSet.DisableItem(SID_CONNECTOR_LINE_ARROWS);
        rSet.ClearItem(SID_CONNECTOR_LINE_CIRCLE_START);
        rSet.DisableItem(SID_CONNECTOR_LINE_CIRCLE_START);
        rSet.ClearItem(SID_CONNECTOR_LINE_CIRCLE_END);
        rSet.DisableItem(SID_CONNECTOR_LINE_CIRCLE_END);
        rSet.ClearItem(SID_CONNECTOR_LINE_CIRCLES);
        rSet.DisableItem(SID_CONNECTOR_LINE_CIRCLES);
        rSet.ClearItem(SID_CONNECTOR_CURVE);
        rSet.DisableItem(SID_CONNECTOR_CURVE);
        rSet.ClearItem(SID_CONNECTOR_CURVE_ARROW_START);
        rSet.DisableItem(SID_CONNECTOR_CURVE_ARROW_START);
        rSet.ClearItem(SID_CONNECTOR_CURVE_ARROW_END);
        rSet.DisableItem(SID_CONNECTOR_CURVE_ARROW_END);
        rSet.ClearItem(SID_CONNECTOR_CURVE_ARROWS);
        rSet.DisableItem(SID_CONNECTOR_CURVE_ARROWS);
        rSet.ClearItem(SID_CONNECTOR_CURVE_CIRCLE_START);
        rSet.DisableItem(SID_CONNECTOR_CURVE_CIRCLE_START);
        rSet.ClearItem(SID_CONNECTOR_CURVE_CIRCLE_END);
        rSet.DisableItem(SID_CONNECTOR_CURVE_CIRCLE_END);
        rSet.ClearItem(SID_CONNECTOR_CURVE_CIRCLES);
        rSet.DisableItem(SID_CONNECTOR_CURVE_CIRCLES);
        rSet.ClearItem(SID_CONNECTOR_LINES);
        rSet.DisableItem(SID_CONNECTOR_LINES);
        rSet.ClearItem(SID_CONNECTOR_LINES_ARROW_START);
        rSet.DisableItem(SID_CONNECTOR_LINES_ARROW_START);
        rSet.ClearItem(SID_CONNECTOR_LINES_ARROW_END);
        rSet.DisableItem(SID_CONNECTOR_LINES_ARROW_END);
        rSet.ClearItem(SID_CONNECTOR_LINES_ARROWS);
        rSet.DisableItem(SID_CONNECTOR_LINES_ARROWS);
        rSet.ClearItem(SID_CONNECTOR_LINES_CIRCLE_START);
        rSet.DisableItem(SID_CONNECTOR_LINES_CIRCLE_START);
        rSet.ClearItem(SID_CONNECTOR_LINES_CIRCLE_END);
        rSet.DisableItem(SID_CONNECTOR_LINES_CIRCLE_END);
        rSet.ClearItem(SID_CONNECTOR_LINES_CIRCLES);
        rSet.DisableItem(SID_CONNECTOR_LINES_CIRCLES);
        rSet.ClearItem(SID_DRAW_ARC);
        rSet.DisableItem(SID_DRAW_ARC);
        rSet.ClearItem(SID_DRAW_CIRCLEARC);
        rSet.DisableItem(SID_DRAW_CIRCLEARC);
        rSet.ClearItem(SID_DRAW_PIE);
        rSet.DisableItem(SID_DRAW_PIE);
        rSet.ClearItem(SID_DRAW_PIE_NOFILL);
        rSet.DisableItem(SID_DRAW_PIE_NOFILL);
        rSet.ClearItem(SID_DRAW_CIRCLEPIE);
        rSet.DisableItem(SID_DRAW_CIRCLEPIE);
        rSet.ClearItem(SID_DRAW_CIRCLEPIE_NOFILL);
        rSet.DisableItem(SID_DRAW_CIRCLEPIE_NOFILL);
        rSet.ClearItem(SID_DRAW_ELLIPSECUT);
        rSet.DisableItem(SID_DRAW_ELLIPSECUT);
        rSet.ClearItem(SID_DRAW_ELLIPSECUT_NOFILL);
        rSet.DisableItem(SID_DRAW_ELLIPSECUT_NOFILL);
        rSet.ClearItem(SID_DRAW_CIRCLECUT);
        rSet.DisableItem(SID_DRAW_CIRCLECUT);
        rSet.ClearItem(SID_DRAW_CIRCLECUT_NOFILL);
        rSet.DisableItem(SID_DRAW_CIRCLECUT_NOFILL);
        rSet.ClearItem(SID_DRAW_POLYGON);
        rSet.DisableItem(SID_DRAW_POLYGON);
        rSet.ClearItem(SID_DRAW_POLYGON_NOFILL);
        rSet.DisableItem(SID_DRAW_POLYGON_NOFILL);
        rSet.ClearItem(SID_DRAW_FREELINE);
        rSet.DisableItem(SID_DRAW_FREELINE);
        rSet.ClearItem(SID_DRAW_FREELINE_NOFILL);
        rSet.DisableItem(SID_DRAW_FREELINE_NOFILL);
        rSet.ClearItem(SID_DRAW_XPOLYGON);
        rSet.DisableItem(SID_DRAW_XPOLYGON);
        rSet.ClearItem(SID_DRAW_XPOLYGON_NOFILL);
        rSet.DisableItem(SID_DRAW_XPOLYGON_NOFILL);
        rSet.ClearItem(SID_DRAW_BEZIER_FILL);
        rSet.DisableItem(SID_DRAW_BEZIER_FILL);
        rSet.ClearItem(SID_DRAW_BEZIER_NOFILL);
        rSet.DisableItem(SID_DRAW_BEZIER_NOFILL);
        rSet.ClearItem(SID_3D_CUBE);
        rSet.DisableItem(SID_3D_CUBE);
        rSet.ClearItem(SID_3D_SHELL);
        rSet.DisableItem(SID_3D_SHELL);
        rSet.ClearItem(SID_3D_SPHERE);
        rSet.DisableItem(SID_3D_SPHERE);
        rSet.ClearItem(SID_3D_HALF_SPHERE);
        rSet.DisableItem(SID_3D_HALF_SPHERE);
        rSet.ClearItem(SID_3D_CYLINDER);
        rSet.DisableItem(SID_3D_CYLINDER);
        rSet.ClearItem(SID_3D_CONE);
        rSet.DisableItem(SID_3D_CONE);
        rSet.ClearItem(SID_3D_TORUS);
        rSet.DisableItem(SID_3D_TORUS);
        rSet.ClearItem(SID_3D_PYRAMID);
        rSet.DisableItem(SID_3D_PYRAMID);
    }

    // Sind die Module verfuegbar?

    if (!SvtModuleOptions().IsCalc())
    {
        rSet.DisableItem( SID_ATTR_TABLE );
    }
    if (!SvtModuleOptions().IsChart())
    {
        rSet.DisableItem( SID_INSERT_DIAGRAM );
    }
#ifdef STARIMAGE_AVAILABLE
    if (!(pApp->HasFeature(SFX_FEATURE_SIMAGE)))
    {
        rSet.DisableItem( SID_INSERT_IMAGE );
    }
#endif
    if (!SvtModuleOptions().IsMath())
    {
        rSet.DisableItem( SID_INSERT_MATH );
    }

    const SvEditObjectProtocol& rProt = GetDocSh()->GetProtocol();

    if ( pFuSlideShow || GetDocSh()->IsPreview() || bInEffectAssignment )
    {
        // Eigene Slots
        rSet.DisableItem( SID_PRESENTATION );
        rSet.DisableItem( SID_ZOOM_IN );
        rSet.DisableItem( SID_ZOOM_OUT );
        rSet.DisableItem( SID_ZOOM_PANNING );
        rSet.DisableItem( SID_ZOOM_NEXT );
        rSet.DisableItem( SID_ZOOM_PREV );
        rSet.DisableItem( SID_SIZE_REAL );
        rSet.DisableItem( SID_SIZE_OPTIMAL );
        rSet.DisableItem( SID_SIZE_ALL );
        rSet.DisableItem( SID_SIZE_PAGE_WIDTH );
        rSet.DisableItem( SID_SIZE_PAGE );
        rSet.DisableItem( SID_INSERTPAGE );
        rSet.DisableItem( SID_DUPLICATE_PAGE );
        rSet.DisableItem( SID_MODIFYPAGE );
        rSet.DisableItem( SID_RENAMEPAGE );
        rSet.DisableItem( SID_RENAMEPAGE_QUICK );
        rSet.DisableItem( SID_DELETE_PAGE );
        rSet.DisableItem( SID_PAGESETUP );

        if( pFuSlideShow || bInEffectAssignment )
        {
            if( pFuSlideShow && pFuSlideShow->IsLivePresentation())
            {
                rSet.DisableItem(SID_ANIMATION_OBJECTS);
            }
            else
            {
                rSet.ClearItem(SID_OBJECT_ALIGN);
                rSet.ClearItem(SID_ZOOM_TOOLBOX);
                rSet.ClearItem(SID_OBJECT_CHOOSE_MODE);
                rSet.ClearItem(SID_DRAWTBX_TEXT);
                rSet.ClearItem(SID_DRAWTBX_RECTANGLES);
                rSet.ClearItem(SID_DRAWTBX_ELLIPSES);
                rSet.ClearItem(SID_DRAWTBX_LINES);
                rSet.ClearItem(SID_DRAWTBX_ARROWS);
                rSet.ClearItem(SID_DRAWTBX_3D_OBJECTS);
                rSet.ClearItem(SID_DRAWTBX_CONNECTORS);
                rSet.ClearItem(SID_OBJECT_CHOOSE_MODE );
                rSet.ClearItem(SID_DRAWTBX_INSERT);
                rSet.ClearItem(SID_INSERTFILE);
                rSet.ClearItem(SID_OBJECT_ROTATE);
                rSet.ClearItem(SID_OBJECT_ALIGN);
                rSet.ClearItem(SID_POSITION);
                rSet.ClearItem(SID_FM_CONFIG);
                rSet.ClearItem(SID_ANIMATION_EFFECTS);
                rSet.ClearItem(SID_ANIMATION_OBJECTS);
                rSet.ClearItem(SID_3D_WIN);

                rSet.DisableItem(SID_OBJECT_ALIGN);
                rSet.DisableItem(SID_ZOOM_TOOLBOX);
                rSet.DisableItem(SID_OBJECT_CHOOSE_MODE);
                rSet.DisableItem(SID_DRAWTBX_TEXT);
                rSet.DisableItem(SID_DRAWTBX_RECTANGLES);
                rSet.DisableItem(SID_DRAWTBX_ELLIPSES);
                rSet.DisableItem(SID_DRAWTBX_LINES);
                rSet.DisableItem(SID_DRAWTBX_ARROWS);
                rSet.DisableItem(SID_DRAWTBX_3D_OBJECTS);
                rSet.DisableItem(SID_DRAWTBX_CONNECTORS);
                rSet.DisableItem(SID_OBJECT_CHOOSE_MODE );
                rSet.DisableItem(SID_DRAWTBX_INSERT);
                rSet.DisableItem(SID_INSERTFILE);
                rSet.DisableItem(SID_OBJECT_ROTATE);
                rSet.DisableItem(SID_OBJECT_ALIGN);
                rSet.DisableItem(SID_POSITION);
                rSet.DisableItem(SID_FM_CONFIG);
                rSet.DisableItem(SID_ANIMATION_EFFECTS);
                rSet.DisableItem(SID_ANIMATION_OBJECTS);
                rSet.DisableItem(SID_3D_WIN);
            }
        }
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_SOUND ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_VIDEO ) )
    {
        ///////////////////////////////////////////////////////////////////////
        // Menuoption : Insert->Object->Sound and Insert->Object->Video
        //              diable, if there isn't installed any appropriate plugin
        //
        if (!SvxPluginFileDlg::IsAvailable (SID_INSERT_SOUND))
            rSet.DisableItem (SID_INSERT_SOUND);
        if (!SvxPluginFileDlg::IsAvailable (SID_INSERT_VIDEO))
            rSet.DisableItem (SID_INSERT_VIDEO);
    }

    ///////////////////////////////////////////////////////////////////////
    // Menuoption: Change->Convert->To Bitmap, Change->Convert->To Metafile
    //             disable, if there only Bitmap or Metafiles marked
    // Menuoption: Format->Area, Format->Line
    //             disabled, if the marked objects not able to handle
    //             these attributes
    //
    if (!pDrView->AreObjectsMarked())
    {
        rSet.DisableItem (SID_CONVERT_TO_METAFILE);
        rSet.DisableItem (SID_CONVERT_TO_BITMAP);
    }
    else
    {
        // get marklist
        SdrMarkList aMarkList = pDrView->GetMarkedObjectList();

        BOOL bFoundBitmap         = FALSE;
        BOOL bFoundMetafile       = FALSE;
        BOOL bFoundObjNoLine      = FALSE;
        BOOL bFoundObjNoArea      = FALSE;
        BOOL bFoundNoGraphicObj = FALSE;
        BOOL bFoundAny            = FALSE;

        for (int i=0; i < (int) aMarkList.GetMarkCount() && !bFoundAny; i++)
        {
            SdrObject* pObj=  aMarkList.GetMark(i)->GetObj();
            UINT16 nId = pObj->GetObjIdentifier();
            UINT32 nInv = pObj->GetObjInventor();

            if(nInv == SdrInventor)
            {
                // 2D objects
                switch( nId )
                {
                    case OBJ_PATHLINE :
                    case OBJ_PLIN :
                    case OBJ_LINE:
                    case OBJ_FREELINE :
                    case OBJ_EDGE:
                    case OBJ_CARC :
                        bFoundObjNoArea      = TRUE;
                        bFoundNoGraphicObj = TRUE;
                        break;
                    case OBJ_OLE2 :
                        bFoundObjNoLine      = TRUE;
                        bFoundObjNoArea      = TRUE;
                        bFoundNoGraphicObj = TRUE;
                        break;
                    case OBJ_GRAF :
                        switch ( ((SdrGrafObj*)pObj)->GetGraphicType() )
                        {
                            case GRAPHIC_BITMAP :
                                bFoundBitmap = TRUE;
                                break;
                            case GRAPHIC_GDIMETAFILE :
                                bFoundMetafile = TRUE;
                                break;
                        }

                        // #i25616# bFoundObjNoLine = TRUE;
                        // #i25616# bFoundObjNoArea = TRUE;
                        break;
                    default :
                        bFoundAny = TRUE;
                }
            }
            else if(nInv == E3dInventor)
            {
                // 3D objects
                bFoundAny = TRUE;
            }
        }

        if (!bFoundAny)
        {
            // Disable menuitem for area-dialog
            if( bFoundObjNoArea ) // #i25616#
                rSet.DisableItem( SID_ATTRIBUTES_AREA );

            // Disable menuitem for line-dialog
            if( bFoundObjNoLine )
                rSet.DisableItem( SID_ATTRIBUTES_LINE );

            if( bFoundBitmap && !bFoundMetafile && !bFoundNoGraphicObj )    // only Bitmaps marked
                rSet.DisableItem( SID_CONVERT_TO_BITMAP );
            else if( !bFoundBitmap && bFoundMetafile && !bFoundNoGraphicObj )   // only Metafiles marked
                rSet.DisableItem( SID_CONVERT_TO_METAFILE );
            else if( !bFoundBitmap && !bFoundMetafile && !bFoundNoGraphicObj )  // nothing to do
            {
                rSet.DisableItem( SID_CONVERT_TO_BITMAP );
                rSet.DisableItem( SID_CONVERT_TO_METAFILE );
            }
        }
    }

    // #96090# moved SID_UNDO to ViewShell::GetMenuState()
    // #96090# moved SID_REDO to ViewShell::GetMenuState()
    // #96090# moved SID_GETUNDOSTRINGS to ViewShell::GetMenuState()
    // #96090# moved SID_GETREDOSTRINGS to ViewShell::GetMenuState()

    ///////////////////////////////////////////////////////////////////////
    // Menuoption: Edit->Hyperlink
    // Disable, if there is no hyperlink
    //
    sal_Bool bDisableEditHyperlink = sal_True;
    if( pDrView->AreObjectsMarked() && ( pDrView->GetMarkedObjectList().GetMarkCount() == 1 ) )
    {
        if( pDrView->IsTextEdit() )
        {
            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();
            if (pOLV)
            {
                const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();
                if (pFieldItem)
                {
                    ESelection aSel = pOLV->GetSelection();
                    if ( abs( aSel.nEndPos - aSel.nStartPos ) == 1 )
                    {
                        const SvxFieldData* pField = pFieldItem->GetField();
                        if ( pField->ISA(SvxURLField) )
                            bDisableEditHyperlink = sal_False;
                    }
                }
            }
        }
        else
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pDrView->GetMarkedObjectList().GetMark(0)->GetObj());

            if ( pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor() )
            {
                uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );
                if( xControlModel.is() )
                {
                    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                    if( xPropSet.is() )
                    {
                        uno::Reference< beans::XPropertySetInfo > xPropInfo( xPropSet->getPropertySetInfo() );
                        if( xPropInfo.is() && xPropInfo->hasPropertyByName(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL"))))
                        {
                            bDisableEditHyperlink = sal_False;
                        }
                    }
                }
            }
        }
    }
    if ( bDisableEditHyperlink || GetDocSh()->IsReadOnly() )
        rSet.DisableItem( SID_EDIT_HYPERLINK );

    if ( bDisableEditHyperlink )
        rSet.DisableItem( SID_OPEN_HYPERLINK );

#if defined WIN || defined WNT || defined UNX
    if( !mxScannerManager.is() )
    {
        rSet.DisableItem( SID_TWAIN_SELECT );
        rSet.DisableItem( SID_TWAIN_TRANSFER );
    }
#endif

// Fuer Win16
#ifndef SOLAR_JAVA
    rSet.DisableItem( SID_INSERT_APPLET );
#endif

    GetModeSwitchingMenuState (rSet);
}

void DrawViewShell::GetModeSwitchingMenuState (SfxItemSet &rSet)
{
    //draview
    rSet.Put(SfxBoolItem(SID_DIAMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_OUTLINEMODE, FALSE));
    if (ePageKind == PK_NOTES)
    {
        rSet.DisableItem(SID_ANIMATIONMODE);
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, FALSE));
        rSet.Put(SfxBoolItem(SID_NOTESMODE, TRUE));
        rSet.Put(SfxBoolItem(SID_HANDOUTMODE, FALSE));
    }
    else if (ePageKind == PK_HANDOUT)
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, FALSE));
        rSet.Put(SfxBoolItem(SID_NOTESMODE, FALSE));
        rSet.Put(SfxBoolItem(SID_HANDOUTMODE, TRUE));
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, TRUE));
        rSet.Put(SfxBoolItem(SID_NOTESMODE, FALSE));
        rSet.Put(SfxBoolItem(SID_HANDOUTMODE, FALSE));
    }

    // #101976# Removed [GetDocSh()->GetActualFunction() ||] from the following
    // clause because the current function of the docshell can only be
    // search and replace or spell checking and in that case switching the
    // view mode is allowed.
    if (GetDocSh()->IsInPlaceActive() || pFuSlideShow)
    {
        if ( !GetDocSh()->IsInPlaceActive() )
        {
            rSet.ClearItem( SID_DRAWINGMODE );
            rSet.DisableItem( SID_DRAWINGMODE );
        }

        rSet.ClearItem( SID_NOTESMODE );
        rSet.DisableItem( SID_NOTESMODE );

        rSet.ClearItem( SID_HANDOUTMODE );
        rSet.DisableItem( SID_HANDOUTMODE );

        rSet.ClearItem( SID_OUTLINEMODE );
        rSet.DisableItem( SID_OUTLINEMODE );

        rSet.ClearItem( SID_DIAMODE );
        rSet.DisableItem( SID_DIAMODE );
    }

    if (GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        // Outplace-Edit: Kein Umschalten erlauben
        rSet.ClearItem( SID_OUTLINEMODE );
        rSet.DisableItem( SID_OUTLINEMODE );

        rSet.ClearItem( SID_DIAMODE );
        rSet.DisableItem( SID_DIAMODE );

        rSet.ClearItem( SID_NOTESMODE );
        rSet.DisableItem( SID_NOTESMODE );

        rSet.ClearItem( SID_HANDOUTMODE );
        rSet.DisableItem( SID_HANDOUTMODE );
    }


    svx::ExtrusionBar::getState( pDrView, rSet );
    svx::FontworkBar::getState( pDrView, rSet );
}



void DrawViewShell::GetState (SfxItemSet& rSet)
{
    // Iterate over all requested items in the set.
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_SEARCH_ITEM:
            case SID_SEARCH_OPTIONS:
                // Forward this request to the the common (old) code of the
                // document shell.
                GetDocSh()->GetState (rSet);
                break;
            default:
                OSL_TRACE ("DrawViewShell::GetState(): can not handle which id %d", nWhich);
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




void DrawViewShell::Execute (SfxRequest& rReq)
{
    FuSlideShow* pFuSlideShow = GetSlideShow();
    if (pFuSlideShow!=NULL && !pFuSlideShow->IsLivePresentation())
    {
        // Do not execute anything during a native slide show.
        return;
    }

    switch (rReq.GetSlot())
    {
        case SID_SEARCH_ITEM:
            // Forward this request to the the common (old) code of the
            // document shell.
            GetDocSh()->Execute (rReq);
        break;

        default:
            OSL_TRACE ("DrawViewShell::Execute(): can not handle slot %d", rReq.GetSlot());
            break;
    }
}

} // end of namespace sd
