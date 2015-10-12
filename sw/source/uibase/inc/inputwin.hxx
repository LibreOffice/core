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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_INPUTWIN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_INPUTWIN_HXX

#include <vcl/edit.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include <sfx2/childwin.hxx>

class SwFieldMgr;
class SwWrtShell;
class SwView;
class SfxDispatcher;

class InputEdit : public Edit
{
public:
                    InputEdit(vcl::Window* pParent, WinBits nStyle) :
                        Edit(pParent , nStyle){}

    void            UpdateRange(const OUString& aSel,
                                const OUString& aTableName );

protected:
    virtual void    KeyInput( const KeyEvent&  ) override;
};

class SwInputWindow : public ToolBox
{
friend class InputEdit;

    VclPtr<Edit>        aPos;
    VclPtr<InputEdit>   aEdit;
    PopupMenu       aPopMenu;
    SwFieldMgr*       pMgr;
    SwWrtShell*     pWrtShell;
    SwView*         pView;
    OUString        aAktTableName, sOldFormula;

    bool            bFirst : 1;  // initialisations at first call
    bool            bActive : 1; // for hide/show when switching documents
    bool            bIsTable : 1;
    bool            bDelSel : 1;
    bool            m_bDoesUndo : 1;
    bool            m_bResetUndo : 1;
    bool            m_bCallUndo : 1;

    void CleanupUglyHackWithUndo();

    void DelBoxContent();
    DECL_LINK( ModifyHdl, void* );

    using Window::IsActive;

protected:
    virtual void    Resize() override;
    virtual void    Click() override;
    DECL_LINK_TYPED( MenuHdl, Menu *, bool );
    DECL_LINK_TYPED( DropdownClickHdl, ToolBox*, void );
    void            ApplyFormula();
    void            CancelFormula();

public:
                    SwInputWindow( vcl::Window* pParent );
    virtual         ~SwInputWindow();
    virtual void    dispose() override;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            ShowWin();

    DECL_LINK_TYPED( SelTableCellsNotify, SwWrtShell&, void );

    void            SetFormula( const OUString& rFormula, bool bDelSel = true );
    const SwView*   GetView() const{return pView;}
};

class SwInputChild : public SfxChildWindow
{
    SfxDispatcher*  pDispatch;
public:
    SwInputChild( vcl::Window* ,
                        sal_uInt16 nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    virtual ~SwInputChild();
    SFX_DECL_CHILDWINDOW_WITHID( SwInputChild );
    void            SetFormula( const OUString& rFormula, bool bDelSel = true )
                    { static_cast<SwInputWindow*>(GetWindow())->SetFormula( rFormula, bDelSel ); }
    const SwView*   GetView() const
                    { return static_cast<SwInputWindow*>(GetWindow())->GetView();}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
