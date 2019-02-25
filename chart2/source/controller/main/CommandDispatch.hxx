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

#include <MutexContainer.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <map>

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }
namespace com { namespace sun { namespace star { namespace util { class XURLTransformer; } } } }

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
        css::frame::XDispatch,
        css::util::XModifyListener >
    CommandDispatch_Base;
}

/** This is the base class for an XDispatch.
 */
class CommandDispatch :
        public MutexContainer,
        public impl::CommandDispatch_Base
{
public:
    explicit CommandDispatch( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~CommandDispatch() override;

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
        const css::uno::Reference< css::frame::XStatusListener > & xSingleListener ) = 0;

    /** calls fireStatusEvent( OUString, xSingleListener )
     */
    void fireAllStatusEvents(
        const css::uno::Reference< css::frame::XStatusListener > & xSingleListener );

    /** sends a status event for a specific command to all registered listeners
        or only the one given when set.

        @param xSingleListener
            If set, the event is only sent to this listener rather than to all
            registered ones.  Whenever a listener adds itself, this method is
            called with this parameter set to give an initial state.
     */
    void fireStatusEventForURL(
        const OUString & rURL,
        const css::uno::Any & rState,
        bool bEnabled,
        const css::uno::Reference< css::frame::XStatusListener > & xSingleListener );

    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;
    virtual void SAL_CALL addStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL ) override;
    virtual void SAL_CALL removeStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL ) override;

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing() override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::util::XURLTransformer >  m_xURLTransformer;

    typedef std::map< OUString, ::comphelper::OInterfaceContainerHelper2* >
        tListenerMap;

    tListenerMap m_aListeners;

};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCH_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
