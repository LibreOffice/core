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
#include <vcl/button.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <vcl/virdev.hxx>

namespace svt
{
    //= ComboBoxControl
    ComboBoxControl::ComboBoxControl(BrowserDataWin* pParent)
        : ControlBase(pParent, "svt/ui/combocontrol.ui", "ComboControl")
        , m_xWidget(m_xBuilder->weld_combo_box("combobox"))
    {
        InitControlBase(m_xWidget.get());
        m_xWidget->set_entry_width_chars(1); // so a smaller than default width can be used
    }

    void ComboBoxControl::dispose()
    {
        m_xWidget.reset();
        ControlBase::dispose();
    }

    //= ComboBoxCellController
    ComboBoxCellController::ComboBoxCellController(ComboBoxControl* pWin)
                             :CellController(pWin)
    {
        GetComboBox().connect_changed(LINK(this, ComboBoxCellController, ModifyHdl));
    }

    IMPL_LINK_NOARG(ComboBoxCellController, ModifyHdl, weld::ComboBox&, void)
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
        : ControlBase(pParent, "svt/ui/listcontrol.ui", "ListControl")
        , m_xWidget(m_xBuilder->weld_combo_box("listbox"))
    {
        InitControlBase(m_xWidget.get());
        m_xWidget->set_size_request(42, -1); // so a later narrow size request can stick
    }

    void ListBoxControl::dispose()
    {
        m_xWidget.reset();
        ControlBase::dispose();
    }

    //= ListBoxCellController
    ListBoxCellController::ListBoxCellController(ListBoxControl* pWin)
                             :CellController(pWin)
    {
        GetListBox().connect_changed(LINK(this, ListBoxCellController, ListBoxSelectHdl));
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

    IMPL_LINK_NOARG(ListBoxCellController, ListBoxSelectHdl, weld::ComboBox&, void)
    {
        callModifyHdl();
    }

    //= CheckBoxControl
    CheckBoxControl::CheckBoxControl(vcl::Window* pParent)
                   :Control(pParent, 0)
    {
        const Wallpaper& rParentBackground = pParent->GetBackground();
        if ( (pParent->GetStyle() & WB_CLIPCHILDREN) || rParentBackground.IsFixed() )
            SetBackground( rParentBackground );
        else
        {
            SetPaintTransparent( true );
            SetBackground();
        }

        EnableChildTransparentMode();

        pBox = VclPtr<CheckBox>::Create(this,WB_CENTER|WB_VCENTER);
        pBox->EnableTriState( true );
        pBox->SetLegacyNoTextAlign( true );
        pBox->EnableChildTransparentMode();
        pBox->SetPaintTransparent( true );
        pBox->SetClickHdl( LINK( this, CheckBoxControl, OnClick ) );
        pBox->Show();
    }

    CheckBoxControl::~CheckBoxControl()
    {
        disposeOnce();
    }

    void CheckBoxControl::dispose()
    {
        pBox.disposeAndClear();
        Control::dispose();
    }


    IMPL_LINK_NOARG(CheckBoxControl, OnClick, Button*, void)
    {
        m_aClickLink.Call(pBox);
        m_aModifyLink.Call(nullptr);
    }


    void CheckBoxControl::Resize()
    {
        Control::Resize();
        pBox->SetPosSizePixel(Point(0,0),GetSizePixel());
    }


    void CheckBoxControl::DataChanged( const DataChangedEvent& _rEvent )
    {
        if ( _rEvent.GetType() == DataChangedEventType::SETTINGS )
            pBox->SetSettings( GetSettings() );
    }


    void CheckBoxControl::StateChanged( StateChangedType nStateChange )
    {
        Control::StateChanged(nStateChange);
        if ( nStateChange == StateChangedType::Zoom )
            pBox->SetZoom(GetZoom());
    }

    void CheckBoxControl::Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags )
    {
        pBox->Draw(pDev, rPos, nFlags);
    }

    void CheckBoxControl::GetFocus()
    {
        if (pBox)
            pBox->GrabFocus();
    }


    void CheckBoxControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rClientRect)
    {
        Control::Paint(rRenderContext, rClientRect);
        if (HasFocus())
            ShowFocus(tools::Rectangle());
    }


    bool CheckBoxControl::PreNotify(NotifyEvent& rEvt)
    {
        switch (rEvt.GetType())
        {
            case MouseNotifyEvent::GETFOCUS:
                ShowFocus(tools::Rectangle());
                break;
            case MouseNotifyEvent::LOSEFOCUS:
                HideFocus();
                break;
            default:
                break;
        }
        return Control::PreNotify(rEvt);
    }


    //= CheckBoxCellController


    CheckBoxCellController::CheckBoxCellController(CheckBoxControl* pWin)
        : CellController(pWin)
    {
        static_cast<CheckBoxControl &>(GetWindow()).SetModifyHdl( LINK(this, CheckBoxCellController, ModifyHdl) );
    }

    bool CheckBoxCellController::WantMouseEvent() const
    {
        return true;
    }


    CheckBox& CheckBoxCellController::GetCheckBox() const
    {
        return static_cast<CheckBoxControl &>(GetWindow()).GetBox();
    }

    bool CheckBoxCellController::IsValueChangedFromSaved() const
    {
        return GetCheckBox().IsValueChangedFromSaved();
    }

    void CheckBoxCellController::SaveValue()
    {
        GetCheckBox().SaveValue();
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

    IMPL_LINK_NOARG(EditImplementation, ModifyHdl, Edit&, void)
    {
        CallModifyHdls();
    }

    //= EditCellController
    EditCellController::EditCellController( Edit* _pEdit )
        :CellController( _pEdit )
        ,m_pEditImplementation( new EditImplementation( *_pEdit ) )
        ,m_bOwnImplementation( true )
    {
        m_pEditImplementation->SetModifyHdl( LINK(this, EditCellController, ModifyHdl) );
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

    ControlBase::ControlBase(BrowserDataWin* pParent, const OUString& rUIXMLDescription, const OString& rID)
        : InterimItemWindow(pParent, rUIXMLDescription, rID)
    {
    }

    bool ControlBase::ControlHasFocus() const
    {
        if (!m_pWidget)
            return false;
        return m_pWidget->has_focus();
    }

    void ControlBase::Draw(OutputDevice* pDevice, const Point& rPos, DrawFlags /*nFlags*/)
    {
        if (!m_pWidget)
            return;
        m_pWidget->draw(*pDevice, tools::Rectangle(rPos, GetSizePixel()));
    }

    void ControlBase::dispose()
    {
        m_pWidget = nullptr;
        InterimItemWindow::dispose();
    }

    void ControlBase::GetFocus()
    {
        if (m_pWidget)
            m_pWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    void ControlBase::InitControlBase(weld::Widget* pWidget)
    {
        m_pWidget = pWidget;
    }

    EditControlBase::EditControlBase(BrowserDataWin* pParent)
        : ControlBase(pParent, "svt/ui/thineditcontrol.ui", "EditControl") // *thin*editcontrol has no frame/border
        , m_pEntry(nullptr) // inheritors are expected to call InitEditControlBase
    {
    }

    void EditControlBase::InitEditControlBase(weld::Entry* pEntry)
    {
        InitControlBase(pEntry);
        m_pEntry = pEntry;
        m_pEntry->show();
        m_pEntry->set_width_chars(1); // so a smaller than default width can be used
        m_pEntry->connect_key_press(LINK(this, ControlBase, KeyInputHdl));
    }

    bool ControlBase::ProcessKey(const KeyEvent& rKEvt)
    {
        return static_cast<BrowserDataWin*>(GetParent())->GetParent()->ProcessKey(rKEvt);
    }

    IMPL_LINK(ControlBase, KeyInputHdl, const KeyEvent&, rKEvt, bool)
    {
        return ProcessKey(rKEvt);
    }

    void EditControlBase::dispose()
    {
        m_pEntry = nullptr;
        ControlBase::dispose();
    }

    EditControl::EditControl(BrowserDataWin* pParent)
        : EditControlBase(pParent)
        , m_xWidget(m_xBuilder->weld_entry("entry"))
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
        , m_xEntry(m_xBuilder->weld_entry("entry"))
        , m_xSpinButton(m_xBuilder->weld_formatted_spin_button("spinbutton"))
    {
    }

    void FormattedControlBase::InitFormattedControlBase()
    {
        if (m_bSpinVariant)
            m_xSpinButton->SetFormatter(m_xEntryFormatter.release());
        InitEditControlBase(m_bSpinVariant ? m_xSpinButton.get() : m_xEntry.get());
    }

    void FormattedControlBase::connect_changed(const Link<weld::Entry&, void>& rLink)
    {
        get_formatter().connect_changed(rLink);
    }

    weld::EntryFormatter& FormattedControlBase::get_formatter()
    {
        if (m_bSpinVariant)
            return static_cast<weld::EntryFormatter&>(m_xSpinButton->GetFormatter());
        else
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
            m_xEntryFormatter.reset(new weld::DoubleNumericEntry(*m_xSpinButton));
        else
            m_xEntryFormatter.reset(new weld::DoubleNumericEntry(*m_xEntry));
        InitFormattedControlBase();
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
            DELETEZ( m_pEditImplementation );
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

    //= SpinCellController
    SpinCellController::SpinCellController(SpinField* pWin)
                         :CellController(pWin)
    {
        GetSpinWindow().SetModifyHdl( LINK(this, SpinCellController, ModifyHdl) );
    }

    const SpinField& SpinCellController::GetSpinWindow() const
    {
        return static_cast<const SpinField &>(GetWindow());
    }

    SpinField& SpinCellController::GetSpinWindow()
    {
        return static_cast<SpinField &>(GetWindow());
    }

    void SpinCellController::SaveValue()
    {
        GetSpinWindow().SaveValue();
    }

    bool SpinCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        bool bResult;
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = GetSpinWindow().GetSelection();
                bResult = !aSel && aSel.Max() == GetSpinWindow().GetText().getLength();
            }   break;
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = GetSpinWindow().GetSelection();
                bResult = !aSel && aSel.Min() == 0;
            }   break;
            default:
                bResult = true;
        }
        return bResult;
    }

    bool SpinCellController::IsValueChangedFromSaved() const
    {
        return GetSpinWindow().IsValueChangedFromSaved();
    }

    IMPL_LINK_NOARG(SpinCellController, ModifyHdl, Edit&, void)
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
        : ControlBase(pParent, "svt/ui/textviewcontrol.ui", "TextViewControl")
        , m_xWidget(m_xBuilder->weld_text_view("textview"))
    {
        InitControlBase(m_xWidget.get());
        m_xWidget->connect_key_press(LINK(this, ControlBase, KeyInputHdl));
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
