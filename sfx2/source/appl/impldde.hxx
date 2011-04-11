/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _IMPLDDE_HXX
#define _IMPLDDE_HXX

#include <sfx2/linksrc.hxx>
#include <tools/string.hxx>

class DdeConnection;
class DdeData;
class DdeLink;
class DdeRequest;
class DdeTransaction;

namespace sfx2
{

class SvDDEObject : public SvLinkSource
{
    String sItem;

    DdeConnection* pConnection;
    DdeLink* pLink;
    DdeRequest* pRequest;
    ::com::sun::star::uno::Any * pGetData;

    sal_uInt8 bWaitForData : 1;  // waiting for data?
    sal_uInt8 nError       : 7;  // Error code for dialogue


    sal_Bool ImplHasOtherFormat( DdeTransaction& );
    DECL_LINK( ImplGetDDEData, DdeData* );
    DECL_LINK( ImplDoneDDEData, void* );

protected:
    virtual ~SvDDEObject();

public:
    SvDDEObject();

    virtual sal_Bool    GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const String & aMimeType,
                                sal_Bool bSynchron = sal_False );

    virtual sal_Bool    Connect( SvBaseLink * );
    virtual void    Edit( Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link& rEndEditHdl );

    virtual sal_Bool    IsPending() const;
    virtual sal_Bool    IsDataComplete() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
