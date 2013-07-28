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
#ifndef CHART2_MODIFYLISTENERHELPER_HXX
#define CHART2_MODIFYLISTENERHELPER_HXX

#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase2.hxx>
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

OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > createModifyEventForwarder();

/** This helper class serves as forwarder of modify events.  It can be used
    whenever an object has to send modify events after it gets a modify event of
    one of its children.

    <p>The listeners are held as WeakReferences if they support XWeak.  Thus the
    life time of the listeners is independent of the broadcaster's lifetime in
    this case.</p>
 */
class ModifyEventForwarder :
        public MutexContainer,
        public ::cppu::WeakComponentImplHelper2<
            ::com::sun::star::util::XModifyBroadcaster,
            ::com::sun::star::util::XModifyListener >
{
public:
    ModifyEventForwarder();

    void FireEvent( const ::com::sun::star::lang::EventObject & rEvent );

    void AddListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener );
    void RemoveListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener );

protected:
    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WeakComponentImplHelperBase ____
    virtual void SAL_CALL disposing();

private:
    /// call disposing() at all listeners and remove all listeners
    void DisposeAndClear( const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XWeak > & xSource );

//     ::osl::Mutex & m_rMutex;
    ::cppu::OBroadcastHelper  m_aModifyListeners;

    typedef ::std::list<
            ::std::pair<
            ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XModifyListener >,
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > > >
        tListenerMap;

    tListenerMap m_aListenerMap;
};

namespace impl
{

template< class InterfaceRef >
struct addListenerFunctor : public ::std::unary_function< InterfaceRef, void >
{
    explicit addListenerFunctor( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >
              xBroadcaster( xObject, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addModifyListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xListener;
};

template< class InterfaceRef >
struct removeListenerFunctor : public ::std::unary_function< InterfaceRef, void >
{
    explicit removeListenerFunctor( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >
              xBroadcaster( xObject, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeModifyListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xListener;
};

template< class Pair >
struct addListenerToMappedElementFunctor : public ::std::unary_function< Pair, void >
{
    explicit addListenerToMappedElementFunctor( const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const Pair & aPair )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >
              xBroadcaster( aPair.second, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addModifyListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xListener;
};

template< class Pair >
struct removeListenerFromMappedElementFunctor : public ::std::unary_function< Pair, void >
{
    explicit removeListenerFromMappedElementFunctor( const ::com::sun::star::uno::Reference<
                                                         ::com::sun::star::util::XModifyListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const Pair & aPair )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >
              xBroadcaster( aPair.second, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeModifyListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xListener;
};

} //  namespace impl

template< class InterfaceRef >
void addListener(
    const InterfaceRef & xObject,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
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
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerFunctor< typename Container::value_type >( xListener ));
}

template< class Container >
void addListenerToAllMapElements(
    const Container & rContainer,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerToMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void addListenerToAllSequenceElements(
    const ::com::sun::star::uno::Sequence< T > & rSequence,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
                         impl::addListenerFunctor< T >( xListener ));
}

template< class InterfaceRef >
void removeListener(
    const InterfaceRef & xObject,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
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
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFunctor< typename Container::value_type >( xListener ));
}

template< class Container >
void removeListenerFromAllMapElements(
    const Container & rContainer,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFromMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void removeListenerFromAllSequenceElements(
    const ::com::sun::star::uno::Sequence< T > & rSequence,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
                         impl::removeListenerFunctor< T >( xListener ));
}

} //  namespace ModifyListenerHelper
} //  namespace chart

// CHART2_MODIFYLISTENERHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
