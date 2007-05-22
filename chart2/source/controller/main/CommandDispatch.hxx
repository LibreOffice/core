/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommandDispatch.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:06:07 $
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
#ifndef CHART2_COMMANDDISPATCH_HXX
#define CHART2_COMMANDDISPATCH_HXX

#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#include <vector>
#include <map>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper2<
        ::com::sun::star::frame::XDispatch,
        ::com::sun::star::util::XModifyListener >
    CommandDispatch_Base;
}

/** This is the base class for an XDispatch.
 */
class CommandDispatch :
        public MutexContainer,
        public impl::CommandDispatch_Base
{
public:
    explicit CommandDispatch(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~CommandDispatch();

    // late initialisation, especially for adding as listener
    virtual void initialize();

protected:
    /** sends a status event for a specific command to all registered listeners
        or only the one given when set.

        This method should be overloaded.  The implementation should call
        fireStatusEventForURL and pass the xSingleListener argument to this
        method unchanged.

        @param rURL
            If empty, all available status events must be fired, otherwise only
            the one for the given command.

        @param xSingleListener
            If set, the event is only sent to this listener rather than to all
            registered ones.  Whenever a listener adds itself, this method is
            called with this parameter set to give an initial state.
     */
    virtual void fireStatusEvent(
        const ::rtl::OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener ) = 0;

    /** calls fireStatusEvent( ::rtl::OUString, xSingleListener )
     */
    void fireAllStatusEvents(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener );

    /** sends a status event for a specific command to all registered listeners
        or only the one given when set.

        @param xSingleListener
            If set, the event is only sent to this listener rather than to all
            registered ones.  Whenever a listener adds itself, this method is
            called with this parameter set to give an initial state.
     */
    void fireStatusEventForURL(
        const ::rtl::OUString & rURL,
        const ::com::sun::star::uno::Any & rState,
        bool bEnabled,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener =
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >(),
        const ::rtl::OUString & rFeatureDescriptor = ::rtl::OUString() );

    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing();

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >  m_xURLTransformer;

private:
    typedef ::std::map< ::rtl::OUString, ::cppu::OInterfaceContainerHelper* >
        tListenerMap;

    tListenerMap m_aListeners;

};

} //  namespace chart

// CHART2_COMMANDDISPATCH_HXX
#endif
