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

#include <vcl/edit.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include <sfx2/childwin.hxx>

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

    sal_Bool        bFirst : 1;  // initialisations at first call
    sal_Bool        bActive : 1; // for hide/show when switching documents
    sal_Bool        bIsTable : 1;
    sal_Bool        bDelSel : 1;
    bool            m_bDoesUndo : 1;
    bool            m_bResetUndo : 1;
    bool            m_bCallUndo : 1;

    void CleanupUglyHackWithUndo();

    void DelBoxCntnt();
    DECL_LINK( ModifyHdl, void* );

    using Window::IsActive;

protected:
    virtual void    Resize();
    virtual void    Click();
    DECL_LINK( MenuHdl, Menu * );
    DECL_LINK( DropdownClickHdl, void* );
    void            ApplyFormula();
    void            CancelFormula();

public:
                    SwInputWindow( Window* pParent, SfxBindings* pBindings );
    virtual         ~SwInputWindow();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SelectHdl( ToolBox*);

    void            ShowWin();

    sal_Bool            IsActive(){ return bActive; };

    DECL_LINK( SelTblCellsNotify, SwWrtShell * );

    void            SetFormula( const String& rFormula, sal_Bool bDelSel = sal_True );
    const SwView*   GetView() const{return pView;}
};

class SwInputChild : public SfxChildWindow
{
    SfxDispatcher*  pDispatch;
public:
    SwInputChild( Window* ,
                        sal_uInt16 nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    ~SwInputChild();
    SFX_DECL_CHILDWINDOW_WITHID( SwInputChild );
    void            SetFormula( const String& rFormula, sal_Bool bDelSel = sal_True )
                        { ((SwInputWindow*)pWindow)->SetFormula(
                                    rFormula, bDelSel ); }
    const SwView*   GetView() const{return ((SwInputWindow*)pWindow)->GetView();}

};

//==================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
