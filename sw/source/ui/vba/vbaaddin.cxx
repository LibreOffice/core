/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "vbaaddin.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAddin::SwVbaAddin( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const OUString& rFileURL, sal_Bool bAutoload ) throw ( uno::RuntimeException ) :
    SwVbaAddin_BASE( rParent, rContext ), msFileURL( rFileURL ), mbAutoload( bAutoload ), mbInstalled( bAutoload )
{
}

SwVbaAddin::~SwVbaAddin()
{
}

OUString SAL_CALL SwVbaAddin::getName() throw (uno::RuntimeException, std::exception)
{
    OUString sName;
    INetURLObject aURL( msFileURL );
    ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    return sName;
}

void SAL_CALL
SwVbaAddin::setName( const OUString& ) throw ( css::uno::RuntimeException, std::exception )
{
    throw uno::RuntimeException(" Fail to set name", uno::Reference< uno::XInterface >() );
}

OUString SAL_CALL SwVbaAddin::getPath() throw (uno::RuntimeException, std::exception)
{
    INetURLObject aURL( msFileURL );
    aURL.CutLastName();
    return aURL.GetURLPath();
}

::sal_Bool SAL_CALL SwVbaAddin::getAutoload() throw (uno::RuntimeException, std::exception)
{
    return mbAutoload;
}

::sal_Bool SAL_CALL SwVbaAddin::getInstalled() throw (uno::RuntimeException, std::exception)
{
    return mbInstalled;
}

void SAL_CALL SwVbaAddin::setInstalled( ::sal_Bool _installed ) throw (uno::RuntimeException, std::exception)
{
    if( _installed != mbInstalled )
    {
        mbInstalled = _installed;
        // TODO: should call AutoExec and AutoExit etc.
    }
}

OUString
SwVbaAddin::getServiceImplName()
{
    return OUString("SwVbaAddin");
}

uno::Sequence< OUString >
SwVbaAddin::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Addin";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
