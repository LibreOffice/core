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

#ifdef IOS
#include <ios/iosinst.hxx>
#endif

#include <unx/gendata.hxx>

#include <unx/font/fontmanager.hxx>

#ifndef IOS

#include <unx/font/GenericFontList.hxx>
#include <printerinfomanager.hxx>

SalData::SalData() {}

SalData::~SalData() {}

#endif

GenericUnixSalData::GenericUnixSalData() {}

GenericUnixSalData::~GenericUnixSalData()
{
#ifndef IOS
    // the font list was enumerated from fontconfig, so it goes first
    m_pGenericFontList.reset();
    m_pFontConfigManager.reset();
    m_pPrinterInfoManager.reset();
#endif
}

#ifndef IOS
GenericFontList* GenericUnixSalData::GetGenericFontList()
{
    if (!m_pGenericFontList)
    {
        // fontconfig has to be up, and know about our own font directories,
        // before it can hand the system fonts over to the font list
        GetFontConfigManager();
        m_pGenericFontList.reset(new GenericFontList);
        m_pGenericFontList->Init();
    }
    return m_pGenericFontList.get();
}

FontConfigManager* GenericUnixSalData::GetFontConfigManager()
{
    if (!m_pFontConfigManager)
        m_pFontConfigManager.reset(new FontConfigManager);
    return m_pFontConfigManager.get();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
