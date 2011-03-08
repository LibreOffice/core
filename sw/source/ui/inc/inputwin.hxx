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
#ifndef SW_INPUTWIN_HXX
#define SW_INPUTWIN_HXX


#include <vcl/menu.hxx>
#include <sfx2/childwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>

class SwFldMgr;
class SwWrtShell;
class SwView;
class SfxDispatcher;

//========================================================================
class InputEdit : public Edit
{
public:
                    InputEdit(Window* pParent, WinBits nStyle) :
                        Edit(pParent , nStyle){}

    void            UpdateRange(const String& aSel,
                                const String& aTblName );

protected:
    virtual void    KeyInput( const KeyEvent&  );
};

//========================================================================

class SwInputWindow : public ToolBox
{
friend class InputEdit;

    Edit            aPos;
    InputEdit       aEdit;
    PopupMenu       aPopMenu;
    SwFldMgr*       pMgr;
    SwWrtShell*     pWrtShell;
    SwView*         pView;
    SfxBindings*    pBindings;
    String          aAktTableName, sOldFml;
    USHORT          nActionCnt;

    BOOL            bFirst : 1;  //Initialisierungen beim ersten Aufruf
    BOOL            bActive : 1; //fuer Hide/Show beim Dokumentwechsel
    BOOL            bIsTable : 1;
    BOOL            bDelSel : 1;
    BOOL            bDoesUndo : 1;
    BOOL            bResetUndo : 1;
    BOOL            bCallUndo : 1;


    void DelBoxCntnt();
    DECL_LINK( ModifyHdl, InputEdit* );

    using Window::IsActive;

protected:
    virtual void    Resize();
    virtual void    Click();
    DECL_LINK( MenuHdl, Menu * );
    DECL_LINK( DropdownClickHdl, ToolBox* );
    void            ApplyFormula();
    void            CancelFormula();

public:
                    SwInputWindow( Window* pParent, SfxBindings* pBindings );
    virtual         ~SwInputWindow();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SelectHdl( ToolBox*);

    void            ShowWin();

    BOOL            IsActive(){ return bActive; };

    DECL_LINK( SelTblCellsNotify, SwWrtShell * );

    void            SetFormula( const String& rFormula, BOOL bDelSel = TRUE );
    const SwView*   GetView() const{return pView;}
};

class SwInputChild : public SfxChildWindow
{
    BOOL            bObjVis;
    SfxDispatcher*  pDispatch;
public:
    SwInputChild( Window* ,
                        USHORT nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    ~SwInputChild();
    SFX_DECL_CHILDWINDOW( SwInputChild );
    void            SetFormula( const String& rFormula, BOOL bDelSel = TRUE )
                        { ((SwInputWindow*)pWindow)->SetFormula(
                                    rFormula, bDelSel ); }
    const SwView*   GetView() const{return ((SwInputWindow*)pWindow)->GetView();}

};

//==================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
