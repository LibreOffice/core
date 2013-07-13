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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <svx/fmglob.hxx>
#include <svx/globl3d.hxx>
#include <svx/svdouno.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/outlobj.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Impress.hxx>
#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/fmshell.hxx>
#include <svl/eitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdograf.hxx>
#include <editeng/unolingu.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>

// #UndoRedo#
#include <svl/slstitm.hxx>
#include <sfx2/app.hxx>
#include <svtools/insdlg.hxx>
#include <unotools/moduleoptions.hxx>
#include <svl/languageoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/request.hxx>

#include <svx/pfiledlg.hxx>
#include <svx/grafctrl.hxx>
#include <svtools/cliplistener.hxx>
#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "PresentationViewShell.hxx"

#include "Outliner.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "Client.hxx"
#include "DrawDocShell.hxx"
#include "zoomlist.hxx"
#include "slideshow.hxx"
#include "drawview.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "LayerTabBar.hxx"
#include "fupoor.hxx"
#include "Window.hxx"
#include "fuediglu.hxx"
#include "fubullet.hxx"
#include "fuformatpaintbrush.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

/** Create a list of clipboard formats that are supported both from the
    current clipboard content and the DrawViewShell.
    The list is stored in a new instance of SvxClipboardFmtItem.
*/
SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<SvxClipboardFmtItem> GetSupportedClipboardFormats (
    TransferableDataHelper& rDataHelper)
{
    ::std::auto_ptr<SvxClipboardFmtItem> pResult (
        new SvxClipboardFmtItem(SID_CLIPBOARD_FORMAT_ITEMS));

    sal_uInt32 nFormatCount (rDataHelper.GetFormatCount());
    for (sal_uInt32 i=0; i<nFormatCount; i++)
    {
        const SotFormatStringId nTestFormat = rDataHelper.GetFormat(i);

        // Check if the current format is the same as one that has already
        // been handled.
        bool bDuplicate (false);
        for (sal_uInt32 j=0; j<i; j++)
        {
            if (nTestFormat == rDataHelper.GetFormat(j))
            {
                bDuplicate = true;
                break;
            }
        }

        // Look up the format among those that are supported by the
        // DrawViewShell.
        if ( ! bDuplicate)
        {
            switch (nTestFormat)
            {
                case SOT_FORMATSTR_ID_EMBED_SOURCE:
                {
                    String sName;

                    TransferableObjectDescriptor aDescriptor;
                    if (rDataHelper.GetTransferableObjectDescriptor(
                        SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDescriptor))
                        {
                            sName = aDescriptor.maTypeName;
                        }
                    if (sName.Len() > 0)
                        pResult->AddClipbrdFormat(nTestFormat, sName);
                    else
                        pResult->AddClipbrdFormat(nTestFormat);

                    break;
                }

                case SOT_FORMATSTR_ID_LINK_SOURCE:
                case SOT_FORMATSTR_ID_DRAWING:
                case SOT_FORMATSTR_ID_SVXB:
                case FORMAT_GDIMETAFILE:
                case FORMAT_BITMAP:
                case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
                case FORMAT_STRING:
                case SOT_FORMATSTR_ID_HTML:
                case FORMAT_RTF:
                case SOT_FORMATSTR_ID_EDITENGINE:
                    pResult->AddClipbrdFormat(nTestFormat);
                    break;
            }
        }
    }

    // Check some OLE formats whose names are handled differently.
    SotFormatStringId nFormat (SOT_FORMATSTR_ID_EMBED_SOURCE_OLE);
    bool bHasFormat (rDataHelper.HasFormat(nFormat));
    if ( ! bHasFormat)
    {
        bHasFormat = rDataHelper.HasFormat(nFormat);
    }
    if (bHasFormat)
    {
        String sName;
        String sSource;
        if (SvPasteObjectHelper::GetEmbeddedName (rDataHelper, sName, sSource, nFormat))
            pResult->AddClipbrdFormat (nFormat, sName);
    }

    return pResult;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

namespace sd {

IMPL_LINK( DrawViewShell, ClipboardChanged, TransferableDataHelper*, pDataHelper )
{
    if ( pDataHelper )
    {
        mbPastePossible = ( pDataHelper->GetFormatCount() != 0 );

        // Update the list of supported clipboard formats according to the
        // new clipboard content.
        // There are some stack traces that indicate the possibility of the
        // DrawViewShell destructor called during the call to
        // GetSupportedClipboardFormats().  If that really has happened then
        // exit immediately.
        TransferableDataHelper aDataHelper (
            TransferableDataHelper::CreateFromSystemClipboard(GetActiveWindow()));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SvxClipboardFmtItem> pFormats (GetSupportedClipboardFormats(aDataHelper));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if (mpDrawView == NULL)
            return 0;
        mpCurrentClipboardFormats = pFormats;

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PASTE );
        rBindings.Invalidate( SID_PASTE_SPECIAL );
        rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    return 0;
}

void DrawViewShell::GetDrawAttrState(SfxItemSet& rSet)
{
    SfxItemSet aSet( mpDrawView->GetGeoAttrFromMarked() );
    rSet.Put(aSet,sal_False);
}

void DrawViewShell::GetMenuState( SfxItemSet &rSet )
{
    if (mpDrawView == NULL)
    {
        // This assertion and return are here to prevent crashes.
        DBG_ASSERT(mpDrawView!=NULL, "Please report this assertion to the Impress team.");
        return;
    }

    ViewShell::GetMenuState(rSet);
    sal_Bool bDisableVerticalText = !SvtLanguageOptions().IsVerticalTextEnabled();

    if ( bDisableVerticalText )
    {
        rSet.DisableItem( SID_DRAW_FONTWORK_VERTICAL );
        rSet.DisableItem( SID_DRAW_CAPTION_VERTICAL );
        rSet.DisableItem( SID_TEXT_FITTOSIZE_VERTICAL );
        rSet.DisableItem( SID_DRAW_TEXT_VERTICAL );
    }

    bool bConvertToPathPossible = mpDrawView->IsConvertToPathObjPossible(sal_False);

    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
    const sal_uLong nMarkCount = rMarkList.GetMarkCount();

    //format paintbrush
    FuFormatPaintBrush::GetMenuState( *this, rSet );

    // State of SfxChild-Windows (Animator, Fontwork etc.)
    SetChildWindowState( rSet );

    // map images of toolboxes (only zoom)
    UpdateToolboxImages( rSet, sal_False );

    if(HasCurrentFunction())
    {
        sal_uInt16 nSId = GetCurrentFunction()->GetSlotID();

        rSet.Put( SfxBoolItem( nSId, sal_True ) );

        // will cause a uncheck of a simulated slot
        sal_uInt16 nId = GetIdBySubId( nSId );

        // map images of the toolboxes
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

    SdrPageView* pPageView = mpDrawView->GetSdrPageView();

    GetMenuStateSel(rSet);

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_ASSIGN_LAYOUT))
    {
        bool bDisable = true;
        if( pPageView )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

            if( pPage && !pPage->IsMasterPage() )
            {
                rSet.Put( SfxUInt32Item( SID_ASSIGN_LAYOUT, static_cast< sal_uInt32 >(pPage->GetAutoLayout()) ) );
                bDisable = false;
            }
        }

        if(bDisable)
        {
            rSet.DisableItem(SID_ASSIGN_LAYOUT);
        }
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        bool bDisable = true;
        if( pPageView )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

            if( pPage && (pPage->GetPageKind() == PK_STANDARD) && !pPage->IsMasterPage() )
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);

                if (pObj!=NULL )
                {
                    if( !pObj->IsEmptyPresObj() )
                    {
                        bDisable = false;
                    }
                    else
                    {
                        // check if the object is in edit, than its temporarely not empty
                        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                        if( pTextObj )
                        {
                            OutlinerParaObject* pParaObj = pTextObj->GetEditOutlinerParaObject();
                            if( pParaObj )
                            {
                                delete pParaObj;
                                bDisable = false;
                            }
                        }
                    }
                }
            }
        }

        if(bDisable)
        {
            rSet.DisableItem(SID_EXPAND_PAGE);
        }
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        bool bDisable = true;
        if( pPageView )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

            if( pPage && (pPage->GetPageKind() == PK_STANDARD) && !pPage->IsMasterPage() )
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_TITLE);

                if(pObj && !pObj->IsEmptyPresObj())
                {
                    bDisable = false;
                }
            }
        }

        if(bDisable)
        {
            rSet.DisableItem(SID_SUMMARY_PAGE);
        }
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_ASSIGN_LAYOUT))
    {
        bool bDisable = true;
        if( pPageView )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

            if( pPage && !pPage->IsMasterPage() )
            {
                rSet.Put( SfxUInt32Item(SID_ASSIGN_LAYOUT, pPage->GetAutoLayout()) );
                bDisable = false;
            }
        }

        if(bDisable)
        {
            rSet.DisableItem(SID_ASSIGN_LAYOUT);
        }
    }

    // is it possible to start the presentation?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_REHEARSE_TIMINGS ) )
    {
        sal_Bool bDisable = sal_True;
        sal_uInt16 nCount = GetDoc()->GetSdPageCount( PK_STANDARD );

        for( sal_uInt16 i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if( !pPage->IsExcluded() )
                bDisable = sal_False;
        }

        if( bDisable || GetDocSh()->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
            rSet.DisableItem( SID_REHEARSE_TIMINGS );
        }
    }

    // glue points
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
        // percent
        TRISTATE eState = mpDrawView->IsMarkedGluePointsPercent();
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_PERCENT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_PERCENT, eState == STATE_CHECK ) );

        // alignment has no effect by percent
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
            // horizontal alignment
            sal_uInt16 nHorz = mpDrawView->GetMarkedGluePointsAlign( sal_False );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_CENTER, nHorz == SDRHORZALIGN_CENTER ) );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_LEFT,   nHorz == SDRHORZALIGN_LEFT ) );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_RIGHT,  nHorz == SDRHORZALIGN_RIGHT ) );
            // vertical alignment
            sal_uInt16 nVert = mpDrawView->GetMarkedGluePointsAlign( sal_True );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_CENTER, nVert == SDRVERTALIGN_CENTER ) );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_TOP,    nVert == SDRVERTALIGN_TOP ) );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_BOTTOM, nVert == SDRVERTALIGN_BOTTOM ) );
        }

        // insert point
        rSet.Put( SfxBoolItem( SID_GLUE_INSERT_POINT, mpDrawView->IsInsGluePointMode() ) );

        // Escape direction
        // left
        eState = mpDrawView->IsMarkedGluePointsEscDir( SDRESC_LEFT );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_LEFT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_LEFT, eState == STATE_CHECK ) );
        // right
        eState = mpDrawView->IsMarkedGluePointsEscDir( SDRESC_RIGHT );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_RIGHT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_RIGHT, eState == STATE_CHECK ) );
        // top
        eState = mpDrawView->IsMarkedGluePointsEscDir( SDRESC_TOP );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_TOP );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_TOP, eState == STATE_CHECK ) );
        // bottom
        eState = mpDrawView->IsMarkedGluePointsEscDir( SDRESC_BOTTOM );
        if( eState == STATE_DONTKNOW )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_BOTTOM );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_BOTTOM, eState == STATE_CHECK ) );
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GRID_FRONT ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_HELPLINES_FRONT ) )
    {
        rSet.Put( SfxBoolItem( SID_GRID_FRONT, mpDrawView->IsGridFront() ) );
        rSet.Put( SfxBoolItem( SID_HELPLINES_FRONT, mpDrawView->IsHlplFront() ) );
    }

    if (!mpDrawView->IsFrameDragSingles())
        rSet.Put(SfxBoolItem(SID_BEZIER_EDIT, sal_True));
    else
        rSet.Put(SfxBoolItem(SID_BEZIER_EDIT, sal_False));

    if(dynamic_cast<FuEditGluePoints*>( GetCurrentFunction().get()))
        rSet.Put(SfxBoolItem(SID_GLUE_EDITMODE, sal_True));
    else
        rSet.Put(SfxBoolItem(SID_GLUE_EDITMODE, sal_False));

    if( !mpDrawView->IsMirrorAllowed( sal_True, sal_True ) )
    {
        rSet.DisableItem( SID_HORIZONTAL );
        rSet.DisableItem( SID_VERTICAL );
        rSet.DisableItem( SID_FLIP_HORIZONTAL );
        rSet.DisableItem( SID_FLIP_VERTICAL );
    }

    if( !mpDrawView->IsMirrorAllowed() )
    {
        rSet.DisableItem( SID_OBJECT_MIRROR );
//        rSet.DisableItem( SID_CONVERT_TO_3D_LATHE );
//        rSet.DisableItem( SID_CONVERT_TO_3D_LATHE_FAST );
    }

    // interactive transparence control
    if(!mpDrawView->IsTransparenceAllowed())
    {
        rSet.DisableItem( SID_OBJECT_TRANSPARENCE );
    }

    // interactive gradient control
    if(!mpDrawView->IsGradientAllowed())
    {
        rSet.DisableItem( SID_OBJECT_GRADIENT );
    }

    // disable morphing if necessary
    if ( !mpDrawView->IsMorphingAllowed() )
        rSet.DisableItem( SID_POLYGON_MORPHING );

    // disable vectorizing if necessary
    if ( !mpDrawView->IsVectorizeAllowed() )
        rSet.DisableItem( SID_VECTORIZE );

    if( !mpDrawView->IsReverseOrderPossible() )
    {
        rSet.DisableItem( SID_REVERSE_ORDER );
    }

    if ( !bConvertToPathPossible &&
         !mpDrawView->IsCrookAllowed( mpDrawView->IsCrookNoContortion() ) )
    {
        // implicit transformation into curve not possible
        rSet.DisableItem(SID_OBJECT_CROOK_ROTATE);
        rSet.DisableItem(SID_OBJECT_CROOK_SLANT);
        rSet.DisableItem(SID_OBJECT_CROOK_STRETCH);
    }

    if ( !mpDrawView->IsGroupEntered() )
    {
        rSet.DisableItem( SID_LEAVE_GROUP );
        rSet.Put( SfxBoolItem( SID_LEAVE_ALL_GROUPS, sal_False ) );
        rSet.ClearItem( SID_LEAVE_ALL_GROUPS );
        rSet.DisableItem( SID_LEAVE_ALL_GROUPS );
    }
    else
        rSet.Put( SfxBoolItem( SID_LEAVE_ALL_GROUPS, sal_True ) );

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_THESAURUS ) )
    {
        if ( !mpDrawView->IsTextEdit() )
        {
            rSet.DisableItem( SID_THESAURUS );
        }
        else
        {
            LanguageType            eLang = GetDoc()->GetLanguage( EE_CHAR_LANGUAGE );
            Reference< XThesaurus > xThesaurus( LinguMgr::GetThesaurus() );

            if (!xThesaurus.is() || eLang == LANGUAGE_NONE || !xThesaurus->hasLocale( LanguageTag::convertToLocale( eLang)) )
                rSet.DisableItem( SID_THESAURUS );
        }
    }

    if ( !mpDrawView->IsTextEdit() )
    {
        rSet.DisableItem( SID_THESAURUS );
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SELECTALL ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SIZE_ALL ) )
    {
        if( pPageView && pPageView->GetObjList()->GetObjCount() == 0 )
        {
            // should be disabled if there is no object on the draw area:
            rSet.DisableItem( SID_SELECTALL );
            rSet.DisableItem( SID_SIZE_ALL );
        }
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONTEXT ) )
        rSet.Put( SfxStringItem( SID_CONTEXT, mpDrawView->GetStatusText() ) );

    // clipboard (paste)
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PASTE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PASTE_SPECIAL ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) )
    {
        if ( !mpClipEvtLstnr )
        {
            // avoid clipboard initialization for
            // read-only presentation views (workaround for NT4.0
            // clipboard prob...)
            if( !ISA(PresentationViewShell) )
            {
                // create listener
                mpClipEvtLstnr = new TransferableClipboardListener( LINK( this, DrawViewShell, ClipboardChanged ) );
                mpClipEvtLstnr->acquire();
                mpClipEvtLstnr->AddRemoveListener( GetActiveWindow(), sal_True );

                // get initial state
                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
                mbPastePossible = ( aDataHelper.GetFormatCount() != 0 );
            }
            else
                mbPastePossible = sal_False;
        }

        if( !mbPastePossible )
        {
            rSet.DisableItem( SID_PASTE );
            rSet.DisableItem( SID_PASTE_SPECIAL );
            rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );
        }
        else if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) )
        {
            if (mpCurrentClipboardFormats.get() != NULL)
                rSet.Put(*mpCurrentClipboardFormats);
        }
    }

    if ( !bConvertToPathPossible )
    {
        rSet.DisableItem(SID_CHANGEBEZIER);
    }

    if (mpDrawView == NULL)
    {
        // The mpDrawView was not NULL but is now.
        // The reason for this may be that the DrawViewShell has been
        // destroyed in the mean time.
        // We can only return immediately and hope that the deleted
        // DrawViewShell is not called again.
        DBG_ASSERT(mpDrawView!=NULL, "Please report this assertion to the Impress team.");
        return;
    }

    if( !( mpDrawView->IsConvertToPolyObjPossible(sal_False) || mpDrawView->IsVectorizeAllowed() ) )
        rSet.DisableItem(SID_CHANGEPOLYGON);

    if( !( mpDrawView->IsConvertToPolyObjPossible(sal_False) || mpDrawView->IsConvertToContourPossible() ) )
        rSet.DisableItem(SID_CONVERT_TO_CONTOUR);

    if ( !mpDrawView->IsConvertTo3DObjPossible() )
    {
        rSet.DisableItem(SID_CONVERT_TO_3D);
        rSet.DisableItem(SID_CONVERT_TO_3D_LATHE);
        rSet.DisableItem(SID_CONVERT_TO_3D_LATHE_FAST);
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MANAGE_LINKS ) )
    {
        if ( GetDoc()->GetLinkCount() == 0 )
        {
            rSet.DisableItem(SID_MANAGE_LINKS);
        }
    }

    if (mePageKind == PK_HANDOUT)
    {
        rSet.DisableItem(SID_PRESENTATION_LAYOUT);
        rSet.DisableItem(SID_SELECT_BACKGROUND);
    }

    if (mePageKind == PK_NOTES)
    {
        rSet.DisableItem(SID_INSERTPAGE);
        rSet.DisableItem(SID_RENAMEPAGE);
        rSet.DisableItem(SID_RENAMEPAGE_QUICK);
        rSet.DisableItem(SID_DUPLICATE_PAGE);
        rSet.ClearItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_EFFECTS);

        if (meEditMode == EM_MASTERPAGE)
            rSet.DisableItem(SID_MODIFYPAGE);

        rSet.DisableItem(SID_SELECT_BACKGROUND);
        rSet.DisableItem(SID_INSERTLAYER);
        rSet.DisableItem(SID_LAYERMODE);
        rSet.DisableItem(SID_INSERTFILE);
    }
    else if (mePageKind == PK_HANDOUT)
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
        rSet.DisableItem(SID_SELECT_BACKGROUND);
    }
    else
    {
        if (meEditMode == EM_MASTERPAGE)
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

    if (meEditMode == EM_PAGE)
    {
        /**********************************************************************
        * page mode
        **********************************************************************/
        rSet.Put(SfxBoolItem(SID_PAGEMODE, sal_True));
        rSet.Put(SfxBoolItem(SID_MASTERPAGE, sal_False));
        rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, sal_False));
        rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, sal_False));
        rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, sal_False));

        if (mePageKind == PK_STANDARD &&
            rSet.GetItemState(SID_TITLE_MASTERPAGE) == SFX_ITEM_AVAILABLE)
        {
            // Is there a page with the AutoLayout "Title"?
            sal_Bool bDisable = sal_True;
            sal_uInt16 i = 0;
            sal_uInt16 nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

            while (i < nCount && bDisable)
            {
                SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

                if (pPage->GetAutoLayout() == AUTOLAYOUT_TITLE)
                {
                    bDisable = sal_False;
                }

                i++;
            }

            if (bDisable)
            {
                rSet.DisableItem(SID_TITLE_MASTERPAGE);
            }
            else
            {
                rSet.Put(SfxBoolItem(SID_TITLE_MASTERPAGE, sal_False));
            }
        }
        else
        {
            rSet.DisableItem(SID_TITLE_MASTERPAGE);
        }

        rSet.DisableItem (SID_INSERT_MASTER_PAGE);
        rSet.DisableItem (SID_DELETE_MASTER_PAGE);
        rSet.DisableItem (SID_RENAME_MASTER_PAGE);
        rSet.DisableItem (SID_CLOSE_MASTER_VIEW);
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_PAGEMODE, sal_False));
        rSet.Put(SfxBoolItem(SID_MASTERPAGE, sal_True));

        /**********************************************************************
        * Background page mode
        **********************************************************************/
        if (mePageKind == PK_STANDARD)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, sal_True));
            rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, sal_False));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, sal_False));

            if (rSet.GetItemState(SID_TITLE_MASTERPAGE) == SFX_ITEM_AVAILABLE)
            {
                sal_Bool bCheck = sal_False;
                sal_Bool bDisable = sal_True;
                if( pPageView )
                {
                    SdPage* pMPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

                    sal_uInt16 i = 0;
                    sal_uInt16 nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

                    // Is there a reference to the current master page from a page
                    // with the AutoLayout "Title"?
                    while (i < nCount && !bCheck && bDisable)
                    {
                        SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

                        // page does reference the current master page
                        if (pPage->GetAutoLayout() == AUTOLAYOUT_TITLE)
                        {
                            // a page does have a AutoLayout "Title"
                            bDisable = sal_False;

                            SdPage& rRefMPage = (SdPage&)(pPage->TRG_GetMasterPage());

                            if(&rRefMPage == pMPage)
                            {
                                // a page with the AutoLayout "Title" does reference
                                // the current master page
                                bCheck = sal_True;
                            }
                        }

                        i++;
                    }
                }

                if (bCheck)
                {
                    rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, sal_False));
                }

                rSet.Put(SfxBoolItem(SID_TITLE_MASTERPAGE, bCheck));

                if (bDisable)
                {
                    rSet.ClearItem(SID_TITLE_MASTERPAGE);
                    rSet.DisableItem(SID_TITLE_MASTERPAGE);
                }
            }
        }
        else if (mePageKind == PK_NOTES)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, sal_False));
            rSet.DisableItem(SID_TITLE_MASTERPAGE);
            rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, sal_True));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, sal_False));
        }
        else if (mePageKind == PK_HANDOUT)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTERPAGE, sal_False));
            rSet.DisableItem(SID_TITLE_MASTERPAGE);
            rSet.Put(SfxBoolItem(SID_NOTES_MASTERPAGE, sal_False));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTERPAGE, sal_True));
        }
    }

    // set state of the ruler
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_RULER ) )
        rSet.Put( SfxBoolItem( SID_RULER, HasRuler() ) );

    // do not delete the last page or a master page
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DELETE_PAGE )
        || SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DELETE_MASTER_PAGE ) )
    {
        if (maTabControl.GetPageCount() == 1 ||
            meEditMode == EM_MASTERPAGE     ||
            mePageKind == PK_NOTES          ||
            mePageKind == PK_HANDOUT        ||
            (GetShellType()!=ST_DRAW&&IsLayerModeActive()))
        {
            if (rSet.GetItemState(SID_DELETE_PAGE) == SFX_ITEM_AVAILABLE)
                rSet.DisableItem(SID_DELETE_PAGE);
            if (rSet.GetItemState(SID_DELETE_MASTER_PAGE)==SFX_ITEM_AVAILABLE)
                rSet.DisableItem(SID_DELETE_MASTER_PAGE);
        }
    }

    // is it allowed to delete the current layer?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DELETE_LAYER ) )
    {
        sal_uInt16        nCurrentLayer = GetLayerTabControl()->GetCurPageId();
        const String& rName         = GetLayerTabControl()->GetPageText(nCurrentLayer);

        sal_Bool bDisableIt = !IsLayerModeActive();
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
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTLINE_BULLET ))
    {
        OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

        // special treatment of for SID_OUTLINE_BULLET if objects with different
        // kinds of NumBullets are marked
        sal_Bool bHasOutliner = sal_False;
        sal_Bool bHasOther    = sal_False;
        for(sal_uLong nNum = 0; nNum < nMarkCount; nNum++)
        {
            SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor )
            {
                if( pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
                {
                    bHasOutliner = sal_True;
                    if(bHasOther)
                        break;
                }
                else
                {
                    bHasOther = sal_True;
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

    }

    FuBullet::GetSlotState( rSet, this, GetViewFrame() );

    if ( GetDocSh()->IsUIActive() )
    {
        rSet.DisableItem( SID_INSERT_OBJECT );
        rSet.DisableItem( SID_INSERT_PLUGIN );
        rSet.DisableItem( SID_INSERT_SOUND );
        rSet.DisableItem( SID_INSERT_VIDEO );
        rSet.DisableItem( SID_INSERT_FLOATINGFRAME );
        rSet.DisableItem( SID_INSERT_MATH );
        rSet.DisableItem( SID_INSERT_DIAGRAM );
        rSet.DisableItem( SID_ATTR_TABLE );
        rSet.DisableItem( SID_SIZE_REAL );
        rSet.DisableItem( SID_SIZE_OPTIMAL );
        rSet.DisableItem( SID_SIZE_ALL );
        rSet.DisableItem( SID_SIZE_PAGE_WIDTH );
        rSet.DisableItem( SID_SIZE_PAGE );
        rSet.DisableItem( SID_DUPLICATE_PAGE );
        rSet.DisableItem( SID_ZOOM_TOOLBOX );
    }

    // Zoom-State
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

    if (!mpZoomList->IsNextPossible())
    {
       rSet.DisableItem(SID_ZOOM_NEXT);
    }
    if (!mpZoomList->IsPreviousPossible())
    {
       rSet.DisableItem(SID_ZOOM_PREV);
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_REMOTE_DLG ) )
    {

        bool bDisableSdremoteForGood = false;
#ifndef ENABLE_SDREMOTE
        bDisableSdremoteForGood = true;
#endif
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        if ( xContext.is() )
            bDisableSdremoteForGood |= ! ( officecfg::Office::Common::Misc::ExperimentalMode::get( xContext ) &&

                                           officecfg::Office::Impress::Misc::Start::EnableSdremote::get( xContext ) );

        // This dialog is only useful for TCP/IP remote control
        // which is unusual, under-tested and a security issue.
        if ( bDisableSdremoteForGood )
        {
            rSet.Put(SfxVisibilityItem(SID_REMOTE_DLG, false));
        }
    }

    // EditText active
    if (GetViewShellBase().GetViewShellManager()->GetShell(RID_DRAW_TEXT_TOOLBOX) != NULL)
    {
        sal_uInt16 nCurrentSId = SID_ATTR_CHAR;

        if(HasCurrentFunction())
        {
            nCurrentSId = GetCurrentFunction()->GetSlotID();
        }
        if( nCurrentSId != SID_TEXT_FITTOSIZE &&
            nCurrentSId != SID_TEXT_FITTOSIZE_VERTICAL &&
            nCurrentSId != SID_ATTR_CHAR_VERTICAL )
            nCurrentSId = SID_ATTR_CHAR;

        rSet.Put( SfxBoolItem( nCurrentSId, sal_True ) );

        // Short version of UpdateToolboxImages()
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
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, sal_True));
        }
        else
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, sal_False));
        }
    }

    SdrPageView* pPV = mpDrawView->GetSdrPageView();
    String aActiveLayer = mpDrawView->GetActiveLayer();

    if ( ( aActiveLayer.Len() != 0 && pPV && ( pPV->IsLayerLocked(aActiveLayer) ||
          !pPV->IsLayerVisible(aActiveLayer) ) ) ||
          SD_MOD()->GetWaterCan() )
    {
        rSet.DisableItem( SID_PASTE );
        rSet.DisableItem( SID_PASTE_SPECIAL );
        rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );

        rSet.DisableItem( SID_INSERT_FLD_DATE_FIX );
        rSet.DisableItem( SID_INSERT_FLD_DATE_VAR );
        rSet.DisableItem( SID_INSERT_FLD_TIME_FIX );
        rSet.DisableItem( SID_INSERT_FLD_TIME_VAR );
        rSet.DisableItem( SID_INSERT_FLD_AUTHOR );
        rSet.DisableItem( SID_INSERT_FLD_PAGE );
        rSet.DisableItem( SID_INSERT_FLD_PAGES );
        rSet.DisableItem( SID_INSERT_FLD_FILE );

        rSet.DisableItem( SID_INSERT_GRAPHIC );
        rSet.DisableItem( SID_INSERT_AVMEDIA );
        rSet.DisableItem( SID_INSERT_DIAGRAM );
        rSet.DisableItem( SID_INSERT_OBJECT );
        rSet.DisableItem( SID_INSERT_PLUGIN );
        rSet.DisableItem( SID_INSERT_SOUND );
        rSet.DisableItem( SID_INSERT_VIDEO );
        rSet.DisableItem( SID_INSERT_FLOATINGFRAME );

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
        rSet.DisableItem( SID_OBJECT_CROP );
        rSet.DisableItem( SID_ATTR_GRAF_CROP );
        rSet.DisableItem( SID_OBJECT_TRANSPARENCE );
        rSet.DisableItem( SID_OBJECT_GRADIENT );
        rSet.DisableItem( SID_OBJECT_CROOK_ROTATE );
        rSet.DisableItem( SID_OBJECT_CROOK_SLANT );
        rSet.DisableItem( SID_OBJECT_CROOK_STRETCH );

        // Disable all object-creating tools
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

    // are the modules available?

    if (!SvtModuleOptions().IsCalc())
    {
        // remove menu entry if module is not available
        rSet.Put( SfxVisibilityItem( SID_ATTR_TABLE, sal_False ) );
    }
    if (!SvtModuleOptions().IsChart())
    {
        rSet.DisableItem( SID_INSERT_DIAGRAM );
    }
    if (!SvtModuleOptions().IsMath())
    {
        rSet.DisableItem( SID_INSERT_MATH );
    }

    rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( (xSlideshow.is() && xSlideshow->isRunning() && (xSlideshow->getAnimationMode() != ANIMATIONMODE_PREVIEW) ) || GetDocSh()->IsPreview() )
    {
        // Own Slots
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

        if( xSlideshow.is() && xSlideshow->isRunning() )
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

    ///////////////////////////////////////////////////////////////////////
    // Menuoption: Change->Convert->To Bitmap, Change->Convert->To Metafile
    //             disable, if there only Bitmap or Metafiles marked
    // Menuoption: Format->Area, Format->Line
    //             disabled, if the marked objects not able to handle
    //             these attributes
    //

    bool bSingleGraphicSelected = false;

    if (!mpDrawView->AreObjectsMarked())
    {
        rSet.DisableItem (SID_CONVERT_TO_METAFILE);
        rSet.DisableItem (SID_CONVERT_TO_BITMAP);
    }
    else
    {
        // get marklist
        SdrMarkList aMarkList = mpDrawView->GetMarkedObjectList();

        sal_Bool bFoundBitmap         = sal_False;
        sal_Bool bFoundMetafile       = sal_False;
        sal_Bool bFoundObjNoLine      = sal_False;
        sal_Bool bFoundObjNoArea      = sal_False;
        sal_Bool bFoundNoGraphicObj = sal_False;
        sal_Bool bFoundAny            = sal_False;
        bool bFoundTable = false;

//      const int nMarkCount = (int) aMarkList.GetMarkCount();
        for (sal_uLong i=0; i < nMarkCount && !bFoundAny; i++)
        {
            SdrObject* pObj=  aMarkList.GetMark(i)->GetMarkedSdrObj();
            sal_uInt16 nId = pObj->GetObjIdentifier();
            sal_uInt32 nInv = pObj->GetObjInventor();

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
                        bFoundObjNoArea      = sal_True;
                        bFoundNoGraphicObj = sal_True;
                        break;
                    case OBJ_OLE2 :
                        // #i118485# #i118525# Allow Line, Area and Graphic (Metafile, Bitmap)
                        bSingleGraphicSelected = nMarkCount == 1;
                        bFoundBitmap = true;
                        bFoundMetafile = true;
                        break;
                    case OBJ_GRAF :
                    {
                        bSingleGraphicSelected = nMarkCount == 1;
                        const SdrGrafObj* pSdrGrafObj = static_cast< const SdrGrafObj* >(pObj);
                        switch(pSdrGrafObj->GetGraphicType())
                        {
                            case GRAPHIC_BITMAP :
                                bFoundBitmap = sal_True;
                                if(pSdrGrafObj->isEmbeddedSvg())
                                {
                                    bFoundMetafile = true;
                                }
                                break;
                            case GRAPHIC_GDIMETAFILE :
                                bFoundMetafile = sal_True;
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case OBJ_TABLE:
                        bFoundTable = true;
                        break;
                    default :
                        bFoundAny = sal_True;
                }
            }
            else if(nInv == E3dInventor)
            {
                // 3D objects
                bFoundAny = sal_True;
            }
        }

        if( bFoundTable )
            rSet.DisableItem( SID_ATTRIBUTES_LINE );

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

    if( !bSingleGraphicSelected )
    {
        rSet.DisableItem (SID_OBJECT_CROP);
        rSet.DisableItem (SID_ATTR_GRAF_CROP);
    }

    ///////////////////////////////////////////////////////////////////////
    // Menuoption: Edit->Hyperlink
    // Disable, if there is no hyperlink
    //
    sal_Bool bDisableEditHyperlink = sal_True;
    if( mpDrawView->AreObjectsMarked() && ( mpDrawView->GetMarkedObjectList().GetMarkCount() == 1 ) )
    {
        if( mpDrawView->IsTextEdit() )
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
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
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj());

            if ( pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor() )
            {
                uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );
                if( xControlModel.is() )
                {
                    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                    if( xPropSet.is() )
                    {
                        uno::Reference< beans::XPropertySetInfo > xPropInfo( xPropSet->getPropertySetInfo() );
                        if( xPropInfo.is() && xPropInfo->hasPropertyByName( "TargetURL") )
                        {
                            bDisableEditHyperlink = sal_False;
                        }
                    }
                }
            }
        }
    }

    //highlight selected custom shape
    {
        sal_uInt16 nCurrentSId = 0;
        if(HasCurrentFunction())
            nCurrentSId = GetCurrentFunction()->GetSlotID();

        if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DRAWTBX_CS_BASIC ) )
            rSet.Put(SfxBoolItem(SID_DRAWTBX_CS_BASIC, SID_DRAWTBX_CS_BASIC == nCurrentSId ));
        if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DRAWTBX_CS_SYMBOL ) )
            rSet.Put(SfxBoolItem(SID_DRAWTBX_CS_SYMBOL, SID_DRAWTBX_CS_SYMBOL == nCurrentSId ));
        if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DRAWTBX_CS_ARROW ) )
            rSet.Put(SfxBoolItem(SID_DRAWTBX_CS_ARROW, SID_DRAWTBX_CS_ARROW == nCurrentSId ));
        if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DRAWTBX_CS_FLOWCHART ) )
            rSet.Put(SfxBoolItem(SID_DRAWTBX_CS_FLOWCHART, SID_DRAWTBX_CS_FLOWCHART == nCurrentSId ));
        if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DRAWTBX_CS_CALLOUT ) )
            rSet.Put(SfxBoolItem(SID_DRAWTBX_CS_CALLOUT,SID_DRAWTBX_CS_CALLOUT == nCurrentSId ));
        if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DRAWTBX_CS_STAR ) )
            rSet.Put(SfxBoolItem(SID_DRAWTBX_CS_STAR, SID_DRAWTBX_CS_STAR == nCurrentSId ));
    }

    if ( bDisableEditHyperlink || GetDocSh()->IsReadOnly() )
        rSet.DisableItem( SID_EDIT_HYPERLINK );

    if ( bDisableEditHyperlink )
        rSet.DisableItem( SID_OPEN_HYPERLINK );

#if defined WNT || defined UNX
    if( !mxScannerManager.is() )
    {
        rSet.DisableItem( SID_TWAIN_SELECT );
        rSet.DisableItem( SID_TWAIN_TRANSFER );
    }
#endif

    // Set the state of two entries in the 'Slide' context sub-menu
    // concerning the visibility of master page background and master page
    // shapes.
    if (rSet.GetItemState(SID_DISPLAY_MASTER_BACKGROUND) == SFX_ITEM_AVAILABLE
        || rSet.GetItemState(SID_DISPLAY_MASTER_OBJECTS) == SFX_ITEM_AVAILABLE)
    {
        SdPage* pPage = GetActualPage();
        if (pPage != NULL
            && GetDoc() != NULL)
        {
            SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt8 aBackgroundId = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
            sal_uInt8 aObjectId = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);
            rSet.Put(SfxBoolItem(SID_DISPLAY_MASTER_BACKGROUND,
                    aVisibleLayers.IsSet(aBackgroundId)));
            rSet.Put(SfxBoolItem(SID_DISPLAY_MASTER_OBJECTS,
                    aVisibleLayers.IsSet(aObjectId)));
        }
    }

    GetModeSwitchingMenuState (rSet);
}

void DrawViewShell::GetModeSwitchingMenuState (SfxItemSet &rSet)
{
    //draview
    rSet.Put(SfxBoolItem(SID_DIAMODE, sal_False));
    rSet.Put(SfxBoolItem(SID_OUTLINEMODE, sal_False));
    if (mePageKind == PK_NOTES)
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, sal_False));
        rSet.Put(SfxBoolItem(SID_NOTESMODE, sal_True));
        rSet.Put(SfxBoolItem(SID_HANDOUTMODE, sal_False));
    }
    else if (mePageKind == PK_HANDOUT)
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, sal_False));
        rSet.Put(SfxBoolItem(SID_NOTESMODE, sal_False));
        rSet.Put(SfxBoolItem(SID_HANDOUTMODE, sal_True));
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, sal_True));
        rSet.Put(SfxBoolItem(SID_NOTESMODE, sal_False));
        rSet.Put(SfxBoolItem(SID_HANDOUTMODE, sal_False));
    }

    // Removed [GetDocSh()->GetCurrentFunction() ||] from the following
    // clause because the current function of the docshell can only be
    // search and replace or spell checking and in that case switching the
    // view mode is allowed.
    const bool bIsRunning = SlideShow::IsRunning(GetViewShellBase());

    if (GetViewFrame()->GetFrame().IsInPlace() || bIsRunning)
    {
        if ( !GetViewFrame()->GetFrame().IsInPlace() )
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
        // Outplace-Edit: do not allow switch
        rSet.ClearItem( SID_OUTLINEMODE );
        rSet.DisableItem( SID_OUTLINEMODE );

        rSet.ClearItem( SID_DIAMODE );
        rSet.DisableItem( SID_DIAMODE );

        rSet.ClearItem( SID_NOTESMODE );
        rSet.DisableItem( SID_NOTESMODE );

        rSet.ClearItem( SID_HANDOUTMODE );
        rSet.DisableItem( SID_HANDOUTMODE );
    }

    svx::ExtrusionBar::getState( mpDrawView, rSet );
    svx::FontworkBar::getState( mpDrawView, rSet );
}



void DrawViewShell::GetState (SfxItemSet& rSet)
{
    // Iterate over all requested items in the set.
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_SEARCH_ITEM:
            case SID_SEARCH_OPTIONS:
                // Forward this request to the common (old) code of the
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
    if(SlideShow::IsRunning(GetViewShellBase()))
    {
        // Do not execute anything during a native slide show.
        return;
    }

    switch (rReq.GetSlot())
    {
        case SID_SEARCH_ITEM:
            // Forward this request to the common (old) code of the
            // document shell.
            GetDocSh()->Execute (rReq);
        break;

        case SID_SPELL_DIALOG:
        {
            SfxViewFrame* pViewFrame = GetViewFrame();
            if (rReq.GetArgs() != NULL)
                pViewFrame->SetChildWindow (SID_SPELL_DIALOG,
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_SPELL_DIALOG))).GetValue());
            else
                pViewFrame->ToggleChildWindow(SID_SPELL_DIALOG);

            pViewFrame->GetBindings().Invalidate(SID_SPELL_DIALOG);
            rReq.Ignore ();
        }
        break;

        default:
            OSL_TRACE ("DrawViewShell::Execute(): can not handle slot %d", rReq.GetSlot());
            break;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
