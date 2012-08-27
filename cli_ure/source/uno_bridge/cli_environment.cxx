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

#include "Cli_environment.h"

#using <cli_ure.dll>
#using <system.dll>
#include "osl/diagnose.h"
#include "cli_proxy.h"

using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Proxies;
using namespace System::Collections;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;

namespace cli_uno
{

inline System::String^ Cli_environment::createKey(System::String^ oid, System::Type^ t)
{
    return System::String::Concat(oid, t->FullName);
}


inline Cli_environment::Cli_environment()
{
#if OSL_DEBUG_LEVEL >= 2
    _numRegisteredObjects = 0;
#endif
}

Cli_environment::~Cli_environment()
{
    OSL_ENSURE(_numRegisteredObjects == 0,
               "cli uno bridge: CLI environment contains unrevoked objects");
}


System::Object^ Cli_environment::registerInterface(
    System::Object^ obj, System::String^ oid)
{
#if OSL_DEBUG_LEVEL >= 1
    //obj must be a transparent proxy
    OSL_ASSERT(RemotingServices::IsTransparentProxy(obj));
    _numRegisteredObjects ++;
#endif
    OSL_ASSERT( ! m_objects->ContainsKey(oid));
    m_objects->Add(oid, gcnew WeakReference(obj));
    return obj;
}
System::Object^ Cli_environment::registerInterface      (
    System::Object^ obj, System::String^ oid, System::Type^ type)
{
#if OSL_DEBUG_LEVEL >= 1
    //obj must be a real cli object
    OSL_ASSERT( ! RemotingServices::IsTransparentProxy(obj));
    _numRegisteredObjects ++;
#endif
    System::String^ key = createKey(oid, type);
    //see synchronization in map_uno2cli in cli_data.cxx
    OSL_ASSERT( ! m_objects->ContainsKey(key));
    m_objects->Add(key, gcnew WeakReference(obj));
    return obj;
}

void Cli_environment::revokeInterface(System::String^ oid, System::Type^ type)
{
    System::String^ key = type != nullptr ? createKey(oid, type) : oid;
#if OSL_DEBUG_LEVEL >= 1
    _numRegisteredObjects --;
#endif
#if OSL_DEBUG_LEVEL >= 2
    int i = 1;
    if (m_objects->ContainsKey(key) == false)
    {
        Trace::WriteLine("cli uno bridge: try to revoke unregistered interface");
        Trace::WriteLine(oid);
        i = 0;
    }
    Trace::WriteLine(System::String::Format(
                         new System::String(S"cli uno bridge: {0} remaining registered interfaces"),
                         __box(m_objects->get_Count() - 1)));
#endif
    m_objects->Remove(key);
}

inline void Cli_environment::revokeInterface(System::String^ oid)
{
    return revokeInterface(oid, nullptr);
}

System::Object^ Cli_environment::getRegisteredInterface(System::String^ oid,
                                                        System::Type^ type)
{
    //try if it is a UNO interface
    System::Object^ ret = nullptr;
    ret = m_objects[oid];
    if (! ret)
    {
        //try if if it is a proxy for a cli object
        oid = createKey(oid, type);
        ret = m_objects[ oid ];
    }
    if (ret != nullptr)
    {
        System::WeakReference^ weakIface =
            static_cast< System::WeakReference ^ >( ret );
        ret = weakIface->Target;
    }
    if (ret == nullptr)
        m_objects->Remove( oid );
    return ret;
}

System::String^ Cli_environment::getObjectIdentifier(System::Object^ obj)
{
    System::String^ oId= nullptr;
    RealProxy^ aProxy= RemotingServices::GetRealProxy(obj);
    if (aProxy)
    {
        UnoInterfaceProxy^ proxyImpl= dynamic_cast<UnoInterfaceProxy^>(aProxy);
        if (proxyImpl)
            oId= proxyImpl->getOid();
    }

    if (oId == nullptr)
    {
        StringBuilder ^ buf= gcnew StringBuilder(256);
        bool bFirst = new bool(false);
        System::Threading::Monitor::Enter(Cli_environment::typeid);
        try {
            buf->Append(m_IDGen->GetId(obj, bFirst));
        } finally
        {
            System::Threading::Monitor::Exit(Cli_environment::typeid);
        }

        buf->Append(sOidPart);
        oId= buf->ToString();
    }
    return oId;
}
} //namespace cli_uno

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
