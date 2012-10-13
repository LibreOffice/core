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
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>
#include <rtl/unload.h>

#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


#define IMPLEMENTATION_NAME "com.sun.star.comp.io.TextOutputStream"
#define SERVICE_NAME "com.sun.star.io.TextOutputStream"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;

namespace io_TextOutputStream
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;
//===========================================================================
// Implementation XTextOutputStream

typedef WeakImplHelper3< XTextOutputStream, XActiveDataSource, XServiceInfo > TextOutputStreamHelper;

class OTextOutputStream : public TextOutputStreamHelper
{
    Reference< XOutputStream > mxStream;

    // Encoding
    OUString mEncoding;
    sal_Bool mbEncodingInitialized;
    rtl_UnicodeToTextConverter  mConvUnicode2Text;
    rtl_UnicodeToTextContext    mContextUnicode2Text;

    Sequence<sal_Int8> implConvert( const OUString& rSource );

public:
    OTextOutputStream();
    ~OTextOutputStream();

    // Methods XTextOutputStream
    virtual void SAL_CALL writeString( const OUString& aString )
        throw(IOException, RuntimeException);
    virtual void SAL_CALL setEncoding( const OUString& Encoding )
        throw(RuntimeException);

    // Methods XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence< sal_Int8 >& aData )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL flush(  )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL closeOutput(  )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);

    // Methods XActiveDataSource
    virtual void SAL_CALL setOutputStream( const Reference< XOutputStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XOutputStream > SAL_CALL getOutputStream(  )
        throw(RuntimeException);

    // Methods XServiceInfo
        virtual OUString              SAL_CALL getImplementationName() throw();
        virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames(void) throw();
        virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) throw();
};

OTextOutputStream::OTextOutputStream()
{
    mbEncodingInitialized = false;
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
    sal_Char *pTarget = (sal_Char *) seqText.getArray();
    while( sal_True )
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
            pTarget = (sal_Char*) seqText.getArray();
            continue;
        }
        break;
    }

    // reduce the size of the buffer (fast, no copy necessary)
    seqText.realloc( nTargetCount );
    return seqText;
}


//===========================================================================
// XTextOutputStream

void OTextOutputStream::writeString( const OUString& aString )
    throw(IOException, RuntimeException)
{
    if( !mbEncodingInitialized )
    {
        OUString aUtf8Str( RTL_CONSTASCII_USTRINGPARAM("utf8") );
        setEncoding( aUtf8Str );
    }
    if( !mbEncodingInitialized )
        return;

    Sequence<sal_Int8> aByteSeq = implConvert( aString );
    mxStream->writeBytes( aByteSeq );
}

void OTextOutputStream::setEncoding( const OUString& Encoding )
    throw(RuntimeException)
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

//===========================================================================
// XOutputStream
void OTextOutputStream::writeBytes( const Sequence< sal_Int8 >& aData )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    mxStream->writeBytes( aData );
}

void OTextOutputStream::flush(  )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    mxStream->flush();
}

void OTextOutputStream::closeOutput(  )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    mxStream->closeOutput();
}


//===========================================================================
// XActiveDataSource

void OTextOutputStream::setOutputStream( const Reference< XOutputStream >& aStream )
    throw(RuntimeException)
{
    mxStream = aStream;
}

Reference< XOutputStream > OTextOutputStream::getOutputStream()
    throw(RuntimeException)
{
    return mxStream;
}


Reference< XInterface > SAL_CALL TextOutputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext > &)
{
    return Reference < XInterface >( ( OWeakObject * ) new OTextOutputStream() );
}

OUString TextOutputStream_getImplementationName() SAL_THROW(  () )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
}


Sequence< OUString > TextOutputStream_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    return seqNames;
}

OUString OTextOutputStream::getImplementationName() throw()
{
    return TextOutputStream_getImplementationName();
}

sal_Bool OTextOutputStream::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

Sequence< OUString > OTextOutputStream::getSupportedServiceNames(void) throw()
{
    return TextOutputStream_getSupportedServiceNames();
}


}

using namespace io_TextOutputStream;

static struct ImplementationEntry g_entries[] =
{
    {
        TextOutputStream_CreateInstance, TextOutputStream_getImplementationName ,
        TextOutputStream_getSupportedServiceNames, createSingleComponentFactory ,
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
SAL_DLLPUBLIC_EXPORT void * SAL_CALL textoutstream_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
