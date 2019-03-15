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

#undef SC_DLLIMPLEMENTATION

#include <dapitype.hxx>

using namespace com::sun::star;

ScDataPilotSourceTypeDlg::ScDataPilotSourceTypeDlg(weld::Window* pParent, bool bEnableExternal)
    : GenericDialogController(pParent, "modules/scalc/ui/selectsource.ui", "SelectSourceDialog")
    , m_xBtnSelection(m_xBuilder->weld_radio_button("selection"))
    , m_xBtnNamedRange(m_xBuilder->weld_radio_button("namedrange"))
    , m_xBtnDatabase(m_xBuilder->weld_radio_button("database"))
    , m_xBtnExternal(m_xBuilder->weld_radio_button("external"))
    , m_xLbNamedRange(m_xBuilder->weld_combo_box("rangelb"))
{
    m_xBtnSelection->connect_toggled( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );
    m_xBtnNamedRange->connect_toggled( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );
    m_xBtnDatabase->connect_toggled( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );
    m_xBtnExternal->connect_toggled( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );

    if (!bEnableExternal)
        m_xBtnExternal->set_sensitive(false);

    m_xBtnSelection->set_active(true);

    // Disabled unless at least one named range exists.
    m_xLbNamedRange->set_sensitive(false);
    m_xBtnNamedRange->set_sensitive(false);

    // Intentionally hide this button to see if anyone complains.
    m_xBtnExternal->hide();
}

ScDataPilotSourceTypeDlg::~ScDataPilotSourceTypeDlg()
{
}

bool ScDataPilotSourceTypeDlg::IsDatabase() const
{
    return m_xBtnDatabase->get_active();
}

bool ScDataPilotSourceTypeDlg::IsExternal() const
{
    return m_xBtnExternal->get_active();
}

bool ScDataPilotSourceTypeDlg::IsNamedRange() const
{
    return m_xBtnNamedRange->get_active();
}

OUString ScDataPilotSourceTypeDlg::GetSelectedNamedRange() const
{
    return m_xLbNamedRange->get_active_text();
}

void ScDataPilotSourceTypeDlg::AppendNamedRange(const OUString& rName)
{
    m_xLbNamedRange->append_text(rName);
    if (m_xLbNamedRange->get_count() == 1)
    {
        // Select position 0 only for the first time.
        m_xLbNamedRange->set_active(0);
        m_xBtnNamedRange->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(ScDataPilotSourceTypeDlg, RadioClickHdl, weld::ToggleButton&, void)
{
    m_xLbNamedRange->set_sensitive(m_xBtnNamedRange->get_active());
}

ScDataPilotServiceDlg::ScDataPilotServiceDlg(weld::Window* pParent, const std::vector<OUString>& rServices)
    : GenericDialogController(pParent, "modules/scalc/ui/dapiservicedialog.ui", "DapiserviceDialog")
    , m_xLbService(m_xBuilder->weld_combo_box("service"))
    , m_xEdSource(m_xBuilder->weld_entry("source"))
    , m_xEdName(m_xBuilder->weld_entry("name"))
    , m_xEdUser(m_xBuilder->weld_entry("user"))
    , m_xEdPasswd(m_xBuilder->weld_entry("password"))
{
    for (const OUString& aName : rServices)
    {
        m_xLbService->append_text(aName);
    }
    m_xLbService->set_active(0);
}

ScDataPilotServiceDlg::~ScDataPilotServiceDlg()
{
}

OUString ScDataPilotServiceDlg::GetServiceName() const
{
    return m_xLbService->get_active_text();
}

OUString ScDataPilotServiceDlg::GetParSource() const
{
    return m_xEdSource->get_text();
}

OUString ScDataPilotServiceDlg::GetParName() const
{
    return m_xEdName->get_text();
}

OUString ScDataPilotServiceDlg::GetParUser() const
{
    return m_xEdUser->get_text();
}

OUString ScDataPilotServiceDlg::GetParPass() const
{
    return m_xEdPasswd->get_text();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
