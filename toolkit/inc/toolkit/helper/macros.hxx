/*************************************************************************
 *
 *  $RCSfile: macros.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2001-02-27 13:57:43 $
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

#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#define _TOOLKIT_HELPER_MACROS_HXX_

// -------------------------------------------------------------------------------------

#define IMPL_XUNOTUNNEL( ClassName ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( ClassName::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) ) \
    { \
        return (sal_Int64)this; \
    } \
    return 0; \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 >& ClassName::GetUnoTunnelId() throw() \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = NULL; \
    if( !pSeq ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pSeq ) \
        { \
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 ); \
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True ); \
            pSeq = &aSeq; \
        } \
    } \
    return *pSeq; \
} \
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? (ClassName*)xUT->getSomething( ClassName::GetUnoTunnelId() ) : NULL; \
}

#define IMPL_XUNOTUNNEL2( ClassName, BaseClass ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( ClassName::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) ) \
    { \
        return (sal_Int64)this; \
    } \
    return BaseClass::getSomething( rIdentifier ); \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 >& ClassName::GetUnoTunnelId() throw() \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = NULL; \
    if( !pSeq ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pSeq ) \
        { \
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 ); \
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True ); \
            pSeq = &aSeq; \
        } \
    } \
    return *pSeq; \
} \
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? (ClassName*)xUT->getSomething( ClassName::GetUnoTunnelId() ) : NULL; \
}

// -------------------------------------------------------------------------------------

#define IMPL_XTYPEPROVIDER_START( ClassName )   \
::com::sun::star::uno::Sequence< sal_Int8 > ClassName::getImplementationId() throw(::com::sun::star::uno::RuntimeException) \
{ \
    static ::cppu::OImplementationId* pId = NULL; \
    if( !pId ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( ! pId ) \
        { \
            static ::cppu::OImplementationId id( sal_False ); \
            pId = &id; \
        } \
    } \
    return (*pId).getImplementationId(); \
} \
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > ClassName::getTypes() throw(::com::sun::star::uno::RuntimeException) \
{ \
    static ::cppu::OTypeCollection* pCollection = NULL; \
    if( !pCollection ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pCollection ) \
        { \
            static ::cppu::OTypeCollection collection(

// -------------------------------------------------------------------------------------

#define IMPL_XTYPEPROVIDER_END \
            ); \
            pCollection = &collection; \
        } \
    } \
    return (*pCollection).getTypes(); \
}

// -------------------------------------------------------------------------------------

#define DECL_LISTENERMULTIPLEXER_START( ClassName, InterfaceName ) \
class ClassName : public ListenerMultiplexerBase, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException); \
    void                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { ListenerMultiplexerBase::acquire(); } \
    void                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { ListenerMultiplexerBase::release(); } \
    void                        SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// -------------------------------------------------------------------------------------

#define DECL_LISTENERMULTIPLEXER_END \
};

// -------------------------------------------------------------------------------------

#define IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ClassName, InterfaceName ) \
ClassName::ClassName( ::cppu::OWeakObject& rSource ) \
    : ListenerMultiplexerBase( rSource ) \
{ \
} \
::com::sun::star::uno::Any ClassName::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType, \
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, this ), \
                                        SAL_STATIC_CAST( InterfaceName*, this ) ); \
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType )); \
} \
void ClassName::disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException) \
{ \
}

// -------------------------------------------------------------------------------------

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ClassName, InterfaceName, MethodName, EventType ) \
void ClassName::MethodName( const EventType& e ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    EventType aMulti( e ); \
    aMulti.Source = &GetContext(); \
    ::cppu::OInterfaceIteratorHelper aIt( *this ); \
    while( aIt.hasMoreElements() ) \
        ((InterfaceName*)aIt.next())->MethodName( aMulti ); \
}

// -------------------------------------------------------------------------------------

#define DECLIMPL_SERVICEINFO( ImplName, ServiceName ) \
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) { return ::rtl::OUString::createFromAscii( "stardiv.Toolkit." #ImplName ); } \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)   \
                            { \
                                ::rtl::OUString aServiceName( ServiceName ); \
                                return ::com::sun::star::uno::Sequence< ::rtl::OUString >( &aServiceName, 1);\
                            } \
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException) \
                            { return rServiceName == ServiceName; }






#endif // _TOOLKIT_HELPER_MACROS_HXX_

