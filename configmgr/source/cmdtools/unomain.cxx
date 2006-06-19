/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomain.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:21:55 $
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
    { (void)argc; (void)argv; }

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

