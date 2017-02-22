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

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/ui/XDeck.hpp>
#include <com/sun/star/ui/XPanel.hpp>
#include "SlideBackground.hxx"
#include "TransitionPreset.hxx"
#include "sdresid.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "DrawDocShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "drawdoc.hxx"
#include "filedlg.hxx"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "DocumentHelper.hxx"
#include "MasterPagesSelector.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "sdresid.hxx"
#include <svtools/controldims.hrc>
#include <svx/gallery.hxx>
#include <svx/drawitem.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <tools/resary.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include "app.hrc"
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svxids.hrc>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xattr.hxx>
#include <svx/xflhtit.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <svx/dlgutil.hxx>
#include <algorithm>
#include "EventMultiplexer.hxx"
#include "glob.hrc"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace {
   void lcl_FillPaperSizeListbox ( ListBox &rListBox)
   {
       std::map< sal_Int32, Paper > aPaperSizeMap =
       {
          { 0, PAPER_A6 },
          { 1, PAPER_A5 },
          { 2, PAPER_A4 },
          { 3, PAPER_A3 },
          { 4, PAPER_A2 },
          { 5, PAPER_A1 },
          { 6, PAPER_A0 },
          { 7, PAPER_B6_ISO },
          { 8, PAPER_B5_ISO },
          { 9, PAPER_B4_ISO },
          { 10, PAPER_LETTER },
          { 11, PAPER_LEGAL },
          { 12, PAPER_FANFOLD_LEGAL_DE },
          { 13, PAPER_TABLOID },
          { 14, PAPER_B6_JIS },
          { 15, PAPER_B5_JIS },
          { 16, PAPER_B4_JIS },
          { 17, PAPER_KAI16 },
          { 18, PAPER_KAI32 },
          { 19, PAPER_KAI32BIG },
          { 20, PAPER_USER },
          { 21, PAPER_ENV_DL },
          { 22, PAPER_ENV_C6 },
          { 23, PAPER_ENV_C65 },
          { 24, PAPER_ENV_C5 },
          { 25, PAPER_ENV_C4 },
          { 26, PAPER_SLIDE_DIA },
          { 27, PAPER_SCREEN_4_3 },
          { 28, PAPER_SCREEN_16_9 },
          { 29, PAPER_SCREEN_16_10 },
          { 30, PAPER_POSTCARD_JP }
       };

       for ( sal_Int32 nIdx = 0; nIdx < rListBox.GetEntryCount(); nIdx++ )
       {
           Paper eSize = aPaperSizeMap[nIdx];
           rListBox.SetEntryData( nIdx, reinterpret_cast<void*>( (sal_uLong)eSize ));
       }
   }
}

namespace sd { namespace sidebar {

SlideBackground::SlideBackground(
    Window * pParent,
    ViewShellBase& rBase,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings
     ) :
    PanelLayout( pParent, "SlideBackgroundPanel", "modules/simpress/ui/sidebarslidebackground.ui", rxFrame ),
    mrBase( rBase ),
    maPaperSizeController(SID_ATTR_PAGE_SIZE, *pBindings, *this),
    maPaperOrientationController(SID_ATTR_PAGE, *pBindings, *this),
    maBckColorController(SID_ATTR_PAGE_COLOR, *pBindings, *this),
    maBckGradientController(SID_ATTR_PAGE_GRADIENT, *pBindings, *this),
    maBckHatchController(SID_ATTR_PAGE_HATCH, *pBindings, *this),
    maBckBitmapController(SID_ATTR_PAGE_BITMAP, *pBindings, *this),
    maBckFillStyleController(SID_ATTR_PAGE_FILLSTYLE, *pBindings, *this),
    maBckImageController(SID_SELECT_BACKGROUND, *pBindings, *this),
    maDspBckController(SID_DISPLAY_MASTER_BACKGROUND, *pBindings, *this),
    maDspObjController(SID_DISPLAY_MASTER_OBJECTS, *pBindings, *this),
    maMetricController(SID_ATTR_METRIC, *pBindings, *this),
    mpFillStyleItem(),
    mpColorItem(),
    mpGradientItem(),
    mpHatchItem(),
    mpBitmapItem(),
    mxFrame(rxFrame),
    maContext(),
    mbTitle(false),
    mpBindings(pBindings)
{
    get(mpPaperSizeBox,"paperformat");
    get(mpPaperOrientation, "orientation");
    get(mpMasterSlide, "masterslide");
    get(mpFillAttr, "fillattr1");
    get(mpFillGrad, "fillattr2");
    get(mpFillStyle, "fillstyle");
    get(mpFillLB, "fillattr");
    get(mpDspMasterBackground, "displaymasterbackground");
    get(mpDspMasterObjects, "displaymasterobjects");
    addListener();
    Initialize();
}

SlideBackground::~SlideBackground()
{
    disposeOnce();
}

void SlideBackground::Initialize()
{
    lcl_FillPaperSizeListbox( *mpPaperSizeBox );
    mpPaperSizeBox->SetSelectHdl(LINK(this,SlideBackground,PaperSizeModifyHdl));
    mpPaperOrientation->SetSelectHdl(LINK(this,SlideBackground,PaperSizeModifyHdl));


    meUnit = maPaperSizeController.GetCoreMetric();

    mpMasterSlide->SetSelectHdl(LINK(this, SlideBackground, AssignMasterPage));

    mpFillStyle->SetSelectHdl(LINK(this, SlideBackground, FillStyleModifyHdl));
    mpFillLB->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mpFillGrad->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mpFillAttr->SetSelectHdl(LINK(this, SlideBackground, FillBackgroundHdl));

    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
    if (pMainViewShell)
    {
        FrameView *pFrameView = pMainViewShell->GetFrameView();

        if ( pFrameView->GetViewShEditMode() ==  EM_PAGE )
        {
            DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
            SdPage* mpPage = pDrawViewShell->getCurrentPage();
            populateMasterSlideDropdown();

            OUString aLayoutName( mpPage->GetLayoutName() );
            aLayoutName = aLayoutName.copy(0,aLayoutName.indexOf(SD_LT_SEPARATOR));
            mpMasterSlide->SelectEntry(aLayoutName);
        }
    }

    mpFillStyle->SelectEntryPos(0);

    mpDspMasterBackground->SetClickHdl(LINK(this, SlideBackground, DspBackground));
    mpDspMasterObjects->SetClickHdl(LINK(this,SlideBackground, DspObjects));

    Update();
}

void SlideBackground::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if (maContext == rContext)
        return;
    maContext = rContext;
}

void SlideBackground::Update()
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if (!pSh)
        return;

    switch(eXFS)
    {
        case drawing::FillStyle_NONE:
        {
            mpFillLB->Hide();
            mpFillAttr->Hide();
            mpFillGrad->Hide();
        }
        break;
        case drawing::FillStyle_SOLID:
        {
            mpFillAttr->Hide();
            mpFillGrad->Hide();
            mpFillLB->Show();
            mpFillLB->Clear();
            const SvxColorListItem aItem( *static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE)));
            mpFillLB->Fill(aItem.GetColorList());

            const Color aColor = GetColorSetOrDefault();
            mpFillLB->SelectEntry( aColor );

            if(mpFillLB->GetSelectEntryCount() == 0)
            {
                mpFillLB->InsertEntry(aColor, OUString());
                mpFillLB->SelectEntry(aColor);
            }
        }
        break;
        case drawing::FillStyle_GRADIENT:
        {
            mpFillLB->Show();
            const SvxColorListItem aItem(*static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE)));
            mpFillAttr->Hide();
            mpFillGrad->Show();
            mpFillLB->Clear();
            mpFillGrad->Clear();
            mpFillLB->Fill(aItem.GetColorList());
            mpFillGrad->Fill(aItem.GetColorList());

            const XGradient xGradient = GetGradientSetOrDefault();
            const Color aStartColor = xGradient.GetStartColor();
            const Color aEndColor = xGradient.GetEndColor();
            mpFillLB->SelectEntry( aStartColor );
            mpFillGrad->SelectEntry( aEndColor );

            if(mpFillLB->GetSelectEntryCount() == 0)
            {
                mpFillLB->InsertEntry(aStartColor, OUString());
                mpFillLB->SelectEntry(aStartColor);
            }

            if(mpFillGrad->GetSelectEntryCount() == 0)
            {
                mpFillGrad->InsertEntry(aEndColor, OUString());
                mpFillGrad->SelectEntry(aEndColor);
            }
        }
        break;

        case drawing::FillStyle_HATCH:
        {
            mpFillLB->Hide();
            const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
            mpFillAttr->Show();
            mpFillAttr->Clear();
            mpFillAttr->Fill(aItem.GetHatchList());
            mpFillGrad->Hide();

            const OUString aHatchName = GetHatchingSetOrDefault();
            mpFillAttr->SelectEntry( aHatchName );
        }
        break;

        case drawing::FillStyle_BITMAP:
        {
            mpFillLB->Hide();
            const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
            mpFillAttr->Show();
            mpFillAttr->Clear();
            mpFillAttr->Fill(aItem.GetBitmapList());
            mpFillGrad->Hide();

            const OUString aBitmapName = GetBitmapSetOrDefault();
            mpFillAttr->SelectEntry( aBitmapName );
        }
        break;

        default:
            break;
    }
}

void SlideBackground::SetPanelTitle( const OUString& rTitle )
{
    Reference<frame::XController2> xController( mxFrame->getController(), uno::UNO_QUERY);
    if ( !xController.is() )
        return;

    Reference<ui::XSidebarProvider> xSidebarProvider( xController->getSidebar(), uno::UNO_QUERY );
    if ( !xSidebarProvider.is() )
        return;

    Reference<ui::XDecks> xDecks ( xSidebarProvider->getDecks(), uno::UNO_QUERY);
    if ( !xDecks.is() )
        return;

    Reference<ui::XDeck> xDeck ( xDecks->getByName("PropertyDeck"), uno::UNO_QUERY);
    if ( !xDeck.is() )
        return;

    Reference<ui::XPanels> xPanels ( xDeck->getPanels(), uno::UNO_QUERY);
    if ( !xPanels.is() )
        return;

    Reference<ui::XPanel> xPanel ( xPanels->getByName("SlideBackgroundPanel"), uno::UNO_QUERY);
    if ( !xPanel.is() )
        return;

    xPanel->setTitle( rTitle );
}

void SlideBackground::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this, SlideBackground, EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener (
        aLink,
        tools::EventMultiplexerEvent::EID_CURRENT_PAGE |
        tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED |
        tools::EventMultiplexerEvent::EID_SHAPE_CHANGED |
        tools::EventMultiplexerEvent::EID_VIEW_ADDED);
}

void SlideBackground::removeListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this, SlideBackground, EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK_TYPED(SlideBackground, EventMultiplexerListener,
                tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        // add more events as per requirement
        // Master Page change triggers a shape change event. Solves sync problem.
        case tools::EventMultiplexerEvent::EID_SHAPE_CHANGED:
            populateMasterSlideDropdown();
            break;
        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
        {
            ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();

            if (pMainViewShell)
            {
                DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
                EditMode eMode = pDrawViewShell->GetEditMode();

                if ( eMode == EM_MASTERPAGE)
                    mpMasterSlide->Disable();
            }
        }
        break;
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        {
            static sal_uInt16 SidArray[] = {
                SID_ATTR_PAGE_COLOR,
                SID_ATTR_PAGE_GRADIENT,
                SID_ATTR_PAGE_HATCH,
                SID_ATTR_PAGE_BITMAP,
                SID_ATTR_PAGE_FILLSTYLE,
                SID_DISPLAY_MASTER_BACKGROUND,
                SID_DISPLAY_MASTER_OBJECTS,
                0 };
            updateMasterSlideSelection();
            GetBindings()->Invalidate( SidArray );
        }
        break;
        case tools::EventMultiplexerEvent::EID_VIEW_ADDED:
        {
            if(!mbTitle)
            {
                sfx2::sidebar::EnumContext rDrawContext(sfx2::sidebar::EnumContext::Application_Draw,
                                                        sfx2::sidebar::EnumContext::Context_DrawPage);
                sfx2::sidebar::EnumContext rImpressContext(sfx2::sidebar::EnumContext::Application_Impress,
                                                        sfx2::sidebar::EnumContext::Context_DrawPage);
                if(maContext == rDrawContext)
                {
                    SetPanelTitle(SD_RESSTR(STR_PAGE_NAME));
                    mbTitle = true;
                }
                else if(maContext == rImpressContext)
                {
                    SetPanelTitle(SD_RESSTR(STR_SLIDE_NAME));
                    mbTitle = true;
                }
            }
        }
        break;
        default:
            break;
    }
}

void SlideBackground::populateMasterSlideDropdown()
{
    mpMasterSlide->Clear();
    ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell*>( SfxObjectShell::Current() );
    SdDrawDocument* pDoc = pDocSh ? pDocSh->GetDoc() : nullptr;
    sal_uInt16 nCount = pDoc ? pDoc->GetMasterPageCount() : 0;
    for( sal_uInt16 nLayout = 0; nLayout < nCount; nLayout++ )
    {
        SdPage* pMaster = static_cast<SdPage*>(pDoc->GetMasterPage(nLayout));
        if( pMaster->GetPageKind() == PK_STANDARD)
        {
            OUString aLayoutName(pMaster->GetLayoutName());
            aLayoutName = aLayoutName.copy(0,aLayoutName.indexOf(SD_LT_SEPARATOR));
            mpMasterSlide->InsertEntry(aLayoutName);
        }
    }
    updateMasterSlideSelection();
}

void SlideBackground::updateMasterSlideSelection()
{
    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
    SdPage* pPage = pMainViewShell ? pMainViewShell->getCurrentPage() : nullptr;
    if (pPage != nullptr && pPage->TRG_HasMasterPage())
    {
        SdrPage& rMasterPage (pPage->TRG_GetMasterPage());
        SdPage* pMasterPage = static_cast<SdPage*>(&rMasterPage);
        mpMasterSlide->SelectEntry(pMasterPage->GetName());
    }
    else
        mpMasterSlide->SetNoSelection();
}

void SlideBackground::dispose()
{
    removeListener();

    mpPaperSizeBox.clear();
    mpPaperOrientation.clear();
    mpMasterSlide.clear();
    mpFillAttr.clear();
    mpFillGrad.clear();
    mpFillStyle.clear();
    mpFillLB.clear();
    mpDspMasterBackground.clear();
    mpDspMasterObjects.clear();

    maPaperSizeController.dispose();
    maPaperOrientationController.dispose();
    maBckColorController.dispose();
    maBckGradientController.dispose();
    maBckHatchController.dispose();
    maBckBitmapController.dispose();
    maBckFillStyleController.dispose();
    maBckImageController.dispose();
    maDspBckController.dispose();
    maDspObjController.dispose();
    maMetricController.dispose();

    PanelLayout::dispose();
}

Color SlideBackground::GetColorSetOrDefault()
{
   // Tango Sky Blue 1, to be consistent w/ area fill panel (b/c COL_AUTO for slides is transparent)
   if ( !mpColorItem )
        mpColorItem.reset( new XFillColorItem( OUString(), Color(0x72, 0x9f, 0xcf) ) );

   return mpColorItem->GetColorValue();
}

XGradient SlideBackground::GetGradientSetOrDefault()
{
    if( !mpGradientItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxGradientListItem aGradListItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
        const XGradient aGradient = aGradListItem.GetGradientList()->GetGradient(0)->GetGradient();
        const OUString aGradientName = aGradListItem.GetGradientList()->GetGradient(0)->GetName();

        mpGradientItem.reset( new XFillGradientItem( aGradientName, aGradient ) );
    }

    return mpGradientItem->GetGradientValue();
}

const OUString SlideBackground::GetHatchingSetOrDefault()
{
    if( !mpHatchItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxHatchListItem aHatchListItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
        const XHatch aHatch = aHatchListItem.GetHatchList()->GetHatch(0)->GetHatch();
        const OUString aHatchName = aHatchListItem.GetHatchList()->GetHatch(0)->GetName();

        mpHatchItem.reset( new XFillHatchItem( aHatchName, aHatch ) );
    }

    return mpHatchItem->GetName();
}

const OUString SlideBackground::GetBitmapSetOrDefault()
{
    if( !mpBitmapItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxBitmapListItem aBmpListItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
        const GraphicObject aGraphObj = aBmpListItem.GetBitmapList()->GetBitmap(0)->GetGraphicObject();
        const OUString aBmpName = aBmpListItem.GetBitmapList()->GetBitmap(0)->GetName();

        mpBitmapItem.reset( new XFillBitmapItem( aBmpName, aGraphObj ) );
    }

    return mpBitmapItem->GetName();
}

void SlideBackground::DataChanged (const DataChangedEvent& /*rEvent*/)
{

}

void SlideBackground::NotifyItemUpdate(
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch(nSID)
    {

        case SID_ATTR_PAGE_COLOR:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(1);
                mpColorItem.reset(pState ? static_cast< XFillColorItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_HATCH:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(3);
                mpHatchItem.reset(pState ? static_cast < XFillHatchItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_GRADIENT:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(2);
                mpGradientItem.reset(pState ? static_cast< XFillGradientItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;
        case SID_ATTR_PAGE_BITMAP:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(4);
                mpBitmapItem.reset(pState ? static_cast< XFillBitmapItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_FILLSTYLE:
        {
            const XFillStyleItem* pFillStyleItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pFillStyleItem = dynamic_cast< const XFillStyleItem* >(pState);
            if (pFillStyleItem)
            {
                css::drawing::FillStyle eXFS = pFillStyleItem->GetValue();
                switch(eXFS)
                {
                    case drawing::FillStyle_NONE:
                        mpFillStyle->SelectEntryPos(0);
                        break;
                    case drawing::FillStyle_SOLID:
                        mpFillStyle->SelectEntryPos(1);
                        break;
                    case drawing::FillStyle_GRADIENT:
                        mpFillStyle->SelectEntryPos(2);
                        break;
                    case drawing::FillStyle_HATCH:
                        mpFillStyle->SelectEntryPos(3);
                        break;
                    case drawing::FillStyle_BITMAP:
                        mpFillStyle->SelectEntryPos(4);
                        break;
                    default:
                        break;
                }
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_SIZE:
        {
            const SvxSizeItem* pSizeItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pSizeItem = dynamic_cast< const SvxSizeItem* >(pState);
            if (pSizeItem)
            {
                Size aPaperSize = pSizeItem->GetSize();
                if(mpPaperOrientation->GetSelectEntryPos() == 0)
                   Swap(aPaperSize);

                Paper ePaper = SvxPaperInfo::GetSvxPaper(aPaperSize, static_cast<MapUnit>(meUnit),true);
                sal_Int32 nEntryCount = mpPaperSizeBox->GetEntryCount();

                for (sal_Int32 i = 0; i < nEntryCount; ++i )
                {
                    Paper eTmp = (Paper)reinterpret_cast<sal_uLong>(mpPaperSizeBox->GetEntryData(i));

                    if ( eTmp == ePaper )
                    {
                        mpPaperSizeBox->SelectEntryPos(i);
                        break;
                    }
                }
            }
        }
        break;

        case SID_ATTR_PAGE:
        {
            const SvxPageItem* pPageItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pPageItem = dynamic_cast< const SvxPageItem* >(pState);
            if (pPageItem)
            {
                bool bIsLandscape = pPageItem->IsLandscape();

                mpPaperOrientation->SelectEntryPos( bIsLandscape ? 0 : 1 );
            }
        }
        break;

        case SID_DISPLAY_MASTER_BACKGROUND:
        {
            const SfxBoolItem* pBoolItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pBoolItem = dynamic_cast< const SfxBoolItem* >(pState);
            if (pBoolItem)
                mpDspMasterBackground->Check(pBoolItem->GetValue());
        }
        break;
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            const SfxBoolItem* pBoolItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pBoolItem = dynamic_cast< const SfxBoolItem* >(pState);
            if (pBoolItem)
                mpDspMasterObjects->Check(pBoolItem->GetValue());
        }
        break;

        case SID_SELECT_BACKGROUND:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(4);
                Update();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED(SlideBackground, FillStyleModifyHdl, ListBox&, void)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    Update();

    switch (eXFS)
    {
        case drawing::FillStyle_NONE:
        {
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_FILLSTYLE, SfxCallMode::RECORD, { &aXFillStyleItem });
        }
        break;

        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aItem( OUString(), mpColorItem->GetColorValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case drawing::FillStyle_GRADIENT:
        {
            XFillGradientItem aItem( mpGradientItem->GetName(), mpGradientItem->GetGradientValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case drawing::FillStyle_HATCH:
        {
            XFillHatchItem aItem( mpHatchItem->GetName(), mpHatchItem->GetHatchValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case drawing::FillStyle_BITMAP:
        {
            XFillBitmapItem aItem( mpBitmapItem->GetName(), mpBitmapItem->GetGraphicObject() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
        }
        break;

        default:
        break;
    }
    mpFillStyle->Selected();
}

IMPL_LINK_NOARG_TYPED(SlideBackground, PaperSizeModifyHdl, ListBox&, void)
{
    sal_uInt32 nPos = mpPaperSizeBox->GetSelectEntryPos();
    Paper ePaper = (Paper)reinterpret_cast<sal_uLong>( mpPaperSizeBox->GetEntryData( nPos ) );
    Size  aSize(SvxPaperInfo::GetPaperSize(ePaper, (MapUnit)(meUnit)));

    if(mpPaperOrientation->GetSelectEntryPos() == 0)
        Swap(aSize);

    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE,aSize);
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem });
}

IMPL_LINK_NOARG_TYPED(SlideBackground, FillColorHdl, ListBox&, void)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    switch(eXFS)
    {
        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aItem(OUString(), mpFillLB->GetSelectEntryColor());
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case drawing::FillStyle_GRADIENT:
        {
            XGradient aGradient;
            aGradient.SetStartColor(mpFillLB->GetSelectEntryColor());
            aGradient.SetEndColor(mpFillGrad->GetSelectEntryColor());

            XFillGradientItem aItem(aGradient);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED(SlideBackground, FillBackgroundHdl, ListBox&, void)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpFillStyle->GetSelectEntryPos();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {

        case drawing::FillStyle_HATCH:
        {
            const SvxHatchListItem aHatchListItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
            sal_uInt16 nPos = mpFillAttr->GetSelectEntryPos();
            XHatch aHatch = aHatchListItem.GetHatchList()->GetHatch(nPos)->GetHatch();
            const OUString aHatchName = aHatchListItem.GetHatchList()->GetHatch(nPos)->GetName();

            XFillHatchItem aItem(aHatchName, aHatch);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case drawing::FillStyle_BITMAP:
        {
            SvxBitmapListItem aBitmapListItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
            sal_Int16 nPos = mpFillAttr->GetSelectEntryPos();
            GraphicObject aBitmap = aBitmapListItem.GetBitmapList()->GetBitmap(nPos)->GetGraphicObject();
            OUString aBitmapName = aBitmapListItem.GetBitmapList()->GetBitmap(nPos)->GetName();

            XFillBitmapItem aItem(aBitmapName, aBitmap);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED(SlideBackground, AssignMasterPage, ListBox&, void)
{
    ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell*>( SfxObjectShell::Current() );
    SdDrawDocument* pDoc = pDocSh ? pDocSh->GetDoc() : nullptr;
    if (!pDoc)
        return;
    sal_uInt16 nSelectedPage = SDRPAGE_NOTFOUND;
    for( sal_uInt16 nPage = 0; nPage < pDoc->GetSdPageCount(PK_STANDARD); nPage++ )
    {
        if (pDoc->GetSdPage(nPage,PK_STANDARD)->IsSelected())
        {
            nSelectedPage = nPage;
            break;
        }
    }
    OUString aLayoutName(mpMasterSlide->GetSelectEntry());
    pDoc->SetMasterPage(nSelectedPage, aLayoutName, pDoc, false, false);
}

IMPL_LINK_NOARG_TYPED(SlideBackground, DspBackground, Button*, void)
{
    bool IsChecked = mpDspMasterBackground->IsChecked();
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_BACKGROUND, IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList(SID_DISPLAY_MASTER_BACKGROUND, SfxCallMode::RECORD, { &aBoolItem });
}

IMPL_LINK_NOARG_TYPED(SlideBackground, DspObjects, Button*, void)
{
    bool IsChecked = mpDspMasterObjects->IsChecked();
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_OBJECTS,IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList(SID_DISPLAY_MASTER_OBJECTS, SfxCallMode::RECORD, { &aBoolItem, &aBoolItem });
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
