/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
