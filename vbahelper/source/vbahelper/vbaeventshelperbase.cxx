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

#include "vbahelper/vbaeventshelperbase.hxx"
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <filter/msfilter/msvbahelper.hxx>
#include <unotools/eventcfg.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;



VbaEventsHelperBase::VbaEventsHelperBase( const uno::Sequence< uno::Any >& rArgs, const uno::Reference< uno::XComponentContext >& /*xContext*/ ) :
    mpShell( 0 ),
    mbDisposed( true )
{
    try
    {
        mxModel = getXSomethingFromArgs< frame::XModel >( rArgs, 0, false );
        mpShell = getSfxObjShell( mxModel );
    }
    catch( uno::Exception& )
    {
    }
    mbDisposed = mpShell == 0;
    startListening();
}

VbaEventsHelperBase::~VbaEventsHelperBase()
{
    SAL_WARN_IF( !mbDisposed, "vbahelper", "VbaEventsHelperBase::~VbaEventsHelperBase - missing disposing notification" );
}

sal_Bool SAL_CALL VbaEventsHelperBase::processVbaEvent( sal_Int32 nEventId, const uno::Sequence< uno::Any >& rArgs )
        throw (lang::IllegalArgumentException, util::VetoException, uno::RuntimeException)
{
    /*  Derived classes may add new event identifiers to be processed while
        processing the original event. All unprocessed events are collected in
        a queue. First element in the queue is the next event to be processed. */
    EventQueue aEventQueue;
    aEventQueue.push_back( EventQueueEntry( nEventId, rArgs ) );

    /*  bCancel will contain the current Cancel value. It is possible that
        multiple events will try to modify the Cancel value. Every event
        handler receives the Cancel value of the previous event handler. */
    bool bCancel = false;

    /*  bExecuted will change to true if at least one event handler has been
        found and executed. */
    bool bExecuted = false;

    /*  Loop as long as there are more events to be processed. Derived classes
        may add new events to be processed in the virtual implPrepareEvent()
        function. */
    while( !aEventQueue.empty() )
    {
        /*  Check that all class members are available, and that we are not
            disposed (this may have happened at any time during execution of
            the last event handler). */
        if( mbDisposed || !mxModel.is() || !mpShell )
            throw uno::RuntimeException();

        
        const EventHandlerInfo& rInfo = getEventHandlerInfo( aEventQueue.front().mnEventId );
        uno::Sequence< uno::Any > aEventArgs = aEventQueue.front().maArgs;
        aEventQueue.pop_front();
        SAL_INFO("vbahelper", "VbaEventsHelperBase::processVbaEvent( \"" << rInfo.maMacroName << "\" )");

        /*  Let derived classes prepare the event, they may add new events for
            next iteration. If false is returned, the event handler must not be
            called. */
        if( implPrepareEvent( aEventQueue, rInfo, aEventArgs ) )
        {
            
            OUString aMacroPath = getEventHandlerPath( rInfo, aEventArgs );
            if( !aMacroPath.isEmpty() )
            {
                
                uno::Sequence< uno::Any > aVbaArgs = implBuildArgumentList( rInfo, aEventArgs );
                
                if( rInfo.mnCancelIndex >= 0 )
                {
                    if( rInfo.mnCancelIndex >= aVbaArgs.getLength() )
                        throw lang::IllegalArgumentException();
                    aVbaArgs[ rInfo.mnCancelIndex ] <<= bCancel;
                }
                
                uno::Any aRet, aCaller;
                executeMacro( mpShell, aMacroPath, aVbaArgs, aRet, aCaller );
                
                if( rInfo.mnCancelIndex >= 0 )
                {
                    checkArgument( aVbaArgs, rInfo.mnCancelIndex );
                    bCancel = extractBoolFromAny( aVbaArgs[ rInfo.mnCancelIndex ] );
                }
                
                bExecuted = true;
            }
        }
        
        implPostProcessEvent( aEventQueue, rInfo, bCancel );
    }

    
    if( bCancel )
        throw util::VetoException();

    
    return bExecuted;
}

void SAL_CALL VbaEventsHelperBase::notifyEvent( const document::EventObject& rEvent ) throw (uno::RuntimeException)
{
    SAL_INFO("vbahelper", "VbaEventsHelperBase::notifyEvent( \"" << rEvent.EventName << "\" )");
    if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ) )
        stopListening();
}

void SAL_CALL VbaEventsHelperBase::changesOccurred( const util::ChangesEvent& rEvent ) throw (uno::RuntimeException)
{
    
    try
    {
        ensureVBALibrary();
    }
    catch( uno::Exception& )
    {
        return;
    }

    
    uno::Reference< script::vba::XVBAModuleInfo > xSender( rEvent.Base, uno::UNO_QUERY );
    if( mxModuleInfos.get() != xSender.get() )
        return;

    
    for( sal_Int32 nIndex = 0, nLength = rEvent.Changes.getLength(); nIndex < nLength; ++nIndex )
    {
        const util::ElementChange& rChange = rEvent.Changes[ nIndex ];
        OUString aModuleName;
        if( (rChange.Accessor >>= aModuleName) && !aModuleName.isEmpty() ) try
        {
            
            if( getModuleType( aModuleName ) == script::ModuleType::NORMAL )
                
                maEventPaths.erase( OUString() );
            else
                
                maEventPaths.erase( aModuleName );
        }
        catch( uno::Exception& )
        {
        }
    }
}

void SAL_CALL VbaEventsHelperBase::disposing( const lang::EventObject& rEvent ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xSender( rEvent.Source, uno::UNO_QUERY );
    if( xSender.is() )
        stopListening();
}

void VbaEventsHelperBase::processVbaEventNoThrow( sal_Int32 nEventId, const uno::Sequence< uno::Any >& rArgs )
{
    try
    {
        processVbaEvent( nEventId, rArgs );
    }
    catch( uno::Exception& )
    {
    }
}



void VbaEventsHelperBase::registerEventHandler( sal_Int32 nEventId, sal_Int32 nModuleType,
        const sal_Char* pcMacroName, sal_Int32 nCancelIndex, const uno::Any& rUserData )
{
    EventHandlerInfo& rInfo = maEventInfos[ nEventId ];
    rInfo.mnEventId = nEventId;
    rInfo.mnModuleType = nModuleType;
    rInfo.maMacroName = OUString::createFromAscii( pcMacroName );
    rInfo.mnCancelIndex = nCancelIndex;
    rInfo.maUserData = rUserData;
}



void VbaEventsHelperBase::startListening()
{
    if( mbDisposed )
        return;

    uno::Reference< document::XEventBroadcaster > xEventBroadcaster( mxModel, uno::UNO_QUERY );
    if( xEventBroadcaster.is() )
        try { xEventBroadcaster->addEventListener( this ); } catch( uno::Exception& ) {}
}

void VbaEventsHelperBase::stopListening()
{
    if( mbDisposed )
        return;

    uno::Reference< document::XEventBroadcaster > xEventBroadcaster( mxModel, uno::UNO_QUERY );
    if( xEventBroadcaster.is() )
        try { xEventBroadcaster->removeEventListener( this ); } catch( uno::Exception& ) {}

    mxModel.clear();
    mpShell = 0;
    maEventInfos.clear();
    mbDisposed = true;
}

sal_Bool SAL_CALL VbaEventsHelperBase::hasVbaEventHandler( sal_Int32 nEventId, const uno::Sequence< uno::Any >& rArgs )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    EventHandlerInfoMap::const_iterator aIt = maEventInfos.find( nEventId );
    if( aIt == maEventInfos.end() )
        return sal_False; 
    else 
        return !getEventHandlerPath( aIt->second, rArgs ).isEmpty();
}

const VbaEventsHelperBase::EventHandlerInfo& VbaEventsHelperBase::getEventHandlerInfo(
        sal_Int32 nEventId ) const throw (lang::IllegalArgumentException)
{
    EventHandlerInfoMap::const_iterator aIt = maEventInfos.find( nEventId );
    if( aIt == maEventInfos.end() )
        throw lang::IllegalArgumentException();
    return aIt->second;
}

OUString VbaEventsHelperBase::getEventHandlerPath( const EventHandlerInfo& rInfo,
        const uno::Sequence< uno::Any >& rArgs ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OUString aModuleName;
    switch( rInfo.mnModuleType )
    {
        
        case script::ModuleType::NORMAL:
        break;

        
        case script::ModuleType::DOCUMENT:
            aModuleName = implGetDocumentModuleName( rInfo, rArgs );
            if( aModuleName.isEmpty() )
                throw lang::IllegalArgumentException();
        break;

        default:
            throw uno::RuntimeException(); 
    }

    /*  Performance improvement: Check the list of existing event handlers
        instead of searching in Basic source code every time. */
    EventHandlerPathMap::iterator aIt = maEventPaths.find( aModuleName );
    ModulePathMap& rPathMap = (aIt == maEventPaths.end()) ? updateModulePathMap( aModuleName ) : aIt->second;
    return rPathMap[ rInfo.mnEventId ];
}

void VbaEventsHelperBase::ensureVBALibrary() throw (uno::RuntimeException)
{
    if( !mxModuleInfos.is() ) try
    {
        maLibraryName = getDefaultProjectName( mpShell );
        if( maLibraryName.isEmpty() )
            throw uno::RuntimeException();
        uno::Reference< beans::XPropertySet > xModelProps( mxModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xBasicLibs( xModelProps->getPropertyValue(
            "BasicLibraries" ), uno::UNO_QUERY_THROW );
        mxModuleInfos.set( xBasicLibs->getByName( maLibraryName ), uno::UNO_QUERY_THROW );
        
        uno::Reference< util::XChangesNotifier > xChangesNotifier( mxModuleInfos, uno::UNO_QUERY_THROW );
        xChangesNotifier->addChangesListener( this );
    }
    catch( uno::Exception& )
    {
        
        stopListening();
        throw uno::RuntimeException();
    }
}

sal_Int32 VbaEventsHelperBase::getModuleType( const OUString& rModuleName ) throw (uno::RuntimeException)
{
    
    ensureVBALibrary();

    
    if( rModuleName.isEmpty() )
        return script::ModuleType::NORMAL;

    
    try
    {
        return mxModuleInfos->getModuleInfo( rModuleName ).ModuleType;
    }
    catch( uno::Exception& )
    {
    }
    throw uno::RuntimeException();
}

VbaEventsHelperBase::ModulePathMap& VbaEventsHelperBase::updateModulePathMap( const OUString& rModuleName ) throw (uno::RuntimeException)
{
    
    sal_Int32 nModuleType = getModuleType( rModuleName );
    
    ModulePathMap& rPathMap = maEventPaths[ rModuleName ];
    for( EventHandlerInfoMap::iterator aIt = maEventInfos.begin(), aEnd = maEventInfos.end(); aIt != aEnd; ++aIt )
    {
        const EventHandlerInfo& rInfo = aIt->second;
        if( rInfo.mnModuleType == nModuleType )
            rPathMap[ rInfo.mnEventId ] = resolveVBAMacro( mpShell, maLibraryName, rModuleName, rInfo.maMacroName );
    }
    return rPathMap;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
