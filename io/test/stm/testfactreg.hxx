/*************************************************************************
 *
 *  $RCSfile: testfactreg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
