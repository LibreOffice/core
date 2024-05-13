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

#include <svtools/editbrowsebox.hxx>
#include <vcl/svapp.hxx>

namespace svt
{
    //= ComboBoxControl
    ComboBoxControl::ComboBoxControl(BrowserDataWin* pParent)
        : ControlBase(pParent, u"svt/ui/combocontrol.ui"_ustr, u"ComboControl"_ustr)
        , m_xWidget(m_xBuilder->weld_combo_box(u"combobox"_ustr))
    {
        InitControlBase(m_xWidget.get());
        m_xWidget->set_entry_width_chars(1); // so a smaller than default width can be used
        m_xWidget->connect_changed(LINK(this, ComboBoxControl, SelectHdl));
        m_xWidget->connect_key_press(LINK(this, ControlBase, KeyInputHdl));
        m_xWidget->connect_key_release(LINK(this, ControlBase, KeyReleaseHdl));
        m_xWidget->connect_focus_in(LINK(this, ControlBase, FocusInHdl));
        m_xWidget->connect_focus_out(LINK(this, ControlBase, FocusOutHdl));
        m_xWidget->connect_mouse_press(LINK(this, ControlBase, MousePressHdl));
        m_xWidget->connect_mouse_release(LINK(this, ControlBase, MouseReleaseHdl));
        m_xWidget->connect_mouse_move(LINK(this, ControlBase, MouseMoveHdl));
    }

    void ComboBoxControl::SetPointFont(const vcl::Font& rFont)
    {
        m_xWidget->set_entry_font(rFont);
    }

    void ComboBoxControl::dispose()
    {
        m_xWidget.reset();
        ControlBase::dispose();
    }

    IMPL_LINK_NOARG(ComboBoxControl, SelectHdl, weld::ComboBox&, void)
    {
        CallModifyHdls();
    }

    //= ComboBoxCellController
    ComboBoxCellController::ComboBoxCellController(ComboBoxControl* pWin)
                             :CellController(pWin)
    {
        static_cast<ComboBoxControl&>(GetWindow()).SetModifyHdl(LINK(this, ComboBoxCellController, ModifyHdl));
    }

    IMPL_LINK_NOARG(ComboBoxCellController, ModifyHdl, LinkParamNone*, void)
    {
        callModifyHdl();
    }

    bool ComboBoxCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        weld::ComboBox& rBox = GetComboBox();
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                int nStartPos, nEndPos;
                bool bNoSelection = rBox.get_entry_selection_bounds(nStartPos, nEndPos);
                return bNoSelection && nEndPos == rBox.get_active_text().getLength();
            }
            case KEY_HOME:
            case KEY_LEFT:
            {
                int nStartPos, nEndPos;
                bool bNoSelection = rBox.get_entry_selection_bounds(nStartPos, nEndPos);
                return bNoSelection && nStartPos == 0;
            }
            case KEY_UP:
            case KEY_DOWN:
                if (rBox.get_popup_shown())
                    return false;
                if (!rEvt.GetKeyCode().IsShift() &&
                     rEvt.GetKeyCode().IsMod1())
                    return false;
                // drop down the list box
                else if (rEvt.GetKeyCode().IsMod2() && rEvt.GetKeyCode().GetCode() == KEY_DOWN)
                    return false;
                [[fallthrough]];
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case KEY_RETURN:
                if (rBox.get_popup_shown())
                    return false;
                [[fallthrough]];
            default:
                return true;
        }
    }

    bool ComboBoxCellController::IsValueChangedFromSaved() const
    {
        return GetComboBox().get_value_changed_from_saved();
    }

    void ComboBoxCellController::SaveValue()
    {
        GetComboBox().save_value();
    }

    //= ListBoxControl
    ListBoxControl::ListBoxControl(BrowserDataWin* pParent)
        : ControlBase(pParent, u"svt/ui/listcontrol.ui"_ustr, u"ListControl"_ustr)
        , m_xWidget(m_xBuilder->weld_combo_box(u"listbox"_ustr))
    {
        InitControlBase(m_xWidget.get());
        m_xWidget->set_size_request(42, -1); // so a later narrow size request can stick
        m_xWidget->connect_changed(LINK(this, ListBoxControl, SelectHdl));
        m_xWidget->connect_key_press(LINK(this, ControlBase, KeyInputHdl));
        m_xWidget->connect_key_release(LINK(this, ControlBase, KeyReleaseHdl));
        m_xWidget->connect_focus_in(LINK(this, ControlBase, FocusInHdl));
        m_xWidget->connect_focus_out(LINK(this, ControlBase, FocusOutHdl));
        m_xWidget->connect_mouse_press(LINK(this, ControlBase, MousePressHdl));
        m_xWidget->connect_mouse_release(LINK(this, ControlBase, MouseReleaseHdl));
        m_xWidget->connect_mouse_move(LINK(this, ControlBase, MouseMoveHdl));
    }

    void ListBoxControl::SetPointFont(const vcl::Font& rFont)
    {
        m_xWidget->set_font(rFont);
    }

    void ListBoxControl::dispose()
    {
        m_xWidget.reset();
        ControlBase::dispose();
    }

    IMPL_LINK_NOARG(ListBoxControl, SelectHdl, weld::ComboBox&, void)
    {
        CallModifyHdls();
    }

    //= ListBoxCellController
    ListBoxCellController::ListBoxCellController(ListBoxControl* pWin)
                             :CellController(pWin)
    {
        static_cast<ListBoxControl&>(GetWindow()).SetModifyHdl(LINK(this, ListBoxCellController, ListBoxSelectHdl));
    }

    bool ListBoxCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        const weld::ComboBox& rBox = GetListBox();
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_UP:
            case KEY_DOWN:
                if (!rEvt.GetKeyCode().IsShift() &&
                     rEvt.GetKeyCode().IsMod1())
                    return false;
                // drop down the list box
                else
                    if (rEvt.GetKeyCode().IsMod2() && rEvt.GetKeyCode().GetCode() == KEY_DOWN)
                        return false;
                [[fallthrough]];
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
                if (rBox.get_popup_shown())
                    return false;
                [[fallthrough]];
            default:
                return true;
        }
    }

    bool ListBoxCellController::IsValueChangedFromSaved() const
    {
        return GetListBox().get_value_changed_from_saved();
    }

    void ListBoxCellController::SaveValue()
    {
        GetListBox().save_value();
    }

    IMPL_LINK_NOARG(ListBoxCellController, ListBoxSelectHdl, LinkParamNone*, void)
    {
        callModifyHdl();
    }

    //= CheckBoxControl
    CheckBoxControl::CheckBoxControl(BrowserDataWin* pParent)
        : ControlBase(pParent, u"svt/ui/checkboxcontrol.ui"_ustr, u"CheckBoxControl"_ustr)
        , m_xBox(m_xBuilder->weld_check_button(u"checkbox"_ustr))
    {
        m_aModeState.bTriStateEnabled = true;
        InitControlBase(m_xBox.get());
        m_xBox->connect_key_press(LINK(this, ControlBase, KeyInputHdl));
        m_xBox->connect_key_release(LINK(this, ControlBase, KeyReleaseHdl));
        m_xBox->connect_focus_in(LINK(this, ControlBase, FocusInHdl));
        m_xBox->connect_focus_out(LINK(this, ControlBase, FocusOutHdl));
        m_xBox->connect_mouse_press(LINK(this, ControlBase, MousePressHdl));
        m_xBox->connect_mouse_release(LINK(this, ControlBase, MouseReleaseHdl));
        m_xBox->connect_mouse_move(LINK(this, ControlBase, MouseMoveHdl));
        m_xBox->connect_toggled(LINK(this, CheckBoxControl, OnToggle));
    }

    void CheckBoxControl::SetPointFont(const vcl::Font& /*rFont*/)
    {
    }

    void CheckBoxControl::EnableTriState( bool bTriState )
    {
        if (m_aModeState.bTriStateEnabled != bTriState)
        {
            m_aModeState.bTriStateEnabled = bTriState;

            if (!m_aModeState.bTriStateEnabled && GetState() == TRISTATE_INDET)
                SetState(TRISTATE_FALSE);
        }
    }

    void CheckBoxControl::SetState(TriState eState)
    {
        if (!m_aModeState.bTriStateEnabled && (eState == TRISTATE_INDET))
            eState = TRISTATE_FALSE;
        m_aModeState.eState = eState;
        m_xBox->set_state(eState);
    }

    CheckBoxControl::~CheckBoxControl()
    {
        disposeOnce();
    }

    void CheckBoxControl::dispose()
    {
        m_xBox.reset();
        ControlBase::dispose();
    }

    void CheckBoxControl::Clicked()
    {
        // if tristate is enabled, m_aModeState will take care of setting the
        // next state in the sequence via TriStateEnabled::ButtonToggled
        if (!m_aModeState.bTriStateEnabled)
            m_xBox->set_active(!m_xBox->get_active());
        OnToggle(*m_xBox);
    }

    IMPL_LINK_NOARG(CheckBoxControl, OnToggle, weld::Toggleable&, void)
    {
        m_aModeState.ButtonToggled(*m_xBox);
        m_aToggleLink.Call(*m_xBox);
        CallModifyHdls();
    }

    //= CheckBoxCellController
    CheckBoxCellController::CheckBoxCellController(CheckBoxControl* pWin)
        : CellController(pWin)
    {
        static_cast<CheckBoxControl &>(GetWindow()).SetModifyHdl( LINK(this, CheckBoxCellController, ModifyHdl) );
    }

    void CheckBoxCellController::ActivatingMouseEvent(const BrowserMouseEvent& rEvt, bool /*bUp*/)
    {
        CheckBoxControl& rControl = static_cast<CheckBoxControl&>(GetWindow());
        rControl.GrabFocus();

        // we have to adjust the position of the event relative to the controller's window
        Point aPos = rEvt.GetPosPixel() - rEvt.GetRect().TopLeft();

        Size aControlSize = rControl.GetSizePixel();
        Size aBoxSize = rControl.GetBox().get_preferred_size();
        tools::Rectangle aHotRect(Point((aControlSize.Width() - aBoxSize.Width()) / 2,
                                        (aControlSize.Height() - aBoxSize.Height()) / 2),
                                  aBoxSize);

        // we want the initial mouse event to act as if it was performed on the checkbox
        if (aHotRect.Contains(aPos))
            rControl.Clicked();
    }

    weld::CheckButton& CheckBoxCellController::GetCheckBox() const
    {
        return static_cast<CheckBoxControl &>(GetWindow()).GetBox();
    }

    bool CheckBoxCellController::IsValueChangedFromSaved() const
    {
        return GetCheckBox().get_state_changed_from_saved();
    }

    void CheckBoxCellController::SaveValue()
    {
        GetCheckBox().save_state();
    }

    IMPL_LINK_NOARG(CheckBoxCellController, ModifyHdl, LinkParamNone*, void)
    {
        callModifyHdl();
    }

    //= MultiLineEditImplementation
    OUString MultiLineEditImplementation::GetText(LineEnd eSeparator) const
    {
        weld::TextView& rEntry = m_rEdit.get_widget();
        return convertLineEnd(rEntry.get_text(), eSeparator);
    }

    OUString MultiLineEditImplementation::GetSelected(LineEnd eSeparator) const
    {
        int nStartPos, nEndPos;
        weld::TextView& rEntry = m_rEdit.get_widget();
        rEntry.get_selection_bounds(nStartPos, nEndPos);
        return convertLineEnd(rEntry.get_text().copy(nStartPos, nEndPos - nStartPos), eSeparator);
    }

    IMPL_LINK_NOARG(MultiLineEditImplementation, ModifyHdl, weld::TextView&, void)
    {
        CallModifyHdls();
    }

    EditCellController::EditCellController( IEditImplementation* _pImplementation )
        :CellController( &_pImplementation->GetControl() )
        ,m_pEditImplementation( _pImplementation )
        ,m_bOwnImplementation( false )
    {
        m_pEditImplementation->SetModifyHdl( LINK(this, EditCellController, ModifyHdl) );
    }

    IMPL_LINK_NOARG(EntryImplementation, ModifyHdl, weld::Entry&, void)
    {
        CallModifyHdls();
    }

    ControlBase::ControlBase(BrowserDataWin* pParent, const OUString& rUIXMLDescription, const OUString& rID)
        : InterimItemWindow(pParent, rUIXMLDescription, rID)
    {
    }

    void ControlBase::SetEditableReadOnly(bool /*bReadOnly*/)
    {
        // expected to be overridden for Entry, TextView or the editable entry part of a ComboBox
    }

    EditControlBase::EditControlBase(BrowserDataWin* pParent)
        : ControlBase(pParent, u"svt/ui/thineditcontrol.ui"_ustr, u"EditControl"_ustr) // *thin*editcontrol has no frame/border
        , m_pEntry(nullptr) // inheritors are expected to call InitEditControlBase
    {
    }

    void EditControlBase::InitEditControlBase(weld::Entry* pEntry)
    {
        InitControlBase(pEntry);
        m_pEntry = pEntry;
        m_pEntry->show();
        m_pEntry->set_width_chars(1); // so a smaller than default width can be used
        connect_focus_in(LINK(this, ControlBase, FocusInHdl));   // need to chain with pattern handler
        connect_focus_out(LINK(this, ControlBase, FocusOutHdl)); // need to chain with pattern handler
        connect_key_press(LINK(this, ControlBase, KeyInputHdl)); // need to chain with pattern handler
        m_pEntry->connect_key_release(LINK(this, ControlBase, KeyReleaseHdl));
        m_pEntry->connect_mouse_press(LINK(this, ControlBase, MousePressHdl));
        m_pEntry->connect_mouse_release(LINK(this, ControlBase, MouseReleaseHdl));
        m_pEntry->connect_mouse_move(LINK(this, ControlBase, MouseMoveHdl));
    }

    bool ControlBase::ProcessKey(const KeyEvent& rKEvt)
    {
        return static_cast<BrowserDataWin*>(GetParent())->GetParent()->ProcessKey(rKEvt);
    }

    IMPL_LINK(ControlBase, KeyInputHdl, const KeyEvent&, rKEvt, bool)
    {
        m_aKeyInputHdl.Call(rKEvt);
        return ProcessKey(rKEvt);
    }

    IMPL_LINK(ControlBase, KeyReleaseHdl, const KeyEvent&, rKEvt, bool)
    {
        m_aKeyReleaseHdl.Call(rKEvt);
        return false;
    }

    IMPL_LINK_NOARG(ControlBase, FocusInHdl, weld::Widget&, void)
    {
        m_aFocusInHdl.Call(nullptr);
        static_cast<BrowserDataWin*>(GetParent())->GetParent()->ChildFocusIn();
    }

    IMPL_LINK_NOARG(ControlBase, FocusOutHdl, weld::Widget&, void)
    {
        m_aFocusOutHdl.Call(nullptr);
        static_cast<BrowserDataWin*>(GetParent())->GetParent()->ChildFocusOut();
    }

    IMPL_LINK(ControlBase, MousePressHdl, const MouseEvent&, rEvent, bool)
    {
        m_aMousePressHdl.Call(rEvent);
        return false;
    }

    IMPL_LINK(ControlBase, MouseReleaseHdl, const MouseEvent&, rEvent, bool)
    {
        m_aMouseReleaseHdl.Call(rEvent);
        return false;
    }

    IMPL_LINK(ControlBase, MouseMoveHdl, const MouseEvent&, rEvent, bool)
    {
        m_aMouseMoveHdl.Call(rEvent);
        return false;
    }

    void EditControlBase::dispose()
    {
        m_pEntry = nullptr;
        ControlBase::dispose();
    }

    EditControl::EditControl(BrowserDataWin* pParent)
        : EditControlBase(pParent)
        , m_xWidget(m_xBuilder->weld_entry(u"entry"_ustr))
    {
        InitEditControlBase(m_xWidget.get());
    }

    void EditControl::dispose()
    {
        m_xWidget.reset();
        EditControlBase::dispose();
    }

    FormattedControlBase::FormattedControlBase(BrowserDataWin* pParent, bool bSpinVariant)
        : EditControlBase(pParent)
        , m_bSpinVariant(bSpinVariant)
        , m_xEntry(m_xBuilder->weld_entry(u"entry"_ustr))
        , m_xSpinButton(m_xBuilder->weld_formatted_spin_button(u"spinbutton"_ustr))
    {
    }

    void FormattedControlBase::InitFormattedControlBase()
    {
        InitEditControlBase(m_bSpinVariant ? m_xSpinButton.get() : m_xEntry.get());
    }

    void FormattedControlBase::connect_changed(const Link<weld::Entry&, void>& rLink)
    {
        get_formatter().connect_changed(rLink);
    }

    void FormattedControlBase::connect_focus_in(const Link<weld::Widget&, void>& rLink)
    {
        get_widget().connect_focus_in(rLink);
    }

    void FormattedControlBase::connect_focus_out(const Link<weld::Widget&, void>& rLink)
    {
        get_formatter().connect_focus_out(rLink);
    }

    void FormattedControlBase::connect_key_press(const Link<const KeyEvent&, bool>& rLink)
    {
        get_widget().connect_key_press(rLink);
    }

    weld::EntryFormatter& FormattedControlBase::get_formatter()
    {
        return *m_xEntryFormatter;
    }

    void FormattedControlBase::dispose()
    {
        m_xEntryFormatter.reset();
        m_xSpinButton.reset();
        m_xEntry.reset();
        EditControlBase::dispose();
    }

    FormattedControl::FormattedControl(BrowserDataWin* pParent, bool bSpinVariant)
        : FormattedControlBase(pParent, bSpinVariant)
    {
        if (bSpinVariant)
            m_xEntryFormatter.reset(new weld::EntryFormatter(*m_xSpinButton));
        else
            m_xEntryFormatter.reset(new weld::EntryFormatter(*m_xEntry));
        InitFormattedControlBase();
    }

    DoubleNumericControl::DoubleNumericControl(BrowserDataWin* pParent, bool bSpinVariant)
        : FormattedControlBase(pParent, bSpinVariant)
    {
        if (bSpinVariant)
            m_xEntryFormatter.reset(new weld::DoubleNumericFormatter(*m_xSpinButton));
        else
            m_xEntryFormatter.reset(new weld::DoubleNumericFormatter(*m_xEntry));
        InitFormattedControlBase();
    }

    LongCurrencyControl::LongCurrencyControl(BrowserDataWin* pParent, bool bSpinVariant)
        : FormattedControlBase(pParent, bSpinVariant)
    {
        if (bSpinVariant)
            m_xEntryFormatter.reset(new weld::LongCurrencyFormatter(*m_xSpinButton));
        else
            m_xEntryFormatter.reset(new weld::LongCurrencyFormatter(*m_xEntry));
        InitFormattedControlBase();
    }

    TimeControl::TimeControl(BrowserDataWin* pParent, bool bSpinVariant)
        : FormattedControlBase(pParent, bSpinVariant)
    {
        if (bSpinVariant)
            m_xEntryFormatter.reset(new weld::TimeFormatter(*m_xSpinButton));
        else
            m_xEntryFormatter.reset(new weld::TimeFormatter(*m_xEntry));
        InitFormattedControlBase();
    }

    DateControl::DateControl(BrowserDataWin* pParent, bool bDropDown)
        : FormattedControlBase(pParent, false)
        , m_xMenuButton(m_xBuilder->weld_menu_button(u"button"_ustr))
        , m_xCalendarBuilder(Application::CreateBuilder(m_xMenuButton.get(), u"svt/ui/datewindow.ui"_ustr))
        , m_xTopLevel(m_xCalendarBuilder->weld_widget(u"date_popup_window"_ustr))
        , m_xCalendar(m_xCalendarBuilder->weld_calendar(u"date_picker"_ustr))
        , m_xExtras(m_xCalendarBuilder->weld_widget(u"extras"_ustr))
        , m_xTodayBtn(m_xCalendarBuilder->weld_button(u"today"_ustr))
        , m_xNoneBtn(m_xCalendarBuilder->weld_button(u"none"_ustr))
    {
        m_xEntryFormatter.reset(new weld::DateFormatter(*m_xEntry));
        InitFormattedControlBase();

        m_xMenuButton->set_popover(m_xTopLevel.get());
        m_xMenuButton->set_visible(bDropDown);
        m_xMenuButton->connect_toggled(LINK(this, DateControl, ToggleHdl));

        m_xExtras->show();

        m_xTodayBtn->connect_clicked(LINK(this, DateControl, ImplClickHdl));
        m_xNoneBtn->connect_clicked(LINK(this, DateControl, ImplClickHdl));

        m_xCalendar->connect_activated(LINK(this, DateControl, ActivateHdl));
    }

    IMPL_LINK(DateControl, ImplClickHdl, weld::Button&, rBtn, void)
    {
        m_xMenuButton->set_active(false);
        get_widget().grab_focus();

        if (&rBtn == m_xTodayBtn.get())
        {
            Date aToday(Date::SYSTEM);
            SetDate(aToday);
        }
        else if (&rBtn == m_xNoneBtn.get())
        {
            get_widget().set_text(OUString());
        }
    }

    IMPL_LINK(DateControl, ToggleHdl, weld::Toggleable&, rButton, void)
    {
        if (rButton.get_active())
            m_xCalendar->set_date(static_cast<weld::DateFormatter&>(get_formatter()).GetDate());
    }

    IMPL_LINK_NOARG(DateControl, ActivateHdl, weld::Calendar&, void)
    {
        if (m_xMenuButton->get_active())
            m_xMenuButton->set_active(false);
        static_cast<weld::DateFormatter&>(get_formatter()).SetDate(m_xCalendar->get_date());
    }

    void DateControl::SetDate(const Date& rDate)
    {
        static_cast<weld::DateFormatter&>(get_formatter()).SetDate(rDate);
        m_xCalendar->set_date(rDate);
    }

    void DateControl::dispose()
    {
        m_xTodayBtn.reset();
        m_xNoneBtn.reset();
        m_xExtras.reset();
        m_xCalendar.reset();
        m_xTopLevel.reset();
        m_xCalendarBuilder.reset();
        m_xMenuButton.reset();
        FormattedControlBase::dispose();
    }

    PatternControl::PatternControl(BrowserDataWin* pParent)
        : EditControlBase(pParent)
        , m_xWidget(m_xBuilder->weld_entry(u"entry"_ustr))
    {
        m_xEntryFormatter.reset(new weld::PatternFormatter(*m_xWidget));
        InitEditControlBase(m_xWidget.get());
    }

    void PatternControl::connect_changed(const Link<weld::Entry&, void>& rLink)
    {
        m_xEntryFormatter->connect_changed(rLink);
    }

    void PatternControl::connect_focus_in(const Link<weld::Widget&, void>& rLink)
    {
        m_xEntryFormatter->connect_focus_in(rLink);
    }

    void PatternControl::connect_focus_out(const Link<weld::Widget&, void>& rLink)
    {
        m_xEntryFormatter->connect_focus_out(rLink);
    }

    void PatternControl::connect_key_press(const Link<const KeyEvent&, bool>& rLink)
    {
        m_xEntryFormatter->connect_key_press(rLink);
    }

    void PatternControl::dispose()
    {
        m_xEntryFormatter.reset();
        m_xWidget.reset();
        EditControlBase::dispose();
    }

    EditCellController::EditCellController(EditControlBase* pEdit)
        : CellController(pEdit)
        , m_pEditImplementation(new EntryImplementation(*pEdit))
        , m_bOwnImplementation(true)
    {
        m_pEditImplementation->SetModifyHdl( LINK(this, EditCellController, ModifyHdl) );
    }

    EditCellController::~EditCellController( )
    {
        if ( m_bOwnImplementation )
            delete m_pEditImplementation;
    }

    void EditCellController::SaveValue()
    {
        m_pEditImplementation->SaveValue();
    }

    bool EditCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        bool bResult;
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = m_pEditImplementation->GetSelection();
                bResult = !aSel && aSel.Max() == m_pEditImplementation->GetText( LINEEND_LF ).getLength();
                break;
            }
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = m_pEditImplementation->GetSelection();
                bResult = !aSel && aSel.Min() == 0;
                break;
            }
            case KEY_DOWN:
            {
                bResult = !m_pEditImplementation->CanDown();
                break;
            }
            case KEY_UP:
            {
                bResult = !m_pEditImplementation->CanUp();
                break;
            }
            default:
                bResult = true;
        }
        return bResult;
    }

    bool EditCellController::IsValueChangedFromSaved() const
    {
        return m_pEditImplementation->IsValueChangedFromSaved();
    }

    IMPL_LINK_NOARG(EditCellController, ModifyHdl, LinkParamNone*, void)
    {
        callModifyHdl();
    }

    //= FormattedFieldCellController
    FormattedFieldCellController::FormattedFieldCellController( FormattedControlBase* _pFormatted )
        : EditCellController(_pFormatted)
    {
    }

    void FormattedFieldCellController::CommitModifications()
    {
        static_cast<FormattedControl&>(GetWindow()).get_formatter().Commit();
    }

    MultiLineTextCell::MultiLineTextCell(BrowserDataWin* pParent)
        : ControlBase(pParent, u"svt/ui/textviewcontrol.ui"_ustr, u"TextViewControl"_ustr)
        , m_xWidget(m_xBuilder->weld_text_view(u"textview"_ustr))
    {
        InitControlBase(m_xWidget.get());
        m_xWidget->connect_key_press(LINK(this, ControlBase, KeyInputHdl));
        m_xWidget->connect_key_release(LINK(this, ControlBase, KeyReleaseHdl));
        m_xWidget->connect_focus_in(LINK(this, ControlBase, FocusInHdl));
        m_xWidget->connect_focus_out(LINK(this, ControlBase, FocusOutHdl));
        m_xWidget->connect_mouse_press(LINK(this, ControlBase, MousePressHdl));
        m_xWidget->connect_mouse_release(LINK(this, ControlBase, MouseReleaseHdl));
        m_xWidget->connect_mouse_move(LINK(this, ControlBase, MouseMoveHdl));
        // so any the natural size doesn't have an effect
        m_xWidget->set_size_request(1, 1);
    }

    void MultiLineTextCell::GetFocus()
    {
        if (m_xWidget)
            m_xWidget->select_region(-1, 0);
        ControlBase::GetFocus();
    }

    void MultiLineTextCell::dispose()
    {
        m_xWidget.reset();
        ControlBase::dispose();
    }

    bool MultiLineTextCell::ProcessKey(const KeyEvent& rKEvt)
    {
        bool bSendToDataWindow = true;

        sal_uInt16 nCode  = rKEvt.GetKeyCode().GetCode();
        bool bShift = rKEvt.GetKeyCode().IsShift();
        bool bCtrl = rKEvt.GetKeyCode().IsMod1();
        bool bAlt =  rKEvt.GetKeyCode().IsMod2();

        if (!bAlt && !bCtrl && !bShift)
        {
            switch (nCode)
            {
                case KEY_DOWN:
                    bSendToDataWindow = !m_xWidget->can_move_cursor_with_down();
                    break;
                case KEY_UP:
                    bSendToDataWindow = !m_xWidget->can_move_cursor_with_up();
                    break;
            }
        }

        if (bSendToDataWindow)
            return ControlBase::ProcessKey(rKEvt);
        return false;
    }
}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
