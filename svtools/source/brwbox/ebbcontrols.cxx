/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <svtools/editbrowsebox.hxx>
#include <vcl/decoview.hxx>
#include <svtools/fmtfield.hxx>
#include <svtools/xtextedt.hxx>

#include <algorithm>

// .......................................................................
namespace svt
{
// .......................................................................

    TYPEINIT0(CellController);
    TYPEINIT1(EditCellController, CellController);
    TYPEINIT1(SpinCellController, CellController);
    TYPEINIT1(CheckBoxCellController, CellController);
    TYPEINIT1(ComboBoxCellController, CellController);
    TYPEINIT1(ListBoxCellController, CellController);

    TYPEINIT1( FormattedFieldCellController, EditCellController );

    //==================================================================
    //= ComboBoxControl
    //==================================================================
    ComboBoxControl::ComboBoxControl(Window* pParent, WinBits nWinStyle)
                   :ComboBox(pParent, nWinStyle|WB_DROPDOWN|WB_NOBORDER)
    {
        EnableAutoSize(sal_False);
        EnableAutocomplete(sal_True);
        SetDropDownLineCount(5);
    }

    //------------------------------------------------------------------
    long ComboBoxControl::PreNotify( NotifyEvent& rNEvt )
    {
        switch (rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
                if (!IsInDropDown())
                {
                    const KeyEvent *pEvt = rNEvt.GetKeyEvent();
                    const KeyCode rKey = pEvt->GetKeyCode();

                    if ((rKey.GetCode() == KEY_UP || rKey.GetCode() == KEY_DOWN) &&
                        (!pEvt->GetKeyCode().IsShift() && pEvt->GetKeyCode().IsMod1()))
                    {
                        // select next resp. previous entry
                        int nPos = GetEntryPos(GetText());
                        nPos = nPos + (rKey.GetCode() == KEY_DOWN ? 1 : -1);
                        if (nPos < 0)
                            nPos = 0;
                        if (nPos >= GetEntryCount())
                            nPos = GetEntryCount() - 1;
                        SetText(GetEntry(sal::static_int_cast< sal_uInt16 >(nPos)));
                        return 1;
                    }
                }
                break;
        }
        return ComboBox::PreNotify(rNEvt);
    }

    //==================================================================
    //= ComboBoxCellController
    //==================================================================
    //------------------------------------------------------------------
    ComboBoxCellController::ComboBoxCellController(ComboBoxControl* pWin)
                             :CellController(pWin)
    {
    }

    //------------------------------------------------------------------
    sal_Bool ComboBoxCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        ComboBoxControl& rBox = GetComboBox();
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = rBox.GetSelection();
                return !aSel && aSel.Max() == rBox.GetText().Len();
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
                    return sal_False;
                if (!rEvt.GetKeyCode().IsShift() &&
                     rEvt.GetKeyCode().IsMod1())
                    return sal_False;
                // drop down the list box
                else if (rEvt.GetKeyCode().IsMod2() && rEvt.GetKeyCode().GetCode() == KEY_DOWN)
                    return sal_False;
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case KEY_RETURN:
                if (rBox.IsInDropDown())
                    return sal_False;
            default:
                return sal_True;
        }
    }

    //------------------------------------------------------------------
    sal_Bool ComboBoxCellController::IsModified() const
    {
        return GetComboBox().GetSavedValue() != GetComboBox().GetText();
    }

    //------------------------------------------------------------------
    void ComboBoxCellController::ClearModified()
    {
        GetComboBox().SaveValue();
    }

    //------------------------------------------------------------------
    void ComboBoxCellController::SetModifyHdl(const Link& rLink)
    {
        GetComboBox().SetModifyHdl(rLink);
    }

    //==================================================================
    //= ListBoxControl
    //==================================================================
    //------------------------------------------------------------------
    ListBoxControl::ListBoxControl(Window* pParent, WinBits nWinStyle)
                  :ListBox(pParent, nWinStyle|WB_DROPDOWN|WB_NOBORDER)
    {
        EnableAutoSize(sal_False);
        EnableMultiSelection(sal_False);
        SetDropDownLineCount(20);
    }

    //------------------------------------------------------------------
    long ListBoxControl::PreNotify( NotifyEvent& rNEvt )
    {
        switch (rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
                if (!IsInDropDown())
                {
                    const KeyEvent *pEvt = rNEvt.GetKeyEvent();
                    const KeyCode rKey = pEvt->GetKeyCode();

                    if ((rKey.GetCode() == KEY_UP || rKey.GetCode() == KEY_DOWN) &&
                        (!pEvt->GetKeyCode().IsShift() && pEvt->GetKeyCode().IsMod1()))
                    {
                        // select next resp. previous entry
                        int nPos = GetSelectEntryPos();
                        nPos = nPos + (rKey.GetCode() == KEY_DOWN ? 1 : -1);
                        if (nPos < 0)
                            nPos = 0;
                        if (nPos >= GetEntryCount())
                            nPos = GetEntryCount() - 1;
                        SelectEntryPos(sal::static_int_cast< sal_uInt16 >(nPos));
                        Select();   // for calling Modify
                        return 1;
                    }
                    else if (GetParent()->PreNotify(rNEvt))
                        return 1;
                }
                break;
        }
        return ListBox::PreNotify(rNEvt);
    }

    //==================================================================
    //= ListBoxCellController
    //==================================================================
    //------------------------------------------------------------------
    ListBoxCellController::ListBoxCellController(ListBoxControl* pWin)
                             :CellController(pWin)
    {
    }

    //------------------------------------------------------------------
    sal_Bool ListBoxCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        ListBoxControl& rBox = GetListBox();
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_UP:
            case KEY_DOWN:
                if (!rEvt.GetKeyCode().IsShift() &&
                     rEvt.GetKeyCode().IsMod1())
                    return sal_False;
                // drop down the list box
                else
                    if (rEvt.GetKeyCode().IsMod2() && rEvt.GetKeyCode().GetCode() == KEY_DOWN)
                        return sal_False;
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
                if (rBox.IsTravelSelect())
                    return sal_False;
            default:
                return sal_True;
        }
    }

    //------------------------------------------------------------------
    sal_Bool ListBoxCellController::IsModified() const
    {
        return GetListBox().GetSelectEntryPos() != GetListBox().GetSavedValue();
    }

    //------------------------------------------------------------------
    void ListBoxCellController::ClearModified()
    {
        GetListBox().SaveValue();
    }

    //------------------------------------------------------------------
    void ListBoxCellController::SetModifyHdl(const Link& rLink)
    {
        GetListBox().SetSelectHdl(rLink);
    }

    //==================================================================
    //= CheckBoxControl
    //==================================================================
    //------------------------------------------------------------------
    CheckBoxControl::CheckBoxControl(Window* pParent, WinBits nWinStyle)
                   :Control(pParent, nWinStyle)
    {
        const Wallpaper& rParentBackground = pParent->GetBackground();
        if ( (pParent->GetStyle() & WB_CLIPCHILDREN) || rParentBackground.IsFixed() )
            SetBackground( rParentBackground );
        else
        {
            SetPaintTransparent( sal_True );
            SetBackground();
        }

        EnableChildTransparentMode();

        pBox = new TriStateBox(this,WB_CENTER|WB_VCENTER);
        pBox->SetLegacyNoTextAlign( true );
        pBox->EnableChildTransparentMode();
        pBox->SetPaintTransparent( sal_True );
        pBox->SetClickHdl( LINK( this, CheckBoxControl, OnClick ) );
        pBox->Show();
    }

    //------------------------------------------------------------------
    CheckBoxControl::~CheckBoxControl()
    {
        delete pBox;
    }

    //------------------------------------------------------------------
    IMPL_LINK_NOARG(CheckBoxControl, OnClick)
    {
        m_aClickLink.Call(pBox);
        return m_aModifyLink.Call(pBox);
    }

    //------------------------------------------------------------------
    void CheckBoxControl::Resize()
    {
        Control::Resize();
        pBox->SetPosSizePixel(Point(0,0),GetSizePixel());
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::DataChanged( const DataChangedEvent& _rEvent )
    {
        if ( _rEvent.GetType() == DATACHANGED_SETTINGS )
            pBox->SetSettings( GetSettings() );
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::StateChanged( StateChangedType nStateChange )
    {
        Control::StateChanged(nStateChange);
        if ( nStateChange == STATE_CHANGE_ZOOM )
            pBox->SetZoom(GetZoom());
    }

    //------------------------------------------------------------------
    void CheckBoxControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
    {
        pBox->Draw(pDev,rPos,rSize,nFlags);
    }

    //------------------------------------------------------------------
    void CheckBoxControl::GetFocus()
    {
        pBox->GrabFocus();
    }

    //------------------------------------------------------------------
    void CheckBoxControl::Paint(const Rectangle& rClientRect)
    {
        Control::Paint(rClientRect);
        if (HasFocus())
            ShowFocus(aFocusRect);
    }

    //------------------------------------------------------------------
    long CheckBoxControl::PreNotify(NotifyEvent& rEvt)
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

    //==================================================================
    //= CheckBoxCellController
    //==================================================================
    //------------------------------------------------------------------
    sal_Bool CheckBoxCellController::WantMouseEvent() const
    {
        return sal_True;
    }

    //------------------------------------------------------------------
    CheckBox& CheckBoxCellController::GetCheckBox() const
    {
        return ((CheckBoxControl &)GetWindow()).GetBox();
    }

    //------------------------------------------------------------------
    sal_Bool CheckBoxCellController::IsModified() const
    {
        return GetCheckBox().GetSavedValue() != GetCheckBox().GetState();
    }

    //------------------------------------------------------------------
    void CheckBoxCellController::ClearModified()
    {
        GetCheckBox().SaveValue();
    }

    //------------------------------------------------------------------
    void CheckBoxCellController::SetModifyHdl(const Link& rLink)
    {
        ((CheckBoxControl &)GetWindow()).SetModifyHdl(rLink);
    }

    //==================================================================
    //= MultiLineEditImplementation
    //==================================================================
    //------------------------------------------------------------------
    String MultiLineEditImplementation::GetText( LineEnd aSeparator ) const
    {
        return const_cast< MultiLineEditImplementation* >( this )->GetEditWindow().GetText( aSeparator );
    }

    //------------------------------------------------------------------
    String MultiLineEditImplementation::GetSelected( LineEnd aSeparator ) const
    {
        return const_cast< MultiLineEditImplementation* >( this )->GetEditWindow().GetSelected( aSeparator );
    }

    //==================================================================
    //= EditCellController
    //==================================================================
    //------------------------------------------------------------------
    EditCellController::EditCellController( Edit* _pEdit )
        :CellController( _pEdit )
        ,m_pEditImplementation( new EditImplementation( *_pEdit ) )
        ,m_bOwnImplementation( sal_True )
    {
    }

    //------------------------------------------------------------------
    EditCellController::EditCellController( IEditImplementation* _pImplementation )
        :CellController( &_pImplementation->GetControl() )
        ,m_pEditImplementation( _pImplementation )
        ,m_bOwnImplementation( sal_False )
    {
    }

    //-----------------------------------------------------------------------------
    EditCellController::~EditCellController( )
    {
        if ( m_bOwnImplementation )
            DELETEZ( m_pEditImplementation );
    }

    //-----------------------------------------------------------------------------
    void EditCellController::SetModified()
    {
        m_pEditImplementation->SetModified();
    }

    //-----------------------------------------------------------------------------
    void EditCellController::ClearModified()
    {
        m_pEditImplementation->ClearModified();
    }

    //------------------------------------------------------------------
    sal_Bool EditCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        sal_Bool bResult;
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = m_pEditImplementation->GetSelection();
                bResult = !aSel && aSel.Max() == m_pEditImplementation->GetText( LINEEND_LF ).Len();
            }   break;
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = m_pEditImplementation->GetSelection();
                bResult = !aSel && aSel.Min() == 0;
            }   break;
            default:
                bResult = sal_True;
        }
        return bResult;
    }

    //------------------------------------------------------------------
    sal_Bool EditCellController::IsModified() const
    {
        return m_pEditImplementation->IsModified();
    }

    //------------------------------------------------------------------
    void EditCellController::SetModifyHdl(const Link& rLink)
    {
        m_pEditImplementation->SetModifyHdl(rLink);
    }

    //==================================================================
    //= SpinCellController
    //==================================================================
    //------------------------------------------------------------------
    SpinCellController::SpinCellController(SpinField* pWin)
                         :CellController(pWin)
    {
    }

    //-----------------------------------------------------------------------------
    void SpinCellController::SetModified()
    {
        GetSpinWindow().SetModifyFlag();
    }

    //-----------------------------------------------------------------------------
    void SpinCellController::ClearModified()
    {
        GetSpinWindow().ClearModifyFlag();
    }

    //------------------------------------------------------------------
    sal_Bool SpinCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        sal_Bool bResult;
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = GetSpinWindow().GetSelection();
                bResult = !aSel && aSel.Max() == GetSpinWindow().GetText().Len();
            }   break;
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = GetSpinWindow().GetSelection();
                bResult = !aSel && aSel.Min() == 0;
            }   break;
            default:
                bResult = sal_True;
        }
        return bResult;
    }

    //------------------------------------------------------------------
    sal_Bool SpinCellController::IsModified() const
    {
        return GetSpinWindow().IsModified();
    }

    //------------------------------------------------------------------
    void SpinCellController::SetModifyHdl(const Link& rLink)
    {
        GetSpinWindow().SetModifyHdl(rLink);
    }

    //==================================================================
    //= FormattedFieldCellController
    //==================================================================
    //------------------------------------------------------------------
    FormattedFieldCellController::FormattedFieldCellController( FormattedField* _pFormatted )
        :EditCellController( _pFormatted )
    {
    }

    //------------------------------------------------------------------
    void FormattedFieldCellController::CommitModifications()
    {
        static_cast< FormattedField& >( GetWindow() ).Commit();
    }

    //==================================================================
    //= MultiLineTextCell
    //==================================================================
    //------------------------------------------------------------------
    void MultiLineTextCell::Modify()
    {
        GetTextEngine()->SetModified( sal_True );
        MultiLineEdit::Modify();
    }

    //------------------------------------------------------------------
    sal_Bool MultiLineTextCell::dispatchKeyEvent( const KeyEvent& _rEvent )
    {
        Selection aOldSelection( GetSelection() );

        sal_Bool bWasModified = IsModified();
        ClearModifyFlag( );

        sal_Bool bHandled = GetTextView()->KeyInput( _rEvent );

        sal_Bool bIsModified = IsModified();
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
                return sal_True;
        }
        return sal_False;
    }

    //------------------------------------------------------------------
    long MultiLineTextCell::PreNotify( NotifyEvent& rNEvt )
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
                const KeyCode&  rKeyCode  = pKeyEvent->GetKeyCode();
                sal_uInt16 nCode = rKeyCode.GetCode();

                if ( ( nCode == KEY_RETURN ) && ( rKeyCode.GetModifier() == KEY_MOD1 ) )
                {
                    KeyEvent aEvent( pKeyEvent->GetCharCode(),
                        KeyCode( KEY_RETURN ),
                        pKeyEvent->GetRepeat()
                    );
                    if ( dispatchKeyEvent( aEvent ) )
                        return 1;
                }

                if ( ( nCode != KEY_TAB ) && ( nCode != KEY_RETURN ) )   // everything but tab and enter
                {
                    if ( dispatchKeyEvent( *pKeyEvent ) )
                        return 1;
                }
            }
        }
        return MultiLineEdit::PreNotify( rNEvt );
    }

// .......................................................................
}   // namespace svt
// .......................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
