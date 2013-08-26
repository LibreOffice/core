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
#ifndef _SWSERV_HXX
#define _SWSERV_HXX

#include <sfx2/linksrc.hxx>
#include <IMark.hxx>

class SwSectionNode;
class SwBaseLink;
class SwTableNode;
struct SwPosition;
class SwPaM;

class SwServerObject : public ::sfx2::SvLinkSource
{
    using sfx2::SvLinkSource::SendDataChanged;

protected:
    enum ServerModes { BOOKMARK_SERVER, TABLE_SERVER, SECTION_SERVER, NONE_SERVER } eType;
    union {
        ::sw::mark::IMark* pBkmk;
        SwTableNode* pTblNd;
        SwSectionNode* pSectNd;
    } CNTNT_TYPE;

    SwServerObject();

public:
    SwServerObject( ::sw::mark::IMark& rBookmark )
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

    virtual sal_Bool GetData( ::com::sun::star::uno::Any & rData,
                            const OUString & rMimeType,
                            sal_Bool bSynchron = sal_False );

    sal_Bool SetData( const String & rMimeType,
                    const ::com::sun::star::uno::Any& rData );

    virtual void SendDataChanged( const SwPosition& rPos );
    virtual void SendDataChanged( const SwPaM& rRange );

    sal_Bool IsLinkInServer( const SwBaseLink* ) const;

    void SetNoServer();
    void SetDdeBookmark( ::sw::mark::IMark& rBookmark);
};

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif

#endif  // _SWSERV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
