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
#include "precompiled_cppuhelper.hxx"

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
using namespace ::rtl;

static sal_Bool tryService(const char * serviceName)
{
    // use micro deployment to create initial context
    OUString libraryFileUrl;
    ::osl::Module::getUrlFromAddress((void *)tryService, libraryFileUrl);

    OUString iniName = libraryFileUrl.copy(0, libraryFileUrl.lastIndexOf((sal_Unicode)'.')); // cut the library extension
    iniName += OUString(RTL_CONSTASCII_USTRINGPARAM(SAL_CONFIGFILE(""))); // add the rc file extension

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
