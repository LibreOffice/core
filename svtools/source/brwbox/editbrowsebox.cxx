/*************************************************************************
 *
 *  $RCSfile: editbrowsebox.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:21:55 $
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

#ifndef _SVTOOLS_EDITBROWSEBOX_HRC_
#include "editbrowsebox.hrc"
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#ifndef _SV_SPINFLD_HXX //autogen
#include <vcl/spinfld.hxx>
#endif

#ifndef _SVTOOLS_SVTDATA_HXX
#include "svtdata.hxx"
#endif

#ifndef _SVTOOLS_HRC
#include "svtools.hrc"
#endif

#include <algorithm>

#ifndef _SV_MULTISEL_HXX
#include <tools/multisel.hxx>
#endif
#ifndef SVTOOLS_EDITBROWSEBOX_IMPL_HXX
#include "editbrowseboximpl.hxx"
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SVTOOLS_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX
#include "editbrowseboxcell.hxx"
#endif

// .......................................................................
namespace svt
{
// .......................................................................
    namespace
    {
        //..............................................................
        sal_Bool isHiContrast(Window* _pWindow)
        {
            OSL_ENSURE(_pWindow,"Window must be not null!");
            Window* pIter = _pWindow;
            //  while( pIter &&  pIter->GetBackground().GetColor().GetColor() == COL_TRANSPARENT )
            while( pIter )
            {
                const Color& aColor = pIter->GetBackground().GetColor();
                if ( aColor.GetColor() == COL_TRANSPARENT )
                    pIter = pIter->GetParent();
                else
                    break;
            }
            return pIter && pIter->GetBackground().GetColor().IsDark();
        }

        //..............................................................
        sal_uInt16 getRealGetFocusFlags( Window* _pWindow )
        {
            sal_uInt16 nFlags = 0;
            while ( _pWindow && !nFlags )
            {
                nFlags = _pWindow->GetGetFocusFlags( );
                _pWindow = _pWindow->GetParent();
            }
            return nFlags;
        }
    }

    using namespace com::sun::star::accessibility::AccessibleEventId;
    using  com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::uno::Reference;
    //==================================================================

    #define HANDLE_ID   0

    //==================================================================
    //= EditBrowserHeader
    //==================================================================
    //------------------------------------------------------------------------------
    void EditBrowserHeader::DoubleClick()
    {
        sal_uInt16 nColId = GetCurItemId();
        if (nColId)
        {
            sal_uInt32 nAutoWidth = ((EditBrowseBox*)GetParent())->GetAutoColumnWidth(nColId);
            if (nAutoWidth != ((EditBrowseBox*)GetParent())->GetColumnWidth(nColId))
            {
                ((EditBrowseBox*)GetParent())->SetColumnWidth(nColId, nAutoWidth);
                ((EditBrowseBox*)GetParent())->ColumnResized(nColId);
            }
        }
    }


    //==================================================================
    //= EditBrowseBox
    //==================================================================
    //------------------------------------------------------------------------------
    void EditBrowseBox::BrowserMouseEventPtr::Clear()
    {
        DELETEZ(pEvent);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::BrowserMouseEventPtr::Set(const BrowserMouseEvent* pEvt, sal_Bool bIsDown)
    {
        if (pEvt == pEvent)
        {
            bDown = bIsDown;
            return;
        }
        Clear();
        if (pEvt)
        {
            pEvent = new BrowserMouseEvent(pEvt->GetWindow(),
                                           *pEvt,
                                           pEvt->GetRow(),
                                           pEvt->GetColumn(),
                                           pEvt->GetColumnId(),
                                           pEvt->GetRect());
            bDown = bIsDown;
        }
    }


    DBG_NAME(EditBrowseBox);
    void EditBrowseBox::Construct()
    {
        m_aImpl = ::std::auto_ptr<EditBrowseBoxImpl>(new EditBrowseBoxImpl());
        m_aImpl->m_pFocusCell = NULL;
        m_aImpl->m_bHiContrast = isHiContrast(&GetDataWindow());

        SetCompoundControl(sal_True);
        SetLineColor(Color(COL_LIGHTGRAY));

        // HACK: the BrowseBox does not invalidate it's children (as it should be)
        // Thus we reset WB_CLIPCHILDREN, which forces the invalidation of the children
        WinBits aStyle = GetStyle();
        if( aStyle & WB_CLIPCHILDREN )
        {
            aStyle &= ~WB_CLIPCHILDREN;
            SetStyle( aStyle );
        }
        ImplInitSettings(sal_True, sal_True, sal_True);

        pCheckBoxPaint = new CheckBoxControl(&GetDataWindow());
        pCheckBoxPaint->SetPaintTransparent( sal_True );
        pCheckBoxPaint->SetBackground();
    }
    //------------------------------------------------------------------------------
    EditBrowseBox::EditBrowseBox(Window* pParent, const ResId& rId, sal_Int32 nBrowserFlags, BrowserMode _nMode )
                  :BrowseBox( pParent, rId, _nMode )
                  ,nEditRow(-1)
                  ,nPaintRow(-1)
                  ,nOldEditRow(-1)
                  ,nEditCol(0)
                  ,nOldEditCol(0)
                  ,bHasFocus(sal_False)
                  ,bPaintStatus(sal_True)
                  ,nStartEvent(0)
                  ,nEndEvent(0)
                  ,nCellModifiedEvent(0)
                  ,m_nBrowserFlags(nBrowserFlags)
    {
        DBG_CTOR(EditBrowseBox,NULL);

        Construct();
    }

    //==================================================================
    EditBrowseBox::EditBrowseBox( Window* pParent, sal_Int32 nBrowserFlags, WinBits nBits, BrowserMode _nMode )
                  :BrowseBox( pParent, nBits, _nMode )
                  ,nEditRow(-1)
                  ,nPaintRow(-1)
                  ,nOldEditRow(-1)
                  ,nEditCol(0)
                  ,nOldEditCol(0)
                  ,bHasFocus(sal_False)
                  ,bPaintStatus(sal_True)
                  ,nStartEvent(0)
                  ,nEndEvent(0)
                  ,nCellModifiedEvent(0)
                  ,pHeader(NULL)
                  ,m_nBrowserFlags(nBrowserFlags)
    {
        DBG_CTOR(EditBrowseBox,NULL);

        Construct();
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::Init()
    {
        // spaetes Construieren,
    }

    //------------------------------------------------------------------------------
    EditBrowseBox::~EditBrowseBox()
    {
        if (nStartEvent)
            Application::RemoveUserEvent(nStartEvent);
        if (nEndEvent)
            Application::RemoveUserEvent(nEndEvent);
        if (nCellModifiedEvent)
            Application::RemoveUserEvent(nCellModifiedEvent);

        delete pCheckBoxPaint;

        DBG_DTOR(EditBrowseBox,NULL);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::RemoveRows()
    {
        BrowseBox::Clear();
        nOldEditRow = nEditRow = nPaintRow = -1;
        nEditCol = nOldEditCol = 0;
    }

    //------------------------------------------------------------------------------
    BrowserHeader* EditBrowseBox::CreateHeaderBar(BrowseBox* pParent)
    {
        pHeader = imp_CreateHeaderBar(pParent);
        if (!IsUpdateMode())
            pHeader->SetUpdateMode(sal_False);
        return pHeader;
    }

    //------------------------------------------------------------------------------
    BrowserHeader* EditBrowseBox::imp_CreateHeaderBar(BrowseBox* pParent)
    {
        return new EditBrowserHeader(pParent);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::LoseFocus()
    {
        BrowseBox::LoseFocus();
        DetermineFocus( 0 );
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::GetFocus()
    {
        BrowseBox::GetFocus();

        // This should handle the case that the BrowseBox (or one of it's children)
        // gets the focus from outside by pressing Tab
        if (IsEditing() && Controller()->GetWindow().IsVisible())
            Controller()->GetWindow().GrabFocus();

        DetermineFocus( getRealGetFocusFlags( this ) );
    }

    //------------------------------------------------------------------------------
    sal_Bool EditBrowseBox::SeekRow(long nRow)
    {
        nPaintRow = nRow;
        return sal_True;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(EditBrowseBox, StartEditHdl, void*, EMPTYTAG)
    {
        nStartEvent = 0;
        if (IsEditing())
        {
            EnableAndShow();
            if (!aController->GetWindow().HasFocus() && (m_pFocusWhileRequest == Application::GetFocusWindow()))
                aController->GetWindow().GrabFocus();
        }
        return 0;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::PaintField( OutputDevice& rDev, const Rectangle& rRect,
                                    sal_uInt16 nColumnId ) const
    {
        if (nColumnId == HANDLE_ID)
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

    //------------------------------------------------------------------------------
    Image EditBrowseBox::GetImage(RowStatus eStatus) const
    {
        sal_Bool bHiContrast = isHiContrast(&GetDataWindow());
        if ( !m_aStatusImages.GetImageCount() || (bHiContrast != m_aImpl->m_bHiContrast) )
        {
            m_aImpl->m_bHiContrast = bHiContrast;
            const_cast<EditBrowseBox*>(this)->m_aStatusImages = ImageList(SvtResId(bHiContrast ? RID_SVTOOLS_IMAGELIST_EDITBWSEBOX_H : RID_SVTOOLS_IMAGELIST_EDITBROWSEBOX));
        }

        Image aImage;
        switch (eStatus)
        {
            case CURRENT:
                aImage = m_aStatusImages.GetImage(IMG_EBB_CURRENT);
                break;
            case CURRENTNEW:
                aImage = m_aStatusImages.GetImage(IMG_EBB_CURRENTNEW);
                break;
            case MODIFIED:
                aImage = m_aStatusImages.GetImage(IMG_EBB_MODIFIED);
                break;
            case NEW:
                aImage = m_aStatusImages.GetImage(IMG_EBB_NEW);
                break;
            case DELETED:
                aImage = m_aStatusImages.GetImage(IMG_EBB_DELETED);
                break;
            case PRIMARYKEY:
                aImage = m_aStatusImages.GetImage(IMG_EBB_PRIMARYKEY);
                break;
            case CURRENT_PRIMARYKEY:
                aImage = m_aStatusImages.GetImage(IMG_EBB_CURRENT_PRIMARYKEY);
                break;
            case FILTER:
                aImage = m_aStatusImages.GetImage(IMG_EBB_FILTER);
                break;
        }
        return aImage;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const
    {
        if (nPaintRow < 0)
            return;

        RowStatus eStatus = GetRowStatus( nPaintRow );
        sal_Int32 nBrowserFlags = GetBrowserFlags();

        if (nBrowserFlags & EBBF_NO_HANDLE_COLUMN_CONTENT)
            return;

        // draw the text of the header column
        if (nBrowserFlags & EBBF_HANDLE_COLUMN_TEXT )
        {
            rDev.DrawText( rRect, GetCellText( nPaintRow, 0 ),
                           TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_CLIP );
        }
        // draw an image
        else if (eStatus != CLEAN && rDev.GetOutDevType() == OUTDEV_WINDOW)
        {
            Image aImage(GetImage(eStatus));
            // calc the image position
            Size aImageSize(aImage.GetSizePixel());
            aImageSize.Width() = CalcZoom(aImageSize.Width());
            aImageSize.Height() = CalcZoom(aImageSize.Height());
            Point aPos(rRect.TopLeft());

            if (aImageSize.Width() > rRect.GetWidth() ||
                aImageSize.Height() > rRect.GetHeight())
                rDev.SetClipRegion(rRect);

            if (aImageSize.Width() < rRect.GetWidth())
                aPos.X() += (rRect.GetWidth() - aImageSize.Width()) / 2;

            if (IsZoom())
                rDev.DrawImage(aPos, aImageSize, aImage, 0);
            else
                rDev.DrawImage(aPos, aImage, 0);

            if (rDev.IsClipRegion())
                rDev.SetClipRegion();
        }
    }

    //------------------------------------------------------------------------------
    EditBrowseBox::RowStatus EditBrowseBox::GetRowStatus(long nRow) const
    {
        return CLEAN;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::KeyInput( const KeyEvent& rEvt )
    {
        sal_uInt16 nCode = rEvt.GetKeyCode().GetCode();
        sal_Bool   bShift = rEvt.GetKeyCode().IsShift();
        sal_Bool   bCtrl = rEvt.GetKeyCode().IsMod1();

        switch (nCode)
        {
            case KEY_RETURN:
                if (!bCtrl && !bShift && IsTabAllowed(sal_True))
                {
                    Dispatch(BROWSER_CURSORRIGHT);
                }
                else
                    BrowseBox::KeyInput(rEvt);
                return;
            case KEY_TAB:
                if (!bCtrl && !bShift)
                {
                    if (IsTabAllowed(sal_True))
                        Dispatch(BROWSER_CURSORRIGHT);
                    else
                        // do NOT call BrowseBox::KeyInput : this would handle the tab, but we already now
                        // that tab isn't allowed here. So give the Control class a chance
                        Control::KeyInput(rEvt);
                    return;
                }
                else if (!bCtrl && bShift)
                {
                    if (IsTabAllowed(sal_False))
                        Dispatch(BROWSER_CURSORLEFT);
                    else
                        // do NOT call BrowseBox::KeyInput : this would handle the tab, but we already now
                        // that tab isn't allowed here. So give the Control class a chance
                        Control::KeyInput(rEvt);
                    return;
                }
            default:
                BrowseBox::KeyInput(rEvt);
        }
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::MouseButtonDown(const BrowserMouseEvent& rEvt)
    {
        sal_uInt16  nColPos = GetColumnPos( rEvt.GetColumnId() );
        long    nRow    = rEvt.GetRow();

        // absorb double clicks
        if (rEvt.GetClicks() > 1 && rEvt.GetRow() >= 0)
            return;

        // change to a new position
        if (IsEditing() && (nColPos != nEditCol || nRow != nEditRow) && (nColPos != BROWSER_INVALIDID) && (nRow < GetRowCount()))
        {
            CellControllerRef aController(Controller());
            HideAndDisable(aController);
        }

        // we are about to leave the current cell. If there is a "this cell has been modified" notification
        // pending (asynchronously), this may be deadly -> do it synchronously
        // 95826 - 2002-10-14 - fs@openoffice.org
        if ( nCellModifiedEvent )
        {
            Application::RemoveUserEvent( nCellModifiedEvent );
            nCellModifiedEvent = 0;
            LINK( this, EditBrowseBox, CellModifiedHdl ).Call( NULL );
        }

        if (0 == rEvt.GetColumnId())
        {   // it was the handle column. save the current cell content if necessary
            // (clicking on the handle column results in selecting the current row)
            // 23.01.2001 - 82797 - FS
            if (IsEditing() && aController->IsModified())
                SaveModified();
        }

        aMouseEvent.Set(&rEvt,sal_True);
        BrowseBox::MouseButtonDown(rEvt);
        aMouseEvent.Clear();

        if (0 != (m_nBrowserFlags & EBBF_ACTIVATE_ON_BUTTONDOWN))
        {
            // the base class does not travel upon MouseButtonDown, but implActivateCellOnMouseEvent assumes we traveled ...
            GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
            if (rEvt.GetRow() >= 0)
                implActivateCellOnMouseEvent(rEvt, sal_False);
        }
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::MouseButtonUp( const BrowserMouseEvent& rEvt )
    {
        // unused variables.  Sideeffects?
        sal_uInt16  nColPos = GetColumnPos( rEvt.GetColumnId() );
        long    nRow = rEvt.GetRow();

        // absorb double clicks
        if (rEvt.GetClicks() > 1 && rEvt.GetRow() >= 0)
            return;

        aMouseEvent.Set(&rEvt,sal_False);
        BrowseBox::MouseButtonUp(rEvt);
        aMouseEvent.Clear();

        if (0 == (m_nBrowserFlags & EBBF_ACTIVATE_ON_BUTTONDOWN))
            if (rEvt.GetRow() >= 0)
                implActivateCellOnMouseEvent(rEvt, sal_True);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::implActivateCellOnMouseEvent(const BrowserMouseEvent& _rEvt, sal_Bool _bUp)
    {
        if (!IsEditing())
            ActivateCell();
        else if (IsEditing() && !aController->GetWindow().IsEnabled())
            DeactivateCell();
        else if (IsEditing() && !aController->GetWindow().HasChildPathFocus())
            AsynchGetFocus();

        if (IsEditing() && aController->GetWindow().IsEnabled() && aController->WantMouseEvent())
        {   // forwards the event to the control

            // If the field has been moved previously, we have to adjust the position

            aController->GetWindow().GrabFocus();

            // the position of the event relative to the controller's window
            Point aPos = _rEvt.GetPosPixel() - _rEvt.GetRect().TopLeft();
            // the (child) window which should really get the event
            Window* pRealHandler = aController->GetWindow().FindWindow(aPos);
            if (pRealHandler)
                // the coords relative to this real handler
                aPos -= pRealHandler->GetPosPixel();
            else
                pRealHandler = &aController->GetWindow();

            // the faked event
            MouseEvent aEvent(aPos, _rEvt.GetClicks(), _rEvt.GetMode(),
                              _rEvt.GetButtons(),
                              _rEvt.GetModifier());

            pRealHandler->MouseButtonDown(aEvent);
            if (_bUp)
                pRealHandler->MouseButtonUp(aEvent);

            Window *pWin = &aController->GetWindow();
            if (!pWin->IsTracking())
            {
                for (pWin = pWin->GetWindow(WINDOW_FIRSTCHILD);
                     pWin && !pWin->IsTracking();
                     pWin = pWin->GetWindow(WINDOW_NEXT))
                {
                }
            }
            if (pWin && pWin->IsTracking())
                pWin->EndTracking();
        }
    }

    //------------------------------------------------------------------------------
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
                    SelectColumnPos( FirstSelectedColumn(), sal_False );
                Select();
            }
        }
        BrowseBox::Dispatch( _nId );
    }

    //------------------------------------------------------------------------------
    long EditBrowseBox::PreNotify(NotifyEvent& rEvt)
    {
        switch (rEvt.GetType())
        {
            case EVENT_KEYINPUT:
                if  (   (IsEditing() && Controller()->GetWindow().HasChildPathFocus())
                    ||  rEvt.GetWindow() == &GetDataWindow()
                    ||  (!IsEditing() && HasChildPathFocus())
                    )
                {
                    const KeyEvent* pKeyEvent = rEvt.GetKeyEvent();
                    sal_uInt16 nCode  = pKeyEvent->GetKeyCode().GetCode();
                    sal_Bool   bShift = pKeyEvent->GetKeyCode().IsShift();
                    sal_Bool   bCtrl  = pKeyEvent->GetKeyCode().IsMod1();
                    sal_Bool   bAlt =   pKeyEvent->GetKeyCode().IsMod2();
                    sal_Bool   bSelect= sal_False;
                    sal_Bool   bNonEditOnly =   sal_False;
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
                                if (IsTabAllowed(sal_True))
                                    nId = BROWSER_CURSORRIGHT;
                                break;

                            case KEY_RETURN:
                                // save the cell content (if necessary)
                                if (IsEditing() && aController->IsModified() && !((EditBrowseBox *) this)->SaveModified())
                                {
                                    // maybe we're not visible ...
                                    EnableAndShow();
                                    aController->GetWindow().GrabFocus();
                                    return 1;
                                }
                                // ask if traveling to the next cell is allowed
                                if (IsTabAllowed(sal_True))
                                    nId = BROWSER_CURSORRIGHT;

                                break;
                            case KEY_RIGHT:         nId = BROWSER_CURSORRIGHT; break;
                            case KEY_LEFT:          nId = BROWSER_CURSORLEFT; break;
                            case KEY_SPACE:         nId = BROWSER_SELECT; bNonEditOnly = bSelect = sal_True;break;
                        }

                    if ( !bAlt && !bCtrl && bShift )
                        switch ( nCode )
                        {
                            case KEY_DOWN:          nId = BROWSER_SELECTDOWN; bSelect = sal_True;break;
                            case KEY_UP:            nId = BROWSER_SELECTUP; bSelect = sal_True;break;
                            case KEY_HOME:          nId = BROWSER_SELECTHOME; bSelect = sal_True;break;
                            case KEY_END:           nId = BROWSER_SELECTEND; bSelect = sal_True;break;
                            case KEY_SPACE:         nId = BROWSER_SELECTCOLUMN; bSelect = sal_True; break;
                            case KEY_TAB:
                                if (IsTabAllowed(sal_False))
                                    nId = BROWSER_CURSORLEFT;
                                break;
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
                            case KEY_SPACE:         nId = BROWSER_ENHANCESELECTION; bSelect = sal_True;break;
                        }


                    if  (   ( nId != BROWSER_NONE )
                        &&  (   !IsEditing()
                            ||  (   !bNonEditOnly
                                &&  aController->MoveAllowed( *pKeyEvent )
                                )
                            )
                        )
                    {
                        if (nId == BROWSER_SELECT)
                        {
                            // save the cell content (if necessary)
                            if (IsEditing() && aController->IsModified() && !((EditBrowseBox *) this)->SaveModified())
                            {
                                // maybe we're not visible ...
                                EnableAndShow();
                                aController->GetWindow().GrabFocus();
                                return 1;
                            }
                        }

                        Dispatch(nId);

                        if (bSelect && (GetSelectRowCount() || GetSelection() != NULL))
                            DeactivateCell();
                        return 1;
                    }
                }
        }
        return BrowseBox::PreNotify(rEvt);
    }

    //------------------------------------------------------------------------------
    sal_Bool EditBrowseBox::IsTabAllowed(sal_Bool bRight) const
    {
        return sal_True;
    }

    //------------------------------------------------------------------------------
    long EditBrowseBox::Notify(NotifyEvent& rEvt)
    {
        switch (rEvt.GetType())
        {
            case EVENT_GETFOCUS:
                DetermineFocus( getRealGetFocusFlags( this ) );
                break;

            case EVENT_LOSEFOCUS:
                DetermineFocus( 0 );
                break;
        }
        return BrowseBox::Notify(rEvt);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::StateChanged( StateChangedType nType )
    {
        BrowseBox::StateChanged( nType );

        if ( nType == STATE_CHANGE_ZOOM )
        {
            ImplInitSettings( sal_True, sal_False, sal_False );
            if (IsEditing())
            {
                DeactivateCell();
                ActivateCell();
            }
        }
        else if ( nType == STATE_CHANGE_CONTROLFONT )
        {
            ImplInitSettings( sal_True, sal_False, sal_False );
            Invalidate();
        }
        else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        {
            ImplInitSettings( sal_False, sal_True, sal_False );
            Invalidate();
        }
        else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        {
            ImplInitSettings( sal_False, sal_False, sal_True );
            Invalidate();
        }
        else if (nType == STATE_CHANGE_STYLE)
        {
            WinBits nStyle = GetStyle();
            if (!(nStyle & WB_NOTABSTOP) )
                nStyle |= WB_TABSTOP;

            SetStyle(nStyle);
        }
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::DataChanged( const DataChangedEvent& rDCEvt )
    {
        BrowseBox::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            ImplInitSettings( sal_True, sal_True, sal_True );
            Invalidate();
        }
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        if (bFont)
        {
            Font aFont = rStyleSettings.GetFieldFont();
            if (IsControlFont())
            {
                GetDataWindow().SetControlFont(GetControlFont());
                aFont.Merge(GetControlFont());
            }
            else
                GetDataWindow().SetControlFont();

            GetDataWindow().SetZoomedPointFont(aFont);
        }

        if ( bFont || bForeground )
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

        if ( bBackground )
        {
            if (GetDataWindow().IsControlBackground())
            {
                GetDataWindow().SetControlBackground(GetControlBackground());
                GetDataWindow().SetBackground(GetDataWindow().GetControlBackground());
                GetDataWindow().SetFillColor(GetDataWindow().GetControlBackground());
            }
            else
            {
                GetDataWindow().SetControlBackground();
                GetDataWindow().SetBackground( rStyleSettings.GetFieldColor() );
                GetDataWindow().SetFillColor( rStyleSettings.GetFieldColor() );
            }
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool EditBrowseBox::IsCursorMoveAllowed(long nNewRow, sal_uInt16 nNewColId) const
    {
        sal_uInt16 nInfo = 0;

        if (GetSelectColumnCount() || (aMouseEvent.Is() && aMouseEvent->GetRow() < 0))
            nInfo |= COLSELECT;
        if ((GetSelection() != NULL && GetSelectRowCount()) ||
            (aMouseEvent.Is() && aMouseEvent->GetColumnId() == HANDLE_ID))
            nInfo |= ROWSELECT;
        if (!nInfo && nNewRow != nEditRow)
            nInfo |= ROWCHANGE;
        if (!nInfo && nNewColId != nEditCol)
            nInfo |= COLCHANGE;

        if (nInfo == 0)   // nothing happened
            return sal_True;

        // save the cell content
        if (IsEditing() && aController->IsModified() && !((EditBrowseBox *) this)->SaveModified())
        {
            // maybe we're not visible ...
            EnableAndShow();
            aController->GetWindow().GrabFocus();
            return sal_False;
        }

        EditBrowseBox * pTHIS = (EditBrowseBox *) this;

        // save the cell content if
        // a) a selection is beeing made
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
            return sal_False;
        }

        if (nNewRow != nEditRow)
        {
            Window& rWindow = GetDataWindow();
            // don't paint too much
            // update the status immediatly if possible
            if ((nEditRow >= 0) && (GetBrowserFlags() & EBBF_NO_HANDLE_COLUMN_CONTENT) == 0)
            {
                Rectangle aRect = GetFieldRectPixel(nEditRow, 0, sal_False );
                // status cell should be painted if and only if text is displayed
                // note: bPaintStatus is mutable, but Solaris has problems with assigning
                // probably because it is part of a bitfield
                pTHIS->bPaintStatus = static_cast< sal_Bool >
                    (( GetBrowserFlags() & EBBF_HANDLE_COLUMN_TEXT ) == EBBF_HANDLE_COLUMN_TEXT );
                rWindow.Paint(aRect);
                pTHIS->bPaintStatus = sal_True;
            }

            // don't paint during row change
            rWindow.EnablePaint(sal_False);

            // the last veto chance for derived classes
            if (!pTHIS->CursorMoving(nNewRow, nNewColId))
            {
                pTHIS->InvalidateStatusCell(nEditRow);
                rWindow.EnablePaint(sal_True);
                return sal_False;
            }
            else
            {
                rWindow.EnablePaint(sal_True);
                return sal_True;
            }
        }
        else
            return pTHIS->CursorMoving(nNewRow, nNewColId);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::ColumnMoved(sal_uInt16 nId)
    {
        BrowseBox::ColumnMoved(nId);
        if (IsEditing())
        {
            Rectangle aRect( GetCellRect(nEditRow, nEditCol, sal_False));
            CellControllerRef aControllerRef = Controller();
            ResizeController(aControllerRef, aRect);
            Controller()->GetWindow().GrabFocus();
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool EditBrowseBox::SaveRow()
    {
        return sal_True;
    }

    //------------------------------------------------------------------------------
    sal_Bool EditBrowseBox::CursorMoving(long nNewRow, sal_uInt16 nNewCol)
    {
        ((EditBrowseBox *) this)->DeactivateCell(sal_False);
        return sal_True;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::CursorMoved()
    {
        long nNewRow = GetCurRow();
        if (nEditRow != nNewRow)
        {
            if ((GetBrowserFlags() & EBBF_NO_HANDLE_COLUMN_CONTENT) == 0)
                InvalidateStatusCell(nNewRow);
            nEditRow = nNewRow;
        }
        ActivateCell();
        GetDataWindow().EnablePaint(sal_True);
        // should not be called here because the descant event is not needed here
        //BrowseBox::CursorMoved();
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::EndScroll()
    {
        if (IsEditing())
        {
            Rectangle aRect = GetCellRect(nEditRow, nEditCol, sal_False);
            ResizeController(aController,aRect);
            AsynchGetFocus();
        }
        BrowseBox::EndScroll();
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::ActivateCell(long nRow, sal_uInt16 nCol, sal_Bool bCellFocus)
    {
        if (IsEditing())
            return;

        nEditCol = nCol;

        if ((GetSelectRowCount() && GetSelection() != NULL) || GetSelectColumnCount() ||
            (aMouseEvent.Is() && (aMouseEvent.IsDown() || aMouseEvent->GetClicks() > 1))) // bei MouseDown passiert noch nichts
        {
            return;
        }

        if (nEditRow >= 0 && nEditCol > HANDLE_ID)
        {
            aController = GetController(nRow, nCol);
            if (aController.Is())
            {
                Rectangle aRect( GetCellRect(nEditRow, nEditCol, sal_False));
                ResizeController(aController, aRect);

                InitController(aController, nEditRow, nEditCol);

                aController->ClearModified();
                aController->SetModifyHdl(LINK(this,EditBrowseBox,ModifyHdl));
                EnableAndShow();

                // activate the cell only of the browser has the focus
                if ( bHasFocus && bCellFocus )
                {
                    CreateAccessibleControl(0);
                    AsynchGetFocus();
                }
            }
            else if ( isAccessibleCreated() && HasFocus() )
                commitTableEvent(ACTIVE_DESCENDANT_CHANGED,
                                 com::sun::star::uno::makeAny(CreateAccessibleCell(nRow,nCol)),
                                 com::sun::star::uno::Any());
        }
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::DeactivateCell(sal_Bool bUpdate)
    {
        if (IsEditing())
        {
            commitBrowseBoxEvent(CHILD,com::sun::star::uno::Any(),com::sun::star::uno::makeAny(m_aImpl->m_xActiveCell));
            m_aImpl->disposeCell();

            m_aImpl->m_pFocusCell  = NULL;
            m_aImpl->m_xActiveCell = NULL;

            aOldController = aController;
            aController.Clear();

            // reset the modify handler
            aOldController->SetModifyHdl(Link());

            if (bHasFocus)
                GrabFocus(); // ensure that we have (and keep) the focus

            HideAndDisable(aOldController);

            // update if requested
            if (bUpdate)
                Update();

            nOldEditCol = nEditCol;
            nOldEditRow = nEditRow;

            // release the controller (asynchronously)
            if (nEndEvent)
                Application::RemoveUserEvent(nEndEvent);
            nEndEvent = Application::PostUserEvent(LINK(this,EditBrowseBox,EndEditHdl));
        }
    }

    //------------------------------------------------------------------------------
    Rectangle EditBrowseBox::GetCellRect(long nRow, sal_uInt16 nColId, sal_Bool bRel) const
    {
        Rectangle aRect( GetFieldRectPixel(nRow, nColId, bRel));
        if ((GetMode()  & BROWSER_CURSOR_WO_FOCUS) == BROWSER_CURSOR_WO_FOCUS)
        {
            aRect.Top() += 1;
            aRect.Bottom() -= 1;
        }
        return aRect;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(EditBrowseBox, EndEditHdl, void*, EMPTYTAG)
    {
        nEndEvent = 0;
        ReleaseController(aOldController, nOldEditRow, nOldEditCol);

        aOldController  = CellControllerRef();
        nOldEditRow     = -1;
        nOldEditCol     =  0;

        return 0;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(EditBrowseBox, ModifyHdl, void*, EMPTYTAG)
    {
        if (nCellModifiedEvent)
            Application::RemoveUserEvent(nCellModifiedEvent);
        nCellModifiedEvent = Application::PostUserEvent(LINK(this,EditBrowseBox,CellModifiedHdl));
        return 0;
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(EditBrowseBox, CellModifiedHdl, void*, EMPTYTAG)
    {
        nCellModifiedEvent = 0;
        CellModified();
        return 0;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::ColumnResized( sal_uInt16 nColId )
    {
        if (IsEditing())
        {
            Rectangle aRect( GetCellRect(nEditRow, nEditCol, sal_False));
            CellControllerRef aControllerRef = Controller();
            ResizeController(aControllerRef, aRect);
            Controller()->GetWindow().GrabFocus();
        }
    }

    //------------------------------------------------------------------------------
    sal_uInt16 EditBrowseBox::GetDefaultColumnWidth(const String& rName) const
    {
        return GetDataWindow().GetTextWidth(rName) + GetDataWindow().GetTextWidth('0') * 4;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::InsertHandleColumn(sal_uInt16 nWidth)
    {
        if (!nWidth)
            nWidth = GetDefaultColumnWidth(String());

        BrowseBox::InsertHandleColumn(nWidth, sal_True);
    }

    //------------------------------------------------------------------------------
    sal_uInt16 EditBrowseBox::AppendColumn(const String& rName, sal_uInt16 nWidth, sal_uInt16 nPos, sal_uInt16 nId)
    {
        if (nId == (sal_uInt16)-1)
        {
            // look for the next free id
            for (nId = ColCount(); nId > 0 && GetColumnPos(nId) != BROWSER_INVALIDID; nId--)
                ;

            if (!nId)
            {
                // if there is no handle column
                // increment the id
                if (!ColCount() || GetColumnId(0))
                    nId = ColCount() + 1;
            }
        }

        DBG_ASSERT(nId, "EditBrowseBox::AppendColumn: invalid id!");

        if (!nWidth)
            nWidth = GetDefaultColumnWidth(rName);

        InsertDataColumn(nId, rName, nWidth, (HIB_CENTER | HIB_VCENTER | HIB_CLICKABLE), nPos);
        return nId;
    }

    //------------------------------------------------------------------------------
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
        sal_uInt16 nX = (sal_uInt16)aPoint.X();

        ArrangeControls(nX, (sal_uInt16)aPoint.Y());

        if (!nX)
            nX = 0;
        ReserveControlArea((sal_uInt16)nX);
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::ArrangeControls(sal_uInt16& nX, sal_uInt16 nY)
    {
    }

    //------------------------------------------------------------------------------
    CellController* EditBrowseBox::GetController(long, sal_uInt16)
    {
        return NULL;
    }

    //-----------------------------------------------------------------------------
    void EditBrowseBox::ResizeController(CellControllerRef& rController, const Rectangle& rRect)
    {
        rController->GetWindow().SetPosSizePixel(rRect.TopLeft(), rRect.GetSize());
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::InitController(CellControllerRef& rController, long nRow, sal_uInt16 nCol)
    {
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::ReleaseController(CellControllerRef& rController, long nRow, sal_uInt16 nCol)
    {
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::CellModified()
    {
    }


    //------------------------------------------------------------------------------
    sal_Bool EditBrowseBox::SaveModified()
    {
        return sal_True;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::DoubleClick(const BrowserMouseEvent& rEvt)
    {
        // when double clicking on the column, the optimum size will be calculated
        sal_uInt16 nColId = rEvt.GetColumnId();
        if (nColId != HANDLE_ID)
            SetColumnWidth(nColId, GetAutoColumnWidth(nColId));
    }

    //------------------------------------------------------------------------------
    sal_uInt32 EditBrowseBox::GetAutoColumnWidth(sal_uInt16 nColId)
    {
        sal_uInt32 nCurColWidth  = GetColumnWidth(nColId);
        sal_uInt32 nMinColWidth = CalcZoom(20); // minimum
        sal_uInt32 nNewColWidth = nMinColWidth;
        long nMaxRows      = Min(long(GetVisibleRows()), GetRowCount());
        long nLastVisRow   = GetTopRow() + nMaxRows - 1;

        if (GetTopRow() <= nLastVisRow) // calc the column with using the cell contents
        {
            for (long i = GetTopRow(); i <= nLastVisRow; ++i)
                nNewColWidth = std::max(nNewColWidth,GetTotalCellWidth(i,nColId) + 12);

            if (nNewColWidth == nCurColWidth)   // size has not changed
                nNewColWidth = GetDefaultColumnWidth(GetColumnTitle(nColId));
        }
        else
            nNewColWidth = GetDefaultColumnWidth(GetColumnTitle(nColId));
        return nNewColWidth;
    }

    //------------------------------------------------------------------------------
    sal_uInt32 EditBrowseBox::GetTotalCellWidth(long nRow, sal_uInt16 nColId)
    {
        return 0;
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::InvalidateHandleColumn()
    {
        Rectangle aHdlFieldRect( GetFieldRectPixel( 0, 0 ));
        Rectangle aInvalidRect( Point(0,0), GetOutputSizePixel() );
        aInvalidRect.Right() = aHdlFieldRect.Right();
        Invalidate( aInvalidRect );
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::PaintTristate(OutputDevice& rDev, const Rectangle& rRect,const TriState& eState,sal_Bool _bEnabled) const
    {
        pCheckBoxPaint->GetBox().SetState(eState);
        pCheckBoxPaint->SetPosSizePixel(rRect.TopLeft(), rRect.GetSize());

        // First update the parent, preventing that while painting this window
        // an update for the parent is done (because it's in the queue already)
        // which may lead to hiding this window immediately
// #95598# comment out OJ
/*      if (pCheckBoxPaint->GetParent())
            pCheckBoxPaint->GetParent()->Update();
*/
        pCheckBoxPaint->GetBox().Enable(_bEnabled);
        pCheckBoxPaint->Show();
        pCheckBoxPaint->SetParentUpdateMode( sal_False );
        pCheckBoxPaint->Update();
        pCheckBoxPaint->Hide();
        pCheckBoxPaint->SetParentUpdateMode( sal_True );
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::AsynchGetFocus()
    {
        if (nStartEvent)
            Application::RemoveUserEvent(nStartEvent);

        m_pFocusWhileRequest = Application::GetFocusWindow();
        nStartEvent = Application::PostUserEvent(LINK(this,EditBrowseBox,StartEditHdl));
    }

    //------------------------------------------------------------------------------
    void EditBrowseBox::SetBrowserFlags(sal_Int32 nFlags)
    {
        if (m_nBrowserFlags == nFlags)
            return;

        sal_Bool RowPicturesChanges = ((m_nBrowserFlags & EBBF_NO_HANDLE_COLUMN_CONTENT) !=
                                       (nFlags & EBBF_NO_HANDLE_COLUMN_CONTENT));
        m_nBrowserFlags = nFlags;

        if (RowPicturesChanges)
            InvalidateStatusCell(GetCurRow());
    }
    //------------------------------------------------------------------------------
    inline void EditBrowseBox::HideAndDisable(CellControllerRef& rController)
    {
        rController->suspend();
    }
    //------------------------------------------------------------------------------
    inline void EditBrowseBox::EnableAndShow() const
    {
        Controller()->resume();
    }
    //===============================================================================

    DBG_NAME(CellController);
    //------------------------------------------------------------------------------
    CellController::CellController(Control* pW)
                   :pWindow( pW )
                   ,bSuspended( sal_True )
    {
        DBG_CTOR(CellController,NULL);

        DBG_ASSERT(pWindow, "CellController::CellController: missing the window!");
        DBG_ASSERT(!pWindow->IsVisible(), "CellController::CellController: window should not be visible!");
    }

    //-----------------------------------------------------------------------------
    CellController::~CellController()
    {

        DBG_DTOR(CellController,NULL);
    }

    //-----------------------------------------------------------------------------
    void CellController::suspend( )
    {
        DBG_ASSERT( bSuspended == !GetWindow().IsVisible(), "CellController::suspend: inconsistence!" );
        if ( !isSuspended( ) )
        {
            CommitModifications();
            GetWindow().Hide( );
            GetWindow().Disable( );
            bSuspended = sal_True;
        }
    }

    //-----------------------------------------------------------------------------
    void CellController::resume( )
    {
        DBG_ASSERT( bSuspended == !GetWindow().IsVisible(), "CellController::resume: inconsistence!" );
        if ( isSuspended( ) )
        {
            GetWindow().Enable( );
            GetWindow().Show( );
            bSuspended = sal_False;
        }
    }

    //-----------------------------------------------------------------------------
    void CellController::CommitModifications()
    {
        // nothing to do in this base class
    }

    //-----------------------------------------------------------------------------
    sal_Bool CellController::WantMouseEvent() const
    {
        return sal_False;
    }

    //-----------------------------------------------------------------------------
    void CellController::SetModified()
    {
    }

    //-----------------------------------------------------------------------------
    sal_Bool CellController::MoveAllowed(const KeyEvent& rEvt) const
    {
        return sal_True;
    }

// .......................................................................
}   // namespace svt
// .......................................................................

