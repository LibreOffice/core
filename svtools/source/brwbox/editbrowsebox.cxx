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

#include <tools/debug.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include <bitmaps.hlst>

#include <algorithm>
#include "editbrowseboximpl.hxx"
#include <com/sun/star/accessibility/AccessibleEventId.hpp>


namespace svt
{

    namespace
    {

        GetFocusFlags getRealGetFocusFlags( vcl::Window* _pWindow )
        {
            GetFocusFlags nFlags = GetFocusFlags::NONE;
            while ( _pWindow && nFlags == GetFocusFlags::NONE )
            {
                nFlags = _pWindow->GetGetFocusFlags( );
                _pWindow = _pWindow->GetParent();
            }
            return nFlags;
        }
    }

    using namespace ::com::sun::star::uno;
    using namespace com::sun::star::accessibility::AccessibleEventId;


    IEditImplementation::~IEditImplementation()
    {
    }

    //= EditBrowserHeader

    void EditBrowserHeader::DoubleClick()
    {
        sal_uInt16 nColId = GetCurItemId();
        if (nColId)
        {
            sal_uInt32 nAutoWidth = static_cast<EditBrowseBox*>(GetParent())->GetAutoColumnWidth(nColId);
            if (nAutoWidth != static_cast<EditBrowseBox*>(GetParent())->GetColumnWidth(nColId))
            {
                static_cast<EditBrowseBox*>(GetParent())->SetColumnWidth(nColId, nAutoWidth);
                static_cast<EditBrowseBox*>(GetParent())->ColumnResized(nColId);
            }
        }
    }

    //= EditBrowseBox

    void EditBrowseBox::BrowserMouseEventPtr::Clear()
    {
        pEvent.reset();
    }

    void EditBrowseBox::BrowserMouseEventPtr::Set(const BrowserMouseEvent* pEvt, bool bIsDown)
    {
        if (pEvt == pEvent.get())
        {
            bDown = bIsDown;
            return;
        }
        pEvent.reset();
        if (pEvt)
        {
            pEvent.reset(new BrowserMouseEvent(pEvt->GetWindow(),
                                           *pEvt,
                                           pEvt->GetRow(),
                                           pEvt->GetColumn(),
                                           pEvt->GetColumnId(),
                                           pEvt->GetRect()));
            bDown = bIsDown;
        }
    }

    EditBrowseBox::EditBrowseBox( vcl::Window* pParent, EditBrowseBoxFlags nBrowserFlags, WinBits nBits, BrowserMode _nMode )
                  :BrowseBox( pParent, nBits, _nMode )
                  ,nStartEvent(nullptr)
                  ,nEndEvent(nullptr)
                  ,nCellModifiedEvent(nullptr)
                  ,m_pFocusWhileRequest(nullptr)
                  ,nPaintRow(-1)
                  ,nEditRow(-1)
                  ,nEditCol(0)
                  ,bHasFocus(false)
                  ,bPaintStatus(true)
                  ,bActiveBeforeTracking( false )
                  ,m_nBrowserFlags(nBrowserFlags)
                  ,pHeader(nullptr)
    {
        m_aImpl.reset(new EditBrowseBoxImpl);

        SetCompoundControl(true);

        ImplInitSettings(true, true, true);

        pCheckBoxPaint = VclPtr<CheckBoxControl>::Create(&GetDataWindow());
        pCheckBoxPaint->SetPaintTransparent( true );
        pCheckBoxPaint->SetBackground();
    }

    void EditBrowseBox::Init()
    {
        // late construction
    }

    EditBrowseBox::~EditBrowseBox()
    {
        disposeOnce();
    }

    void EditBrowseBox::dispose()
    {
        if (nStartEvent)
            Application::RemoveUserEvent(nStartEvent);
        if (nEndEvent)
            Application::RemoveUserEvent(nEndEvent);
        if (nCellModifiedEvent)
            Application::RemoveUserEvent(nCellModifiedEvent);

        pCheckBoxPaint.disposeAndClear();
        m_pFocusWhileRequest.clear();
        pHeader.clear();
        BrowseBox::dispose();
    }


    void EditBrowseBox::RemoveRows()
    {
        BrowseBox::Clear();
        nEditRow = nPaintRow = -1;
        nEditCol = 0;
    }

    VclPtr<BrowserHeader> EditBrowseBox::CreateHeaderBar(BrowseBox* pParent)
    {
        pHeader = imp_CreateHeaderBar(pParent);
        if (!IsUpdateMode())
            pHeader->SetUpdateMode(false);
        return pHeader;
    }

    VclPtr<BrowserHeader> EditBrowseBox::imp_CreateHeaderBar(BrowseBox* pParent)
    {
        return VclPtr<EditBrowserHeader>::Create(pParent);
    }

    void EditBrowseBox::LoseFocus()
    {
        BrowseBox::LoseFocus();
        DetermineFocus();
    }

    void EditBrowseBox::GetFocus()
    {
        BrowseBox::GetFocus();

        // This should handle the case that the BrowseBox (or one of its children)
        // gets the focus from outside by pressing Tab
        if (IsEditing() && Controller()->GetWindow().IsVisible())
            Controller()->GetWindow().GrabFocus();

        DetermineFocus(getRealGetFocusFlags(this));
    }

    bool EditBrowseBox::SeekRow(sal_Int32 nRow)
    {
        nPaintRow = nRow;
        return true;
    }

    IMPL_LINK_NOARG(EditBrowseBox, StartEditHdl, void*, void)
    {
        nStartEvent = nullptr;
        if (IsEditing())
        {
            EnableAndShow();
            if (!ControlHasFocus() && (m_pFocusWhileRequest.get() == Application::GetFocusWindow()))
                aController->GetWindow().GrabFocus();
        }
    }

    void EditBrowseBox::PaintField( vcl::RenderContext& rDev, const tools::Rectangle& rRect,
                                    sal_uInt16 nColumnId ) const
    {
        if (nColumnId == HandleColumnId)
        {
             if (bPaintStatus)
                PaintStatusCell(rDev, rRect);
        }
        else
        {
            // don't paint the current cell
            if (&rDev == &GetDataWindow())
                // but only if we're painting onto our data win (which is the usual painting)
                if (nPaintRow == nEditRow)
                {
                    if (IsEditing() && nEditCol == nColumnId && aController->GetWindow().IsVisible())
                        return;
                }
            PaintCell(rDev, rRect, nColumnId);
        }
    }

    Image EditBrowseBox::GetImage(RowStatus eStatus) const
    {
        BitmapEx aBitmap;
        bool bNeedMirror = IsRTLEnabled();
        switch (eStatus)
        {
            case CURRENT:
                aBitmap = BitmapEx(BMP_CURRENT);
                break;
            case CURRENTNEW:
                aBitmap = BitmapEx(BMP_CURRENTNEW);
                break;
            case MODIFIED:
                aBitmap = BitmapEx(BMP_MODIFIED);
                bNeedMirror = false;    // the pen is not mirrored
                break;
            case NEW:
                aBitmap = BitmapEx(BMP_NEW);
                break;
            case DELETED:
                aBitmap = BitmapEx(BMP_DELETED);
                break;
            case PRIMARYKEY:
                aBitmap = BitmapEx(BMP_PRIMARYKEY);
                break;
            case CURRENT_PRIMARYKEY:
                aBitmap = BitmapEx(BMP_CURRENT_PRIMARYKEY);
                break;
            case FILTER:
                aBitmap = BitmapEx(BMP_FILTER);
                break;
            case HEADERFOOTER:
                aBitmap = BitmapEx(BMP_HEADERFOOTER);
                break;
            case CLEAN:
                break;
        }
        if ( bNeedMirror )
        {
            aBitmap.Mirror( BmpMirrorFlags::Horizontal );
        }
        return Image(aBitmap);
    }

    void EditBrowseBox::PaintStatusCell(OutputDevice& rDev, const tools::Rectangle& rRect) const
    {
        if (nPaintRow < 0)
            return;

        RowStatus eStatus = GetRowStatus( nPaintRow );
        EditBrowseBoxFlags nBrowserFlags = GetBrowserFlags();

        if (nBrowserFlags & EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT)
            return;

        // draw the text of the header column
        if (nBrowserFlags & EditBrowseBoxFlags::HANDLE_COLUMN_TEXT )
        {
            rDev.DrawText( rRect, GetCellText( nPaintRow, 0 ),
                           DrawTextFlags::Center | DrawTextFlags::VCenter | DrawTextFlags::Clip );
        }
        // draw an image
        else if (eStatus != CLEAN && rDev.GetOutDevType() == OUTDEV_WINDOW)
        {
            Image aImage(GetImage(eStatus));
            // calc the image position
            Size aImageSize(aImage.GetSizePixel());
            aImageSize.setWidth( CalcZoom(aImageSize.Width()) );
            aImageSize.setHeight( CalcZoom(aImageSize.Height()) );
            Point aPos( rRect.TopLeft() );

            if ( ( aImageSize.Width() > rRect.GetWidth() ) || ( aImageSize.Height() > rRect.GetHeight() ) )
                rDev.SetClipRegion(vcl::Region(rRect));

            if ( aImageSize.Width() < rRect.GetWidth() )
                aPos.AdjustX(( rRect.GetWidth() - aImageSize.Width() ) / 2 );

            if ( aImageSize.Height() < rRect.GetHeight() )
                aPos.AdjustY(( rRect.GetHeight() - aImageSize.Height() ) / 2 );

            if ( IsZoom() )
                rDev.DrawImage( aPos, aImageSize, aImage );
            else
                rDev.DrawImage( aPos, aImage );

            if (rDev.IsClipRegion())
                rDev.SetClipRegion();
        }
    }


    void EditBrowseBox::ImplStartTracking()
    {
        bActiveBeforeTracking = IsEditing();
        if ( bActiveBeforeTracking )
        {
            DeactivateCell();
            PaintImmediately();
        }

        BrowseBox::ImplStartTracking();
    }


    void EditBrowseBox::ImplEndTracking()
    {
        if ( bActiveBeforeTracking )
            ActivateCell();
        bActiveBeforeTracking = false;

        BrowseBox::ImplEndTracking();
    }


    void EditBrowseBox::RowHeightChanged()
    {
        if ( IsEditing() )
        {
            tools::Rectangle aRect( GetCellRect( nEditRow, nEditCol, false ) );
            CellControllerRef aCellController( Controller() );
            ResizeController( aCellController, aRect );
            aCellController->GetWindow().GrabFocus();
        }

        BrowseBox::RowHeightChanged();
    }


    EditBrowseBox::RowStatus EditBrowseBox::GetRowStatus(sal_Int32) const
    {
        return CLEAN;
    }


    void EditBrowseBox::KeyInput( const KeyEvent& rEvt )
    {
        sal_uInt16 nCode = rEvt.GetKeyCode().GetCode();
        bool   bShift = rEvt.GetKeyCode().IsShift();
        bool   bCtrl = rEvt.GetKeyCode().IsMod1();

        switch (nCode)
        {
            case KEY_RETURN:
                if (!bCtrl && !bShift && IsTabAllowed(true))
                {
                    Dispatch(BROWSER_CURSORRIGHT);
                }
                else
                    BrowseBox::KeyInput(rEvt);
                return;
            case KEY_TAB:
                if (!bCtrl && !bShift)
                {
                    if (IsTabAllowed(true))
                        Dispatch(BROWSER_CURSORRIGHT);
                    else
                        // do NOT call BrowseBox::KeyInput : this would handle the tab, but we already now
                        // that tab isn't allowed here. So give the Control class a chance
                        Control::KeyInput(rEvt);
                    return;
                }
                else if (!bCtrl && bShift)
                {
                    if (IsTabAllowed(false))
                        Dispatch(BROWSER_CURSORLEFT);
                    else
                        // do NOT call BrowseBox::KeyInput : this would handle the tab, but we already now
                        // that tab isn't allowed here. So give the Control class a chance
                        Control::KeyInput(rEvt);
                    return;
                }
                [[fallthrough]];
            default:
                BrowseBox::KeyInput(rEvt);
        }
    }

    void EditBrowseBox::ChildFocusIn()
    {
        DetermineFocus(getRealGetFocusFlags(this));
    }

    void EditBrowseBox::ChildFocusOut()
    {
        DetermineFocus();
    }

    void EditBrowseBox::MouseButtonDown(const BrowserMouseEvent& rEvt)
    {
        // absorb double clicks
        if (rEvt.GetClicks() > 1 && rEvt.GetRow() >= 0)
            return;

        // we are about to leave the current cell. If there is a "this cell has been modified" notification
        // pending (asynchronously), this may be deadly -> do it synchronously
        if ( nCellModifiedEvent )
        {
            Application::RemoveUserEvent( nCellModifiedEvent );
            nCellModifiedEvent = nullptr;
            LINK( this, EditBrowseBox, CellModifiedHdl ).Call( nullptr );
        }

        if (rEvt.GetColumnId() == HandleColumnId)
        {   // it was the handle column. save the current cell content if necessary
            // (clicking on the handle column results in selecting the current row)
            if (IsEditing() && aController->IsValueChangedFromSaved())
                SaveModified();
        }

        aMouseEvent.Set(&rEvt,true);
        BrowseBox::MouseButtonDown(rEvt);
        aMouseEvent.Clear();

        if (m_nBrowserFlags & EditBrowseBoxFlags::ACTIVATE_ON_BUTTONDOWN)
        {
            // the base class does not travel upon MouseButtonDown, but implActivateCellOnMouseEvent assumes we traveled ...
            GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
            if (rEvt.GetRow() >= 0)
                implActivateCellOnMouseEvent(rEvt, false);
        }
    }

    void EditBrowseBox::MouseButtonUp( const BrowserMouseEvent& rEvt )
    {
        // absorb double clicks
        if (rEvt.GetClicks() > 1 && rEvt.GetRow() >= 0)
            return;

        aMouseEvent.Set(&rEvt,false);
        BrowseBox::MouseButtonUp(rEvt);
        aMouseEvent.Clear();

        if (!(m_nBrowserFlags & EditBrowseBoxFlags::ACTIVATE_ON_BUTTONDOWN))
            if (rEvt.GetRow() >= 0)
                implActivateCellOnMouseEvent(rEvt, true);
    }

    bool EditBrowseBox::ControlHasFocus() const
    {
        Window* pControlWindow = aController ? &aController->GetWindow() : nullptr;
        if (ControlBase* pControlBase = dynamic_cast<ControlBase*>(pControlWindow))
            return pControlBase->ControlHasFocus();
        return pControlWindow && pControlWindow->HasChildPathFocus();
    }

    void EditBrowseBox::implActivateCellOnMouseEvent(const BrowserMouseEvent& _rEvt, bool _bUp)
    {
        if (!IsEditing())
            ActivateCell();
        else if (IsEditing() && !aController->GetWindow().IsEnabled())
            DeactivateCell();
        else if (IsEditing() && !ControlHasFocus())
            AsynchGetFocus();

        if (!IsEditing() || !aController->GetWindow().IsEnabled())
            return;

        // forwards the event to the control
        aController->ActivatingMouseEvent(_rEvt, _bUp);
    }

    void EditBrowseBox::Dispatch( sal_uInt16 _nId )
    {
        if ( _nId == BROWSER_ENHANCESELECTION )
        {   // this is a workaround for the bug in the base class:
            // if the row selection is to be extended (which is what BROWSER_ENHANCESELECTION tells us)
            // then the base class does not revert any column selections, while, for doing a "simple"
            // selection (BROWSER_SELECT), it does. In fact, it does not only revert the col selection then,
            // but also any current row selections.
            // This clearly tells me that the both ids are for row selection only - there this behaviour does
            // make sense.
            // But here, where we have column selection, too, we take care of this ourself.
            if ( GetSelectColumnCount( ) )
            {
                while ( GetSelectColumnCount( ) )
                    SelectColumnPos(
                        sal::static_int_cast< sal_uInt16 >(FirstSelectedColumn()),
                        false );
                Select();
            }
        }
        BrowseBox::Dispatch( _nId );
    }

    bool EditBrowseBox::ProcessKey(const KeyEvent& rKeyEvent)
    {
        sal_uInt16 nCode  = rKeyEvent.GetKeyCode().GetCode();
        bool       bShift = rKeyEvent.GetKeyCode().IsShift();
        bool       bCtrl  = rKeyEvent.GetKeyCode().IsMod1();
        bool       bAlt =   rKeyEvent.GetKeyCode().IsMod2();
        bool       bLocalSelect = false;
        bool       bNonEditOnly = false;
        sal_uInt16 nId = BROWSER_NONE;

        if (!bAlt && !bCtrl && !bShift )
            switch ( nCode )
            {
                case KEY_DOWN:          nId = BROWSER_CURSORDOWN; break;
                case KEY_UP:            nId = BROWSER_CURSORUP; break;
                case KEY_PAGEDOWN:      nId = BROWSER_CURSORPAGEDOWN; break;
                case KEY_PAGEUP:        nId = BROWSER_CURSORPAGEUP; break;
                case KEY_HOME:          nId = BROWSER_CURSORHOME; break;
                case KEY_END:           nId = BROWSER_CURSOREND; break;

                case KEY_TAB:
                    // ask if traveling to the next cell is allowed
                    if (IsTabAllowed(true))
                        nId = BROWSER_CURSORRIGHT;
                    break;

                case KEY_RETURN:
                    // save the cell content (if necessary)
                    if (IsEditing() && aController->IsValueChangedFromSaved() && !SaveModified())
                    {
                        // maybe we're not visible ...
                        EnableAndShow();
                        aController->GetWindow().GrabFocus();
                        return true;
                    }
                    // ask if traveling to the next cell is allowed
                    if (IsTabAllowed(true))
                        nId = BROWSER_CURSORRIGHT;

                    break;
                case KEY_RIGHT:         nId = BROWSER_CURSORRIGHT; break;
                case KEY_LEFT:          nId = BROWSER_CURSORLEFT; break;
                case KEY_SPACE:         nId = BROWSER_SELECT; bNonEditOnly = bLocalSelect = true; break;
            }

        if ( !bAlt && !bCtrl && bShift )
            switch ( nCode )
            {
                case KEY_DOWN:          nId = BROWSER_SELECTDOWN; bLocalSelect = true; break;
                case KEY_UP:            nId = BROWSER_SELECTUP; bLocalSelect = true; break;
                case KEY_HOME:          nId = BROWSER_SELECTHOME; bLocalSelect = true; break;
                case KEY_END:           nId = BROWSER_SELECTEND; bLocalSelect = true; break;
                case KEY_TAB:
                    if (IsTabAllowed(false))
                        nId = BROWSER_CURSORLEFT;
                    break;
            }

        if ( !bAlt && bCtrl && bShift )
            switch ( nCode )
            {
                case KEY_SPACE:         nId = BROWSER_SELECTCOLUMN; bLocalSelect = true; break;
            }


        if ( !bAlt && bCtrl && !bShift )
            switch ( nCode )
            {
                case KEY_DOWN:          nId = BROWSER_SCROLLUP; break;
                case KEY_UP:            nId = BROWSER_SCROLLDOWN; break;
                case KEY_PAGEDOWN:      nId = BROWSER_CURSORENDOFFILE; break;
                case KEY_PAGEUP:        nId = BROWSER_CURSORTOPOFFILE; break;
                case KEY_HOME:          nId = BROWSER_CURSORTOPOFSCREEN; break;
                case KEY_END:           nId = BROWSER_CURSORENDOFSCREEN; break;
                case KEY_SPACE:         nId = BROWSER_ENHANCESELECTION; bLocalSelect = true; break;
            }


        if  (   ( nId != BROWSER_NONE )
            &&  (   !IsEditing()
                ||  (   !bNonEditOnly
                    &&  aController->MoveAllowed(rKeyEvent)
                    )
                )
            )
        {
            if (nId == BROWSER_SELECT || BROWSER_SELECTCOLUMN == nId )
            {
                // save the cell content (if necessary)
                if (IsEditing() && aController->IsValueChangedFromSaved() && !SaveModified())
                {
                    // maybe we're not visible ...
                    EnableAndShow();
                    aController->GetWindow().GrabFocus();
                    return true;
                }
            }

            Dispatch(nId);

            if (bLocalSelect && (GetSelectRowCount() || GetSelection() != nullptr))
                DeactivateCell();
            return true;
        }
        return false;
    }

    bool EditBrowseBox::PreNotify(NotifyEvent& rEvt)
    {
        if (rEvt.GetType() == MouseNotifyEvent::KEYINPUT)
        {
            if  (   (IsEditing() && ControlHasFocus())
                ||  rEvt.GetWindow() == &GetDataWindow()
                ||  (!IsEditing() && HasChildPathFocus())
                )
            {
                if (ProcessKey(*rEvt.GetKeyEvent()))
                    return true;
            }
        }
        return BrowseBox::PreNotify(rEvt);
    }

    bool EditBrowseBox::IsTabAllowed(bool) const
    {
        return true;
    }


    bool EditBrowseBox::EventNotify(NotifyEvent& rEvt)
    {
        switch (rEvt.GetType())
        {
            case MouseNotifyEvent::GETFOCUS:
                DetermineFocus(getRealGetFocusFlags(this));
                break;

            case MouseNotifyEvent::LOSEFOCUS:
                DetermineFocus();
                break;

            default:
                break;
        }
        return BrowseBox::EventNotify(rEvt);
    }


    void EditBrowseBox::StateChanged( StateChangedType nType )
    {
        BrowseBox::StateChanged( nType );

        bool bNeedCellReActivation = false;
        if ( nType == StateChangedType::Mirroring )
        {
            bNeedCellReActivation = true;
        }
        else if ( nType == StateChangedType::Zoom )
        {
            ImplInitSettings( true, false, false );
            bNeedCellReActivation = true;
        }
        else if ( nType == StateChangedType::ControlFont )
        {
            ImplInitSettings( true, false, false );
            Invalidate();
        }
        else if ( nType == StateChangedType::ControlForeground )
        {
            ImplInitSettings( false, true, false );
            Invalidate();
        }
        else if ( nType == StateChangedType::ControlBackground )
        {
            ImplInitSettings( false, false, true );
            Invalidate();
        }
        else if (nType == StateChangedType::Style)
        {
            WinBits nStyle = GetStyle();
            if (!(nStyle & WB_NOTABSTOP) )
                nStyle |= WB_TABSTOP;

            SetStyle(nStyle);
        }
        if ( bNeedCellReActivation )
        {
            if ( IsEditing() )
            {
                DeactivateCell();
                ActivateCell();
            }
        }
    }


    void EditBrowseBox::DataChanged( const DataChangedEvent& rDCEvt )
    {
        BrowseBox::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DataChangedEventType::SETTINGS )   ||
            ( rDCEvt.GetType() == DataChangedEventType::DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE        ))
        {
            ImplInitSettings( true, true, true );
            Invalidate();
        }
    }

    void EditBrowseBox::ImplInitSettings( bool bFont, bool bForeground, bool bBackground )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        if (bFont)
        {
            vcl::Font aFont = rStyleSettings.GetFieldFont();
            if (IsControlFont())
            {
                GetDataWindow().SetControlFont(GetControlFont());
                aFont.Merge(GetControlFont());
            }
            else
                GetDataWindow().SetControlFont();

            GetDataWindow().SetZoomedPointFont(GetDataWindow(), aFont);
        }

        if (bFont || bForeground)
        {
            Color aTextColor = rStyleSettings.GetFieldTextColor();
            if (IsControlForeground())
            {
                aTextColor = GetControlForeground();
                GetDataWindow().SetControlForeground(aTextColor);
            }
            else
                GetDataWindow().SetControlForeground();

            GetDataWindow().SetTextColor( aTextColor );
        }

        if (!bBackground) // FIXME: Outside of Paint Hierarchy
            return;

        if (GetDataWindow().IsControlBackground())
        {
            GetDataWindow().SetControlBackground(GetControlBackground());
            GetDataWindow().SetBackground(GetDataWindow().GetControlBackground());
            GetDataWindow().SetFillColor(GetDataWindow().GetControlBackground());
        }
        else
        {
            GetDataWindow().SetControlBackground();
            GetDataWindow().SetBackground(rStyleSettings.GetFieldColor());
            GetDataWindow().SetFillColor(rStyleSettings.GetFieldColor());
        }
    }


    bool EditBrowseBox::IsCursorMoveAllowed(sal_Int32 nNewRow, sal_uInt16 nNewColId) const
    {
        sal_uInt16 nInfo = 0;

        if (GetSelectColumnCount() || (aMouseEvent.Is() && aMouseEvent->GetRow() < 0))
            nInfo |= COLSELECT;
        if ((GetSelection() != nullptr && GetSelectRowCount()) ||
            (aMouseEvent.Is() && aMouseEvent->GetColumnId() == HandleColumnId))
            nInfo |= ROWSELECT;
        if (!nInfo && nNewRow != nEditRow)
            nInfo |= ROWCHANGE;
        if (!nInfo && nNewColId != nEditCol)
            nInfo |= COLCHANGE;

        if (nInfo == 0)   // nothing happened
            return true;

        // save the cell content
        if (IsEditing() && aController->IsValueChangedFromSaved() && !const_cast<EditBrowseBox *>(this)->SaveModified())
        {
            // maybe we're not visible ...
            EnableAndShow();
            aController->GetWindow().GrabFocus();
            return false;
        }

        EditBrowseBox * pTHIS = const_cast<EditBrowseBox *> (this);

        // save the cell content if
        // a) a selection is being made
        // b) the row is changing
        if (IsModified() && (nInfo & (ROWCHANGE | COLSELECT | ROWSELECT)) &&
            !pTHIS->SaveRow())
        {
            if (nInfo & COLSELECT ||
                nInfo & ROWSELECT)
            {
                // cancel selected
                pTHIS->SetNoSelection();
            }

            if (IsEditing())
            {
                if (!Controller()->GetWindow().IsVisible())
                {
                    EnableAndShow();
                }
                aController->GetWindow().GrabFocus();
            }
            return false;
        }

        if (nNewRow != nEditRow)
        {
            vcl::Window& rWindow = GetDataWindow();
            if ((nEditRow >= 0) && !(GetBrowserFlags() & EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT))
            {
                tools::Rectangle aRect = GetFieldRectPixel(nEditRow, 0, false );
                // status cell should be painted if and only if text is displayed
                pTHIS->bPaintStatus = ( GetBrowserFlags() & EditBrowseBoxFlags::HANDLE_COLUMN_TEXT ) == EditBrowseBoxFlags::HANDLE_COLUMN_TEXT;
                rWindow.Invalidate(aRect);
                pTHIS->bPaintStatus = true;
            }

            // don't paint during row change
            rWindow.EnablePaint(false);

            // the last veto chance for derived classes
            if (!pTHIS->CursorMoving(nNewRow, nNewColId))
            {
                pTHIS->InvalidateStatusCell(nEditRow);
                rWindow.EnablePaint(true);
                return false;
            }
            else
            {
                rWindow.EnablePaint(true);
                return true;
            }
        }
        else
            return pTHIS->CursorMoving(nNewRow, nNewColId);
    }


    void EditBrowseBox::ColumnMoved(sal_uInt16 nId)
    {
        BrowseBox::ColumnMoved(nId);
        if (IsEditing())
        {
            tools::Rectangle aRect( GetCellRect(nEditRow, nEditCol, false));
            CellControllerRef aControllerRef = Controller();
            ResizeController(aControllerRef, aRect);
            Controller()->GetWindow().GrabFocus();
        }
    }


    bool EditBrowseBox::SaveRow()
    {
        return true;
    }


    bool EditBrowseBox::CursorMoving(sal_Int32, sal_uInt16)
    {
        DeactivateCell(false);
        return true;
    }


    void EditBrowseBox::CursorMoved()
    {
        sal_Int32 nNewRow = GetCurRow();
        if (nEditRow != nNewRow)
        {
            if (!(GetBrowserFlags() & EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT))
                InvalidateStatusCell(nNewRow);
            nEditRow = nNewRow;
        }
        ActivateCell();
        GetDataWindow().EnablePaint(true);
        // should not be called here because the descant event is not needed here
        //BrowseBox::CursorMoved();
    }


    void EditBrowseBox::EndScroll()
    {
        if (IsEditing())
        {
            tools::Rectangle aRect = GetCellRect(nEditRow, nEditCol, false);
            ResizeController(aController,aRect);
            AsynchGetFocus();
        }
        BrowseBox::EndScroll();
    }


    void EditBrowseBox::ActivateCell(sal_Int32 nRow, sal_uInt16 nCol, bool bCellFocus)
    {
        if (IsEditing())
            return;

        nEditCol = nCol;

        if ((GetSelectRowCount() && GetSelection() != nullptr) || GetSelectColumnCount() ||
            (aMouseEvent.Is() && (aMouseEvent.IsDown() || aMouseEvent->GetClicks() > 1))) // nothing happens on MouseDown
        {
            return;
        }

        if (nEditRow < 0 || nEditCol <= HandleColumnId)
            return;

        aController = GetController(nRow, nCol);
        if (aController.is())
        {
            tools::Rectangle aRect( GetCellRect(nEditRow, nEditCol, false));
            ResizeController(aController, aRect);

            InitController(aController, nEditRow, nEditCol);

            aController->SaveValue();
            aController->SetModifyHdl(LINK(this,EditBrowseBox,ModifyHdl));
            EnableAndShow();

            if ( isAccessibleAlive() )
                implCreateActiveAccessible();

            // activate the cell only of the browser has the focus
            if ( bHasFocus && bCellFocus )
                AsynchGetFocus();
        }
        else
        {
            // no controller -> we have a new "active descendant"
            if ( isAccessibleAlive() && HasFocus() )
            {
                commitTableEvent(
                    ACTIVE_DESCENDANT_CHANGED,
                    makeAny( CreateAccessibleCell( nRow, GetColumnPos( nCol -1) ) ),
                    Any()
                );
            }
        }
    }


    void EditBrowseBox::DeactivateCell(bool bUpdate)
    {
        if (!IsEditing())
            return;

        if ( isAccessibleAlive() )
        {
            commitBrowseBoxEvent( CHILD, Any(), makeAny( m_aImpl->m_xActiveCell ) );
            m_aImpl->clearActiveCell();
        }

        aOldController = aController;
        aController.clear();

        // reset the modify handler
        aOldController->SetModifyHdl(Link<LinkParamNone*,void>());

        if (bHasFocus)
            GrabFocus(); // ensure that we have (and keep) the focus

        aOldController->suspend();

        // update if requested
        if (bUpdate)
            PaintImmediately();

        // release the controller (asynchronously)
        if (nEndEvent)
            Application::RemoveUserEvent(nEndEvent);
        nEndEvent = Application::PostUserEvent(LINK(this,EditBrowseBox,EndEditHdl), nullptr, true);
    }


    tools::Rectangle EditBrowseBox::GetCellRect(sal_Int32 nRow, sal_uInt16 nColId, bool bRel) const
    {
        tools::Rectangle aRect( GetFieldRectPixel(nRow, nColId, bRel));
        if ((GetMode()  & BrowserMode::CURSOR_WO_FOCUS) == BrowserMode::CURSOR_WO_FOCUS)
        {
            aRect.AdjustTop(1 );
            aRect.AdjustBottom( -1 );
        }
        return aRect;
    }


    IMPL_LINK_NOARG(EditBrowseBox, EndEditHdl, void*, void)
    {
        nEndEvent = nullptr;

        aOldController  = CellControllerRef();
    }


    IMPL_LINK_NOARG(EditBrowseBox, ModifyHdl, LinkParamNone*, void)
    {
        if (nCellModifiedEvent)
            Application::RemoveUserEvent(nCellModifiedEvent);
        nCellModifiedEvent = Application::PostUserEvent(LINK(this,EditBrowseBox,CellModifiedHdl), nullptr, true);
    }


    IMPL_LINK_NOARG(EditBrowseBox, CellModifiedHdl, void*, void)
    {
        nCellModifiedEvent = nullptr;
        CellModified();
    }

    void EditBrowseBox::ColumnResized( sal_uInt16 )
    {
        if (IsEditing())
        {
            tools::Rectangle aRect( GetCellRect(nEditRow, nEditCol, false));
            CellControllerRef aControllerRef = Controller();
            ResizeController(aControllerRef, aRect);
            // don't grab focus if Field Properties panel is being
            // resized by split pane drag resizing
            if (Application::IsUICaptured())
                return;
            Controller()->GetWindow().GrabFocus();
        }
    }

    sal_uInt16 EditBrowseBox::AppendColumn(const OUString& rName, sal_uInt16 nWidth, sal_uInt16 nPos, sal_uInt16 nId)
    {
        if (nId == BROWSER_INVALIDID)
        {
            // look for the next free id
            for (nId = ColCount(); nId > 0 && GetColumnPos(nId) != BROWSER_INVALIDID; nId--)
                ;

            if (!nId)
            {
                // if there is no handle column
                // increment the id
                if ( ColCount() == 0 || GetColumnId(0) != HandleColumnId )
                    nId = ColCount() + 1;
            }
        }

        DBG_ASSERT(nId, "EditBrowseBox::AppendColumn: invalid id!");

        tools::Long w = nWidth;
        if (!w)
            w = GetDefaultColumnWidth(rName);

        InsertDataColumn(nId, rName, w, (HeaderBarItemBits::CENTER | HeaderBarItemBits::CLICKABLE), nPos);
        return nId;
    }

    void EditBrowseBox::Resize()
    {
        BrowseBox::Resize();

        // if the window is smaller than "title line height" + "control area",
        // do nothing
        if (GetOutputSizePixel().Height() <
           (GetControlArea().GetHeight() + GetDataWindow().GetPosPixel().Y()))
            return;

        // the size of the control area
        Point aPoint(GetControlArea().TopLeft());
        sal_uInt16 nX = static_cast<sal_uInt16>(aPoint.X());

        ArrangeControls(nX, static_cast<sal_uInt16>(aPoint.Y()));

        if (!nX)
            nX = USHRT_MAX;

        bool bChanged = ReserveControlArea(nX);

        //tdf#97731 if the reserved area changed size, give the controls a
        //chance to adapt to the new size
        if (bChanged)
        {
            nX = static_cast<sal_uInt16>(aPoint.X());
            ArrangeControls(nX, static_cast<sal_uInt16>(aPoint.Y()));
        }
    }

    void EditBrowseBox::ArrangeControls(sal_uInt16&, sal_uInt16)
    {
    }

    CellController* EditBrowseBox::GetController(sal_Int32, sal_uInt16)
    {
        return nullptr;
    }

    void EditBrowseBox::ResizeController(CellControllerRef const & rController, const tools::Rectangle& rRect)
    {
        Point aPoint(rRect.TopLeft());
        Size aSize(rRect.GetSize());
        Control& rControl = rController->GetWindow();
        auto nMinHeight = rControl.get_preferred_size().Height();
        if (nMinHeight > aSize.Height())
        {
            auto nOffset = (nMinHeight - aSize.Height()) / 2;
            aPoint.AdjustY(-nOffset);
            aSize.setHeight(nMinHeight);
        }
        rControl.SetPosSizePixel(aPoint, aSize);
    }

    void EditBrowseBox::InitController(CellControllerRef&, sal_Int32, sal_uInt16)
    {
    }


    void EditBrowseBox::CellModified()
    {
    }


    bool EditBrowseBox::SaveModified()
    {
        return true;
    }


    void EditBrowseBox::DoubleClick(const BrowserMouseEvent& rEvt)
    {
        // when double clicking on the column, the optimum size will be calculated
        sal_uInt16 nColId = rEvt.GetColumnId();
        if (nColId != HandleColumnId)
            SetColumnWidth(nColId, GetAutoColumnWidth(nColId));
    }


    sal_uInt32 EditBrowseBox::GetAutoColumnWidth(sal_uInt16 nColId)
    {
        sal_uInt32 nCurColWidth  = GetColumnWidth(nColId);
        sal_uInt32 nMinColWidth = CalcZoom(20); // minimum
        sal_uInt32 nNewColWidth = nMinColWidth;
        sal_Int32 nMaxRows      = std::min(sal_Int32(GetVisibleRows()), GetRowCount());
        sal_Int32 nLastVisRow   = GetTopRow() + nMaxRows - 1;

        if (GetTopRow() <= nLastVisRow) // calc the column with using the cell contents
        {
            for (tools::Long i = GetTopRow(); i <= nLastVisRow; ++i)
                nNewColWidth = std::max(nNewColWidth,GetTotalCellWidth(i,nColId) + 12);

            if (nNewColWidth == nCurColWidth)   // size has not changed
                nNewColWidth = GetDefaultColumnWidth(GetColumnTitle(nColId));
        }
        else
            nNewColWidth = GetDefaultColumnWidth(GetColumnTitle(nColId));
        return nNewColWidth;
    }

    sal_uInt32 EditBrowseBox::GetTotalCellWidth(sal_Int32, sal_uInt16)
    {
        return 0;
    }

    void EditBrowseBox::InvalidateHandleColumn()
    {
        tools::Rectangle aHdlFieldRect( GetFieldRectPixel( 0, 0 ));
        tools::Rectangle aInvalidRect( Point(0,0), GetOutputSizePixel() );
        aInvalidRect.SetRight( aHdlFieldRect.Right() );
        Invalidate( aInvalidRect );
    }

    void EditBrowseBox::PaintTristate(const tools::Rectangle& rRect, const TriState& eState, bool _bEnabled) const
    {
        pCheckBoxPaint->SetState(eState);

        pCheckBoxPaint->GetBox().set_sensitive(_bEnabled);

        Size aBoxSize = pCheckBoxPaint->GetBox().get_preferred_size();
        tools::Rectangle aRect(Point(rRect.Left() + ((rRect.GetWidth() - aBoxSize.Width()) / 2),
                                     rRect.Top() + ((rRect.GetHeight() - aBoxSize.Height()) / 2)),
                               aBoxSize);
        pCheckBoxPaint->SetPosSizePixel(aRect.TopLeft(), aRect.GetSize());

        pCheckBoxPaint->Draw(&GetDataWindow(), aRect.TopLeft(), DrawFlags::NONE);
    }

    void EditBrowseBox::AsynchGetFocus()
    {
        if (nStartEvent)
            Application::RemoveUserEvent(nStartEvent);

        m_pFocusWhileRequest = Application::GetFocusWindow();
        nStartEvent = Application::PostUserEvent(LINK(this,EditBrowseBox,StartEditHdl), nullptr, true);
    }


    void EditBrowseBox::SetBrowserFlags(EditBrowseBoxFlags nFlags)
    {
        if (m_nBrowserFlags == nFlags)
            return;

        bool RowPicturesChanges = ((m_nBrowserFlags & EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT) !=
                                       (nFlags & EditBrowseBoxFlags::NO_HANDLE_COLUMN_CONTENT));
        m_nBrowserFlags = nFlags;

        if (RowPicturesChanges)
            InvalidateStatusCell(GetCurRow());
    }

    inline void EditBrowseBox::EnableAndShow() const
    {
        Controller()->resume();
    }


    CellController::CellController(Control* pW)
                   :pWindow( pW )
                   ,bSuspended( true )
    {

        DBG_ASSERT(pWindow, "CellController::CellController: missing the window!");
        DBG_ASSERT(!pWindow->IsVisible(), "CellController::CellController: window should not be visible!");
    }


    CellController::~CellController()
    {

    }


    void CellController::suspend( )
    {
        DBG_ASSERT( bSuspended == !GetWindow().IsVisible(), "CellController::suspend: inconsistence!" );
        if ( !isSuspended( ) )
        {
            CommitModifications();
            GetWindow().Hide( );
            GetWindow().Disable( );
            bSuspended = true;
        }
    }

    void CellController::resume( )
    {
        DBG_ASSERT( bSuspended == !GetWindow().IsVisible(), "CellController::resume: inconsistence!" );
        if ( isSuspended( ) )
        {
            GetWindow().Enable( );
            GetWindow().Show( );
            bSuspended = false;
        }
    }

    void CellController::CommitModifications()
    {
        // nothing to do in this base class
    }

    void CellController::ActivatingMouseEvent(const BrowserMouseEvent& /*rEvt*/, bool /*bUp*/)
    {
        // nothing to do in this base class
    }

    bool CellController::MoveAllowed(const KeyEvent&) const
    {
        return true;
    }

}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
