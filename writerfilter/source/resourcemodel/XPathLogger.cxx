/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <stdio.h>
#include <resourcemodel/XPathLogger.hxx>

namespace writerfilter
{
XPathLogger::XPathLogger()
: mp_tokenMap(new TokenMap_t)
{
}

XPathLogger::~XPathLogger()
{
}

string XPathLogger::getXPath() const
{
    return m_currentPath;
}

void XPathLogger::updateCurrentPath()
{
    m_currentPath = "";

    for (vector<string>::const_iterator aIt = m_path.begin();
         aIt != m_path.end();
         aIt++)
    {
        if (m_currentPath.size() > 0)
            m_currentPath += "/";

        m_currentPath += *aIt;
    }
}

void XPathLogger::startElement(string _token)
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

}
