/*************************************************************************
 *
 *  $RCSfile: defbootstrap_lib.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:36:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
