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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cli_ure.hxx"

#include "Cli_environment.h"

#using <mscorlib.dll>
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

inline System::String* Cli_environment::createKey(System::String* oid, System::Type* t)
{
    return System::String::Concat(oid, t->get_FullName());
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


System::Object* Cli_environment::registerInterface(
    System::Object* obj, System::String* oid)
{
#if OSL_DEBUG_LEVEL >= 1
    //obj must be a transparent proxy
    OSL_ASSERT(RemotingServices::IsTransparentProxy(obj));
    _numRegisteredObjects ++;
#endif
    OSL_ASSERT( ! m_objects->ContainsKey(oid));
    m_objects->Add(oid, new WeakReference(obj));
    return obj;
}
System::Object* Cli_environment::registerInterface      (
    System::Object* obj, System::String* oid, System::Type* type)
{
#if OSL_DEBUG_LEVEL >= 1
    //obj must be a real cli object
    OSL_ASSERT( ! RemotingServices::IsTransparentProxy(obj));
    _numRegisteredObjects ++;
#endif
    System::String* key = createKey(oid, type);
    //see synchronization in map_uno2cli in cli_data.cxx
    OSL_ASSERT( ! m_objects->ContainsKey(key));
    m_objects->Add(key, new WeakReference(obj));
    return obj;
}

void Cli_environment::revokeInterface(System::String* oid, System::Type* type)
{
    System::String* key = type != NULL ? createKey(oid, type) : oid;
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

inline void Cli_environment::revokeInterface(System::String* oid)
{
    return revokeInterface(oid, NULL);
}

System::Object* Cli_environment::getRegisteredInterface(System::String* oid,
                                                        System::Type* type)
{
    //try if it is a UNO interface
    System::Object* ret = NULL;
    ret = m_objects->get_Item(oid);
    if (! ret)
    {
        //try if if it is a proxy for a cli object
        oid = createKey(oid, type);
        ret = m_objects->get_Item( oid );
    }
    if (ret != 0)
    {
        System::WeakReference* weakIface =
            static_cast< System::WeakReference * >( ret );
        ret = weakIface->Target;
    }
    if (ret == 0)
        m_objects->Remove( oid );
    return ret;
}

System::String* Cli_environment::getObjectIdentifier(System::Object* obj)
{
    System::String* oId= 0;
    RealProxy* aProxy= RemotingServices::GetRealProxy(obj);
    if (aProxy)
    {
        UnoInterfaceProxy* proxyImpl= dynamic_cast<UnoInterfaceProxy*>(aProxy);
        if (proxyImpl)
            oId= proxyImpl->getOid();
    }

    if (oId == 0)
    {
        StringBuilder * buf= new StringBuilder(256);
        bool bFirst = false;
        System::Threading::Monitor::Enter(__typeof(Cli_environment));
        try {
            buf->Append(m_IDGen->GetId(obj, & bFirst));
        } __finally
        {
            System::Threading::Monitor::Exit(__typeof(Cli_environment));
        }

        buf->Append(sOidPart);
        oId= buf->ToString();
    }
    return oId;
}
} //namespace cli_uno

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
