/*************************************************************************
 *
 *  $RCSfile: edit.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 11:41:39 $
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
#ifndef EDIT_HXX
#define EDIT_HXX

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _MyEDITDATA_HXX //autogen
#include <svx/editdata.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

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

/**************************************************************************/

    void SmGetLeftSelectionPart(const ESelection aSelection,
                                USHORT &nPara, USHORT &nPos);

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
    void                InsertText(const String &rText);
    void                InsertCommand(USHORT nCommand);
    void                MarkError(const Point &rPos);
    void                SelNextMark();
    void                SelPrevMark();
    BOOL                HasMark(const String &rText) const;

    void                Flush();
    void                DeleteEditView( SmViewShell &rView );

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

    // for Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    SmEditAccessible *   GetAccessible()  { return pAccessible; }
};


#endif

