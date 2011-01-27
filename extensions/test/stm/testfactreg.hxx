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
XInterfaceRef OPipeTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OPipeTest_getSupportedServiceNames(void) THROWS( () );
UString     OPipeTest_getServiceName() THROWS( () );
UString     OPipeTest_getImplementationName() THROWS( () );

XInterfaceRef ODataStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> ODataStreamTest_getSupportedServiceNames( int i) THROWS( () );
UString     ODataStreamTest_getServiceName( int i) THROWS( ( ) );
UString     ODataStreamTest_getImplementationName( int i) THROWS( () );

XInterfaceRef OMarkableOutputStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OMarkableOutputStreamTest_getSupportedServiceNames(void) THROWS( () );
UString     OMarkableOutputStreamTest_getServiceName() THROWS( () );
UString     OMarkableOutputStreamTest_getImplementationName() THROWS( () );

XInterfaceRef OMarkableInputStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OMarkableInputStreamTest_getSupportedServiceNames(void) THROWS( () );
UString     OMarkableInputStreamTest_getServiceName() THROWS( () );
UString     OMarkableInputStreamTest_getImplementationName() THROWS( () );

XInterfaceRef OObjectStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OObjectStreamTest_getSupportedServiceNames( int i) THROWS( () );
UString     OObjectStreamTest_getServiceName( int i) THROWS( () );
UString     OObjectStreamTest_getImplementationName( int i) THROWS( () );

XInterfaceRef       OMyPersistObject_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString>   OMyPersistObject_getSupportedServiceNames( ) THROWS( () );
UString             OMyPersistObject_getServiceName( ) THROWS( () );
UString             OMyPersistObject_getImplementationName( ) THROWS( () );

Sequence<BYTE> createSeq( char * p );
Sequence<BYTE> createIntSeq( INT32 i );

#define BUILD_ERROR(expr, Message)\
        {\
            m_seqErrors.realloc( m_seqErrors.getLen() + 1 ); \
        m_seqExceptions.realloc(  m_seqExceptions.getLen() + 1 ); \
        String str; \
        str += __FILE__;\
        str += " "; \
        str += "(" ; \
        str += __LINE__ ;\
        str += ")\n";\
        str += "[ " ; \
        str += #expr; \
        str += " ] : " ; \
        str += Message; \
        m_seqErrors.getArray()[ m_seqErrors.getLen()-1] = StringToOUString( str , CHARSET_SYSTEM ); \
        }\
        ((void)0)


#define WARNING_ASSERT(expr, Message) \
        if( ! (expr) ) { \
            m_seqWarnings.realloc( m_seqErrors.getLen() +1 ); \
            String str;\
            str += __FILE__;\
            str += " "; \
            str += "(" ; \
            str += __LINE__ ;\
            str += ")\n";\
            str += "[ " ; \
            str += #expr; \
            str += " ] : " ; \
            str += Message; \
            m_seqWarnings.getArray()[ m_seqWarnings.getLen()-1] = StringToOUString( str , CHARSET_SYSTEM ); \
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
        m_seqExceptions.getArray()[ m_seqExceptions.getLen()-1] = UsrAny( Exception );\
        return; \
    } \
    ((void)0)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
