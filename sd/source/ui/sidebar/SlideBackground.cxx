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
#include <svx/colorbox.hxx>
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
#include <vcl/salbtype.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace sd { namespace sidebar {

enum eFillStyle
{
    NONE,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

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
    maCloseMasterController(SID_CLOSE_MASTER_VIEW, *pBindings, *this),
    mpPageItem( new SvxPageItem(SID_ATTR_PAGE) ),
    mpColorItem(),
    mpGradientItem(),
    mpHatchItem(),
    mpBitmapItem(),
    mbEditModeChangePending(false),
    mxFrame(rxFrame),
    maContext(),
    maApplication(vcl::EnumContext::Application::NONE),
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
    get(mpCloseMaster, "closemasterslide");
    get(mpEditMaster, "masterslidebutton");
    get(mpMasterLabel, "masterlabel");
    addListener();
    Initialize();
}

SlideBackground::~SlideBackground()
{
    disposeOnce();
}

bool SlideBackground::IsImpress()
{
    return ( maApplication == vcl::EnumContext::Application::Impress );
}

void SlideBackground::Initialize()
{
    mpPaperSizeBox->FillPaperSizeEntries( PaperSizeApp::Draw );
    mpPaperSizeBox->SetSelectHdl(LINK(this,SlideBackground,PaperSizeModifyHdl));
    mpPaperOrientation->SetSelectHdl(LINK(this,SlideBackground,PaperSizeModifyHdl));
    mpCloseMaster->Hide();
    mpCloseMaster->SetClickHdl(LINK(this, SlideBackground, CloseMasterHdl));
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

        if ( pFrameView->GetViewShEditMode() ==  EditMode::Page )
        {
            DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
            SdPage* mpPage = pDrawViewShell->getCurrentPage();
            populateMasterSlideDropdown();

            OUString aLayoutName( mpPage->GetLayoutName() );
            aLayoutName = aLayoutName.copy(0,aLayoutName.indexOf(SD_LT_SEPARATOR));
            mpMasterSlide->SelectEntry(aLayoutName);
        }
    }

    mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(NONE));

    mpDspMasterBackground->SetClickHdl(LINK(this, SlideBackground, DspBackground));
    mpDspMasterObjects->SetClickHdl(LINK(this,SlideBackground, DspObjects));

    Update();
}

void SlideBackground::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;
    maContext = rContext;
}

void SlideBackground::Update()
{
    const eFillStyle nPos = (eFillStyle)mpFillStyle->GetSelectEntryPos();

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if (!pSh)
        return;

    switch(nPos)
    {
        case NONE:
        {
            mpFillLB->Hide();
            mpFillAttr->Hide();
            mpFillGrad->Hide();
        }
        break;
        case SOLID:
        {
            mpFillAttr->Hide();
            mpFillGrad->Hide();
            mpFillLB->Show();
            const Color aColor = GetColorSetOrDefault();
            mpFillLB->SelectEntry(aColor);
        }
        break;
        case GRADIENT:
        {
            mpFillLB->Show();
            mpFillAttr->Hide();
            mpFillGrad->Show();

            const XGradient xGradient = GetGradientSetOrDefault();
            const Color aStartColor = xGradient.GetStartColor();
            mpFillLB->SelectEntry(aStartColor);
            const Color aEndColor = xGradient.GetEndColor();
            mpFillGrad->SelectEntry(aEndColor);
        }
        break;

        case HATCH:
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

        case BITMAP:
        case PATTERN:
        {
            mpFillLB->Hide();
            mpFillAttr->Show();
            mpFillAttr->Clear();
            mpFillGrad->Hide();
            OUString aName;
            if(nPos == BITMAP)
            {
                const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
                mpFillAttr->Fill(aItem.GetBitmapList());
                aName = GetBitmapSetOrDefault();
            }
            else if(nPos == PATTERN)
            {
                const SvxPatternListItem aItem(*static_cast<const SvxPatternListItem*>(pSh->GetItem(SID_PATTERN_LIST)));
                mpFillAttr->Fill(aItem.GetPatternList());
                aName = GetPatternSetOrDefault();
            }
            mpFillAttr->SelectEntry( aName );
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
    if ( !xPanels.is() )
        return;

    xPanel->setTitle( rTitle );
}

void SlideBackground::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this, SlideBackground, EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener( aLink );
}

void SlideBackground::removeListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this, SlideBackground, EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK(SlideBackground, EventMultiplexerListener,
                tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        // add more events as per requirement
        // Master Page change triggers a shape change event. Solves sync problem.
        case EventMultiplexerEventId::ShapeChanged:
            populateMasterSlideDropdown();
            break;
        case EventMultiplexerEventId::EditModeNormal:
        case EventMultiplexerEventId::EditModeMaster:
            mbEditModeChangePending = true;
            break;
        case EventMultiplexerEventId::EditViewSelection:
        case EventMultiplexerEventId::EndTextEdit:
        {
            if (mbEditModeChangePending)
            {
                ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();

                if (pMainViewShell)
                {
                    DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
                    EditMode eMode = pDrawViewShell->GetEditMode();

                    if ( eMode == EditMode::MasterPage)
                    {
                        if( IsImpress() )
                        {
                            SetPanelTitle(SD_RESSTR(STR_MASTERSLIDE_NAME));
                            mpEditMaster->Hide();
                            mpCloseMaster->Show();
                        }
                        else
                            SetPanelTitle(SD_RESSTR(STR_MASTERPAGE_NAME));
                        mpMasterSlide->Disable();
                        mpDspMasterBackground->Disable();
                        mpDspMasterObjects->Disable();
                    }
                    else // EditMode::Page
                    {
                        if( IsImpress() )
                        {
                            SetPanelTitle(SD_RESSTR(STR_SLIDE_NAME));
                            mpCloseMaster->Hide();
                            mpEditMaster->Show();
                        }
                        else
                            SetPanelTitle(SD_RESSTR(STR_PAGE_NAME));
                        mpMasterSlide->Enable();
                        mpDspMasterBackground->Enable();
                        mpDspMasterObjects->Enable();
                    }
                }
                mbEditModeChangePending = false;
            }
        }
        break;
        case EventMultiplexerEventId::CurrentPageChanged:
        {
            static const sal_uInt16 SidArray[] = {
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
        case EventMultiplexerEventId::ViewAdded:
        {
            if(!mbTitle)
            {
                vcl::EnumContext aDrawOtherContext(vcl::EnumContext::Application::Draw,
                                              vcl::EnumContext::Context::DrawPage);
                vcl::EnumContext aDrawMasterContext(vcl::EnumContext::Application::Draw,
                                              vcl::EnumContext::Context::MasterPage);
                vcl::EnumContext aImpressOtherContext(vcl::EnumContext::Application::Impress,
                                                 vcl::EnumContext::Context::DrawPage);
                vcl::EnumContext aImpressMasterContext(vcl::EnumContext::Application::Impress,
                                                       vcl::EnumContext::Context::MasterPage);
                if(maContext == aDrawOtherContext || maContext == aDrawMasterContext)
                {
                    mpMasterLabel->SetText(SD_RESSTR(STR_MASTERPAGE_NAME));
                    maApplication = vcl::EnumContext::Application::Draw;
                    mpCloseMaster->Hide();
                    mpEditMaster->Hide();
                    if( maContext == aDrawMasterContext)
                        SetPanelTitle(SD_RESSTR(STR_MASTERPAGE_NAME));
                    else
                        SetPanelTitle(SD_RESSTR(STR_PAGE_NAME));
                }
                else if ( maContext == aImpressOtherContext || maContext == aImpressMasterContext )
                {
                    mpMasterLabel->SetText(SD_RESSTR(STR_MASTERSLIDE_NAME));
                    maApplication = vcl::EnumContext::Application::Impress;
                    mpCloseMaster->Hide();
                    mpEditMaster->Show();
                    if( maContext == aImpressMasterContext )
                        SetPanelTitle(SD_RESSTR(STR_MASTERSLIDE_NAME));
                    else
                        SetPanelTitle(SD_RESSTR(STR_SLIDE_NAME));
                }
                mbTitle = true;
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
        if( pMaster->GetPageKind() == PageKind::Standard)
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
    mpMasterLabel.clear();
    mpEditMaster.clear();
    mpCloseMaster.clear();

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
    maCloseMasterController.dispose();

    mpPageItem.reset();
    mpColorItem.reset();
    mpHatchItem.reset();
    mpBitmapItem.reset();
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
    if( !mpBitmapItem || mpBitmapItem->isPattern())
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxBitmapListItem aBmpListItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
        const GraphicObject aGraphObj = aBmpListItem.GetBitmapList()->GetBitmap(0)->GetGraphicObject();
        const OUString aBmpName = aBmpListItem.GetBitmapList()->GetBitmap(0)->GetName();

        mpBitmapItem.reset( new XFillBitmapItem( aBmpName, aGraphObj ) );
    }

    return mpBitmapItem->GetName();
}

const OUString SlideBackground::GetPatternSetOrDefault()
{
    if( !mpBitmapItem || !(mpBitmapItem->isPattern()))
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxPatternListItem aPtrnListItem(*static_cast<const SvxPatternListItem*>(pSh->GetItem(SID_PATTERN_LIST)));
        const GraphicObject aGraphObj = aPtrnListItem.GetPatternList()->GetBitmap(0)->GetGraphicObject();
        const OUString aPtrnName = aPtrnListItem.GetPatternList()->GetBitmap(0)->GetName();

        mpBitmapItem.reset( new XFillBitmapItem( aPtrnName, aGraphObj ) );
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
                mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(SOLID));
                mpColorItem.reset(pState ? static_cast< XFillColorItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_HATCH:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(HATCH));
                mpHatchItem.reset(pState ? static_cast < XFillHatchItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_GRADIENT:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpFillStyle->SelectEntryPos(static_cast< sal_Int32>(GRADIENT));
                mpGradientItem.reset(pState ? static_cast< XFillGradientItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;
        case SID_ATTR_PAGE_BITMAP:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpBitmapItem.reset(pState ? static_cast< XFillBitmapItem* >(pState->Clone()) : nullptr);
                if(mpBitmapItem)
                {
                    if(mpBitmapItem->isPattern())
                        mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(PATTERN));
                    else
                        mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(BITMAP));
                }
                else
                    mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(BITMAP));
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
                        mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(NONE));
                        break;
                    case drawing::FillStyle_SOLID:
                        mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(SOLID));
                        break;
                    case drawing::FillStyle_GRADIENT:
                        mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(GRADIENT));
                        break;
                    case drawing::FillStyle_HATCH:
                        mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(HATCH));
                        break;
                    case drawing::FillStyle_BITMAP:
                    {
                        if(mpBitmapItem->isPattern())
                            mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(PATTERN));
                        else
                            mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(BITMAP));
                    }
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

                Paper ePaper = SvxPaperInfo::GetSvxPaper(aPaperSize, meUnit, true);
                mpPaperSizeBox->SetSelection( ePaper );
            }
        }
        break;

        case SID_ATTR_PAGE:
        {
            if (eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast< const SvxPageItem *>( pState ) !=  nullptr)
            {
                mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
                bool bIsLandscape = mpPageItem->IsLandscape();
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
                mpFillStyle->SelectEntryPos(static_cast< sal_Int32 >(BITMAP));
                Update();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(SlideBackground, FillStyleModifyHdl, ListBox&, void)
{
    const eFillStyle nPos = (eFillStyle)mpFillStyle->GetSelectEntryPos();
    Update();

    switch (nPos)
    {
        case NONE:
        {
            const XFillStyleItem aXFillStyleItem(drawing::FillStyle_NONE);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_FILLSTYLE, SfxCallMode::RECORD, { &aXFillStyleItem });
        }
        break;

        case SOLID:
        {
            const XFillColorItem aItem( OUString(), mpColorItem->GetColorValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case GRADIENT:
        {
            const XFillGradientItem aItem( mpGradientItem->GetName(), mpGradientItem->GetGradientValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case HATCH:
        {
            const XFillHatchItem aItem( mpHatchItem->GetName(), mpHatchItem->GetHatchValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            const XFillBitmapItem aItem( mpBitmapItem->GetName(), mpBitmapItem->GetGraphicObject() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
        }
        break;

        default:
        break;
    }
    mpFillStyle->Selected();
}

IMPL_LINK_NOARG(SlideBackground, PaperSizeModifyHdl, ListBox&, void)
{
    Paper ePaper =  mpPaperSizeBox->GetSelection();
    Size  aSize(SvxPaperInfo::GetPaperSize(ePaper, (MapUnit)(meUnit)));

    if(mpPaperOrientation->GetSelectEntryPos() == 0)
        Swap(aSize);

    mpPageItem->SetLandscape(mpPaperOrientation->GetSelectEntryPos() == 0);
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    // Page/slide properties dialog (FuPage::ExecuteDialog and ::ApplyItemSet) misuses
    // SID_ATTR_PAGE_EXT1 to distinguish between Impress and Draw, as for whether to fit
    // objects to paper size. Until that is handled somehow better, we do the same here
    SfxBoolItem aFitObjs(SID_ATTR_PAGE_EXT1, IsImpress());

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem, mpPageItem.get(), &aFitObjs});
}

IMPL_LINK_NOARG(SlideBackground, FillColorHdl, SvxColorListBox&, void)
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

IMPL_LINK_NOARG(SlideBackground, FillBackgroundHdl, ListBox&, void)
{
    const eFillStyle nFillPos = (eFillStyle)mpFillStyle->GetSelectEntryPos();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(nFillPos)
    {

        case HATCH:
        {
            const SvxHatchListItem aHatchListItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
            sal_uInt16 nPos = mpFillAttr->GetSelectEntryPos();
            XHatch aHatch = aHatchListItem.GetHatchList()->GetHatch(nPos)->GetHatch();
            const OUString aHatchName = aHatchListItem.GetHatchList()->GetHatch(nPos)->GetName();

            XFillHatchItem aItem(aHatchName, aHatch);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            sal_Int16 nPos = mpFillAttr->GetSelectEntryPos();
            GraphicObject aBitmap;
            OUString aName;
            if(nPos == static_cast< sal_Int32 >(BITMAP))
            {
                SvxBitmapListItem aBitmapListItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
                aBitmap = aBitmapListItem.GetBitmapList()->GetBitmap(nPos)->GetGraphicObject();
                aName = aBitmapListItem.GetBitmapList()->GetBitmap(nPos)->GetName();
            }
            else if(nPos == static_cast< sal_Int32 >(PATTERN))
            {
                SvxPatternListItem aPatternListItem(*static_cast<const SvxPatternListItem*>(pSh->GetItem(SID_PATTERN_LIST)));
                aBitmap = aPatternListItem.GetPatternList()->GetBitmap(nPos)->GetGraphicObject();
                aName = aPatternListItem.GetPatternList()->GetBitmap(nPos)->GetName();
            }
            XFillBitmapItem aItem(aName, aBitmap);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
        }
        break;

        default:
            break;
    }
}

IMPL_LINK_NOARG(SlideBackground, AssignMasterPage, ListBox&, void)
{
    ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell*>( SfxObjectShell::Current() );
    SdDrawDocument* pDoc = pDocSh ? pDocSh->GetDoc() : nullptr;
    if (!pDoc)
        return;
    sal_uInt16 nSelectedPage = SDRPAGE_NOTFOUND;
    for( sal_uInt16 nPage = 0; nPage < pDoc->GetSdPageCount(PageKind::Standard); nPage++ )
    {
        if (pDoc->GetSdPage(nPage,PageKind::Standard)->IsSelected())
        {
            nSelectedPage = nPage;
            break;
        }
    }
    OUString aLayoutName(mpMasterSlide->GetSelectEntry());
    pDoc->SetMasterPage(nSelectedPage, aLayoutName, pDoc, false, false);
}

IMPL_LINK_NOARG(SlideBackground, CloseMasterHdl, Button*, void)
{
    GetBindings()->GetDispatcher()->Execute( SID_CLOSE_MASTER_VIEW, SfxCallMode::RECORD );
}

IMPL_LINK_NOARG(SlideBackground, DspBackground, Button*, void)
{
    bool IsChecked = mpDspMasterBackground->IsChecked();
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_BACKGROUND, IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList(SID_DISPLAY_MASTER_BACKGROUND, SfxCallMode::RECORD, { &aBoolItem });
}

IMPL_LINK_NOARG(SlideBackground, DspObjects, Button*, void)
{
    bool IsChecked = mpDspMasterObjects->IsChecked();
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_OBJECTS,IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList(SID_DISPLAY_MASTER_OBJECTS, SfxCallMode::RECORD, { &aBoolItem, &aBoolItem });
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
