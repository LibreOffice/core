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

#include <sfx2/InterimItemWindow.hxx>
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

class PosEdit final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::Entry> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
public:
    PosEdit(vcl::Window* pParent)
        : InterimItemWindow(pParent, "modules/swriter/ui/poseditbox.ui", "PosEditBox")
        , m_xWidget(m_xBuilder->weld_entry("entry"))
    {
        m_xWidget->connect_key_press(LINK(this, PosEdit, KeyInputHdl));
        SetSizePixel(m_xWidget->get_preferred_size());
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    void set_text(const OUString& rText)
    {
        m_xWidget->set_text(rText);
    }

    void set_accessible_name(const OUString& rName)
    {
        m_xWidget->set_accessible_name(rName);
    }

    virtual ~PosEdit() override
    {
        disposeOnce();
    }
};

class SwInputWindow final : public ToolBox
{
friend class InputEdit;

    VclPtr<PosEdit> mxPos;
    VclPtr<InputEdit> aEdit;
    std::unique_ptr<SwFieldMgr> pMgr;
    SwWrtShell*     pWrtShell;
    SwView*         pView;
    OUString        aCurrentTableName, sOldFormula;

    bool            bFirst : 1;  // initialisations at first call
    bool            bIsTable : 1;
    bool            bDelSel : 1;
    bool            m_bDoesUndo : 1;
    bool            m_bResetUndo : 1;
    bool            m_bCallUndo : 1;

    void CleanupUglyHackWithUndo();

    void DelBoxContent();
    DECL_LINK( ModifyHdl, Edit&, void );

    using Window::IsActive;

    virtual void    Resize() override;
    virtual void    Click() override;
    DECL_LINK( MenuHdl, Menu *, bool );
    DECL_LINK( DropdownClickHdl, ToolBox*, void );
    void            ApplyFormula();
    void            CancelFormula();

public:
    SwInputWindow(vcl::Window* pParent, SfxDispatcher const * pDispatcher);
    virtual         ~SwInputWindow() override;
    virtual void    dispose() override;

    void            ShowWin();

    DECL_LINK( SelTableCellsNotify, SwWrtShell&, void );

    void            SetFormula( const OUString& rFormula );
    const SwView*   GetView() const{return pView;}
};

class SwInputChild : public SfxChildWindow
{
    SfxDispatcher*  pDispatch;
public:
    SwInputChild( vcl::Window* ,
                        sal_uInt16 nId,
                        SfxBindings const *,
                        SfxChildWinInfo*  );
    virtual ~SwInputChild() override;
    SFX_DECL_CHILDWINDOW_WITHID( SwInputChild );
    void            SetFormula( const OUString& rFormula )
                    { static_cast<SwInputWindow*>(GetWindow())->SetFormula( rFormula ); }
    const SwView*   GetView() const
                    { return static_cast<SwInputWindow*>(GetWindow())->GetView();}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
