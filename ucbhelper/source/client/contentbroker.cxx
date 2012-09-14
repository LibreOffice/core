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

#include "sal/config.h"

#ifdef ANDROID

#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/bootstrap.hxx>

using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

//TODO: Is this relevant still?
extern "C" __attribute__ ((visibility("default"))) void
InitUCBHelper()
{
    Reference< XComponentContext > xCtx;
    try
    {
        xCtx = ::cppu::defaultBootstrap_InitialComponentContext();
    }
    catch( Exception& )
    {
    }

    if( !xCtx.is() )
    {
        fprintf( stderr,
                 "Could not bootstrap UNO, installation must be in disorder. Exiting.\n" );
        exit( 1 );
    }

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    UniversalContentBroker::create( xCtx );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
