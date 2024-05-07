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

#include <cuiimapwnd.hxx>

/*************************************************************************
|*
|*  URLDlg
|*
\************************************************************************/

URLDlg::URLDlg(weld::Widget* pWindow, const OUString& rURL, const OUString& rAlternativeText,
               const OUString& rDescription, const OUString& rTarget, const OUString& rName,
               TargetList& rTargetList)
    : GenericDialogController(pWindow, u"cui/ui/cuiimapdlg.ui"_ustr, u"IMapDialog"_ustr)
    , m_xEdtURL(m_xBuilder->weld_entry(u"urlentry"_ustr))
    , m_xCbbTargets(m_xBuilder->weld_combo_box(u"frameCB"_ustr))
    , m_xEdtName(m_xBuilder->weld_entry(u"nameentry"_ustr))
    , m_xEdtAlternativeText(m_xBuilder->weld_entry(u"textentry"_ustr))
    , m_xEdtDescription(m_xBuilder->weld_text_view(u"descTV"_ustr))
{
    m_xEdtDescription->set_size_request(m_xEdtDescription->get_approximate_digit_width() * 51,
                                        m_xEdtDescription->get_height_rows(5));

    m_xEdtURL->set_text(rURL);
    m_xEdtAlternativeText->set_text(rAlternativeText);
    m_xEdtDescription->set_text(rDescription);
    m_xEdtName->set_text(rName);

    for (const OUString& a : rTargetList)
        m_xCbbTargets->append_text(a);

    if (rTarget.isEmpty())
        m_xCbbTargets->set_entry_text(u"_self"_ustr);
    else
        m_xCbbTargets->set_entry_text(rTarget);
}

URLDlg::~URLDlg() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
