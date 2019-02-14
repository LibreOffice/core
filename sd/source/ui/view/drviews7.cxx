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

#include <memory>

#include <sal/config.h>
#include <sal/log.hxx>

#include <utility>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <svx/fmglob.hxx>
#include <svx/globl3d.hxx>
#include <svx/rulritem.hxx>
#include <svx/svdouno.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Impress.hxx>
#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/fmshell.hxx>
#include <svl/eitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdograf.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
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

#include <svx/grafctrl.hxx>
#include <svtools/cliplistener.hxx>
#include <sfx2/viewfrm.hxx>

#include <app.hrc>
#include <strings.hrc>

#include <PresentationViewShell.hxx>

#include <Outliner.hxx>
#include <drawdoc.hxx>
#include <DrawViewShell.hxx>
#include <sdmod.hxx>
#include <unokywds.hxx>
#include <sdpage.hxx>
#include <Client.hxx>
#include <DrawDocShell.hxx>
#include <zoomlist.hxx>
#include <slideshow.hxx>
#include <drawview.hxx>
#include <View.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <LayerTabBar.hxx>
#include <fupoor.hxx>
#include <Window.hxx>
#include <fuediglu.hxx>
#include <fubullet.hxx>
#include <fuconcs.hxx>
#include <fuformatpaintbrush.hxx>
#include <stlsheet.hxx>

#include <config_features.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

/** Create a list of clipboard formats that are supported both from the
    current clipboard content and the DrawViewShell.
    The list is stored in a new instance of SvxClipboardFormatItem.
*/
static ::std::unique_ptr<SvxClipboardFormatItem> GetSupportedClipboardFormats (
    TransferableDataHelper& rDataHelper)
{
    ::std::unique_ptr<SvxClipboardFormatItem> pResult (
        new SvxClipboardFormatItem(SID_CLIPBOARD_FORMAT_ITEMS));

    sal_uInt32 nFormatCount (rDataHelper.GetFormatCount());
    for (sal_uInt32 i=0; i<nFormatCount; i++)
    {
        const SotClipboardFormatId nTestFormat = rDataHelper.GetFormat(i);

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
                case SotClipboardFormatId::EMBED_SOURCE:
                {
                    OUString sName;

                    TransferableObjectDescriptor aDescriptor;
                    if (rDataHelper.GetTransferableObjectDescriptor(
                        SotClipboardFormatId::OBJECTDESCRIPTOR, aDescriptor))
                    {
                        sName = aDescriptor.maTypeName;
                    }
                    if (!sName.isEmpty())
                        pResult->AddClipbrdFormat(nTestFormat, sName);
                    else
                        pResult->AddClipbrdFormat(nTestFormat);

                    break;
                }


                case SotClipboardFormatId::LINK_SOURCE:
                case SotClipboardFormatId::DRAWING:
                case SotClipboardFormatId::SVXB:
                case SotClipboardFormatId::GDIMETAFILE:
                case SotClipboardFormatId::BITMAP:
                case SotClipboardFormatId::NETSCAPE_BOOKMARK:
                case SotClipboardFormatId::STRING:
                case SotClipboardFormatId::HTML:
                case SotClipboardFormatId::RTF:
                case SotClipboardFormatId::RICHTEXT:
                case SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT:
                    pResult->AddClipbrdFormat(nTestFormat);
                    break;
                default: break;
            }
        }
    }

    // Check some OLE formats whose names are handled differently.
    SotClipboardFormatId nFormat (SotClipboardFormatId::EMBED_SOURCE_OLE);
    bool bHasFormat (rDataHelper.HasFormat(nFormat));
    if ( ! bHasFormat)
    {
        bHasFormat = rDataHelper.HasFormat(nFormat);
    }
    if (bHasFormat)
    {
        OUString sName;
        OUString sSource;
        if (SvPasteObjectHelper::GetEmbeddedName (rDataHelper, sName, sSource, nFormat))
            pResult->AddClipbrdFormat (nFormat, sName);
    }

    return pResult;
}

namespace sd {

IMPL_LINK( DrawViewShell, ClipboardChanged, TransferableDataHelper*, pDataHelper, void )
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
    ::std::unique_ptr<SvxClipboardFormatItem> pFormats (GetSupportedClipboardFormats(aDataHelper));
    if (mpDrawView == nullptr)
        return;
    mpCurrentClipboardFormats = std::move(pFormats);

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );
    rBindings.Invalidate( SID_PASTE_UNFORMATTED );
    rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
}

void DrawViewShell::GetDrawAttrState(SfxItemSet& rSet)
{
    SfxItemSet aSet( mpDrawView->GetGeoAttrFromMarked() );
    rSet.Put(aSet,false);
}

::Outliner* DrawViewShell::GetOutlinerForMasterPageOutlineTextObj(ESelection &rSel)
{
    if( !mpDrawView )
        return nullptr;

    //when there is one object selected
    if (!mpDrawView->AreObjectsMarked() || (mpDrawView->GetMarkedObjectList().GetMarkCount() != 1))
        return nullptr;

    //and we are editing the outline object
    if (!mpDrawView->IsTextEdit())
        return nullptr;

    SdrPageView* pPageView = mpDrawView->GetSdrPageView();
    if (!pPageView)
        return nullptr;

    SdPage* pPage = static_cast<SdPage*>(pPageView->GetPage());
    //only show these in a normal master page
    if (!pPage || (pPage->GetPageKind() != PageKind::Standard) || !pPage->IsMasterPage())
        return nullptr;

    OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
    ::Outliner* pOL = pOLV ? pOLV->GetOutliner() : nullptr;
    if (!pOL)
        return nullptr;
    rSel = pOLV->GetSelection();

    return pOL;
}

void DrawViewShell::GetMarginProperties( SfxItemSet &rSet )
{
    SdPage *pPage = getCurrentPage();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_ATTR_PAGE_LRSPACE:
            {
                // const SvxLRSpaceItem aTmpPageLRSpace ( rDesc.GetMaster().GetLRSpace() );
                const SvxLongLRSpaceItem aLongLR(
                    static_cast<long>(pPage->GetLeftBorder()),
                    static_cast<long>(pPage->GetRightBorder()),
                    SID_ATTR_PAGE_LRSPACE );
                rSet.Put( aLongLR );
            }
            break;

            case SID_ATTR_PAGE_ULSPACE:
            {
                // const SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
                SvxLongULSpaceItem aLongUL(
                    static_cast<long>(pPage->GetUpperBorder()),
                    static_cast<long>(pPage->GetLowerBorder()),
                    SID_ATTR_PAGE_ULSPACE );
                rSet.Put( aLongUL );
            }
            break;

            default:
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void DrawViewShell::GetMenuState( SfxItemSet &rSet )
{
    if (mpDrawView == nullptr)
    {
        // This assertion and return are here to prevent crashes.
        DBG_ASSERT(mpDrawView!=nullptr, "Please report this assertion to the Impress team.");
        return;
    }

    ViewShell::GetMenuState(rSet);
    bool bDisableVerticalText = !SvtLanguageOptions().IsVerticalTextEnabled();

    if ( bDisableVerticalText )
    {
        rSet.DisableItem( SID_DRAW_FONTWORK_VERTICAL );
        rSet.DisableItem( SID_DRAW_CAPTION_VERTICAL );
        rSet.DisableItem( SID_TEXT_FITTOSIZE_VERTICAL );
        rSet.DisableItem( SID_DRAW_TEXT_VERTICAL );
    }

    bool bConvertToPathPossible = mpDrawView->IsConvertToPathObjPossible();

    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();

    //format paintbrush
    FuFormatPaintBrush::GetMenuState( *this, rSet );

    // State of SfxChild-Windows (Animator, Fontwork etc.)
    SetChildWindowState( rSet );

    if(HasCurrentFunction())
    {
        sal_uInt16 nSId = GetCurrentFunction()->GetSlotID();
        rSet.Put( SfxBoolItem( nSId, true ) );
    }

    SdrPageView* pPageView = mpDrawView->GetSdrPageView();

    GetMenuStateSel(rSet);

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_ASSIGN_LAYOUT))
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

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        bool bDisable = true;
        if( pPageView )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

            if( pPage && (pPage->GetPageKind() == PageKind::Standard) && !pPage->IsMasterPage() )
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);

                if (pObj!=nullptr )
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
                            std::unique_ptr<OutlinerParaObject> pParaObj = pTextObj->GetEditOutlinerParaObject();
                            if( pParaObj )
                            {
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

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        bool bDisable = true;
        if( pPageView )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pPageView->GetPage() );

            if( pPage && (pPage->GetPageKind() == PageKind::Standard) && !pPage->IsMasterPage() )
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

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_ASSIGN_LAYOUT))
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
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_PRESENTATION ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_REHEARSE_TIMINGS ) )
    {
        bool bDisable = true;
        sal_uInt16 nCount = GetDoc()->GetSdPageCount( PageKind::Standard );

        for( sal_uInt16 i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PageKind::Standard);

            if( !pPage->IsExcluded() )
                bDisable = false;
        }

        if( bDisable || GetDocSh()->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
            rSet.DisableItem( SID_REHEARSE_TIMINGS );
        }
    }

    // glue points
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_EDITMODE ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_INSERT_POINT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_PERCENT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_ESCDIR ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_ESCDIR_LEFT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_ESCDIR_RIGHT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_ESCDIR_TOP ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_ESCDIR_BOTTOM ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_HORZALIGN_CENTER ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_HORZALIGN_LEFT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_HORZALIGN_RIGHT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_VERTALIGN_CENTER ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_VERTALIGN_TOP ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_GLUE_VERTALIGN_BOTTOM ) )
    {
        // percent
        TriState eState = mpDrawView->IsMarkedGluePointsPercent();
        if( eState == TRISTATE_INDET )
            rSet.InvalidateItem( SID_GLUE_PERCENT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_PERCENT, eState == TRISTATE_TRUE ) );

        // alignment has no effect by percent
        if( eState == TRISTATE_TRUE )
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
            SdrAlign nHorz = mpDrawView->GetMarkedGluePointsAlign( false );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_CENTER, nHorz == SdrAlign::HORZ_CENTER ) );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_LEFT,   nHorz == SdrAlign::HORZ_LEFT ) );
            rSet.Put( SfxBoolItem( SID_GLUE_HORZALIGN_RIGHT,  nHorz == SdrAlign::HORZ_RIGHT ) );
            // vertical alignment
            SdrAlign nVert = mpDrawView->GetMarkedGluePointsAlign( true );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_CENTER, nVert == SdrAlign::VERT_CENTER ) );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_TOP,    nVert == SdrAlign::VERT_TOP ) );
            rSet.Put( SfxBoolItem( SID_GLUE_VERTALIGN_BOTTOM, nVert == SdrAlign::VERT_BOTTOM ) );
        }

        // insert point
        rSet.Put( SfxBoolItem( SID_GLUE_INSERT_POINT, mpDrawView->IsInsGluePointMode() ) );

        // Escape direction
        // left
        eState = mpDrawView->IsMarkedGluePointsEscDir( SdrEscapeDirection::LEFT );
        if( eState == TRISTATE_INDET )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_LEFT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_LEFT, eState == TRISTATE_TRUE ) );
        // right
        eState = mpDrawView->IsMarkedGluePointsEscDir( SdrEscapeDirection::RIGHT );
        if( eState == TRISTATE_INDET )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_RIGHT );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_RIGHT, eState == TRISTATE_TRUE ) );
        // top
        eState = mpDrawView->IsMarkedGluePointsEscDir( SdrEscapeDirection::TOP );
        if( eState == TRISTATE_INDET )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_TOP );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_TOP, eState == TRISTATE_TRUE ) );
        // bottom
        eState = mpDrawView->IsMarkedGluePointsEscDir( SdrEscapeDirection::BOTTOM );
        if( eState == TRISTATE_INDET )
            rSet.InvalidateItem( SID_GLUE_ESCDIR_BOTTOM );
        else
            rSet.Put( SfxBoolItem( SID_GLUE_ESCDIR_BOTTOM, eState == TRISTATE_TRUE ) );
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_GRID_FRONT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_HELPLINES_FRONT ) )
    {
        rSet.Put( SfxBoolItem( SID_GRID_FRONT, mpDrawView->IsGridFront() ) );
        rSet.Put( SfxBoolItem( SID_HELPLINES_FRONT, mpDrawView->IsHlplFront() ) );
    }

    if (!mpDrawView->IsFrameDragSingles())
        rSet.Put(SfxBoolItem(SID_BEZIER_EDIT, true));
    else
        rSet.Put(SfxBoolItem(SID_BEZIER_EDIT, false));

    if(dynamic_cast<FuEditGluePoints*>( GetCurrentFunction().get()))
        rSet.Put(SfxBoolItem(SID_GLUE_EDITMODE, true));
    else
        rSet.Put(SfxBoolItem(SID_GLUE_EDITMODE, false));

    if( !mpDrawView->IsMirrorAllowed( true, true ) )
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
        rSet.Put( SfxBoolItem( SID_LEAVE_ALL_GROUPS, false ) );
        rSet.ClearItem( SID_LEAVE_ALL_GROUPS );
        rSet.DisableItem( SID_LEAVE_ALL_GROUPS );
    }
    else
        rSet.Put( SfxBoolItem( SID_LEAVE_ALL_GROUPS, true ) );

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_THESAURUS ) )
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

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_SELECTALL ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_SIZE_ALL ) )
    {
        if( pPageView && pPageView->GetObjList()->GetObjCount() == 0 )
        {
            // should be disabled if there is no object on the draw area:
            rSet.DisableItem( SID_SELECTALL );
            rSet.DisableItem( SID_SIZE_ALL );
        }
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_CONTEXT ) )
        rSet.Put( SfxStringItem( SID_CONTEXT, mpDrawView->GetStatusText() ) );

    // clipboard (paste)
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_PASTE ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_PASTE_SPECIAL ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_PASTE_UNFORMATTED ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) )
    {
        if ( !mxClipEvtLstnr.is() )
        {
            // avoid clipboard initialization for
            // read-only presentation views (workaround for NT4.0
            // clipboard prob...)
            if( dynamic_cast< const PresentationViewShell *>( this ) ==  nullptr )
            {
                // create listener
                mxClipEvtLstnr = new TransferableClipboardListener( LINK( this, DrawViewShell, ClipboardChanged ) );
                mxClipEvtLstnr->AddListener( GetActiveWindow() );

                // get initial state
                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
                mbPastePossible = ( aDataHelper.GetFormatCount() != 0 );
                mpCurrentClipboardFormats = GetSupportedClipboardFormats( aDataHelper );
            }
            else
                mbPastePossible = false;
        }

        if( !mbPastePossible )
        {
            rSet.DisableItem( SID_PASTE );
            rSet.DisableItem( SID_PASTE_SPECIAL );
            rSet.DisableItem( SID_PASTE_UNFORMATTED );
            rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );
        }
        else if( SfxItemState::DEFAULT == rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) )
        {
            if (mpCurrentClipboardFormats != nullptr)
                rSet.Put(*mpCurrentClipboardFormats);
        }
    }

    if ( !bConvertToPathPossible )
    {
        rSet.DisableItem(SID_CHANGEBEZIER);
    }

    if (mpDrawView == nullptr)
    {
        // The mpDrawView was not NULL but is now.
        // The reason for this may be that the DrawViewShell has been
        // destroyed in the mean time.
        // We can only return immediately and hope that the deleted
        // DrawViewShell is not called again.
        DBG_ASSERT(mpDrawView!=nullptr, "Please report this assertion to the Impress team.");
        return;
    }

    if( !( mpDrawView->IsConvertToPolyObjPossible() || mpDrawView->IsVectorizeAllowed() ) )
        rSet.DisableItem(SID_CHANGEPOLYGON);

    if( !( mpDrawView->IsConvertToPolyObjPossible() || mpDrawView->IsConvertToContourPossible() ) )
        rSet.DisableItem(SID_CONVERT_TO_CONTOUR);

    if ( !mpDrawView->IsConvertTo3DObjPossible() )
    {
        rSet.DisableItem(SID_CONVERT_TO_3D);
        rSet.DisableItem(SID_CONVERT_TO_3D_LATHE);
        rSet.DisableItem(SID_CONVERT_TO_3D_LATHE_FAST);
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_MANAGE_LINKS ) )
    {
        if ( GetDoc()->GetLinkCount() == 0 )
        {
            rSet.DisableItem(SID_MANAGE_LINKS);
        }
    }

    if (mePageKind == PageKind::Handout)
    {
        rSet.DisableItem(SID_PRESENTATION_LAYOUT);
        rSet.DisableItem(SID_SELECT_BACKGROUND);
        rSet.DisableItem(SID_SAVE_BACKGROUND);
    }

    if (mePageKind == PageKind::Notes)
    {
        rSet.DisableItem(SID_INSERTPAGE);
        rSet.DisableItem(SID_RENAMEPAGE);
        rSet.DisableItem(SID_RENAMEPAGE_QUICK);
        rSet.DisableItem(SID_DUPLICATE_PAGE);
        rSet.ClearItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_OBJECTS);
        rSet.DisableItem(SID_ANIMATION_EFFECTS);

        if (meEditMode == EditMode::MasterPage)
            rSet.DisableItem(SID_MODIFYPAGE);

        rSet.DisableItem(SID_SELECT_BACKGROUND);
        rSet.DisableItem(SID_SAVE_BACKGROUND);
        rSet.DisableItem(SID_INSERTLAYER);
        rSet.DisableItem(SID_LAYERMODE);
        rSet.DisableItem(SID_INSERTFILE);
    }
    else if (mePageKind == PageKind::Handout)
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
        rSet.DisableItem(SID_SAVE_BACKGROUND);
    }
    else
    {
        if (meEditMode == EditMode::MasterPage)
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

    if (meEditMode == EditMode::Page)
    {
        /**********************************************************************
        * page mode
        **********************************************************************/
        rSet.Put(SfxBoolItem(SID_PAGEMODE, true));
        rSet.Put(SfxBoolItem(SID_MASTERPAGE, false));
        rSet.Put(SfxBoolItem(SID_SLIDE_MASTER_MODE, false));
        rSet.Put(SfxBoolItem(SID_NOTES_MASTER_MODE, false));
        rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, false));

        rSet.DisableItem (SID_INSERT_MASTER_PAGE);
        rSet.DisableItem (SID_DELETE_MASTER_PAGE);
        rSet.DisableItem (SID_RENAME_MASTER_PAGE);
        rSet.DisableItem (SID_CLOSE_MASTER_VIEW);
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_PAGEMODE, false));
        rSet.Put(SfxBoolItem(SID_MASTERPAGE, true));

        /**********************************************************************
        * Background page mode
        **********************************************************************/
        if (mePageKind == PageKind::Standard)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTER_MODE, true));
            rSet.Put(SfxBoolItem(SID_NOTES_MASTER_MODE, false));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, false));

        }
        else if (mePageKind == PageKind::Notes)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTER_MODE, false));
            rSet.Put(SfxBoolItem(SID_NOTES_MASTER_MODE, true));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, false));
        }
        else if (mePageKind == PageKind::Handout)
        {
            rSet.Put(SfxBoolItem(SID_SLIDE_MASTER_MODE, false));
            rSet.Put(SfxBoolItem(SID_NOTES_MASTER_MODE, false));
            rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, true));
        }
    }

    // set state of the ruler
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_RULER ) )
        rSet.Put( SfxBoolItem( SID_RULER, HasRuler() ) );

    // do not delete the last page or a master page
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_DELETE_PAGE )
        || SfxItemState::DEFAULT == rSet.GetItemState( SID_DELETE_MASTER_PAGE ) )
    {
        if (maTabControl->GetPageCount() == 1 ||
            meEditMode == EditMode::MasterPage     ||
            mePageKind == PageKind::Notes          ||
            mePageKind == PageKind::Handout        ||
            (GetShellType()!=ST_DRAW&&IsLayerModeActive()))
        {
            if (rSet.GetItemState(SID_DELETE_PAGE) == SfxItemState::DEFAULT)
                rSet.DisableItem(SID_DELETE_PAGE);
            if (rSet.GetItemState(SID_DELETE_MASTER_PAGE)==SfxItemState::DEFAULT)
                rSet.DisableItem(SID_DELETE_MASTER_PAGE);
        }
    }

    // is it allowed to delete the current layer?
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_DELETE_LAYER )
        || SfxItemState::DEFAULT == rSet.GetItemState( SID_RENAMELAYER ) )
    {
        if(GetLayerTabControl()) // #i87182#
        {
            sal_uInt16 nCurrentLayer = GetLayerTabControl()->GetCurPageId();
            const OUString& rName = GetLayerTabControl()->GetLayerName(nCurrentLayer);

            if (!IsLayerModeActive() || LayerTabBar::IsRealNameOfStandardLayer(rName))
            {
                rSet.DisableItem(SID_DELETE_LAYER);
                rSet.DisableItem(SID_RENAMELAYER);
            }
        }
        else
        {
            OSL_ENSURE(false, "No LayerTabBar (!)");
        }
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_CUT ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_COPY ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_OUTLINE_BULLET ))
    {
        OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

        // special treatment of for SID_OUTLINE_BULLET if objects with different
        // kinds of NumBullets are marked
        bool bHasOutliner = false;
        bool bHasOther    = false;
        for(size_t nNum = 0; nNum < nMarkCount; ++nNum)
        {
            SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
            if( pObj->GetObjInventor() == SdrInventor::Default )
            {
                if( pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
                {
                    bHasOutliner = true;
                    if(bHasOther)
                        break;
                }
                else
                {
                    bHasOther = true;
                    if(bHasOutliner)
                        break;
                }
            }
        }

        if( bHasOther && bHasOutliner )
            rSet.DisableItem( SID_OUTLINE_BULLET );

        if (pOlView)
        {
            if (pOlView->GetSelected().isEmpty())
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
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_ZOOM_IN ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_ZOOM_OUT )||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_ZOOM_PANNING ) )
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

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_REMOTE_DLG ) )
    {

        bool bDisableSdremoteForGood = false;
#ifndef ENABLE_SDREMOTE
        bDisableSdremoteForGood = true;
#endif
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        if ( xContext.is() )
            bDisableSdremoteForGood |= ! ( /*officecfg::Office::Common::Misc::ExperimentalMode::get( xContext ) &&*/

                                           officecfg::Office::Impress::Misc::Start::EnableSdremote::get( xContext ) );

        // This dialog is only useful for TCP/IP remote control
        // which is unusual, under-tested and a security issue.
        if ( bDisableSdremoteForGood )
        {
            rSet.Put(SfxVisibilityItem(SID_REMOTE_DLG, false));
        }
    }

    // EditText active
    if (GetViewShellBase().GetViewShellManager()->GetShell(ToolbarId::Draw_Text_Toolbox_Sd) != nullptr)
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

        rSet.Put( SfxBoolItem( nCurrentSId, true ) );
    }

    if ( GetDocSh()->IsReadOnly() )
    {
        rSet.DisableItem( SID_AUTOSPELL_CHECK );
    }
    else
    {
        if (GetDoc()->GetOnlineSpell())
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, true));
        }
        else
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, false));
        }
    }

    SdrPageView* pPV = mpDrawView->GetSdrPageView();
    OUString aActiveLayer = mpDrawView->GetActiveLayer();

    if ( ( !aActiveLayer.isEmpty() && pPV && ( pPV->IsLayerLocked(aActiveLayer) ||
          !pPV->IsLayerVisible(aActiveLayer) ) ) ||
          SD_MOD()->GetWaterCan() )
    {
        rSet.DisableItem( SID_PASTE );
        rSet.DisableItem( SID_PASTE_SPECIAL );
        rSet.DisableItem( SID_PASTE_UNFORMATTED );
        rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );

        rSet.DisableItem( SID_INSERT_FLD_DATE_FIX );
        rSet.DisableItem( SID_INSERT_FLD_DATE_VAR );
        rSet.DisableItem( SID_INSERT_FLD_TIME_FIX );
        rSet.DisableItem( SID_INSERT_FLD_TIME_VAR );
        rSet.DisableItem( SID_INSERT_FLD_AUTHOR );
        rSet.DisableItem( SID_INSERT_FLD_PAGE );
        rSet.DisableItem( SID_INSERT_FLD_PAGE_TITLE );
        rSet.DisableItem( SID_INSERT_FLD_PAGES );
        rSet.DisableItem( SID_INSERT_FLD_FILE );

        rSet.DisableItem( SID_INSERT_GRAPHIC );
        rSet.DisableItem( SID_INSERT_AVMEDIA );
        rSet.DisableItem( SID_INSERT_DIAGRAM );
        rSet.DisableItem( SID_INSERT_OBJECT );
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
        rSet.Put( SfxVisibilityItem( SID_ATTR_TABLE, false ) );
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
        rSet.DisableItem( SID_ZOOM_MODE );
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
            rSet.ClearItem(SID_INSERTFILE);
            rSet.ClearItem(SID_OBJECT_ROTATE);
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
            rSet.DisableItem(SID_POSITION);
            rSet.DisableItem(SID_FM_CONFIG);
            rSet.DisableItem(SID_ANIMATION_EFFECTS);
            rSet.DisableItem(SID_ANIMATION_OBJECTS);
            rSet.DisableItem(SID_3D_WIN);
        }
    }

    // Menuoption: Change->Convert->To Bitmap, Change->Convert->To Metafile
    //             disable, if there only Bitmap or Metafiles marked
    // Menuoption: Format->Area, Format->Line
    //             disabled, if the marked objects not able to handle
    //             these attributes

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

        bool bFoundBitmap         = false;
        bool bFoundMetafile       = false;
        bool bFoundObjNoArea      = false;
        bool bFoundNoGraphicObj = false;
        bool bFoundAny            = false;
        bool bFoundTable = false;

//      const size_t nMarkCount = aMarkList.GetMarkCount();
        for (size_t i=0; i < nMarkCount && !bFoundAny; ++i)
        {
            SdrObject*  pObj = aMarkList.GetMark(i)->GetMarkedSdrObj();
            sal_uInt16  nId  = pObj->GetObjIdentifier();
            SdrInventor nInv = pObj->GetObjInventor();

            if(nInv == SdrInventor::Default)
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
                        bFoundObjNoArea      = true;
                        bFoundNoGraphicObj = true;
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

                        // Current size of the OBJ_GRAF
                        const ::tools::Rectangle aRect = pObj->GetLogicRect();
                        const Size aCurrentSizeofObj = aRect.GetSize();

                        // Original size of the OBJ_GRAF
                        const Size aOriginalSizeofObj = pSdrGrafObj->getOriginalSize();

                        if(aCurrentSizeofObj == aOriginalSizeofObj )
                            rSet.DisableItem(SID_ORIGINAL_SIZE);

                        switch(pSdrGrafObj->GetGraphicType())
                        {
                            case GraphicType::Bitmap :
                                bFoundBitmap = true;
                                if(pSdrGrafObj->isEmbeddedVectorGraphicData())
                                {
                                    bFoundMetafile = true;
                                }
                                break;
                            case GraphicType::GdiMetafile :
                                bFoundMetafile = true;
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
                        bFoundAny = true;
                }
            }
            else if(nInv == SdrInventor::E3d)
            {
                // 3D objects
                bFoundAny = true;
            }
        }

        if( bFoundTable )
            rSet.DisableItem( SID_ATTRIBUTES_LINE );

        if (!bFoundAny)
        {
            // Disable menuitem for area-dialog
            if( bFoundObjNoArea ) // #i25616#
                rSet.DisableItem( SID_ATTRIBUTES_AREA );

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

    // Menuoption: Edit->Hyperlink
    // Disable, if there is no hyperlink

    bool bDisableEditHyperlink = true;
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
                        if ( dynamic_cast< const SvxURLField *>( pField ) !=  nullptr )
                            bDisableEditHyperlink = false;
                    }
                }
            }
        }
        else
        {
            SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj() );

            if ( pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor() )
            {
                const uno::Reference< awt::XControlModel >& xControlModel( pUnoCtrl->GetUnoControlModel() );
                if( xControlModel.is() )
                {
                    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                    if( xPropSet.is() )
                    {
                        uno::Reference< beans::XPropertySetInfo > xPropInfo( xPropSet->getPropertySetInfo() );
                        if( xPropInfo.is() && xPropInfo->hasPropertyByName( "TargetURL") )
                        {
                            bDisableEditHyperlink = false;
                        }
                    }
                }
            }
        }
    }

    //highlight selected custom shape
    {
        if(HasCurrentFunction())
        {
            rtl::Reference< FuPoor > xFunc( GetCurrentFunction() );
            FuConstructCustomShape* pShapeFunc = dynamic_cast< FuConstructCustomShape* >( xFunc.get() );

            static const sal_uInt16 nCSTbArray[] = { SID_DRAWTBX_CS_BASIC, SID_DRAWTBX_CS_SYMBOL,
                                                     SID_DRAWTBX_CS_ARROW, SID_DRAWTBX_CS_FLOWCHART,
                                                     SID_DRAWTBX_CS_CALLOUT, SID_DRAWTBX_CS_STAR };

            const sal_uInt16 nCurrentSId = GetCurrentFunction()->GetSlotID();
            for (sal_uInt16 i : nCSTbArray)
            {
                rSet.ClearItem( i ); // Why is this necessary?
                rSet.Put( SfxStringItem( i, nCurrentSId == i && pShapeFunc
                                         ? pShapeFunc->GetShapeType() : OUString() ) );
            }
        }
    }

    if ( bDisableEditHyperlink || GetDocSh()->IsReadOnly() )
        rSet.DisableItem( SID_EDIT_HYPERLINK );

    if ( bDisableEditHyperlink )
        rSet.DisableItem( SID_OPEN_HYPERLINK );

    //fdo#78151 enable show next level/hide last level if editing a master page
    //PRESOBJ_OUTLINE object and the current selection allow that to happen
    {
        bool bDisableShowNextLevel = true;
        bool bDisableHideLastLevel = true;

        ESelection aSel;
        ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
        if (pOL)
        {
            //and are on the last paragraph
            aSel.Adjust();
            if (aSel.nEndPara == pOL->GetParagraphCount() - 1)
            {
                sal_uInt16 nDepth = pOL->GetDepth(aSel.nEndPara);
                if (nDepth != sal_uInt16(-1))
                {
                    //there exists another numbering level that
                    //is currently hidden
                    if (nDepth < 8)
                        bDisableShowNextLevel = false;
                    //there exists a previous numbering level
                    if (nDepth > 0)
                        bDisableHideLastLevel = false;
                }
            }
        }

        if (bDisableShowNextLevel)
            rSet.DisableItem(SID_SHOW_NEXT_LEVEL);

        if (bDisableHideLastLevel)
            rSet.DisableItem(SID_HIDE_LAST_LEVEL);
    }

#if defined(_WIN32) || defined UNX
    if( !mxScannerManager.is() )
    {
        rSet.DisableItem( SID_TWAIN_SELECT );
        rSet.DisableItem( SID_TWAIN_TRANSFER );
    }
#endif

    // Set the state of two entries in the 'Slide' context sub-menu
    // concerning the visibility of master page background and master page
    // shapes.
    if (rSet.GetItemState(SID_DISPLAY_MASTER_BACKGROUND) == SfxItemState::DEFAULT
        || rSet.GetItemState(SID_DISPLAY_MASTER_OBJECTS) == SfxItemState::DEFAULT)
    {
        SdPage* pPage = GetActualPage();
        if (pPage != nullptr && GetDoc() != nullptr)
        {
            SdrLayerIDSet aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            SdrLayerID aBackgroundId = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
            SdrLayerID aObjectId = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
            rSet.Put(SfxBoolItem(SID_DISPLAY_MASTER_BACKGROUND,
                    aVisibleLayers.IsSet(aBackgroundId)));
            rSet.Put(SfxBoolItem(SID_DISPLAY_MASTER_OBJECTS,
                    aVisibleLayers.IsSet(aObjectId)));
        }
    }

    if (rSet.GetItemState(SID_SAVE_BACKGROUND) == SfxItemState::DEFAULT)
    {
        bool bDisableSaveBackground = true;
        SdPage* pPage = GetActualPage();
        if (pPage != nullptr && GetDoc() != nullptr)
        {
            SfxItemSet aMergedAttr(GetDoc()->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
            SdStyleSheet* pStyleSheet = pPage->getPresentationStyle(HID_PSEUDOSHEET_BACKGROUND);
            MergePageBackgroundFilling(pPage, pStyleSheet, meEditMode == EditMode::MasterPage, aMergedAttr);
            if (drawing::FillStyle_BITMAP == aMergedAttr.Get(XATTR_FILLSTYLE).GetValue())
            {
                bDisableSaveBackground = false;
            }
        }
        if (bDisableSaveBackground)
            rSet.DisableItem(SID_SAVE_BACKGROUND);
    }

    GetModeSwitchingMenuState (rSet);
}

void DrawViewShell::GetModeSwitchingMenuState (SfxItemSet &rSet)
{
    //DrawView
    rSet.Put(SfxBoolItem(SID_SLIDE_SORTER_MODE, false));
    rSet.Put(SfxBoolItem(SID_OUTLINE_MODE, false));
    rSet.Put(SfxBoolItem(SID_SLIDE_MASTER_MODE, false));
    rSet.Put(SfxBoolItem(SID_NOTES_MASTER_MODE, false));
    if (mePageKind == PageKind::Notes)
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, false));
        rSet.Put(SfxBoolItem(SID_NOTES_MODE, true));
        rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, false));
    }
    else if (mePageKind == PageKind::Handout)
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, false));
        rSet.Put(SfxBoolItem(SID_NOTES_MODE, false));
        rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, true));
    }
    else
    {
        rSet.Put(SfxBoolItem(SID_DRAWINGMODE, true));
        rSet.Put(SfxBoolItem(SID_NOTES_MODE, false));
        rSet.Put(SfxBoolItem(SID_HANDOUT_MASTER_MODE, false));
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

        rSet.ClearItem( SID_NOTES_MODE );
        rSet.DisableItem( SID_NOTES_MODE );

        rSet.ClearItem( SID_HANDOUT_MASTER_MODE );
        rSet.DisableItem( SID_HANDOUT_MASTER_MODE );

        rSet.ClearItem( SID_OUTLINE_MODE );
        rSet.DisableItem( SID_OUTLINE_MODE );

        rSet.ClearItem( SID_SLIDE_MASTER_MODE );
        rSet.DisableItem( SID_SLIDE_MASTER_MODE );

        rSet.ClearItem( SID_NOTES_MASTER_MODE );
        rSet.DisableItem( SID_NOTES_MASTER_MODE );

        rSet.ClearItem( SID_SLIDE_SORTER_MODE );
        rSet.DisableItem( SID_SLIDE_SORTER_MODE );
    }

    if (GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED)
    {
        // Outplace-Edit: do not allow switch
        rSet.ClearItem( SID_OUTLINE_MODE );
        rSet.DisableItem( SID_OUTLINE_MODE );

        rSet.ClearItem( SID_SLIDE_SORTER_MODE );
        rSet.DisableItem( SID_SLIDE_SORTER_MODE );

        rSet.ClearItem( SID_NOTES_MODE );
        rSet.DisableItem( SID_NOTES_MODE );

        rSet.ClearItem( SID_HANDOUT_MASTER_MODE );
        rSet.DisableItem( SID_HANDOUT_MASTER_MODE );

        rSet.ClearItem( SID_SLIDE_MASTER_MODE );
        rSet.DisableItem( SID_SLIDE_MASTER_MODE );

        rSet.ClearItem( SID_NOTES_MASTER_MODE );
        rSet.DisableItem( SID_NOTES_MASTER_MODE );
    }

    svx::ExtrusionBar::getState( mpDrawView.get(), rSet );
    svx::FontworkBar::getState( mpDrawView.get(), rSet );
}

void DrawViewShell::GetPageProperties( SfxItemSet &rSet )
{
    SdPage *pPage = getCurrentPage();

    if (pPage == nullptr || GetDoc() == nullptr)
        return;

    SvxPageItem aPageItem(SID_ATTR_PAGE);
    aPageItem.SetLandscape( pPage->GetOrientation() == Orientation::Landscape );

    rSet.Put(SvxSizeItem( SID_ATTR_PAGE_SIZE, pPage->GetSize() ));
    rSet.Put(aPageItem);

    const SfxItemSet &rPageAttr = pPage->getSdrPageProperties().GetItemSet();
    drawing::FillStyle eXFS = rPageAttr.GetItem( XATTR_FILLSTYLE )->GetValue();
    XFillStyleItem aFillStyleItem( eXFS );
    aFillStyleItem.SetWhich( SID_ATTR_PAGE_FILLSTYLE );
    rSet.Put(aFillStyleItem);

    switch (eXFS)
    {
        case drawing::FillStyle_SOLID:
        {
            Color aColor =  rPageAttr.GetItem( XATTR_FILLCOLOR )->GetColorValue();
            XFillColorItem aFillColorItem( OUString(), aColor );
            aFillColorItem.SetWhich( SID_ATTR_PAGE_COLOR );
            rSet.Put( aFillColorItem );
        }
        break;

        case drawing::FillStyle_GRADIENT:
        {
            const XFillGradientItem *pGradient =  rPageAttr.GetItem( XATTR_FILLGRADIENT );
            XFillGradientItem aFillGradientItem( pGradient->GetName(), pGradient->GetGradientValue(), SID_ATTR_PAGE_GRADIENT );
            rSet.Put( aFillGradientItem );
        }
        break;

        case drawing::FillStyle_HATCH:
        {
            const XFillHatchItem *pFillHatchItem( rPageAttr.GetItem( XATTR_FILLHATCH ) );
            XFillHatchItem aFillHatchItem( pFillHatchItem->GetName(), pFillHatchItem->GetHatchValue());
            aFillHatchItem.SetWhich( SID_ATTR_PAGE_HATCH );
            rSet.Put( aFillHatchItem );
        }
        break;

        case drawing::FillStyle_BITMAP:
        {
            const XFillBitmapItem *pFillBitmapItem = rPageAttr.GetItem( XATTR_FILLBITMAP );
            XFillBitmapItem aFillBitmapItem( pFillBitmapItem->GetName(), pFillBitmapItem->GetGraphicObject() );
            aFillBitmapItem.SetWhich( SID_ATTR_PAGE_BITMAP );
            rSet.Put( aFillBitmapItem );
        }
        break;

        default:
        break;
    }
}

void DrawViewShell::SetPageProperties (SfxRequest& rReq)
{
    SdPage *pPage = getCurrentPage();
    if (!pPage)
        return;
    sal_uInt16 nSlotId = rReq.GetSlot();
    const SfxItemSet *pArgs = rReq.GetArgs();
    if (!pArgs)
        return;

    if ( ( nSlotId >= SID_ATTR_PAGE_COLOR ) && ( nSlotId <= SID_ATTR_PAGE_FILLSTYLE ) )
    {
        SdrPageProperties& rPageProperties = pPage->getSdrPageProperties();
        const SfxItemSet &aPageItemSet = rPageProperties.GetItemSet();
        std::unique_ptr<SfxItemSet> pTempSet = aPageItemSet.Clone(false, &mpDrawView->GetModel()->GetItemPool());

        rPageProperties.ClearItem(XATTR_FILLSTYLE);
        rPageProperties.ClearItem(XATTR_FILLGRADIENT);
        rPageProperties.ClearItem(XATTR_FILLHATCH);
        rPageProperties.ClearItem(XATTR_FILLBITMAP);

        switch (nSlotId)
        {
            case SID_ATTR_PAGE_FILLSTYLE:
            {
                XFillStyleItem aFSItem( pArgs->Get( XATTR_FILLSTYLE ) );
                drawing::FillStyle eXFS = aFSItem.GetValue();

                if ( eXFS == drawing::FillStyle_NONE )
                     rPageProperties.PutItem( XFillStyleItem( eXFS ) );
            }
            break;

            case SID_ATTR_PAGE_COLOR:
            {
                XFillColorItem aColorItem( pArgs->Get( XATTR_FILLCOLOR ) );
                rPageProperties.PutItem( XFillStyleItem( drawing::FillStyle_SOLID ) );
                rPageProperties.PutItem( aColorItem );
            }
            break;

            case SID_ATTR_PAGE_GRADIENT:
            {
                XFillGradientItem aGradientItem( pArgs->Get( XATTR_FILLGRADIENT ) );

                // MigrateItemSet guarantees unique gradient names
                SfxItemSet aMigrateSet( mpDrawView->GetModel()->GetItemPool(), svl::Items<XATTR_FILLGRADIENT, XATTR_FILLGRADIENT>{} );
                aMigrateSet.Put( aGradientItem );
                SdrModel::MigrateItemSet( &aMigrateSet, pTempSet.get(), mpDrawView->GetModel() );

                rPageProperties.PutItemSet( *pTempSet );
                rPageProperties.PutItem( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
            }
            break;

            case SID_ATTR_PAGE_HATCH:
            {
                XFillHatchItem aHatchItem( pArgs->Get( XATTR_FILLHATCH ) );
                rPageProperties.PutItem( XFillStyleItem( drawing::FillStyle_HATCH ) );
                rPageProperties.PutItem( aHatchItem );
            }
            break;

            case SID_ATTR_PAGE_BITMAP:
            {
                XFillBitmapItem aBitmapItem( pArgs->Get( XATTR_FILLBITMAP ) );
                rPageProperties.PutItem( XFillStyleItem( drawing::FillStyle_BITMAP ) );
                rPageProperties.PutItem( aBitmapItem );
            }
            break;

            default:
            break;
        }

        rReq.Done();
    }
    else
    {
        PageKind            ePageKind = GetPageKind();
        const SfxPoolItem*  pPoolItem = nullptr;
        Size                aNewSize(pPage->GetSize());
        sal_Int32           nLeft  = -1, nRight = -1, nUpper = -1, nLower = -1;
        bool                bScaleAll = true;
        Orientation         eOrientation = pPage->GetOrientation();
        SdPage*             pMasterPage = pPage->IsMasterPage() ? pPage : &static_cast<SdPage&>(pPage->TRG_GetMasterPage());
        bool                bFullSize = pMasterPage->IsBackgroundFullSize();
        sal_uInt16          nPaperBin = pPage->GetPaperBin();

        switch (nSlotId)
        {
            case SID_ATTR_PAGE_LRSPACE:
                if( pArgs->GetItemState(GetPool().GetWhich(SID_ATTR_PAGE_LRSPACE),
                                        true,&pPoolItem) == SfxItemState::SET )
                {
                    nLeft = static_cast<const SvxLongLRSpaceItem*>(pPoolItem)->GetLeft();
                    nRight = static_cast<const SvxLongLRSpaceItem*>(pPoolItem)->GetRight();
                    if (nLeft != -1)
                    {
                        nUpper  = pPage->GetUpperBorder();
                        nLower  = pPage->GetLowerBorder();
                    }
                    SetPageSizeAndBorder(ePageKind, aNewSize, nLeft, nRight, nUpper, nLower, bScaleAll, eOrientation, nPaperBin, bFullSize );
                }
                break;

            case SID_ATTR_PAGE_ULSPACE:
                if( pArgs->GetItemState(SID_ATTR_PAGE_ULSPACE,
                                        true,&pPoolItem) == SfxItemState::SET )
                {
                    nUpper = static_cast<const SvxLongULSpaceItem*>(pPoolItem)->GetUpper();
                    nLower = static_cast<const SvxLongULSpaceItem*>(pPoolItem)->GetLower();
                    if (nUpper != -1)
                    {
                        nLeft   = pPage->GetLeftBorder();
                        nRight  = pPage->GetRightBorder();
                    }
                    SetPageSizeAndBorder(ePageKind, aNewSize, nLeft, nRight, nUpper, nLower, bScaleAll, eOrientation, nPaperBin, bFullSize );
                }
                break;

            default:
            break;
        }
    }
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
                SAL_WARN("sd", "DrawViewShell::GetState(): can not handle which id " << nWhich);
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
            if (rReq.GetArgs() != nullptr)
                pViewFrame->SetChildWindow (SID_SPELL_DIALOG,
                    static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                        Get(SID_SPELL_DIALOG)).GetValue());
            else
                pViewFrame->ToggleChildWindow(SID_SPELL_DIALOG);

            pViewFrame->GetBindings().Invalidate(SID_SPELL_DIALOG);
            rReq.Ignore ();
        }
        break;

        default:
            SAL_WARN("sd", "DrawViewShell::Execute(): can not handle slot " << rReq.GetSlot());
            break;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
