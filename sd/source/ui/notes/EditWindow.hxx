/*************************************************************************
 *
 *  $RCSfile: EditWindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:07:34 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

