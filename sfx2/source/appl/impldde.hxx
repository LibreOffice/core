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
#pragma once

#include <rtl/ustring.hxx>
#include <sfx2/linksrc.hxx>
#include <tools/link.hxx>

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

    std::unique_ptr<DdeConnection> pConnection;
    std::unique_ptr<DdeLink> pLink;
    std::unique_ptr<DdeRequest> pRequest;
    css::uno::Any * pGetData;

    bool bWaitForData;  // waiting for data?


    static bool ImplHasOtherFormat( DdeTransaction& );
    DECL_LINK( ImplGetDDEData, const DdeData*, void );
    DECL_LINK( ImplDoneDDEData, bool, void );

protected:
    virtual ~SvDDEObject() override;

public:
    SvDDEObject();

    virtual bool    GetData( css::uno::Any & rData /*out param*/,
                                const OUString & aMimeType,
                                bool bSynchron = false ) override;

    virtual bool    Connect( SvBaseLink * ) override;
    virtual void    Edit(weld::Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link<const OUString&, void>& rEndEditHdl) override;

    virtual bool    IsPending() const override;
    virtual bool    IsDataComplete() const override;
};

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
