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
#pragma once

#include <com/sun/star/lang/XComponent.hpp>

#include <utility>

namespace com::sun::star::lang { class XEventListener; }

namespace chart
{
namespace EventListenerHelper
{

namespace impl
{

template< class InterfaceRef >
struct addListenerFunctor
{
    explicit addListenerFunctor( css::uno::Reference< css::lang::XEventListener > xListener ) :
            m_xListener(std::move( xListener ))
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        css::uno::Reference< css::lang::XComponent >
              xBroadcaster( xObject, css::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addEventListener( m_xListener );
    }
private:
    css::uno::Reference< css::lang::XEventListener > m_xListener;
};

template< class InterfaceRef >
struct removeListenerFunctor
{
    explicit removeListenerFunctor( css::uno::Reference<  css::lang::XEventListener > xListener ) :
            m_xListener(std::move( xListener ))
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        css::uno::Reference< css::lang::XComponent >
              xBroadcaster( xObject, css::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeEventListener( m_xListener );
    }
private:
    css::uno::Reference< css::lang::XEventListener > m_xListener;
};

} //  namespace impl

template< class InterfaceRef >
void addListener(
    const InterfaceRef & xObject,
    const css::uno::Reference< css::lang::XEventListener > & xListener )
{
    if( xListener.is())
    {
        impl::addListenerFunctor< InterfaceRef > aFunctor( xListener );
        aFunctor( xObject );
    }
}

template< class Container >
void addListenerToAllElements(
    const Container & rContainer,
    const css::uno::Reference< css::lang::XEventListener > & xListener )
{
    if( xListener.is())
        std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerFunctor< typename Container::value_type >( xListener ));
}

template< class InterfaceRef >
void removeListener(
    const InterfaceRef & xObject,
    const css::uno::Reference< css::lang::XEventListener > & xListener )
{
    if( xListener.is())
    {
        impl::removeListenerFunctor< InterfaceRef > aFunctor( xListener );
        aFunctor( xObject );
    }
}

template< class Container >
void removeListenerFromAllElements(
    const Container & rContainer,
    const css::uno::Reference< css::lang::XEventListener > & xListener )
{
    if( xListener.is())
        std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFunctor< typename Container::value_type >( xListener ));
}

} //  namespace EventListenerHelper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
