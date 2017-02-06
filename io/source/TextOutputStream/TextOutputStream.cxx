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


#include <osl/diagnose.h>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XTextOutputStream2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include "services.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.TextOutputStream"
#define SERVICE_NAME "com.sun.star.io.TextOutputStream"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;

namespace io_TextOutputStream
{

// Implementation XTextOutputStream

class OTextOutputStream : public WeakImplHelper< XTextOutputStream2, XServiceInfo >
{
    Reference< XOutputStream > mxStream;

    // Encoding
    OUString mEncoding;
    bool mbEncodingInitialized;
    rtl_UnicodeToTextConverter  mConvUnicode2Text;
    rtl_UnicodeToTextContext    mContextUnicode2Text;

    Sequence<sal_Int8> implConvert( const OUString& rSource );
    /// @throws IOException
    void checkOutputStream();

public:
    OTextOutputStream();
    virtual ~OTextOutputStream() override;

    // Methods XTextOutputStream
    virtual void SAL_CALL writeString( const OUString& aString ) override;
    virtual void SAL_CALL setEncoding( const OUString& Encoding ) override;

    // Methods XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence< sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL closeOutput(  ) override;

    // Methods XActiveDataSource
    virtual void SAL_CALL setOutputStream( const Reference< XOutputStream >& aStream ) override;
    virtual Reference< XOutputStream > SAL_CALL getOutputStream(  ) override;

    // Methods XServiceInfo
        virtual OUString              SAL_CALL getImplementationName() override;
        virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames() override;
        virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) override;
};

OTextOutputStream::OTextOutputStream()
    : mbEncodingInitialized(false)
    , mConvUnicode2Text(nullptr)
    , mContextUnicode2Text(nullptr)
{
}

OTextOutputStream::~OTextOutputStream()
{
    if( mbEncodingInitialized )
    {
        rtl_destroyUnicodeToTextContext( mConvUnicode2Text, mContextUnicode2Text );
        rtl_destroyUnicodeToTextConverter( mConvUnicode2Text );
    }
}

Sequence<sal_Int8> OTextOutputStream::implConvert( const OUString& rSource )
{
    const sal_Unicode *puSource = rSource.getStr();
    sal_Int32 nSourceSize = rSource.getLength();

    sal_Size nTargetCount = 0;
    sal_Size nSourceCount = 0;

    sal_uInt32 uiInfo;
    sal_Size nSrcCvtChars;

    // take nSourceSize * 3 as preference
    // this is an upper boundary for converting to utf8,
    // which most often used as the target.
    sal_Int32 nSeqSize =  nSourceSize * 3;

    Sequence<sal_Int8> seqText( nSeqSize );
    sal_Char *pTarget = reinterpret_cast<char *>(seqText.getArray());
    while( true )
    {
        nTargetCount += rtl_convertUnicodeToText(
                                    mConvUnicode2Text,
                                    mContextUnicode2Text,
                                    &( puSource[nSourceCount] ),
                                    nSourceSize - nSourceCount ,
                                    &( pTarget[nTargetCount] ),
                                    nSeqSize - nTargetCount,
                                    RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                                    RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT ,
                                    &uiInfo,
                                    &nSrcCvtChars);
        nSourceCount += nSrcCvtChars;

        if( uiInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL )
        {
            nSeqSize *= 2;
            seqText.realloc( nSeqSize );  // double array size
            pTarget = reinterpret_cast<char*>(seqText.getArray());
            continue;
        }
        break;
    }

    // reduce the size of the buffer (fast, no copy necessary)
    seqText.realloc( nTargetCount );
    return seqText;
}


// XTextOutputStream

void OTextOutputStream::writeString( const OUString& aString )
{
    checkOutputStream();
    if( !mbEncodingInitialized )
    {
        OUString aUtf8Str("utf8");
        setEncoding( aUtf8Str );
    }
    if( !mbEncodingInitialized )
        return;

    Sequence<sal_Int8> aByteSeq = implConvert( aString );
    mxStream->writeBytes( aByteSeq );
}

void OTextOutputStream::setEncoding( const OUString& Encoding )
{
    OString aOEncodingStr = OUStringToOString( Encoding, RTL_TEXTENCODING_ASCII_US );
    rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( aOEncodingStr.getStr() );
    if( RTL_TEXTENCODING_DONTKNOW == encoding )
        return;

    mbEncodingInitialized = true;
    mConvUnicode2Text   = rtl_createUnicodeToTextConverter( encoding );
    mContextUnicode2Text = rtl_createUnicodeToTextContext( mConvUnicode2Text );
    mEncoding = Encoding;
}


// XOutputStream
void OTextOutputStream::writeBytes( const Sequence< sal_Int8 >& aData )
{
    checkOutputStream();
    mxStream->writeBytes( aData );
}

void OTextOutputStream::flush(  )
{
    checkOutputStream();
    mxStream->flush();
}

void OTextOutputStream::closeOutput(  )
{
    checkOutputStream();
    mxStream->closeOutput();
}


void OTextOutputStream::checkOutputStream()
{
    if (! mxStream.is() )
        throw IOException("output stream is not initialized, you have to use setOutputStream first");
}


// XActiveDataSource

void OTextOutputStream::setOutputStream( const Reference< XOutputStream >& aStream )
{
    mxStream = aStream;
}

Reference< XOutputStream > OTextOutputStream::getOutputStream()
{
    return mxStream;
}


Reference< XInterface > SAL_CALL TextOutputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext > &)
{
    return Reference < XInterface >( static_cast<OWeakObject *>(new OTextOutputStream()) );
}

OUString TextOutputStream_getImplementationName()
{
    return OUString( IMPLEMENTATION_NAME );
}


Sequence< OUString > TextOutputStream_getSupportedServiceNames()
{
    Sequence< OUString > seqNames { SERVICE_NAME };
    return seqNames;
}

OUString OTextOutputStream::getImplementationName()
{
    return TextOutputStream_getImplementationName();
}

sal_Bool OTextOutputStream::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > OTextOutputStream::getSupportedServiceNames()
{
    return TextOutputStream_getSupportedServiceNames();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
