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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCHCONTAINER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCHCONTAINER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <set>
#include <map>

namespace chart
{

class ChartController;
class DrawCommandDispatch;
class ShapeController;

/** @HTML

    Helper class for implementing the <code>XDispatchProvider</code> interface
    of the ChartController. This class handles all commands to queryDispatch and
    queryDispatches in the following way:

    <ul>
      <li>Check if there is a cached <code>XDispatch</code> for a given command.
        If so, use it.</li>
      <li>Check if the command is handled by this class, e.g. Undo.  If so,
        return a corresponding <code>XDispatch</code> implementation, and cache
        this implementation for later use</li>
      <li>Otherwise send the command to the chart dispatch provider, if it
        can handle this dispatch (determined by the list of commands given in
        <code>setChartDispatch()</code>).</li>
    </ul>

    <p>The <code>XDispatch</code>Provider is designed to return different
    <code>XDispatch</code> implementations for each command.  This class here
    decides which implementation to use for which command.</p>

    <p>As most commands need much information of the controller and are
    implemented there, the controller handles most of the commands itself (it
    also implements <code>XDispatch</code>).  Therefore it is set here as
    chart dispatch.</p>
 */
class CommandDispatchContainer
{
public:
    // note: the chart dispatcher should be removed when all commands are
    // handled by other dispatchers.  (Chart is currently the controller
    // itself)
    explicit CommandDispatchContainer(
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        ChartController* pController );

    void setModel(
        const css::uno::Reference< css::frame::XModel > & xModel );

    /** Set a chart dispatcher that is used for all commands contained in
        rChartCommands
     */
    void setChartDispatch(
        const css::uno::Reference< css::frame::XDispatch >& rChartDispatch,
        const ::std::set< OUString > & rChartCommands );

    /** Returns the dispatch that is able to do the command given in rURL, if
        implemented here.  If the URL is not implemented here, it should be
        checked whether the command is one of the commands given via
        the setChartDispatch() method.  If so, call the chart dispatch.

        <p>If all this fails, return an empty dispatch.</p>
     */
    css::uno::Reference< css::frame::XDispatch > getDispatchForURL(
                const css::util::URL & rURL );

    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > getDispatchesForURLs(
                const css::uno::Sequence< css::frame::DispatchDescriptor > & aDescriptors );

    void DisposeAndClear();

    static css::uno::Reference< css::frame::XDispatch >
        getContainerDispatchForURL(
            const css::uno::Reference< css::frame::XController > & xChartController,
            const css::util::URL & rURL );

    void setDrawCommandDispatch( DrawCommandDispatch* pDispatch );
    DrawCommandDispatch* getDrawCommandDispatch() { return m_pDrawCommandDispatch; }
    void setShapeController( ShapeController* pController );
    ShapeController* getShapeController() { return m_pShapeController; }

private:
    typedef
        ::std::map< OUString,
            css::uno::Reference< css::frame::XDispatch > >
        tDispatchMap;

    typedef
        ::std::vector< css::uno::Reference< css::frame::XDispatch > > tDisposeVector;

    mutable tDispatchMap m_aCachedDispatches;
    mutable tDisposeVector m_aToBeDisposedDispatches;

    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    css::uno::WeakReference< css::frame::XModel >         m_xModel;

    css::uno::Reference< css::frame::XDispatch >          m_xChartDispatcher;
    ::std::set< OUString >                                m_aChartCommands;

    ChartController* m_pChartController;
    DrawCommandDispatch* m_pDrawCommandDispatch;
    ShapeController* m_pShapeController;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_COMMANDDISPATCHCONTAINER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
