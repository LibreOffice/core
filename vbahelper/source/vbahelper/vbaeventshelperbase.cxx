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

#include "vbahelper/vbaeventshelperbase.hxx"
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <filter/msfilter/msvbahelper.hxx>
#include <unotools/eventcfg.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

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
    OSL_ENSURE( mbDisposed, "VbaEventsHelperBase::~VbaEventsHelperBase - missing disposing notification" );
}

sal_Bool SAL_CALL VbaEventsHelperBase::hasVbaEventHandler( sal_Int32 nEventId, const uno::Sequence< uno::Any >& rArgs )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    // getEventHandlerInfo() throws, if unknown event dentifier has been passed
    const EventHandlerInfo& rInfo = getEventHandlerInfo( nEventId );
    // getEventHandlerPath() searches for the macro in the document
    return getEventHandlerPath( rInfo, rArgs ).getLength() > 0;
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

        // get info for next event
        const EventHandlerInfo& rInfo = getEventHandlerInfo( aEventQueue.front().mnEventId );
        uno::Sequence< uno::Any > aEventArgs = aEventQueue.front().maArgs;
        aEventQueue.pop_front();
        OSL_TRACE( "VbaEventsHelperBase::processVbaEvent( \"%s\" )", ::rtl::OUStringToOString( rInfo.maMacroName, RTL_TEXTENCODING_UTF8 ).getStr() );

        /*  Let derived classes prepare the event, they may add new events for
            next iteration. If false is returned, the event handler must not be
            called. */
        if( implPrepareEvent( aEventQueue, rInfo, aEventArgs ) )
        {
            // search the event handler macro in the document
            OUString aMacroPath = getEventHandlerPath( rInfo, aEventArgs );
            if( aMacroPath.getLength() > 0 )
            {
                // build the argument list
                uno::Sequence< uno::Any > aVbaArgs = implBuildArgumentList( rInfo, aEventArgs );
                // insert current cancel value
                if( rInfo.mnCancelIndex >= 0 )
                {
                    if( rInfo.mnCancelIndex >= aVbaArgs.getLength() )
                        throw lang::IllegalArgumentException();
                    aVbaArgs[ rInfo.mnCancelIndex ] <<= bCancel;
                }
                // execute the event handler
                uno::Any aRet, aCaller;
                executeMacro( mpShell, aMacroPath, aVbaArgs, aRet, aCaller );
                // extract new cancel value (may be boolean or any integer type)
                if( rInfo.mnCancelIndex >= 0 )
                {
                    checkArgument( aVbaArgs, rInfo.mnCancelIndex );
                    bCancel = extractBoolFromAny( aVbaArgs[ rInfo.mnCancelIndex ] );
                }
                // event handler has been found
                bExecuted = true;
            }
        }
        // post processing (also, if event handler does not exist, or disabled, or on error
        implPostProcessEvent( aEventQueue, rInfo, bCancel );
    }

    // if event handlers want to cancel the event, do so regardless of any errors
    if( bCancel )
        throw util::VetoException();

    // return true, if at least one event handler has been found
    return bExecuted;
}

void SAL_CALL VbaEventsHelperBase::notifyEvent( const document::EventObject& rEvent ) throw (uno::RuntimeException)
{
    OSL_TRACE( "VbaEventsHelperBase::notifyEvent( \"%s\" )", ::rtl::OUStringToOString( rEvent.EventName, RTL_TEXTENCODING_UTF8 ).getStr() );
    if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ) )
        stopListening();
}

void SAL_CALL VbaEventsHelperBase::changesOccurred( const util::ChangesEvent& rEvent ) throw (uno::RuntimeException)
{
    // make sure the VBA library exists
    try
    {
        ensureVBALibrary();
    }
    catch( uno::Exception& )
    {
        return;
    }

    // check that the sender of the event is the VBA library
    uno::Reference< script::vba::XVBAModuleInfo > xSender( rEvent.Base, uno::UNO_QUERY );
    if( mxModuleInfos.get() != xSender.get() )
        return;

    // process all changed modules
    for( sal_Int32 nIndex = 0, nLength = rEvent.Changes.getLength(); nIndex < nLength; ++nIndex )
    {
        const util::ElementChange& rChange = rEvent.Changes[ nIndex ];
        OUString aModuleName;
        if( (rChange.Accessor >>= aModuleName) && (aModuleName.getLength() > 0) ) try
        {
            // invalidate event handler path map depending on module type
            if( getModuleType( aModuleName ) == script::ModuleType::NORMAL )
                // paths to global event handlers are stored with empty key (will be searched in all normal code modules)
                maEventPaths.erase( OUString() );
            else
                // paths to class/form/document event handlers are keyed by module name
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

// protected ------------------------------------------------------------------

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

// private --------------------------------------------------------------------

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
        // global event handlers may exist in any standard code module
        case script::ModuleType::NORMAL:
        break;

        // document event: get name of the code module associated to the event sender
        case script::ModuleType::DOCUMENT:
            aModuleName = implGetDocumentModuleName( rInfo, rArgs );
            if( aModuleName.getLength() == 0 )
                throw lang::IllegalArgumentException();
        break;

        default:
            throw uno::RuntimeException(); // unsupported module type
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
        if( maLibraryName.getLength() == 0 )
            throw uno::RuntimeException();
        uno::Reference< beans::XPropertySet > xModelProps( mxModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xBasicLibs( xModelProps->getPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicLibraries" ) ) ), uno::UNO_QUERY_THROW );
        mxModuleInfos.set( xBasicLibs->getByName( maLibraryName ), uno::UNO_QUERY_THROW );
        // listen to changes in the VBA source code
        uno::Reference< util::XChangesNotifier > xChangesNotifier( mxModuleInfos, uno::UNO_QUERY_THROW );
        xChangesNotifier->addChangesListener( this );
    }
    catch( uno::Exception& )
    {
        // error accessing the Basic library, so this object is useless
        stopListening();
        throw uno::RuntimeException();
    }
}

sal_Int32 VbaEventsHelperBase::getModuleType( const OUString& rModuleName ) throw (uno::RuntimeException)
{
    // make sure the VBA library exists
    ensureVBALibrary();

    // no module specified: global event handler in standard code modules
    if( rModuleName.getLength() == 0 )
        return script::ModuleType::NORMAL;

    // get module type from module info
    try
    {
        return mxModuleInfos->getModuleInfo( rModuleName ).ModuleType;
    }
    catch( uno::Exception& )
    {
    }
    throw uno::RuntimeException();
}

VbaEventsHelperBase::ModulePathMap& VbaEventsHelperBase::updateModulePathMap( const ::rtl::OUString& rModuleName ) throw (uno::RuntimeException)
{
    // get type of the specified module (throws on error)
    sal_Int32 nModuleType = getModuleType( rModuleName );
    // search for all event handlers
    ModulePathMap& rPathMap = maEventPaths[ rModuleName ];
    for( EventHandlerInfoMap::iterator aIt = maEventInfos.begin(), aEnd = maEventInfos.end(); aIt != aEnd; ++aIt )
    {
        const EventHandlerInfo& rInfo = aIt->second;
        if( rInfo.mnModuleType == nModuleType )
            rPathMap[ rInfo.mnEventId ] = resolveVBAMacro( mpShell, maLibraryName, rModuleName, rInfo.maMacroName );
    }
    return rPathMap;
}

// ============================================================================
