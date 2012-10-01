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
#ifndef EDIT_HXX
#define EDIT_HXX

#include <vcl/window.hxx>
#include <vcl/timer.hxx>
#include <svtools/transfer.hxx>
#include <editeng/editdata.hxx>
#include <svtools/colorcfg.hxx>

class SmDocShell;
class SmViewShell;
class EditView;
class EditEngine;
class EditStatus;
class ScrollBar;
class ScrollBarBox;
class DataChangedEvent;
class Menu;
class SmCmdBoxWindow;
class SmEditAccessible;
class CommandEvent;

/**************************************************************************/

    void SmGetLeftSelectionPart(const ESelection &rSelection,
                                sal_uInt16 &nPara, sal_uInt16 &nPos);

/**************************************************************************/

class SmEditWindow : public Window, public DropTargetHelper
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  xAccessible;
    SmEditAccessible *                                          pAccessible;

    SmCmdBoxWindow &rCmdBox;
    EditView       *pEditView;
    ScrollBar      *pHScrollBar,
                   *pVScrollBar;
    ScrollBarBox   *pScrollBox;
    Timer           aModifyTimer,
                    aCursorMoveTimer;
    ESelection      aOldSelection;

    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void Command(const CommandEvent& rCEvt);
    DECL_LINK(MenuSelectHdl, Menu *);
    DECL_LINK(ModifyTimerHdl, Timer *);
    DECL_LINK(CursorMoveTimerHdl, Timer *);

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
    void        InvalidateSlots();

public:
    SmEditWindow( SmCmdBoxWindow &rMyCmdBoxWin );
    ~SmEditWindow();

    SmDocShell *    GetDoc();
    SmViewShell *   GetView();
    EditView *      GetEditView()   { return pEditView; }
    EditEngine *    GetEditEngine();
    SfxItemPool *   GetEditEngineItemPool();

    // Window
    virtual void        SetText(const XubString &rText);
    virtual String      GetText() const;
    virtual void        GetFocus();
    virtual void        LoseFocus();

    ESelection          GetSelection() const;
    void                SetSelection(const ESelection &rSel);

    bool                IsEmpty() const;
    bool                IsSelected() const;
    bool                IsAllSelected() const;
    void                Cut();
    void                Copy();
    void                Paste();
    void                Delete();
    void                SelectAll();
    void                InsertText(const String &rText);
    void                InsertCommand(sal_uInt16 nCommand);
    void                MarkError(const Point &rPos);
    void                SelNextMark();
    void                SelPrevMark();
    bool                HasMark(const OUString &rText) const;

    void                Flush();
    void                DeleteEditView( SmViewShell &rView );

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

    bool                HandleWheelCommands( const CommandEvent &rCEvt );
    bool                IsInlineEditEnabled();
    void                StartCursorMove();

    // for Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    using Window::GetAccessible;
    SmEditAccessible *   GetAccessible()  { return pAccessible; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
