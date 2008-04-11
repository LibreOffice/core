/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unomain.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_configmgr.hxx"

#include <stdio.h>

#include "unomain.hxx"

#include <sal/main.h>

#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/bootstrap.hxx>

#include <rtl/string.hxx>

#include <rtl/process.h>
#include <osl/thread.h>
#include <osl/file.h>

namespace uno = ::com::sun::star::uno;

namespace
{
    struct AutoDispose
    {
        typedef uno::Reference< ::com::sun::star::lang::XComponent > Component;
        AutoDispose()
        : component()
        {}

        ~AutoDispose()
        {
            if (component.is())
                component->dispose();
        }

        void set( Component const & xComp )
        {
            component = xComp;
        }
        void set( uno::BaseReference const & xComp )
        {
            component.set(xComp,uno::UNO_QUERY);
        }

        Component component;
    };
}

::rtl::OUString SAL_CALL unoapp::getAppCommandURL()
{
    rtl::OUString aURL;
    OSL_VERIFY( osl_Process_E_None == osl_getExecutableFile(&aURL.pData) );
    return aURL;
}

::rtl::OUString SAL_CALL unoapp::getAppCommandPath()
{
    rtl::OUString const aURL = getAppCommandURL();

    rtl::OUString aPath;
    OSL_VERIFY( osl_File_E_None == osl_getSystemPathFromFileURL(aURL.pData,&aPath.pData) );

    return aPath;
}

::rtl::OUString SAL_CALL unoapp::getAppCommandName()
{
    rtl::OUString aURL = getAppCommandURL();

    sal_Int32 const nNameIndex = aURL.lastIndexOf( sal_Unicode('/') );

    return aURL.copy(nNameIndex+1);
}

uno::Sequence< ::rtl::OUString > SAL_CALL unoapp::getAppCommandArgs()
{
    sal_uInt32 const count = rtl_getAppCommandArgCount();

    uno::Sequence< ::rtl::OUString > aResult( count );
    ::rtl::OUString * pResult = aResult.getArray();

    for (sal_uInt32 i = 0; i < count; ++i)
    {
        rtl_getAppCommandArg( i, & pResult[i].pData);
    }

    return aResult;
}

bool unoapp::g_bQuiet = false;

SAL_IMPLEMENT_MAIN()
{
    AutoDispose aDisposer;
    try
    {
        uno::Reference< uno::XComponentContext > xUnoContext = cppu::defaultBootstrap_InitialComponentContext();

        aDisposer.component.set( xUnoContext, uno::UNO_QUERY );

        return unoapp::uno_main(xUnoContext);
    }
    catch (uno::Exception & e)
    {
        using namespace rtl;
        OString sApp = OUStringToOString( unoapp::getAppCommandName(), osl_getThreadTextEncoding() );
        OString sMessage = OUStringToOString( e.Message, osl_getThreadTextEncoding() );

        if (!unoapp::g_bQuiet)
            fprintf( stderr, "%s - Unhandled exception caught in main: \"%s\"\n", sApp.getStr(), sMessage.getStr() );

        return 42;
    }
}

