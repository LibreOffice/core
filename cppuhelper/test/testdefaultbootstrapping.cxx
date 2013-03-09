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


#include <sal/main.h>



#include <cstdio>

#include <rtl/process.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;


SAL_IMPLEMENT_MAIN()
{
    sal_Bool result = sal_True;

    try {
        Reference<XComponentContext> xComponentContext = defaultBootstrap_InitialComponentContext();

        Reference<XMultiServiceFactory> smgr(xComponentContext->getServiceManager(), UNO_QUERY);

        for(sal_uInt32 i = 0; i < rtl_getAppCommandArgCount(); ++ i) {
            OUString arg;

            rtl_getAppCommandArg(i, &arg.pData);
            if (!arg.isEmpty())
            {
                Reference<XInterface> xInterface = smgr->createInstance(arg);
                OString tmp = OUStringToOString(arg, RTL_TEXTENCODING_ASCII_US);
#if OSL_DEBUG_LEVEL > 1
                fprintf(stderr, "got the %s service %p\n", tmp.getStr(), xInterface.get());
#endif

                result = result && (xInterface.get() != 0);
            }
        }
    }
    catch(Exception & exception) {
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);

        fprintf(stderr, "an exception occurred: %s\n", message.getStr());
    }

#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE("---------------------------------- %i", result);
#endif

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
