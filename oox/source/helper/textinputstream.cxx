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

#include "oox/helper/textinputstream.hxx"

#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/TextInputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace {

typedef ::cppu::WeakImplHelper< XInputStream > UnoBinaryInputStream_BASE;

/** Implementation of a UNO input stream wrapping a binary input stream.
 */
class UnoBinaryInputStream : public UnoBinaryInputStream_BASE
{
public:
    explicit            UnoBinaryInputStream( BinaryInputStream& rInStrm );

    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

private:
    /// @throws NotConnectedException
    void                ensureConnected() const;

private:
    BinaryInputStream*  mpInStrm;
};

UnoBinaryInputStream::UnoBinaryInputStream( BinaryInputStream& rInStrm ) :
    mpInStrm( &rInStrm )
{
}

sal_Int32 SAL_CALL UnoBinaryInputStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
{
    ensureConnected();
    return mpInStrm->readData( rData, nBytesToRead );
}

sal_Int32 SAL_CALL UnoBinaryInputStream::readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
{
    ensureConnected();
    return mpInStrm->readData( rData, nMaxBytesToRead );
}

void SAL_CALL UnoBinaryInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    ensureConnected();
    mpInStrm->skip( nBytesToSkip );
}

sal_Int32 SAL_CALL UnoBinaryInputStream::available()
{
    ensureConnected();
    throw RuntimeException( "Functionality not supported", Reference< XInputStream >() );
}

void SAL_CALL UnoBinaryInputStream::closeInput()
{
    ensureConnected();
    mpInStrm->close();
    mpInStrm = nullptr;
}

void UnoBinaryInputStream::ensureConnected() const
{
    if( !mpInStrm )
        throw NotConnectedException( "Stream closed" );
}

} // namespace

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
    catch (const Exception&)
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
    catch (const Exception&)
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
        OUString aString = createFinalString( mxTextStrm->readString( aDelimiters, false ) );
        // remove last character from string and remember it for next call
        if( !bIncludeChar && !aString.isEmpty() && (aString[ aString.getLength() - 1 ] == cChar) )
        {
            mcPendingChar = cChar;
            aString = aString.copy( 0, aString.getLength() - 1 );
        }
        return aString;
    }
    catch (const Exception&)
    {
        mxTextStrm.clear();
    }
    return OUString();
}

Reference< XTextInputStream2 > TextInputStream::createXTextInputStream(
        const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, rtl_TextEncoding eTextEnc )
{
    Reference< XTextInputStream2 > xTextStrm;
    const char* pcCharset = rtl_getBestMimeCharsetFromTextEncoding( eTextEnc );
    OSL_ENSURE( pcCharset, "TextInputStream::createXTextInputStream - unsupported text encoding" );
    if( rxContext.is() && rxInStrm.is() && pcCharset ) try
    {
        xTextStrm = css::io::TextInputStream::create( rxContext );
        xTextStrm->setInputStream( rxInStrm );
        xTextStrm->setEncoding( OUString::createFromAscii( pcCharset ) );
    }
    catch (const Exception&)
    {
    }
    return xTextStrm;
}

// private --------------------------------------------------------------------

OUString TextInputStream::createFinalString( const OUString& rString )
{
    if( mcPendingChar == 0 )
        return rString;

    OUString aString = OUStringLiteral1( mcPendingChar ) + rString;
    mcPendingChar = 0;
    return aString;
}

void TextInputStream::init( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, rtl_TextEncoding eTextEnc )
{
    mcPendingChar = 0;
    mxTextStrm = createXTextInputStream( rxContext, rxInStrm, eTextEnc );
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
