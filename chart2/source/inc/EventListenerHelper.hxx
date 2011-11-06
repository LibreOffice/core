/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART2_EVENTLISTENERHELPER_HXX
#define CHART2_EVENTLISTENERHELPER_HXX

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <list>
#include <algorithm>
#include <functional>
#include <utility>

namespace chart
{
namespace EventListenerHelper
{

namespace impl
{

template< class InterfaceRef >
struct addListenerFunctor : public ::std::unary_function< InterfaceRef, void >
{
    explicit addListenerFunctor( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::lang::XEventListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
              xBroadcaster( xObject, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addEventListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > m_xListener;
};

template< class InterfaceRef >
struct removeListenerFunctor : public ::std::unary_function< InterfaceRef, void >
{
    explicit removeListenerFunctor( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::lang::XEventListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const InterfaceRef & xObject )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
              xBroadcaster( xObject, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeEventListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > m_xListener;
};

template< class Pair >
struct addListenerToMappedElementFunctor : public ::std::unary_function< Pair, void >
{
    explicit addListenerToMappedElementFunctor( const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::lang::XEventListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const Pair & aPair )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
              xBroadcaster( aPair.second, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->addEventListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > m_xListener;
};

template< class Pair >
struct removeListenerFromMappedElementFunctor : public ::std::unary_function< Pair, void >
{
    explicit removeListenerFromMappedElementFunctor( const ::com::sun::star::uno::Reference<
                                                         ::com::sun::star::lang::XEventListener > & xListener ) :
            m_xListener( xListener )
    {}

    void operator() ( const Pair & aPair )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
              xBroadcaster( aPair.second, ::com::sun::star::uno::UNO_QUERY );
        if( xBroadcaster.is() && m_xListener.is())
            xBroadcaster->removeEventListener( m_xListener );
    }
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > m_xListener;
};

} //  namespace impl

// --------------------------------------------------------------------------------

template< class InterfaceRef >
void addListener(
    const InterfaceRef & xObject,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener > & xListener )
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
        ::com::sun::star::lang::XEventListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerFunctor< typename Container::value_type >( xListener ));
}

template< class Container >
void addListenerToAllMapElements(
    const Container & rContainer,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::addListenerToMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void addListenerToAllSequenceElements(
    const ::com::sun::star::uno::Sequence< T > & rSequence,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
                         impl::addListenerFunctor< T >( xListener ));
}

template< class InterfaceRef >
void removeListener(
    const InterfaceRef & xObject,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener > & xListener )
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
        ::com::sun::star::lang::XEventListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFunctor< typename Container::value_type >( xListener ));
}

template< class Container >
void removeListenerFromAllMapElements(
    const Container & rContainer,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rContainer.begin(), rContainer.end(),
                         impl::removeListenerFromMappedElementFunctor< typename Container::value_type >( xListener ));
}

template< typename T >
void removeListenerFromAllSequenceElements(
    const ::com::sun::star::uno::Sequence< T > & rSequence,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener > & xListener )
{
    if( xListener.is())
        ::std::for_each( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
                         impl::removeListenerFunctor< T >( xListener ));
}

} //  namespace EventListenerHelper
} //  namespace chart

// CHART2_EVENTLISTENERHELPER_HXX
#endif
