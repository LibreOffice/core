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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GOTODLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GOTODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/idle.hxx>

class NumEditAction;
class SwView;
class SwWrtShell;

class SwGotoPageDlg : public ModalDialog
{
public:
    SwGotoPageDlg(vcl::Window *parent = nullptr, SfxBindings* _pBindings = nullptr);

    virtual ~SwGotoPageDlg();
    virtual void dispose() override;

private:

    SwView*  GetCreateView() const;
    void GotoPage();
    void UsePage();
    void EditActionHdl();

    DECL_LINK_TYPED( EditAction, NumEditAction&, void );
    DECL_LINK_TYPED( EditGetFocus, Control&, void );
    DECL_LINK_TYPED( PageEditModifyHdl, SpinField&, void );
    DECL_LINK_TYPED( ChangePageHdl, Idle*, void );

    VclPtr<NumEditAction> mpMtrPageCtrl;

    SwView       *m_pCreateView;
    SfxBindings  &m_rBindings;
    Idle         m_aPageChgIdle;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */