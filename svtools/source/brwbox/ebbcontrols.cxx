/*************************************************************************
 *
 *  $RCSfile: ebbcontrols.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:37:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVTOOLS_EDITBROWSEBOX_HXX_
#include "editbrowsebox.hxx"
#endif

#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#include "fmtfield.hxx"

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
    ComboBoxControl::ComboBoxControl(Window* pParent, WinBits nWinStyle)
                   :ComboBox(pParent, nWinStyle|WB_DROPDOWN|WB_NOBORDER)
    {
        EnableAutoSize(sal_False);
        EnableAutocomplete(sal_True);
        SetDropDownLineCount(5);
    }

    //------------------------------------------------------------------------------
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
                        nPos += rKey.GetCode() == KEY_DOWN ? 1 : -1;
                        if (nPos < 0)
                            nPos = 0;
                        if (nPos >= GetEntryCount())
                            nPos = GetEntryCount() - 1;
                        SetText(GetEntry(nPos));
                        return 1;
                    }
                }
                break;
        }
        return ComboBox::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------------------
    ComboBoxCellController::ComboBoxCellController(ComboBoxControl* pWin)
                             :CellController(pWin)
    {
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    sal_Bool ComboBoxCellController::IsModified() const
    {
        return GetComboBox().GetSavedValue() != GetComboBox().GetText();
    }

    //------------------------------------------------------------------------------
    void ComboBoxCellController::ClearModified()
    {
        GetComboBox().SaveValue();
    }

    //------------------------------------------------------------------------------
    void ComboBoxCellController::SetModifyHdl(const Link& rLink)
    {
        GetComboBox().SetModifyHdl(rLink);
    }

    //==================================================================
    ListBoxControl::ListBoxControl(Window* pParent, WinBits nWinStyle)
                  :ListBox(pParent, nWinStyle|WB_DROPDOWN|WB_NOBORDER)
    {
        EnableAutoSize(sal_False);
        EnableMultiSelection(sal_False);
        SetDropDownLineCount(5);
    }

    //------------------------------------------------------------------------------
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
                        nPos += rKey.GetCode() == KEY_DOWN ? 1 : -1;
                        if (nPos < 0)
                            nPos = 0;
                        if (nPos >= GetEntryCount())
                            nPos = GetEntryCount() - 1;
                        SelectEntryPos(nPos);
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

    //------------------------------------------------------------------------------
    ListBoxCellController::ListBoxCellController(ListBoxControl* pWin)
                             :CellController(pWin)
    {
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    sal_Bool ListBoxCellController::IsModified() const
    {
        return GetListBox().GetSelectEntryPos() != GetListBox().GetSavedValue();
    }

    //------------------------------------------------------------------------------
    void ListBoxCellController::ClearModified()
    {
        GetListBox().SaveValue();
    }

    //------------------------------------------------------------------------------
    void ListBoxCellController::SetModifyHdl(const Link& rLink)
    {
        GetListBox().SetSelectHdl(rLink);
    }


    //==============================================================================
    //------------------------------------------------------------------------------
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
        pBox->EnableChildTransparentMode();
        pBox->SetPaintTransparent( sal_True );
        pBox->SetClickHdl( LINK( this, CheckBoxControl, OnClick ) );
        pBox->Show();
    }

    //------------------------------------------------------------------------------
    CheckBoxControl::~CheckBoxControl()
    {
        delete pBox;
    }

    //------------------------------------------------------------------
    IMPL_LINK( CheckBoxControl, OnClick, void*, EMPTYARG )
    {
        m_aClickLink.Call(pBox);
        return m_aModifyLink.Call(pBox);
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::Resize()
    {
        Control::Resize();
        pBox->SetPosSizePixel(Point(0,0),GetSizePixel());
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::StateChanged( StateChangedType nStateChange )
    {
        Control::StateChanged(nStateChange);
        if ( nStateChange == STATE_CHANGE_ZOOM )
            pBox->SetZoom(GetZoom());
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
    {
        pBox->Draw(pDev,rPos,rSize,nFlags);
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::GetFocus()
    {
        pBox->GrabFocus();
    }

    //------------------------------------------------------------------------------
    void CheckBoxControl::Paint(const Rectangle& rClientRect)
    {
        Control::Paint(rClientRect);
        if (HasFocus())
            ShowFocus(aFocusRect);
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    sal_Bool CheckBoxCellController::WantMouseEvent() const
    {
        return sal_True;
    }

    //------------------------------------------------------------------------------
    CheckBox& CheckBoxCellController::GetCheckBox() const
    {
        return ((CheckBoxControl &)GetWindow()).GetBox();
    }

    //------------------------------------------------------------------------------
    sal_Bool CheckBoxCellController::IsModified() const
    {
        return GetCheckBox().GetSavedValue() != GetCheckBox().GetState();
    }

    //------------------------------------------------------------------------------
    void CheckBoxCellController::ClearModified()
    {
        GetCheckBox().SaveValue();
    }

    //------------------------------------------------------------------------------
    void CheckBoxCellController::SetModifyHdl(const Link& rLink)
    {
        ((CheckBoxControl &)GetWindow()).SetModifyHdl(rLink);
    }

    //------------------------------------------------------------------------------
    EditCellController::EditCellController(Edit* pWin)
                         :CellController(pWin)
    {
    }

    //-----------------------------------------------------------------------------
    void EditCellController::SetModified()
    {
        GetEditWindow().SetModifyFlag();
    }

    //-----------------------------------------------------------------------------
    void EditCellController::ClearModified()
    {
        GetEditWindow().ClearModifyFlag();
    }

    //------------------------------------------------------------------------------
    sal_Bool EditCellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        sal_Bool bResult;
        switch (rEvt.GetKeyCode().GetCode())
        {
            case KEY_END:
            case KEY_RIGHT:
            {
                Selection aSel = GetEditWindow().GetSelection();
                bResult = !aSel && aSel.Max() == GetEditWindow().GetText().Len();
            }   break;
            case KEY_HOME:
            case KEY_LEFT:
            {
                Selection aSel = GetEditWindow().GetSelection();
                bResult = !aSel && aSel.Min() == 0;
            }   break;
            default:
                bResult = sal_True;
        }
        return bResult;
    }

    //------------------------------------------------------------------------------
    sal_Bool EditCellController::IsModified() const
    {
        return GetEditWindow().IsModified();
    }

    //------------------------------------------------------------------------------
    void EditCellController::SetModifyHdl(const Link& rLink)
    {
        GetEditWindow().SetModifyHdl(rLink);
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    sal_Bool SpinCellController::IsModified() const
    {
        return GetSpinWindow().IsModified();
    }

    //------------------------------------------------------------------------------
    void SpinCellController::SetModifyHdl(const Link& rLink)
    {
        GetSpinWindow().SetModifyHdl(rLink);
    }

    //------------------------------------------------------------------------------
    FormattedFieldCellController::FormattedFieldCellController( FormattedField* _pFormatted )
        :EditCellController( _pFormatted )
    {
    }

    //------------------------------------------------------------------------------
    void FormattedFieldCellController::CommitModifications()
    {
        static_cast< FormattedField& >( GetWindow() ).Commit();
    }

// .......................................................................
}   // namespace svt
// .......................................................................


