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

#ifndef INCLUDED_SAX_TEST_SAX_FACTORY_HXX
#define INCLUDED_SAX_TEST_SAX_FACTORY_HXX

#include <rtl/strbuf.hxx>

namespace sax_test {
Reference< XInterface > SAL_CALL OSaxWriterTest_CreateInstance(
    const Reference< XMultiServiceFactory > & rSMgr ) throw ( Exception );
OUString    OSaxWriterTest_getServiceName( ) throw();
OUString    OSaxWriterTest_getImplementationName( ) throw();
Sequence<OUString> OSaxWriterTest_getSupportedServiceNames( ) throw();
}
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

#endif // INCLUDED_SAX_TEST_SAX_FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
