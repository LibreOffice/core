/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModifyListenerHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:59:23 $
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
#ifndef CHART2_MODIFYLISTENERHELPER_HXX
#define CHART2_MODIFYLISTENERHELPER_HXX

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#include "MutexContainer.hxx"

#include <list>
#include <algorithm>
#include <functional>
#include <utility>

namespace com { namespace sun { namespace star {
namespace chart2
{
    class XChartDocument;
}}}}

namespace chart
{
namespace ModifyListenerHelper
{

// ================================================================================

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

    void FireEvent( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XWeak > & xSource );
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

// ================================================================================

class ModifyListenerOnDemmandRefresh :
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::util::XModifyListener >
{
public:
    explicit ModifyListenerOnDemmandRefresh( ::osl::Mutex & rMutex );

    void listenAtDocument( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    void update();
    bool needsUpdate() const;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::osl::Mutex & m_rMutex;
    bool m_bNeedsUpdate;
};

// ================================================================================

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

// --------------------------------------------------------------------------------


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
