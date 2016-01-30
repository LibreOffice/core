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


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <cstdio>

#include <osl/module.hxx>
#include <rtl/process.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

static sal_Bool tryService(const char * serviceName)
{
    // use micro deployment to create initial context
    OUString libraryFileUrl;
    ::osl::Module::getUrlFromAddress((void *)tryService, libraryFileUrl);

    OUString iniName = libraryFileUrl.copy(0, libraryFileUrl.lastIndexOf((sal_Unicode)'.')); // cut the library extension
    iniName += SAL_CONFIGFILE(""); // add the rc file extension

#if OSL_DEBUG_LEVEL > 1
    OString sIniName = OUStringToOString(iniName, RTL_TEXTENCODING_ASCII_US);
    fprintf(stderr, "defbootstrap.cxx:tryService: using ini: %s\n", sIniName.getStr());
#endif

    sal_Bool result = sal_True;

    try {
        Reference<XComponentContext> xComponentContext = defaultBootstrap_InitialComponentContext(iniName);
        Reference<XMultiServiceFactory> smgr(xComponentContext->getServiceManager(), UNO_QUERY);

        OUString arg = OUString::createFromAscii(serviceName);
        Reference<XInterface> xInterface = smgr->createInstance(arg);

#if OSL_DEBUG_LEVEL > 1
        fprintf(stderr, "got the %s service %p\n", serviceName, xInterface.get());
#endif
        result = result && (xInterface.get() != 0);
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

XS(XS_UNO_tryService)
{
    dXSARGS;
    if (items != 1)
    Perl_croak(aTHX_ "Usage: UNO::tryService(input)");
    {
        const char * input = (const char *)SvPV(ST(0),PL_na);
        int RETVAL;
        dXSTARG;

        RETVAL = tryService(input);
        XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}

extern "C" {
XS(boot_UNO)
{
    dXSARGS;
    char* file = __FILE__;

/*    XS_VERSION_BOOTCHECK ;*/

    newXS("UNO::tryService", XS_UNO_tryService, file);
    XSRETURN_YES;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
