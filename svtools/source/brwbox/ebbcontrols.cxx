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

#include <svtools/editbrowsebox.hxx>
#include <vcl/decoview.hxx>
#include <svtools/fmtfield.hxx>
#include <vcl/xtextedt.hxx>

#include <algorithm>


namespace svt
{


    TYPEINIT0(CellController);
    TYPEINIT1(EditCellController, CellController);
    TYPEINIT1(SpinCellController, CellController);
    TYPEINIT1(CheckBoxCellController, CellController);
    TYPEINIT1(ComboBoxCellController, CellController);
    TYPEINIT1(ListBoxCellController, CellController);

    TYPEINIT1( FormattedFieldCellController, EditCellController );


    //= ComboBoxControl

    ComboBoxControl::ComboBoxControl(vcl::Window* pParent, WinBits nWinStyle)
                   :ComboBox(pParent, nWinStyle|WB_DROPDOWN|WB_NOBORDER)
    {
        EnableAutoSize(false);
        EnableAutocomplete(true);
        SetDropDownLineCount(5);
    }


    bool ComboBoxControl::PreNotify( NotifyEvent& rNEvt )
    {
        switch (rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
                if (!IsInDropDown())
                {
                    const KeyEvent *pEvt = rNEvt.GetKeyEvent();
                    const vcl::KeyCode rKey = pEvt->GetKeyCode();

                    if ((rKey.GetCode() == KEY_UP || rKey.GetCode() == KEY_DOWN) &&
                        (!pEvt->GetKeyCode().IsShift() && pEvt->GetKeyCode().IsMod1()))
                    {
                        // select next resp. previous entry
                        sal_Int32 nPos = GetEntryPos(GetText());
                        int nDir = (rKey.GetCode() == KEY_DOWN ? 1 : -1);
                        if (!((nPos == 0 && nDir == -1) || (nPos >= GetEntryCount() && nDir == 1)))
                        {
                            nPos += nDir;
                            SetText(GetEntry(nPos));
                        }
                        return true;
                    }
                }
                break;
        }
        return ComboBox::PreNotify(rNEvt);
    }


    //= ComboBoxCellController


    ComboBoxCellController::ComboBoxCellController(ComboBoxControl* pWin)
                             :CellController(pWin)
    {
    }


    bool ComboBoxCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        ComboBoxControl& rBox = GetComboBox();
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = rBox.GetSelection();
                return !aSel && aSel.Max() == rBox.GetText().getLength();
            }
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = rBox.GetSelection();
                return !aSel && aSel.Min() == 0;
            }
            case KEY_UP:
            case KEY_DOWN:
                if (rBox.IsInDropDown())
                    return false;
                if (!rEvt.GetKeyCode().IsShift() &&
                     rEvt.GetKeyCode().IsMod1())
                    return false;
                // drop down the list box
                else if (rEvt.GetKeyCode().IsMod2() && rEvt.GetKeyCode().GetCode() == KEY_DOWN)
                    return false;
                // fall-through
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case KEY_RETURN:
                if (rBox.IsInDropDown())
                    return false;
            default:
                return true;
        }
    }


    bool ComboBoxCellController::IsModified() const
    {
        return GetComboBox().IsValueChangedFromSaved();
    }


    void ComboBoxCellController::ClearModified()
    {
        GetComboBox().SaveValue();
    }


    void ComboBoxCellController::SetModifyHdl(const Link& rLink)
    {
        GetComboBox().SetModifyHdl(rLink);
    }


    //= ListBoxControl


    ListBoxControl::ListBoxControl(vcl::Window* pParent, WinBits nWinStyle)
                  :ListBox(pParent, nWinStyle|WB_DROPDOWN|WB_NOBORDER)
    {
        EnableAutoSize(false);
        EnableMultiSelection(false);
        SetDropDownLineCount(20);
    }


    bool ListBoxControl::PreNotify( NotifyEvent& rNEvt )
    {
        switch (rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
                if (!IsInDropDown())
                {
                    const KeyEvent *pEvt = rNEvt.GetKeyEvent();
                    const vcl::KeyCode rKey = pEvt->GetKeyCode();

                    if ((rKey.GetCode() == KEY_UP || rKey.GetCode() == KEY_DOWN) &&
                        (!pEvt->GetKeyCode().IsShift() && pEvt->GetKeyCode().IsMod1()))
                    {
                        // select next resp. previous entry
                        sal_Int32 nPos = GetSelectEntryPos();
                        int nDir = (rKey.GetCode() == KEY_DOWN ? 1 : -1);
                        if (!((nPos == 0 && nDir == -1) || (nPos >= GetEntryCount() && nDir == 1)))
                        {
                            nPos += nDir;
                            SelectEntryPos(nPos);
                        }
                        Select();   // for calling Modify
                        return true;
                    }
                    else if (GetParent()->PreNotify(rNEvt))
                        return true;
                }
                break;
        }
        return ListBox::PreNotify(rNEvt);
    }


    //= ListBoxCellController


    ListBoxCellController::ListBoxCellController(ListBoxControl* pWin)
                             :CellController(pWin)
    {
    }


    bool ListBoxCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        const ListBoxControl& rBox = GetListBox();
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
                // fall-through
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
                if (rBox.IsTravelSelect())
                    return false;
            default:
                return true;
        }
    }


    bool ListBoxCellController::IsModified() const
    {
        return GetListBox().IsValueChangedFromSaved();
    }


    void ListBoxCellController::ClearModified()
    {
        GetListBox().SaveValue();
    }


    void ListBoxCellController::SetModifyHdl(const Link& rLink)
    {
        GetListBox().SetSelectHdl(rLink);
    }


    //= CheckBoxControl


    CheckBoxControl::CheckBoxControl(vcl::Window* pParent, WinBits nWinStyle)
                   :Control(pParent, nWinStyle)
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

        pBox = new TriStateBox(this,WB_CENTER|WB_VCENTER);
        pBox->SetLegacyNoTextAlign( true );
        pBox->EnableChildTransparentMode();
        pBox->SetPaintTransparent( true );
        pBox->SetClickHdl( LINK( this, CheckBoxControl, OnClick ) );
        pBox->Show();
    }


    CheckBoxControl::~CheckBoxControl()
    {
        delete pBox;
    }


    IMPL_LINK_NOARG(CheckBoxControl, OnClick)
    {
        m_aClickLink.Call(pBox);
        return m_aModifyLink.Call(pBox);
    }


    void CheckBoxControl::Resize()
    {
        Control::Resize();
        pBox->SetPosSizePixel(Point(0,0),GetSizePixel());
    }


    void CheckBoxControl::DataChanged( const DataChangedEvent& _rEvent )
    {
        if ( _rEvent.GetType() == DATACHANGED_SETTINGS )
            pBox->SetSettings( GetSettings() );
    }


    void CheckBoxControl::StateChanged( StateChangedType nStateChange )
    {
        Control::StateChanged(nStateChange);
        if ( nStateChange == StateChangedType::ZOOM )
            pBox->SetZoom(GetZoom());
    }


    void CheckBoxControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
    {
        pBox->Draw(pDev,rPos,rSize,nFlags);
    }


    void CheckBoxControl::GetFocus()
    {
        pBox->GrabFocus();
    }


    void CheckBoxControl::Paint(const Rectangle& rClientRect)
    {
        Control::Paint(rClientRect);
        if (HasFocus())
            ShowFocus(aFocusRect);
    }


    bool CheckBoxControl::PreNotify(NotifyEvent& rEvt)
    {
        switch (rEvt.GetType())
        {
            case EVENT_GETFOCUS:
                ShowFocus(aFocusRect);
                break;
            case EVENT_LOSEFOCUS:
                HideFocus();
        }
        return Control::PreNotify(rEvt);
    }


    //= CheckBoxCellController


    bool CheckBoxCellController::WantMouseEvent() const
    {
        return true;
    }


    CheckBox& CheckBoxCellController::GetCheckBox() const
    {
        return static_cast<CheckBoxControl &>(GetWindow()).GetBox();
    }


    bool CheckBoxCellController::IsModified() const
    {
        return GetCheckBox().IsValueChangedFromSaved();
    }


    void CheckBoxCellController::ClearModified()
    {
        GetCheckBox().SaveValue();
    }


    void CheckBoxCellController::SetModifyHdl(const Link& rLink)
    {
        static_cast<CheckBoxControl &>(GetWindow()).SetModifyHdl(rLink);
    }


    //= MultiLineEditImplementation


    OUString MultiLineEditImplementation::GetText( LineEnd aSeparator ) const
    {
        return const_cast< MultiLineEditImplementation* >( this )->GetEditWindow().GetText( aSeparator );
    }


    OUString MultiLineEditImplementation::GetSelected( LineEnd aSeparator ) const
    {
        return const_cast< MultiLineEditImplementation* >( this )->GetEditWindow().GetSelected( aSeparator );
    }


    //= EditCellController


    EditCellController::EditCellController( Edit* _pEdit )
        :CellController( _pEdit )
        ,m_pEditImplementation( new EditImplementation( *_pEdit ) )
        ,m_bOwnImplementation( true )
    {
    }


    EditCellController::EditCellController( IEditImplementation* _pImplementation )
        :CellController( &_pImplementation->GetControl() )
        ,m_pEditImplementation( _pImplementation )
        ,m_bOwnImplementation( false )
    {
    }


    EditCellController::~EditCellController( )
    {
        if ( m_bOwnImplementation )
            DELETEZ( m_pEditImplementation );
    }


    void EditCellController::SetModified()
    {
        m_pEditImplementation->SetModified();
    }


    void EditCellController::ClearModified()
    {
        m_pEditImplementation->ClearModified();
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
            }   break;
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = m_pEditImplementation->GetSelection();
                bResult = !aSel && aSel.Min() == 0;
            }   break;
            default:
                bResult = true;
        }
        return bResult;
    }


    bool EditCellController::IsModified() const
    {
        return m_pEditImplementation->IsModified();
    }


    void EditCellController::SetModifyHdl(const Link& rLink)
    {
        m_pEditImplementation->SetModifyHdl(rLink);
    }


    //= SpinCellController


    SpinCellController::SpinCellController(SpinField* pWin)
                         :CellController(pWin)
    {
    }


    void SpinCellController::SetModified()
    {
        GetSpinWindow().SetModifyFlag();
    }


    void SpinCellController::ClearModified()
    {
        GetSpinWindow().ClearModifyFlag();
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


    bool SpinCellController::IsModified() const
    {
        return GetSpinWindow().IsModified();
    }


    void SpinCellController::SetModifyHdl(const Link& rLink)
    {
        GetSpinWindow().SetModifyHdl(rLink);
    }


    //= FormattedFieldCellController


    FormattedFieldCellController::FormattedFieldCellController( FormattedField* _pFormatted )
        :EditCellController( _pFormatted )
    {
    }


    void FormattedFieldCellController::CommitModifications()
    {
        static_cast< FormattedField& >( GetWindow() ).Commit();
    }


    //= MultiLineTextCell


    void MultiLineTextCell::Modify()
    {
        GetTextEngine()->SetModified( true );
        MultiLineEdit::Modify();
    }


    bool MultiLineTextCell::dispatchKeyEvent( const KeyEvent& _rEvent )
    {
        Selection aOldSelection( GetSelection() );

        bool bWasModified = IsModified();
        ClearModifyFlag( );

        bool bHandled = GetTextView()->KeyInput( _rEvent );

        bool bIsModified = IsModified();
        if ( bWasModified && !bIsModified )
            // not sure whether this can really happen
            SetModifyFlag();

        if ( bHandled ) // the view claimed it handled the key input
        {
            // unfortunately, KeyInput also returns <TRUE/> (means "I handled this key input")
            // when nothing really changed. Let's care for this.
            Selection aNewSelection( GetSelection() );
            if  (  aNewSelection != aOldSelection   // selection changed
                || bIsModified                      // or some other modification
                )
                return true;
        }
        return false;
    }


    bool MultiLineTextCell::PreNotify( NotifyEvent& rNEvt )
    {
        if ( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            if ( IsWindowOrChild( rNEvt.GetWindow() ) )
            {
                // give the text view a chance to handle the keys
                // this is necessary since a lot of keys which are normally handled
                // by this view (in KeyInput) are intercepted by the EditBrowseBox,
                // which uses them for other reasons. An example is the KeyUp key,
                // which is used by both the text view and the edit browse box

                const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
                const vcl::KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
                sal_uInt16 nCode = rKeyCode.GetCode();

                if ( ( nCode == KEY_RETURN ) && ( rKeyCode.GetModifier() == KEY_MOD1 ) )
                {
                    KeyEvent aEvent( pKeyEvent->GetCharCode(),
                        vcl::KeyCode( KEY_RETURN ),
                        pKeyEvent->GetRepeat()
                    );
                    if ( dispatchKeyEvent( aEvent ) )
                        return true;
                }

                if ( ( nCode != KEY_TAB ) && ( nCode != KEY_RETURN ) )   // everything but tab and enter
                {
                    if ( dispatchKeyEvent( *pKeyEvent ) )
                        return true;
                }
            }
        }
        return MultiLineEdit::PreNotify( rNEvt );
    }


}   // namespace svt



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
