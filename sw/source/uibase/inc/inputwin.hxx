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

#include <vcl/InterimItemWindow.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include <sfx2/childwin.hxx>

class SwFieldMgr;
class SwWrtShell;
class SwView;
class SfxDispatcher;

class InputEdit final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::Entry> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);
public:
    InputEdit(vcl::Window* pParent)
        : InterimItemWindow(pParent, "modules/swriter/ui/inputeditbox.ui", "InputEditBox")
        , m_xWidget(m_xBuilder->weld_entry("entry"))
    {
        m_xWidget->connect_key_press(LINK(this, InputEdit, KeyInputHdl));
        m_xWidget->connect_activate(LINK(this, InputEdit, ActivateHdl));
        SetSizePixel(m_xWidget->get_preferred_size());
    }

    void UpdateRange(const OUString& rSel, const OUString& rTableName);

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

    OUString get_text() const
    {
        return m_xWidget->get_text();
    }

    void set_accessible_name(const OUString& rName)
    {
        m_xWidget->set_accessible_name(rName);
    }

    void replace_selection(const OUString& rText)
    {
        int nStartPos, nEndPos;
        m_xWidget->get_selection_bounds(nStartPos, nEndPos);
        if (nStartPos > nEndPos)
            std::swap(nStartPos, nEndPos);

        m_xWidget->replace_selection(rText);

        nStartPos = nStartPos + rText.getLength();
        m_xWidget->select_region(nStartPos, nStartPos);
    }

    void select_region(int nStartPos, int nEndPos)
    {
        m_xWidget->select_region(nStartPos, nEndPos);
    }

    void connect_changed(const Link<weld::Entry&, void>& rLink)
    {
        m_xWidget->connect_changed(rLink);
    }

    virtual ~InputEdit() override
    {
        disposeOnce();
    }
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
    VclPtr<InputEdit> mxEdit;
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
    DECL_LINK(ModifyHdl, weld::Entry&, void);

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
