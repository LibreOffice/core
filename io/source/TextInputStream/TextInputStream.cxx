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

#include <rtl/unload.h>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


#define IMPLEMENTATION_NAME "com.sun.star.comp.io.TextInputStream"
#define SERVICE_NAME "com.sun.star.io.TextInputStream"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;

namespace io_TextInputStream
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

//===========================================================================
// Implementation XTextInputStream

typedef WeakImplHelper3< XTextInputStream, XActiveDataSink, XServiceInfo > TextInputStreamHelper;

#define INITIAL_UNICODE_BUFFER_CAPACITY     0x100
#define READ_BYTE_COUNT                     0x100

class OTextInputStream : public TextInputStreamHelper
{
    Reference< XInputStream > mxStream;

    // Encoding
    OUString mEncoding;
    sal_Bool mbEncodingInitialized;
    rtl_TextToUnicodeConverter  mConvText2Unicode;
    rtl_TextToUnicodeContext    mContextText2Unicode;
    Sequence<sal_Int8>          mSeqSource;

    // Internal buffer for characters that are already converted successfully
    sal_Unicode* mpBuffer;
    sal_Int32 mnBufferSize;
    sal_Int32 mnCharsInBuffer;
    sal_Bool mbReachedEOF;

    void implResizeBuffer( void );
    OUString implReadString( const Sequence< sal_Unicode >& Delimiters,
        sal_Bool bRemoveDelimiter, sal_Bool bFindLineEnd )
            throw(IOException, RuntimeException);
    sal_Int32 implReadNext() throw(IOException, RuntimeException);

public:
    OTextInputStream();
    virtual ~OTextInputStream();

    // Methods XTextInputStream
    virtual OUString SAL_CALL readLine(  )
        throw(IOException, RuntimeException);
    virtual OUString SAL_CALL readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter )
        throw(IOException, RuntimeException);
    virtual sal_Bool SAL_CALL isEOF(  )
        throw(IOException, RuntimeException);
    virtual void SAL_CALL setEncoding( const OUString& Encoding ) throw(RuntimeException);

    // Methods XInputStream
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(NotConnectedException, IOException, RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException);

    // Methods XActiveDataSink
    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XInputStream > SAL_CALL getInputStream()
        throw(RuntimeException);

    // Methods XServiceInfo
        virtual OUString              SAL_CALL getImplementationName() throw();
        virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames(void) throw();
        virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) throw();
};

OTextInputStream::OTextInputStream()
    : mSeqSource( READ_BYTE_COUNT ), mpBuffer( NULL ), mnBufferSize( 0 )
    , mnCharsInBuffer( 0 ), mbReachedEOF( sal_False )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    mbEncodingInitialized = false;
}

OTextInputStream::~OTextInputStream()
{
    if( mbEncodingInitialized )
    {
        rtl_destroyUnicodeToTextContext( mConvText2Unicode, mContextText2Unicode );
        rtl_destroyUnicodeToTextConverter( mConvText2Unicode );
    }
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void OTextInputStream::implResizeBuffer( void )
{
    sal_Int32 mnNewBufferSize = mnBufferSize * 2;
    sal_Unicode* pNewBuffer = new sal_Unicode[ mnNewBufferSize ];
    memcpy( pNewBuffer, mpBuffer, mnCharsInBuffer * sizeof( sal_Unicode ) );
    mpBuffer = pNewBuffer;
    mnBufferSize = mnNewBufferSize;
}


//===========================================================================
// XTextInputStream

OUString OTextInputStream::readLine(  )
    throw(IOException, RuntimeException)
{
    static Sequence< sal_Unicode > aDummySeq;
    return implReadString( aDummySeq, sal_True, sal_True );
}

OUString OTextInputStream::readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter )
        throw(IOException, RuntimeException)
{
    return implReadString( Delimiters, bRemoveDelimiter, sal_False );
}

sal_Bool OTextInputStream::isEOF()
    throw(IOException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    if( mnCharsInBuffer == 0 && mbReachedEOF )
        bRet = sal_True;
    return bRet;
}


OUString OTextInputStream::implReadString( const Sequence< sal_Unicode >& Delimiters,
                                           sal_Bool bRemoveDelimiter, sal_Bool bFindLineEnd )
        throw(IOException, RuntimeException)
{
    OUString aRetStr;
    if( !mbEncodingInitialized )
    {
        OUString aUtf8Str( RTL_CONSTASCII_USTRINGPARAM("utf8") );
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
    sal_Bool bFound = sal_False;
    sal_Bool bFoundFirstLineEndChar = sal_False;
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
                bFound = sal_True;
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
                bFoundFirstLineEndChar = sal_True;
                cFirstLineEndChar = c;
            }
        }
        else
        {
            for( sal_Int32 i = 0 ; i < nDelimCount ; i++ )
            {
                if( c == pDelims[ i ] )
                {
                    bFound = sal_True;
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
        throw(IOException, RuntimeException)
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
        if( nRead < nBytesToRead )
            mbReachedEOF = sal_True;

        // Try to convert
        sal_uInt32 uiInfo;
        sal_Size nSrcCvtBytes = 0;
        sal_Size nTargetCount = 0;
        sal_Size nSourceCount = 0;
        while( sal_True )
        {
            const sal_Int8 *pbSource = mSeqSource.getConstArray();

            // All invalid characters are transformed to the unicode undefined char
            nTargetCount += rtl_convertTextToUnicode(
                                mConvText2Unicode,
                                mContextText2Unicode,
                                (const sal_Char*) &( pbSource[nSourceCount] ),
                                nTotalRead - nSourceCount,
                                mpBuffer + mnCharsInBuffer + nTargetCount,
                                nFreeBufferSize - nTargetCount,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                &uiInfo,
                                &nSrcCvtBytes );
            nSourceCount += nSrcCvtBytes;

            sal_Bool bCont = sal_False;
            if( uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL )
            {
                implResizeBuffer();
                bCont = sal_True;
            }

            if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL )
            {
                // read next byte
                static Sequence< sal_Int8 > aOneByteSeq( 1 );
                nRead = mxStream->readSomeBytes( aOneByteSeq, 1 );
                if( nRead == 0 )
                {
                    mbReachedEOF = sal_True;
                    break;
                }

                sal_Int32 nOldLen = mSeqSource.getLength();
                nTotalRead++;
                if( nTotalRead > nOldLen )
                {
                    mSeqSource.realloc( nTotalRead );
                }
                mSeqSource.getArray()[ nOldLen ] = aOneByteSeq.getConstArray()[ 0 ];
                pbSource = mSeqSource.getConstArray();
                bCont = sal_True;
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
    throw(RuntimeException)
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

//===========================================================================
// XInputStream

sal_Int32 OTextInputStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return mxStream->readBytes( aData, nBytesToRead );
}

sal_Int32 OTextInputStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return mxStream->readSomeBytes( aData, nMaxBytesToRead );
}

void OTextInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    mxStream->skipBytes( nBytesToSkip );
}

sal_Int32 OTextInputStream::available(  )
    throw(NotConnectedException, IOException, RuntimeException)
{
    return mxStream->available();
}

void OTextInputStream::closeInput(  )
    throw(NotConnectedException, IOException, RuntimeException)
{
    mxStream->closeInput();
}


//===========================================================================
// XActiveDataSink

void OTextInputStream::setInputStream( const Reference< XInputStream >& aStream )
    throw(RuntimeException)
{
    mxStream = aStream;
}

Reference< XInputStream > OTextInputStream::getInputStream()
    throw(RuntimeException)
{
    return mxStream;
}


Reference< XInterface > SAL_CALL TextInputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext > &)
{
    return Reference < XInterface >( ( OWeakObject * ) new OTextInputStream() );
}

OUString TextInputStream_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
}

Sequence< OUString > TextInputStream_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    return seqNames;
}

OUString OTextInputStream::getImplementationName() throw()
{
    return TextInputStream_getImplementationName();
}

sal_Bool OTextInputStream::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

Sequence< OUString > OTextInputStream::getSupportedServiceNames(void) throw()
{
    return TextInputStream_getSupportedServiceNames();
}

}

using namespace io_TextInputStream;

static struct ImplementationEntry g_entries[] =
{
    {
        TextInputStream_CreateInstance, TextInputStream_getImplementationName ,
        TextInputStream_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL textinstream_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
