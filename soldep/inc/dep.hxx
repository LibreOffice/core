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

#ifndef _DEP_HXX
#define _DEP_HXX

#include <soldep/sstring.hxx>

class SByteExtStringList : public SByteStringList
{
    private:
        sal_Bool        bDeleteStrings;
    public:
        SByteExtStringList(sal_Bool bDelete = sal_True) : bDeleteStrings (bDelete) {}
        ~SByteExtStringList();
};

class VersionDepInfo : public SByteExtStringList
{
    private:
        ByteString*                 pVersion;
    public:
        VersionDepInfo() : pVersion (NULL) {}
        VersionDepInfo(const ByteString& rCurrentVersion) : pVersion (NULL) { pVersion = new ByteString(rCurrentVersion); }
        ~VersionDepInfo();
        void                Clear();
        const ByteString*   GetVersion() { return pVersion; }
        //void              SetVersion(ByteString* pStr) { pVersion = pStr; }
};

DECLARE_LIST( VersionDepInfoList_Impl, VersionDepInfo* )

class VersionDepInfoList : public VersionDepInfoList_Impl
{
    public:
        VersionDepInfoList () {}
        ~VersionDepInfoList () {}
        VersionDepInfo*     GetVersion (ByteString& rVersion);
        void                RemoveVersion (ByteString& rVersion);
        VersionDepInfo*     InsertVersion (ByteString& rVersion);

};


#endif
