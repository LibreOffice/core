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

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <mutex>
#include <algorithm>
#include <utility>

namespace chart
{

/** This helper class serves as forwarder of modify events.  It can be used
    whenever an object has to send modify events after it gets a modify event of
    one of its children.
 */
class ModifyEventForwarder final :
        public ::comphelper::WeakComponentImplHelper<
            css::util::XModifyBroadcaster,
            css::util::XModifyListener >
{
public:
    ModifyEventForwarder();

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

private:
    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ WeakComponentImplHelperBase ____
    virtual void disposing(std::unique_lock<std::mutex>& ) override;

    comphelper::OInterfaceContainerHelper4<css::util::XModifyListener>  m_aModifyListeners;
};

}

namespace chart::ModifyListenerHelper
{

namespace impl
{

template< class InterfaceRef >
struct addListenerFunctor
{
    explicit addListenerFunctor( css::uno::Reference< css::util::XModifyListener > xListener ) :
            m_xListener(std::move( xListener ))
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        css::uno::Reference< css::util::XModifyBroadcaster >
              xBroadcaster( xObject, css::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addModifyListener( m_xListener );
    }
private:
    css::uno::Reference< css::util::XModifyListener > m_xListener;
};

template< class InterfaceRef >
struct removeListenerFunctor
{
    explicit removeListenerFunctor( css::uno::Reference< css::util::XModifyListener > xListener ) :
            m_xListener(std::move( xListener ))
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        css::uno::Reference< css::util::XModifyBroadcaster >
              xBroadcaster( xObject, css::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeModifyListener( m_xListener );
    }
private:
    css::uno::Reference< css::util::XModifyListener > m_xListener;
};

template< class Pair >
struct addListenerToMappedElementFunctor
{
    explicit addListenerToMappedElementFunctor( css::uno::Reference< css::util::XModifyListener >  xListener ) :
            m_xListener(std::move( xListener ))
    {}

    void operator() ( const Pair & aPair )
    {
        css::uno::Reference< css::util::XModifyBroadcaster >
              xBroadcaster( aPair.second, css::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addModifyListener( m_xListener );
    }
private:
    css::uno::Reference< css::util::XModifyListener > m_xListener;
};

template< class Pair >
struct removeListenerFromMappedElementFunctor
{
    explicit removeListenerFromMappedElementFunctor( css::uno::Reference< css::util::XModifyListener > xListener ) :
            m_xListener(std::move( xListener ))
    {}

    void operator() ( const Pair & aPair )
    {
        css::uno::Reference< css::util::XModifyBroadcaster >
              xBroadcaster( aPair.second, css::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeModifyListener( m_xListener );
    }
private:
    css::uno::Reference< css::util::XModifyListener > m_xListener;
};

} //  namespace impl

template< class InterfaceRef >
void addListener(
    const InterfaceRef & xObject,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
    {
        impl::addListenerFunctor< InterfaceRef > aFunctor( xListener );
        aFunctor( xObject );
    }
}
template< class T >
void addListener(
    const rtl::Reference<T> & xBroadcaster,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xBroadcaster && xListener  )
        xBroadcaster->addModifyListener( xListener );
}

template< class Container >
void addListenerToAllElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerFunctor< typename Container::value_type >( xListener ));
}

template< class T >
void addListenerToAllElements(
    const std::vector<rtl::Reference<T>> & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( !xListener )
        return;
    for (auto const & i : rContainer)
        i->addModifyListener(xListener);
}

template< class Container >
void addListenerToAllMapElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerToMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void addListenerToAllSequenceElements(
    const css::uno::Sequence< T > & rSequence,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        std::for_each( rSequence.begin(), rSequence.end(),
                         impl::addListenerFunctor< T >( xListener ));
}

template< class InterfaceRef >
void removeListener(
    const InterfaceRef & xObject,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
    {
        impl::removeListenerFunctor< InterfaceRef > aFunctor( xListener );
        aFunctor( xObject );
    }
}

template< class T >
void removeListener(
    const rtl::Reference<T> & xBroadcaster,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xBroadcaster && xListener  )
        xBroadcaster->removeModifyListener( xListener );
}

template< class Container >
void removeListenerFromAllElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFunctor< typename Container::value_type >( xListener ));
}

template< class T >
void removeListenerFromAllElements(
    const std::vector<rtl::Reference<T>> & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( !xListener )
        return;
    for (auto const & i : rContainer)
        i->removeModifyListener(xListener);
}

template< class Container >
void removeListenerFromAllMapElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFromMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void removeListenerFromAllSequenceElements(
    const css::uno::Sequence< T > & rSequence,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        std::for_each( rSequence.begin(), rSequence.end(),
                         impl::removeListenerFunctor< T >( xListener ));
}

} //  namespace chart::ModifyListenerHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
