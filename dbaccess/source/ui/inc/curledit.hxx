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

#pragma once

#include <vcl/weld.hxx>
#include <dsntypes.hxx>

namespace dbaui
{

class OConnectionURLEdit
{
    OUString m_sSavedValue;

    ::dbaccess::ODsnTypeCollection* m_pTypeCollection;
    OUString m_sSaveValueNoPrefix;
    bool m_bShowPrefix; // when <TRUE> the prefix will be visible, otherwise not

    std::unique_ptr<weld::Entry> m_xEntry;
    std::unique_ptr<weld::Label> m_xForcedPrefix;

public:
    OConnectionURLEdit(std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::Label> xForcedPrefix);
    ~OConnectionURLEdit();

public:
    bool get_visible() const { return m_xEntry->get_visible(); }
    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_xEntry->connect_changed(rLink); }
    void set_help_id(const OString& rName) { m_xEntry->set_help_id(rName); }
    void hide()
    {
        m_xEntry->hide();
        if (m_bShowPrefix)
            m_xForcedPrefix->hide();
    }
    void show()
    {
        m_xEntry->show();
        if (m_bShowPrefix)
            m_xForcedPrefix->show();
    }
    void save_value() { m_sSavedValue = GetText(); }
    bool get_value_changed_from_saved() const { return m_sSavedValue != GetText(); }
    void grab_focus()
    {
        m_xEntry->grab_focus();
    }
    void set_sensitive(bool bSensitive)
    {
        m_xEntry->set_sensitive(bSensitive);
        if (m_bShowPrefix)
            m_xForcedPrefix->set_sensitive(bSensitive);
    }
    void connect_focus_in(const Link<weld::Widget&, void>& rLink)
    {
        m_xEntry->connect_focus_in(rLink);
    }
    void connect_focus_out(const Link<weld::Widget&, void>& rLink)
    {
        m_xEntry->connect_focus_out(rLink);
    }

    // Edit overridables
    void    SetText(const OUString& _rStr);
    void    SetText(const OUString& _rStr, const Selection& _rNewSelection);
    OUString  GetText() const;

    /** Shows the Prefix
        @param  _bShowPrefix
            If <TRUE/> than the prefix will be visible, otherwise not.
    */
    void     ShowPrefix(bool _bShowPrefix);
    /// get the currently set text, excluding the prefix indicating the type
    OUString GetTextNoPrefix() const;
    /// set a new text, leave the current prefix unchanged
    void     SetTextNoPrefix(const OUString& _rText);

    void      SaveValueNoPrefix()             { m_sSaveValueNoPrefix = GetTextNoPrefix(); }
    const OUString&  GetSavedValueNoPrefix() const   { return m_sSaveValueNoPrefix; }
    void      SetTypeCollection(::dbaccess::ODsnTypeCollection* _pTypeCollection) { m_pTypeCollection = _pTypeCollection; }
};

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
