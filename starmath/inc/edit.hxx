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
#ifndef EDIT_HXX
#define EDIT_HXX

#include <vcl/window.hxx>
#include <vcl/timer.hxx>
#include <svtools/transfer.hxx>
#include <editeng/editdata.hxx>
#include <svtools/colorcfg.hxx>

//#ifndef _ACCESSIBILITY_HXX_
//#include "accessibility.hxx"
//#endif

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

    void SmGetLeftSelectionPart(const ESelection aSelection,
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

    sal_Bool                IsEmpty() const;
    sal_Bool                IsSelected() const;
    sal_Bool                IsAllSelected() const;
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
    sal_Bool                HasMark(const String &rText) const;

    void                Flush();
    void                DeleteEditView( SmViewShell &rView );

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

    sal_Bool                HandleWheelCommands( const CommandEvent &rCEvt );

    // for Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    using Window::GetAccessible;
    SmEditAccessible *   GetAccessible()  { return pAccessible; }
};


#endif

