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

#include <stdio.h>
#include <resourcemodel/XPathLogger.hxx>

#ifdef DEBUG_CONTEXT_HANDLER

namespace writerfilter
{
XPathLogger::XPathLogger()
: mp_tokenMap(new TokenMap_t)
{
}

XPathLogger::~XPathLogger()
{
}

std::string XPathLogger::getXPath() const
{
    return m_currentPath;
}

void XPathLogger::updateCurrentPath()
{
    m_currentPath = "";

    for (std::vector<std::string>::const_iterator aIt = m_path.begin();
         aIt != m_path.end();
         ++aIt)
    {
        if (m_currentPath.size() > 0)
            m_currentPath += "/";

        m_currentPath += *aIt;
    }
}

void XPathLogger::startElement(std::string _token)
{
    TokenMap_t::const_iterator aIt = mp_tokenMap->find(_token);

    if (aIt == mp_tokenMap->end())
        (*mp_tokenMap)[_token] = 1;
    else
        (*mp_tokenMap)[_token]++;

    static char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "[%d]", (*mp_tokenMap)[_token]);
    m_path.push_back(_token + sBuffer);

    m_tokenMapStack.push(mp_tokenMap);
    mp_tokenMap.reset(new TokenMap_t);

    updateCurrentPath();
}

void XPathLogger::endElement()
{
    mp_tokenMap = m_tokenMapStack.top();
    m_tokenMapStack.pop();
    m_path.pop_back();

    updateCurrentPath();
}

} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
