/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EditWindow.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:00:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_EDIT_WINDOW_HXX
#define SD_EDIT_WINDOW_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _MyEDITDATA_HXX
#include <svx/editdata.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

class EditEngine;
class EditStatus;
class EditView;
class Menu;
class ScrollBar;
class ScrollBarBox;
class SfxItemPool;
class Timer;


namespace sd { namespace notes {

class EditWindow
    : public Window,
      public DropTargetHelper
{
public:
    EditWindow (Window* pParentWindow, SfxItemPool* pItemPool);
    ~EditWindow (void);

    void InsertText (const String &rText);

    using Window::GetText;
private:
    EditView* mpEditView;
    EditEngine* mpEditEngine;
    SfxItemPool* mpEditEngineItemPool;
    ScrollBar* mpHorizontalScrollBar;
    ScrollBar* mpVerticalScrollBar;
    ScrollBarBox* mpScrollBox;
    Timer maModifyTimer;
    Timer maCursorMoveTimer;
    ESelection maOldSelection;

    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void Command(const CommandEvent& rCEvt);
    DECL_LINK(MenuSelectHdl, Menu *);

    virtual void DataChanged( const DataChangedEvent& );
    virtual void Resize();
    virtual void MouseMove(const MouseEvent &rEvt);
    virtual void MouseButtonUp(const MouseEvent &rEvt);
    virtual void MouseButtonDown(const MouseEvent &rEvt);

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void Paint(const Rectangle& rRect);

    DECL_LINK(EditStatusHdl ,EditStatus *);
    DECL_LINK(ScrollHdl, ScrollBar *);

    void        CreateEditView();

    Rectangle   AdjustScrollBars();
    void        SetScrollBarRanges();
    void        InitScrollBars();

    //    SmDocShell *    GetDoc();
    //    SmViewShell *   GetView();
    EditView* GetEditView (void);
    EditEngine* GetEditEngine (void);
    EditEngine* CreateEditEngine (void);

    // Window
    virtual void        SetText(const XubString &rText);
    virtual XubString   GetText();
    virtual void        GetFocus();
    virtual void        LoseFocus();

    ESelection          GetSelection() const;
    void                SetSelection(const ESelection &rSel);

    BOOL                IsEmpty() const;
    BOOL                IsSelected() const;
    BOOL                IsAllSelected() const;
    void                Cut();
    void                Copy();
    void                Paste();
    void                Delete();
    void                SelectAll();
    void                MarkError(const Point &rPos);
    void                SelNextMark();
    void                SelPrevMark();
    BOOL                HasMark(const String &rText) const;

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );
};

} } // end of namespace ::sd::notes

#endif

