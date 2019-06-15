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

#include <qt5/Qt5Instance.hxx>

#include <KWayland/Client/registry.h>

class KDE5SalInstance final : public Qt5Instance
{
    Q_OBJECT

    KWayland::Client::Registry m_aRegistry;
    KWayland::Client::XdgShell* m_pXdgShell;

    Qt5FilePicker* createPicker(QFileDialog::FileMode) override;

    SalFrame* CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle) override;
    bool hasNativeFileSelection() const override { return true; }

private Q_SLOTS:
    void xdgShellAnnounced(quint32, quint32);

public:
    explicit KDE5SalInstance(std::unique_ptr<QApplication>& pQApp);

    KWayland::Client::XdgShell* xdgShell() const { return m_pXdgShell; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
