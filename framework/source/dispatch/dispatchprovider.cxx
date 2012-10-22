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

#include <stdio.h>
#include <dispatch/dispatchprovider.hxx>
#include <loadenv/loadenv.hxx>
#include <dispatch/loaddispatcher.hxx>
#include <dispatch/closedispatcher.hxx>
#include <dispatch/menudispatcher.hxx>
#include <dispatch/startmoduledispatcher.hxx>

#include <pattern/window.hxx>
#include <threadhelp/transactionguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <dispatchcommands.h>
#include <protocols.h>
#include <services.h>
#include <targets.h>
#include <general.h>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <osl/diagnose.h>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

namespace framework{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_2( DispatchProvider                               ,
                     OWeakObject                                    ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider     ),
                     DIRECT_INTERFACE(css::frame::XDispatchProvider)
                   )

DEFINE_XTYPEPROVIDER_2( DispatchProvider             ,
                        css::lang::XTypeProvider     ,
                        css::frame::XDispatchProvider
                      )

/**
    @short      standard ctor/dtor
    @descr      These initialize a new instance of tihs class with needed informations for work.
                We hold a weakreference to our owner frame which start dispatches at us.
                We can't use a normal reference because he hold a reference of us too ...
                nobody can die so ...!

    @seealso    using at owner

    @param      xFactory
                    reference to servicemanager to create new services.
    @param      xFrame
                    reference to our owner frame.
*/
DispatchProvider::DispatchProvider( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory  ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame    )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xFactory    ( xFactory                      )
        , m_xFrame      ( xFrame                        )
{
}

/**
    @short      protected(!) dtor for deinitializing
    @descr      We made it protected to prevent using of us as base class instead as a member.
 */
DispatchProvider::~DispatchProvider()
{
}

/**
    @interface  XDispatchProvider
    @short      search a dispatcher for given URL
    @descr      If no interceptor is set on owner, we search for right frame and dispatch URL to it.
                If no frame was found, we do nothing.
                But we doesn't do it directly here. We detect the type of our owner frame and calls
                specialized queryDispatch() helper dependen from that. Because a Desktop handle some
                requests in another way then a normal frame.

    @param      aURL
                    URL to dispatch.
    @param      sTargetFrameName
                    name of searched frame.
    @param      nSearchFlags
                    flags for searching.
    @return     A reference to a dispatch object for this URL (if someone was found!).

    @threadsafe yes
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL DispatchProvider::queryDispatch( const css::util::URL&  aURL             ,
                                                                                       const ::rtl::OUString& sTargetFrameName ,
                                                                                             sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDesktop > xDesktopCheck( xOwner, css::uno::UNO_QUERY );

    if (xDesktopCheck.is())
        xDispatcher = implts_queryDesktopDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);
    else
        xDispatcher = implts_queryFrameDispatch(xOwner, aURL, sTargetFrameName, nSearchFlags);

    return xDispatcher;
}

/**
    @interface  XDispatchProvider
    @short      do the same like queryDispatch() ... but handle multiple dispatches at the same time
    @descr      It's an optimism. User give us a list of queries ... and we return a list of dispatcher.
                If one of given queries couldn't be solved to a real existing dispatcher ...
                we return a list with empty references in it! Order of both lists will be retained!

    @seealso    method queryDispatch()

    @param      lDescriptions
                    a list of all dispatch parameters for multiple requests
    @return     A reference a list of dispatch objects for these URLs - may with some <NULL/> values inside.

    @threadsafe yes
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL DispatchProvider::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions ) throw( css::uno::RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    sal_Int32                                                          nCount     = lDescriptions.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );

    // Step over all descriptors and try to get any dispatcher for it.
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( lDescriptions[i].FeatureURL  ,
                                        lDescriptions[i].FrameName   ,
                                        lDescriptions[i].SearchFlags );
    }

    return lDispatcher;
}

::sal_Bool lcl_isStartModuleDispatch (const css::util::URL& aURL)
{
    return aURL.Complete == CMD_UNO_SHOWSTARTMODULE;
}

/**
    @short      helper for queryDispatch()
    @descr      Every member of the frame tree (frame, desktop) must handle such request
                in another way. So we implement different specialized metods for every one.

    @threadsafe yes
 */
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryDesktopDispatch( const css::uno::Reference< css::frame::XFrame > xDesktop         ,
                                                                                            const css::util::URL&                           aURL             ,
                                                                                            const ::rtl::OUString&                          sTargetFrameName ,
                                                                                                  sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    // ignore wrong requests which are not supported
    if (
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )   ||    // valid for frame dispatches - not for desktop
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    // we have no parent by definition
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         // beamer frames are allowed as child of tasks only -
                                                            // and they exist more then ones. We have no idea which our sub tasks is the right one
       )
    {
        return NULL;
    }

    //-----------------------------------------------------------------------------------------------------
    // I) handle special cases which not right for using findFrame() first
    //-----------------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank"
    //  It's not the right place to create a new task here - because we are queried for a dispatch object
    //  only, which can handle such request. Such dispatcher should create the required task on demand.
    //  Normaly the functionality for "_blank" is provided by findFrame() - but that would create it directly
    //  here. Thats why we must "intercept" here.
    //-----------------------------------------------------------------------------------------------------
    if (sTargetFrameName==SPECIALTARGET_BLANK)
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_BLANKDISPATCHER, xDesktop );
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) "_default"
    //  This is a combination of search an empty task for recycling - or create a new one.
    //-----------------------------------------------------------------------------------------------------
    else
    if (sTargetFrameName==SPECIALTARGET_DEFAULT)
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_DEFAULTDISPATCHER, xDesktop );

        if (lcl_isStartModuleDispatch(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_STARTMODULEDISPATCHER, xDesktop );
    }

    //-----------------------------------------------------------------------------------------------------
    // I.III) "_self", "", "_top"
    //  The desktop can't load any document - but he can handle some special protocols like "uno", "slot" ...
    //  Why is "top" here handled too? Because the desktop is the topest frame. Normaly it's superflous
    //  to use this target - but we can handle it in the same manner then "_self".
    //-----------------------------------------------------------------------------------------------------
    else if (
             (sTargetFrameName==SPECIALTARGET_SELF)  ||
             (sTargetFrameName==SPECIALTARGET_TOP )  ||
             (sTargetFrameName.isEmpty())
            )
    {
        xDispatcher = implts_searchProtocolHandler(aURL);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.IV) no further special targets exist
    //  Now we have to search for the right target frame by calling findFrame() - but should provide our code
    //  against creation of a new task if no frame could be found.
    //  I said it b efore - it's allowed for dispatch() only.
    //-----------------------------------------------------------------------------------------------------
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        // try to find any existing target and ask him for his dispatcher
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xDesktop->findFrame(sTargetFrameName, nRightFlags);
        if (xFoundFrame.is())
        {
            css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
            xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        // if it couldn't be found - but creation was allowed
        // use special dispatcher for creatio or froward it to the browser
        else if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
            xDispatcher = implts_getOrCreateDispatchHelper( E_CREATEDISPATCHER, xDesktop, sTargetFrameName, nSearchFlags );
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryFrameDispatch( const css::uno::Reference< css::frame::XFrame > xFrame           ,
                                                                                          const css::util::URL&                           aURL             ,
                                                                                          const ::rtl::OUString&                          sTargetFrameName ,
                                                                                                sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    //-----------------------------------------------------------------------------------------------------
    // 0) Some URLs are dispatched in a generic way (e.g. by the menu) using the default target "".
    //    But they are specified to use her own fix target. Detect such URLs here and use the correct target.
    //-----------------------------------------------------------------------------------------------------

    ::rtl::OUString sTargetName = sTargetFrameName;

    //-----------------------------------------------------------------------------------------------------
    // I) handle special cases which not right for using findFrame() first
    //-----------------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank", "_default"
    //  It's not the right place to create a new task here. Only the desktop can do that.
    //  Normaly the functionality for "_blank" is provided by findFrame() - but that would create it directly
    //  here. Thats why we must "intercept" here.
    //-----------------------------------------------------------------------------------------------------
    if (
        (sTargetName==SPECIALTARGET_BLANK  ) ||
        (sTargetName==SPECIALTARGET_DEFAULT)
       )
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            xDispatcher = xParent->queryDispatch(aURL, sTargetName, 0); // its a special target - ignore search flags
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) "_menubar"
    //  Special mode on frame or task to receive the local menu. Not supported by findFrame()
    //-----------------------------------------------------------------------------------------------------
    else if (sTargetName==SPECIALTARGET_MENUBAR)
    {
        xDispatcher = implts_getOrCreateDispatchHelper( E_MENUDISPATCHER, xFrame );
    }

    //-----------------------------------------------------------------------------------------------------
    // I.IV) "_helpagent"
    //  Special sub frame of a top frame only. Search or create it. ... OK it's currently a little bit HACKI.
    //  Only the sfx (means the controller) can create it it.
    //-----------------------------------------------------------------------------------------------------
    else if (sTargetName==SPECIALTARGET_BEAMER)
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
                // force using of special target - but use original search flags
                // May the caller used the CREATE flag or not!
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_BEAMER, nSearchFlags);
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.V) "_parent"
    //  Our parent frame (if it exist) should handle this URL.
    //-----------------------------------------------------------------------------------------------------
    else if (sTargetName==SPECIALTARGET_PARENT)
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            // SELF => we must address the parent directly... and not his parent or any other parent!
            xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.VI) "_top"
    //  This request must be forwarded to any parent frame, till we reach a top frame.
    //  If no parent exist, we can handle itself.
    //-----------------------------------------------------------------------------------------------------
    else if (sTargetName==SPECIALTARGET_TOP)
    {
        if (xFrame->isTop())
        {
            // If we are this top frame itself (means our owner frame)
            // we should call ourself recursiv with a better target "_self".
            // So we can share the same code! (see reaction for "_self" inside this method too.)
            xDispatcher = this->queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            // Normaly if isTop() returned sal_False ... the parent frame MUST(!) exist ...
            // But it seams to be better to check that here to prevent us against an access violation.
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_TOP, 0);
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.VII) "_self", ""
    //  Our owner frame should handle this URL. But we can't do it for all of them.
    //  So we ask the internal setted controller first. If he disagree we try to find a registered
    //  protocol handler. If this failed too - we check for a loadable content and in case of true
    //  we load it into the frame by returning specilized dispatch object.
    //-----------------------------------------------------------------------------------------------------
    else if (
             (sTargetName==SPECIALTARGET_SELF)  ||
             (sTargetName.isEmpty())
            )
    {
        // There exist a hard coded interception for special URLs.
        if ( aURL.Complete == ".uno:CloseDoc" || aURL.Complete == ".uno:CloseWin" )
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            // In case the frame is not a top one, is not based on system window and has a parent,
            // the parent frame should to be queried for the correct dispatcher.
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
            // Ask our controller for his agreement for these dispatched URL ...
            // because some URLs are internal and can be handled faster by SFX - which most is the current controller!
            // But in case of e.g. the bibliography not all queries will be handled successfully here.
            css::uno::Reference< css::frame::XDispatchProvider > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (xController.is())
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
        }

        // If controller has no fun to dispatch these URL - we must search another right dispatcher.
        // Search for any registered protocol handler first.
        if (!xDispatcher.is())
            xDispatcher = implts_searchProtocolHandler(aURL);

        // Not for controller - not for protocol handler
        // It should be a loadable content - may be a file. Check it ...
        // This check is neccessary to found out, that
        // support for some protocols isn't installed by user. May be
        // "ftp" isn't available. So we suppress creation of our self dispatcher.
        // The result will be clear. He can't handle it - but he would try it.
        if (
            ( ! xDispatcher.is()             )  &&
            ( implts_isLoadableContent(aURL) )
           )
        {
            xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
        }
    }

    //-----------------------------------------------------------------------------------------------------
    // I.VI) no further special handlings exist
    //  Now we have to search for the right target frame by calling findFrame() - but should provide our code
    //  against creation of a new task if no frame could be found.
    //  I said it before - it's allowed for dispatch() only.
    //-----------------------------------------------------------------------------------------------------
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        // try to find any existing target and ask him for his dispatcher
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xFrame->findFrame(sTargetName, nRightFlags);
        if (xFoundFrame.is())
        {
            // Attention: Found target is our own owner frame!
            // Don't ask him for his dispatcher. We know it already - it's our self dispatch helper.
            // Otherwhise we can start a never ending recursiv call. Why?
            // Somewere called our owner frame - he called some interceptor objects - and may by this dispatch provider
            // is called. If wa use queryDispatch() on our owner frame again - we start this call stack again ... and again.
            if (xFoundFrame==xFrame)
                xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
            else
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
            }
        }
        else
        // if it couldn't be found - but creation was allowed
        // forward request to the desktop.
        // Note: The given target name must be used to set the name on new created task!
        //       Don't forward request by changing it to a special one e.g _blank.
        //       Use the CREATE flag only to prevent call against further searches.
        //       We already know it - the target must be created new.
        if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, sTargetName, css::frame::FrameSearchFlag::CREATE);
        }
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      search for a registered protocol handler and ask him for a dispatch object
    @descr      Wes earch a suitable handler inside our cfg package org.openoffice.Office.ProtocolHandler.
                If we found anyone, we create and initialize it. Initialize means: we set our owner frame on it
                as context information. He can use it or leave it. Of course - we are aware of handler implementations,
                which doesn't support initialization. It's an optional feature.

    @param      aURL
                    the dispatch URL for which may a handler is registered

    @return     A dispatch object if a handler was found and agree with the given URL or <NULL/> otherwhise.

    @threadsafe yes
*/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_searchProtocolHandler( const css::util::URL& aURL )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    ProtocolHandler                              aHandler   ;

    // This member is threadsafe by himself and lives if we live - we doesn't need any mutex here.
    if (m_aProtocolHandlerCache.search(aURL,&aHandler))
    {
        /* SAFE { */
        ReadGuard aReadLock( m_aLock );

        // create it
        css::uno::Reference< css::frame::XDispatchProvider > xHandler;
        try
        {
            xHandler = css::uno::Reference< css::frame::XDispatchProvider >(
                            m_xFactory->createInstance(aHandler.m_sUNOName),
                            css::uno::UNO_QUERY);
        }
        catch(const css::uno::Exception&) {}

        // look if initialization is neccessary
        css::uno::Reference< css::lang::XInitialization > xInit( xHandler, css::uno::UNO_QUERY );
        if (xInit.is())
        {
            css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
            LOG_ASSERT(xOwner.is(), "DispatchProvider::implts_searchProtocolHandler()\nCouldn't get reference to my owner frame. So I can't set may needed context information for this protocol handler.")
            if (xOwner.is())
            {
                try
                {
                    // but do it only, if all context informations are OK
                    css::uno::Sequence< css::uno::Any > lContext(1);
                    lContext[0] <<= xOwner;
                    xInit->initialize(lContext);
                }
                catch(const css::uno::Exception&) {}
            }
        }

        aReadLock.unlock();
        /* } SAFE */

        // ask for his (sub)dispatcher for the given URL
        if (xHandler.is())
            xDispatcher = xHandler->queryDispatch(aURL,SPECIALTARGET_SELF,0);
    }

    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      get or create new dispatch helper
    @descr      Sometimes we need some helper implementations to support dispatching of special URLs or commands.
                But it's not a good idea to hold these services for the whole life time of this provider instance.
                We should create it on demand ...
                Thats why we implement this method. It return an already existing helper or create a new one otherwise.

    @attention  The parameter sTarget and nSearchFlags are defaulted to "" and 0!
                Please use it only, if you can be shure, that the realy given by the outside calli!
                Mostly it depends from the parameter eHelper is they are required or not.

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
                                                                                                 const ::rtl::OUString&                           sTarget     ,
                                                                                                       sal_Int32                                  nSearchFlags)
{
    css::uno::Reference< css::frame::XDispatch > xDispatchHelper;

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    /* } SAFE */

    switch (eHelper)
    {
        case E_MENUDISPATCHER :
                {
                    // Attention: Such menue dispatcher must be a singleton for this frame - means our owner frame.
                    // Otherwhise he can make some trouble.
                    /* SAFE { */
                    WriteGuard aWriteLock( m_aLock );
                    if ( ! m_xMenuDispatcher.is() )
                    {
                        MenuDispatcher* pDispatcher = new MenuDispatcher( xFactory, xOwner );
                        m_xMenuDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                    xDispatchHelper = m_xMenuDispatcher;
                    aWriteLock.unlock();
                    /* } SAFE */
                }
                break;

        case E_CREATEDISPATCHER :
                {
                    LoadDispatcher* pDispatcher = new LoadDispatcher(xFactory, xOwner, sTarget, nSearchFlags);
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_BLANKDISPATCHER :
                {
                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                    if (xDesktop.is())
                    {
                        LoadDispatcher* pDispatcher = new LoadDispatcher(xFactory, xOwner, SPECIALTARGET_BLANK, 0);
                        xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                }
                break;

        case E_DEFAULTDISPATCHER :
                {
                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                    if (xDesktop.is())
                    {
                        LoadDispatcher* pDispatcher = new LoadDispatcher(xFactory, xOwner, SPECIALTARGET_DEFAULT, 0);
                        xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                }
                break;

        case E_SELFDISPATCHER :
                {
                    LoadDispatcher* pDispatcher = new LoadDispatcher(xFactory, xOwner, SPECIALTARGET_SELF, 0);
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_CLOSEDISPATCHER :
                {
                    CloseDispatcher* pDispatcher = new CloseDispatcher( xFactory, xOwner, sTarget );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_STARTMODULEDISPATCHER :
                {
                    StartModuleDispatcher* pDispatcher = new StartModuleDispatcher( xFactory, xOwner, sTarget );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;
    }

    return xDispatchHelper;
}

//_________________________________________________________________________________________________________________

/**
    @short      check URL for support by our used loader or handler
    @descr      If we must return our own dispatch helper implementations (self, blank, create dispatcher!)
                we should be shure, that URL describe any loadable content. Otherwise slot/uno URLs
                will be detected ... but there exist nothing for ral loading into a target frame!

    @param      aURL
                    URL which should be "detected"
    @return     <TRUE/> if somewhere could handle that - <FALSE/> otherwise.

    @threadsafe yes
*/
sal_Bool DispatchProvider::implts_isLoadableContent( const css::util::URL& aURL )
{
    LoadEnv::EContentType eType = LoadEnv::classifyContent(aURL.Complete, css::uno::Sequence< css::beans::PropertyValue >());
    return ( eType == LoadEnv::E_CAN_BE_LOADED );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
