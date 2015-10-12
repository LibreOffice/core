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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCH_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCH_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/compbase.hxx>
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
typedef ::cppu::WeakComponentImplHelper<
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

        This method should be overridden.  The implementation should call
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
        const OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener ) = 0;

    /** calls fireStatusEvent( OUString, xSingleListener )
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
        const OUString & rURL,
        const ::com::sun::star::uno::Any & rState,
        bool bEnabled,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener =
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >(),
        const OUString & rFeatureDescriptor = OUString() );

    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addStatusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeStatusListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing() override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >  m_xURLTransformer;

private:
    typedef ::std::map< OUString, ::cppu::OInterfaceContainerHelper* >
        tListenerMap;

    tListenerMap m_aListeners;

};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCH_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
