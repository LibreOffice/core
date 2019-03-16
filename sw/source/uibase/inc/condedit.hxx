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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONDEDIT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONDEDIT_HXX

#include <vcl/edit.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <swdllapi.h>

class SW_DLLPUBLIC ConditionEdit : public Edit, public DropTargetHelper
{
    bool bBrackets, bEnableDrop;

    SAL_DLLPRIVATE virtual sal_Int8  AcceptDrop( const AcceptDropEvent& rEvt ) override;
    SAL_DLLPRIVATE virtual sal_Int8  ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    ConditionEdit(vcl::Window* pParent, WinBits nStyle);

    void ShowBrackets(bool bShow)
    {
        bBrackets = bShow;
    }

    void SetDropEnable(bool bFlag)
    {
        bEnableDrop = bFlag;
    }
};

class SwConditionEdit;

class SW_DLLPUBLIC SwConditionEditDropTarget : public DropTargetHelper
{
private:
    SwConditionEdit& m_rEdit;

    SAL_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    SAL_DLLPRIVATE virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    SwConditionEditDropTarget(SwConditionEdit& rEdit);
};

class SW_DLLPUBLIC SwConditionEdit
{
    std::unique_ptr<weld::Entry> m_xControl;
    SwConditionEditDropTarget m_aDropTargetHelper;
    bool bBrackets, bEnableDrop;

public:
    SwConditionEdit(std::unique_ptr<weld::Entry> xControl);

    OUString get_text() const { return m_xControl->get_text(); }
    void set_text(const OUString& rText) { m_xControl->set_text(rText); }
    void set_visible(bool bVisible) { m_xControl->set_visible(bVisible); }
    void set_accessible_name(const OUString& rName) { m_xControl->set_accessible_name(rName); }
    bool get_sensitive() const { return m_xControl->get_sensitive(); }
    void save_value() { m_xControl->save_value(); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }
    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_xControl->connect_changed(rLink); }
    void hide() { m_xControl->hide(); }
    weld::Entry& get_widget() { return *m_xControl; }

    void ShowBrackets(bool bShow) { bBrackets = bShow; }
    bool GetBrackets() const { return bBrackets; }
    void SetDropEnable(bool bFlag) { bEnableDrop = bFlag; }
    bool GetDropEnable() const { return bEnableDrop; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
