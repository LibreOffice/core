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

class ScDataPilotSourceTypeDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::RadioButton> m_xBtnSelection;
    std::unique_ptr<weld::RadioButton> m_xBtnNamedRange;
    std::unique_ptr<weld::RadioButton> m_xBtnDatabase;
    std::unique_ptr<weld::RadioButton> m_xBtnExternal;
    std::unique_ptr<weld::ComboBox> m_xLbNamedRange;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

public:
    ScDataPilotSourceTypeDlg(weld::Window* pParent, bool bEnableExternal);
    virtual ~ScDataPilotSourceTypeDlg() override;
    bool IsDatabase() const;
    bool IsExternal() const;
    bool IsNamedRange() const;
    OUString GetSelectedNamedRange() const;
    void AppendNamedRange(const OUString& rNames);

private:
    DECL_LINK(RadioClickHdl, weld::ToggleButton&, void);
    DECL_LINK(ResponseHdl, weld::Button&, void);
};

class ScDataPilotServiceDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::ComboBox> m_xLbService;
    std::unique_ptr<weld::Entry> m_xEdSource;
    std::unique_ptr<weld::Entry> m_xEdName;
    std::unique_ptr<weld::Entry> m_xEdUser;
    std::unique_ptr<weld::Entry> m_xEdPasswd;

public:
    ScDataPilotServiceDlg(weld::Window* pParent, const std::vector<OUString>& rServices);
    virtual ~ScDataPilotServiceDlg() override;

    OUString GetServiceName() const;
    OUString GetParSource() const;
    OUString GetParName() const;
    OUString GetParUser() const;
    OUString GetParPass() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
