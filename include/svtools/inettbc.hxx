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

#include <memory>
#include <rtl/ref.hxx>
#include <svtools/svtdllapi.h>
#include <tools/urlobj.hxx>

#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

class SvtMatchContext_Impl;
class SvtURLBox_Impl;

class SVT_DLLPUBLIC SvtURLBox
{
    friend class SvtMatchContext_Impl;
    friend class SvtURLBox_Impl;

    Idle                            aChangedIdle;
    OUString                        aBaseURL;
    OUString                        aPlaceHolder;
    rtl::Reference<SvtMatchContext_Impl> pCtx;
    std::unique_ptr<SvtURLBox_Impl> pImpl;
    INetProtocol                    eSmartProtocol;
    bool                            bOnlyDirectories    : 1;
    bool                            bHistoryDisabled    : 1;
    bool                            bNoSelection        : 1;

    Link<weld::ComboBox&, void>     aChangeHdl;
    Link<weld::Widget&, void>       aFocusInHdl;
    Link<weld::Widget&, void>       aFocusOutHdl;

    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_DLLPRIVATE_LINK(           TryAutoComplete, Timer*, void);
    SVT_DLLPRIVATE void             UpdatePicklistForSmartProtocol_Impl();
    DECL_DLLPRIVATE_LINK(           ChangedHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(           FocusInHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(           FocusOutHdl, weld::Widget&, void);
    SVT_DLLPRIVATE void             Init();

public:
    SvtURLBox(std::unique_ptr<weld::ComboBox> xWidget);
    ~SvtURLBox();

    void                set_entry_text(const OUString& rStr) { m_xWidget->set_entry_text(rStr); }
    void                show() { m_xWidget->show(); }
    void                clear() { m_xWidget->clear(); }
    void                connect_entry_activate(const Link<weld::ComboBox&, bool>& rLink) { m_xWidget->connect_entry_activate(rLink); }
    void                connect_key_press(const Link<const KeyEvent&, bool>& rLink) { m_xWidget->connect_key_press(rLink); }
    void                connect_changed(const Link<weld::ComboBox&, void>& rLink) { aChangeHdl = rLink; }
    void                trigger_changed() { aChangeHdl.Call(*m_xWidget); }
    void                connect_focus_in(const Link<weld::Widget&, void>& rLink) { aFocusInHdl = rLink; }
    void                connect_focus_out(const Link<weld::Widget&, void>& rLink) { aFocusOutHdl = rLink; }
    void                append_text(const OUString& rStr) { m_xWidget->append_text(rStr); }
    int                 find_text(const OUString& rStr) const { return m_xWidget->find_text(rStr); }
    OUString            get_active_text() const { return m_xWidget->get_active_text(); }
    void                grab_focus() { m_xWidget->grab_focus(); }
    void                set_sensitive(bool bSensitive) { m_xWidget->set_sensitive(bSensitive); }
    void                set_help_id(const OString& rHelpId) { m_xWidget->set_help_id(rHelpId); }
    void                select_entry_region(int nStartPos, int nEndPos) { m_xWidget->select_entry_region(nStartPos, nEndPos); }
    Size                get_preferred_size() const { return m_xWidget->get_preferred_size(); }

    void                EnableAutocomplete(bool bEnable = true) { m_xWidget->set_entry_completion(bEnable); }
    void                SetBaseURL( const OUString& rURL );
    const OUString&     GetBaseURL() const { return aBaseURL; }
    void                SetOnlyDirectories( bool bDir );
    void                SetNoURLSelection( bool bSet );
    void                SetSmartProtocol( INetProtocol eProt );
    INetProtocol        GetSmartProtocol() const { return eSmartProtocol; }
    OUString            GetURL();
    void                DisableHistory();

    weld::ComboBox*     getWidget() { return m_xWidget.get(); }

    static OUString     ParseSmart( const OUString& aText, const OUString& aBaseURL );

    void                SetPlaceHolder(const OUString& sPlaceHolder) { aPlaceHolder = sPlaceHolder; }
    const OUString&     GetPlaceHolder() const { return aPlaceHolder; }
    bool                MatchesPlaceHolder(const OUString& sToMatch) const
    {
        return (!aPlaceHolder.isEmpty() && aPlaceHolder == sToMatch);
    }

    void                SetFilter(const OUString& _sFilter);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
