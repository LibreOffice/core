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

#include "TaskPaneShellManager.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "helpids.h"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"
#include "taskpane/ToolPanelViewShell.hxx"
#include "taskpane/ScrollPanel.hxx"
#include "tools/SlotStateListener.hxx"
#include "EventMultiplexer.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"

#include <vector>
#include <memory>
#include <sfx2/objface.hxx>
#include "sdresid.hxx"
#include <vcl/image.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/app.hxx>
#include "taskpane/TitledControl.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>

using namespace ::sd::toolpanel;
#define LayoutMenu
#include "sdslots.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using namespace ::sd::slidesorter;
using ::sd::framework::FrameworkHelper;

namespace sd { namespace toolpanel {

class LayoutMenuRootFactory
    : public ControlFactory
{
public:
    LayoutMenuRootFactory (ToolPanelViewShell& i_rPanelViewShell)
        :mrPanelViewShell(i_rPanelViewShell)
    {
    }

protected:
    virtual TreeNode* InternalCreateControl( ::Window& i_rParent )
    {
        ScrollPanel* pScrollPanel = new ScrollPanel (i_rParent);
        ::std::auto_ptr<TreeNode> pMenu (
            new LayoutMenu (
                pScrollPanel,
                mrPanelViewShell));
        pScrollPanel->AddControl(pMenu);
        return pScrollPanel;
    }

private:
    ToolPanelViewShell& mrPanelViewShell;
};


SFX_IMPL_INTERFACE(LayoutMenu, SfxShell,
    SdResId(STR_TASKPANELAYOUTMENU))
{
    SFX_POPUPMENU_REGISTRATION(SdResId(RID_TASKPANE_LAYOUTMENU_POPUP));
}

TYPEINIT1(LayoutMenu, SfxShell);

struct snewfoil_value_info
{
    sal_uInt16 mnBmpResId;
    sal_uInt16 mnStrResId;
    WritingMode meWritingMode;
    AutoLayout maAutoLayout;
};

static const snewfoil_value_info notes[] =
{
    {BMP_FOILN_01, STR_AUTOLAYOUT_NOTES, WritingMode_LR_TB,
     AUTOLAYOUT_NOTES},
    {0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE},
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
    {0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static const snewfoil_value_info standard[] =
{
    {BMP_LAYOUT_EMPTY, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,        AUTOLAYOUT_NONE},
    {BMP_LAYOUT_HEAD03, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,       AUTOLAYOUT_TITLE},
    {BMP_LAYOUT_HEAD02, STR_AUTOLAYOUT_CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_ENUM},
    {BMP_LAYOUT_HEAD02A, STR_AUTOLAYOUT_2CONTENT, WritingMode_LR_TB,       AUTOLAYOUT_2TEXT},
    {BMP_LAYOUT_HEAD01, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,  AUTOLAYOUT_ONLY_TITLE},
    {BMP_LAYOUT_TEXTONLY, STR_AUTOLAYOUT_ONLY_TEXT, WritingMode_LR_TB,   AUTOLAYOUT_ONLY_TEXT},
    {BMP_LAYOUT_HEAD03B, STR_AUTOLAYOUT_2CONTENT_CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_2OBJTEXT},
    {BMP_LAYOUT_HEAD03C, STR_AUTOLAYOUT_CONTENT_2CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TEXT2OBJ},
    {BMP_LAYOUT_HEAD03A, STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,WritingMode_LR_TB, AUTOLAYOUT_2OBJOVERTEXT},
    {BMP_LAYOUT_HEAD02B, STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, WritingMode_LR_TB, AUTOLAYOUT_OBJOVERTEXT},
    {BMP_LAYOUT_HEAD04, STR_AUTOLAYOUT_4CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_4OBJ},
    {BMP_LAYOUT_HEAD06, STR_AUTOLAYOUT_6CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_6CLIPART},

    // vertical
    {BMP_LAYOUT_VERTICAL02, STR_AL_VERT_TITLE_TEXT_CHART, WritingMode_TB_RL,AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART},
    {BMP_LAYOUT_VERTICAL01, STR_AL_VERT_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02A, STR_AL_TITLE_VERT_OUTLINE_CLIPART,   WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART},
    {0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

LayoutMenu::LayoutMenu( TreeNode* pParent, ToolPanelViewShell& i_rPanelViewShell )
    : ValueSet(pParent->GetWindow(), WB_ITEMBORDER),
      TreeNode(pParent),
      DragSourceHelper(this),
      DropTargetHelper(this),
      mrBase( i_rPanelViewShell.GetViewShellBase() ),
      mpShellManager (&i_rPanelViewShell.GetSubShellManager()),
      mbUseOwnScrollBar( false ),
      mnPreferredColumnCount(3),
      mxListener(NULL),
      mbSelectionUpdatePending(true),
      mbIsMainViewChangePending(false)
{
    implConstruct( *mrBase.GetDocument()->GetDocSh() );
}


void LayoutMenu::implConstruct( DrawDocShell& rDocumentShell )
{
    OSL_ENSURE( mrBase.GetDocument()->GetDocSh() == &rDocumentShell,
        "LayoutMenu::implConstruct: hmm?" );
    // if this fires, then my assumption that the rDocumentShell parameter to our first ctor is superfluous ...

    SetStyle (
        ( GetStyle()  & ~(WB_ITEMBORDER) )
        | WB_TABSTOP
        | WB_NO_DIRECTSELECT
        );
    if (mbUseOwnScrollBar)
        SetStyle (GetStyle() | WB_VSCROLL);
    SetExtraSpacing(2);
    SetSelectHdl (LINK(this, LayoutMenu, ClickHandler));
    SetPool (&rDocumentShell.GetDoc()->GetPool());
    SetName(OUString("LayoutMenu"));
    InvalidateContent();

    Link aEventListenerLink (LINK(this,LayoutMenu,EventMultiplexerListener));
    mrBase.GetEventMultiplexer()->AddEventListener(aEventListenerLink,
        ::sd::tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | ::sd::tools::EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION
        | ::sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | ::sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | ::sd::tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED
        | ::sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_NORMAL
        | ::sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_MASTER);

    Window::SetHelpId(HID_SD_TASK_PANE_PREVIEW_LAYOUTS);
    SetAccessibleName(SdResId(STR_TASKPANEL_LAYOUT_MENU_TITLE));

    Link aStateChangeLink (LINK(this,LayoutMenu,StateChangeHandler));
    mxListener = new ::sd::tools::SlotStateListener(
        aStateChangeLink,
        Reference<frame::XDispatchProvider>(mrBase.GetController()->getFrame(), UNO_QUERY),
        ".uno:VerticalTextState");

    // Add this new object as shell to the shell factory.
    GetShellManager()->AddSubShell(SHELLID_SD_TASK_PANE_PREVIEW_LAYOUTS,this,this);
}



LayoutMenu::~LayoutMenu (void)
{
    // Tell the shell factory that this object is no longer available.
    if (GetShellManager() != NULL)
        GetShellManager()->RemoveSubShell(this);

    Reference<lang::XComponent> xComponent (mxListener, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    Clear();
    Link aLink (LINK(this,LayoutMenu,EventMultiplexerListener));
    mrBase.GetEventMultiplexer()->RemoveEventListener (aLink);
}




::std::auto_ptr<ControlFactory> LayoutMenu::CreateControlFactory (
    ToolPanelViewShell& i_rPanelViewShell )
{
    return ::std::auto_ptr<ControlFactory>(new LayoutMenuRootFactory(i_rPanelViewShell));
}




AutoLayout LayoutMenu::GetSelectedAutoLayout (void)
{
    AutoLayout aResult = AUTOLAYOUT_NONE;

    if ( ! IsNoSelection() && GetSelectItemId()!=0)
    {
        AutoLayout* pLayout = static_cast<AutoLayout*>(GetItemData(GetSelectItemId()));
        if (pLayout != NULL)
            aResult = *pLayout;
    }

    return aResult;
}




/** The preferred size depends on the preferred number of columns, the
    number of items, and the size of the items.
*/
Size LayoutMenu::GetPreferredSize (void)
{
    Size aItemSize = CalcItemSizePixel (Size());
    Size aPreferredWindowSize = CalcWindowSizePixel (
        aItemSize,
         (sal_uInt16)mnPreferredColumnCount,
        (sal_uInt16)CalculateRowCount (aItemSize,mnPreferredColumnCount));
    return aPreferredWindowSize;
}




sal_Int32 LayoutMenu::GetPreferredWidth (sal_Int32 nHeight)
{
    sal_Int32 nPreferredWidth = 100;
    if (GetItemCount() > 0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        if (nHeight>0 && aItemSize.Height()>0)
        {
            int nRowCount = nHeight / aItemSize.Height();
            if (nRowCount <= 0)
                nRowCount = 1;
            int nColumnCount = (GetItemCount() + nRowCount-1) / nRowCount;
            nPreferredWidth = nColumnCount * aItemSize.Width();
        }
    }

    return nPreferredWidth;
}




sal_Int32 LayoutMenu::GetPreferredHeight (sal_Int32 nWidth)
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
    return nPreferredHeight;
}




sal_Int32 LayoutMenu::GetMinimumWidth (void)
{
    sal_Int32 nMinimumWidth = 0;
    if (GetItemCount()>0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        nMinimumWidth = aItemSize.Width();
    }
    return nMinimumWidth;
}




bool LayoutMenu::IsResizable (void)
{
    return true;
}




void LayoutMenu::UpdateEnabledState (const MasterMode eMode)
{
    bool bIsEnabled (false);

    ::boost::shared_ptr<ViewShell> pMainViewShell (mrBase.GetMainViewShell());
    if (pMainViewShell)
    {
        switch (pMainViewShell->GetShellType())
        {
            case ViewShell::ST_NONE:
            case ViewShell::ST_OUTLINE:
            case ViewShell::ST_PRESENTATION:
            case ViewShell::ST_TASK_PANE:
                // The complete task pane is disabled for these values or
                // not even visible.  Disabling the LayoutMenu would be
                // logical but unnecessary.  The main disadvantage is that
                // after re-enabling it (typically) another panel is
                // expanded.
                bIsEnabled = true;
                break;

            case ViewShell::ST_DRAW:
            case ViewShell::ST_IMPRESS:
            {
                switch (eMode)
                {
                    case MM_UNKNOWN:
                    {
                        ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
                            ::boost::dynamic_pointer_cast<DrawViewShell>(pMainViewShell));
                        if (pDrawViewShell)
                            bIsEnabled = pDrawViewShell->GetEditMode() != EM_MASTERPAGE;
                        break;
                    }
                    case MM_NORMAL:
                        bIsEnabled = true;
                        break;

                    case MM_MASTER:
                        bIsEnabled = false;
                        break;
                }
                break;
            }

            case ViewShell::ST_HANDOUT:
            case ViewShell::ST_NOTES:
            case ViewShell::ST_SLIDE_SORTER:
            default:
                bIsEnabled = true;
                break;
        }

        TreeNode* pParentNode = GetParentNode();
        if (pParentNode != NULL)
        {
            TitledControl* pGrandParentNode
                = dynamic_cast<TitledControl*>(pParentNode->GetParentNode());
            if (pGrandParentNode != NULL)
                pGrandParentNode->SetEnabledState(bIsEnabled);
        }

    }
}




::Window* LayoutMenu::GetWindow (void)
{
    return this;
}




void LayoutMenu::Paint (const Rectangle& rRect)
{
    SetBackground (GetSettings().GetStyleSettings().GetWindowColor());

    if (mbSelectionUpdatePending)
    {
        mbSelectionUpdatePending = false;
        UpdateSelection();
    }
    ValueSet::Paint (rRect);

    SetBackground (Wallpaper());
}




void LayoutMenu::Resize (void)
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




void LayoutMenu::Execute (SfxRequest& rRequest)
{
    switch (rRequest.GetSlot())
    {
        case SID_TP_APPLY_TO_SELECTED_SLIDES:
            AssignLayoutToSelectedSlides(GetSelectedAutoLayout());
            rRequest.Done();
            break;

        case SID_INSERTPAGE_LAYOUT_MENU:
            // Add arguments to this slot and forward it to the main view
            // shell.
            InsertPageWithLayout(GetSelectedAutoLayout());
            break;
    }
}




void LayoutMenu::GetState (SfxItemSet& rItemSet)
{
    // Cut and paste is not supported.  The SID_(CUT,COPY,PASTE) entries
    // therefore must not show up in the context menu.
    rItemSet.DisableItem (SID_CUT);
    rItemSet.DisableItem (SID_COPY);
    rItemSet.DisableItem (SID_PASTE);

    // The SID_INSERTPAGE_LAYOUT_MENU slot depends on the SID_INSERTPAGE
    // slot being supported elsewhere.
    const SfxPoolItem* pItem = NULL;
    const SfxItemState aState (
        mrBase.GetViewFrame()->GetDispatcher()->QueryState(SID_INSERTPAGE, pItem));
    if (aState == SFX_ITEM_DISABLED)
        rItemSet.DisableItem(SID_INSERTPAGE_LAYOUT_MENU);
}




void LayoutMenu::InsertPageWithLayout (AutoLayout aLayout)
{
    ViewShell* pViewShell = mrBase.GetMainViewShell().get();
    if (pViewShell == NULL)
        return;

    SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
    if (pViewFrame == NULL)
        return;

    SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
    if (pDispatcher == NULL)
        return;

    // Call SID_INSERTPAGE with the right arguments.  This is because
    // the popup menu can not call this slot with arguments directly.
    SfxRequest aRequest (CreateRequest(SID_INSERTPAGE, aLayout));
    if (aRequest.GetArgs() != NULL)
    {
        pDispatcher->Execute(
            SID_INSERTPAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
            *aRequest.GetArgs());
    }
    UpdateSelection();
}




TaskPaneShellManager* LayoutMenu::GetShellManager()
{
    if ( mpShellManager )
        return mpShellManager;
    return TreeNode::GetShellManager();
}

void LayoutMenu::InvalidateContent (void)
{
    // The number of items may have changed.  Request a resize so that the
    // vertical size of this control can be adapted.
    RequestResize();

    // Throw away the current set and fill the menu anew according to the
    // current settings (this includes the support for vertical writing.)
    Fill();
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




IMPL_LINK_NOARG(LayoutMenu, ClickHandler)
{
    AssignLayoutToSelectedSlides (GetSelectedAutoLayout());
    return 0;
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
        if (pMainViewShell == NULL)
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
                if (pDrawViewShell != NULL)
                    if (pDrawViewShell->GetEditMode() == EM_MASTERPAGE)
                        bMasterPageMode = true;
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
        SlideSorterViewShell* pSlideSorter = NULL;
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
        if (pSlideSorter != NULL)
        {
            // There is a slide sorter visible so get the list of selected pages from it.
            pPageSelection = pSlideSorter->GetPageSelection();
        }

        if( (pSlideSorter == NULL) || (pPageSelection.get() == 0) || pPageSelection->empty() )
        {
            // No valid slide sorter available.  Ask the main view shell for
            // its current page.
            pPageSelection.reset(new ::sd::slidesorter::SlideSorterViewShell::PageSelection());
            pPageSelection->push_back(pMainViewShell->GetActualPage());
        }


        if (pPageSelection->empty())
            break;

        ::std::vector<SdPage*>::iterator iPage;
        for (iPage=pPageSelection->begin(); iPage!=pPageSelection->end(); ++iPage)
            {
                if ((*iPage) == NULL)
                    continue;

                // Call the SID_ASSIGN_LAYOUT slot with all the necessary parameters.
                SfxRequest aRequest (mrBase.GetViewFrame(), SID_ASSIGN_LAYOUT);
                aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATPAGE, ((*iPage)->GetPageNum()-1)/2));
                aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATLAYOUT, aLayout));
                pMainViewShell->ExecuteSlot (aRequest, sal_Bool(sal_False));
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
        sal_uInt8 aBackground (rLayerAdmin.GetLayerID(
            String(SdResId(STR_LAYER_BCKGRND)), sal_False));
        sal_uInt8 aBackgroundObject (rLayerAdmin.GetLayerID(
            String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False));
        ViewShell* pViewShell = mrBase.GetMainViewShell().get();
        if (pViewShell == NULL)
            break;
        SdPage* pPage = pViewShell->GetActualPage();
        if (pPage == NULL)
            break;

        SetOfByte aVisibleLayers (pPage->TRG_GetMasterPageVisibleLayers());

        aRequest.AppendItem(
            SfxStringItem (ID_VAL_PAGENAME, String()));//pPage->GetName()));
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




void LayoutMenu::Fill (void)
{
    SvtLanguageOptions aLanguageOptions;
    sal_Bool bVertical = aLanguageOptions.IsVerticalTextEnabled();
    SdDrawDocument* pDocument = mrBase.GetDocument();
    sal_Bool bRightToLeft = (pDocument!=NULL
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

    const snewfoil_value_info* pInfo = NULL;
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
        pInfo = NULL;
    }

    Clear();
    int n = 0;
    for (sal_uInt16 i=1; pInfo!=NULL&&pInfo->mnBmpResId!=0; i++,pInfo++)
    {
        if ((WritingMode_TB_RL != pInfo->meWritingMode) || bVertical)
        {
            BitmapEx aBmp(SdResId(pInfo->mnBmpResId));

            if (bRightToLeft && (WritingMode_TB_RL != pInfo->meWritingMode))
                aBmp.Mirror (BMP_MIRROR_HORZ);

            InsertItem (i, aBmp, String (SdResId (pInfo->mnStrResId)));
            SetItemData (i, new AutoLayout(pInfo->maAutoLayout));
            n++;
        }
    }

    mbSelectionUpdatePending = true;
}




void LayoutMenu::Clear (void)
{
    for (sal_uInt16 nId=1; nId<=GetItemCount(); nId++)
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
        case COMMAND_CONTEXTMENU:
            if ( ! SD_MOD()->GetWaterCan())
            {
                if (GetShellManager() != NULL)
                    GetShellManager()->MoveToTop(this);
                Point aPosition (0,0);
                if (rEvent.IsMouseEvent())
                {
                    // Do not show the context menu when the mouse was not
                    // pressed over an item.
                    // We have to explicitly specify the location of the menu
                    // when the LayoutMenu is undocked
                    if (GetItemId(rEvent.GetMousePosPixel()) > 0)
                        aPosition = rEvent.GetMousePosPixel();
                }
                else
                {
                    // When the command event was not caused by a mouse
                    // event (for example a key press instead) then show the
                    // popup menu at the center of the current item.
                    if (GetSelectItemId() != (sal_uInt16)-1)
                    {
                        Rectangle aBBox (GetItemRect(GetSelectItemId()));
                        aPosition = aBBox.Center();
                    }
                }
                mrBase.GetViewFrame()->GetDispatcher()->ExecutePopup(
                            SdResId(RID_TASKPANE_LAYOUTMENU_POPUP),
                            this,
                            &aPosition);
            }
            break;

        default:
            ValueSet::Command(rEvent);
            break;
    }
}




IMPL_LINK_NOARG(LayoutMenu, StateChangeHandler)
{
    InvalidateContent();
    return 0;
}




void LayoutMenu::UpdateSelection (void)
{
    bool bItemSelected = false;

    do
    {
        // Get current page of main view.
        ViewShell* pViewShell = mrBase.GetMainViewShell().get();
        if (pViewShell == NULL)
            break;

        SdPage* pCurrentPage = pViewShell->getCurrentPage();
        if (pCurrentPage == NULL)
            break;

        // Get layout of current page.
        AutoLayout aLayout (pCurrentPage->GetAutoLayout());
        if (aLayout<AUTOLAYOUT__START || aLayout>AUTOLAYOUT__END)
            break;

        // Find the entry of the menu for to the layout.
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




IMPL_LINK(LayoutMenu, EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent*, pEvent)
{
    switch (pEvent->meEventId)
    {
        case ::sd::tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        case ::sd::tools::EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION:
            if ( ! mbSelectionUpdatePending)
                UpdateSelection();
            break;

        case ::sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mbIsMainViewChangePending = true;
            UpdateEnabledState(MM_UNKNOWN);
            break;

        case ::sd::tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            HideFocus();
            break;

        case ::sd::tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED:
            if (mbIsMainViewChangePending)
            {
                mbIsMainViewChangePending = false;
                InvalidateContent();
            }
            break;

        case ::sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_NORMAL:
            UpdateEnabledState(MM_NORMAL);
            break;

        case ::sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_MASTER:
            UpdateEnabledState(MM_MASTER);
            break;

        default:
            /* Ignored */
            break;
    }

    return 0;
}




void LayoutMenu::DataChanged (const DataChangedEvent& rEvent)
{
    Fill();
    ValueSet::DataChanged(rEvent);
}


} } // end of namespace ::sd::toolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
