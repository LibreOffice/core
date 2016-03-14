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
#ifndef INCLUDED_CHART2_SOURCE_INC_MODIFYLISTENERHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_MODIFYLISTENERHELPER_HXX

#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>

#include "MutexContainer.hxx"
#include "charttoolsdllapi.hxx"

#include <list>
#include <algorithm>
#include <functional>
#include <utility>

namespace chart
{
namespace ModifyListenerHelper
{

OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::util::XModifyListener > createModifyEventForwarder();

/** This helper class serves as forwarder of modify events.  It can be used
    whenever an object has to send modify events after it gets a modify event of
    one of its children.

    <p>The listeners are held as WeakReferences if they support XWeak.  Thus the
    life time of the listeners is independent of the broadcaster's lifetime in
    this case.</p>
 */
class ModifyEventForwarder :
        public MutexContainer,
        public ::cppu::WeakComponentImplHelper<
            css::util::XModifyBroadcaster,
            css::util::XModifyListener >
{
public:
    ModifyEventForwarder();

    void FireEvent( const css::lang::EventObject & rEvent );

    void AddListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener );
    void RemoveListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener );

protected:
    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ WeakComponentImplHelperBase ____
    virtual void SAL_CALL disposing() override;

private:
    /// call disposing() at all listeners and remove all listeners
    void DisposeAndClear( const css::uno::Reference<
                              css::uno::XWeak > & xSource );

//     ::osl::Mutex & m_rMutex;
    ::cppu::OBroadcastHelper  m_aModifyListeners;

    typedef ::std::list<
            ::std::pair<
            css::uno::WeakReference< css::util::XModifyListener >,
            css::uno::Reference< css::util::XModifyListener > > >
        tListenerMap;

    tListenerMap m_aListenerMap;
};

namespace impl
{

template< class InterfaceRef >
struct addListenerFunctor : public ::std::unary_function< InterfaceRef, void >
{
    explicit addListenerFunctor( const css::uno::Reference< css::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
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
struct removeListenerFunctor : public ::std::unary_function< InterfaceRef, void >
{
    explicit removeListenerFunctor( const css::uno::Reference< css::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
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
struct addListenerToMappedElementFunctor : public ::std::unary_function< Pair, void >
{
    explicit addListenerToMappedElementFunctor( const css::uno::Reference< css::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
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
struct removeListenerFromMappedElementFunctor : public ::std::unary_function< Pair, void >
{
    explicit removeListenerFromMappedElementFunctor( const css::uno::Reference< css::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
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

template< class Container >
void addListenerToAllElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerFunctor< typename Container::value_type >( xListener ));
}

template< class Container >
void addListenerToAllMapElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerToMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void addListenerToAllSequenceElements(
    const css::uno::Sequence< T > & rSequence,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
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

template< class Container >
void removeListenerFromAllElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFunctor< typename Container::value_type >( xListener ));
}

template< class Container >
void removeListenerFromAllMapElements(
    const Container & rContainer,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFromMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void removeListenerFromAllSequenceElements(
    const css::uno::Sequence< T > & rSequence,
    const css::uno::Reference< css::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
                         impl::removeListenerFunctor< T >( xListener ));
}

} //  namespace ModifyListenerHelper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_MODIFYLISTENERHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
