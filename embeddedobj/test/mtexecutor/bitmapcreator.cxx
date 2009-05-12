/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmapcreator.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_embeddedobj.hxx"

#include "bitmapcreator.hxx"

#include <vcl/bitmapex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL VCLBitmapCreator::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.BitmapCreator");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.BitmapCreator");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL VCLBitmapCreator::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.BitmapCreator");
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

