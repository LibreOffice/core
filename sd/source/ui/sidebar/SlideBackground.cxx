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

#include <com/sun/star/ui/XDeck.hpp>
#include <com/sun/star/ui/XPanel.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include "SlideBackground.hxx"
#include <sdresid.hxx>
#include <ViewShellBase.hxx>
#include <FrameView.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include "PageMarginUtils.hxx"
#include <strings.hrc>
#include <pageformatpanel.hrc>
#include <DrawViewShell.hxx>
#include <svl/intitem.hxx>
#include <svx/colorbox.hxx>
#include <svx/dlgutil.hxx>
#include <svx/drawitem.hxx>
#include <svx/itemwin.hxx>
#include <svx/pageitem.hxx>
#include <app.hrc>
#include <editeng/paperinf.hxx>
#include <svx/xflgrit.hxx>
#include <svx/rulritem.hxx>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <EventMultiplexer.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/svapp.hxx>

#include <editeng/sizeitem.hxx>
#include <comphelper/lok.hxx>
#include <unomodel.hxx>
#include <sfx2/lokhelper.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace sd::sidebar {

namespace {

enum eFillStyle
{
    NONE,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

}

SlideBackground::SlideBackground(
    Window * pParent,
    ViewShellBase& rBase,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings
     ) :
    PanelLayout( pParent, "SlideBackgroundPanel", "modules/simpress/ui/sidebarslidebackground.ui", rxFrame ),
    mrBase( rBase ),
    mxPaperSizeBox(new SvxPaperSizeListBox(m_xBuilder->weld_combo_box("paperformat"))),
    mxPaperOrientation(m_xBuilder->weld_combo_box("orientation")),
    mxMasterSlide(m_xBuilder->weld_combo_box("masterslide")),
    mxBackgroundLabel(m_xBuilder->weld_label("label3")),
    mxFillStyle(m_xBuilder->weld_combo_box("fillstyle")),
    mxFillLB(new ColorListBox(m_xBuilder->weld_menu_button("fillattr"), GetFrameWeld())),
    mxFillAttr(m_xBuilder->weld_combo_box("fillattr1")),
    mxFillGrad1(new ColorListBox(m_xBuilder->weld_menu_button("fillattr2"), GetFrameWeld())),
    mxFillGrad2(new ColorListBox(m_xBuilder->weld_menu_button("fillattr3"), GetFrameWeld())),
    mxInsertImage(m_xBuilder->weld_button("button2")),
    mxDspMasterBackground(m_xBuilder->weld_check_button("displaymasterbackground")),
    mxDspMasterObjects(m_xBuilder->weld_check_button("displaymasterobjects")),
    mxCloseMaster(m_xBuilder->weld_button("closemasterslide")),
    mxEditMaster(m_xBuilder->weld_button("masterslidebutton")),
    mxMasterLabel(m_xBuilder->weld_label("masterlabel")),
    mxMarginSelectBox(m_xBuilder->weld_combo_box("marginLB")),
    mxCustomEntry(m_xBuilder->weld_label("customlabel")),
    mxMarginLabel(m_xBuilder->weld_label("labelmargin")),
    maPaperSizeController(SID_ATTR_PAGE_SIZE, *pBindings, *this),
    maPaperOrientationController(SID_ATTR_PAGE, *pBindings, *this),
    maPaperMarginLRController(SID_ATTR_PAGE_LRSPACE, *pBindings, *this),
    maPaperMarginULController(SID_ATTR_PAGE_ULSPACE, *pBindings, *this),
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
    mbSwitchModeToNormal(false),
    mbSwitchModeToMaster(false),
    mxFrame(rxFrame),
    maContext(),
    maDrawOtherContext(vcl::EnumContext::Application::Draw, vcl::EnumContext::Context::DrawPage),
    maDrawMasterContext(vcl::EnumContext::Application::Draw, vcl::EnumContext::Context::MasterPage),
    maImpressOtherContext(vcl::EnumContext::Application::Impress, vcl::EnumContext::Context::DrawPage),
    maImpressMasterContext(vcl::EnumContext::Application::Impress, vcl::EnumContext::Context::MasterPage),
    maImpressHandoutContext(vcl::EnumContext::Application::Impress, vcl::EnumContext::Context::HandoutPage),
    maImpressNotesContext(vcl::EnumContext::Application::Impress, vcl::EnumContext::Context::NotesPage),
    mbTitle(false),
    mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) ),
    mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) ),
    m_nPageLeftMargin(0),
    m_nPageRightMargin(0),
    m_nPageTopMargin(0),
    m_nPageBottomMargin(0),
    meFUnit(GetModuleFieldUnit()),
    maCustomEntry(),
    mpBindings(pBindings)
{
    //let the listbox shrink to any size so the sidebar isn't forced to grow to
    //the size of the longest master slide name in the document
    mxMasterSlide->set_size_request(42, -1);

    maCustomEntry = mxCustomEntry->get_label();

    addListener();
    Initialize();
}

SlideBackground::~SlideBackground()
{
    disposeOnce();
}

bool SlideBackground::IsDraw()
{
    return ( maContext == maDrawMasterContext ||
             maContext == maDrawOtherContext );
}

bool SlideBackground::IsImpress()
{
    return ( maContext == maImpressMasterContext ||
             maContext == maImpressOtherContext ||
             maContext == maImpressHandoutContext ||
             maContext == maImpressNotesContext );
}

FieldUnit SlideBackground::GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState)
{
    FieldUnit eUnit;

    if (pState && eState >= SfxItemState::DEFAULT)
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    else
        eUnit = GetModuleFieldUnit();

    return eUnit;
}

void SlideBackground::SetMarginsFieldUnit()
{
    auto nSelected = mxMarginSelectBox->get_active();
    mxMarginSelectBox->clear();

    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();

    if (IsInch(meFUnit))
    {
        OUString sSuffix = weld::MetricSpinButton::MetricToString(FieldUnit::INCH);
        for (size_t i = 0; i < SAL_N_ELEMENTS(RID_PAGEFORMATPANEL_MARGINS_INCH); ++i)
        {
            OUString sMeasurement = rLocaleData.getNum(RID_PAGEFORMATPANEL_MARGINS_INCH[i].second, 2, true, false) + sSuffix;
            mxMarginSelectBox->append_text(SdResId(RID_PAGEFORMATPANEL_MARGINS_INCH[i].first).replaceFirst("%1", sMeasurement));
        }
    }
    else
    {
        OUString sSuffix = " " + weld::MetricSpinButton::MetricToString(FieldUnit::CM);
        for (size_t i = 0; i < SAL_N_ELEMENTS(RID_PAGEFORMATPANEL_MARGINS_CM); ++i)
        {
            OUString sMeasurement = rLocaleData.getNum(RID_PAGEFORMATPANEL_MARGINS_CM[i].second, 2, true, false) + sSuffix;
            mxMarginSelectBox->append_text(SdResId(RID_PAGEFORMATPANEL_MARGINS_CM[i].first).replaceFirst("%1", sMeasurement));
        }
    }

    mxMarginSelectBox->set_active(nSelected);
}

void SlideBackground::Initialize()
{
    SvxFillTypeBox::Fill(*mxFillStyle);

    SetMarginsFieldUnit();

    mxPaperSizeBox->FillPaperSizeEntries( PaperSizeApp::Draw );
    mxPaperSizeBox->connect_changed(LINK(this,SlideBackground,PaperSizeModifyHdl));
    mxPaperOrientation->connect_changed(LINK(this,SlideBackground,PaperSizeModifyHdl));
    mxEditMaster->connect_clicked(LINK(this, SlideBackground, EditMasterHdl));
    mxCloseMaster->connect_clicked(LINK(this, SlideBackground, CloseMasterHdl));
    mxInsertImage->connect_clicked(LINK(this, SlideBackground, SelectBgHdl));
    meUnit = maPaperSizeController.GetCoreMetric();

    mxMasterSlide->connect_changed(LINK(this, SlideBackground, AssignMasterPage));

    mxFillStyle->connect_changed(LINK(this, SlideBackground, FillStyleModifyHdl));
    mxFillLB->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mxFillGrad1->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mxFillGrad2->SetSelectHdl(LINK(this, SlideBackground, FillColorHdl));
    mxFillAttr->connect_changed(LINK(this, SlideBackground, FillBackgroundHdl));

    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
    if (pMainViewShell)
    {
        FrameView *pFrameView = pMainViewShell->GetFrameView();

        if ( pFrameView->GetViewShEditMode() ==  EditMode::Page )
        {
            SdPage* mpPage = pMainViewShell->getCurrentPage();
            populateMasterSlideDropdown();

            OUString aLayoutName( mpPage->GetLayoutName() );
            aLayoutName = aLayoutName.copy(0,aLayoutName.indexOf(SD_LT_SEPARATOR));
            mxMasterSlide->set_active_text(aLayoutName);
        }
    }

    mxFillStyle->set_active(static_cast< sal_Int32 >(NONE));

    mxDspMasterBackground->connect_clicked(LINK(this, SlideBackground, DspBackground));
    mxDspMasterObjects->connect_clicked(LINK(this,SlideBackground, DspObjects));

    //margins
    mxMarginSelectBox->connect_changed(LINK(this, SlideBackground, ModifyMarginHdl));

    Update();
    UpdateMarginBox();
}

void SlideBackground::DumpAsPropertyTree(::tools::JsonWriter& rJsonWriter)
{
    if (mxPaperSizeBox->get_active() == -1)
    {
        mpBindings->Update(SID_ATTR_PAGE_SIZE);
    }

    Control::DumpAsPropertyTree(rJsonWriter);
}

void SlideBackground::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;
    maContext = rContext;

    if ( IsImpress() )
    {
        mxMasterLabel->set_label(SdResId(STR_MASTERSLIDE_LABEL));

        // margin selector is only for Draw
        mxMarginSelectBox->hide();
        mxMarginLabel->hide();

        if ( maContext == maImpressMasterContext )
        {
            mxCloseMaster->show();
            mxEditMaster->hide();
            mxMasterSlide->set_sensitive(false);
            mxMasterSlide->clear();
            mxDspMasterBackground->set_sensitive(false);
            mxDspMasterObjects->set_sensitive(false);
            mxFillStyle->hide();
            mxBackgroundLabel->hide();
            mxInsertImage->show();

            mxFillLB->hide();
            mxFillAttr->hide();
            mxFillGrad1->hide();
            mxFillGrad2->hide();
        }
        else if ( maContext == maImpressHandoutContext  || maContext == maImpressNotesContext )
        {
            mxCloseMaster->hide();
            mxEditMaster->hide();
            mxMasterSlide->set_sensitive(false);
            mxMasterSlide->clear();
            mxDspMasterBackground->set_sensitive(false);
            mxDspMasterObjects->set_sensitive(false);
            mxFillStyle->hide();
            mxBackgroundLabel->hide();
            mxInsertImage->hide();
        }
        else if (maContext == maImpressOtherContext)
        {
            mxCloseMaster->hide();
            mxEditMaster->show();
            mxMasterSlide->set_sensitive(true);
            populateMasterSlideDropdown();
            mxDspMasterBackground->set_sensitive(true);
            mxDspMasterObjects->set_sensitive(true);
            mxFillStyle->show();
            mxBackgroundLabel->show();
            mxInsertImage->show();
        }

        // The Insert Image button in the sidebar issues .uno:SelectBackground,
        // which when invoked without arguments will open the file-open-dialog
        // to prompt the user to select a file. This is useless in LOOL.
        // Hide for now so the user will only be able to use the menu to insert
        // background image, which prompts the user for file selection in the browser.
        if (comphelper::LibreOfficeKit::isActive())
            mxInsertImage->hide();

        // Need to do a relayouting, otherwise the panel size is not updated after show / hide controls
        sfx2::sidebar::Panel* pPanel = dynamic_cast<sfx2::sidebar::Panel*>(GetParent());
        if(pPanel)
            pPanel->TriggerDeckLayouting();
    }
    else if ( IsDraw() )
    {
        mxMasterLabel->set_label(SdResId(STR_MASTERPAGE_LABEL));
        mxDspMasterBackground->hide();
        mxDspMasterObjects->hide();

        if (maContext == maDrawOtherContext)
        {
            mxEditMaster->hide();
            mxFillStyle->show();
            mxBackgroundLabel->show();
        }
        else if (maContext == maDrawMasterContext)
        {
            mxFillStyle->hide();
            mxBackgroundLabel->hide();
        }
    }
}

void SlideBackground::Update()
{
    eFillStyle nPos = static_cast<eFillStyle>(mxFillStyle->get_active());

    if(maContext != maImpressOtherContext && maContext != maDrawOtherContext)
        nPos = NONE;

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if (!pSh)
        return;

    switch(nPos)
    {
        case NONE:
        {
            mxFillLB->hide();
            mxFillAttr->hide();
            mxFillGrad1->hide();
            mxFillGrad2->hide();
        }
        break;
        case SOLID:
        {
            mxFillAttr->hide();
            mxFillGrad1->hide();
            mxFillGrad2->hide();
            mxFillLB->show();
            const Color aColor = GetColorSetOrDefault();
            mxFillLB->SelectEntry(aColor);
        }
        break;
        case GRADIENT:
        {
            mxFillLB->hide();
            mxFillAttr->hide();
            mxFillGrad1->show();
            mxFillGrad2->show();

            const XGradient xGradient = GetGradientSetOrDefault();
            const Color aStartColor = xGradient.GetStartColor();
            mxFillGrad1->SelectEntry(aStartColor);
            const Color aEndColor = xGradient.GetEndColor();
            mxFillGrad2->SelectEntry(aEndColor);
        }
        break;

        case HATCH:
        {
            mxFillLB->hide();
            mxFillAttr->show();
            mxFillAttr->clear();
            SvxFillAttrBox::Fill(*mxFillAttr, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
            mxFillGrad1->hide();
            mxFillGrad2->hide();

            const OUString aHatchName = GetHatchingSetOrDefault();
            mxFillAttr->set_active_text( aHatchName );
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            mxFillLB->hide();
            mxFillAttr->show();
            mxFillAttr->clear();
            mxFillGrad1->hide();
            mxFillGrad2->hide();
            OUString aName;
            if(nPos == BITMAP)
            {
                SvxFillAttrBox::Fill(*mxFillAttr, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                aName = GetBitmapSetOrDefault();
            }
            else if(nPos == PATTERN)
            {
                SvxFillAttrBox::Fill(*mxFillAttr, pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                aName = GetPatternSetOrDefault();
            }
            mxFillAttr->set_active_text( aName );
        }
        break;
        default:
            break;
    }
}

void SlideBackground::UpdateMarginBox()
{
    m_nPageLeftMargin = mpPageLRMarginItem->GetLeft();
    m_nPageRightMargin = mpPageLRMarginItem->GetRight();
    m_nPageTopMargin = mpPageULMarginItem->GetUpper();
    m_nPageBottomMargin = mpPageULMarginItem->GetLower();

    int nCustomIndex = mxMarginSelectBox->find_text(maCustomEntry);

    if( IsNone(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(0);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else if( IsNarrow(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(1);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else if( IsModerate(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(2);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else if( IsNormal075(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(3);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else if( IsNormal100(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(4);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else if( IsNormal125(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(5);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else if( IsWide(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin) )
    {
        mxMarginSelectBox->set_active(6);
        if (nCustomIndex != -1)
            mxMarginSelectBox->remove(nCustomIndex);
    }
    else
    {
        if (nCustomIndex == -1)
            mxMarginSelectBox->append_text(maCustomEntry);
        mxMarginSelectBox->set_active_text(maCustomEntry);
    }
}

void SlideBackground::SetPanelTitle( const OUString& rTitle )
{
    Reference<frame::XController2> xController( mxFrame->getController(), uno::UNO_QUERY);
    if ( !xController.is() )
        return;

    Reference<ui::XSidebarProvider> xSidebarProvider = xController->getSidebar();
    if ( !xSidebarProvider.is() )
        return;

    Reference<ui::XDecks> xDecks = xSidebarProvider->getDecks();
    if ( !xDecks.is() )
        return;

    Reference<ui::XDeck> xDeck ( xDecks->getByName("PropertyDeck"), uno::UNO_QUERY);
    if ( !xDeck.is() )
        return;

    Reference<ui::XPanels> xPanels = xDeck->getPanels();
    if ( !xPanels.is() )
        return;

    if (xPanels->hasByName("SlideBackgroundPanel"))
    {
        Reference<ui::XPanel> xPanel ( xPanels->getByName("SlideBackgroundPanel"), uno::UNO_QUERY);
        if ( !xPanel.is() )
            return;

        xPanel->setTitle( rTitle );
    }
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
            mbSwitchModeToNormal = true;
            break;
        case EventMultiplexerEventId::EditModeMaster:
            mbSwitchModeToMaster = true;
            break;
        case EventMultiplexerEventId::EditViewSelection:
        case EventMultiplexerEventId::EndTextEdit:
        {
            if ( mbSwitchModeToMaster )
            {
                if( IsImpress() )
                    SetPanelTitle(SdResId(STR_MASTERSLIDE_NAME));
                else
                    SetPanelTitle(SdResId(STR_MASTERPAGE_NAME));
                mbSwitchModeToMaster = false;
            }
            else if ( mbSwitchModeToNormal )
            {
                if( IsImpress() )
                    SetPanelTitle(SdResId(STR_SLIDE_NAME));
                else
                    SetPanelTitle(SdResId(STR_PAGE_NAME));
                mbSwitchModeToNormal = false;
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
                if( IsDraw() )
                {
                    mxCloseMaster->hide();
                    mxEditMaster->hide();
                    if( maContext == maDrawMasterContext)
                        SetPanelTitle(SdResId(STR_MASTERPAGE_NAME));
                    else
                        SetPanelTitle(SdResId(STR_PAGE_NAME));
                }
                else if ( maContext == maImpressOtherContext || maContext == maImpressMasterContext )
                {
                    if( maContext == maImpressMasterContext )
                        SetPanelTitle(SdResId(STR_MASTERSLIDE_NAME));
                    else
                        SetPanelTitle(SdResId(STR_SLIDE_NAME));
                }
                else if ( maContext == maImpressNotesContext )
                {
                    mxMasterLabel->set_label(SdResId(STR_MASTERSLIDE_LABEL));
                    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();

                    if (pMainViewShell)
                    {
                        DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
                        if ( pDrawViewShell->GetEditMode() == EditMode::MasterPage)
                            SetPanelTitle(SdResId(STR_MASTERSLIDE_NAME));
                        else // EditMode::Page
                            SetPanelTitle(SdResId(STR_SLIDE_NAME));
                    }
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
    mxMasterSlide->clear();
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
            mxMasterSlide->append_text(aLayoutName);
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
        mxMasterSlide->set_active_text(pMasterPage->GetName());
    }
}

void SlideBackground::dispose()
{
    removeListener();

    mxCustomEntry.reset();
    mxMarginLabel.reset();
    mxPaperSizeBox.reset();
    mxPaperOrientation.reset();
    mxMasterSlide.reset();
    mxBackgroundLabel.reset();
    mxFillAttr.reset();
    mxFillGrad1.reset();
    mxFillGrad2.reset();
    mxFillStyle.reset();
    mxFillLB.reset();
    mxInsertImage.reset();
    mxMarginSelectBox.reset();
    mxDspMasterBackground.reset();
    mxDspMasterObjects.reset();
    mxMasterLabel.reset();
    mxEditMaster.reset();
    mxCloseMaster.reset();

    maPaperSizeController.dispose();
    maPaperOrientationController.dispose();
    maPaperMarginLRController.dispose();
    maPaperMarginULController.dispose();
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
    mpPageLRMarginItem.reset();
    mpPageULMarginItem.reset();
    PanelLayout::dispose();
}

void SlideBackground::ExecuteMarginLRChange(const ::tools::Long mnPageLeftMargin, const ::tools::Long mnPageRightMargin)
{
    mpPageLRMarginItem->SetLeft(mnPageLeftMargin);
    mpPageLRMarginItem->SetRight(mnPageRightMargin);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE, SfxCallMode::RECORD, { mpPageLRMarginItem.get() } );
}

void SlideBackground::ExecuteMarginULChange(const ::tools::Long mnPageTopMargin, const ::tools::Long mnPageBottomMargin)
{
    mpPageULMarginItem->SetUpper(mnPageTopMargin);
    mpPageULMarginItem->SetLower(mnPageBottomMargin);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_ULSPACE, SfxCallMode::RECORD, { mpPageULMarginItem.get() } );
}

Color const & SlideBackground::GetColorSetOrDefault()
{
   // Tango Sky Blue 1, to be consistent w/ area fill panel (b/c COL_AUTO for slides is transparent)
   if ( !mpColorItem )
        mpColorItem.reset( new XFillColorItem( OUString(), Color(0x72, 0x9f, 0xcf) ) );

   return mpColorItem->GetColorValue();
}

XGradient const & SlideBackground::GetGradientSetOrDefault()
{
    if( !mpGradientItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxGradientListItem * pGradListItem = pSh->GetItem(SID_GRADIENT_LIST);
        const XGradient aGradient = pGradListItem->GetGradientList()->GetGradient(0)->GetGradient();
        const OUString aGradientName = pGradListItem->GetGradientList()->GetGradient(0)->GetName();

        mpGradientItem.reset( new XFillGradientItem( aGradientName, aGradient ) );
    }

    return mpGradientItem->GetGradientValue();
}

OUString const & SlideBackground::GetHatchingSetOrDefault()
{
    if( !mpHatchItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxHatchListItem * pHatchListItem = pSh->GetItem(SID_HATCH_LIST);
        const XHatch aHatch = pHatchListItem->GetHatchList()->GetHatch(0)->GetHatch();
        const OUString aHatchName = pHatchListItem->GetHatchList()->GetHatch(0)->GetName();

        mpHatchItem.reset( new XFillHatchItem( aHatchName, aHatch ) );
    }

    return mpHatchItem->GetName();
}

OUString const & SlideBackground::GetBitmapSetOrDefault()
{
    if( !mpBitmapItem || mpBitmapItem->isPattern())
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxBitmapListItem * pBmpListItem = pSh->GetItem(SID_BITMAP_LIST);
        const GraphicObject aGraphObj = pBmpListItem->GetBitmapList()->GetBitmap(0)->GetGraphicObject();
        const OUString aBmpName = pBmpListItem->GetBitmapList()->GetBitmap(0)->GetName();

        mpBitmapItem.reset( new XFillBitmapItem( aBmpName, aGraphObj ) );
    }

    return mpBitmapItem->GetName();
}

OUString const & SlideBackground::GetPatternSetOrDefault()
{
    if( !mpBitmapItem || !(mpBitmapItem->isPattern()))
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxPatternListItem * pPtrnListItem = pSh->GetItem(SID_PATTERN_LIST);
        const GraphicObject aGraphObj = pPtrnListItem->GetPatternList()->GetBitmap(0)->GetGraphicObject();
        const OUString aPtrnName = pPtrnListItem->GetPatternList()->GetBitmap(0)->GetName();

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
    const SfxPoolItem* pState)
{
    switch(nSID)
    {

        case SID_ATTR_PAGE_COLOR:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxFillStyle->set_active(static_cast< sal_Int32 >(SOLID));
                mpColorItem.reset(pState ? static_cast< XFillColorItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_HATCH:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxFillStyle->set_active(static_cast< sal_Int32 >(HATCH));
                mpHatchItem.reset(pState ? static_cast < XFillHatchItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_GRADIENT:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxFillStyle->set_active(static_cast< sal_Int32>(GRADIENT));
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
                        mxFillStyle->set_active(static_cast< sal_Int32 >(PATTERN));
                    else
                        mxFillStyle->set_active(static_cast< sal_Int32 >(BITMAP));
                }
                else
                    mxFillStyle->set_active(static_cast< sal_Int32 >(BITMAP));
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
                        mxFillStyle->set_active(static_cast< sal_Int32 >(NONE));
                        break;
                    case drawing::FillStyle_SOLID:
                        mxFillStyle->set_active(static_cast< sal_Int32 >(SOLID));
                        break;
                    case drawing::FillStyle_GRADIENT:
                        mxFillStyle->set_active(static_cast< sal_Int32 >(GRADIENT));
                        break;
                    case drawing::FillStyle_HATCH:
                        mxFillStyle->set_active(static_cast< sal_Int32 >(HATCH));
                        break;
                    case drawing::FillStyle_BITMAP:
                    {
                        if(mpBitmapItem->isPattern())
                            mxFillStyle->set_active(static_cast< sal_Int32 >(PATTERN));
                        else
                            mxFillStyle->set_active(static_cast< sal_Int32 >(BITMAP));
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
                pSizeItem = dynamic_cast<const SvxSizeItem*>(pState);
            if (pSizeItem)
            {
                Size aPaperSize = pSizeItem->GetSize();
                if (mxPaperOrientation->get_active() == 0)
                   Swap(aPaperSize);

                Paper ePaper = SvxPaperInfo::GetSvxPaper(aPaperSize, meUnit);
                mxPaperSizeBox->set_active_id( ePaper );
            }
        }
        break;

        case SID_ATTR_PAGE:
        {
            const SvxPageItem* pPageItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pPageItem = dynamic_cast<const SvxPageItem*>(pState);
            if (pPageItem)
            {
                mpPageItem.reset(pPageItem->Clone());
                bool bIsLandscape = mpPageItem->IsLandscape();
                mxPaperOrientation->set_active( bIsLandscape ? 0 : 1 );
            }
        }
        break;

        case SID_ATTR_PAGE_LRSPACE:
        {
            const SvxLongLRSpaceItem* pLRItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pLRItem = dynamic_cast<const SvxLongLRSpaceItem*>(pState);
            if (pLRItem)
            {
                mpPageLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                UpdateMarginBox();
            }
        }
        break;

        case SID_ATTR_PAGE_ULSPACE:
        {
            const SvxLongULSpaceItem* pULItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pULItem = dynamic_cast<const SvxLongULSpaceItem*>(pState);
            if (pULItem)
            {
                mpPageULMarginItem.reset( static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                UpdateMarginBox();
            }
        }
        break;

        case SID_DISPLAY_MASTER_BACKGROUND:
        {
            const SfxBoolItem* pBoolItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pBoolItem = dynamic_cast< const SfxBoolItem* >(pState);
            if (pBoolItem)
                mxDspMasterBackground->set_active(pBoolItem->GetValue());
        }
        break;
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            const SfxBoolItem* pBoolItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pBoolItem = dynamic_cast< const SfxBoolItem* >(pState);
            if (pBoolItem)
                mxDspMasterObjects->set_active(pBoolItem->GetValue());
        }
        break;
        case SID_SELECT_BACKGROUND:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxFillStyle->set_active(static_cast< sal_Int32 >(BITMAP));
                Update();
            }
        }
        break;
        case SID_ATTR_METRIC:
        {
            FieldUnit eFUnit = GetCurrentUnit(eState, pState);
            if (meFUnit != eFUnit)
            {
                meFUnit = eFUnit;
                SetMarginsFieldUnit();
                UpdateMarginBox();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(SlideBackground, FillStyleModifyHdl, weld::ComboBox&, void)
{
    const eFillStyle nPos = static_cast<eFillStyle>(mxFillStyle->get_active());
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
//TODO    mxFillStyle->Selected();
}

IMPL_LINK_NOARG(SlideBackground, PaperSizeModifyHdl, weld::ComboBox&, void)
{
    const Paper ePaper = mxPaperSizeBox->get_active_id();
    Size aSize(SvxPaperInfo::GetPaperSize(ePaper, meUnit));

    if (mxPaperOrientation->get_active() == 0)
        Swap(aSize);

    mpPageItem->SetLandscape(mxPaperOrientation->get_active() == 0);
    const SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    // Page/slide properties dialog (FuPage::ExecuteDialog and ::ApplyItemSet) misuses
    // SID_ATTR_PAGE_EXT1 to distinguish between Impress and Draw, as for whether to fit
    // objects to paper size. Until that is handled somehow better, we do the same here
    const SfxBoolItem aFitObjs(SID_ATTR_PAGE_EXT1, IsImpress());

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD,
                                                { &aSizeItem, mpPageItem.get(), &aFitObjs });

    // Notify LOK clients of the page size change.
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId() == mrBase.GetDocId())
        {
            SdXImpressDocument* pDoc = comphelper::getUnoTunnelImplementation<SdXImpressDocument>(pViewShell->GetCurrentDocument());
            SfxLokHelper::notifyDocumentSizeChangedAllViews(pDoc);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

IMPL_LINK_NOARG(SlideBackground, FillColorHdl, ColorListBox&, void)
{
    const drawing::FillStyle eXFS = static_cast<drawing::FillStyle>(mxFillStyle->get_active());
    switch(eXFS)
    {
        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aItem(OUString(), mxFillLB->GetSelectEntryColor());
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case drawing::FillStyle_GRADIENT:
        {
            XGradient aGradient;
            aGradient.SetStartColor(mxFillGrad1->GetSelectEntryColor());
            aGradient.SetEndColor(mxFillGrad2->GetSelectEntryColor());

            // the name doesn't really matter, it'll be converted to unique one eventually,
            // but it has to be non-empty
            XFillGradientItem aItem("gradient", aGradient);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(SlideBackground, FillBackgroundHdl, weld::ComboBox&, void)
{
    const eFillStyle nFillPos = static_cast<eFillStyle>(mxFillStyle->get_active());
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(nFillPos)
    {

        case HATCH:
        {
            const SvxHatchListItem * pHatchListItem = pSh->GetItem(SID_HATCH_LIST);
            sal_uInt16 nPos = mxFillAttr->get_active();
            XHatch aHatch = pHatchListItem->GetHatchList()->GetHatch(nPos)->GetHatch();
            const OUString aHatchName = pHatchListItem->GetHatchList()->GetHatch(nPos)->GetName();

            XFillHatchItem aItem(aHatchName, aHatch);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            sal_Int16 nPos = mxFillAttr->get_active();
            GraphicObject aBitmap;
            OUString aName;
            if( nFillPos == BITMAP )
            {
                SvxBitmapListItem const * pBitmapListItem = pSh->GetItem(SID_BITMAP_LIST);
                aBitmap = pBitmapListItem->GetBitmapList()->GetBitmap(nPos)->GetGraphicObject();
                aName = pBitmapListItem->GetBitmapList()->GetBitmap(nPos)->GetName();
            }
            else if( nFillPos == PATTERN )
            {
                SvxPatternListItem const * pPatternListItem = pSh->GetItem(SID_PATTERN_LIST);
                aBitmap = pPatternListItem->GetPatternList()->GetBitmap(nPos)->GetGraphicObject();
                aName = pPatternListItem->GetPatternList()->GetBitmap(nPos)->GetName();
            }
            XFillBitmapItem aItem(aName, aBitmap);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
        }
        break;

        default:
            break;
    }
}

IMPL_LINK_NOARG(SlideBackground, AssignMasterPage, weld::ComboBox&, void)
{
    ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell*>( SfxObjectShell::Current() );
    SdDrawDocument* pDoc = pDocSh ? pDocSh->GetDoc() : nullptr;
    if (!pDoc)
        return;

    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(mrBase);
    if (pSSVS == nullptr)
        return;

    auto& rSSController = pSSVS->GetSlideSorter().GetController();
    auto& rPageSelector = rSSController.GetPageSelector();

    for( sal_uInt16 nPage = 0; nPage < pDoc->GetSdPageCount(PageKind::Standard); nPage++ )
    {
        if (rPageSelector.IsPageSelected(nPage))
        {
            OUString aLayoutName(mxMasterSlide->get_active_text());
            pDoc->SetMasterPage(nPage, aLayoutName, pDoc, false, false);
        }
    }
}

IMPL_LINK_NOARG(SlideBackground, EditMasterHdl, weld::Button&, void)
{
    GetBindings()->GetDispatcher()->Execute( SID_SLIDE_MASTER_MODE, SfxCallMode::RECORD );
}

IMPL_LINK_NOARG(SlideBackground, SelectBgHdl, weld::Button&, void)
{
    GetBindings()->GetDispatcher()->Execute( SID_SELECT_BACKGROUND, SfxCallMode::RECORD );
}

IMPL_LINK_NOARG(SlideBackground, CloseMasterHdl, weld::Button&, void)
{
    GetBindings()->GetDispatcher()->Execute( SID_CLOSE_MASTER_VIEW, SfxCallMode::RECORD );
}

IMPL_LINK_NOARG(SlideBackground, DspBackground, weld::Button&, void)
{
    bool IsChecked = mxDspMasterBackground->get_active();
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_BACKGROUND, IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList(SID_DISPLAY_MASTER_BACKGROUND, SfxCallMode::RECORD, { &aBoolItem });
}

IMPL_LINK_NOARG(SlideBackground, DspObjects, weld::Button&, void)
{
    bool IsChecked = mxDspMasterObjects->get_active();
    const SfxBoolItem aBoolItem(SID_DISPLAY_MASTER_OBJECTS,IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList(SID_DISPLAY_MASTER_OBJECTS, SfxCallMode::RECORD, { &aBoolItem, &aBoolItem });
}

IMPL_LINK_NOARG( SlideBackground, ModifyMarginHdl, weld::ComboBox&, void )
{
    bool bApplyNewPageMargins = true;
    switch ( mxMarginSelectBox->get_active() )
    {
        case 0:
            SetNone(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        case 1:
            SetNarrow(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        case 2:
            SetModerate(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        case 3:
            SetNormal075(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        case 4:
            SetNormal100(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        case 5:
            SetNormal125(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        case 6:
            SetWide(m_nPageLeftMargin, m_nPageRightMargin, m_nPageTopMargin, m_nPageBottomMargin);
            break;
        default:
            bApplyNewPageMargins = false;
            break;
    }

    if(bApplyNewPageMargins)
    {
        ExecuteMarginLRChange(m_nPageLeftMargin, m_nPageRightMargin);
        ExecuteMarginULChange(m_nPageTopMargin, m_nPageBottomMargin);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
