/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_COMMANDDISPATCH_HXX
#define CHART2_COMMANDDISPATCH_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
