/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <bridges/net_uno/net_types.hxx>

namespace net_uno
{
typedef IntPtr (*CreateProxyFunc)(String pOid, String pInterfaceName, IntPtr pBridge,
                                  IntPtr pUnoInterface, IntPtr pTD);
typedef String (*LookupObjectIdFunc)(IntPtr pNetInterface);
typedef IntPtr (*RegisterInterfaceFunc)(IntPtr pNetInterface, String pOid, String pInterfaceName);
typedef IntPtr (*LookupInterfaceFunc)(String pOid, String pInterfaceName);
typedef void (*RevokeInterfaceFunc)(String pOid, String pInterfaceName);
typedef sal_Int8 (*DispatchCallFunc)(IntPtr pNetInterface, String pMethodName, Value* pArgs,
                                     Value* pRet, Value* pExc);
typedef void (*ThrowErrorFunc)(String pWhere, String pMessage);

struct Context
{
    CreateProxyFunc createProxy;
    LookupObjectIdFunc lookupObjectId;
    RegisterInterfaceFunc registerInterface;
    LookupInterfaceFunc lookupInterface;
    RevokeInterfaceFunc revokeInterface;
    DispatchCallFunc dispatchCall;
    ThrowErrorFunc throwError;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
