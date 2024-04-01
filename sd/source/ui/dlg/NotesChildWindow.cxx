/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <EventMultiplexer.hxx>
#include <NotesChildWindow.hxx>
#include <Outliner.hxx>
#include <ViewShellBase.hxx>
#include <app.hrc>
#include <config_wasm_strip.h>
#include <drawdoc.hxx>
#include <drawview.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>

#include <editeng/crossedoutitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editstat.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/ptrstyle.hxx>
#include <strings.hrc>

namespace sd
{
SFX_IMPL_DOCKINGWINDOW_WITHID(NotesChildWindow, SID_NOTES_WINDOW);

NotesChildWindow::NotesChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId,
                                   SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParentWindow, nId)
{
    VclPtr<NotesChildDockingWindow> pWin
        = VclPtr<NotesChildDockingWindow>::Create(pBindings, this, pParentWindow);
    SetWindow(pWin);
    SetAlignment(SfxChildAlignment::BOTTOM);
    pWin->setDeferredProperties();
    pWin->SetSizePixel({ 200, 200 });
    pWin->set_border_width(3);
    pWin->set_margin_top(11);
    pWin->Initialize(pInfo);
}

NotesChildDockingWindow::NotesChildDockingWindow(SfxBindings* _pBindings,
                                                 SfxChildWindow* pChildWindow, Window* pParent)
    : SfxDockingWindow(_pBindings, pChildWindow, pParent, "NotesChildEditWindow",
                       "modules/simpress/ui/noteschildwindow.ui")
    , mpViewShellBase(ViewShellBase::GetViewShellBase(_pBindings->GetDispatcher()->GetFrame()))
    , m_xEditWindow(
          new NotesEditWindow(*this, m_xBuilder->weld_scrolled_window("scrolledwin", true)))
{
    mpOutliner = std::make_unique<Outliner>(&mpViewShellBase->GetDocShell()->GetPool(),
                                            OutlinerMode::TextObject);

    mpOutlinerView = std::make_unique<OutlinerView>(mpOutliner.get(), nullptr);
    mpOutliner->InsertView(mpOutlinerView.get());

    m_xEditWindowWeld
        = std::make_unique<weld::CustomWeld>(*m_xBuilder, "noteschildeditview", *m_xEditWindow);
}

DrawViewShell* NotesChildDockingWindow::GetDrawViewShell()
{
    auto pDocShell = GetViewShellBase()->GetDocShell();
    if (!pDocShell)
        return nullptr;

    return dynamic_cast<DrawViewShell*>(pDocShell->GetViewShell());
}

NotesChildDockingWindow::~NotesChildDockingWindow() { disposeOnce(); }

void NotesChildDockingWindow::dispose()
{
    m_xEditWindow.reset();
    m_xEditWindowWeld.reset();
    SfxDockingWindow::dispose();
}

NotesEditWindow::NotesEditWindow(NotesChildDockingWindow& rParentWindow,
                                 std::unique_ptr<weld::ScrolledWindow> pScrolledWindow)
    : mrParentWindow(rParentWindow)
    , m_xScrolledWindow(std::move(pScrolledWindow))
    , aModifyIdle("NotesEditWindow ModifyIdle")
{
    aModifyIdle.SetInvokeHandler(LINK(this, NotesEditWindow, ModifyTimerHdl));
    aModifyIdle.SetPriority(TaskPriority::LOWEST);

    SetAcceptsTab(true);
    m_xScrolledWindow->connect_vadjustment_changed(LINK(this, NotesEditWindow, ScrollHdl));
}

NotesEditWindow::~NotesEditWindow()
{
    aModifyIdle.Stop();
    m_xScrolledWindow.reset();
    if (!mrParentWindow.GetViewShellBase())
        return;

    mrParentWindow.GetViewShellBase()->GetEventMultiplexer()->RemoveEventListener(
        LINK(this, NotesEditWindow, EventMultiplexerListener));
}

IMPL_LINK(NotesEditWindow, EventMultiplexerListener, tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::CurrentPageChanged:
        case EventMultiplexerEventId::MainViewRemoved:
        case EventMultiplexerEventId::MainViewAdded:
            provideNoteText();
            break;
        default:
            break;
    }
}

void NotesEditWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(pDrawingArea->get_size_request());
    if (aSize.Width() == -1)
        aSize.setWidth(500);
    if (aSize.Height() == -1)
        aSize.setHeight(100);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    SetOutputSizePixel(aSize);

    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    rDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    rDevice.SetBackground(aBgColor);

    Size aOutputSize(rDevice.PixelToLogic(aSize));

    EditView* pEditView = GetEditView();
    pEditView->setEditViewCallbacks(this);

    EditEngine* pEditEngine = GetEditEngine();
    pEditEngine->SetPaperSize(aOutputSize);
    pEditEngine->SetRefDevice(&rDevice);

    pEditEngine->SetControlWord(pEditEngine->GetControlWord() | EEControlBits::MARKFIELDS);
    pEditView->SetOutputArea(::tools::Rectangle(Point(0, 0), aOutputSize));

    pEditView->SetBackgroundColor(aBgColor);

    pDrawingArea->set_cursor(PointerStyle::Text);

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    InitAccessible();
#endif

    mrParentWindow.GetViewShellBase()->GetEventMultiplexer()->AddEventListener(
        LINK(this, NotesEditWindow, EventMultiplexerListener));

    // For setGlobalScale to work correctly EEControlBits::STRETCHING must be set.
    mrParentWindow.GetOutliner()->SetControlWord(mrParentWindow.GetOutliner()->GetControlWord()
                                                 | EEControlBits::STRETCHING);
    mrParentWindow.GetOutliner()->setScalingParameters({ 30.0, 30.0 });

    provideNoteText();

    GetEditEngine()->SetStatusEventHdl(LINK(this, NotesEditWindow, EditStatusHdl));
}

void NotesEditWindow::doScroll()
{
    if (m_xEditView)
    {
        auto currentDocPos = m_xEditView->GetVisArea().Top();
        auto nDiff = currentDocPos - m_xScrolledWindow->vadjustment_get_value();
        // we expect SetScrollBarRange callback to be triggered by Scroll
        // to set where we ended up
        m_xEditView->Scroll(0, nDiff);
    }
}

void NotesEditWindow::setScrollBarRange()
{
    EditEngine* pEditEngine = GetEditEngine();
    if (!pEditEngine)
        return;
    if (!m_xScrolledWindow)
        return;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    int nVUpper = pEditEngine->GetTextHeight();
    int nVCurrentDocPos = pEditView->GetVisArea().Top();
    const Size aOut(pEditView->GetOutputArea().GetSize());
    int nVStepIncrement = aOut.Height() * 2 / 10;
    int nVPageIncrement = aOut.Height() * 8 / 10;
    int nVPageSize = aOut.Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nVPageSize = std::min(nVPageSize, nVUpper);

    m_xScrolledWindow->vadjustment_configure(nVCurrentDocPos, 0, nVUpper, nVStepIncrement,
                                             nVPageIncrement, nVPageSize);
}

void NotesEditWindow::showContextMenu(const Point& rPos)
{
    ::tools::Rectangle aRect(rPos, Size(1, 1));
    weld::Widget* pPopupParent = GetDrawingArea();
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(pPopupParent, "modules/simpress/ui/notespanelcontextmenu.ui"));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu("menu"));

    auto xFrame = mrParentWindow.GetDrawViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));

    SfxItemSet aSet(mrParentWindow.GetOutlinerView()->GetAttribs());
    int nInsertPos = 0;
    xMenu->insert(nInsertPos++, ".uno:Bold",
                  vcl::CommandInfoProvider::GetMenuLabelForCommand(
                      vcl::CommandInfoProvider::GetCommandProperties(".uno:Bold", aModuleName)),
                  nullptr, nullptr,
                  vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:Bold", xFrame),
                  TRISTATE_TRUE);

    if (aSet.GetItemState(EE_CHAR_WEIGHT) == SfxItemState::SET)
    {
        if (aSet.Get(EE_CHAR_WEIGHT).GetWeight() == WEIGHT_BOLD)
            xMenu->set_active(".uno:Bold", true);
    }

    xMenu->insert(nInsertPos++, ".uno:Italic",
                  vcl::CommandInfoProvider::GetMenuLabelForCommand(
                      vcl::CommandInfoProvider::GetCommandProperties(".uno:Italic", aModuleName)),
                  nullptr, nullptr,
                  vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:Italic", xFrame),
                  TRISTATE_TRUE);

    if (aSet.GetItemState(EE_CHAR_ITALIC) == SfxItemState::SET)
    {
        if (aSet.Get(EE_CHAR_ITALIC).GetPosture() != ITALIC_NONE)
            xMenu->set_active(".uno:Italic", true);
    }

    xMenu->insert(
        nInsertPos++, ".uno:Underline",
        vcl::CommandInfoProvider::GetMenuLabelForCommand(
            vcl::CommandInfoProvider::GetCommandProperties(".uno:Underline", aModuleName)),
        nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:Underline", xFrame),
        TRISTATE_TRUE);

    if (aSet.GetItemState(EE_CHAR_UNDERLINE) == SfxItemState::SET)
    {
        if (aSet.Get(EE_CHAR_UNDERLINE).GetLineStyle() != LINESTYLE_NONE)
            xMenu->set_active(".uno:Underline", true);
    }

    xMenu->insert(
        nInsertPos++, ".uno:Strikeout",
        vcl::CommandInfoProvider::GetMenuLabelForCommand(
            vcl::CommandInfoProvider::GetCommandProperties(".uno:Strikeout", aModuleName)),
        nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:Strikeout", xFrame),
        TRISTATE_TRUE);

    if (aSet.GetItemState(EE_CHAR_STRIKEOUT) == SfxItemState::SET)
    {
        if (aSet.Get(EE_CHAR_STRIKEOUT).GetStrikeout() != STRIKEOUT_NONE)
            xMenu->set_active(".uno:Strikeout", true);
    }

    xMenu->insert_separator(nInsertPos++, "separator2");

    xMenu->insert(nInsertPos++, ".uno:Copy",
                  vcl::CommandInfoProvider::GetMenuLabelForCommand(
                      vcl::CommandInfoProvider::GetCommandProperties(".uno:Copy", aModuleName)),
                  nullptr, nullptr,
                  vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:Copy", xFrame),
                  TRISTATE_INDET);

    xMenu->insert(nInsertPos++, ".uno:Paste",
                  vcl::CommandInfoProvider::GetMenuLabelForCommand(
                      vcl::CommandInfoProvider::GetCommandProperties(".uno:Paste", aModuleName)),
                  nullptr, nullptr,
                  vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:Paste", xFrame),
                  TRISTATE_INDET);

    bool bCanPaste = false;
    {
        TransferableDataHelper aDataHelper(
            TransferableDataHelper::CreateFromClipboard(GetClipboard()));
        bCanPaste = aDataHelper.GetFormatCount() != 0;
    }

    xMenu->insert_separator(nInsertPos++, "separator3");

    xMenu->insert(
        nInsertPos++, ".uno:DefaultBullet",
        vcl::CommandInfoProvider::GetMenuLabelForCommand(
            vcl::CommandInfoProvider::GetCommandProperties(".uno:DefaultBullet", aModuleName)),
        nullptr, nullptr,
        vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:DefaultBullet", xFrame),
        TRISTATE_TRUE);

    ESelection aSel(GetEditView()->GetSelection());
    aSel.Adjust();
    bool bBulletsEnabled = true;
    for (sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++)
    {
        if (mrParentWindow.GetOutliner()->GetDepth(nPara) == -1)
        {
            bBulletsEnabled = false;
            break;
        }
    }

    if (bBulletsEnabled)
        xMenu->set_active(".uno:DefaultBullet", true);

    xMenu->insert(
        nInsertPos++, ".uno:OutlineLeft",
        vcl::CommandInfoProvider::GetMenuLabelForCommand(
            vcl::CommandInfoProvider::GetCommandProperties(".uno:OutlineLeft", aModuleName)),
        nullptr, nullptr,
        vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:OutlineLeft", xFrame),
        TRISTATE_INDET);

    xMenu->insert(
        nInsertPos++, ".uno:OutlineRight",
        vcl::CommandInfoProvider::GetMenuLabelForCommand(
            vcl::CommandInfoProvider::GetCommandProperties(".uno:OutlineRight", aModuleName)),
        nullptr, nullptr,
        vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:OutlineRight", xFrame),
        TRISTATE_INDET);

    xMenu->set_sensitive(".uno:Copy", mrParentWindow.GetOutlinerView()->HasSelection());
    xMenu->set_sensitive(".uno:Paste", bCanPaste);

    auto sId = xMenu->popup_at_rect(pPopupParent, aRect);

    if (sId == ".uno:Copy")
    {
        mrParentWindow.GetOutlinerView()->Copy();
    }
    else if (sId == ".uno:Paste")
    {
        mrParentWindow.GetOutlinerView()->PasteSpecial();
    }
    else if (sId == ".uno:DefaultBullet")
    {
        mrParentWindow.GetOutlinerView()->ToggleBullets();
    }
    else if (sId == ".uno:OutlineLeft" || sId == ".uno:OutlineRight")
    {
        bool isOutlineLeft = sId == ".uno:OutlineLeft";
        for (sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++)
        {
            auto nCurrentDepth = mrParentWindow.GetOutliner()->GetDepth(nPara);
            if (nCurrentDepth == -1)
                continue;

            mrParentWindow.GetOutlinerView()->SetDepth(nPara, isOutlineLeft ? --nCurrentDepth
                                                                            : ++nCurrentDepth);
        }
    }
    else if (!sId.isEmpty())
    {
        SfxItemSet aEditAttr(mrParentWindow.GetOutlinerView()->GetAttribs());
        SfxItemSet aNewAttr(mrParentWindow.GetOutliner()->GetEmptyItemSet());

        if (sId == ".uno:Bold")
        {
            FontWeight eFW = aEditAttr.Get(EE_CHAR_WEIGHT).GetWeight();
            aNewAttr.Put(
                SvxWeightItem(eFW == WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT));
        }
        else if (sId == ".uno:Italic")
        {
            FontItalic eFI = aEditAttr.Get(EE_CHAR_ITALIC).GetPosture();
            aNewAttr.Put(
                SvxPostureItem(eFI == ITALIC_NORMAL ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC));
        }
        else if (sId == ".uno:Underline")
        {
            FontLineStyle eFU = aEditAttr.Get(EE_CHAR_UNDERLINE).GetLineStyle();
            aNewAttr.Put(SvxUnderlineItem(
                eFU == LINESTYLE_SINGLE ? LINESTYLE_NONE : LINESTYLE_SINGLE, EE_CHAR_UNDERLINE));
        }
        else if (sId == ".uno:Strikeout")
        {
            FontStrikeout eFSO = aEditAttr.Get(EE_CHAR_STRIKEOUT).GetStrikeout();
            aNewAttr.Put(SvxCrossedOutItem(
                eFSO == STRIKEOUT_SINGLE ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT));
        }

        mrParentWindow.GetOutlinerView()->SetAttribs(aNewAttr);
    }
}

void NotesEditWindow::EditViewScrollStateChange() { setScrollBarRange(); }

EditView* NotesEditWindow::GetEditView() const
{
    if (!mrParentWindow.GetOutlinerView())
        return nullptr;
    return &mrParentWindow.GetOutlinerView()->GetEditView();
}

EditEngine* NotesEditWindow::GetEditEngine() const
{
    if (!mrParentWindow.GetOutlinerView())
        return nullptr;
    return &mrParentWindow.GetOutlinerView()->GetEditView().getEditEngine();
}

void NotesEditWindow::GetFocus()
{
    if (auto pDrawViewShell = mrParentWindow.GetDrawViewShell())
    {
        if (auto pDrawView = pDrawViewShell->GetDrawView())
        {
            pDrawView->EndTextEditCurrentView();
            pDrawView->UnmarkAll();
        }
    }

    if (mpTextObj && mpTextObj->IsEmptyPresObj())
    {
        // clear the "Click to add Notes" text on enter of the window.
        mrParentWindow.GetOutliner()->SetToEmptyText();
    }
    WeldEditView::GetFocus();
}

bool NotesEditWindow::CanFocus() const
{
    auto pDocShell = mrParentWindow.GetViewShellBase()->GetDocShell();
    if (pDocShell && pDocShell->IsReadOnly())
        return false;

    return mpTextObj;
}

void NotesEditWindow::LoseFocus()
{
    aModifyIdle.Stop();
    if (mpTextObj)
    {
        if (GetText().getLength() == 0)
        {
            // if the notes are empty restore the placeholder text and state.
            SdPage* pPage = dynamic_cast<SdPage*>(mpTextObj->getSdrPageFromSdrObject());
            if (pPage)
                pPage->RestoreDefaultText(mpTextObj);
        }
        else
            setNotesToDoc();
    }

    WeldEditView::LoseFocus();
}

bool NotesEditWindow::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        showContextMenu(rCEvt.GetMousePosPixel());
        return true;
    }

    return WeldEditView::Command(rCEvt);
}

void NotesEditWindow::provideNoteText()
{
    removeListener();
    mpTextObj = nullptr;
    SetText(SdResId(STR_PRESOBJ_NOTESTEXT_MISSING));

    const auto pDrawViewShell = mrParentWindow.GetDrawViewShell();
    if (!pDrawViewShell)
        return;

    SdPage* pCurrentPage = pDrawViewShell->getCurrentPage();
    if (!pCurrentPage)
        return;

    SdDrawDocument* pDoc = pDrawViewShell->GetDoc();
    if (!pDoc)
        return;

    SdPage* pNotesPage = pDoc->GetSdPage((pCurrentPage->GetPageNum() - 1) >> 1, PageKind::Notes);
    if (!pNotesPage)
        return;

    SdrObject* pNotesObj = pNotesPage->GetPresObj(PresObjKind::Notes);
    if (!pNotesObj)
        return;

    mpTextObj = dynamic_cast<SdrTextObj*>(pNotesObj);
    addListener();
    getNotesFromDoc();
}

void NotesEditWindow::removeListener()
{
    if (mpTextObj)
        mpTextObj->RemoveListener(*this);
}
void NotesEditWindow::addListener()
{
    if (mpTextObj)
        mpTextObj->AddListener(*this);
}

void NotesEditWindow::setListenerIgnored(bool bIgnore) { mbIgnoreNotifications = bIgnore; }
bool NotesEditWindow::isListenerIgnored() { return mbIgnoreNotifications; }

void NotesEditWindow::getNotesFromDoc()
{
    if (!mpTextObj)
        return;

    // Ignore notifications that will rebound from updating the text
    SetModifyHdl(Link<LinkParamNone*, void>());
    setListenerIgnored(true);

    if (OutlinerParaObject* pPara = mpTextObj->GetOutlinerParaObject())
        mrParentWindow.GetOutliner()->SetText(*pPara);

    setListenerIgnored(false);
    SetModifyHdl(LINK(this, NotesEditWindow, EditModifiedHdl));
}

void NotesEditWindow::setNotesToDoc()
{
    if (!mpTextObj)
        return;

    setListenerIgnored(true);

    std::optional<OutlinerParaObject> pNewText = mrParentWindow.GetOutliner()->CreateParaObject();
    mpTextObj->SetOutlinerParaObject(std::move(pNewText));
    if (mpTextObj->IsEmptyPresObj())
        mpTextObj->SetEmptyPresObj(false);

    setListenerIgnored(false);
}

void NotesEditWindow::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (isListenerIgnored())
        return;

    if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
    {
        const SdrHint& rSdrHint = reinterpret_cast<const SdrHint&>(rHint);
        switch (rSdrHint.GetKind())
        {
            case SdrHintKind::ObjectRemoved:
            case SdrHintKind::ModelCleared:
                provideNoteText();
                break;
            case SdrHintKind::ObjectChange:
            case SdrHintKind::EndEdit:
                getNotesFromDoc();
                break;
            default:
                break;
        }
    }
}

IMPL_LINK_NOARG(NotesEditWindow, EditStatusHdl, EditStatus&, void) { Resize(); }

IMPL_LINK_NOARG(NotesEditWindow, EditModifiedHdl, LinkParamNone*, void)
{
    // EditEngine calls ModifyHdl many times in succession for some edits.
    // (e.g. when deleting multiple lines)
    // Debounce the rapid ModifyHdl calls using a timer.
    aModifyIdle.Start();
    return;
}

IMPL_LINK_NOARG(NotesEditWindow, ModifyTimerHdl, Timer*, void)
{
    setNotesToDoc();
    aModifyIdle.Stop();
}

IMPL_LINK(NotesEditWindow, ScrollHdl, weld::ScrolledWindow&, rScrolledWindow, void)
{
    if (EditView* pEditView = GetEditView())
    {
        pEditView->SetVisArea(::tools::Rectangle(Point(0, rScrolledWindow.vadjustment_get_value()),
                                                 pEditView->GetVisArea().GetSize()));
        pEditView->Invalidate();
    }
    doScroll();
}

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
