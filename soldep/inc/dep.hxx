 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dep.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:33:36 $
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

#ifndef _DEP_HXX
#define _DEP_HXX

#include <bootstrp/sstring.hxx>

class SByteExtStringList : public SByteStringList
{
    private:
        BOOL        bDeleteStrings;
    public:
        SByteExtStringList(BOOL bDelete = TRUE) : bDeleteStrings (bDelete) {}
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
