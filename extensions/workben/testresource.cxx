/*************************************************************************
 *
 *  $RCSfile: testresource.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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

#include <smart/com/sun/star/registry/XImplementationRegistration.hxx>
#include <smart/com/sun/star/script/XInvocation.hxx>

#include <rtl/ustring.hxx>
#include <vos/dynload.hxx>
#include <vos/diagnose.hxx>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>

#ifdef _USE_NAMESPACE
using namespace rtl;
using namespace vos;
using namespace usr;
#endif

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

