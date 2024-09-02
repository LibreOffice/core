/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "net_bridge.hxx"

#include <bridges/net_uno/net_types.hxx>
#include <rtl/ustring.hxx>

namespace net_uno
{
struct NetProxy : public uno_Interface
{
    mutable oslInterlockedCount m_ref;
    Bridge& m_bridge;
    IntPtr m_netI;
    OUString m_oid;
    typelib_InterfaceTypeDescription* m_TD;

    NetProxy(Bridge& rBridge, IntPtr pNetI, typelib_InterfaceTypeDescription* pTD,
             const OUString& sOid);
    ~NetProxy();

    void acquire();
    void release();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
