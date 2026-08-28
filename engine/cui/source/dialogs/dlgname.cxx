/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <dlgname.hxx>

#include <comphelper/string.hxx>

/*************************************************************************
|*
|* Dialog for editing a name
|*
\************************************************************************/

SvxNameDialog::SvxNameDialog(weld::Window* pParent, const OUString& rName, const OUString& rDesc,
                             const OUString& rTitle)
    : GenericDialogController(pParent, u"cui/ui/namedialog.ui"_ustr, u"NameDialog"_ustr)
    , m_xEdtName(m_xBuilder->weld_entry(u"name_entry"_ustr))
    , m_xFtDescription(m_xBuilder->weld_label(u"description_label"_ustr))
    , m_xBtnOK(m_xBuilder->weld_button(u"ok"_ustr))
    , m_aCheckName(nullptr)
{
    m_xFtDescription->set_label(rDesc);
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);
    ModifyHdl(*m_xEdtName);
    m_xEdtName->connect_changed(LINK(this, SvxNameDialog, ModifyHdl));
    if (!rTitle.isEmpty())
        set_title(rTitle);
}

IMPL_LINK_NOARG(SvxNameDialog, ModifyHdl, weld::Entry&, void)
{
    // Do not allow empty names, unless custom CheckNameHdl is specified
    bool bEnable;
    if (m_aCheckNameHdl.IsSet())
        bEnable = m_aCheckNameHdl.Call(*this);
    else if (m_aCheckName)
        bEnable = m_aCheckName(m_xEdtName->get_text());
    else
        bEnable = !m_xEdtName->get_text().isEmpty();
    m_xBtnOK->set_sensitive(bEnable);
    // tdf#129032: feedback on reason to disabled controls
    m_xEdtName->set_message_type(bEnable ? weld::EntryMessageType::Normal
                                         : weld::EntryMessageType::Error);
    OUString rTip = u""_ustr;
    if (!bEnable && m_aCheckNameTooltipHdl.IsSet())
        rTip = m_aCheckNameTooltipHdl.Call(*this);
    m_xBtnOK->set_tooltip_text(rTip);
    m_xEdtName->set_tooltip_text(rTip);
}

// #i68101#
// Dialog for editing Object Name
// plus uniqueness-callback-linkHandler

SvxObjectNameDialog::SvxObjectNameDialog(weld::Window* pParent, const OUString& rName)
    : GenericDialogController(pParent, u"cui/ui/objectnamedialog.ui"_ustr, u"ObjectNameDialog"_ustr)
    , m_xEdtName(m_xBuilder->weld_entry(u"object_name_entry"_ustr))
    , m_xBtnOK(m_xBuilder->weld_button(u"ok"_ustr))
{
    // set name
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);

    // activate name
    ModifyHdl(*m_xEdtName);
    m_xEdtName->connect_changed(LINK(this, SvxObjectNameDialog, ModifyHdl));
}

IMPL_LINK_NOARG(SvxObjectNameDialog, ModifyHdl, weld::Entry&, void)
{
    if (aCheckNameHdl.IsSet())
    {
        m_xBtnOK->set_sensitive(aCheckNameHdl.Call(*this));
    }
}

// #i68101#
// Dialog for editing Object Title and Description

SvxObjectTitleDescDialog::SvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle,
                                                   const OUString& rDescription,
                                                   bool const isDecorative)
    : GenericDialogController(pParent, u"cui/ui/objecttitledescdialog.ui"_ustr,
                              u"ObjectTitleDescDialog"_ustr)
    , m_xTitleFT(m_xBuilder->weld_label(u"object_title_label"_ustr))
    , m_xEdtTitle(m_xBuilder->weld_entry(u"object_title_entry"_ustr))
    , m_xDescriptionFT(m_xBuilder->weld_label(u"desc_label"_ustr))
    , m_xEdtDescription(m_xBuilder->weld_text_view(u"desc_entry"_ustr))
    , m_xDecorativeCB(m_xBuilder->weld_check_button(u"decorative"_ustr))
{
    //lock height to initial height
    m_xEdtDescription->set_size_request(-1, m_xEdtDescription->get_text_height() * 5);
    // set title & desc
    m_xEdtTitle->set_text(rTitle);
    m_xEdtDescription->set_text(rDescription);

    // activate title
    m_xEdtTitle->select_region(0, -1);

    m_xDecorativeCB->set_active(isDecorative);
    m_xDecorativeCB->connect_toggled(LINK(this, SvxObjectTitleDescDialog, DecorativeHdl));
    DecorativeHdl(*m_xDecorativeCB);
}

IMPL_LINK_NOARG(SvxObjectTitleDescDialog, DecorativeHdl, weld::Toggleable&, void)
{
    bool const bEnable(!m_xDecorativeCB->get_active());
    m_xEdtTitle->set_sensitive(bEnable);
    m_xTitleFT->set_sensitive(bEnable);
    m_xEdtDescription->set_sensitive(bEnable);
    m_xDescriptionFT->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
