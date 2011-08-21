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
#include "vbaaddin.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAddin::SwVbaAddin( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const rtl::OUString& rFileURL, sal_Bool bAutoload ) throw ( uno::RuntimeException ) :
    SwVbaAddin_BASE( rParent, rContext ), msFileURL( rFileURL ), mbAutoload( bAutoload ), mbInstalled( bAutoload )
{
}

SwVbaAddin::~SwVbaAddin()
{
}

::rtl::OUString SAL_CALL SwVbaAddin::getName() throw (uno::RuntimeException)
{
    rtl::OUString sName;
    INetURLObject aURL( msFileURL );
    ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    return sName;
}

void SAL_CALL
SwVbaAddin::setName( const rtl::OUString& ) throw ( css::uno::RuntimeException )
{
    throw uno::RuntimeException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(" Fail to set name")), uno::Reference< uno::XInterface >() );
}

::rtl::OUString SAL_CALL SwVbaAddin::getPath() throw (uno::RuntimeException)
{
    INetURLObject aURL( msFileURL );
    aURL.CutLastName();
    return aURL.GetURLPath();
}

::sal_Bool SAL_CALL SwVbaAddin::getAutoload() throw (uno::RuntimeException)
{
    return mbAutoload;
}

::sal_Bool SAL_CALL SwVbaAddin::getInstalled() throw (uno::RuntimeException)
{
    return mbInstalled;
}

void SAL_CALL SwVbaAddin::setInstalled( ::sal_Bool _installed ) throw (uno::RuntimeException)
{
    if( _installed != mbInstalled )
    {
        mbInstalled = _installed;
        // TODO: should call AutoExec and AutoExit etc.
    }
}

rtl::OUString&
SwVbaAddin::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAddin") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaAddin::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Addin" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
