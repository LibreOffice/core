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
    OUString sItem;

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
                                const OUString & aMimeType,
                                sal_Bool bSynchron = sal_False );

    virtual sal_Bool    Connect( SvBaseLink * );
    virtual void    Edit( Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link& rEndEditHdl );

    virtual sal_Bool    IsPending() const;
    virtual sal_Bool    IsDataComplete() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
