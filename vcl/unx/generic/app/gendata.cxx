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

SalData::SalData() {}

SalData::~SalData() {}

#endif

GenericUnixSalData::GenericUnixSalData() {}

GenericUnixSalData::~GenericUnixSalData()
{
#ifndef IOS
    // at least for GetPrintFontManager the sequence is important
    m_pPrintFontManager.reset();
    m_pFreetypeManager.reset();
    m_pPrinterInfoManager.reset();
#endif
}

#ifndef IOS
FreetypeManager* GenericUnixSalData::GetFreetypeManager()
{
    if (!m_pFreetypeManager)
        m_pFreetypeManager.reset(new FreetypeManager);
    return m_pFreetypeManager.get();
}

psp::PrintFontManager* GenericUnixSalData::GetPrintFontManager()
{
    if (!m_pPrintFontManager)
    {
        GetFreetypeManager();
        m_pPrintFontManager.reset(new psp::PrintFontManager);
        m_pPrintFontManager->initialize();
    }
    // PrintFontManager needs the FreetypeManager
    assert(m_pFreetypeManager);
    return m_pPrintFontManager.get();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
