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
#include <rtl/strbuf.hxx>

Reference< XInterface > SAL_CALL OPipeTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw( Exception );
Sequence<OUString> OPipeTest_getSupportedServiceNames(void) throw();
OUString     OPipeTest_getServiceName() throw();
OUString    OPipeTest_getImplementationName() throw();

Reference< XInterface > SAL_CALL OPumpTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw( Exception );
Sequence<OUString> OPumpTest_getSupportedServiceNames(void) throw();
OUString     OPumpTest_getServiceName() throw();
OUString    OPumpTest_getImplementationName() throw();

Reference< XInterface > SAL_CALL ODataStreamTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception);
Sequence<OUString> ODataStreamTest_getSupportedServiceNames( int i) throw();
OUString     ODataStreamTest_getServiceName( int i) throw();
OUString    ODataStreamTest_getImplementationName( int i) throw();

Reference< XInterface > SAL_CALL OMarkableOutputStreamTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception);
Sequence<OUString> OMarkableOutputStreamTest_getSupportedServiceNames(void) throw();
OUString     OMarkableOutputStreamTest_getServiceName() throw();
OUString    OMarkableOutputStreamTest_getImplementationName() throw();

Reference< XInterface > SAL_CALL OMarkableInputStreamTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception);
Sequence<OUString> OMarkableInputStreamTest_getSupportedServiceNames(void) throw();
OUString     OMarkableInputStreamTest_getServiceName() throw();
OUString    OMarkableInputStreamTest_getImplementationName() throw();

Reference< XInterface > SAL_CALL OObjectStreamTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception);
Sequence<OUString> OObjectStreamTest_getSupportedServiceNames( int i) throw();
OUString     OObjectStreamTest_getServiceName( int i) throw();
OUString    OObjectStreamTest_getImplementationName( int i) throw();

Reference< XInterface > SAL_CALL OMyPersistObject_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception);
Sequence<OUString>  OMyPersistObject_getSupportedServiceNames( ) throw();
OUString            OMyPersistObject_getServiceName( ) throw();
OUString            OMyPersistObject_getImplementationName( ) throw();

Sequence<sal_Int8> createSeq( char * p );
Sequence<sal_Int8> createIntSeq( sal_Int32 i );

#define BUILD_ERROR(expr, Message)\
        {\
            m_seqErrors.realloc( m_seqErrors.getLength() + 1 ); \
        m_seqExceptions.realloc(  m_seqExceptions.getLength() + 1 ); \
        OStringBuffer str(128); \
        str.append( __FILE__ );\
        str.append( " " ); \
        str.append( "(" ); \
        str.append( OString::valueOf( (sal_Int32)__LINE__) );\
        str.append(")\n" );\
        str.append( "[ " ); \
        str.append( #expr ); \
        str.append( " ] : " ); \
        str.append( Message ); \
        m_seqErrors.getArray()[ m_seqErrors.getLength()-1] =\
                       OStringToOUString( str.makeStringAndClear() , RTL_TEXTENCODING_ASCII_US ); \
        }\
        ((void)0)


#define WARNING_ASSERT(expr, Message) \
        if( ! (expr) ) { \
            m_seqWarnings.realloc( m_seqErrors.getLength() +1 ); \
            OStringBuffer str(128);\
            str.append( __FILE__);\
            str.append( " "); \
            str.append( "(" ); \
            str.append(OString::valueOf( (sal_Int32)__LINE__)) ;\
            str.append( ")\n");\
            str.append( "[ " ); \
            str.append( #expr ); \
            str.append( " ] : ") ; \
            str.append( Message); \
            m_seqWarnings.getArray()[ m_seqWarnings.getLength()-1] =\
                          OStringToOUString( str.makeStringAndClear() , RTL_TEXTENCODING_ASCII_US ); \
            return; \
        }\
        ((void)0)

#define ERROR_ASSERT(expr, Message) \
        if( ! (expr) ) { \
            BUILD_ERROR(expr, Message );\
            return; \
        }\
        ((void)0)

#define ERROR_EXCEPTION_ASSERT(expr, Message, Exception) \
    if( !(expr)) { \
        BUILD_ERROR(expr,Message);\
        m_seqExceptions.getArray()[ m_seqExceptions.getLength()-1] = Any( Exception );\
        return; \
    } \
    ((void)0)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
