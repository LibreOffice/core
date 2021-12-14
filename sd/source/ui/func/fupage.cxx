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

// arrange Tab-Page

#include <sfx2/sfxdlg.hxx>
#include <svx/pageitem.hxx>
#include <svx/svxids.hrc>
#include <svl/itempool.hxx>
#include <svl/grabbagitem.hxx>
#include <sfx2/request.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/graphicfilter.hxx>
#include <stlsheet.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/graphichelper.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/pbinitem.hxx>
#include <sfx2/opengrf.hxx>
#include <sal/log.hxx>

#include <strings.hrc>
#include <sdpage.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <pres.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>
#include <app.hrc>
#include <unchss.hxx>
#include <undoback.hxx>
#include <sdabstdlg.hxx>
#include <sdresid.hxx>

#include <memory>

using namespace com::sun::star;

namespace sd {

// 50 cm 28350
// adapted from writer
#define MAXHEIGHT 28350
#define MAXWIDTH  28350


static void mergeItemSetsImpl( SfxItemSet& rTarget, const SfxItemSet& rSource )
{
    const WhichRangesContainer& rRanges = rSource.GetRanges();
    sal_uInt16 p1, p2;
    for (sal_Int32 i = 0; i < rRanges.size(); ++i)
    {
        p1 = rRanges[i].first;
        p2 = rRanges[i].second;

        // make ranges discrete
        while(i < rRanges.size()-1 && (rRanges[i+1].first - p2 == 1))
        {
            p2 = rRanges[i+1].second;
            ++i;
        }
        rTarget.MergeRange( p1, p2 );
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

void FuPage::DoExecute(SfxRequest& rReq)
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
    const SfxPoolItem* pItem;
    if (!mpArgs || mpArgs->GetItemState(SID_SELECT_BACKGROUND, true, &pItem) == SfxItemState::SET)
    {
        mpView->SdrEndTextEdit();
        mpArgs = ExecuteDialog(mpWindow ? mpWindow->GetFrameWeld() : nullptr, rReq);
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

const SfxItemSet* FuPage::ExecuteDialog(weld::Window* pParent, const SfxRequest& rReq)
{
    if (!mpDrawViewShell)
        return nullptr;

    SfxItemSetFixed<
                  XATTR_FILL_FIRST, XATTR_FILL_LAST,
                  EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                  SID_ATTR_BORDER_OUTER, SID_ATTR_BORDER_OUTER,
                  SID_ATTR_BORDER_SHADOW, SID_ATTR_BORDER_SHADOW,
                  SID_ATTR_PAGE, SID_ATTR_PAGE_SHARED,
                  SID_ATTR_CHAR_GRABBAG, SID_ATTR_CHAR_GRABBAG,
                  SID_ATTR_PAGE_COLOR, SID_ATTR_PAGE_FILLSTYLE
              >  aNewAttr(mpDoc->GetPool());
    // Keep it sorted
    aNewAttr.MergeRange(mpDoc->GetPool().GetWhich(SID_ATTR_LRSPACE),
                        mpDoc->GetPool().GetWhich(SID_ATTR_ULSPACE));

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

    SfxGrabBagItem grabBag(SID_ATTR_CHAR_GRABBAG);
    grabBag.GetGrabBag()["BackgroundFullSize"] <<= bFullSize;

    if (mpDoc->GetDocumentType() == DocumentType::Impress && mpPage->IsMasterPage())
    {
        // A master slide may have a theme.
        svx::Theme* pTheme = mpPage->getSdrPageProperties().GetTheme();
        if (pTheme)
        {
            uno::Any aTheme;
            pTheme->ToAny(aTheme);
            grabBag.GetGrabBag()["Theme"] = aTheme;
        }
    }

    aNewAttr.Put(grabBag);

    // Merge ItemSet for dialog

    const WhichRangesContainer& rRanges = aNewAttr.GetRanges();
    sal_uInt16 p1 = rRanges[0].first, p2 = rRanges[0].second;
    sal_Int32 idx = 1;
    while(idx < rRanges.size() && (rRanges[idx].first - p2 == 1))
    {
        p2 = rRanges[idx].second;
        ++idx;
    }
    SfxItemSet aMergedAttr( *aNewAttr.GetPool(), p1, p2 );

    mergeItemSetsImpl( aMergedAttr, aNewAttr );

    SdStyleSheet* pStyleSheet = mpPage->getPresentationStyle(HID_PSEUDOSHEET_BACKGROUND);

    // merge page background filling to the dialogs input set
    if( mbDisplayBackgroundTabPage )
    {
        MergePageBackgroundFilling(mpPage, pStyleSheet, mbMasterPage, aMergedAttr);
    }

    std::optional< SfxItemSet > pTempSet;

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
        Graphic aGraphic;
        ErrCode nError = ERRCODE_GRFILTER_OPENERROR;

        const SfxItemSet* pArgs = rReq.GetArgs();
        const SfxPoolItem* pItem;

        if (pArgs && pArgs->GetItemState(SID_SELECT_BACKGROUND, true, &pItem) == SfxItemState::SET)
        {
            OUString aFileName(static_cast<const SfxStringItem*>(pItem)->GetValue());
            OUString aFilterName;

            if (pArgs->GetItemState(FN_PARAM_FILTER, true, &pItem) == SfxItemState::SET)
                aFilterName = static_cast<const SfxStringItem*>(pItem)->GetValue();

            nError = GraphicFilter::LoadGraphic(aFileName, aFilterName, aGraphic,
                                                &GraphicFilter::GetGraphicFilter());
        }
        else
        {
            SvxOpenGraphicDialog aDlg(SdResId(STR_SET_BACKGROUND_PICTURE), pParent);

            nError = aDlg.Execute();
            if (nError == ERRCODE_NONE)
            {
                nError = aDlg.GetGraphic(aGraphic);
            }
        }

        if (nError == ERRCODE_NONE)
        {
            pTempSet.emplace( mpDoc->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST> );

            pTempSet->Put( XFillStyleItem( drawing::FillStyle_BITMAP ) );

            // MigrateItemSet makes sure the XFillBitmapItem will have a unique name
            SfxItemSetFixed<XATTR_FILLBITMAP, XATTR_FILLBITMAP> aMigrateSet( mpDoc->GetPool() );
            aMigrateSet.Put(XFillBitmapItem("background", aGraphic));
            SdrModel::MigrateItemSet( &aMigrateSet, &*pTempSet, mpDoc );

            pTempSet->Put( XFillBmpStretchItem( true ));
            pTempSet->Put( XFillBmpTileItem( false ));
        }
    }

    else
    {
        bool bIsImpressDoc = mpDrawViewShell->GetDoc()->GetDocumentType() == DocumentType::Impress;

        // create the dialog
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        ScopedVclPtr<SfxAbstractTabDialog> pDlg( pFact->CreateSdTabPageDialog(mpViewShell->GetFrameWeld(), &aMergedAttr, mpDocSh, mbDisplayBackgroundTabPage, bIsImpressDoc, mbMasterPage) );
        if( pDlg->Execute() == RET_OK )
            pTempSet.emplace( *pDlg->GetOutputItemSet() );
    }

    if (pTempSet && pStyleSheet)
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
            const XFillStyleItem* pTempFillStyleItem = pTempSet->GetItem<XFillStyleItem>(XATTR_FILLSTYLE);
            assert(pTempFillStyleItem);
            if (pTempFillStyleItem->GetValue() == drawing::FillStyle_NONE)
                mbPageBckgrdDeleted = true;
            else
            {
                if (pTempSet->GetItemState(XATTR_FILLSTYLE) == SfxItemState::DEFAULT)
                {
                    const XFillStyleItem* pMergedFillStyleItem = aMergedAttr.GetItem<XFillStyleItem>(XATTR_FILLSTYLE);
                    assert(pMergedFillStyleItem);
                    if (pMergedFillStyleItem->GetValue() == drawing::FillStyle_NONE)
                        mbPageBckgrdDeleted = true;
                }
            }

            const XFillGradientItem* pTempGradItem = pTempSet->GetItem<XFillGradientItem>(XATTR_FILLGRADIENT);
            if (pTempGradItem && pTempGradItem->GetName().isEmpty())
            {
                // MigrateItemSet guarantees unique gradient names
                SfxItemSetFixed<XATTR_FILLGRADIENT, XATTR_FILLGRADIENT> aMigrateSet( mpDoc->GetPool() );
                aMigrateSet.Put( XFillGradientItem("gradient", pTempGradItem->GetGradientValue()) );
                SdrModel::MigrateItemSet( &aMigrateSet, &*pTempSet, mpDoc);
            }

            const XFillHatchItem* pTempHatchItem = pTempSet->GetItem<XFillHatchItem>(XATTR_FILLHATCH);
            if (pTempHatchItem && pTempHatchItem->GetName().isEmpty())
            {
                // MigrateItemSet guarantees unique hatch names
                SfxItemSetFixed<XATTR_FILLHATCH, XATTR_FILLHATCH> aMigrateSet( mpDoc->GetPool() );
                aMigrateSet.Put( XFillHatchItem("hatch", pTempHatchItem->GetHatchValue()) );
                SdrModel::MigrateItemSet( &aMigrateSet, &*pTempSet, mpDoc);
            }

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

    if (SfxItemState::SET == pArgs->GetItemState(SID_ATTR_CHAR_GRABBAG, true, &pPoolItem))
    {
        SfxGrabBagItem const*const pGrabBag(static_cast<SfxGrabBagItem const*>(pPoolItem));
        if (pGrabBag->GetGrabBag().find("BackgroundFullSize")->second >>= bFullSize)
        {
            if (pMasterPage->IsBackgroundFullSize() != bFullSize)
            {
                bSetPageSizeAndBorder = true;
            }
        }

        if (mpDoc->GetDocumentType() == DocumentType::Impress && mpPage->IsMasterPage())
        {
            // The item set may have a theme.
            auto it = pGrabBag->GetGrabBag().find("Theme");
            if (it != pGrabBag->GetGrabBag().end())
            {
                std::unique_ptr<svx::Theme> pTheme = svx::Theme::FromAny(it->second);
                pMasterPage->getSdrPageProperties().SetTheme(std::move(pTheme));
            }
            else
            {
                SAL_WARN("sd.ui", "FuPage::ApplyItemSet: got no theme");
            }
        }
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
