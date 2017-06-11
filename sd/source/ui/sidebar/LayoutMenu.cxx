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

#include "LayoutMenu.hxx"

#include "app.hrc"
#include "drawdoc.hxx"
#include "framework/FrameworkHelper.hxx"
#include "strings.hrc"
#include "glob.hxx"
#include "helpids.h"
#include "pres.hxx"

#include "sdpage.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "bitmaps.hlst"
#include "tools/SlotStateListener.hxx"
#include "DrawController.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "EventMultiplexer.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShellBase.hxx"
#include <sfx2/sidebar/Theme.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/image.hxx>
#include <vcl/floatwin.hxx>
#include <xmloff/autolayout.hxx>

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using namespace ::sd::slidesorter;
using ::sd::framework::FrameworkHelper;

namespace sd { namespace sidebar {

struct snewfoil_value_info
{
    const char* msBmpResId;
    const char* mpStrResId;
    WritingMode meWritingMode;
    AutoLayout maAutoLayout;
};

static const snewfoil_value_info notes[] =
{
    {BMP_FOILN_01, STR_AUTOLAYOUT_NOTES, WritingMode_LR_TB,
     AUTOLAYOUT_NOTES},
    {"", nullptr, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static const snewfoil_value_info handout[] =
{
    {BMP_FOILH_01, STR_AUTOLAYOUT_HANDOUT1, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT1},
    {BMP_FOILH_02, STR_AUTOLAYOUT_HANDOUT2, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT2},
    {BMP_FOILH_03, STR_AUTOLAYOUT_HANDOUT3, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT3},
    {BMP_FOILH_04, STR_AUTOLAYOUT_HANDOUT4, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT4},
    {BMP_FOILH_06, STR_AUTOLAYOUT_HANDOUT6, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT6},
    {BMP_FOILH_09, STR_AUTOLAYOUT_HANDOUT9, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT9},
    {"", nullptr, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static const snewfoil_value_info standard[] =
{
    {BMP_LAYOUT_EMPTY, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,        AUTOLAYOUT_NONE},
    {BMP_LAYOUT_HEAD03, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,       AUTOLAYOUT_TITLE},
    {BMP_LAYOUT_HEAD02, STR_AUTOLAYOUT_CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_TITLE_CONTENT},
    {BMP_LAYOUT_HEAD02A, STR_AUTOLAYOUT_2CONTENT, WritingMode_LR_TB,       AUTOLAYOUT_TITLE_2CONTENT},
    {BMP_LAYOUT_HEAD01, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,  AUTOLAYOUT_TITLE_ONLY},
    {BMP_LAYOUT_TEXTONLY, STR_AUTOLAYOUT_ONLY_TEXT, WritingMode_LR_TB,   AUTOLAYOUT_ONLY_TEXT},
    {BMP_LAYOUT_HEAD03B, STR_AUTOLAYOUT_2CONTENT_CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TITLE_2CONTENT_CONTENT},
    {BMP_LAYOUT_HEAD03C, STR_AUTOLAYOUT_CONTENT_2CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TITLE_CONTENT_2CONTENT},
    {BMP_LAYOUT_HEAD03A, STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,WritingMode_LR_TB, AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT},
    {BMP_LAYOUT_HEAD02B, STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, WritingMode_LR_TB, AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT},
    {BMP_LAYOUT_HEAD04, STR_AUTOLAYOUT_4CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_TITLE_4CONTENT},
    {BMP_LAYOUT_HEAD06, STR_AUTOLAYOUT_6CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TITLE_6CONTENT},

    // vertical
    {BMP_LAYOUT_VERTICAL02, STR_AL_VERT_TITLE_TEXT_CHART, WritingMode_TB_RL, AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT},
    {BMP_LAYOUT_VERTICAL01, STR_AL_VERT_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_VTITLE_VCONTENT},
    {BMP_LAYOUT_HEAD02, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_TITLE_VCONTENT},
    {BMP_LAYOUT_HEAD02A, STR_AL_TITLE_VERT_OUTLINE_CLIPART,   WritingMode_TB_RL, AUTOLAYOUT_TITLE_2VTEXT},
    {"", nullptr, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

LayoutMenu::LayoutMenu (
    vcl::Window* pParent,
    ViewShellBase& rViewShellBase,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : ValueSet (pParent, WB_ITEMBORDER),
      DragSourceHelper(this),
      DropTargetHelper(this),
      mrBase(rViewShellBase),
      mbUseOwnScrollBar(false),
      mxListener(nullptr),
      mbSelectionUpdatePending(true),
      mbIsMainViewChangePending(false),
      mxSidebar(rxSidebar),
      mbIsDisposed(false)
{
    implConstruct( *mrBase.GetDocument()->GetDocSh() );
    SAL_INFO("sd.ui", "created LayoutMenu at " << this);

    SetStyle(GetStyle() | WB_ITEMBORDER | WB_FLATVALUESET | WB_TABSTOP);

    SetBackground(sfx2::sidebar::Theme::GetWallpaper(sfx2::sidebar::Theme::Paint_PanelBackground));
    SetColor(sfx2::sidebar::Theme::GetColor(sfx2::sidebar::Theme::Paint_PanelBackground));

#ifdef DEBUG
    SetText(OUString("sd:LayoutMenu"));
#endif
}

void LayoutMenu::implConstruct( DrawDocShell& rDocumentShell )
{
    OSL_ENSURE( mrBase.GetDocument()->GetDocSh() == &rDocumentShell,
        "LayoutMenu::implConstruct: hmm?" );
    // if this fires, then my assumption that the rDocumentShell parameter to our first ctor is superfluous ...
    (void) rDocumentShell;

    SetStyle (
        ( GetStyle()  & ~(WB_ITEMBORDER) )
        | WB_TABSTOP
        | WB_MENUSTYLEVALUESET
        | WB_NO_DIRECTSELECT
        );
    if (mbUseOwnScrollBar)
        SetStyle (GetStyle() | WB_VSCROLL);
    SetExtraSpacing(2);
    SetSelectHdl (LINK(this, LayoutMenu, ClickHandler));
    InvalidateContent();

    Link<::sd::tools::EventMultiplexerEvent&,void> aEventListenerLink (LINK(this,LayoutMenu,EventMultiplexerListener));
    mrBase.GetEventMultiplexer()->AddEventListener(aEventListenerLink);

    Window::SetHelpId(HID_SD_TASK_PANE_PREVIEW_LAYOUTS);
    SetAccessibleName(SdResId(STR_TASKPANEL_LAYOUT_MENU_TITLE));

    Link<const OUString&,void> aStateChangeLink (LINK(this,LayoutMenu,StateChangeHandler));
    mxListener = new ::sd::tools::SlotStateListener(
        aStateChangeLink,
        Reference<frame::XDispatchProvider>(mrBase.GetController()->getFrame(), UNO_QUERY),
        ".uno:VerticalTextState");

    SetSizePixel(GetParent()->GetSizePixel());
    Link<VclWindowEvent&,void> aWindowEventHandlerLink (LINK(this,LayoutMenu,WindowEventHandler));
    GetParent()->AddEventListener(aWindowEventHandlerLink);
}

LayoutMenu::~LayoutMenu()
{
    disposeOnce();
}

void LayoutMenu::dispose()
{
    SAL_INFO("sd.ui", "destroying LayoutMenu at " << this);
    Dispose();
    DragSourceHelper::dispose();
    DropTargetHelper::dispose();
    ValueSet::dispose();
}

void LayoutMenu::Dispose()
{
    if (mbIsDisposed)
        return;

    SAL_INFO("sd.ui", "disposing LayoutMenu at " << this);

    mbIsDisposed = true;

    Reference<lang::XComponent> xComponent (mxListener, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    Clear();
    Link<tools::EventMultiplexerEvent&,void> aLink (LINK(this,LayoutMenu,EventMultiplexerListener));
    mrBase.GetEventMultiplexer()->RemoveEventListener (aLink);

    Link<VclWindowEvent&,void> aWindowEventHandlerLink (LINK(this,LayoutMenu,WindowEventHandler));
    GetParent()->RemoveEventListener(aWindowEventHandlerLink);
}

AutoLayout LayoutMenu::GetSelectedAutoLayout()
{
    AutoLayout aResult = AUTOLAYOUT_NONE;

    if ( ! IsNoSelection() && GetSelectItemId()!=0)
    {
        AutoLayout* pLayout = static_cast<AutoLayout*>(GetItemData(GetSelectItemId()));
        if (pLayout != nullptr)
            aResult = *pLayout;
    }

    return aResult;
}

ui::LayoutSize LayoutMenu::GetHeightForWidth (const sal_Int32 nWidth)
{
    sal_Int32 nPreferredHeight = 200;
    if ( ! mbUseOwnScrollBar && GetItemCount()>0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        if (nWidth>0 && aItemSize.Width()>0)
        {
            aItemSize.Width() += 8;
            aItemSize.Height() += 8;
            int nColumnCount = nWidth / aItemSize.Width();
            if (nColumnCount <= 0)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;
            int nRowCount = (GetItemCount() + nColumnCount-1) / nColumnCount;
            nPreferredHeight = nRowCount * aItemSize.Height();
        }
    }
    return ui::LayoutSize(nPreferredHeight,nPreferredHeight,nPreferredHeight);
}

void LayoutMenu::Paint (vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect)
{
    if (mbSelectionUpdatePending)
    {
        mbSelectionUpdatePending = false;
        UpdateSelection();
    }
    ValueSet::Paint(rRenderContext, rRect);
}

void LayoutMenu::Resize()
{
    Size aWindowSize = GetOutputSizePixel();
    if (IsVisible() && aWindowSize.Width() > 0)
    {
        // Calculate the number of rows and columns.
        if (GetItemCount() > 0)
        {
            Image aImage = GetItemImage(GetItemId(0));
            Size aItemSize = CalcItemSizePixel (
                aImage.GetSizePixel());
            aItemSize.Width() += 8;
            aItemSize.Height() += 8;
            int nColumnCount = aWindowSize.Width() / aItemSize.Width();
            if (nColumnCount < 1)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;

            int nRowCount = CalculateRowCount (aItemSize, nColumnCount);

            SetColCount ((sal_uInt16)nColumnCount);
            SetLineCount ((sal_uInt16)nRowCount);
        }
    }

    ValueSet::Resize ();
}

void LayoutMenu::MouseButtonDown (const MouseEvent& rEvent)
{
    // As a preparation for the context menu the item under the mouse is
    // selected.
    if (rEvent.IsRight())
    {
        ReleaseMouse();
        sal_uInt16 nIndex = GetItemId (rEvent.GetPosPixel());
        if (nIndex > 0)
            SelectItem(nIndex);
    }

    ValueSet::MouseButtonDown (rEvent);
}

void LayoutMenu::InsertPageWithLayout (AutoLayout aLayout)
{
    ViewShell* pViewShell = mrBase.GetMainViewShell().get();
    if (pViewShell == nullptr)
        return;

    SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
    if (pViewFrame == nullptr)
        return;

    SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
    if (pDispatcher == nullptr)
        return;

    // Call SID_INSERTPAGE with the right arguments.  This is because
    // the popup menu can not call this slot with arguments directly.
    SfxRequest aRequest (CreateRequest(SID_INSERTPAGE, aLayout));
    if (aRequest.GetArgs() != nullptr)
    {
        pDispatcher->Execute(
            SID_INSERTPAGE,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            *aRequest.GetArgs());
    }
    UpdateSelection();
}

void LayoutMenu::InvalidateContent()
{
    // Throw away the current set and fill the menu anew according to the
    // current settings (this includes the support for vertical writing.)
    Fill();

    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

int LayoutMenu::CalculateRowCount (const Size&, int nColumnCount)
{
    int nRowCount = 0;

    if (GetItemCount() > 0 && nColumnCount > 0)
    {
        nRowCount = (GetItemCount() + nColumnCount - 1) / nColumnCount;
        //        nRowCount = GetOutputSizePixel().Height() / rItemSize.Height();
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return nRowCount;
}

IMPL_LINK_NOARG(LayoutMenu, ClickHandler, ValueSet*, void)
{
    AssignLayoutToSelectedSlides( GetSelectedAutoLayout() );
}

/** The specified layout is assigned to the current page of the view shell
    in the center pane.
*/
void LayoutMenu::AssignLayoutToSelectedSlides (AutoLayout aLayout)
{
    using namespace ::sd::slidesorter;
    using namespace ::sd::slidesorter::controller;

    do
    {
        // The view shell in the center pane has to be present.
        ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
        if (pMainViewShell == nullptr)
            break;

        // Determine if the current view is in an invalid master page mode.
        // The handout view is always in master page mode and therefore not
        // invalid.
        bool bMasterPageMode (false);
        switch (pMainViewShell->GetShellType())
        {
            case ViewShell::ST_NOTES:
            case ViewShell::ST_IMPRESS:
            {
                DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pMainViewShell);
                if (pDrawViewShell != nullptr)
                    if (pDrawViewShell->GetEditMode() == EditMode::MasterPage)
                        bMasterPageMode = true;
                break;
            }
            default:
                break;
        }
        if (bMasterPageMode)
            break;

        // Get a list of all selected slides and call the SID_MODIFYPAGE
        // slot for all of them.
        ::sd::slidesorter::SharedPageSelection pPageSelection;

        // Get a list of selected pages.
        // First we try to obtain this list from a slide sorter.  This is
        // possible only some of the view shells in the center pane.  When
        // no valid slide sorter is available then ask the main view shell
        // for its current page.
        SlideSorterViewShell* pSlideSorter = nullptr;
        switch (pMainViewShell->GetShellType())
        {
            case ViewShell::ST_IMPRESS:
            case ViewShell::ST_NOTES:
            case ViewShell::ST_SLIDE_SORTER:
                pSlideSorter = SlideSorterViewShell::GetSlideSorter(mrBase);
                break;
            default:
                break;
        }
        if (pSlideSorter != nullptr)
        {
            // There is a slide sorter visible so get the list of selected pages from it.
            pPageSelection = pSlideSorter->GetPageSelection();
        }

        if( (pSlideSorter == nullptr) || (pPageSelection.get() == nullptr) || pPageSelection->empty() )
        {
            // No valid slide sorter available.  Ask the main view shell for
            // its current page.
            pPageSelection.reset(new ::sd::slidesorter::SlideSorterViewShell::PageSelection);
            pPageSelection->push_back(pMainViewShell->GetActualPage());
        }

        if (pPageSelection->empty())
            break;

        ::std::vector<SdPage*>::iterator iPage;
        for (iPage=pPageSelection->begin(); iPage!=pPageSelection->end(); ++iPage)
            {
                if ((*iPage) == nullptr)
                    continue;

                // Call the SID_ASSIGN_LAYOUT slot with all the necessary parameters.
                SfxRequest aRequest (mrBase.GetViewFrame(), SID_ASSIGN_LAYOUT);
                aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATPAGE, ((*iPage)->GetPageNum()-1)/2));
                aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATLAYOUT, aLayout));
                pMainViewShell->ExecuteSlot (aRequest, false);
            }
    }
    while(false);
}

SfxRequest LayoutMenu::CreateRequest (
    sal_uInt16 nSlotId,
    AutoLayout aLayout)
{
    SfxRequest aRequest (mrBase.GetViewFrame(), nSlotId);

    do
    {
        SdrLayerAdmin& rLayerAdmin (mrBase.GetDocument()->GetLayerAdmin());
        SdrLayerID aBackground (rLayerAdmin.GetLayerID(
            SdResId(STR_LAYER_BCKGRND)));
        SdrLayerID aBackgroundObject (rLayerAdmin.GetLayerID(
            SdResId(STR_LAYER_BCKGRNDOBJ)));
        ViewShell* pViewShell = mrBase.GetMainViewShell().get();
        if (pViewShell == nullptr)
            break;
        SdPage* pPage = pViewShell->GetActualPage();
        if (pPage == nullptr)
            break;

        SdrLayerIDSet aVisibleLayers (pPage->TRG_GetMasterPageVisibleLayers());

        aRequest.AppendItem(
            SfxStringItem (ID_VAL_PAGENAME, OUString()));//pPage->GetName()));
        aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATLAYOUT, aLayout));
        aRequest.AppendItem(
            SfxBoolItem(ID_VAL_ISPAGEBACK, aVisibleLayers.IsSet(aBackground)));
        aRequest.AppendItem(
            SfxBoolItem(
                ID_VAL_ISPAGEOBJ,
                aVisibleLayers.IsSet(aBackgroundObject)));
    }
    while (false);

    return aRequest;
}

void LayoutMenu::Fill()
{
    SvtLanguageOptions aLanguageOptions;
    bool bVertical = aLanguageOptions.IsVerticalTextEnabled();
    SdDrawDocument* pDocument = mrBase.GetDocument();
    bool bRightToLeft = (pDocument!=nullptr
        && pDocument->GetDefaultWritingMode() == WritingMode_RL_TB);

    // Get URL of the view in the center pane.
    OUString sCenterPaneViewName;
    try
    {
        Reference<XControllerManager> xControllerManager (
            Reference<XWeak>(&mrBase.GetDrawController()), UNO_QUERY_THROW);
        Reference<XResourceId> xPaneId (ResourceId::create(
            ::comphelper::getProcessComponentContext(),
            FrameworkHelper::msCenterPaneURL));
        Reference<XView> xView (FrameworkHelper::Instance(mrBase)->GetView(xPaneId));
        if (xView.is())
            sCenterPaneViewName = xView->getResourceId()->getResourceURL();
    }
    catch (RuntimeException&)
    {}

    const snewfoil_value_info* pInfo = nullptr;
    if (sCenterPaneViewName.equals(framework::FrameworkHelper::msNotesViewURL))
    {
        pInfo = notes;
    }
    else if (sCenterPaneViewName.equals(framework::FrameworkHelper::msHandoutViewURL))
    {
        pInfo = handout;
    }
    else if (sCenterPaneViewName.equals(framework::FrameworkHelper::msImpressViewURL)
        || sCenterPaneViewName.equals(framework::FrameworkHelper::msSlideSorterURL))
    {
        pInfo = standard;
    }
    else
    {
        pInfo = nullptr;
    }

    Clear();
    for (sal_uInt16 i=1; pInfo!=nullptr && pInfo->mpStrResId != nullptr; i++, pInfo++)
    {
        if ((WritingMode_TB_RL != pInfo->meWritingMode) || bVertical)
        {
            BitmapEx aBmp(OUString::createFromAscii(pInfo->msBmpResId));

            if (bRightToLeft && (WritingMode_TB_RL != pInfo->meWritingMode))
                aBmp.Mirror (BmpMirrorFlags::Horizontal);

            InsertItem(i, Image(aBmp), SdResId(pInfo->mpStrResId));
            SetItemData (i, new AutoLayout(pInfo->maAutoLayout));
        }
    }

    mbSelectionUpdatePending = true;
}

void LayoutMenu::Clear()
{
    for (size_t nId=1; nId<=GetItemCount(); nId++)
        delete static_cast<AutoLayout*>(GetItemData(nId));
    ValueSet::Clear();
}

void LayoutMenu::StartDrag (sal_Int8 , const Point& )
{
}

sal_Int8 LayoutMenu::AcceptDrop (const AcceptDropEvent& )
{
    return 0;
}

sal_Int8 LayoutMenu::ExecuteDrop (const ExecuteDropEvent& )
{
    return 0;
}

void LayoutMenu::Command (const CommandEvent& rEvent)
{
    switch (rEvent.GetCommand())
    {
        case CommandEventId::ContextMenu:
            if ( ! SD_MOD()->GetWaterCan())
            {
                // Determine the position where to show the menu.
                Point aMenuPosition;
                if (rEvent.IsMouseEvent())
                {
                    if (GetItemId(rEvent.GetMousePosPixel()) <= 0)
                        return;
                    aMenuPosition = rEvent.GetMousePosPixel();
                }
                else
                {
                    if (GetSelectItemId() == (sal_uInt16)-1)
                        return;
                    ::tools::Rectangle aBBox (GetItemRect(GetSelectItemId()));
                    aMenuPosition = aBBox.Center();
                }

                // Setup the menu.
                VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/simpress/ui/layoutmenu.ui", "");
                VclPtr<PopupMenu> pMenu(aBuilder.get_menu("menu"));
                FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
                if (pMenuWindow != nullptr)
                    pMenuWindow->SetPopupModeFlags(
                        pMenuWindow->GetPopupModeFlags() | FloatWinPopupFlags::NoMouseUpClose);
                pMenu->SetSelectHdl(LINK(this, LayoutMenu, OnMenuItemSelected));

                // Disable the SID_INSERTPAGE_LAYOUT_MENU item when
                // the document is read-only.
                const SfxPoolItem* pItem = nullptr;
                const SfxItemState aState (
                    mrBase.GetViewFrame()->GetDispatcher()->QueryState(SID_INSERTPAGE, pItem));
                if (aState == SfxItemState::DISABLED)
                    pMenu->EnableItem(SID_INSERTPAGE_LAYOUT_MENU, false);

                // Show the menu.
                pMenu->Execute(this, ::tools::Rectangle(aMenuPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
            }
            break;

        default:
            ValueSet::Command(rEvent);
            break;
    }
}

IMPL_LINK_NOARG(LayoutMenu, StateChangeHandler, const OUString&, void)
{
    InvalidateContent();
}

IMPL_LINK(LayoutMenu, OnMenuItemSelected, Menu*, pMenu, bool)
{
    if (pMenu == nullptr)
    {
        OSL_ENSURE(pMenu!=nullptr, "LayoutMenu::OnMenuItemSelected: illegal menu!");
        return false;
    }

    pMenu->Deactivate();
    OString sIdent = pMenu->GetCurItemIdent();

    if (sIdent == "apply")
    {
        AssignLayoutToSelectedSlides(GetSelectedAutoLayout());
    }
    else if (sIdent == "insert")
    {
        // Add arguments to this slot and forward it to the main view
        // shell.
        InsertPageWithLayout(GetSelectedAutoLayout());
    }

    return false;
}

void LayoutMenu::UpdateSelection()
{
    bool bItemSelected = false;

    do
    {
        // Get current page of main view.
        ViewShell* pViewShell = mrBase.GetMainViewShell().get();
        if (pViewShell == nullptr)
            break;

        SdPage* pCurrentPage = pViewShell->getCurrentPage();
        if (pCurrentPage == nullptr)
            break;

        // Get layout of current page.
        AutoLayout aLayout (pCurrentPage->GetAutoLayout());
        if (aLayout<AUTOLAYOUT_START || aLayout>AUTOLAYOUT_END)
            break;

        // Find the entry of the menu for to the layout.
        SetNoSelection();
        sal_uInt16 nItemCount (GetItemCount());
        for (sal_uInt16 nId=1; nId<=nItemCount; nId++)
        {
            if (*static_cast<AutoLayout*>(GetItemData(nId)) == aLayout)
            {
                SelectItem(nId);
                bItemSelected = true;
                break;
            }
        }
    }
    while (false);

    if ( ! bItemSelected)
        SetNoSelection();
}

IMPL_LINK(LayoutMenu, EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::CurrentPageChanged:
        case EventMultiplexerEventId::SlideSortedSelection:
            if ( ! mbSelectionUpdatePending)
                UpdateSelection();
            break;

        case EventMultiplexerEventId::MainViewAdded:
            mbIsMainViewChangePending = true;
            break;

        case EventMultiplexerEventId::MainViewRemoved:
            HideFocus();
            break;

        case EventMultiplexerEventId::ConfigurationUpdated:
            if (mbIsMainViewChangePending)
            {
                mbIsMainViewChangePending = false;
                InvalidateContent();
            }
            break;

        default:
            break;
    }
}

IMPL_LINK(LayoutMenu, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
        case VclEventId::WindowShow:
        case VclEventId::WindowResize:
            SetSizePixel(GetParent()->GetSizePixel());
            break;

        default: break;
    }
}

void LayoutMenu::DataChanged (const DataChangedEvent& rEvent)
{
    Fill();
    ValueSet::DataChanged(rEvent);
    SetBackground(sfx2::sidebar::Theme::GetWallpaper(sfx2::sidebar::Theme::Paint_PanelBackground));
    SetColor(sfx2::sidebar::Theme::GetColor(sfx2::sidebar::Theme::Paint_PanelBackground));
}

} } // end of namespace ::sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
