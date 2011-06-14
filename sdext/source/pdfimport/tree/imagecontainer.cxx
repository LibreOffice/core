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
#include "precompiled_sdext.hxx"

#include "imagecontainer.hxx"
#include "genericelements.hxx"
#include "xmlemitter.hxx"

#include <rtl/ustrbuf.hxx>
#include <osl/file.h>
#include <rtl/crc.h>

#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>

#include <boost/bind.hpp>

using namespace com::sun::star;

namespace pdfi
{

namespace
{

static const sal_Char aBase64EncodeTable[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

rtl::OUString encodeBase64( const sal_Int8* i_pBuffer, const sal_uInt32 i_nBufferLength )
{
    rtl::OUStringBuffer aBuf( (i_nBufferLength+1) * 4 / 3 );
    const sal_Int32 nRemain(i_nBufferLength%3);
    const sal_Int32 nFullTripleLength( i_nBufferLength - (i_nBufferLength%3));
    sal_Int32 nBufPos( 0 );
    for( sal_Int32 i = 0; i < nFullTripleLength; i += 3, nBufPos += 4 )
    {
        const sal_Int32 nBinary = (((sal_uInt8)i_pBuffer[i + 0]) << 16) +
                                  (((sal_uInt8)i_pBuffer[i + 1]) <<  8) +
                                  ((sal_uInt8)i_pBuffer[i + 2]);

        aBuf.appendAscii("====");

        sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinary & 0xFC0000) >> 18));
        aBuf.setCharAt(nBufPos, aBase64EncodeTable [nIndex]);

        nIndex = static_cast<sal_uInt8>((nBinary & 0x3F000) >> 12);
        aBuf.setCharAt(nBufPos+1, aBase64EncodeTable [nIndex]);

        nIndex = static_cast<sal_uInt8>((nBinary & 0xFC0) >> 6);
        aBuf.setCharAt(nBufPos+2, aBase64EncodeTable [nIndex]);

        nIndex = static_cast<sal_uInt8>((nBinary & 0x3F));
        aBuf.setCharAt(nBufPos+3, aBase64EncodeTable [nIndex]);
    }
    if( nRemain > 0 )
    {
        aBuf.appendAscii("====");
        sal_Int32 nBinary( 0 );
        const sal_Int32 nStart(i_nBufferLength-nRemain);
        switch(nRemain)
        {
            case 1: nBinary = ((sal_uInt8)i_pBuffer[nStart + 0]) << 16;
                break;
            case 2: nBinary = (((sal_uInt8)i_pBuffer[nStart + 0]) << 16) +
                              (((sal_uInt8)i_pBuffer[nStart + 1]) <<  8);
                break;
        }
        sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinary & 0xFC0000) >> 18));
        aBuf.setCharAt(nBufPos, aBase64EncodeTable [nIndex]);

        nIndex = static_cast<sal_uInt8>((nBinary & 0x3F000) >> 12);
        aBuf.setCharAt(nBufPos+1, aBase64EncodeTable [nIndex]);

        if( nRemain == 2 )
        {
            nIndex = static_cast<sal_uInt8>((nBinary & 0xFC0) >> 6);
            aBuf.setCharAt(nBufPos+2, aBase64EncodeTable [nIndex]);
        }
    }

    return aBuf.makeStringAndClear();
}

} // namespace

ImageContainer::ImageContainer() :
    m_aImages()
{}

ImageId ImageContainer::addImage( const uno::Sequence<beans::PropertyValue>& xBitmap )
{
    m_aImages.push_back( xBitmap );
    return m_aImages.size()-1;
}

void ImageContainer::writeBase64EncodedStream( ImageId nId, EmitContext& rContext )
{
    OSL_ASSERT( nId >= 0 && nId < ImageId( m_aImages.size()) );

    const uno::Sequence<beans::PropertyValue>& rEntry( m_aImages[nId] );

    // find "InputSequence" property
    const beans::PropertyValue* pAry(rEntry.getConstArray());
    const sal_Int32             nLen(rEntry.getLength());
    const beans::PropertyValue* pValue(
        std::find_if(pAry,pAry+nLen,
                     boost::bind(comphelper::TPropertyValueEqualFunctor(),
                                 _1,
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputSequence")))));
    OSL_ENSURE( pValue != pAry+nLen,
                "InputSequence not found" );

    uno::Sequence<sal_Int8> aData;
    if( !(pValue->Value >>= aData) )
        OSL_FAIL("Wrong data type");

    rContext.rEmitter.write( encodeBase64( aData.getConstArray(), aData.getLength() ));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
