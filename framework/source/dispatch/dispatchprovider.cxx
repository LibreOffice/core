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

#include <dispatch/dispatchprovider.hxx>
#include <loadenv/loadenv.hxx>
#include <dispatch/loaddispatcher.hxx>
#include <dispatch/closedispatcher.hxx>
#include <dispatch/startmoduledispatcher.hxx>

#include <pattern/window.hxx>
#include <targets.h>
#include "isstartmoduledispatch.hxx"

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XCacheInfo.hpp>

#include <rtl/ustring.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <framework/dispatchhelper.hxx>

namespace framework{

/**
    @short      standard ctor/dtor
    @descr      These initialize a new instance of this class with needed information for work.
                We hold a weak reference to our owner frame which starts dispatches to us.
                We can't use a normal reference because it holds a reference of us, too -
                nobody can die so ...!

    @seealso    using at owner

    @param      rxContext
                    reference to servicemanager to create new services.
    @param      xFrame
                    reference to our owner frame.
*/
DispatchProvider::DispatchProvider( css::uno::Reference< css::uno::XComponentContext >  xContext  ,
                                    const css::uno::Reference< css::frame::XFrame >&    xFrame    )
        : m_xContext    (std::move( xContext                     ))
        , m_xFrame      ( xFrame                        )
{
}

/**
    @short      protected(!) dtor for deinitializing
    @descr      We made it protected to prevent using us as a base class instead of as a member.
 */
DispatchProvider::~DispatchProvider()
{
}

/**
    @interface  XDispatchProvider
    @short      search a dispatcher for given URL
    @descr      If no interceptor is set on owner, we search for the correct frame and dispatch URL to it.
                If no frame was found, we do nothing.
                But we don't do it directly here. We detect the type of our owner frame and call
                a specialized queryDispatch() helper. Because a Desktop handles some
                requests in a different way than a normal frame.

    @param      aURL
                    URL to dispatch.
    @param      sTargetFrameName
                    name of searched frame.
    @param      nSearchFlags
                    flags for searching.
    @return     A reference to a dispatch object for this URL (if one was found!).

    @threadsafe yes
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL DispatchProvider::queryDispatch( const css::util::URL&  aURL             ,
                                                                                       const OUString& sTargetFrameName ,
                                                                                             sal_Int32        nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    css::uno::Reference< css::frame::XFrame > xOwner(m_xFrame);

    css::uno::Reference< css::frame::XDesktop > xDesktopCheck( xOwner, css::uno::UNO_QUERY );

    if (xDesktopCheck.is())
        xDispatcher = implts_queryDesktopDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);
    else
        xDispatcher = implts_queryFrameDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);

    return xDispatcher;
}

/**
    @interface  XDispatchProvider
    @short      do the same as queryDispatch(), but handle multiple dispatches at the same time
    @descr      It's optimistic. The user gives us a list of queries and we return a list of dispatchers.
                If one of the given queries couldn't be solved to an actually existing dispatcher,
                we return an empty reference for that entry. Order of both lists will be retained!

    @seealso    method queryDispatch()

    @param      lDescriptions
                    a list of all dispatch parameters for multiple requests
    @return     A reference to a list of dispatch objects for these URLs - may have some <NULL/> values inside.

    @threadsafe yes
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL DispatchProvider::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions )
{
    // Create return list - which must have the same size as the given descriptor.
    // It's not allowed to pack it!
    sal_Int32                                                          nCount     = lDescriptions.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    auto lDispatcherRange = asNonConstRange(lDispatcher);
    // Step over all descriptors and try to get a dispatcher for it.
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcherRange[i] = queryDispatch( lDescriptions[i].FeatureURL  ,
                                        lDescriptions[i].FrameName   ,
                                        lDescriptions[i].SearchFlags );
    }

    return lDispatcher;
}

/**
    @short      helper for queryDispatch()
    @descr      Every member of the frame tree (frame, desktop) may handle such a request
                in a different way. So we implement different specialized methods for everyone.

    @threadsafe yes
 */
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryDesktopDispatch( const css::uno::Reference< css::frame::XFrame >& xDesktop         ,
                                                                                            const css::util::URL&                           aURL             ,
                                                                                            const OUString&                          sTargetFrameName ,
                                                                                                  sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    // ignore wrong requests which are not supported
    if (
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    // We have no parent by definition
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         // Beamer frames are allowed as children of tasks only -
                                                            // and they exist more than once. We have no idea which of our sub tasks is the right one
       )
    {
        return nullptr;
    }

    // I) First handle special cases which are not suited for using findFrame()

    // I.I) "_blank"
    //  It's not the right place to create a new task here - because we are queried for a dispatch object
    //  only, which can handle such request. Such a dispatcher should create the required task on demand.
    //  Normally the functionality for "_blank" is provided by findFrame(), but that would create it directly
    //  here. That's why we must "intercept" here.

    if (sTargetFrameName==SPECIALTARGET_BLANK)
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_BLANKDISPATCHER, xDesktop );
    }

    // I.II) "_default"
    //  This is a combination of searching an empty task for recycling or creating a new one.

    else if (sTargetFrameName==SPECIALTARGET_DEFAULT)
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_DEFAULTDISPATCHER, xDesktop );

        if (isStartModuleDispatch(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_STARTMODULEDISPATCHER, xDesktop );
    }

    // I.III) "_self", "", "_top"
    //  The desktop can't load any document, but it can handle some special protocols like "uno", "slot" ...
    //  Why is "top" handled here as well? Because the desktop is the topmost frame. Normally it's superfluous
    //  to use this target, but we can handle it in the same manner as "_self".

    else if (
             (sTargetFrameName==SPECIALTARGET_SELF)  ||
             (sTargetFrameName==SPECIALTARGET_TOP )  ||
             (sTargetFrameName.isEmpty())
            )
    {
        xDispatcher = implts_searchProtocolHandler(aURL);
    }

    // I.IV) no further special targets exist
    //  Now we have to search for the right target frame by calling findFrame(), but should provide our code
    //  against the creation of a new task if no frame could be found.
    //  As mentioned earlier, it's allowed for dispatch() only.

    else
    {
        sal_Int32 nRightFlags  = nSearchFlags & ~css::frame::FrameSearchFlag::CREATE;

        // try to find any existing target and ask it for its dispatcher
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xDesktop->findFrame(sTargetFrameName, nRightFlags);
        if (xFoundFrame.is())
        {
            css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
            xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        // If it couldn't be found, but creation was allowed,
        // use a special dispatcher for creation or forwarding to the browser
        else if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
            xDispatcher = implts_getOrCreateDispatchHelper( E_CREATEDISPATCHER, xDesktop, sTargetFrameName, nSearchFlags );
    }

    return xDispatcher;
}

css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryFrameDispatch( const css::uno::Reference< css::frame::XFrame >& xFrame           ,
                                                                                          const css::util::URL&                           aURL             ,
                                                                                          const OUString&                          sTargetFrameName ,
                                                                                                sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    // 0) Some URLs are dispatched in a generic way (e.g. by the menu) using the default target "".
    //    But they are specified to use their own fixed target. Detect such URLs here and use the correct target.

    // I) First handle special cases which are not suited for using findFrame()

    // I.I) "_blank", "_default"
    //  It's not the right place to create a new task here. Only the desktop can do that.
    //  Normally the functionality for "_blank" is provided by findFrame(), but that would create it directly
    //  here. That's why we must "intercept" here.

    if (
        (sTargetFrameName==SPECIALTARGET_BLANK  ) ||
        (sTargetFrameName==SPECIALTARGET_DEFAULT)
       )
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            xDispatcher = xParent->queryDispatch(aURL, sTargetFrameName, 0); // it's a special target - ignore search flags
    }

    // I.II) "_beamer"
    //  Special sub frame of a top frame only. Search or create it. It's currently a little bit hacky.
    //  Only the sfx (meaning the controller) can create it.

    else if (sTargetFrameName==SPECIALTARGET_BEAMER)
    {
        css::uno::Reference< css::frame::XDispatchProvider > xBeamer( xFrame->findFrame( SPECIALTARGET_BEAMER, css::frame::FrameSearchFlag::CHILDREN | css::frame::FrameSearchFlag::SELF ), css::uno::UNO_QUERY );
        if (xBeamer.is())
        {
            xDispatcher = xBeamer->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (xController.is())
                // Force using of a special target, but use original search flags.
                // The caller may or may not use the CREATE flag!
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_BEAMER, nSearchFlags);
        }
    }

    // I.IV) "_parent"
    //  Our parent frame (if it exists) should handle this URL.

    else if (sTargetFrameName==SPECIALTARGET_PARENT)
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            // SELF => we must address the parent directly, and not its parent or any other parent!
            xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    }

    // I.V) "_top"
    //  This request must be forwarded to any parent frame, until we reach a top frame.
    //  If no parent exists, the frame itself can be handled.

    else if (sTargetFrameName==SPECIALTARGET_TOP)
    {
        if (xFrame->isTop())
        {
            // If we are this top frame itself (meaning our owner frame),
            // we should call ourselves recursively with a better target "_self".
            // So we can share the same code! (see reaction for "_self" inside this method too.)
            xDispatcher = queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            // Normally if isTop() returned sal_False, the parent frame MUST(!) exist,
            // but it seems to be better to check that here to avoid an access violation.
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_TOP, 0);
        }
    }

    // I.VI) "_self", ""
    //  Our owner frame should handle this URL. But we can't do it for all of them.
    //  So we ask the internal set controller first. If it disagrees we try to find a registered
    //  protocol handler. If this failed as well, we check for loadable content and in case of true
    //  we load it into the frame by returning a specialized dispatch object.

    else if (
             (sTargetFrameName==SPECIALTARGET_SELF)  ||
             (sTargetFrameName.isEmpty())
            )
    {
        // There is a hardcoded interception for special URLs.
        if ( aURL.Complete == ".uno:CloseDoc" || aURL.Complete == ".uno:CloseWin" )
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            // In case the frame is not a top one, is not based on system window and has a parent,
            // the parent frame should be queried for the correct dispatcher.
            // See i93473
            if (
                !WindowHelper::isTopWindow(xFrame->getContainerWindow()) &&
                !VCLUnoHelper::GetWindow(xFrame->getContainerWindow())->IsSystemWindow() &&
                xParent.is()
               )
                xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
            else
                xDispatcher = implts_getOrCreateDispatchHelper( E_CLOSEDISPATCHER, xFrame );
        }
        else if ( aURL.Complete == ".uno:CloseFrame" )
            xDispatcher = implts_getOrCreateDispatchHelper( E_CLOSEDISPATCHER, xFrame );

        if ( ! xDispatcher.is())
        {
            // Ask our controller for its opinion on these dispatched URLs,
            // because some URLs are internal and can be handled faster by SFX - which usually is the current controller!
            // But in case of e.g. the bibliography not all queries will be handled successfully here.
            css::uno::Reference< css::frame::XDispatchProvider > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (xController.is())
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
        }

        // If the controller doesn't feel like dispatching these URLs, we must find an appropriate dispatcher.
        // Search for any registered protocol handlers first.
        if (!xDispatcher.is())
            xDispatcher = implts_searchProtocolHandler(aURL);

        // Not for controller - not for protocol handler.
        // It should be a loadable content, maybe a file. Check it.
        // This check is necessary for finding out, if
        // support for some protocols isn't installed by the user.
        // If a protocol is not available, we suppress the creation of our dispatcher.
        // The result will be clear. It can't handle it, but will try.
        if (
            ( ! xDispatcher.is()             )  &&
            ( implts_isLoadableContent(aURL) )
           )
        {
            xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
        }
    }

    // I.VII) No further special handlings exist.
    //  Now we have to search for the right target frame by calling findFrame(), but should provide our code
    //  against the creation of a new task if no frame could be found.
    //  As mentioned earlier, it's allowed for dispatch() only.

    else
    {
        sal_Int32 nRightFlags  = nSearchFlags & ~css::frame::FrameSearchFlag::CREATE;

        // try to find any existing target and ask it for its dispatcher
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xFrame->findFrame(sTargetFrameName, nRightFlags);
        if (xFoundFrame.is())
        {
            // Attention: Found target is our own owner frame!
            // Don't ask it for its dispatcher. We know it already - it's our dispatch helper.
            // Otherwise we can start a neverending recursive call. Why?
            // Our owner frame was called somewhere, it called some interceptor objects and maybe this dispatch provider
            // is called. If we use queryDispatch() on our owner frame again, we start this call stack again and again.
            if (xFoundFrame==xFrame)
                xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
            else
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
            }
        }
        else
        // If it couldn't be found, but creation was allowed
        // forward request to the desktop.
        // Note: The given target name must be used to set the name on a new created task!
        //       Don't forward the request by changing it to a special one e.g _blank.
        //       Use the CREATE flag only to prevent call against further searches.
        //       We already know it: the target must be created new.
        if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, sTargetFrameName, css::frame::FrameSearchFlag::CREATE);
        }
    }

    return xDispatcher;
}

/**
    @short      search for a registered protocol handler and ask it for a dispatch object
    @descr      We search a suitable handler inside our cfg package org.openoffice.Office.ProtocolHandler.
                If we found any, we create and initialize it. Initializing means: we set our owner frame on it
                as context information. It can use it or leave it. Of course - we are aware of handler implementations, which don't support initialization. It's an optional feature.

    @param      aURL
                    the dispatch URL for which a handler may be registered

    @return     A dispatch object if a handler was found and agrees with the given URL or <NULL/> otherwise.

    @threadsafe yes
*/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_searchProtocolHandler( const css::util::URL& aURL )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    ProtocolHandler                              aHandler;

    // This member itself is threadsafe and lives if we live - we don't need any mutex here.
    if (framework::HandlerCache::search(aURL,&aHandler))
    {
        css::uno::Reference< css::frame::XDispatchProvider > xHandler;
        {
            SolarMutexGuard g;

            // create it
            bool bInitialize = true;
            try
            {
                // Only create the protocol handler instance once, the creation is expensive.
                auto it = m_aProtocolHandlers.find(aHandler.m_sUNOName);
                if (it == m_aProtocolHandlers.end())
                {
                    xHandler.set(
                        css::uno::Reference<css::lang::XMultiServiceFactory>(m_xContext->getServiceManager(), css::uno::UNO_QUERY_THROW)
                          ->createInstance(aHandler.m_sUNOName),
                        css::uno::UNO_QUERY);

                    // Check if the handler explicitly requested to avoid caching.
                    css::uno::Reference<css::util::XCacheInfo> xCacheInfo(xHandler, css::uno::UNO_QUERY);
                    if (!xCacheInfo.is() || xCacheInfo->isCachingAllowed())
                    {
                        m_aProtocolHandlers.emplace(aHandler.m_sUNOName, xHandler);
                    }
                }
                else
                {
                    xHandler = it->second;
                    bInitialize = false;
                }
            }
            catch(const css::uno::Exception&) {}

            // check if initialization is necessary
            css::uno::Reference< css::lang::XInitialization > xInit( xHandler, css::uno::UNO_QUERY );
            if (xInit.is() && bInitialize)
            {
                css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
                SAL_WARN_IF(!xOwner.is(), "fwk", "DispatchProvider::implts_searchProtocolHandler(): Couldn't get reference to my owner frame. So I can't set may needed context information for this protocol handler.");
                if (xOwner.is())
                {
                    try
                    {
                        // but do it only, if all context information is OK
                        css::uno::Sequence< css::uno::Any > lContext{ css::uno::Any(xOwner) };
                        xInit->initialize(lContext);
                    }
                    catch(const css::uno::Exception&) {}
                }
            }
        }

        // ask for its (sub)dispatcher for the given URL
        if (xHandler.is())
            xDispatcher = xHandler->queryDispatch(aURL,SPECIALTARGET_SELF,0);
    }

    return xDispatcher;
}

/**
    @short      get or create a new dispatch helper
    @descr      Sometimes we need some helper implementations to support dispatching of special URLs or commands.
                But it's not a good idea to hold these services for the whole life time of this provider instance.
                We should create it on demand.
                That's why we implement this method. It returns an already existing helper or creates a new one otherwise.

    @attention  The parameters sTarget and nSearchFlags are defaulted to "" and 0!
                Mostly it depends on the parameter eHelper, if they are required or not.

    @param      eHelper
                    specify the requested dispatch helper
    @param      xOwner
                    the target of possible dispatch() call on created dispatch helper
    @param      sTarget
                    the target parameter of the original queryDispatch() request
    @param      nSearchFlags
                    the flags parameter of the original queryDispatch() request
    @return     A reference to a dispatch helper.

    @threadsafe yes
*/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_getOrCreateDispatchHelper( EDispatchHelper                                  eHelper     ,
                                                                                                 const css::uno::Reference< css::frame::XFrame >& xOwner      ,
                                                                                                 const OUString&                           sTarget     ,
                                                                                                 sal_Int32                                  nSearchFlags)
{
    css::uno::Reference< css::frame::XDispatch > xDispatchHelper;

    switch (eHelper)
    {
        case E_CREATEDISPATCHER :
                xDispatchHelper = new LoadDispatcher(m_xContext, xOwner, sTarget, nSearchFlags);
                break;

        case E_BLANKDISPATCHER :
                {
                    if (xOwner.is())
                        xDispatchHelper = new LoadDispatcher(m_xContext, xOwner, SPECIALTARGET_BLANK, 0);
                }
                break;

        case E_DEFAULTDISPATCHER :
                {
                    if (xOwner.is())
                        xDispatchHelper = new LoadDispatcher(m_xContext, xOwner, SPECIALTARGET_DEFAULT, 0);
                }
                break;

        case E_SELFDISPATCHER :
                xDispatchHelper = new LoadDispatcher(m_xContext, xOwner, SPECIALTARGET_SELF, 0);
                break;

        case E_CLOSEDISPATCHER :
                xDispatchHelper = new CloseDispatcher( m_xContext, xOwner, sTarget );
                break;

        case E_STARTMODULEDISPATCHER :
                xDispatchHelper = new StartModuleDispatcher( m_xContext );
                break;
    }

    return xDispatchHelper;
}

/**
    @short      check URL for support by our used loader or handler
    @descr      If we must return our own dispatch helper implementations (self, blank, create dispatcher!)
                we should be sure, that the URL describes any loadable content. Otherwise slot/uno URLs
                will be detected, but there exists nothing for real loading into a target frame!

    @param      aURL
                    URL which should be "detected"
    @return     <TRUE/> if it could be handled somewhere - <FALSE/> otherwise.

    @threadsafe yes
*/
// static
bool DispatchProvider::implts_isLoadableContent( const css::util::URL& aURL )
{
    LoadEnv::EContentType eType = LoadEnv::classifyContent(aURL.Complete, css::uno::Sequence< css::beans::PropertyValue >());
    return ( eType == LoadEnv::E_CAN_BE_LOADED );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
