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
#include "precompiled_filter.hxx"
#include "filtertracer.hxx"
#include <uno/mapping.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
// ----------------
// - FILTERTRACER -
// ----------------

rtl::OUString FilterTracer_getImplementationName()
    throw( NMSP_UNO::RuntimeException )
{
    return B2UCONST( "com.sun.star.util.FilterTracer" );
}
sal_Bool SAL_CALL FilterTracer_supportsService( const rtl::OUString& ServiceName ) 
    throw( NMSP_UNO::RuntimeException )
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.util.logging.Logger" ) );
}
SEQ( rtl::OUString ) SAL_CALL FilterTracer_getSupportedServiceNames() 
    throw( NMSP_UNO::RuntimeException )
{
    SEQ( rtl::OUString ) aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = B2UCONST( "com.sun.star.util.logging.Logger" );
    return aRet;
}

// -----------------------------------------------------------------------------

FilterTracer::FilterTracer( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    xFact		( rxMgr ),
    mpStream	( NULL ),
    mnLogLevel	( NMSP_LOGGING::LogLevel::ALL )
{
    REF( NMSP_UNO::XInterface ) xObj( rxMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.util.TextSearch" ) ) );
    mxTextSearch = REF( NMSP_UTIL::XTextSearch )( xObj, ::com::sun::star::uno::UNO_QUERY );
}
FilterTracer::~FilterTracer()
{
    if ( mpStream )
    {
        mxOutputStream = NULL;
        delete mpStream;
    }
}

// -----------------------------------------------------------------------------

// XInterface
void SAL_CALL FilterTracer::acquire() throw()
{
    OWeakObject::acquire();
}
void SAL_CALL FilterTracer::release() throw()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

// checks if the tokens of rFilter can be found in rString
sal_Bool FilterTracer::ImplFilter( const rtl::OUString& rFilter, const rtl::OUString& rString )
{
    sal_Bool bFilter = sal_False;
    if ( mxTextSearch.is() )
    {
        maSearchOptions.searchString = rFilter;
        mxTextSearch->setOptions( maSearchOptions );
        NMSP_UTIL::SearchResult aSearchResult = mxTextSearch->searchForward( rString, 0, rString.getLength() );
        bFilter = aSearchResult.subRegExpressions != 0;
    }
    return bFilter;
}

// -----------------------------------------------------------------------------

// XInitialization
void SAL_CALL FilterTracer::initialize( const SEQ( NMSP_UNO::Any )& aArguments )
    throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException )
{
    sal_Int32 i;
    SEQ( NMSP_BEANS::PropertyValue ) aParameter;
    for ( i = 0; i < aArguments.getLength(); i++ )
    {
        if ( aArguments[ i ] >>= aParameter )
            break;
    }
    for ( i = 0; i < aParameter.getLength(); i++ )
    {
        const NMSP_BEANS::PropertyValue& rProp = aParameter[ i ];
        if ( rProp.Name.equalsAscii( "LogLevel" ) )
            rProp.Value >>= mnLogLevel;
        else if ( rProp.Name.equalsAscii( "ClassFilter" ) )
            rProp.Value >>= msClassFilter;
        else if ( rProp.Name.equalsAscii( "MethodFilter" ) )
            rProp.Value >>= msMethodFilter;
        else if ( rProp.Name.equalsAscii( "MessageFilter" ) )
            rProp.Value >>= msMessageFilter;
        else if ( rProp.Name.equalsAscii( "OutputStream" ) )
            rProp.Value >>= mxOutputStream;
        else if ( rProp.Name.equalsAscii( "URL" ) )
            rProp.Value >>= msURL;
        else if ( rProp.Name.equalsAscii( "DocumentHandler" ) )
            rProp.Value >>= mxDocumentHandler;
    }

    // check if we have to create the XOutputStream
    if ( !mxOutputStream.is() && msURL.getLength() )
    {
        mpStream = ::utl::UcbStreamHelper::CreateStream( msURL, STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYNONE );
        if ( mpStream )
        {
            ::utl::OOutputStreamWrapper* pHelper = new ::utl::OOutputStreamWrapper( *mpStream );
            mxOutputStream = pHelper;
        }
    }
}

// -----------------------------------------------------------------------------

// XServiceInfo
rtl::OUString SAL_CALL FilterTracer::getImplementationName() 
    throw( NMSP_UNO::RuntimeException )
{
    return FilterTracer_getImplementationName();
}
sal_Bool SAL_CALL FilterTracer::supportsService( const rtl::OUString& rServiceName ) 
    throw( NMSP_UNO::RuntimeException )
{
    return FilterTracer_supportsService( rServiceName );
}
SEQ( rtl::OUString ) SAL_CALL FilterTracer::getSupportedServiceNames() 
    throw ( NMSP_UNO::RuntimeException )
{
    return FilterTracer_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

// XLogger
REF( NMSP_LOGGING::XLogger ) SAL_CALL  FilterTracer::getLogger( const rtl::OUString& /* rName */ )
     throw (::com::sun::star::uno::RuntimeException)
{
    REF( NMSP_LOGGING::XLogger ) xLog( this );
    return xLog;
}
sal_Int32 SAL_CALL FilterTracer::getLevel() throw (::com::sun::star::uno::RuntimeException)
{
    return mnLogLevel;
}
rtl::OUString SAL_CALL FilterTracer::getName() throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString aName;
    return aName;
}
sal_Bool SAL_CALL FilterTracer::isLoggable( sal_Int32 nLevel )
     throw (::com::sun::star::uno::RuntimeException)
{
    return mnLogLevel <= nLevel;
}
void SAL_CALL FilterTracer::logp( sal_Int32 /* nLevel */, const rtl::OUString& rSourceClass,
        const rtl::OUString& rSourceMethod, const rtl::OUString& rMessage )
     throw (::com::sun::star::uno::RuntimeException)
{
    if ( mxOutputStream.is() || mxDocumentHandler.is() )
    {
        if ( ! ( ImplFilter( msClassFilter, rSourceClass ) || ImplFilter( msMethodFilter, rSourceMethod )
            || ImplFilter( msMessageFilter, rMessage ) ) )
        {
            rtl::OString sClass( rtl::OUStringToOString( rSourceClass, RTL_TEXTENCODING_UTF8 ) );
            rtl::OString sMethod( rtl::OUStringToOString( rSourceMethod, RTL_TEXTENCODING_UTF8 ) );
            rtl::OString sMessage( rtl::OUStringToOString( rMessage, RTL_TEXTENCODING_UTF8 ) );
            try
            {
                SEQ( sal_Int8 ) aData( sClass.getLength() + sMethod.getLength() + sMessage.getLength() );
                sal_Int8* pPtr = aData.getArray();
                memcpy( pPtr, sClass.getStr(), sClass.getLength() );
                pPtr += sClass.getLength();
                memcpy( pPtr, sMethod.getStr(), sMethod.getLength() );
                pPtr += sMethod.getLength();
                memcpy( pPtr, sMessage.getStr(), sMessage.getLength() );
                pPtr += sMessage.getLength();
                if ( mxOutputStream.is() )
                    mxOutputStream->writeBytes( aData );
                if ( mxDocumentHandler.is() )
                    mxDocumentHandler->characters( ::rtl::OUString( (sal_Char*)aData.getArray(), aData.getLength(), RTL_TEXTENCODING_UTF8 ) );
            }
            catch ( ... )
            {

            }
        }
    }
}

// -----------------------------------------------------------------------------

// XTextSearch
void SAL_CALL  FilterTracer::setOptions( const NMSP_UTIL::SearchOptions& rSearchOptions ) 
    throw (::com::sun::star::uno::RuntimeException)
{
    maSearchOptions = rSearchOptions;
}

// -----------------------------------------------------------------------------

NMSP_UTIL::SearchResult SAL_CALL FilterTracer::searchForward( const rtl::OUString& rSearchStr,
        sal_Int32 nStartPos, sal_Int32 nEndPos ) throw (::com::sun::star::uno::RuntimeException)
{
    NMSP_UTIL::SearchResult nSearchResult;
    if ( mxTextSearch.is() )
        mxTextSearch->searchForward( rSearchStr, nStartPos, nEndPos );
    return nSearchResult;
}

// -----------------------------------------------------------------------------

NMSP_UTIL::SearchResult SAL_CALL FilterTracer::searchBackward( const rtl::OUString& rSearchStr,
        sal_Int32 nStartPos, sal_Int32 nEndPos ) throw (::com::sun::star::uno::RuntimeException)
{
    NMSP_UTIL::SearchResult nSearchResult;
    if ( mxTextSearch.is() )
        mxTextSearch->searchBackward( rSearchStr, nStartPos, nEndPos );
    return nSearchResult;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
