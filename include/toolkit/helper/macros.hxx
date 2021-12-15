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

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

#define IMPL_IMPLEMENTATION_ID( ClassName ) \
css::uno::Sequence< sal_Int8 > ClassName::getImplementationId() \
{ \
    return css::uno::Sequence<sal_Int8>(); \
}


#define DECL_LISTENERMULTIPLEXER_START( ClassName, InterfaceName ) \
class ClassName final : public ListenerMultiplexerBase<InterfaceName>, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override; \
    void                        SAL_CALL acquire() noexcept override; \
    void                        SAL_CALL release() noexcept override; \
    void                        SAL_CALL disposing( const css::lang::EventObject& Source ) override;


#define DECL_LISTENERMULTIPLEXER_START_DLLPUB( ClassName, InterfaceName ) \
class TOOLKIT_DLLPUBLIC ClassName final : public ListenerMultiplexerBase<InterfaceName>, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override; \
    void                        SAL_CALL acquire() noexcept override; \
    void                        SAL_CALL release() noexcept override; \
    void                        SAL_CALL disposing( const css::lang::EventObject& Source ) override;


#define DECL_LISTENERMULTIPLEXER_END \
};


#define IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ClassName, InterfaceName ) \
ClassName::ClassName( ::cppu::OWeakObject& rSource ) \
    : ListenerMultiplexerBase<InterfaceName>(rSource) \
{ \
} \
void SAL_CALL ClassName::acquire() noexcept { ListenerMultiplexerBase::acquire(); } \
void SAL_CALL ClassName::release() noexcept { ListenerMultiplexerBase::release(); } \
css::uno::Any ClassName::queryInterface( const css::uno::Type & rType ) \
{ \
    css::uno::Any aRet = ::cppu::queryInterface( rType, \
                                        (static_cast< css::lang::XEventListener* >(this)), \
                                        (static_cast< InterfaceName* >(this)) ); \
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType )); \
} \
void ClassName::disposing( const css::lang::EventObject& ) \
{ \
}


#if OSL_DEBUG_LEVEL > 0
    #define DISPLAY_EXCEPTION( ClassName, MethodName )    \
        css::uno::Any ex( cppu::getCaughtException() ); \
        SAL_WARN( "toolkit", #ClassName "::" #MethodName ": caught an exception! " << exceptionToString(ex));
#else
    #define DISPLAY_EXCEPTION( ClassName, MethodName )
#endif

#define IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( ClassName, InterfaceName, MethodName, ParamType1 ) \
{ \
    ParamType1 aMulti( evt ); \
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this); \
    while( aIt.hasMoreElements() ) \
    { \
        css::uno::Reference<InterfaceName> xListener(aIt.next()); \
        try \
        { \
            xListener->MethodName( aMulti ); \
        } \
        catch(const css::lang::DisposedException& e) \
        { \
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" ); \
            if ( e.Context == xListener || !e.Context.is() ) \
                aIt.remove(); \
        } \
        catch(const css::uno::RuntimeException&) \
        { \
            DISPLAY_EXCEPTION( ClassName, MethodName ) \
        } \
    } \
}

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType ) \
{ \
    EventType aMulti( evt ); \
    aMulti.Source = &GetContext(); \
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this); \
    while( aIt.hasMoreElements() ) \
    { \
        css::uno::Reference<InterfaceName> xListener(aIt.next()); \
        try \
        { \
            xListener->MethodName( aMulti ); \
        } \
        catch(const css::lang::DisposedException& e) \
        { \
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" ); \
            if ( e.Context == xListener || !e.Context.is() ) \
                aIt.remove(); \
        } \
        catch(const css::uno::RuntimeException&) \
        { \
            DISPLAY_EXCEPTION( ClassName, MethodName ) \
        } \
    } \
}

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_EXCEPTION( ClassName, InterfaceName, MethodName, EventType, Exception ) \
void ClassName::MethodName( const EventType& evt ) \
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType )

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ClassName, InterfaceName, MethodName, EventType ) \
void ClassName::MethodName( const EventType& evt ) \
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_BODY( ClassName, InterfaceName, MethodName, EventType )

#define DECLIMPL_SERVICEINFO_DERIVED( ImplName, BaseClass, ServiceName ) \
    OUString SAL_CALL getImplementationName(  ) override { return "stardiv.Toolkit." #ImplName; } \
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override   \
                            { \
                                css::uno::Sequence< OUString > aNames = BaseClass::getSupportedServiceNames( ); \
                                aNames.realloc( aNames.getLength() + 1 ); \
                                aNames.getArray()[ aNames.getLength() - 1 ] = ServiceName; \
                                return aNames; \
                            } \

#endif // INCLUDED_TOOLKIT_HELPER_MACROS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
