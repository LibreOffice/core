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

#ifndef INCLUDED_EXTENSIONS_SOURCE_OLE_OLE2UNO_HXX
#define INCLUDED_EXTENSIONS_SOURCE_OLE_OLE2UNO_HXX


#ifdef _MSC_VER
#pragma once
#endif

#include <wincrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
#include <sal/types.h>
#include <typelib/typeclass.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/process.h>
#include <rtl/uuid.h>

#define UNO_2_OLE_EXCEPTIONCODE 1001
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace osl;
using namespace std;

namespace ole_adapter
{

const VARTYPE getVarType( const Any& val);
/* creates a Type object for a given type name.

    The function returns false if the name does not represent
    a valid type.
*/
bool getType( BSTR name, Type & type);
void o2u_attachCurrentThread();

class BridgeRuntimeError
{
public:
    explicit BridgeRuntimeError(const OUString& sMessage)
        : message(sMessage)
    {
    }
    OUString message;
};


Mutex* getBridgeMutex();

} // end namespace



#endif // INCLUDED_EXTENSIONS_SOURCE_OLE_OLE2UNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
