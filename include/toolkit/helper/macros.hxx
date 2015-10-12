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

#ifndef INCLUDED_TOOLKIT_HELPER_MACROS_HXX
#define INCLUDED_TOOLKIT_HELPER_MACROS_HXX

#include <string.h>

#include <comphelper/servicehelper.hxx>

#define IMPL_XUNOTUNNEL_MINIMAL( ClassName ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) \
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
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? reinterpret_cast<ClassName*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( ClassName::GetUnoTunnelId() ))) : NULL; \
}

#define IMPL_IMPLEMENTATION_ID( ClassName ) \
::com::sun::star::uno::Sequence< sal_Int8 > ClassName::getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) \
{ \
    return css::uno::Sequence<sal_Int8>(); \
}

#define IMPL_XTYPEPROVIDER_START( ClassName )   \
IMPL_IMPLEMENTATION_ID( ClassName ) \
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > ClassName::getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) \
{ \
    static ::cppu::OTypeCollection* pCollection = NULL; \
    if( !pCollection ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pCollection ) \
        { \
            static ::cppu::OTypeCollection collection( \
            cppu::UnoType<css::lang::XTypeProvider>::get(),



#define IMPL_XTYPEPROVIDER_END \
            ); \
            pCollection = &collection; \
        } \
    } \
    return (*pCollection).getTypes(); \
}



#define DECL_LISTENERMULTIPLEXER_START( ClassName, InterfaceName ) \
class ClassName : public ListenerMultiplexerBase, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override; \
    void                        SAL_CALL acquire() throw() override; \
    void                        SAL_CALL release() throw() override; \
    void                        SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;



#define DECL_LISTENERMULTIPLEXER_START_DLLPUB( ClassName, InterfaceName ) \
class TOOLKIT_DLLPUBLIC ClassName : public ListenerMultiplexerBase, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override; \
    void                        SAL_CALL acquire() throw() override; \
    void                        SAL_CALL release() throw() override; \
    void                        SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;



#define DECL_LISTENERMULTIPLEXER_END \
};



#define IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ClassName, InterfaceName ) \
ClassName::ClassName( ::cppu::OWeakObject& rSource ) \
    : ListenerMultiplexerBase( rSource ) \
{ \
} \
void SAL_CALL ClassName::acquire() throw() { ListenerMultiplexerBase::acquire(); } \
void SAL_CALL ClassName::release() throw() { ListenerMultiplexerBase::release(); } \
::com::sun::star::uno::Any ClassName::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) \
{ \
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType, \
                                        (static_cast< ::com::sun::star::lang::XEventListener* >(this)), \
                                        (static_cast< InterfaceName* >(this)) ); \
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType )); \
} \
void ClassName::disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException, std::exception) \
{ \
}



#if OSL_DEBUG_LEVEL > 0
    #define DISPLAY_EXCEPTION( ClassName, MethodName, e )    \
        OString sMessage( #ClassName "::" #MethodName ": caught an exception!\n" ); \
        sMessage += OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US ); \
        OSL_FAIL( sMessage.getStr() );
#else
    #define DISPLAY_EXCEPTION( ClassName, MethodName, e ) (void)e;
#endif

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
void ClassName::MethodName( const EventType& evt ) throw(::com::sun::star::uno::RuntimeException, Exception, std::exception) \
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType )

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ClassName, InterfaceName, MethodName, EventType ) \
void ClassName::MethodName( const EventType& evt ) throw(::com::sun::star::uno::RuntimeException, std::exception) \
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType )

#define DECLIMPL_SERVICEINFO_DERIVED( ImplName, BaseClass, ServiceName ) \
    OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return OUString("stardiv.Toolkit." #ImplName ); } \
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) override   \
                            { \
                                ::com::sun::star::uno::Sequence< OUString > aNames = BaseClass::getSupportedServiceNames( ); \
                                aNames.realloc( aNames.getLength() + 1 ); \
                                aNames[ aNames.getLength() - 1 ] = ServiceName; \
                                return aNames; \
                            } \

#endif // INCLUDED_TOOLKIT_HELPER_MACROS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
