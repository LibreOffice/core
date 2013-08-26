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
                             const OUString & rMimeType,
                             sal_Bool bSynchron = false );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
            void    EndListeningAll();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
