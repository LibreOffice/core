/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

/**
    @short      standard ctor/dtor
    @descr      These initialize a new instance of this class with needed information for work.
                We hold a weakreference to our owner frame which start dispatches at us.
                We can't use a normal reference because he hold a reference of us too ...
                nobody can die so ...!

    @seealso    using at owner

    @param      rxContext
                    reference to servicemanager to create new services.
    @param      xFrame
                    reference to our owner frame.
*/
DispatchProvider::DispatchProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext  ,
                                    const css::uno::Reference< css::frame::XFrame >&              xFrame    )
        
        : ThreadHelpBase( &Application::GetSolarMutex() )
        
        , m_xContext    ( rxContext                     )
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
                But we don't do it directly here. We detect the type of our owner frame and calls
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
                                                                                       const OUString& sTargetFrameName ,
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
    
    
    sal_Int32                                                          nCount     = lDescriptions.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );

    
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
                                                                                            const OUString&                          sTargetFrameName ,
                                                                                                  sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    
    if (
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )   ||    
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         
                                                            
       )
    {
        return NULL;
    }

    
    
    

    
    
    
    
    
    
    
    if (sTargetFrameName==SPECIALTARGET_BLANK)
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_BLANKDISPATCHER, xDesktop );
    }

    
    
    
    
    else if (sTargetFrameName==SPECIALTARGET_DEFAULT)
    {
        if (implts_isLoadableContent(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_DEFAULTDISPATCHER, xDesktop );

        if (lcl_isStartModuleDispatch(aURL))
            xDispatcher = implts_getOrCreateDispatchHelper( E_STARTMODULEDISPATCHER, xDesktop );
    }

    
    
    
    
    
    
    else if (
             (sTargetFrameName==SPECIALTARGET_SELF)  ||
             (sTargetFrameName==SPECIALTARGET_TOP )  ||
             (sTargetFrameName.isEmpty())
            )
    {
        xDispatcher = implts_searchProtocolHandler(aURL);
    }

    
    
    
    
    
    
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xDesktop->findFrame(sTargetFrameName, nRightFlags);
        if (xFoundFrame.is())
        {
            css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
            xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        
        
        else if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
            xDispatcher = implts_getOrCreateDispatchHelper( E_CREATEDISPATCHER, xDesktop, sTargetFrameName, nSearchFlags );
    }

    return xDispatcher;
}



css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_queryFrameDispatch( const css::uno::Reference< css::frame::XFrame > xFrame           ,
                                                                                          const css::util::URL&                           aURL             ,
                                                                                          const OUString&                          sTargetFrameName ,
                                                                                                sal_Int32                                 nSearchFlags     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;

    
    
    
    

    OUString sTargetName = sTargetFrameName;

    
    
    

    
    
    
    
    
    
    if (
        (sTargetName==SPECIALTARGET_BLANK  ) ||
        (sTargetName==SPECIALTARGET_DEFAULT)
       )
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            xDispatcher = xParent->queryDispatch(aURL, sTargetName, 0); 
    }

    
    
    
    
    else if (sTargetName==SPECIALTARGET_MENUBAR)
    {
        xDispatcher = implts_getOrCreateDispatchHelper( E_MENUDISPATCHER, xFrame );
    }

    
    
    
    
    
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
                
                
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_BEAMER, nSearchFlags);
        }
    }

    
    
    
    
    else if (sTargetName==SPECIALTARGET_PARENT)
    {
        css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
        if (xParent.is())
            
            xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    }

    
    
    
    
    
    else if (sTargetName==SPECIALTARGET_TOP)
    {
        if (xFrame->isTop())
        {
            
            
            
            xDispatcher = this->queryDispatch(aURL,SPECIALTARGET_SELF,0);
        }
        else
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            
            
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, SPECIALTARGET_TOP, 0);
        }
    }

    
    
    
    
    
    
    
    else if (
             (sTargetName==SPECIALTARGET_SELF)  ||
             (sTargetName.isEmpty())
            )
    {
        
        if ( aURL.Complete == ".uno:CloseDoc" || aURL.Complete == ".uno:CloseWin" )
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            
            
            
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
            
            
            
            css::uno::Reference< css::frame::XDispatchProvider > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (xController.is())
                xDispatcher = xController->queryDispatch(aURL, SPECIALTARGET_SELF, 0);
        }

        
        
        if (!xDispatcher.is())
            xDispatcher = implts_searchProtocolHandler(aURL);

        
        
        
        
        
        
        if (
            ( ! xDispatcher.is()             )  &&
            ( implts_isLoadableContent(aURL) )
           )
        {
            xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
        }
    }

    
    
    
    
    
    
    else
    {
        sal_Int32 nRightFlags  = nSearchFlags;
                  nRightFlags &= ~css::frame::FrameSearchFlag::CREATE;

        
        css::uno::Reference< css::frame::XFrame > xFoundFrame = xFrame->findFrame(sTargetName, nRightFlags);
        if (xFoundFrame.is())
        {
            
            
            
            
            
            if (xFoundFrame==xFrame)
                xDispatcher = implts_getOrCreateDispatchHelper( E_SELFDISPATCHER, xFrame );
            else
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider( xFoundFrame, css::uno::UNO_QUERY );
                xDispatcher = xProvider->queryDispatch(aURL,SPECIALTARGET_SELF,0);
            }
        }
        else
        
        
        
        
        
        
        if (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
        {
            css::uno::Reference< css::frame::XDispatchProvider > xParent( xFrame->getCreator(), css::uno::UNO_QUERY );
            if (xParent.is())
                xDispatcher = xParent->queryDispatch(aURL, sTargetName, css::frame::FrameSearchFlag::CREATE);
        }
    }

    return xDispatcher;
}



/**
    @short      search for a registered protocol handler and ask him for a dispatch object
    @descr      Wes earch a suitable handler inside our cfg package org.openoffice.Office.ProtocolHandler.
                If we found anyone, we create and initialize it. Initialize means: we set our owner frame on it
                as context information. He can use it or leave it. Of course - we are aware of handler implementations,
                which doesn't support initialization. It's an optional feature.

    @param      aURL
                    the dispatch URL for which may a handler is registered

    @return     A dispatch object if a handler was found and agree with the given URL or <NULL/> otherwise.

    @threadsafe yes
*/
css::uno::Reference< css::frame::XDispatch > DispatchProvider::implts_searchProtocolHandler( const css::util::URL& aURL )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    ProtocolHandler                              aHandler   ;

    
    if (m_aProtocolHandlerCache.search(aURL,&aHandler))
    {
        /* SAFE { */
        ReadGuard aReadLock( m_aLock );

        
        css::uno::Reference< css::frame::XDispatchProvider > xHandler;
        try
        {
            xHandler = css::uno::Reference< css::frame::XDispatchProvider >(
                            css::uno::Reference<css::lang::XMultiServiceFactory>(m_xContext->getServiceManager(), css::uno::UNO_QUERY_THROW)
                               ->createInstance(aHandler.m_sUNOName),
                            css::uno::UNO_QUERY);
        }
        catch(const css::uno::Exception&) {}

        
        css::uno::Reference< css::lang::XInitialization > xInit( xHandler, css::uno::UNO_QUERY );
        if (xInit.is())
        {
            css::uno::Reference< css::frame::XFrame > xOwner( m_xFrame.get(), css::uno::UNO_QUERY );
            SAL_WARN_IF(!xOwner.is(), "fwk", "DispatchProvider::implts_searchProtocolHandler(): Couldn't get reference to my owner frame. So I can't set may needed context information for this protocol handler.");
            if (xOwner.is())
            {
                try
                {
                    
                    css::uno::Sequence< css::uno::Any > lContext(1);
                    lContext[0] <<= xOwner;
                    xInit->initialize(lContext);
                }
                catch(const css::uno::Exception&) {}
            }
        }

        aReadLock.unlock();
        /* } SAFE */

        
        if (xHandler.is())
            xDispatcher = xHandler->queryDispatch(aURL,SPECIALTARGET_SELF,0);
    }

    return xDispatcher;
}



/**
    @short      get or create new dispatch helper
    @descr      Sometimes we need some helper implementations to support dispatching of special URLs or commands.
                But it's not a good idea to hold these services for the whole life time of this provider instance.
                We should create it on demand ...
                Thats why we implement this method. It return an already existing helper or create a new one otherwise.

    @attention  The parameter sTarget and nSearchFlags are defaulted to "" and 0!
                Please use it only, if you can be shure, that the really given by the outside calli!
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
                                                                                                 const OUString&                           sTarget     ,
                                                                                                       sal_Int32                                  nSearchFlags)
{
    css::uno::Reference< css::frame::XDispatch > xDispatchHelper;

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    /* } SAFE */

    switch (eHelper)
    {
        case E_MENUDISPATCHER :
                {
                    
                    
                    /* SAFE { */
                    WriteGuard aWriteLock( m_aLock );
                    if ( ! m_xMenuDispatcher.is() )
                    {
                        MenuDispatcher* pDispatcher = new MenuDispatcher( xContext, xOwner );
                        m_xMenuDispatcher = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                    xDispatchHelper = m_xMenuDispatcher;
                    aWriteLock.unlock();
                    /* } SAFE */
                }
                break;

        case E_CREATEDISPATCHER :
                {
                    LoadDispatcher* pDispatcher = new LoadDispatcher(xContext, xOwner, sTarget, nSearchFlags);
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_BLANKDISPATCHER :
                {
                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                    if (xDesktop.is())
                    {
                        LoadDispatcher* pDispatcher = new LoadDispatcher(xContext, xOwner, SPECIALTARGET_BLANK, 0);
                        xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                }
                break;

        case E_DEFAULTDISPATCHER :
                {
                    css::uno::Reference< css::frame::XFrame > xDesktop( xOwner, css::uno::UNO_QUERY );
                    if (xDesktop.is())
                    {
                        LoadDispatcher* pDispatcher = new LoadDispatcher(xContext, xOwner, SPECIALTARGET_DEFAULT, 0);
                        xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                    }
                }
                break;

        case E_SELFDISPATCHER :
                {
                    LoadDispatcher* pDispatcher = new LoadDispatcher(xContext, xOwner, SPECIALTARGET_SELF, 0);
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_CLOSEDISPATCHER :
                {
                    CloseDispatcher* pDispatcher = new CloseDispatcher( xContext, xOwner, sTarget );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;

        case E_STARTMODULEDISPATCHER :
                {
                    StartModuleDispatcher* pDispatcher = new StartModuleDispatcher( xContext, xOwner, sTarget );
                    xDispatchHelper = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(pDispatcher), css::uno::UNO_QUERY );
                }
                break;
    }

    return xDispatchHelper;
}



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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
