/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// simple client application registering and using the counter component.
#include "sal/config.h"

#include <cstdlib>
#include <stdio.h>

#include <sal/main.h>
#include <rtl/ustring.hxx>

#include <osl/diagnose.h>

#include <cppuhelper/bootstrap.hxx>

// generated c++ interfaces
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <foo/XCountable.hpp>


using namespace foo;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using namespace ::rtl;


SAL_IMPLEMENT_MAIN()
{
    try {

        Reference< XComponentContext > xContext(::cppu::defaultBootstrap_InitialComponentContext());
        OSL_ENSURE( xContext.is(), "### bootstrap failed!" );

        Reference< XMultiComponentFactory > xMgr = xContext->getServiceManager();
        OSL_ENSURE( xMgr.is(), "### cannot get initial service manager!" );

        Reference< XInterface > xx = xMgr->createInstanceWithContext( "foo.Counter", xContext);

        OSL_ENSURE( xx.is(), "### cannot get service instance of \"foo.Counter\"!" );

        Reference< XCountable > xCount( xx, UNO_QUERY );
        OSL_ENSURE( xCount.is(), "### cannot query XCountable interface of service instance \"foo.Counter\"!" );

        if (xCount.is())
        {
             xCount->setCount( 42 );
             fprintf( stdout , "%d," , (int)xCount->getCount() );
             fprintf( stdout , "%d," , (int)xCount->increment() );
             fprintf( stdout , "%d\n" , (int)xCount->decrement() );
        }

        Reference< XComponent >::query( xContext )->dispose();
        return EXIT_SUCCESS;

    } catch( Exception& e) {
        printf("Error: caught exception:\n       %s\n",
               OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        return EXIT_FAILURE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
