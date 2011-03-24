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

#include "oox/helper/textinputstream.hxx"

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <rtl/tencinfo.h>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

namespace {

typedef ::cppu::WeakImplHelper1< XInputStream > UnoBinaryInputStream_BASE;

/** Implementation of a UNO input stream wrapping a binary input stream.
 */
class UnoBinaryInputStream : public UnoBinaryInputStream_BASE
{
public:
    explicit            UnoBinaryInputStream( BinaryInputStream& rInStrm );

    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
                        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
                        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
                        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL available()
                        throw (NotConnectedException, IOException, RuntimeException);
    virtual void SAL_CALL closeInput()
                        throw (NotConnectedException, IOException, RuntimeException);

private:
    void                ensureConnected() const throw (NotConnectedException);

private:
    BinaryInputStream*  mpInStrm;
};

// ----------------------------------------------------------------------------

UnoBinaryInputStream::UnoBinaryInputStream( BinaryInputStream& rInStrm ) :
    mpInStrm( &rInStrm )
{
}

sal_Int32 SAL_CALL UnoBinaryInputStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    ensureConnected();
    return mpInStrm->readData( rData, nBytesToRead, 1 );
}

sal_Int32 SAL_CALL UnoBinaryInputStream::readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    ensureConnected();
    return mpInStrm->readData( rData, nMaxBytesToRead, 1 );
}

void SAL_CALL UnoBinaryInputStream::skipBytes( sal_Int32 nBytesToSkip )
        throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    ensureConnected();
    mpInStrm->skip( nBytesToSkip, 1 );
}

sal_Int32 SAL_CALL UnoBinaryInputStream::available() throw (NotConnectedException, IOException, RuntimeException)
{
    ensureConnected();
    throw RuntimeException( CREATE_OUSTRING( "Functionality not supported" ), Reference< XInputStream >() );
}

void SAL_CALL UnoBinaryInputStream::closeInput() throw (NotConnectedException, IOException, RuntimeException)
{
    ensureConnected();
    mpInStrm->close();
    mpInStrm = 0;
}

void UnoBinaryInputStream::ensureConnected() const throw (NotConnectedException)
{
    if( !mpInStrm )
        throw NotConnectedException( CREATE_OUSTRING( "Stream closed" ), Reference< XInterface >() );
}

} // namespace

// ============================================================================

TextInputStream::TextInputStream( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, rtl_TextEncoding eTextEnc )
{
    init( rxContext, rxInStrm, eTextEnc );
}

TextInputStream::TextInputStream( const Reference< XComponentContext >& rxContext, BinaryInputStream& rInStrm, rtl_TextEncoding eTextEnc )
{
    init( rxContext, new UnoBinaryInputStream( rInStrm ), eTextEnc );
}

TextInputStream::~TextInputStream()
{
}

bool TextInputStream::isEof() const
{
    if( mxTextStrm.is() ) try
    {
        return mxTextStrm->isEOF();
    }
    catch( Exception& )
    {
    }
    return true;
}

OUString TextInputStream::readLine()
{
    if( mxTextStrm.is() ) try
    {
        /*  The function createFinalString() adds a character that may have
            been buffered in the previous call of readToChar() (see below). */
        return createFinalString( mxTextStrm->readLine() );
    }
    catch( Exception& )
    {
        mxTextStrm.clear();
    }
    return OUString();
}

OUString TextInputStream::readToChar( sal_Unicode cChar, bool bIncludeChar )
{
    if( mxTextStrm.is() ) try
    {
        Sequence< sal_Unicode > aDelimiters( 1 );
        aDelimiters[ 0 ] = cChar;
        /*  Always get the delimiter character from the UNO text input stream.
            In difference to this implementation, it will not return it in the
            next call but silently skip it. If caller specifies to exclude the
            character in this call, it will be returned in the next call of one
            of the own member functions. The function createFinalString() adds
            a character that has been buffered in the previous call. */
        OUString aString = createFinalString( mxTextStrm->readString( aDelimiters, sal_False ) );
        // remove last character from string and remember it for next call
        if( !bIncludeChar && (aString.getLength() > 0) && (aString[ aString.getLength() - 1 ] == cChar) )
        {
            mcPendingChar = cChar;
            aString = aString.copy( 0, aString.getLength() - 1 );
        }
        return aString;
    }
    catch( Exception& )
    {
        mxTextStrm.clear();
    }
    return OUString();
}

/*static*/ Reference< XTextInputStream > TextInputStream::createXTextInputStream(
        const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, rtl_TextEncoding eTextEnc )
{
    Reference< XTextInputStream > xTextStrm;
    const char* pcCharset = rtl_getMimeCharsetFromTextEncoding( eTextEnc );
    OSL_ENSURE( pcCharset, "TextInputStream::createXTextInputStream - unsupported text encoding" );
    if( rxContext.is() && rxInStrm.is() && pcCharset ) try
    {
        Reference< XMultiServiceFactory > xFactory( rxContext->getServiceManager(), UNO_QUERY_THROW );
        Reference< XActiveDataSink > xDataSink( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TextInputStream" ) ), UNO_QUERY_THROW );
        xDataSink->setInputStream( rxInStrm );
        xTextStrm.set( xDataSink, UNO_QUERY_THROW );
        xTextStrm->setEncoding( OUString::createFromAscii( pcCharset ) );
    }
    catch( Exception& )
    {
    }
    return xTextStrm;
}

// private --------------------------------------------------------------------

OUString TextInputStream::createFinalString( const OUString& rString )
{
    if( mcPendingChar == 0 )
        return rString;

    OUString aString = OUString( mcPendingChar ) + rString;
    mcPendingChar = 0;
    return aString;
}

void TextInputStream::init( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, rtl_TextEncoding eTextEnc )
{
    mcPendingChar = 0;
    mxTextStrm = createXTextInputStream( rxContext, rxInStrm, eTextEnc );
}

// ============================================================================

} // namespace oox
