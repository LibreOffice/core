/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WeakListenerAdapter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:24:07 $
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
#ifndef CHART2_WEAKLISTENERADAPTER_HXX
#define CHART2_WEAKLISTENERADAPTER_HXX

#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace chart
{
// --------------------------------------------------------------------------------

/** Adapter that enables adding listeners as weak UNO references.  Thus, adding
    an object as listener to a broadcaster does not increase its reference
    count.

    <p>The helper class, of course, is held as hard reference at the
    broadcaster, but this should never be a problem as the adapter's life time
    depends on no other object.</p>

    <p>Note that in order to remove an object as listener, you have to remove
    the same wrapper that you added, i.e., you should store the adapter as a
    member in the adaptee class for later use.</p>
 */
template< class Listener >
    class WeakListenerAdapter : public
    ::cppu::WeakImplHelper1< Listener >
{
public:
    explicit WeakListenerAdapter( const ::com::sun::star::uno::Reference< Listener > & xListener ) :
            m_xListener( xListener )
    {}
    explicit WeakListenerAdapter( const ::com::sun::star::uno::WeakReference< Listener > & xListener ) :
            m_xListener( xListener )
    {}
    virtual ~WeakListenerAdapter()
    {}

protected:
    // ____ XEventListener (base of all listeners) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Reference<
              ::com::sun::star::lang::XEventListener > xEventListener(
                  ::com::sun::star::uno::Reference< Listener >( m_xListener), ::com::sun::star::uno::UNO_QUERY );
        if( xEventListener.is())
            xEventListener->disposing( Source );
    }

    ::com::sun::star::uno::Reference< Listener > getListener() const
    {
        return m_xListener;
    }

private:
    ::com::sun::star::uno::WeakReference< Listener > m_xListener;
};

// --------------------------------------------------------------------------------

class WeakModifyListenerAdapter :
        public WeakListenerAdapter< ::com::sun::star::util::XModifyListener >
{
public:
    explicit WeakModifyListenerAdapter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > & xListener );
    explicit WeakModifyListenerAdapter(
        const ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XModifyListener > & xListener );
    virtual ~WeakModifyListenerAdapter();

protected:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
};

// --------------------------------------------------------------------------------

class WeakSelectionChangeListenerAdapter :
        public WeakListenerAdapter< ::com::sun::star::view::XSelectionChangeListener >
{
public:
    explicit WeakSelectionChangeListenerAdapter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener );
    explicit WeakSelectionChangeListenerAdapter(
        const ::com::sun::star::uno::WeakReference< ::com::sun::star::view::XSelectionChangeListener > & xListener );
    virtual ~WeakSelectionChangeListenerAdapter();

protected:
    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
};

} //  namespace chart

// CHART2_WEAKLISTENERADAPTER_HXX
#endif
