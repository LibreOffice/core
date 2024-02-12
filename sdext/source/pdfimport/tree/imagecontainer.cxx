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


#include <imagecontainer.hxx>
#include <genericelements.hxx>
#include <xmlemitter.hxx>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;

namespace pdfi
{

namespace
{

const char aBase64EncodeTable[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

OUString encodeBase64( const sal_Int8* i_pBuffer, const sal_Int32 i_nBufferLength )
{
    OUStringBuffer aBuf( (i_nBufferLength+1) * 4 / 3 );
    const sal_Int32 nRemain(i_nBufferLength%3);
    const sal_Int32 nFullTripleLength( i_nBufferLength - (i_nBufferLength%3));
    sal_Int32 nBufPos( 0 );
    for( sal_Int32 i = 0; i < nFullTripleLength; i += 3, nBufPos += 4 )
    {
        const sal_Int32 nBinary = (static_cast<sal_uInt8>(i_pBuffer[i + 0]) << 16) +
                                  (static_cast<sal_uInt8>(i_pBuffer[i + 1]) <<  8) +
                                  static_cast<sal_uInt8>(i_pBuffer[i + 2]);

        aBuf.append("====");

        sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinary & 0xFC0000) >> 18));
        aBuf[nBufPos] = aBase64EncodeTable [nIndex];

        nIndex = static_cast<sal_uInt8>((nBinary & 0x3F000) >> 12);
        aBuf[nBufPos+1] = aBase64EncodeTable [nIndex];

        nIndex = static_cast<sal_uInt8>((nBinary & 0xFC0) >> 6);
        aBuf[nBufPos+2] = aBase64EncodeTable [nIndex];

        nIndex = static_cast<sal_uInt8>((nBinary & 0x3F));
        aBuf[nBufPos+3] = aBase64EncodeTable [nIndex];
    }
    if( nRemain > 0 )
    {
        aBuf.append("====");
        sal_Int32 nBinary( 0 );
        const sal_Int32 nStart(i_nBufferLength-nRemain);
        switch(nRemain)
        {
            case 1: nBinary = static_cast<sal_uInt8>(i_pBuffer[nStart + 0]) << 16;
                break;
            case 2: nBinary = (static_cast<sal_uInt8>(i_pBuffer[nStart + 0]) << 16) +
                              (static_cast<sal_uInt8>(i_pBuffer[nStart + 1]) <<  8);
                break;
        }
        sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinary & 0xFC0000) >> 18));
        aBuf[nBufPos] = aBase64EncodeTable [nIndex];

        nIndex = static_cast<sal_uInt8>((nBinary & 0x3F000) >> 12);
        aBuf[nBufPos+1] = aBase64EncodeTable [nIndex];

        if( nRemain == 2 )
        {
            nIndex = static_cast<sal_uInt8>((nBinary & 0xFC0) >> 6);
            aBuf[nBufPos+2] = aBase64EncodeTable [nIndex];
        }
    }

    return aBuf.makeStringAndClear();
}

} // namespace

ImageContainer::ImageContainer()
{}

ImageId ImageContainer::addImage( const uno::Sequence<beans::PropertyValue>& xBitmap )
{
    m_aImages.push_back( xBitmap );
    return m_aImages.size()-1;
}

void ImageContainer::writeBase64EncodedStream( ImageId nId, EmitContext& rContext )
{
    OSL_ASSERT( nId >= 0 && o3tl::make_unsigned(nId) < m_aImages.size() );

    const uno::Sequence<beans::PropertyValue>& rEntry( m_aImages[nId] );

    // find "InputSequence" property
    const beans::PropertyValue* pAry(rEntry.getConstArray());
    const sal_Int32             nLen(rEntry.getLength());
    const beans::PropertyValue* pValue(
        std::find_if(pAry, pAry+nLen,
            [] (beans::PropertyValue const& v) -> bool {
                return v.Name == "InputSequence";
            }));

    if (pValue == pAry + nLen )
    {
        SAL_WARN("sdext.pdfimport", "InputSequence not found");
        return;
    }

    uno::Sequence<sal_Int8> aData;
    if( !(pValue->Value >>= aData) )
    {
        SAL_WARN("sdext.pdfimport", "Wrong data type");
        return;
    }

    rContext.rEmitter.write( encodeBase64( aData.getConstArray(), aData.getLength() ));
}

OUString ImageContainer::asBase64EncodedString( ImageId nId ) const
{
    OSL_ASSERT( nId >= 0 && o3tl::make_unsigned(nId) < m_aImages.size() );

    const uno::Sequence<beans::PropertyValue>& rEntry( m_aImages[nId] );

    // find "InputSequence" property
    const beans::PropertyValue* pAry(rEntry.getConstArray());
    const sal_Int32             nLen(rEntry.getLength());
    const beans::PropertyValue* pValue(
        std::find_if(pAry, pAry+nLen,
            [] (beans::PropertyValue const& v) -> bool {
                return v.Name == "InputSequence";
            }));

    if (pValue == pAry + nLen )
    {
        SAL_WARN("sdext.pdfimport", "InputSequence not found");
        return "";
    }

    uno::Sequence<sal_Int8> aData;
    if( !(pValue->Value >>= aData) )
    {
        SAL_WARN("sdext.pdfimport", "Wrong data type");
        return "";
    }

    return encodeBase64( aData.getConstArray(), aData.getLength() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
