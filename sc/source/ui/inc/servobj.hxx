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

#ifndef SC_SERVOBJ_HXX
#define SC_SERVOBJ_HXX

#include <svl/lstner.hxx>
#include <svl/listener.hxx>
#include <sfx2/linksrc.hxx>
#include "global.hxx"
#include "address.hxx"

class ScDocShell;
class ScServerObject;

class ScServerObjectSvtListenerForwarder : public SvtListener
{
    ScServerObject* pObj;
    SfxBroadcaster  aBroadcaster;
public:
                    ScServerObjectSvtListenerForwarder( ScServerObject* pObjP);
    virtual         ~ScServerObjectSvtListenerForwarder();
    virtual void    Notify( SvtBroadcaster& rBC, const SfxHint& rHint);
};

class ScServerObject : public ::sfx2::SvLinkSource, public SfxListener
{
private:
    ScServerObjectSvtListenerForwarder  aForwarder;
    ScDocShell*     pDocSh;
    ScRange         aRange;
    String          aItemStr;
    sal_Bool            bRefreshListener;

    void    Clear();

public:
            ScServerObject( ScDocShell* pShell, const String& rItem );
    virtual ~ScServerObject();

    virtual sal_Bool GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                             const String & rMimeType,
                             sal_Bool bSynchron = sal_False );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
            void    EndListeningAll();
};

//SO2_DECL_REF( ScServerObject )


#endif
