/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/builderpage.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>

BuilderPage::BuilderPage(weld::Widget* pParent, weld::DialogController* pController,
                         const OUString& rUIXMLDescription, const OString& rID, bool bIsMobile)
    : m_pDialogController(pController)
    , m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription, bIsMobile))
    , m_xContainer(m_xBuilder->weld_container(rID))
{
}

void BuilderPage::Activate() {}

void BuilderPage::Deactivate() {}

BuilderPage::~BuilderPage() COVERITY_NOEXCEPT_FALSE {}

namespace weld
{
bool DialogController::runAsync(const std::shared_ptr<DialogController>& rController,
                                const std::function<void(sal_Int32)>& func)
{
    return rController->getDialog()->runAsync(rController, func);
}

DialogController::~DialogController() COVERITY_NOEXCEPT_FALSE {}

Dialog* GenericDialogController::getDialog() { return m_xDialog.get(); }

GenericDialogController::GenericDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                                 const OString& rDialogId, bool bMobile)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile, bMobile))
    , m_xDialog(m_xBuilder->weld_dialog(rDialogId))
{
}

GenericDialogController::~GenericDialogController() COVERITY_NOEXCEPT_FALSE {}

Dialog* MessageDialogController::getDialog() { return m_xDialog.get(); }

MessageDialogController::MessageDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                                 const OString& rDialogId,
                                                 const OString& rRelocateId)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xDialog(m_xBuilder->weld_message_dialog(rDialogId))
    , m_xContentArea(m_xDialog->weld_message_area())
{
    if (!rRelocateId.isEmpty())
    {
        m_xRelocate = m_xBuilder->weld_container(rRelocateId);
        m_xOrigParent = m_xRelocate->weld_parent();
        //fdo#75121, a bit tricky because the widgets we want to align with
        //don't actually exist in the ui description, they're implied
        m_xOrigParent->move(m_xRelocate.get(), m_xContentArea.get());
    }
}

MessageDialogController::~MessageDialogController()
{
    if (m_xRelocate)
    {
        m_xContentArea->move(m_xRelocate.get(), m_xOrigParent.get());
    }
}

AssistantController::AssistantController(weld::Widget* pParent, const OUString& rUIFile,
                                         const OString& rDialogId)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xAssistant(m_xBuilder->weld_assistant(rDialogId))
{
}

Dialog* AssistantController::getDialog() { return m_xAssistant.get(); }

AssistantController::~AssistantController() {}

void TriStateEnabled::ButtonToggled(weld::ToggleButton& rToggle)
{
    if (bTriStateEnabled)
    {
        switch (eState)
        {
            case TRISTATE_INDET:
                rToggle.set_state(TRISTATE_FALSE);
                break;
            case TRISTATE_TRUE:
                rToggle.set_state(TRISTATE_INDET);
                break;
            case TRISTATE_FALSE:
                rToggle.set_state(TRISTATE_TRUE);
                break;
        }
    }
    eState = rToggle.get_state();
}

void RemoveParentKeepChildren(weld::TreeView& rTreeView, weld::TreeIter& rParent)
{
    if (rTreeView.iter_has_child(rParent))
    {
        std::unique_ptr<weld::TreeIter> xNewParent(rTreeView.make_iterator(&rParent));
        if (!rTreeView.iter_parent(*xNewParent))
            xNewParent.reset();

        while (true)
        {
            std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator(&rParent));
            if (!rTreeView.iter_children(*xChild))
                break;
            rTreeView.move_subtree(*xChild, xNewParent.get(), -1);
        }
    }
    rTreeView.remove(rParent);
}

EntryFormatter::EntryFormatter(weld::FormattedSpinButton& rSpinButton)
    : m_rEntry(rSpinButton)
    , m_pSpinButton(&rSpinButton)
    , m_eOptions(Application::GetSettings().GetStyleSettings().GetSelectionOptions())
{
    Init();
}

EntryFormatter::EntryFormatter(weld::Entry& rEntry)
    : m_rEntry(rEntry)
    , m_pSpinButton(nullptr)
    , m_eOptions(Application::GetSettings().GetStyleSettings().GetSelectionOptions())
{
    Init();
}

EntryFormatter::~EntryFormatter()
{
    m_rEntry.connect_changed(Link<weld::Entry&, void>());
    m_rEntry.connect_focus_out(Link<weld::Widget&, void>());
    if (m_pSpinButton)
        m_pSpinButton->SetFormatter(nullptr);
}

void EntryFormatter::Init()
{
    m_rEntry.connect_changed(LINK(this, EntryFormatter, ModifyHdl));
    m_rEntry.connect_focus_out(LINK(this, EntryFormatter, FocusOutHdl));
    if (m_pSpinButton)
        m_pSpinButton->SetFormatter(this);
}

Selection EntryFormatter::GetEntrySelection() const
{
    int nStartPos, nEndPos;
    m_rEntry.get_selection_bounds(nStartPos, nEndPos);
    return Selection(nStartPos, nEndPos);
}

OUString EntryFormatter::GetEntryText() const { return m_rEntry.get_text(); }

void EntryFormatter::SetEntryText(const OUString& rText, const Selection& rSel)
{
    m_rEntry.set_text(rText);
    auto nMin = rSel.Min();
    auto nMax = rSel.Max();
    m_rEntry.select_region(nMin < 0 ? 0 : nMin, nMax == SELECTION_MAX ? -1 : nMax);
}

void EntryFormatter::SetEntryTextColor(const Color* pColor)
{
    m_rEntry.set_font_color(pColor ? *pColor : COL_AUTO);
}

void EntryFormatter::UpdateCurrentValue(double dCurrentValue)
{
    Formatter::UpdateCurrentValue(dCurrentValue);
    if (m_pSpinButton)
        m_pSpinButton->sync_value_from_formatter();
}

void EntryFormatter::ClearMinValue()
{
    Formatter::ClearMinValue();
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::SetMinValue(double dMin)
{
    Formatter::SetMinValue(dMin);
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::ClearMaxValue()
{
    Formatter::ClearMaxValue();
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::SetMaxValue(double dMin)
{
    Formatter::SetMaxValue(dMin);
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::SetSpinSize(double dStep)
{
    Formatter::SetSpinSize(dStep);
    if (m_pSpinButton)
        m_pSpinButton->sync_increments_from_formatter();
}

SelectionOptions EntryFormatter::GetEntrySelectionOptions() const { return m_eOptions; }

void EntryFormatter::FieldModified() { m_aModifyHdl.Call(m_rEntry); }

IMPL_LINK_NOARG(EntryFormatter, ModifyHdl, weld::Entry&, void)
{
    // This leads to FieldModified getting called at the end of Modify() and
    // FieldModified then calls any modification callback
    Modify();
}

IMPL_LINK_NOARG(EntryFormatter, FocusOutHdl, weld::Widget&, void)
{
    EntryLostFocus();
    m_aFocusOutHdl.Call(m_rEntry);
}

DoubleNumericFormatter::DoubleNumericFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
{
    ResetConformanceTester();
}

DoubleNumericFormatter::DoubleNumericFormatter(weld::FormattedSpinButton& rSpinButton)
    : EntryFormatter(rSpinButton)
{
    ResetConformanceTester();
}

DoubleNumericFormatter::~DoubleNumericFormatter() = default;

void DoubleNumericFormatter::FormatChanged(FORMAT_CHANGE_TYPE nWhat)
{
    ResetConformanceTester();
    EntryFormatter::FormatChanged(nWhat);
}

bool DoubleNumericFormatter::CheckText(const OUString& sText) const
{
    // We'd like to implement this using the NumberFormatter::IsNumberFormat, but unfortunately, this doesn't
    // recognize fragments of numbers (like, for instance "1e", which happens during entering e.g. "1e10")
    // Thus, the roundabout way via a regular expression
    return m_pNumberValidator->isValidNumericFragment(sText);
}

void DoubleNumericFormatter::ResetConformanceTester()
{
    // the thousands and the decimal separator are language dependent
    const SvNumberformat* pFormatEntry = GetOrCreateFormatter()->GetEntry(m_nFormatKey);

    sal_Unicode cSeparatorThousand = ',';
    sal_Unicode cSeparatorDecimal = '.';
    if (pFormatEntry)
    {
        LocaleDataWrapper aLocaleInfo(LanguageTag(pFormatEntry->GetLanguage()));

        OUString sSeparator = aLocaleInfo.getNumThousandSep();
        if (!sSeparator.isEmpty())
            cSeparatorThousand = sSeparator[0];

        sSeparator = aLocaleInfo.getNumDecimalSep();
        if (!sSeparator.isEmpty())
            cSeparatorDecimal = sSeparator[0];
    }

    m_pNumberValidator.reset(
        new validation::NumberValidator(cSeparatorThousand, cSeparatorDecimal));
}

LongCurrencyFormatter::LongCurrencyFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
    , m_bThousandSep(true)
{
    Init();
}

LongCurrencyFormatter::LongCurrencyFormatter(weld::FormattedSpinButton& rSpinButton)
    : EntryFormatter(rSpinButton)
    , m_bThousandSep(true)
{
    Init();
}

void LongCurrencyFormatter::Init()
{
    SetOutputHdl(LINK(this, LongCurrencyFormatter, FormatOutputHdl));
    SetInputHdl(LINK(this, LongCurrencyFormatter, ParseInputHdl));
}

void LongCurrencyFormatter::SetUseThousandSep(bool b)
{
    m_bThousandSep = b;
    ReFormat();
}

void LongCurrencyFormatter::SetCurrencySymbol(const OUString& rStr)
{
    m_aCurrencySymbol = rStr;
    ReFormat();
}

LongCurrencyFormatter::~LongCurrencyFormatter() = default;

TimeFormatter::TimeFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
    , m_eFormat(TimeFieldFormat::F_NONE)
    , m_eTimeFormat(TimeFormat::Hour24)
    , m_bDuration(false)
{
    Init();
}

TimeFormatter::TimeFormatter(weld::FormattedSpinButton& rSpinButton)
    : EntryFormatter(rSpinButton)
    , m_eFormat(TimeFieldFormat::F_NONE)
    , m_eTimeFormat(TimeFormat::Hour24)
    , m_bDuration(false)
{
    Init();
}

void TimeFormatter::Init()
{
    DisableRemainderFactor(); //so with hh::mm::ss, incrementing mm will not reset ss

    SetOutputHdl(LINK(this, TimeFormatter, FormatOutputHdl));
    SetInputHdl(LINK(this, TimeFormatter, ParseInputHdl));

    SetMin(tools::Time(0, 0));
    SetMax(tools::Time(23, 59, 59, 999999999));

    // so the spin size can depend on which zone the cursor is in
    get_widget().connect_cursor_position(LINK(this, TimeFormatter, CursorChangedHdl));
    // and set the initial spin size
    CursorChangedHdl(get_widget());
}

void TimeFormatter::SetExtFormat(ExtTimeFieldFormat eFormat)
{
    switch (eFormat)
    {
        case ExtTimeFieldFormat::Short24H:
        {
            m_eTimeFormat = TimeFormat::Hour24;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_NONE;
        }
        break;
        case ExtTimeFieldFormat::Long24H:
        {
            m_eTimeFormat = TimeFormat::Hour24;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_SEC;
        }
        break;
        case ExtTimeFieldFormat::Short12H:
        {
            m_eTimeFormat = TimeFormat::Hour12;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_NONE;
        }
        break;
        case ExtTimeFieldFormat::Long12H:
        {
            m_eTimeFormat = TimeFormat::Hour12;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_SEC;
        }
        break;
        case ExtTimeFieldFormat::ShortDuration:
        {
            m_bDuration = true;
            m_eFormat = TimeFieldFormat::F_NONE;
        }
        break;
        case ExtTimeFieldFormat::LongDuration:
        {
            m_bDuration = true;
            m_eFormat = TimeFieldFormat::F_SEC;
        }
        break;
    }

    ReFormat();
}

void TimeFormatter::SetDuration(bool bDuration)
{
    m_bDuration = bDuration;
    ReFormat();
}

void TimeFormatter::SetTimeFormat(TimeFieldFormat eTimeFormat)
{
    m_eFormat = eTimeFormat;
    ReFormat();
}

TimeFormatter::~TimeFormatter() = default;

DateFormatter::DateFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
    , m_eFormat(ExtDateFieldFormat::SystemShort)
{
    Init();
}

void DateFormatter::Init()
{
    SetOutputHdl(LINK(this, DateFormatter, FormatOutputHdl));
    SetInputHdl(LINK(this, DateFormatter, ParseInputHdl));

    SetMin(Date(1, 1, 1900));
    SetMax(Date(31, 12, 2200));
}

void DateFormatter::SetExtDateFormat(ExtDateFieldFormat eFormat)
{
    m_eFormat = eFormat;
    ReFormat();
}

DateFormatter::~DateFormatter() = default;

PatternFormatter::PatternFormatter(weld::Entry& rEntry)
    : m_rEntry(rEntry)
    , m_bStrictFormat(false)
    , m_bSameMask(true)
    , m_bReformat(false)
    , m_bInPattKeyInput(false)
{
    m_rEntry.connect_changed(LINK(this, PatternFormatter, ModifyHdl));
    m_rEntry.connect_focus_in(LINK(this, PatternFormatter, FocusInHdl));
    m_rEntry.connect_focus_out(LINK(this, PatternFormatter, FocusOutHdl));
    m_rEntry.connect_key_press(LINK(this, PatternFormatter, KeyInputHdl));
}

IMPL_LINK_NOARG(PatternFormatter, ModifyHdl, weld::Entry&, void) { Modify(); }

IMPL_LINK_NOARG(PatternFormatter, FocusOutHdl, weld::Widget&, void)
{
    EntryLostFocus();
    m_aFocusOutHdl.Call(m_rEntry);
}

IMPL_LINK_NOARG(PatternFormatter, FocusInHdl, weld::Widget&, void)
{
    EntryGainFocus();
    m_aFocusInHdl.Call(m_rEntry);
}

PatternFormatter::~PatternFormatter()
{
    m_rEntry.connect_changed(Link<weld::Entry&, void>());
    m_rEntry.connect_focus_out(Link<weld::Widget&, void>());
}

int GetMinimumEditHeight()
{
    // load this little .ui just to measure the height of an Entry
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(nullptr, "cui/ui/namedialog.ui"));
    std::unique_ptr<weld::Entry> xEntry(xBuilder->weld_entry("name_entry"));
    return xEntry->get_preferred_size().Height();
}

WidgetStatusListener::WidgetStatusListener(weld::Widget* widget, const OUString& aCommand)
    : mWidget(widget)
{
    css::uno::Reference<css::uno::XComponentContext> xContext
        = ::comphelper::getProcessComponentContext();
    css::uno::Reference<css::frame::XDesktop2> xDesktop = css::frame::Desktop::create(xContext);

    css::uno::Reference<css::frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame = xDesktop;

    mxFrame = xFrame;

    maCommandURL.Complete = aCommand;
    css::uno::Reference<css::util::XURLTransformer> xParser
        = css::util::URLTransformer::create(xContext);
    xParser->parseStrict(maCommandURL);
}

void WidgetStatusListener::startListening()
{
    if (mxDispatch.is())
        mxDispatch->removeStatusListener(this, maCommandURL);

    css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(mxFrame,
                                                                         css::uno::UNO_QUERY);
    if (!xDispatchProvider.is())
        return;

    mxDispatch = xDispatchProvider->queryDispatch(maCommandURL, "", 0);
    if (mxDispatch.is())
        mxDispatch->addStatusListener(this, maCommandURL);
}

void WidgetStatusListener::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    mWidget->set_sensitive(rEvent.IsEnabled);
}

void WidgetStatusListener::disposing(const css::lang::EventObject& /*Source*/)
{
    mxDispatch.clear();
}

void WidgetStatusListener::dispose()
{
    if (mxDispatch.is())
    {
        mxDispatch->removeStatusListener(this, maCommandURL);
        mxDispatch.clear();
    }
    mxFrame.clear();
    mWidget = nullptr;
}

ButtonPressRepeater::ButtonPressRepeater(weld::Button& rButton, const Link<Button&, void>& rLink,
                                         const Link<const CommandEvent&, void>& rContextLink)
    : m_rButton(rButton)
    , m_aLink(rLink)
    , m_aContextLink(rContextLink)
    , m_bModKey(false)
{
    // instead of connect_clicked because we want a button held down to
    // repeat the next/prev
    m_rButton.connect_mouse_press(LINK(this, ButtonPressRepeater, MousePressHdl));
    m_rButton.connect_mouse_release(LINK(this, ButtonPressRepeater, MouseReleaseHdl));

    m_aRepeat.SetInvokeHandler(LINK(this, ButtonPressRepeater, RepeatTimerHdl));
}

IMPL_LINK(ButtonPressRepeater, MousePressHdl, const MouseEvent&, rMouseEvent, bool)
{
    if (rMouseEvent.IsRight())
    {
        m_aContextLink.Call(
            CommandEvent(rMouseEvent.GetPosPixel(), CommandEventId::ContextMenu, true));
        return false;
    }
    m_bModKey = rMouseEvent.IsMod1();
    if (!m_rButton.get_sensitive())
        return false;
    auto self = weak_from_this();
    RepeatTimerHdl(nullptr);
    if (!self.lock())
        return false;
    if (!m_rButton.get_sensitive())
        return false;
    m_aRepeat.SetTimeout(MouseSettings::GetButtonStartRepeat());
    m_aRepeat.Start();
    return false;
}

IMPL_LINK_NOARG(ButtonPressRepeater, MouseReleaseHdl, const MouseEvent&, bool)
{
    m_bModKey = false;
    m_aRepeat.Stop();
    return false;
}

IMPL_LINK_NOARG(ButtonPressRepeater, RepeatTimerHdl, Timer*, void)
{
    m_aRepeat.SetTimeout(Application::GetSettings().GetMouseSettings().GetButtonRepeat());
    m_aLink.Call(m_rButton);
}

weld::Window* GetPopupParent(vcl::Window& rOutWin, tools::Rectangle& rRect)
{
    rRect.SetPos(rOutWin.OutputToScreenPixel(rRect.TopLeft()));
    rRect = FloatingWindow::ImplConvertToAbsPos(&rOutWin, rRect);

    vcl::Window* pWin = &rOutWin;
    while (!pWin->IsTopWindow())
        pWin = pWin->GetParent();

    rRect = FloatingWindow::ImplConvertToRelPos(pWin, rRect);
    rRect.SetPos(pWin->ScreenToOutputPixel(rRect.TopLeft()));

    return rOutWin.GetFrameWeld();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
