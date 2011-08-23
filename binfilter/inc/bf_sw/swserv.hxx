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

#ifndef _SWSERV_HXX
#define _SWSERV_HXX

#ifndef _LINKSRC_HXX
#include <bf_so3/linksrc.hxx>
#endif
namespace binfilter {

class SwBookmark;
class SwSectionNode;
class SwBaseLink;
class SwTableNode;
struct SwPosition;
class SwPaM;

class SwServerObject : public ::binfilter::SvLinkSource
{
protected:
    enum ServerModes { BOOKMARK_SERVER, TABLE_SERVER, SECTION_SERVER, NONE_SERVER } eType;
    union {
        SwBookmark* pBkmk;
        SwTableNode* pTblNd;
        SwSectionNode* pSectNd;
    } CNTNT_TYPE;

    SwServerObject();

public:
    SwServerObject( SwBookmark& rBookmark )
        : eType( BOOKMARK_SERVER )
    {
        CNTNT_TYPE.pBkmk = &rBookmark;
    }
    SwServerObject( SwTableNode& rTableNd )
        : eType( TABLE_SERVER )
    {
        CNTNT_TYPE.pTblNd = &rTableNd;
    }
    SwServerObject( SwSectionNode& rSectNd )
        : eType( SECTION_SERVER )
    {
        CNTNT_TYPE.pSectNd = &rSectNd;
    }
    virtual ~SwServerObject();




    BOOL IsLinkInServer( const SwBaseLink* ) const;

    void SetNoServer()
        {  CNTNT_TYPE.pBkmk = 0, eType = NONE_SERVER; }
};

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif

} //namespace binfilter
#endif	// _SWSERV_HXX

