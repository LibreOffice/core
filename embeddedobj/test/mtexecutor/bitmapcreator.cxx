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

#include "bitmapcreator.hxx"

#include <vcl/bitmapex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL VCLBitmapCreator::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = "com.sun.star.embed.BitmapCreator";
    aRet[1] = "com.sun.star.comp.embed.BitmapCreator";
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL VCLBitmapCreator::impl_staticGetImplementationName()
{
    return ::rtl::OUString("com.sun.star.comp.embed.BitmapCreator");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL VCLBitmapCreator::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new VCLBitmapCreator( xServiceManager ) );
}

//-------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL VCLBitmapCreator::createInstance()
        throw ( uno::Exception,
                uno::RuntimeException)
{
    BitmapEx aBitmap;
    uno::Reference< uno::XInterface> aResult( VCLUnoHelper::CreateBitmap( aBitmap ), uno::UNO_QUERY );

    return aResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL VCLBitmapCreator::createInstanceWithArguments(
                                                const uno::Sequence< uno::Any >& aArguments )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    if ( aArguments.getLength() != 1 )
        throw uno::Exception(); // TODO

    uno::Sequence< sal_Int8 > aOrigBitmap;
    if ( !( aArguments[0] >>= aOrigBitmap ) )
        throw uno::Exception(); // TODO

    BitmapEx aBitmap;
    SvMemoryStream aStream( aOrigBitmap.getArray(), aOrigBitmap.getLength(), STREAM_READ );
    aStream >> aBitmap;
    if ( aStream.GetError() )
        throw uno::Exception(); // TODO

    uno::Reference< uno::XInterface > aResult( VCLUnoHelper::CreateBitmap( aBitmap ), uno::UNO_QUERY );

    return aResult;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL VCLBitmapCreator::getImplementationName()
        throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL VCLBitmapCreator::supportsService( const ::rtl::OUString& ServiceName )
        throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL VCLBitmapCreator::getSupportedServiceNames()
        throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
