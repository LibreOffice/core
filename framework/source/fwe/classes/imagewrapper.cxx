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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <classes/imagewrapper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/stream.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

namespace framework
{

static Sequence< sal_Int8 > impl_getStaticIdentifier()
{
    static sal_uInt8 pGUID[16] = { 0x46, 0xAD, 0x69, 0xFB, 0xA7, 0xBE, 0x44, 0x83, 0xB2, 0xA7, 0xB3, 0xEC, 0x59, 0x4A, 0xB7, 0x00 };
    static ::com::sun::star::uno::Sequence< sal_Int8 > seqID((sal_Int8*)pGUID,16) ;
    return seqID ;
}


ImageWrapper::ImageWrapper( const Image& aImage ) : ThreadHelpBase( &Application::GetSolarMutex() )
                                                    ,   m_aImage( aImage )
{
}


ImageWrapper::~ImageWrapper()
{
}


Sequence< sal_Int8 > ImageWrapper::GetUnoTunnelId()
{
    return impl_getStaticIdentifier();
}

// XBitmap
com::sun::star::awt::Size SAL_CALL ImageWrapper::getSize() throw ( RuntimeException )
{
    SolarMutexGuard aGuard;

    BitmapEx    aBitmapEx( m_aImage.GetBitmapEx() );
    Size        aBitmapSize( aBitmapEx.GetSizePixel() );

    return com::sun::star::awt::Size( aBitmapSize.Width(), aBitmapSize.Height() );
}

Sequence< sal_Int8 > SAL_CALL ImageWrapper::getDIB() throw ( RuntimeException )
{
    SolarMutexGuard aGuard;

    SvMemoryStream aMem;
    aMem << m_aImage.GetBitmapEx().GetBitmap();
    return Sequence< sal_Int8 >( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

Sequence< sal_Int8 > SAL_CALL ImageWrapper::getMaskDIB() throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    BitmapEx    aBmpEx( m_aImage.GetBitmapEx() );

    if ( aBmpEx.IsAlpha() )
    {
        SvMemoryStream aMem;
        aMem << aBmpEx.GetAlpha().GetBitmap();
        return Sequence< sal_Int8 >( (sal_Int8*) aMem.GetData(), aMem.Tell() );
    }
    else if ( aBmpEx.IsTransparent() )
    {
        SvMemoryStream aMem;
        aMem << aBmpEx.GetMask();
        return Sequence< sal_Int8 >( (sal_Int8*) aMem.GetData(), aMem.Tell() );
    }

    return Sequence< sal_Int8 >();
}

// XUnoTunnel
sal_Int64 SAL_CALL ImageWrapper::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw ( RuntimeException )
{
    if ( aIdentifier == impl_getStaticIdentifier() )
        return reinterpret_cast< sal_Int64 >( this );
    else
        return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
