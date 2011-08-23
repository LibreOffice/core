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

#include <bf_so3/linksrc.hxx>
#include <tools/string.hxx>

namespace binfilter
{
class DdeConnection;
class DdeData;
class DdeLink;
class DdeRequest;
class DdeTransaction;

class SvDDEObject : public SvLinkSource
{
    String sItem;

    DdeConnection* pConnection;
    DdeLink* pLink;
    DdeRequest* pRequest;
    ::com::sun::star::uno::Any * pGetData;

    BYTE bWaitForData : 1; 					// wird auf Daten gewartet?
    BYTE nError		: 7;					// Error Code fuer den Dialog


    BOOL ImplHasOtherFormat( DdeTransaction& );
    DECL_LINK( ImplGetDDEData, DdeData* );
    DECL_LINK( ImplDoneDDEData, void* );

protected:
    virtual ~SvDDEObject();

public:
    SvDDEObject();

    virtual BOOL	GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const String & aMimeType,
                                BOOL bSynchron = FALSE );

    virtual BOOL	Connect( SvBaseLink * );
    virtual String	Edit( Window*, SvBaseLink * );

    virtual BOOL	IsPending() const;
    virtual BOOL	IsDataComplete() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
