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
XInterfaceRef OPipeTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OPipeTest_getSupportedServiceNames(void) THROWS(());
UString     OPipeTest_getServiceName() THROWS(());
UString     OPipeTest_getImplementationName() THROWS(());

XInterfaceRef ODataStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> ODataStreamTest_getSupportedServiceNames( int i) THROWS(());
UString     ODataStreamTest_getServiceName( int i) THROWS( ( ) );
UString     ODataStreamTest_getImplementationName( int i) THROWS(());

XInterfaceRef OMarkableOutputStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OMarkableOutputStreamTest_getSupportedServiceNames(void) THROWS(());
UString     OMarkableOutputStreamTest_getServiceName() THROWS(());
UString     OMarkableOutputStreamTest_getImplementationName() THROWS(());

XInterfaceRef OMarkableInputStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OMarkableInputStreamTest_getSupportedServiceNames(void) THROWS(());
UString     OMarkableInputStreamTest_getServiceName() THROWS(());
UString     OMarkableInputStreamTest_getImplementationName() THROWS(());

XInterfaceRef OObjectStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString> OObjectStreamTest_getSupportedServiceNames( int i) THROWS(());
UString     OObjectStreamTest_getServiceName( int i) THROWS(());
UString     OObjectStreamTest_getImplementationName( int i) THROWS(());

XInterfaceRef       OMyPersistObject_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception));
Sequence<UString>   OMyPersistObject_getSupportedServiceNames( ) THROWS(());
UString             OMyPersistObject_getServiceName( ) THROWS(());
UString             OMyPersistObject_getImplementationName( ) THROWS(());

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
