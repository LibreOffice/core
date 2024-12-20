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

#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XTextInputStream2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <vector>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;


// Implementation XTextInputStream

#define INITIAL_UNICODE_BUFFER_CAPACITY     0x100
#define READ_BYTE_COUNT                     0x100

namespace {

class OTextInputStream : public WeakImplHelper< XTextInputStream2, XServiceInfo >
{
    Reference< XInputStream > mxStream;

    // Encoding
    bool mbEncodingInitialized;
    rtl_TextToUnicodeConverter  mConvText2Unicode;
    rtl_TextToUnicodeContext    mContextText2Unicode;
    Sequence<sal_Int8>          mSeqSource;

    // Internal buffer for characters that are already converted successfully
    std::vector<sal_Unicode> mvBuffer;
    sal_Int32 mnCharsInBuffer;
    bool mbReachedEOF;

    /// @throws IOException
    /// @throws RuntimeException
    OUString implReadString( const Sequence< sal_Unicode >& Delimiters,
        bool bRemoveDelimiter, bool bFindLineEnd );
    /// @throws IOException
    /// @throws RuntimeException
    sal_Int32 implReadNext();
    /// @throws RuntimeException
    void checkNull();

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

}

OTextInputStream::OTextInputStream()
    : mbEncodingInitialized(false)
    , mConvText2Unicode(nullptr)
    , mContextText2Unicode(nullptr)
    , mSeqSource(READ_BYTE_COUNT)
    , mvBuffer(INITIAL_UNICODE_BUFFER_CAPACITY, 0)
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
}

// Check uninitialized object

void OTextInputStream::checkNull()
{
    if (mxStream==nullptr){
        throw RuntimeException(u"Uninitialized object"_ustr);
    }
}

// XTextInputStream

OUString OTextInputStream::readLine(  )
{
    checkNull();
    static Sequence< sal_Unicode > aDummySeq;
    return implReadString( aDummySeq, true, true );
}

OUString OTextInputStream::readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter )
{
    checkNull();
    return implReadString( Delimiters, bRemoveDelimiter, false );
}

sal_Bool OTextInputStream::isEOF()
{
    checkNull();
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
        setEncoding( u"utf8"_ustr );
    }
    if( !mbEncodingInitialized )
        return aRetStr;

    // Only for bFindLineEnd
    constexpr sal_Unicode cLineEndChar1 = '\r';
    constexpr sal_Unicode cLineEndChar2 = '\n';

    sal_Int32 nBufferReadPos = 0;
    sal_Int32 nCopyLen = -1;
    sal_Unicode cFirstLineEndChar = 0;
    while (true)
    {
        // Still characters available?
        if( nBufferReadPos == mnCharsInBuffer )
        {
            // Already reached EOF? Then we can't read any more
            // Or no, so read new characters
            if( !implReadNext() )
                break;
        }

        // Now there should be characters available
        // (otherwise the loop should have been broken before)
        sal_Unicode c = mvBuffer[ nBufferReadPos++ ];

        if( bFindLineEnd )
        {
            if (cFirstLineEndChar != 0)
            {
                assert(nCopyLen >= 0);
                // This is a check if the next character after a line end char is its second half
                // Same line end char -> new line break; non-line-end char -> new line start
                if ((c == cFirstLineEndChar) || (c != cLineEndChar1 && c != cLineEndChar2))
                {
                    // Not a two-char line end
                    nBufferReadPos--;
                }
                break;
            }
            else if( c == cLineEndChar1 || c == cLineEndChar2 )
            {
                nCopyLen = nBufferReadPos - 1; // we know what to copy
                cFirstLineEndChar = c; // take one more loop, to check if it's a two-char line end
            }
        }
        else if( comphelper::findValue(Delimiters, c) != -1 )
        {
            nCopyLen = nBufferReadPos;
            if( bRemoveDelimiter )
                nCopyLen--;
            break;
        }
    }

    // Nothing found? Return all
    if (nCopyLen < 0)
        nCopyLen = nBufferReadPos;

    // Create string
    if( nCopyLen )
        aRetStr = OUString( mvBuffer.data(), nCopyLen );

    // Copy rest of buffer
    std::copy(mvBuffer.data() + nBufferReadPos, mvBuffer.data() + mnCharsInBuffer, mvBuffer.data());
    mnCharsInBuffer -= nBufferReadPos;

    return aRetStr;
}


sal_Int32 OTextInputStream::implReadNext()
{
    // Already reached EOF? Then we can't read any more
    if (mbReachedEOF)
        return 0;

    try
    {
        if (mxStream->readSomeBytes(mSeqSource, READ_BYTE_COUNT) == 0)
        {
            mbReachedEOF = true;
            return 0;
        }

        // Try to convert
        sal_uInt32 uiInfo = mvBuffer.size() - mnCharsInBuffer < o3tl::make_unsigned(mSeqSource.getLength())
                                ? RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL
                                : 0;
        const sal_Int32 nOldCharsInBuffer = mnCharsInBuffer;
        sal_Size nSourceCount = 0;
        while( true )
        {
            if (uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL)
            {
                mvBuffer.resize(mvBuffer.size() * 2);
            }

            // All invalid characters are transformed to the unicode undefined char
            sal_Size nSrcCvtBytes = 0;
            mnCharsInBuffer += rtl_convertTextToUnicode(
                                mConvText2Unicode,
                                mContextText2Unicode,
                                reinterpret_cast<const char*>(mSeqSource.getConstArray() + nSourceCount),
                                mSeqSource.getLength() - nSourceCount,
                                mvBuffer.data() + mnCharsInBuffer,
                                mvBuffer.size() - mnCharsInBuffer,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                &uiInfo,
                                &nSrcCvtBytes );
            nSourceCount += nSrcCvtBytes;

            if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL )
            {
                // read next byte
                Sequence<sal_Int8> aOneByteSeq(1);
                if (mxStream->readSomeBytes(aOneByteSeq, 1) == 0)
                {
                    mbReachedEOF = true;
                    return mnCharsInBuffer - nOldCharsInBuffer;
                }

                mSeqSource = comphelper::concatSequences(mSeqSource, aOneByteSeq);
            }
            else if (!(uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL))
                return mnCharsInBuffer - nOldCharsInBuffer; // finished
        }
    }
    catch( NotConnectedException& )
    {
        throw IOException(u"Not connected"_ustr);
        //throw IOException( L"OTextInputStream::implReadString failed" );
    }
    catch( BufferSizeExceededException& )
    {
        throw IOException(u"Buffer size exceeded"_ustr);
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
}


// XInputStream

sal_Int32 OTextInputStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    checkNull();
    return mxStream->readBytes( aData, nBytesToRead );
}

sal_Int32 OTextInputStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    checkNull();
    return mxStream->readSomeBytes( aData, nMaxBytesToRead );
}

void OTextInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    checkNull();
    mxStream->skipBytes( nBytesToSkip );
}

sal_Int32 OTextInputStream::available(  )
{
    checkNull();
    return mxStream->available();
}

void OTextInputStream::closeInput(  )
{
    checkNull();
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

OUString OTextInputStream::getImplementationName()
{
    return u"com.sun.star.comp.io.TextInputStream"_ustr;
}

sal_Bool OTextInputStream::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > OTextInputStream::getSupportedServiceNames()
{
    return { u"com.sun.star.io.TextInputStream"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
io_OTextInputStream_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OTextInputStream());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
