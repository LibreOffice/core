/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testdefaultbootstrapping.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:19:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include <sal/main.h>



#include <cstdio>

#include <rtl/process.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
            if (arg.getLength())
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
