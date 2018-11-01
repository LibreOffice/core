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

#include <sal/config.h>
#include <memory>

#include <QtCore/QObject>
#include <qt5/Qt5Instance.hxx>
#include "KDE5SalFrame.hxx"

class SalYieldMutex;
class SalFrame;

class KDE5SalInstance : public Qt5Instance
{
    Q_OBJECT
public:
    explicit KDE5SalInstance();

    virtual bool hasNativeFileSelection() const override { return true; }

    virtual css::uno::Reference<css::ui::dialogs::XFolderPicker2>
    createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>&) override;

    virtual bool IsMainThread() const override;

Q_SIGNALS:
    SalFrame* createFrameSignal(SalFrame* pParent, SalFrameStyleFlags nStyle);

    css::uno::Reference<css::ui::dialogs::XFilePicker2>
    createFilePickerSignal(const css::uno::Reference<css::uno::XComponentContext>&);

private Q_SLOTS:
    virtual SalFrame* CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle) override;

    virtual css::uno::Reference<css::ui::dialogs::XFilePicker2>
    createFilePicker(const css::uno::Reference<css::uno::XComponentContext>&) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
