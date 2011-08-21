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

#ifndef SD_EDIT_WINDOW_HXX
#define SD_EDIT_WINDOW_HXX

#include <vcl/window.hxx>
#include <svtools/transfer.hxx>
#include <vcl/timer.hxx>
#include <editeng/editdata.hxx>
#include <svtools/colorcfg.hxx>

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

    sal_Bool                IsEmpty() const;
    sal_Bool                IsSelected() const;
    sal_Bool                IsAllSelected() const;
    void                Cut();
    void                Copy();
    void                Paste();
    void                Delete();
    void                SelectAll();
    void                MarkError(const Point &rPos);
    void                SelNextMark();
    void                SelPrevMark();
    sal_Bool                HasMark(const String &rText) const;

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );
};

} } // end of namespace ::sd::notes

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
