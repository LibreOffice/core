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

#ifndef INCLUDED_IO_SOURCE_SERVICES_HXX
#define INCLUDED_IO_SOURCE_SERVICES_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace io_acceptor{
    Reference< XInterface > SAL_CALL acceptor_CreateInstance( const Reference< XComponentContext > & xCtx);
    OUString acceptor_getImplementationName();
    Sequence< OUString > acceptor_getSupportedServiceNames();
}

namespace stoc_connector {
    Reference< XInterface > SAL_CALL connector_CreateInstance( const Reference< XComponentContext > & xCtx);
    OUString connector_getImplementationName();
    Sequence< OUString > connector_getSupportedServiceNames();
}

namespace io_TextInputStream {
    Reference< XInterface > SAL_CALL TextInputStream_CreateInstance(SAL_UNUSED_PARAMETER const Reference< XComponentContext > &);
    OUString TextInputStream_getImplementationName();
    Sequence< OUString > TextInputStream_getSupportedServiceNames();
}

namespace io_TextOutputStream {
    Reference< XInterface > SAL_CALL TextOutputStream_CreateInstance(SAL_UNUSED_PARAMETER const Reference< XComponentContext > &);
    OUString TextOutputStream_getImplementationName();
    Sequence< OUString > TextOutputStream_getSupportedServiceNames();
}

namespace io_stm {
    Reference< XInterface > SAL_CALL OPipeImpl_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
    OUString    OPipeImpl_getImplementationName();
    Sequence<OUString> OPipeImpl_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL ODataInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
    OUString    ODataInputStream_getImplementationName();
    Sequence<OUString> ODataInputStream_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL ODataOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
    OUString ODataOutputStream_getImplementationName();
    Sequence<OUString> ODataOutputStream_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL OMarkableOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
    OUString OMarkableOutputStream_getImplementationName();
    Sequence<OUString> OMarkableOutputStream_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL OMarkableInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
    OUString    OMarkableInputStream_getImplementationName() ;
    Sequence<OUString> OMarkableInputStream_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL OObjectOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw(Exception);
    OUString OObjectOutputStream_getImplementationName();
    Sequence<OUString> OObjectOutputStream_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL OObjectInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw(Exception);
    OUString    OObjectInputStream_getImplementationName() ;
    Sequence<OUString> OObjectInputStream_getSupportedServiceNames();

    Reference< XInterface > SAL_CALL OPumpImpl_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
    OUString OPumpImpl_getImplementationName();
    Sequence<OUString> OPumpImpl_getSupportedServiceNames();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
