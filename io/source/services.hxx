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

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace io_acceptor{
    Reference< XInterface > acceptor_CreateInstance( const Reference< XComponentContext > & xCtx);
    OUString acceptor_getImplementationName();
    Sequence< OUString > acceptor_getSupportedServiceNames();
}

namespace stoc_connector {
    Reference< XInterface > connector_CreateInstance( const Reference< XComponentContext > & xCtx);
    OUString connector_getImplementationName();
    Sequence< OUString > connector_getSupportedServiceNames();
}

namespace io_TextInputStream {
    Reference< XInterface > TextInputStream_CreateInstance(SAL_UNUSED_PARAMETER const Reference< XComponentContext > &);
    OUString TextInputStream_getImplementationName();
    Sequence< OUString > TextInputStream_getSupportedServiceNames();
}

namespace io_TextOutputStream {
    Reference< XInterface > TextOutputStream_CreateInstance(SAL_UNUSED_PARAMETER const Reference< XComponentContext > &);
    OUString TextOutputStream_getImplementationName();
    Sequence< OUString > TextOutputStream_getSupportedServiceNames();
}

namespace io_stm {
    /// @throws Exception
    Reference< XInterface > OPipeImpl_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString    OPipeImpl_getImplementationName();
    Sequence<OUString> OPipeImpl_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > ODataInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString    ODataInputStream_getImplementationName();
    Sequence<OUString> ODataInputStream_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > ODataOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString ODataOutputStream_getImplementationName();
    Sequence<OUString> ODataOutputStream_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > OMarkableOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString OMarkableOutputStream_getImplementationName();
    Sequence<OUString> OMarkableOutputStream_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > OMarkableInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString    OMarkableInputStream_getImplementationName() ;
    Sequence<OUString> OMarkableInputStream_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > OObjectOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString OObjectOutputStream_getImplementationName();
    Sequence<OUString> OObjectOutputStream_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > OObjectInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString    OObjectInputStream_getImplementationName() ;
    Sequence<OUString> OObjectInputStream_getSupportedServiceNames();

    /// @throws Exception
    Reference< XInterface > OPumpImpl_CreateInstance( const Reference< XComponentContext > & rSMgr );
    OUString OPumpImpl_getImplementationName();
    Sequence<OUString> OPumpImpl_getSupportedServiceNames();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
