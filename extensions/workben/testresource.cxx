/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testresource.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:36:10 $
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
#include "precompiled_extensions.hxx"

#include <smart/com/sun/star/registry/XImplementationRegistration.hxx>
#include <smart/com/sun/star/script/XInvocation.hxx>

#include <rtl/ustring.hxx>
#include <vos/dynload.hxx>
#include <vos/diagnose.hxx>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>

using namespace rtl;
using namespace vos;
using namespace usr;

class MyApp : public Application
{
public:
    void        Main();
};

MyApp aMyApp;

// -----------------------------------------------------------------------

void MyApp::Main()
{
    XMultiServiceFactoryRef xSMgr = createRegistryServiceManager();
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XInterfaceRef x = xSMgr->createInstance( L"com.sun.star.registry.ImplementationRegistration" );
    XImplementationRegistrationRef xReg( x, USR_QUERY );
    sal_Char szBuf[1024];
    ORealDynamicLoader::computeModuleName( "res", szBuf, 1024 );
    UString aDllName( StringToOUString( szBuf, CHARSET_SYSTEM ) );
    xReg->registerImplementation( L"com.sun.star.loader.SharedLibrary", aDllName, XSimpleRegistryRef() );

    x = xSMgr->createInstance( L"com.sun.star.resource.VclStringResourceLoader" );
    XInvocationRef xResLoader( x, USR_QUERY );
    XIntrospectionAccessRef xIntrospection = xResLoader->getIntrospection();
    UString aFileName( L"TestResource" );
    UsrAny aVal;
    aVal.setString( aFileName );
    xResLoader->setValue( L"FileName", aVal );

    Sequence< UsrAny > Args( 1 );
    Sequence< INT16 > OutPos;
    Sequence< UsrAny > OutArgs;
    Args.getArray()[0].setINT32( 1000 );

    BOOL b = xResLoader->invoke( L"hasString", Args, OutPos, OutArgs ).getBOOL();
    VOS_ENSHURE( b, "hasString" );

    UString aStr = xResLoader->invoke( L"getString", Args, OutPos, OutArgs ).getString();
    VOS_ENSHURE( aStr == L"Hello", "getString" );

    Args.getArray()[0].setINT32( 1001 );
    b = xResLoader->invoke( L"hasString", Args, OutPos, OutArgs ).getBOOL();
    VOS_ENSHURE( !b, "!hasString" );

    xReg->revokeImplementation( aDllName, XSimpleRegistryRef() );
}

