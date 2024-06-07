/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <NotesPanelViewShell.hxx>
#include <NotesPanelView.hxx>
#include <sal/log.hxx>

#include <DrawController.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <FrameView.hxx>
#include <SpellDialogChildWindow.hxx>
#include <ViewShellBase.hxx>
#include <Window.hxx>
#include <app.hrc>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <drawdoc.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editund2.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/unolingu.hxx>
#include <framework/FrameworkHelper.hxx>
#include <fubullet.hxx>
#include <fuchar.hxx>
#include <fucushow.hxx>
#include <fuexpand.hxx>
#include <fuinsfil.hxx>
#include <fuolbull.hxx>
#include <fuoltext.hxx>
#include <fuprobjs.hxx>
#include <fuscale.hxx>
#include <fusldlg.hxx>
#include <fusumry.hxx>
#include <futempl.hxx>
#include <futhes.hxx>
#include <memory>
#include <sdabstdlg.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/devtools/DevelopmentToolChildWindow.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/zoomitem.hxx>
#include <slideshow.hxx>
#include <sot/formats.hxx>
#include <stlsheet.hxx>
#include <strings.hrc>
#include <svl/cjkoptions.hxx>
#include <svl/srchitem.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svtools/cliplistener.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svxids.hrc>
#include <svx/zoomslideritem.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/commandevent.hxx>
#include <zoomlist.hxx>

#include <TextObjectBar.hxx>

#include <memory>

#define ShellClass_NotesPanelViewShell
using namespace sd;
#include <sdslots.hxx>

namespace sd
{
#define MIN_ZOOM 10 // minimum zoom factor
#define MAX_ZOOM 1000 // maximum zoom factor

/**
 * Declare SFX-Slotmap and standard interface
 */
SFX_IMPL_INTERFACE(NotesPanelViewShell, SfxShell)

void NotesPanelViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("drawtext");

    GetStaticInterface()->RegisterChildWindow(SvxHlinkDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(::sd::SpellDialogChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SID_SEARCH_DLG);
    GetStaticInterface()->RegisterChildWindow(
        sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(DevelopmentToolChildWindow::GetChildWindowId());
}

/**
 * Default constructor, windows must not center themselves automatically
 */
NotesPanelViewShell::NotesPanelViewShell(SfxViewFrame* /*pFrame*/, ViewShellBase& rViewShellBase,
                                         vcl::Window* pParentWindow, FrameView* pFrameViewArgument)
    : ViewShell(pParentWindow, rViewShellBase)
{
    if (pFrameViewArgument != nullptr)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());

    mpFrameView->Connect();

    Construct();

    SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::DrawText));

    doShow();
    mpHorizontalScrollBar->Hide();
}

NotesPanelViewShell::~NotesPanelViewShell()
{
    DisposeFunctions();
    mpFrameView->Disconnect();
    // if ( mxClipEvtLstnr.is() )
    // {
    //     mxClipEvtLstnr->RemoveListener( GetActiveWindow() );
    //     mxClipEvtLstnr->ClearCallbackLink();     // prevent callback if another thread is waiting
    // }
}

void NotesPanelViewShell::Construct()
{
    meShellType = ST_NOTESPANEL;

    Size aSize(29700, 21000);
    Point aWinPos(0, 0);
    Point aViewOrigin(0, 0);
    GetActiveWindow()->SetMinZoomAutoCalc(false);
    GetActiveWindow()->SetMinZoom(MIN_ZOOM);
    GetActiveWindow()->SetMaxZoom(MAX_ZOOM);
    InitWindows(aViewOrigin, aSize, aWinPos);

    mpNotesPanelView = std::make_unique<NotesPanelView>(*GetDocSh(), GetActiveWindow(), *this);
    mpView = mpNotesPanelView.get();

    SetPool(&GetDoc()->GetPool());
    SetZoom(70);

    // Apply settings of FrameView
    ReadFrameViewData(mpFrameView);

    SetName("NotesPanelViewShell");
    // TODO: Help ID
    // GetActiveWindow()->SetHelpId(HID_SDNOTESPANEL);
}

void NotesPanelViewShell::Paint(const ::tools::Rectangle& rRect, ::sd::Window* pWin)
{
    if (mpNotesPanelView)
        mpNotesPanelView->Paint(rRect, pWin);
}

bool NotesPanelViewShell::PrepareClose(bool bUI)
{
    if (!ViewShell::PrepareClose(bUI))
        return false;

    return true;
}

void NotesPanelViewShell::VirtHScrollHdl(ScrollAdaptor* /*pHScroll*/)
{
    // no horizontal scroll
    return;
}

void NotesPanelViewShell::UpdateScrollBars()
{
    if (!mpVerticalScrollBar)
        return;

    OutlinerView* pOutlinerView = mpNotesPanelView->GetOutlinerView();
    const SdOutliner& rOutliner = mpNotesPanelView->GetOutliner();

    int nVUpper = rOutliner.GetTextHeight();
    int nVCurrentDocPos = pOutlinerView->GetVisArea().Top();
    const Size aOut(pOutlinerView->GetOutputArea().GetSize());
    int nVStepIncrement = aOut.Height() * 2 / 10;
    int nVPageIncrement = aOut.Height() * 8 / 10;
    int nVPageSize = aOut.Height();

    nVPageSize = std::min(nVPageSize, nVUpper);

    mpVerticalScrollBar->SetRange({ 0, nVUpper });
    mpVerticalScrollBar->SetVisibleSize(nVPageSize);
    mpVerticalScrollBar->SetThumbPos(nVCurrentDocPos);
    mpVerticalScrollBar->SetLineSize(nVStepIncrement);
    mpVerticalScrollBar->SetPageSize(nVPageIncrement);

    // TODO: This is a workaround for the view going blank when overflow the current view with text.
    // The extra faulty draw call still happens.. Should get rid of that before removing this.
    VirtVScrollHdl(mpVerticalScrollBar);
}

void NotesPanelViewShell::VirtVScrollHdl(ScrollAdaptor* /*pVScroll*/)
{
    OutlinerView* pOutlinerView = mpNotesPanelView->GetOutlinerView();

    if (pOutlinerView)
    {
        pOutlinerView->SetVisArea({ Point(0, mpVerticalScrollBar->GetThumbPos()),
                                    pOutlinerView->GetVisArea().GetSize() });
        pOutlinerView->GetEditView().Invalidate();

        auto currentDocPos = pOutlinerView->GetVisArea().Top();
        auto nDiff = currentDocPos - mpVerticalScrollBar->GetThumbPos();
        pOutlinerView->Scroll(0, nDiff);
    }
}

void NotesPanelViewShell::VisAreaChanged(const ::tools::Rectangle& rRect)
{
    ViewShell::VisAreaChanged(rRect);
    GetViewShellBase().GetDrawController()->FireVisAreaChanged(rRect);
}

void NotesPanelViewShell::onGrabFocus() { mpNotesPanelView->onGrabFocus(); }

void NotesPanelViewShell::onLoseFocus() { mpNotesPanelView->onLoseFocus(); }

void NotesPanelViewShell::ArrangeGUIElements()
{
    // Retrieve the current size (thickness) of the scroll bars.  That is
    // the width of the vertical and the height of the horizontal scroll
    // bar.
    int nScrollBarSize = GetParentWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
    maScrBarWH = Size(nScrollBarSize, nScrollBarSize);

    ViewShell::ArrangeGUIElements();

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow == nullptr)
        return;

    pWindow->SetMinZoomAutoCalc(false);
    mpNotesPanelView->onResize();
}

SdPage* NotesPanelViewShell::GetActualPage() { return getCurrentPage(); }

SdPage* NotesPanelViewShell::getCurrentPage() const
{
    SdPage* pCurrentPage = nullptr;

    std::shared_ptr<ViewShell> pMainViewShell = GetViewShellBase().GetMainViewShell();
    if (pMainViewShell)
        pCurrentPage = pMainViewShell->GetActualPage();

    if (!pCurrentPage)
        return nullptr;

    switch (pCurrentPage->GetPageKind())
    {
        case PageKind::Standard:
            return GetDoc()->GetSdPage((pCurrentPage->GetPageNum() - 1) >> 1, PageKind::Notes);
        case PageKind::Notes:
            return pCurrentPage;
        case PageKind::Handout:
        default:
            return nullptr;
    }
}

css::uno::Reference<css::drawing::XDrawSubController> NotesPanelViewShell::CreateSubController()
{
    // SubController appears is only relevant for MainViewShell
    // NotesPanel isn't meant as a MainViewShell
    return {};
}

void NotesPanelViewShell::ReadFrameViewData(FrameView* /*pView*/)
{
    DrawController& rController(*GetViewShellBase().GetDrawController());
    rController.FireSelectionChangeListener();
}

void NotesPanelViewShell::WriteFrameViewData() {}

/**
 * Activate(): during the first invocation the fields get updated
 */
void NotesPanelViewShell::Activate(bool bIsMDIActivate)
{
    if (!mbInitialized)
    {
        mbInitialized = true;
        SfxRequest aRequest(SID_EDIT_OUTLINER, SfxCallMode::SLOT, GetDoc()->GetItemPool());
        FuPermanent(aRequest);
    }

    ViewShell::Activate(bIsMDIActivate);

    if (bIsMDIActivate)
    {
        OutlinerView* pOutlinerView = mpNotesPanelView->GetOutlinerView();
        ::Outliner* pOutl = pOutlinerView->GetOutliner();
        pOutl->UpdateFields();
    }
}

/**
 * SfxRequests for permanent functions
 */
void NotesPanelViewShell::FuPermanent(SfxRequest& rReq)
{
    if (HasCurrentFunction())
    {
        DeactivateCurrentFunction(true);
    }

    switch (rReq.GetSlot())
    {
        case SID_EDIT_OUTLINER:
        {
            ::Outliner& rOutl = mpNotesPanelView->GetOutliner();
            rOutl.GetUndoManager().Clear();
            rOutl.UpdateFields();

            SetCurrentFunction(FuSimpleOutlinerText::Create(
                this, GetActiveWindow(), mpNotesPanelView.get(), GetDoc(), rReq));

            rReq.Done();
        }
        break;

        default:
            break;
    }

    if (HasOldFunction())
    {
        GetOldFunction()->Deactivate();
        SetOldFunction(nullptr);
    }

    if (HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
        SetOldFunction(GetCurrentFunction());
    }
}

/**
 * Zoom with zoom factor. Inform OutlinerView
 */
void NotesPanelViewShell::SetZoom(::tools::Long nZoom)
{
    ViewShell::SetZoom(nZoom);

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow)
        mpNotesPanelView->onResize();

    GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOM);
    GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOMSLIDER);
}

/**
 * Zoom with zoom rectangle. Inform OutlinerView
 */
void NotesPanelViewShell::SetZoomRect(const ::tools::Rectangle& rZoomRect)
{
    ViewShell::SetZoomRect(rZoomRect);

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow)
        mpNotesPanelView->onResize();

    GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOM);
    GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOMSLIDER);
}

void NotesPanelViewShell::ExecCtrl(SfxRequest& rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            ExecReq(rReq);
            break;
        }

        case SID_OPT_LOCALE_CHANGED:
        {
            mpNotesPanelView->GetOutliner().UpdateFields();
            rReq.Done();
            break;
        }

        default:
            break;
    }
}

void NotesPanelViewShell::GetCtrlState(SfxItemSet& rSet)
{
    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_HYPERLINK_GETLINK))
    {
        SvxHyperlinkItem aHLinkItem;

        OutlinerView* pOLV = mpNotesPanelView->GetOutlinerView();
        if (pOLV)
        {
            const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();
            if (pFieldItem)
            {
                ESelection aSel = pOLV->GetSelection();
                if (abs(aSel.nEndPos - aSel.nStartPos) == 1)
                {
                    const SvxFieldData* pField = pFieldItem->GetField();
                    if (auto pUrlField = dynamic_cast<const SvxURLField*>(pField))
                    {
                        aHLinkItem.SetName(pUrlField->GetRepresentation());
                        aHLinkItem.SetURL(pUrlField->GetURL());
                        aHLinkItem.SetTargetFrame(pUrlField->GetTargetFrame());
                    }
                }
            }
        }
        rSet.Put(aHLinkItem);
    }
    rSet.Put(SfxBoolItem(SID_READONLY_MODE, GetDocSh()->IsReadOnly()));

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN))
        rSet.Put(SfxBoolItem(SID_MAIL_SCROLLBODY_PAGEDOWN, true));

    if (!(SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_HALFWIDTH)
          || SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_FULLWIDTH)
          || SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_HIRAGANA)
          || SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_KATAKANA)))
        return;

    if (!SvtCJKOptions::IsChangeCaseMapEnabled())
    {
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_HALFWIDTH, false);
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_FULLWIDTH, false);
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_HIRAGANA, false);
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_KATAKANA, false);
        rSet.DisableItem(SID_TRANSLITERATE_HALFWIDTH);
        rSet.DisableItem(SID_TRANSLITERATE_FULLWIDTH);
        rSet.DisableItem(SID_TRANSLITERATE_HIRAGANA);
        rSet.DisableItem(SID_TRANSLITERATE_KATAKANA);
    }
    else
    {
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_HALFWIDTH, true);
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_FULLWIDTH, true);
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_HIRAGANA, true);
        GetViewFrame()->GetBindings().SetVisibleState(SID_TRANSLITERATE_KATAKANA, true);
    }
}

void NotesPanelViewShell::GetAttrState(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    SfxAllItemSet aAllSet(*rSet.GetPool());

    while (nWhich)
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich) ? GetPool().GetSlotId(nWhich) : nWhich;

        switch (nSlotId)
        {
            case SID_STYLE_FAMILY2:
            case SID_STYLE_FAMILY3:
            {
                rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_FAMILY5:
            {
                SfxStyleSheet* pStyleSheet = mpNotesPanelView->GetOutlinerView()->GetStyleSheet();

                if (pStyleSheet)
                {
                    pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();

                    if (pStyleSheet)
                    {
                        SfxTemplateItem aItem(nWhich, pStyleSheet->GetName());
                        aAllSet.Put(aItem);
                    }
                }

                if (!pStyleSheet)
                {
                    SfxTemplateItem aItem(nWhich, OUString());
                    aAllSet.Put(aItem);
                }
            }
            break;

            case SID_STYLE_EDIT:
            {
                std::unique_ptr<SfxUInt16Item> pFamilyItem;
                GetViewFrame()->GetBindings().QueryState(SID_STYLE_FAMILY, pFamilyItem);
                if (pFamilyItem
                    && static_cast<SfxStyleFamily>(pFamilyItem->GetValue())
                           == SfxStyleFamily::Pseudo)
                {
                    rSet.DisableItem(nWhich);
                }
            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                OutlinerView* pOV = mpNotesPanelView->GetOutlinerView();
                ESelection aESel(pOV->GetSelection());

                if (aESel.nStartPara != aESel.nEndPara || aESel.nStartPos != aESel.nEndPos)
                    // spanned selection, i.e. StyleSheet and/or
                    // attribution not necessarily unique
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_NEW:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            case SID_STYLE_NEW_BY_EXAMPLE:
            case SID_STYLE_WATERCAN:
            {
                rSet.DisableItem(nWhich);
            }
            break;

            default:
                break;
        }

        nWhich = aIter.NextWhich();
    }

    rSet.Put(aAllSet, false);
    TextObjectBar::GetAttrStateImpl(this, mpView, rSet, nullptr);
}

void NotesPanelViewShell::GetState(SfxItemSet& rSet)
{
    // Iterate over all requested items in the set.
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_SEARCH_ITEM:
            case SID_SEARCH_OPTIONS:
                // Call common (old) implementation in the document shell.
                GetDocSh()->GetState(rSet);
                break;
            default:
                SAL_WARN("sd",
                         "NotesPanelViewShell::GetState(): can not handle which id " << nWhich);
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void NotesPanelViewShell::GetCharState(SfxItemSet& rSet)
{
    TextObjectBar::GetCharStateImpl(this, mpView, rSet);
}

void NotesPanelViewShell::ExecStatusBar(SfxRequest& /*rReq*/) {}

void NotesPanelViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    // Zoom-Item
    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_ATTR_ZOOM))
    {
        sal_uInt16 nZoom = static_cast<sal_uInt16>(GetActiveWindow()->GetZoom());

        std::unique_ptr<SvxZoomItem> pZoomItem(new SvxZoomItem(SvxZoomType::PERCENT, nZoom));

        // limit area
        SvxZoomEnableFlags nZoomValues = SvxZoomEnableFlags::ALL;
        nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
        nZoomValues &= ~SvxZoomEnableFlags::WHOLEPAGE;
        nZoomValues &= ~SvxZoomEnableFlags::PAGEWIDTH;

        pZoomItem->SetValueSet(nZoomValues);
        rSet.Put(std::move(pZoomItem));
    }

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_ATTR_ZOOMSLIDER))
    {
        if (GetDocSh()->IsUIActive() || !GetActiveWindow())
        {
            rSet.DisableItem(SID_ATTR_ZOOMSLIDER);
        }
        else
        {
            sd::Window* pActiveWindow = GetActiveWindow();
            SvxZoomSliderItem aZoomItem(static_cast<sal_uInt16>(pActiveWindow->GetZoom()),
                                        static_cast<sal_uInt16>(pActiveWindow->GetMinZoom()),
                                        static_cast<sal_uInt16>(pActiveWindow->GetMaxZoom()));
            aZoomItem.AddSnappingPoint(100);
            rSet.Put(aZoomItem);
        }
    }

    // TODO: page view and layout strings
    // rSet.Put( SfxStringItem( SID_STATUS_PAGE, aPageStr ) );
    // rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aLayoutStr ) );
}

void NotesPanelViewShell::FuTemporary(SfxRequest& rReq)
{
    DeactivateCurrentFunction();

    OutlinerView* pOutlinerView = mpNotesPanelView->GetOutlinerView();
    sal_uInt16 nSId = rReq.GetSlot();

    switch (nSId)
    {
        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs)
            {
                SvxZoomType eZT = pArgs->Get(SID_ATTR_ZOOM).GetType();
                switch (eZT)
                {
                    case SvxZoomType::PERCENT:
                        SetZoom(static_cast<::tools::Long>(pArgs->Get(SID_ATTR_ZOOM).GetValue()));
                        Invalidate(SID_ATTR_ZOOM);
                        Invalidate(SID_ATTR_ZOOMSLIDER);
                        break;
                    default:
                        break;
                }
                rReq.Done();
            }
            else
            {
                // open the zoom dialog here
                SetCurrentFunction(FuScale::Create(this, GetActiveWindow(), mpNotesPanelView.get(),
                                                   GetDoc(), rReq));
            }
            Cancel();
        }
        break;

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            const SfxUInt16Item* pScale
                = (pArgs && pArgs->Count() == 1) ? rReq.GetArg(SID_ATTR_ZOOMSLIDER) : nullptr;
            if (pScale && CHECK_RANGE(5, pScale->GetValue(), 3000))
            {
                SetZoom(pScale->GetValue());

                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate(SID_ATTR_ZOOM);
                rBindings.Invalidate(SID_ZOOM_IN);
                rBindings.Invalidate(SID_ZOOM_OUT);
                rBindings.Invalidate(SID_ATTR_ZOOMSLIDER);
            }

            Cancel();
            rReq.Done();
            break;
        }

        case SID_ZOOM_IN:
        {
            SetZoom(std::min<::tools::Long>(GetActiveWindow()->GetZoom() * 2,
                                            GetActiveWindow()->GetMaxZoom()));
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic(
                ::tools::Rectangle(Point(0, 0), GetActiveWindow()->GetOutputSizePixel()));
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate(SID_ATTR_ZOOM);
            Invalidate(SID_ZOOM_IN);
            Invalidate(SID_ZOOM_OUT);
            Invalidate(SID_ATTR_ZOOMSLIDER);
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SIZE_REAL:
        {
            SetZoom(100);
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic(
                ::tools::Rectangle(Point(0, 0), GetActiveWindow()->GetOutputSizePixel()));
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate(SID_ATTR_ZOOM);
            Invalidate(SID_ATTR_ZOOMSLIDER);
            Cancel();
            rReq.Done();
        }
        break;

        case SID_ZOOM_OUT:
        {
            SetZoom(std::max<::tools::Long>(GetActiveWindow()->GetZoom() / 2,
                                            GetActiveWindow()->GetMinZoom()));
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic(
                ::tools::Rectangle(Point(0, 0), GetActiveWindow()->GetOutputSizePixel()));
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate(SID_ATTR_ZOOM);
            Invalidate(SID_ZOOM_OUT);
            Invalidate(SID_ZOOM_IN);
            Invalidate(SID_ATTR_ZOOMSLIDER);
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SELECTALL:
        {
            ::Outliner& rOutl = mpNotesPanelView->GetOutliner();
            sal_Int32 nParaCount = rOutl.GetParagraphCount();
            if (nParaCount > 0)
            {
                pOutlinerView->SelectRange(0, nParaCount);
            }
            Cancel();
        }
        break;

        case SID_PRESENTATION:
        case SID_PRESENTATION_CURRENT_SLIDE:
        case SID_REHEARSE_TIMINGS:
        {
            slideshowhelp::ShowSlideShow(rReq, *GetDoc());
            Cancel();
            rReq.Done();
        }
        break;

        case SID_STYLE_EDIT:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if (rReq.GetArgs())
            {
                SetCurrentFunction(FuTemplate::Create(this, GetActiveWindow(),
                                                      mpNotesPanelView.get(), GetDoc(), rReq));
                Cancel();
            }

            rReq.Ignore();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction(FuSlideShowDlg::Create(this, GetActiveWindow(),
                                                      mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_REMOTE_DLG:
        {
#ifdef ENABLE_SDREMOTE
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateRemoteDialog(GetFrameWeld()));
            pDlg->Execute();
#endif
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction(FuCustomShowDlg::Create(this, GetActiveWindow(),
                                                       mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_PHOTOALBUM:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            vcl::Window* pWin = GetActiveWindow();
            ScopedVclPtr<VclAbstractDialog> pDlg(
                pFact->CreateSdPhotoAlbumDialog(pWin ? pWin->GetFrameWeld() : nullptr, GetDoc()));

            pDlg->Execute();

            Cancel();
            rReq.Ignore();
        }
        break;
    }

    if (HasCurrentFunction())
        GetCurrentFunction()->Activate();

    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
    Invalidate(SID_PASTE_UNFORMATTED);
}

void NotesPanelViewShell::FuTemporaryModify(SfxRequest& rReq)
{
    DeactivateCurrentFunction();

    OutlinerView* pOutlinerView = mpNotesPanelView->GetOutlinerView();

    sal_uInt16 nSId = rReq.GetSlot();
    switch (nSId)
    {
        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxHyperlinkItem* pHLItem = &pReqArgs->Get(SID_HYPERLINK_SETLINK);

                SvxFieldItem aURLItem(
                    SvxURLField(pHLItem->GetURL(), pHLItem->GetName(), SvxURLFormat::Repr),
                    EE_FEATURE_FIELD);
                ESelection aSel(pOutlinerView->GetSelection());
                pOutlinerView->InsertField(aURLItem);
                if (aSel.nStartPos <= aSel.nEndPos)
                    aSel.nEndPos = aSel.nStartPos + 1;
                else
                    aSel.nStartPos = aSel.nEndPos + 1;
                pOutlinerView->SetSelection(aSel);
            }

            Cancel();
            rReq.Ignore();
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case FN_INSERT_NNBSP:
        case SID_INSERT_RLM:
        case SID_INSERT_LRM:
        case SID_INSERT_WJ:
        case SID_INSERT_ZWSP:
        case SID_CHARMAP:
        {
            SetCurrentFunction(
                FuBullet::Create(this, GetActiveWindow(), mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        case FN_SVX_SET_BULLET:
        case FN_SVX_SET_NUMBER:
        {
            SetCurrentFunction(FuBulletAndPosition::Create(this, GetActiveWindow(),
                                                           mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction(FuThesaurus::Create(this, GetActiveWindow(), mpNotesPanelView.get(),
                                                   GetDoc(), rReq));
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:
        {
            SetCurrentFunction(
                FuChar::Create(this, GetActiveWindow(), mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_INSERTFILE:
        {
            SetCurrentFunction(FuInsertFile::Create(this, GetActiveWindow(), mpNotesPanelView.get(),
                                                    GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_PRESENTATIONOBJECT:
        {
            SetCurrentFunction(FuPresentationObjects::Create(
                this, GetActiveWindow(), mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_SET_DEFAULT:
        {
            pOutlinerView->RemoveAttribs(true); // sal_True = also paragraph attributes
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            SetCurrentFunction(FuSummaryPage::Create(this, GetActiveWindow(),
                                                     mpNotesPanelView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            SetCurrentFunction(FuExpandPage::Create(this, GetActiveWindow(), mpNotesPanelView.get(),
                                                    GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_PAGE_TITLE:
        case SID_INSERT_FLD_PAGES:
        case SID_INSERT_FLD_FILE:
        {
            std::unique_ptr<SvxFieldItem> pFieldItem;

            switch (nSId)
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxDateField(Date(Date::SYSTEM), SvxDateType::Fix), EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem.reset(new SvxFieldItem(SvxDateField(), EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxExtTimeField(::tools::Time(::tools::Time::SYSTEM), SvxTimeType::Fix),
                        EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem.reset(new SvxFieldItem(SvxExtTimeField(), EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvtUserOptions aUserOptions;
                    pFieldItem.reset(new SvxFieldItem(SvxAuthorField(aUserOptions.GetFirstName(),
                                                                     aUserOptions.GetLastName(),
                                                                     aUserOptions.GetID()),
                                                      EE_FEATURE_FIELD));
                }
                break;

                case SID_INSERT_FLD_PAGE:
                    pFieldItem.reset(new SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_PAGE_TITLE:
                    pFieldItem.reset(new SvxFieldItem(SvxPageTitleField(), EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_PAGES:
                    pFieldItem.reset(new SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD));
                    break;

                case SID_INSERT_FLD_FILE:
                {
                    OUString aName;
                    if (GetDocSh()->HasName())
                        aName = GetDocSh()->GetMedium()->GetName();
                    pFieldItem.reset(new SvxFieldItem(SvxExtFileField(aName), EE_FEATURE_FIELD));
                }
                break;
            }

            const SvxFieldItem* pOldFldItem = pOutlinerView->GetFieldAtSelection();

            if (pOldFldItem
                && (nullptr != dynamic_cast<const SvxURLField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxDateField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxTimeField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxExtTimeField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxExtFileField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxAuthorField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxPageField*>(pOldFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxPagesField*>(pOldFldItem->GetField())))
            {
                // select field, so it gets deleted on Insert
                ESelection aSel = pOutlinerView->GetSelection();
                if (aSel.nStartPos == aSel.nEndPos)
                    aSel.nEndPos++;
                pOutlinerView->SetSelection(aSel);
            }

            if (pFieldItem)
                pOutlinerView->InsertField(*pFieldItem);

            pFieldItem.reset();

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            const SvxFieldItem* pFldItem = pOutlinerView->GetFieldAtSelection();

            if (pFldItem
                && (nullptr != dynamic_cast<const SvxDateField*>(pFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxAuthorField*>(pFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxExtFileField*>(pFldItem->GetField())
                    || nullptr != dynamic_cast<const SvxExtTimeField*>(pFldItem->GetField())))
            {
                // Dialog...
                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                vcl::Window* pWin = GetActiveWindow();
                ScopedVclPtr<AbstractSdModifyFieldDlg> pDlg(pFact->CreateSdModifyFieldDlg(
                    pWin ? pWin->GetFrameWeld() : nullptr, pFldItem->GetField(),
                    pOutlinerView->GetAttribs()));
                if (pDlg->Execute() == RET_OK)
                {
                    std::unique_ptr<SvxFieldData> pField(pDlg->GetField());
                    if (pField)
                    {
                        SvxFieldItem aFieldItem(*pField, EE_FEATURE_FIELD);
                        //pOLV->DeleteSelected(); <-- unfortunately missing!
                        // select field, so it gets deleted on Insert
                        ESelection aSel = pOutlinerView->GetSelection();
                        bool bSel = true;
                        if (aSel.nStartPos == aSel.nEndPos)
                        {
                            bSel = false;
                            aSel.nEndPos++;
                        }
                        pOutlinerView->SetSelection(aSel);

                        pOutlinerView->InsertField(aFieldItem);

                        // reset selection to original state
                        if (!bSel)
                            aSel.nEndPos--;
                        pOutlinerView->SetSelection(aSel);

                        pField.reset();
                    }

                    SfxItemSet aSet(pDlg->GetItemSet());
                    if (aSet.Count())
                    {
                        pOutlinerView->SetAttribs(aSet);

                        ::Outliner* pOutliner = pOutlinerView->GetOutliner();
                        if (pOutliner)
                            pOutliner->UpdateFields();
                    }
                }
            }

            Cancel();
            rReq.Ignore();
        }
        break;
    }

    if (HasCurrentFunction())
        GetCurrentFunction()->Activate();

    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
    Invalidate(SID_PASTE_UNFORMATTED);
}

void NotesPanelViewShell::FuSupport(SfxRequest& rReq)
{
    if (rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(
            static_cast<SfxStyleFamily>(rReq.GetArgs()->Get(SID_STYLE_FAMILY).GetValue()));

    bool bPreviewState = false;
    sal_uInt16 nSlot = rReq.GetSlot();

    switch (nSlot)
    {
        case SID_CUT:
        {
            if (HasCurrentFunction())
            {
                GetCurrentFunction()->DoCut();
            }
            else if (mpNotesPanelView)
            {
                mpNotesPanelView->DoCut();
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_COPY:
        {
            if (HasCurrentFunction())
            {
                GetCurrentFunction()->DoCopy();
            }
            else if (mpNotesPanelView)
            {
                mpNotesPanelView->DoCopy();
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_PASTE:
        {
            if (HasCurrentFunction())
            {
                GetCurrentFunction()->DoPaste();
            }
            else if (mpNotesPanelView)
            {
                mpNotesPanelView->DoPaste();
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_PASTE_UNFORMATTED:
        {
            if (HasCurrentFunction())
            {
                GetCurrentFunction()->DoPasteUnformatted();
            }
            else if (mpNotesPanelView)
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard(GetActiveWindow()));
                if (aDataHelper.GetTransferable().is())
                {
                    sal_Int8 nAction = DND_ACTION_COPY;
                    mpNotesPanelView->InsertData(
                        aDataHelper,
                        GetActiveWindow()->PixelToLogic(
                            ::tools::Rectangle(Point(), GetActiveWindow()->GetOutputSizePixel())
                                .Center()),
                        nAction, false, SotClipboardFormatId::STRING);
                }
            }

            rReq.Ignore();
        }
        break;
        case SID_DELETE:
        {
            if (mpNotesPanelView)
            {
                OutlinerView* pOutlView = mpNotesPanelView->GetOutlinerView();
                if (pOutlView)
                {
                    vcl::KeyCode aKCode(KEY_DELETE);
                    KeyEvent aKEvt(0, aKCode);
                    pOutlView->PostKeyEvent(aKEvt);

                    rtl::Reference<FuPoor> xFunc(GetCurrentFunction());
                    FuOutlineText* pFuOutlineText = dynamic_cast<FuOutlineText*>(xFunc.get());
                    if (pFuOutlineText)
                        pFuOutlineText->UpdateForKeyPress(aKEvt);
                }
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_DRAWINGMODE:
        case SID_SLIDE_MASTER_MODE:
        case SID_NOTES_MODE:
        case SID_NOTES_MASTER_MODE:
        case SID_HANDOUT_MASTER_MODE:
        case SID_SLIDE_SORTER_MODE:
        case SID_OUTLINE_MODE:
            framework::FrameworkHelper::Instance(GetViewShellBase())
                ->HandleModeChangeSlot(nSlot, rReq);
            rReq.Done();
            break;

        case SID_RULER:
            SetRuler(!HasRuler());
            Invalidate(SID_RULER);
            rReq.Done();
            break;

        case SID_ZOOM_PREV:
        {
            if (mpZoomList->IsPreviousPossible())
            {
                SetZoomRect(mpZoomList->GetPreviousZoomRect());
            }
            rReq.Done();
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (mpZoomList->IsNextPossible())
            {
                SetZoomRect(mpZoomList->GetNextZoomRect());
            }
            rReq.Done();
        }
        break;

        case SID_AUTOSPELL_CHECK:
        {
            GetDoc()->SetOnlineSpell(!GetDoc()->GetOnlineSpell());
            rReq.Done();
        }
        break;

        case SID_TRANSLITERATE_SENTENCE_CASE:
        case SID_TRANSLITERATE_TITLE_CASE:
        case SID_TRANSLITERATE_TOGGLE_CASE:
        case SID_TRANSLITERATE_UPPER:
        case SID_TRANSLITERATE_LOWER:
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAKANA:
        {
            OutlinerView* pOLV = mpNotesPanelView ? mpNotesPanelView->GetOutlinerView() : nullptr;
            if (pOLV)
            {
                TransliterationFlags nType = TransliterationFlags::NONE;

                switch (nSlot)
                {
                    case SID_TRANSLITERATE_SENTENCE_CASE:
                        nType = TransliterationFlags::SENTENCE_CASE;
                        break;
                    case SID_TRANSLITERATE_TITLE_CASE:
                        nType = TransliterationFlags::TITLE_CASE;
                        break;
                    case SID_TRANSLITERATE_TOGGLE_CASE:
                        nType = TransliterationFlags::TOGGLE_CASE;
                        break;
                    case SID_TRANSLITERATE_UPPER:
                        nType = TransliterationFlags::LOWERCASE_UPPERCASE;
                        break;
                    case SID_TRANSLITERATE_LOWER:
                        nType = TransliterationFlags::UPPERCASE_LOWERCASE;
                        break;
                    case SID_TRANSLITERATE_HALFWIDTH:
                        nType = TransliterationFlags::FULLWIDTH_HALFWIDTH;
                        break;
                    case SID_TRANSLITERATE_FULLWIDTH:
                        nType = TransliterationFlags::HALFWIDTH_FULLWIDTH;
                        break;
                    case SID_TRANSLITERATE_HIRAGANA:
                        nType = TransliterationFlags::KATAKANA_HIRAGANA;
                        break;
                    case SID_TRANSLITERATE_KATAKANA:
                        nType = TransliterationFlags::HIRAGANA_KATAKANA;
                        break;
                }

                pOLV->TransliterateText(nType);
            }

            rReq.Done();
            bPreviewState = true;
        }
        break;

        // added Undo/Redo handling
        case SID_UNDO:
        {
            ImpSidUndo(rReq);
        }
        break;
        case SID_REDO:
        {
            ImpSidRedo(rReq);
        }
        break;

        default:
            break;
    }

    if (bPreviewState)
        Invalidate(SID_PREVIEW_STATE);

    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
}

void NotesPanelViewShell::Execute(SfxRequest& rReq)
{
    switch (rReq.GetSlot())
    {
        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            sd::View* pView = nullptr;
            if (auto pMainViewSh = GetViewShellBase().GetMainViewShell())
                pView = pMainViewSh->GetView();

            if (pReqArgs)
            {
                if (pView)
                {
                    rtl::Reference<FuSearch>& xFuSearch
                        = pView->getSearchContext().getFunctionSearch();

                    if (!xFuSearch.is())
                    {
                        xFuSearch = rtl::Reference<FuSearch>(FuSearch::createPtr(
                            this, this->GetActiveWindow(), pView, GetDoc(), rReq));

                        pView->getSearchContext().setSearchFunction(xFuSearch);
                    }

                    if (xFuSearch.is())
                    {
                        const SvxSearchItem& rSearchItem = pReqArgs->Get(SID_SEARCH_ITEM);

                        SD_MOD()->SetSearchItem(
                            std::unique_ptr<SvxSearchItem>(rSearchItem.Clone()));
                        xFuSearch->SearchAndReplace(&rSearchItem);
                    }
                }
            }
            rReq.Done();
        }
        break;

        case SID_SEARCH_ITEM:
            // Forward this request to the common (old) code of the
            // document shell.
            GetDocSh()->Execute(rReq);
            break;

        case SID_SPELL_DIALOG:
        {
            SfxViewFrame* pViewFrame = GetViewFrame();
            if (rReq.GetArgs() != nullptr)
                pViewFrame->SetChildWindow(
                    SID_SPELL_DIALOG,
                    static_cast<const SfxBoolItem&>(rReq.GetArgs()->Get(SID_SPELL_DIALOG))
                        .GetValue());
            else
                pViewFrame->ToggleChildWindow(SID_SPELL_DIALOG);

            pViewFrame->GetBindings().Invalidate(SID_SPELL_DIALOG);
            rReq.Done();
        }
        break;

        default:
            break;
    }
}

void NotesPanelViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    // first the base classes
    ViewShell::MouseButtonUp(rMEvt, pWin);

    Invalidate(SID_STYLE_EDIT);
    Invalidate(SID_STYLE_NEW);
    Invalidate(SID_STYLE_DELETE);
    Invalidate(SID_STYLE_HIDE);
    Invalidate(SID_STYLE_SHOW);
    Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE);
    Invalidate(SID_STYLE_NEW_BY_EXAMPLE);
    Invalidate(SID_STYLE_WATERCAN);
    Invalidate(SID_STYLE_FAMILY5);
}

void NotesPanelViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        GetActiveWindow()->ReleaseMouse();

        OutlinerView* pOLV = mpNotesPanelView->GetOutlinerView();
        Point aPos(rCEvt.GetMousePosPixel());

        if (pOLV && pOLV->IsWrongSpelledWordAtPos(aPos))
        {
            // Popup for Online-Spelling now handled by DrawDocShell
            Link<SpellCallbackInfo&, void> aLink
                = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

            pOLV->ExecuteSpellPopup(aPos, aLink);
            pOLV->GetEditView().Invalidate();
        }
        else
        {
            GetViewFrame()->GetDispatcher()->ExecutePopup("drawtext");
        }
    }
    else
    {
        ViewShell::Command(rCEvt, pWin);
    }
}

bool NotesPanelViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    bool bReturn = false;

    if (HasCurrentFunction())
    {
        bReturn = GetCurrentFunction()->KeyInput(rKEvt);
    }
    else
    {
        bReturn = ViewShell::KeyInput(rKEvt, pWin);
    }

    Invalidate(SID_STYLE_EDIT);
    Invalidate(SID_STYLE_NEW);
    Invalidate(SID_STYLE_DELETE);
    Invalidate(SID_STYLE_HIDE);
    Invalidate(SID_STYLE_SHOW);
    Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE);
    Invalidate(SID_STYLE_NEW_BY_EXAMPLE);
    Invalidate(SID_STYLE_WATERCAN);
    Invalidate(SID_STYLE_FAMILY5);

    return bReturn;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
