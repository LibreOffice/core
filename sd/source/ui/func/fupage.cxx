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

#include <fupage.hxx>

#include <sfx2/viewfrm.hxx>

// arrange Tab-Page

#include <sfx2/sfxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/itempool.hxx>
#include <vcl/layout.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/prntypes.hxx>
#include <svl/style.hxx>
#include <stlsheet.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdundo.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/graphichelper.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xsetit.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <sfx2/bindings.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/pbinitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/opengrf.hxx>
#include <rtl/ustring.hxx>

#include <strings.hrc>
#include <sdpage.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <pres.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <app.hrc>
#include <unchss.hxx>
#include <undoback.hxx>
#include <sdabstdlg.hxx>
#include <sdresid.hxx>
#include <sdundogr.hxx>
#include <helpids.h>

#include <memory>

using namespace com::sun::star;

namespace vcl { class Window; }

namespace sd {

// 50 cm 28350
// adapted from writer
#define MAXHEIGHT 28350
#define MAXWIDTH  28350


static void mergeItemSetsImpl( SfxItemSet& rTarget, const SfxItemSet& rSource )
{
    const sal_uInt16* pPtr = rSource.GetRanges();
    sal_uInt16 p1, p2;
    while( *pPtr )
    {
        p1 = pPtr[0];
        p2 = pPtr[1];

        // make ranges discrete
        while(pPtr[2] && (pPtr[2] - p2 == 1))
        {
            p2 = pPtr[3];
            pPtr += 2;
        }
        rTarget.MergeRange( p1, p2 );
        pPtr += 2;
    }

    rTarget.Put(rSource);
}

FuPage::FuPage( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq )
:   FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    mrReq(rReq),
    mpArgs( rReq.GetArgs() ),
    mbPageBckgrdDeleted( false ),
    mbMasterPage( false ),
    mbDisplayBackgroundTabPage( true ),
    mpPage(nullptr),
    mpDrawViewShell(nullptr)
{
}

rtl::Reference<FuPoor> FuPage::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuPage( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuPage::DoExecute( SfxRequest& )
{
    mpDrawViewShell = dynamic_cast<DrawViewShell*>(mpViewShell);
    DBG_ASSERT( mpDrawViewShell, "sd::FuPage::FuPage(), called without a current DrawViewShell!" );

    if( mpDrawViewShell )
    {
        mbMasterPage = mpDrawViewShell->GetEditMode() == EditMode::MasterPage;
        // we don't really want to format page background with SID_ATTR_PAGE[_SIZE] slots
        mbDisplayBackgroundTabPage = ( mpDrawViewShell->GetPageKind() == PageKind::Standard) &&
                                      ( nSlotId != SID_ATTR_PAGE_SIZE) && ( nSlotId != SID_ATTR_PAGE );
        mpPage = mpDrawViewShell->getCurrentPage();
    }

    if( !mpPage )
        return;

    // if there are no arguments given, open the dialog
    if( !mpArgs )
    {
        mpView->SdrEndTextEdit();
        mpArgs = ExecuteDialog(mpWindow ? mpWindow->GetFrameWeld() : nullptr);
    }

    // if we now have arguments, apply them to current page
    if( mpArgs )
    {
        ApplyItemSet( mpArgs );
    }
}

FuPage::~FuPage()
{
}

void FuPage::Activate()
{
}

void FuPage::Deactivate()
{
}

void MergePageBackgroundFilling(SdPage *pPage, SdStyleSheet *pStyleSheet, bool bMasterPage, SfxItemSet& rMergedAttr)
{
    if (bMasterPage)
    {
        if (pStyleSheet)
            mergeItemSetsImpl(rMergedAttr, pStyleSheet->GetItemSet());
    }
    else
    {
        // Only this page, get attributes for background fill
        const SfxItemSet& rBackgroundAttributes = pPage->getSdrPageProperties().GetItemSet();

        if(drawing::FillStyle_NONE != rBackgroundAttributes.Get(XATTR_FILLSTYLE).GetValue())
        {
            // page attributes are used, take them
            rMergedAttr.Put(rBackgroundAttributes);
        }
        else
        {
            if(pStyleSheet
                && drawing::FillStyle_NONE != pStyleSheet->GetItemSet().Get(XATTR_FILLSTYLE).GetValue())
            {
                // if the page has no fill style, use the settings from the
                // background stylesheet (if used)
                mergeItemSetsImpl(rMergedAttr, pStyleSheet->GetItemSet());
            }
            else
            {
                // no fill style from page, start with no fill style
                rMergedAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
            }
        }
    }
}

const SfxItemSet* FuPage::ExecuteDialog(weld::Window* pParent)
{
    if (!mpDrawViewShell)
        return nullptr;

    SfxItemSet aNewAttr(mpDoc->GetPool(),
                        {{mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                        mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE)},
                        {SID_ATTR_PAGE, SID_ATTR_PAGE_SHARED},
                        {SID_ATTR_BORDER_OUTER, SID_ATTR_BORDER_OUTER},
                        {SID_ATTR_BORDER_SHADOW, SID_ATTR_BORDER_SHADOW},
                        {XATTR_FILL_FIRST, XATTR_FILL_LAST},
                        {SID_ATTR_PAGE_COLOR,SID_ATTR_PAGE_FILLSTYLE},
                        {EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR}});

    // Retrieve additional data for dialog

    SvxShadowItem aShadowItem(SID_ATTR_BORDER_SHADOW);
    aNewAttr.Put( aShadowItem );
    SvxBoxItem aBoxItem( SID_ATTR_BORDER_OUTER );
    aNewAttr.Put( aBoxItem );

    aNewAttr.Put( SvxFrameDirectionItem(
        mpDoc->GetDefaultWritingMode() == css::text::WritingMode_RL_TB ? SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB,
        EE_PARA_WRITINGDIR ) );

    // Retrieve page-data for dialog

    SvxPageItem aPageItem( SID_ATTR_PAGE );
    aPageItem.SetDescName( mpPage->GetName() );
    aPageItem.SetPageUsage( SvxPageUsage::All );
    aPageItem.SetLandscape( mpPage->GetOrientation() == Orientation::Landscape );
    aPageItem.SetNumType( mpDoc->GetPageNumType() );
    aNewAttr.Put( aPageItem );

    // size
    maSize = mpPage->GetSize();
    SvxSizeItem aSizeItem( SID_ATTR_PAGE_SIZE, maSize );
    aNewAttr.Put( aSizeItem );

    // Max size
    SvxSizeItem aMaxSizeItem( SID_ATTR_PAGE_MAXSIZE, Size( MAXWIDTH, MAXHEIGHT ) );
    aNewAttr.Put( aMaxSizeItem );

    // paperbin
    SvxPaperBinItem aPaperBinItem( SID_ATTR_PAGE_PAPERBIN, static_cast<sal_uInt8>(mpPage->GetPaperBin()) );
    aNewAttr.Put( aPaperBinItem );

    SvxLRSpaceItem aLRSpaceItem( static_cast<sal_uInt16>(mpPage->GetLeftBorder()), static_cast<sal_uInt16>(mpPage->GetRightBorder()), 0, 0, mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE));
    aNewAttr.Put( aLRSpaceItem );

    SvxULSpaceItem aULSpaceItem( static_cast<sal_uInt16>(mpPage->GetUpperBorder()), static_cast<sal_uInt16>(mpPage->GetLowerBorder()), mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE));
    aNewAttr.Put( aULSpaceItem );

    // Application
    bool bScale = mpDoc->GetDocumentType() != DocumentType::Draw;
    aNewAttr.Put( SfxBoolItem( SID_ATTR_PAGE_EXT1, bScale ) );

    bool bFullSize = mpPage->IsMasterPage() ?
        mpPage->IsBackgroundFullSize() : static_cast<SdPage&>(mpPage->TRG_GetMasterPage()).IsBackgroundFullSize();

    aNewAttr.Put( SfxBoolItem( SID_ATTR_PAGE_EXT2, bFullSize ) );

    // Merge ItemSet for dialog

    const sal_uInt16* pPtr = aNewAttr.GetRanges();
    sal_uInt16 p1 = pPtr[0], p2 = pPtr[1];
    while(pPtr[2] && (pPtr[2] - p2 == 1))
    {
        p2 = pPtr[3];
        pPtr += 2;
    }
    SfxItemSet aMergedAttr( *aNewAttr.GetPool(), {{p1, p2}} );

    mergeItemSetsImpl( aMergedAttr, aNewAttr );

    SdStyleSheet* pStyleSheet = mpPage->getPresentationStyle(HID_PSEUDOSHEET_BACKGROUND);

    // merge page background filling to the dialogs input set
    if( mbDisplayBackgroundTabPage )
    {
        MergePageBackgroundFilling(mpPage, pStyleSheet, mbMasterPage, aMergedAttr);
    }

    std::unique_ptr< SfxItemSet > pTempSet;

    const sal_uInt16 nId = GetSlotID();
    if (nId == SID_SAVE_BACKGROUND)
    {
        const XFillStyleItem& rStyleItem = aMergedAttr.Get(XATTR_FILLSTYLE);
        if (drawing::FillStyle_BITMAP == rStyleItem.GetValue())
        {
            const XFillBitmapItem& rBitmap = aMergedAttr.Get(XATTR_FILLBITMAP);
            const GraphicObject& rGraphicObj = rBitmap.GetGraphicObject();
            GraphicHelper::ExportGraphic(pParent, rGraphicObj.GetGraphic(), "");
        }
    }
    else if (nId == SID_SELECT_BACKGROUND)
    {
        SvxOpenGraphicDialog aDlg(SdResId(STR_SET_BACKGROUND_PICTURE), pParent);

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            Graphic     aGraphic;
            ErrCode nError = aDlg.GetGraphic(aGraphic);
            if( nError == ERRCODE_NONE )
            {
                pTempSet.reset( new SfxItemSet( mpDoc->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{}) );

                pTempSet->Put( XFillStyleItem( drawing::FillStyle_BITMAP ) );

                // MigrateItemSet makes sure the XFillBitmapItem will have a unique name
                SfxItemSet aMigrateSet( mpDoc->GetPool(), svl::Items<XATTR_FILLBITMAP, XATTR_FILLBITMAP>{} );
                aMigrateSet.Put(XFillBitmapItem("background", aGraphic));
                SdrModel::MigrateItemSet( &aMigrateSet, pTempSet.get(), mpDoc );

                pTempSet->Put( XFillBmpStretchItem( true ));
                pTempSet->Put( XFillBmpTileItem( false ));
            }
        }
    }

    else
    {
        // create the dialog
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        ScopedVclPtr<SfxAbstractTabDialog> pDlg( pFact->CreateSdTabPageDialog(mpViewShell->GetFrameWeld(), &aMergedAttr, mpDocSh, mbDisplayBackgroundTabPage) );
        if( pDlg->Execute() == RET_OK )
            pTempSet.reset( new SfxItemSet(*pDlg->GetOutputItemSet()) );
    }

    if (pTempSet.get() && pStyleSheet)
    {
        pStyleSheet->AdjustToFontHeight(*pTempSet);

        if( mbDisplayBackgroundTabPage )
        {
            // if some fillstyle-items are not set in the dialog, then
            // try to use the items before
            bool bChanges = false;
            for( sal_uInt16 i=XATTR_FILL_FIRST; i<XATTR_FILL_LAST; i++ )
            {
                if( aMergedAttr.GetItemState( i ) != SfxItemState::DEFAULT )
                {
                    if( pTempSet->GetItemState( i ) == SfxItemState::DEFAULT )
                        pTempSet->Put( aMergedAttr.Get( i ) );
                    else
                        if( aMergedAttr.GetItem( i ) != pTempSet->GetItem( i ) )
                            bChanges = true;
                }
            }

            // if the background for this page was set to invisible, the background-object has to be deleted, too.
            if( ( pTempSet->GetItem<XFillStyleItem>( XATTR_FILLSTYLE )->GetValue() == drawing::FillStyle_NONE ) ||
                ( ( pTempSet->GetItemState( XATTR_FILLSTYLE ) == SfxItemState::DEFAULT ) &&
                    ( aMergedAttr.GetItem<XFillStyleItem>( XATTR_FILLSTYLE )->GetValue() == drawing::FillStyle_NONE ) ) )
                mbPageBckgrdDeleted = true;

            if( !mbMasterPage && bChanges && mbPageBckgrdDeleted )
            {
                 mpBackgroundObjUndoAction.reset( new SdBackgroundObjUndoAction(
                     *mpDoc, *mpPage, mpPage->getSdrPageProperties().GetItemSet()) );

                 if(!mpPage->IsMasterPage())
                 {
                     // on normal pages, switch off fill attribute usage
                     SdrPageProperties& rPageProperties = mpPage->getSdrPageProperties();
                     rPageProperties.ClearItem( XATTR_FILLBITMAP );
                     rPageProperties.ClearItem( XATTR_FILLGRADIENT );
                     rPageProperties.ClearItem( XATTR_FILLHATCH );
                     rPageProperties.PutItem(XFillStyleItem(drawing::FillStyle_NONE));
                 }
            }


            /* Special treatment: reset the INVALIDS to
               NULL-Pointer (otherwise INVALIDs or pointer point
               to DefaultItems in the template; both would
               prevent the attribute inheritance) */
            pTempSet->ClearInvalidItems();

            if( mbMasterPage )
            {
                mpDocSh->GetUndoManager()->AddUndoAction(std::make_unique<StyleSheetUndoAction>(
                    mpDoc, static_cast<SfxStyleSheet*>(pStyleSheet), &(*pTempSet)));
                pStyleSheet->GetItemSet().Put( *pTempSet );
                sdr::properties::CleanupFillProperties( pStyleSheet->GetItemSet() );
                pStyleSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
            }

            // if background filling is set to master pages then clear from page set
            if( mbMasterPage )
            {
                for( sal_uInt16 nWhich = XATTR_FILL_FIRST; nWhich <= XATTR_FILL_LAST; nWhich++ )
                {
                    pTempSet->ClearItem( nWhich );
                }
                pTempSet->Put(XFillStyleItem(drawing::FillStyle_NONE));
            }

            const SfxPoolItem *pItem;
            if( SfxItemState::SET == pTempSet->GetItemState( EE_PARA_WRITINGDIR, false, &pItem ) )
            {
                SvxFrameDirection nVal = static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue();
                mpDoc->SetDefaultWritingMode( nVal == SvxFrameDirection::Horizontal_RL_TB ? css::text::WritingMode_RL_TB : css::text::WritingMode_LR_TB );
            }

            mpDoc->SetChanged();

            // BackgroundFill of Masterpage: no hard attributes allowed
            SdrPage& rUsedMasterPage = mpPage->IsMasterPage() ? *mpPage : mpPage->TRG_GetMasterPage();
            OSL_ENSURE(rUsedMasterPage.IsMasterPage(), "No MasterPage (!)");
            rUsedMasterPage.getSdrPageProperties().ClearItem();
            OSL_ENSURE(nullptr != rUsedMasterPage.getSdrPageProperties().GetStyleSheet(),
                "MasterPage without StyleSheet detected (!)");
        }

        aNewAttr.Put(*pTempSet);
        mrReq.Done( aNewAttr );

        return mrReq.GetArgs();
    }
    else
    {
        return nullptr;
    }
}

void FuPage::ApplyItemSet( const SfxItemSet* pArgs )
{
    if (!pArgs || !mpDrawViewShell)
        return;

    // Set new page-attributes
    PageKind ePageKind = mpDrawViewShell->GetPageKind();
    const SfxPoolItem*  pPoolItem;
    bool                bSetPageSizeAndBorder = false;
    Size                aNewSize(maSize);
    sal_Int32               nLeft  = -1, nRight = -1, nUpper = -1, nLower = -1;
    bool                bScaleAll = true;
    Orientation         eOrientation = mpPage->GetOrientation();
    SdPage*             pMasterPage = mpPage->IsMasterPage() ? mpPage : &static_cast<SdPage&>(mpPage->TRG_GetMasterPage());
    bool                bFullSize = pMasterPage->IsBackgroundFullSize();
    sal_uInt16          nPaperBin = mpPage->GetPaperBin();

    if( pArgs->GetItemState(SID_ATTR_PAGE, true, &pPoolItem) == SfxItemState::SET )
    {
        mpDoc->SetPageNumType(static_cast<const SvxPageItem*>(pPoolItem)->GetNumType());

        eOrientation = static_cast<const SvxPageItem*>(pPoolItem)->IsLandscape() ?
            Orientation::Landscape : Orientation::Portrait;

        if( mpPage->GetOrientation() != eOrientation )
            bSetPageSizeAndBorder = true;

        mpDrawViewShell->ResetActualPage();
    }

    if( pArgs->GetItemState(SID_ATTR_PAGE_SIZE, true, &pPoolItem) == SfxItemState::SET )
    {
        aNewSize = static_cast<const SvxSizeItem*>(pPoolItem)->GetSize();

        if( mpPage->GetSize() != aNewSize )
            bSetPageSizeAndBorder = true;
    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                            true, &pPoolItem) == SfxItemState::SET )
    {
        nLeft = static_cast<const SvxLRSpaceItem*>(pPoolItem)->GetLeft();
        nRight = static_cast<const SvxLRSpaceItem*>(pPoolItem)->GetRight();

        if( mpPage->GetLeftBorder() != nLeft || mpPage->GetRightBorder() != nRight )
            bSetPageSizeAndBorder = true;

    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE),
                            true, &pPoolItem) == SfxItemState::SET )
    {
        nUpper = static_cast<const SvxULSpaceItem*>(pPoolItem)->GetUpper();
        nLower = static_cast<const SvxULSpaceItem*>(pPoolItem)->GetLower();

        if( mpPage->GetUpperBorder() != nUpper || mpPage->GetLowerBorder() != nLower )
            bSetPageSizeAndBorder = true;
    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_PAGE_EXT1), true, &pPoolItem) == SfxItemState::SET )
    {
        bScaleAll = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
    }

    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_PAGE_EXT2), true, &pPoolItem) == SfxItemState::SET )
    {
        bFullSize = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();

        if(pMasterPage->IsBackgroundFullSize() != bFullSize )
            bSetPageSizeAndBorder = true;
    }

    // Paper Bin
    if( pArgs->GetItemState(mpDoc->GetPool().GetWhich(SID_ATTR_PAGE_PAPERBIN), true, &pPoolItem) == SfxItemState::SET )
    {
        nPaperBin = static_cast<const SvxPaperBinItem*>(pPoolItem)->GetValue();

        if( mpPage->GetPaperBin() != nPaperBin )
            bSetPageSizeAndBorder = true;
    }

    if (nLeft == -1 && nUpper != -1)
    {
        bSetPageSizeAndBorder = true;
        nLeft  = mpPage->GetLeftBorder();
        nRight = mpPage->GetRightBorder();
    }
    else if (nLeft != -1 && nUpper == -1)
    {
        bSetPageSizeAndBorder = true;
        nUpper = mpPage->GetUpperBorder();
        nLower = mpPage->GetLowerBorder();
    }

    if( bSetPageSizeAndBorder || !mbMasterPage )
        mpDrawViewShell->SetPageSizeAndBorder(ePageKind, aNewSize, nLeft, nRight, nUpper, nLower, bScaleAll, eOrientation, nPaperBin, bFullSize );

    // if bMasterPage==sal_False then create a background-object for this page with the
    // properties set in the dialog before, but if mbPageBckgrdDeleted==sal_True then
    // the background of this page was set to invisible, so it would be a mistake
    // to create a new background-object for this page !

    if( mbDisplayBackgroundTabPage )
    {
        if( !mbMasterPage && !mbPageBckgrdDeleted )
        {
            // Only this page
            mpBackgroundObjUndoAction.reset( new SdBackgroundObjUndoAction(
                *mpDoc, *mpPage, mpPage->getSdrPageProperties().GetItemSet()) );
            SfxItemSet aSet( *pArgs );
            sdr::properties::CleanupFillProperties(aSet);
            mpPage->getSdrPageProperties().ClearItem();
            mpPage->getSdrPageProperties().PutItemSet(aSet);
        }
    }

    // add undo action for background object
    if( mpBackgroundObjUndoAction )
    {
        // set merge flag, because a SdUndoGroupAction could have been inserted before
        mpDocSh->GetUndoManager()->AddUndoAction( std::move(mpBackgroundObjUndoAction), true );
    }

    // Objects can not be bigger than ViewSize
    Size aPageSize = mpDoc->GetSdPage(0, ePageKind)->GetSize();
    Size aViewSize(aPageSize.Width() * 3, aPageSize.Height() * 2);
    mpDoc->SetMaxObjSize(aViewSize);

    // if necessary, we tell Preview the new context
    mpDrawViewShell->UpdatePreview( mpDrawViewShell->GetActualPage() );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
