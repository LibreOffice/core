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

#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#define _TOOLKIT_HELPER_MACROS_HXX_

#include <string.h>

#include <comphelper/servicehelper.hxx>

#define IMPL_XUNOTUNNEL_MINIMAL( ClassName ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( ClassName::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) ) \
    { \
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(this)); \
    } \
    return 0; \
} \
namespace \
{ \
    class the##ClassName##UnoTunnelId : public rtl::Static< UnoTunnelIdInit, the##ClassName##UnoTunnelId> {}; \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 >& ClassName::GetUnoTunnelId() throw() \
{ \
    return the##ClassName##UnoTunnelId::get().getSeq(); \
}

#define IMPL_XUNOTUNNEL( ClassName ) \
IMPL_XUNOTUNNEL_MINIMAL( ClassName ) \
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? reinterpret_cast<ClassName*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( ClassName::GetUnoTunnelId() ))) : NULL; \
}

#define IMPL_XUNOTUNNEL2( ClassName, BaseClass ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( ClassName::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) ) \
    { \
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(this)); \
    } \
    return BaseClass::getSomething( rIdentifier ); \
} \
namespace \
{ \
    class the##ClassName##UnoTunnelId : public rtl::Static< UnoTunnelIdInit, the##ClassName##UnoTunnelId> {}; \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 >& ClassName::GetUnoTunnelId() throw() \
{ \
    return the##ClassName##UnoTunnelId::get().getSeq(); \
} \
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? reinterpret_cast<ClassName*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( ClassName::GetUnoTunnelId() ))) : NULL; \
}

// -------------------------------------------------------------------------------------

#define IMPL_IMPLEMENTATION_ID( ClassName ) \
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
}

#define IMPL_XTYPEPROVIDER_START( ClassName )   \
IMPL_IMPLEMENTATION_ID( ClassName ) \
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > ClassName::getTypes() throw(::com::sun::star::uno::RuntimeException) \
{ \
    static ::cppu::OTypeCollection* pCollection = NULL; \
    if( !pCollection ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pCollection ) \
        { \
            static ::cppu::OTypeCollection collection( \
            getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider>* ) NULL ),

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
    void                        SAL_CALL acquire() throw()  { ListenerMultiplexerBase::acquire(); } \
    void                        SAL_CALL release() throw()  { ListenerMultiplexerBase::release(); } \
    void                        SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// -------------------------------------------------------------------------------------

#define DECL_LISTENERMULTIPLEXER_START_DLLPUB( ClassName, InterfaceName ) \
class TOOLKIT_DLLPUBLIC ClassName : public ListenerMultiplexerBase, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException); \
    void                        SAL_CALL acquire() throw()  { ListenerMultiplexerBase::acquire(); } \
    void                        SAL_CALL release() throw()  { ListenerMultiplexerBase::release(); } \
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
                                        (static_cast< ::com::sun::star::lang::XEventListener* >(this)), \
                                        (static_cast< InterfaceName* >(this)) ); \
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType )); \
} \
void ClassName::disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException) \
{ \
}

// -------------------------------------------------------------------------------------

#if OSL_DEBUG_LEVEL > 0
    #define DISPLAY_EXCEPTION( ClassName, MethodName, e )    \
        ::rtl::OString sMessage( #ClassName "::" #MethodName ": caught an exception!\n" ); \
        sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US ); \
        OSL_FAIL( sMessage.getStr() );
#else
    #define DISPLAY_EXCEPTION( ClassName, MethodName, e ) (void)e;
#endif

#define IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_2PARAM( ClassName, InterfaceName, MethodName, ParamType1, ParamType2 ) \
{ \
    ParamType1 aMulti( evt ); \
    ParamType2 aMulti2( evt2 ); \
    ::cppu::OInterfaceIteratorHelper aIt( *this ); \
    while( aIt.hasMoreElements() ) \
    { \
        ::com::sun::star::uno::Reference< InterfaceName > xListener( \
            static_cast< InterfaceName* >( aIt.next() ) ); \
        try \
        { \
            xListener->MethodName( aMulti, aMulti2 ); \
        } \
        catch(const ::com::sun::star::lang::DisposedException& e) \
        { \
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" ); \
            if ( e.Context == xListener || !e.Context.is() ) \
                aIt.remove(); \
        } \
        catch(const ::com::sun::star::uno::RuntimeException& e) \
        { \
            DISPLAY_EXCEPTION( ClassName, MethodName, e ) \
        } \
    } \
}

#define IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( ClassName, InterfaceName, MethodName, ParamType1 ) \
{ \
    ParamType1 aMulti( evt ); \
    ::cppu::OInterfaceIteratorHelper aIt( *this ); \
    while( aIt.hasMoreElements() ) \
    { \
        ::com::sun::star::uno::Reference< InterfaceName > xListener( \
            static_cast< InterfaceName* >( aIt.next() ) ); \
        try \
        { \
            xListener->MethodName( aMulti ); \
        } \
        catch(const ::com::sun::star::lang::DisposedException& e) \
        { \
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" ); \
            if ( e.Context == xListener || !e.Context.is() ) \
                aIt.remove(); \
        } \
        catch(const ::com::sun::star::uno::RuntimeException& e) \
        { \
            DISPLAY_EXCEPTION( ClassName, MethodName, e ) \
        } \
    } \
}

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType ) \
{ \
    EventType aMulti( evt ); \
    aMulti.Source = &GetContext(); \
    ::cppu::OInterfaceIteratorHelper aIt( *this ); \
    while( aIt.hasMoreElements() ) \
    { \
        ::com::sun::star::uno::Reference< InterfaceName > xListener( \
            static_cast< InterfaceName* >( aIt.next() ) ); \
        try \
        { \
            xListener->MethodName( aMulti ); \
        } \
        catch(const ::com::sun::star::lang::DisposedException& e) \
        { \
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" ); \
            if ( e.Context == xListener || !e.Context.is() ) \
                aIt.remove(); \
        } \
        catch(const ::com::sun::star::uno::RuntimeException& e) \
        { \
            DISPLAY_EXCEPTION( ClassName, MethodName, e ) \
        } \
    } \
}

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_EXCEPTION( ClassName, InterfaceName, MethodName, EventType, Exception ) \
void ClassName::MethodName( const EventType& evt ) throw(::com::sun::star::uno::RuntimeException, Exception) \
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType )

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ClassName, InterfaceName, MethodName, EventType ) \
void ClassName::MethodName( const EventType& evt ) throw(::com::sun::star::uno::RuntimeException) \
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType )

// -------------------------------------------------------------------------------------

#define DECLIMPL_SUPPORTS_SERVICE( ) \
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() ); \
        const ::rtl::OUString* pSupported = aServiceNames.getConstArray(); \
        const ::rtl::OUString* pSupportedEnd = pSupported + aServiceNames.getLength(); \
        for ( ; pSupported != pSupportedEnd; ++pSupported ) \
            if ( *pSupported == rServiceName ) \
                return sal_True; \
        return sal_False; \
    }

// -------------------------------------------------------------------------------------

#define DECLIMPL_SERVICEINFO_DERIVED( ImplName, BaseClass, ServiceName ) \
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) { return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stardiv.Toolkit." #ImplName )); } \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)   \
                            { \
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames = BaseClass::getSupportedServiceNames( ); \
                                aNames.realloc( aNames.getLength() + 1 ); \
                                aNames[ aNames.getLength() - 1 ] = ::rtl::OUString::createFromAscii( ServiceName ); \
                                return aNames; \
                            } \

// -------------------------------------------------------------------------------------

#define DECLIMPL_SERVICEINFO( ImplName, ServiceName ) \
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) { return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "stardiv.Toolkit." #ImplName )); } \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)   \
                            { \
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 ); \
                                aNames[ 0 ] = ::rtl::OUString::createFromAscii( ServiceName ); \
                                return aNames; \
                            } \
    DECLIMPL_SUPPORTS_SERVICE( )






#endif // _TOOLKIT_HELPER_MACROS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
