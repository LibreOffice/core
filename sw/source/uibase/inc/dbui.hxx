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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DBUI_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DBUI_HXX

#include <vcl/weld.hxx>

#include <swdllapi.h>

class PrintMonitor : public weld::GenericDialogController
{
public:
    std::unique_ptr<weld::Label> m_xDocName;
    std::unique_ptr<weld::Label> m_xPrinter;
    std::unique_ptr<weld::Label> m_xPrintInfo;

    PrintMonitor(weld::Window* pParent);
    virtual ~PrintMonitor() override;
};

class SW_DLLPUBLIC SaveMonitor : public weld::GenericDialogController
{
public:
    std::unique_ptr<weld::Label> m_xDocName;
    std::unique_ptr<weld::Label> m_xPrinter;
    std::unique_ptr<weld::Label> m_xPrintInfo;

    SaveMonitor(weld::Window* pParent);
    virtual ~SaveMonitor() override;
};

class CreateMonitor : public weld::GenericDialogController
{
public:
    CreateMonitor(weld::Window* pParent);
    virtual ~CreateMonitor() override;

    void SetTotalCount(sal_Int32 nTotal);
    void SetCurrentPosition(sal_Int32 nCurrent);

private:
    void UpdateCountingText();

private:
    OUString m_sCountingPattern;
    sal_Int32 m_nTotalCount;
    sal_Int32 m_nCurrentPosition;

    std::unique_ptr<weld::Label> m_xCounting;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
