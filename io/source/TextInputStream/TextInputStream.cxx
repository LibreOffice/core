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

#include <string.h>
#include <osl/diagnose.h>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XTextInputStream2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include "services.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.TextInputStream"
#define SERVICE_NAME "com.sun.star.io.TextInputStream"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;

namespace io_TextInputStream
{

// Implementation XTextInputStream

#define INITIAL_UNICODE_BUFFER_CAPACITY     0x100
#define READ_BYTE_COUNT                     0x100

class OTextInputStream : public WeakImplHelper< XTextInputStream2, XServiceInfo >
{
    Reference< XInputStream > mxStream;

    // Encoding
    OUString mEncoding;
    bool mbEncodingInitialized;
    rtl_TextToUnicodeConverter  mConvText2Unicode;
    rtl_TextToUnicodeContext    mContextText2Unicode;
    Sequence<sal_Int8>          mSeqSource;

    // Internal buffer for characters that are already converted successfully
    sal_Unicode* mpBuffer;
    sal_Int32 mnBufferSize;
    sal_Int32 mnCharsInBuffer;
    bool mbReachedEOF;

    void implResizeBuffer();
    /// @throws IOException
    /// @throws RuntimeException
    OUString implReadString( const Sequence< sal_Unicode >& Delimiters,
        bool bRemoveDelimiter, bool bFindLineEnd );
    /// @throws IOException
    /// @throws RuntimeException
    sal_Int32 implReadNext();

public:
    OTextInputStream();
    virtual ~OTextInputStream() override;

    // Methods XTextInputStream
    virtual OUString SAL_CALL readLine(  ) override;
    virtual OUString SAL_CALL readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter ) override;
    virtual sal_Bool SAL_CALL isEOF(  ) override;
    virtual void SAL_CALL setEncoding( const OUString& Encoding ) override;

    // Methods XInputStream
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;

    // Methods XActiveDataSink
    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& aStream ) override;
    virtual Reference< XInputStream > SAL_CALL getInputStream() override;

    // Methods XServiceInfo
        virtual OUString              SAL_CALL getImplementationName() override;
        virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames() override;
        virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) override;
};

OTextInputStream::OTextInputStream()
    : mbEncodingInitialized(false)
    , mConvText2Unicode(nullptr)
    , mContextText2Unicode(nullptr)
    , mSeqSource(READ_BYTE_COUNT)
    , mpBuffer(nullptr)
    , mnBufferSize(0)
    , mnCharsInBuffer(0)
    , mbReachedEOF(false)
{
}

OTextInputStream::~OTextInputStream()
{
    if( mbEncodingInitialized )
    {
        rtl_destroyTextToUnicodeContext( mConvText2Unicode, mContextText2Unicode );
        rtl_destroyTextToUnicodeConverter( mConvText2Unicode );
    }

    delete[] mpBuffer;
}

void OTextInputStream::implResizeBuffer()
{
    sal_Int32 nNewBufferSize = mnBufferSize * 2;
    sal_Unicode* pNewBuffer = new sal_Unicode[ nNewBufferSize ];
    memcpy( pNewBuffer, mpBuffer, mnCharsInBuffer * sizeof( sal_Unicode ) );
    delete[] mpBuffer;
    mpBuffer = pNewBuffer;
    mnBufferSize = nNewBufferSize;
}


// XTextInputStream

OUString OTextInputStream::readLine(  )
{
    static Sequence< sal_Unicode > aDummySeq;
    return implReadString( aDummySeq, true, true );
}

OUString OTextInputStream::readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter )
{
    return implReadString( Delimiters, bRemoveDelimiter, false );
}

sal_Bool OTextInputStream::isEOF()
{
    bool bRet = false;
    if( mnCharsInBuffer == 0 && mbReachedEOF )
        bRet = true;
    return bRet;
}


OUString OTextInputStream::implReadString( const Sequence< sal_Unicode >& Delimiters,
                                           bool bRemoveDelimiter, bool bFindLineEnd )
{
    OUString aRetStr;
    if( !mbEncodingInitialized )
    {
        OUString aUtf8Str("utf8");
        setEncoding( aUtf8Str );
    }
    if( !mbEncodingInitialized )
        return aRetStr;

    if( !mpBuffer )
    {
        mnBufferSize = INITIAL_UNICODE_BUFFER_CAPACITY;
        mpBuffer = new sal_Unicode[ mnBufferSize ];
    }

    // Only for bFindLineEnd
    sal_Unicode cLineEndChar1 = 0x0D;
    sal_Unicode cLineEndChar2 = 0x0A;

    sal_Int32 nBufferReadPos = 0;
    sal_Int32 nCopyLen = 0;
    bool bFound = false;
    bool bFoundFirstLineEndChar = false;
    sal_Unicode cFirstLineEndChar = 0;
    const sal_Unicode* pDelims = Delimiters.getConstArray();
    const sal_Int32 nDelimCount = Delimiters.getLength();
    while( !bFound )
    {
        // Still characters available?
        if( nBufferReadPos == mnCharsInBuffer )
        {
            // Already reached EOF? Then we can't read any more
            if( mbReachedEOF )
                break;

            // No, so read new characters
            if( !implReadNext() )
                break;
        }

        // Now there should be characters available
        // (otherwise the loop should have been breaked before)
        sal_Unicode c = mpBuffer[ nBufferReadPos++ ];

        if( bFindLineEnd )
        {
            if( bFoundFirstLineEndChar )
            {
                bFound = true;
                nCopyLen = nBufferReadPos - 2;
                if( c == cLineEndChar1 || c == cLineEndChar2 )
                {
                    // Same line end char -> new line break
                    if( c == cFirstLineEndChar )
                    {
                        nBufferReadPos--;
                    }
                }
                else
                {
                    // No second line end char
                    nBufferReadPos--;
                }
            }
            else if( c == cLineEndChar1 || c == cLineEndChar2 )
            {
                bFoundFirstLineEndChar = true;
                cFirstLineEndChar = c;
            }
        }
        else
        {
            for( sal_Int32 i = 0 ; i < nDelimCount ; i++ )
            {
                if( c == pDelims[ i ] )
                {
                    bFound = true;
                    nCopyLen = nBufferReadPos;
                    if( bRemoveDelimiter )
                        nCopyLen--;
                }
            }
        }
    }

    // Nothing found? Return all
    if( !nCopyLen && !bFound && mbReachedEOF )
        nCopyLen = nBufferReadPos;

    // Create string
    if( nCopyLen )
        aRetStr = OUString( mpBuffer, nCopyLen );

    // Copy rest of buffer
    memmove( mpBuffer, mpBuffer + nBufferReadPos,
        (mnCharsInBuffer - nBufferReadPos) * sizeof( sal_Unicode ) );
    mnCharsInBuffer -= nBufferReadPos;

    return aRetStr;
}


sal_Int32 OTextInputStream::implReadNext()
{
    sal_Int32 nFreeBufferSize = mnBufferSize - mnCharsInBuffer;
    if( nFreeBufferSize < READ_BYTE_COUNT )
        implResizeBuffer();
    nFreeBufferSize = mnBufferSize - mnCharsInBuffer;

    try
    {
        sal_Int32 nBytesToRead = READ_BYTE_COUNT;
        sal_Int32 nRead = mxStream->readSomeBytes( mSeqSource, nBytesToRead );
        sal_Int32 nTotalRead = nRead;
        if( nRead == 0 )
            mbReachedEOF = true;

        // Try to convert
        sal_uInt32 uiInfo;
        sal_Size nSrcCvtBytes = 0;
        sal_Size nTargetCount = 0;
        sal_Size nSourceCount = 0;
        while( true )
        {
            const sal_Int8 *pbSource = mSeqSource.getConstArray();

            // All invalid characters are transformed to the unicode undefined char
            nTargetCount += rtl_convertTextToUnicode(
                                mConvText2Unicode,
                                mContextText2Unicode,
                                reinterpret_cast<const char*>(&( pbSource[nSourceCount] )),
                                nTotalRead - nSourceCount,
                                mpBuffer + mnCharsInBuffer + nTargetCount,
                                nFreeBufferSize - nTargetCount,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                &uiInfo,
                                &nSrcCvtBytes );
            nSourceCount += nSrcCvtBytes;

            bool bCont = false;
            if( uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL )
            {
                implResizeBuffer();
                bCont = true;
            }

            if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL )
            {
                // read next byte
                static Sequence< sal_Int8 > aOneByteSeq( 1 );
                nRead = mxStream->readSomeBytes( aOneByteSeq, 1 );
                if( nRead == 0 )
                {
                    mbReachedEOF = true;
                    break;
                }

                sal_Int32 nOldLen = mSeqSource.getLength();
                nTotalRead++;
                if( nTotalRead > nOldLen )
                {
                    mSeqSource.realloc( nTotalRead );
                }
                mSeqSource.getArray()[ nOldLen ] = aOneByteSeq.getConstArray()[ 0 ];
                bCont = true;
            }

            if( bCont )
                continue;
            break;
        }

        mnCharsInBuffer += nTargetCount;
        return nTargetCount;
    }
    catch( NotConnectedException& )
    {
        throw IOException();
        //throw IOException( L"OTextInputStream::implReadString failed" );
    }
    catch( BufferSizeExceededException& )
    {
        throw IOException();
    }
}

void OTextInputStream::setEncoding( const OUString& Encoding )
{
    OString aOEncodingStr = OUStringToOString( Encoding, RTL_TEXTENCODING_ASCII_US );
    rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( aOEncodingStr.getStr() );
    if( RTL_TEXTENCODING_DONTKNOW == encoding )
        return;

    mbEncodingInitialized = true;
    mConvText2Unicode = rtl_createTextToUnicodeConverter( encoding );
    mContextText2Unicode = rtl_createTextToUnicodeContext( mConvText2Unicode );
    mEncoding = Encoding;
}


// XInputStream

sal_Int32 OTextInputStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    return mxStream->readBytes( aData, nBytesToRead );
}

sal_Int32 OTextInputStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return mxStream->readSomeBytes( aData, nMaxBytesToRead );
}

void OTextInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    mxStream->skipBytes( nBytesToSkip );
}

sal_Int32 OTextInputStream::available(  )
{
    return mxStream->available();
}

void OTextInputStream::closeInput(  )
{
    mxStream->closeInput();
}


// XActiveDataSink

void OTextInputStream::setInputStream( const Reference< XInputStream >& aStream )
{
    mxStream = aStream;
}

Reference< XInputStream > OTextInputStream::getInputStream()
{
    return mxStream;
}


Reference< XInterface > SAL_CALL TextInputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext > &)
{
    return Reference < XInterface >( static_cast<OWeakObject *>(new OTextInputStream()) );
}

OUString TextInputStream_getImplementationName()
{
    return OUString ( IMPLEMENTATION_NAME );
}

Sequence< OUString > TextInputStream_getSupportedServiceNames()
{
    Sequence< OUString > seqNames { SERVICE_NAME };
    return seqNames;
}

OUString OTextInputStream::getImplementationName()
{
    return TextInputStream_getImplementationName();
}

sal_Bool OTextInputStream::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > OTextInputStream::getSupportedServiceNames()
{
    return TextInputStream_getSupportedServiceNames();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
