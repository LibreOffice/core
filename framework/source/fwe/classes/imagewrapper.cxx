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


#include <classes/imagewrapper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/stream.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/dibtools.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

namespace framework
{

static Sequence< sal_Int8 > impl_getStaticIdentifier()
{
    static const sal_uInt8 pGUID[16] = { 0x46, 0xAD, 0x69, 0xFB, 0xA7, 0xBE, 0x44, 0x83, 0xB2, 0xA7, 0xB3, 0xEC, 0x59, 0x4A, 0xB7, 0x00 };
    static ::com::sun::star::uno::Sequence< sal_Int8 > seqID((const sal_Int8*)pGUID,16) ;
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
    WriteDIB(m_aImage.GetBitmapEx().GetBitmap(), aMem, false, true);
    return Sequence< sal_Int8 >( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

Sequence< sal_Int8 > SAL_CALL ImageWrapper::getMaskDIB() throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    BitmapEx    aBmpEx( m_aImage.GetBitmapEx() );

    if ( aBmpEx.IsAlpha() )
    {
        SvMemoryStream aMem;
        WriteDIB(aBmpEx.GetAlpha().GetBitmap(), aMem, false, true);
        return Sequence< sal_Int8 >( (sal_Int8*) aMem.GetData(), aMem.Tell() );
    }
    else if ( aBmpEx.IsTransparent() )
    {
        SvMemoryStream aMem;
        WriteDIB(aBmpEx.GetMask(), aMem, false, true);
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
