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



#include "dep.hxx"

// class SByteExtStringList

/*****************************************************************************/
SByteExtStringList::~SByteExtStringList()
/*****************************************************************************/
{
    if (bDeleteStrings)
        while( Count()) {
            delete GetObject((sal_uIntPtr)0);
            Remove((sal_uIntPtr)0);
        }
}

// class VersionDepInfo

/*****************************************************************************/
VersionDepInfo::~VersionDepInfo()
/*****************************************************************************/
{
    if (pVersion)
        delete pVersion;
}

/*****************************************************************************/
void VersionDepInfo::Clear()
/*****************************************************************************/
{
    while( Count()) {
        delete GetObject((sal_uIntPtr)0);
        Remove((sal_uIntPtr)0);
    }
}

// class VersionDepInfoList

/*****************************************************************************/
VersionDepInfo* VersionDepInfoList::GetVersion (ByteString& rVersion)
/*****************************************************************************/
{
    VersionDepInfo* pInfo = First();
    while (pInfo)
    {
        const ByteString* pStr = pInfo->GetVersion();
        if (*pStr == rVersion)
            return pInfo;
        pInfo = Next();
    }
    return NULL;
}

/*****************************************************************************/
void VersionDepInfoList::RemoveVersion (ByteString& rVersion)
/*****************************************************************************/
{
    VersionDepInfo* pInfo = First();
    while (pInfo)
    {
        const ByteString* pStr = pInfo->GetVersion();
        if (*pStr == rVersion)
        {
            Remove (pInfo);
            delete pInfo;
            return;
        }
        pInfo = Next();
    }
}

/*****************************************************************************/
VersionDepInfo* VersionDepInfoList::InsertVersion (ByteString& rVersion)
/*****************************************************************************/
{
    VersionDepInfo* pInfo = First();
    while (pInfo)
    {
        const ByteString* pStr = pInfo->GetVersion();
        if (*pStr == rVersion)
        {
            pInfo->Clear();
            return pInfo;
        }
        pInfo = Next();
    }
    pInfo = new VersionDepInfo (rVersion);
    Insert (pInfo, LIST_APPEND);
    return pInfo;
}
