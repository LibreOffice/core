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

/// Dialog for editing a name
class SvxNameDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdtName;
    std::unique_ptr<weld::Label> m_xFtDescription;
    std::unique_ptr<weld::Button> m_xBtnOK;

    Link<SvxNameDialog&,bool> m_aCheckNameHdl;
    Link<SvxNameDialog&,OUString> m_aCheckNameTooltipHdl;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    SvxNameDialog(weld::Window* pWindow, const OUString& rName, const OUString& rDesc);

    OUString GetName() const { return m_xEdtName->get_text(); }

    /** add a callback Link that is called whenever the content of the edit
        field is changed.  The Link result determines whether the OK
        Button is enabled (> 0) or disabled (== 0).

        @param rLink a Callback declared with DECL_LINK and implemented with
               IMPL_LINK, that is executed on modification.

        @param bCheckImmediately If true, the Link is called directly after
               setting it. It is recommended to set this flag to true to avoid
               an inconsistent state if the initial String (given in the CTOR)
               does not satisfy the check condition.

        @todo Remove the parameter bCheckImmediately and incorporate the 'true'
              behaviour as default.
     */
    void SetCheckNameHdl(const Link<SvxNameDialog&,bool>& rLink, bool bCheckImmediately)
    {
        m_aCheckNameHdl = rLink;
        if (bCheckImmediately)
            m_xBtnOK->set_sensitive(rLink.Call(*this));
    }

    void SetCheckNameTooltipHdl(const Link<SvxNameDialog&,OUString>& rLink)
    {
        m_aCheckNameTooltipHdl = rLink;
        m_xBtnOK->set_tooltip_text(rLink.Call(*this));
    }

    void SetEditHelpId(const OString& aHelpId) { m_xEdtName->set_help_id(aHelpId);}
};

/** #i68101#
    Dialog for editing Object name
    plus uniqueness-callback-linkHandler */
class SvxObjectNameDialog : public weld::GenericDialogController
{
private:
    // name
    std::unique_ptr<weld::Entry> m_xEdtName;

    // buttons
    std::unique_ptr<weld::Button> m_xBtnOK;

    // callback link for name uniqueness
    Link<SvxObjectNameDialog&,bool> aCheckNameHdl;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    // constructor
    SvxObjectNameDialog(weld::Window* pWindow, const OUString& rName);

    // data access
    OUString GetName() const { return m_xEdtName->get_text(); }

    // set handler
    void SetCheckNameHdl(const Link<SvxObjectNameDialog&,bool>& rLink)
    {
        aCheckNameHdl = rLink;
    }
};

/** #i68101#
    Dialog for editing Object Title and Description */
class SvxObjectTitleDescDialog : public weld::GenericDialogController
{
private:
    // title
    std::unique_ptr<weld::Entry> m_xEdtTitle;

    // description
    std::unique_ptr<weld::TextView> m_xEdtDescription;

public:
    // constructor
    SvxObjectTitleDescDialog(weld::Window* pWindow, const OUString& rTitle, const OUString& rDesc);
    // data access
    OUString GetTitle() const { return m_xEdtTitle->get_text(); }
    OUString GetDescription() const { return m_xEdtDescription->get_text(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
