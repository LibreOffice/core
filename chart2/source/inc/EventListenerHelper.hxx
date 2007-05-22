/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EventListenerHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:16:13 $
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
#ifndef CHART2_EVENTLISTENERHELPER_HXX
#define CHART2_EVENTLISTENERHELPER_HXX

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

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
