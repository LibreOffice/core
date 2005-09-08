/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testfactreg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:00:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
