/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dep.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:31:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
            delete GetObject((ULONG)0);
            Remove((ULONG)0);
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
        delete GetObject((ULONG)0);
        Remove((ULONG)0);
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
