/*************************************************************************
 *
 *  $RCSfile: unodoc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:48:56 $
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

// System - Includes -----------------------------------------------------

#include <tools/string.hxx>
#include <sfx2/docfac.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include "swdll.hxx"
#include "docsh.hxx"
#include "globdoc.hxx"
#include "wdocsh.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::com::sun::star;

//============================================================
// com.sun.star.comp.Writer.TextDocument

uno::Sequence< ::rtl::OUString > SAL_CALL SwTextDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< ::rtl::OUString > aRet ( 2 );
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.TextDocument" ) );
    pArray[1] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.document.OfficeDocument" ) );

    return aRet;
}

::rtl::OUString SAL_CALL SwTextDocument_getImplementationName() throw()
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.TextDocument" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwTextDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > &rSMgr )
    throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    SfxObjectShell* pShell = new SwDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

//============================================================
// com.sun.star.comp.Writer.WebDocument

uno::Sequence< ::rtl::OUString > SAL_CALL SwWebDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< ::rtl::OUString > aRet ( 3 );
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.TextDocument" ) );
    pArray[1] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.document.OfficeDocument" ) );
    pArray[2] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.WebDocument" ) );

    return aRet;
}

::rtl::OUString SAL_CALL SwWebDocument_getImplementationName() throw()
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.WebDocument" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwWebDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > &rSMgr )
    throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    SfxObjectShell* pShell = new SwWebDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

//============================================================
// com.sun.star.comp.Writer.GlobalDocument

uno::Sequence< ::rtl::OUString > SAL_CALL SwGlobalDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< ::rtl::OUString > aRet ( 3 );
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.TextDocument" ) );
    pArray[1] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.document.OfficeDocument" ) );
    pArray[2] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.GlobalDocument" ) );

    return aRet;
}

::rtl::OUString SAL_CALL SwGlobalDocument_getImplementationName() throw()
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.GlobalDocument" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwGlobalDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > &rSMgr )
    throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    SfxObjectShell* pShell = new SwGlobalDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

