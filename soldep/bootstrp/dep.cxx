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
