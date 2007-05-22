/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommandDispatchContainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:06:27 $
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
#ifndef CHART2_COMMANDDISPATCHCONTAINER_HXX
#define CHART2_COMMANDDISPATCHCONTAINER_HXX

#include "UndoManager.hxx"

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include <set>
#include <map>

namespace chart
{

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
      <li>Otherwise send the command to the fallback dispatch provider, if it
        can handle this dispatch (determined by the list of commands given in
        <code>setFallbackDispatch()</code>).</li>
    </ul>

    <p>The <code>XDispatch</code>Provider is designed to return different
    <code>XDispatch</code> implementations for each command.  This class here
    decides which implementation to use for which command.</p>

    <p>As most commands need much information of the controller and are
    implemented there, the controller handles most of the commands itself (it
    also implements <code>XDispatch</code>).  Therefore it is set here as
    fallback dispatch.</p>
 */
class CommandDispatchContainer
{
public:
    // note: the fallback dispatcher should be removed when all commands are
    // handled by other dispatchers.  (Fallback is currently the controller
    // itself)
    explicit CommandDispatchContainer(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

    void setModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    void setUndoManager( UndoManager * pUndoManager );

    /** Set a fallback dispatcher that is used for all commands contained in
        rFallbackCommands
     */
    void setFallbackDispatch(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatch > xFallbackDispatch,
        const ::std::set< ::rtl::OUString > & rFallbackCommands );

    /** Returns the dispatch that is able to do the command given in rURL, if
        implemented here.  If the URL is not implemented here, it should be
        checked whether the command is one of the commands given as fallback via
        the setFallbackDispatch() method.  If so, call the fallback dispatch.

        <p>If all this fails, return an empty dispatch.</p>
     */
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatch > getDispatchForURL(
                const ::com::sun::star::util::URL & rURL );

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatch > > getDispatchesForURLs(
                const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::frame::DispatchDescriptor > & aDescriptors );

    void DisposeAndClear();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
        getContainerDispatchForURL(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > & xChartController,
            const ::com::sun::star::util::URL & rURL );

private:
    typedef
        ::std::map< ::rtl::OUString,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XDispatch > >
        tDispatchMap;

    mutable tDispatchMap m_aCachedDispatches;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;
    UndoManager * m_pUndoManager;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > m_xFallbackDispatcher;
    ::std::set< ::rtl::OUString >                                          m_aFallbackCommands;

    ::std::set< ::rtl::OUString >                                          m_aContainerDocumentCommands;
};

} //  namespace chart

// CHART2_COMMANDDISPATCHCONTAINER_HXX
#endif
