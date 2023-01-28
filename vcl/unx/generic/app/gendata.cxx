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

#include <unx/fontmanager.hxx>

#ifndef IOS

#include <unx/glyphcache.hxx>
#include <printerinfomanager.hxx>

SalData::SalData() { SetSalData(this); }

SalData::~SalData() {}

#endif

GenericUnixSalData::GenericUnixSalData()
    : m_pDisplay(nullptr)
{
}

GenericUnixSalData::~GenericUnixSalData()
{
#ifndef IOS
    // at least for InitPrintFontManager the sequence is important
    m_pPrintFontManager.reset();
    m_pFreetypeManager.reset();
    m_pPrinterInfoManager.reset();
#endif
}

void GenericUnixSalData::Dispose() {}

#ifndef IOS
void GenericUnixSalData::InitFreetypeManager() { m_pFreetypeManager.reset(new FreetypeManager); }
#endif

void GenericUnixSalData::InitPrintFontManager()
{
#ifndef IOS
    GetFreetypeManager();
    m_pPrintFontManager.reset(new psp::PrintFontManager);
    m_pPrintFontManager->initialize();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
