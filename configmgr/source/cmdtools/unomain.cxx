/*************************************************************************
 *
 *  $RCSfile: unomain.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-10-24 15:29:25 $
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

        fprintf( stderr, "%s - Unhandled exception caught in main: \"%s\"\n", sApp.getStr(), sMessage.getStr() );

        return 42;
    }
}

