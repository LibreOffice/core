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

#include "vbahelper/vbaeventshelperbase.hxx"
#include <filter/msfilter/msvbahelper.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

// ============================================================================

VbaEventsHelperBase::VbaEventsHelperBase( const uno::Sequence< uno::Any >& rArgs, const uno::Reference< uno::XComponentContext >& /*xContext*/ ) :
    mpShell( 0 ),
    mbDisposed( false )
{
    try
    {
        mxModel = getXSomethingFromArgs< frame::XModel >( rArgs, 0, false );
        mpShell = getSfxObjShell( mxModel );

        // add dispose listener
        uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY_THROW );
        xComponent->addEventListener( this );
    }
    catch( uno::Exception& )
    {
    }
}

VbaEventsHelperBase::~VbaEventsHelperBase()
{
    stopListening();
}

sal_Bool SAL_CALL VbaEventsHelperBase::hasVbaEventHandler( sal_Int32 nEventId, const uno::Sequence< uno::Any >& rArgs )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    // getEventHandlerInfo() throws, if unknown event dentifier has been passed
    const EventHandlerInfo& rInfo = getEventHandlerInfo( nEventId );
    // getEventHandlerPath() searches for the macro in the document
    return getEventHandlerPath( rInfo, rArgs ).getLength() > 0;
}

void SAL_CALL VbaEventsHelperBase::processVbaEvent( sal_Int32 nEventId, const uno::Sequence< uno::Any >& rArgs )
        throw (lang::IllegalArgumentException, script::provider::ScriptFrameworkErrorException, util::VetoException, uno::RuntimeException)
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

    /*  bSuccess will change to true if at least one event handler has been
        executed successfully. */
    bool bSuccess = false;

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

        /*  Let derived classes prepare the event, they may add new events for
            next iteration. If false is returned, the event handler must not be
            called. */
        bool bEventSuccess = false;
        if( implPrepareEvent( aEventQueue, rInfo, aEventArgs ) )
        {
            // search the event handler macro in the document
            ::rtl::OUString aMacroPath = getEventHandlerPath( rInfo, aEventArgs );
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
                bEventSuccess = executeMacro( mpShell, aMacroPath, aVbaArgs, aRet, aCaller );
                // extract new cancel value
                if( rInfo.mnCancelIndex >= 0 )
                {
                    if( rInfo.mnCancelIndex >= aVbaArgs.getLength() )
                        throw lang::IllegalArgumentException();
                    // cancel value may be boolean or any integer type, Any(bool) does not extract to sal_Int32
                    bool bNewCancel = false;
                    sal_Int32 nNewCancel = 0;
                    if( aVbaArgs[ rInfo.mnCancelIndex ] >>= bNewCancel )
                        bCancel = bNewCancel;
                    else if( aVbaArgs[ rInfo.mnCancelIndex ] >>= nNewCancel )
                        bCancel = nNewCancel != 0;
                }
            }
            // global success, if at least one event handler succeeded
            bSuccess |= bEventSuccess;
        }
        // post processing (also, if event handler does not exist, or disabled, or on error
        implPostProcessEvent( aEventQueue, rInfo, bEventSuccess, bCancel );
    }

    // if event handlers want to cancel the event, do so regardless of any errors
    if( bCancel )
        throw util::VetoException();

    // if no event handler finished successfully, throw
    if( !bSuccess )
        throw script::provider::ScriptFrameworkErrorException();
}

void SAL_CALL VbaEventsHelperBase::disposing( const lang::EventObject& /*aSource*/ ) throw (uno::RuntimeException)
{
    OSL_TRACE( "VbaEventsHelperBase::disposing" );
    stopListening();
    mbDisposed = true;
}

// protected ------------------------------------------------------------------

void VbaEventsHelperBase::registerEventHandler( sal_Int32 nEventId,
        const sal_Char* pcMacroName, EventHandlerType eType, sal_Int32 nCancelIndex, const uno::Any& rUserData )
{
    EventHandlerInfo& rInfo = maEvents[ nEventId ];
    rInfo.mnEventId = nEventId;
    rInfo.maMacroName = ::rtl::OUString::createFromAscii( pcMacroName );
    rInfo.meType = eType;
    rInfo.mnCancelIndex = nCancelIndex;
    rInfo.maUserData = rUserData;
}

// private --------------------------------------------------------------------

const VbaEventsHelperBase::EventHandlerInfo& VbaEventsHelperBase::getEventHandlerInfo(
        sal_Int32 nEventId ) const throw (lang::IllegalArgumentException)
{
    EventHandlerMap::const_iterator aIt = maEvents.find( nEventId );
    if( aIt == maEvents.end() )
        throw lang::IllegalArgumentException();
    return aIt->second;
}

::rtl::OUString VbaEventsHelperBase::getEventHandlerPath( const EventHandlerInfo& rInfo,
        const uno::Sequence< uno::Any >& rArgs ) const throw (lang::IllegalArgumentException)
{
    ::rtl::OUString aMacroName;
    switch( rInfo.meType )
    {
        case EVENTHANDLER_GLOBAL:
            aMacroName = rInfo.maMacroName;
        break;
        case EVENTHANDLER_DOCUMENT:
            aMacroName = ::rtl::OUStringBuffer( implGetDocumentModuleName( rInfo, rArgs ) ).
                append( sal_Unicode( '.' ) ).append( rInfo.maMacroName ).makeStringAndClear();
        break;
    }
    MacroResolvedInfo aMacroInfo = resolveVBAMacro( mpShell, aMacroName, false );
    return aMacroInfo.mbFound ? ::rtl::OUString( aMacroInfo.msResolvedMacro ) : ::rtl::OUString();
}

void VbaEventsHelperBase::stopListening()
{
    if( !mbDisposed ) try
    {
        uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY_THROW );
        xComponent->removeEventListener( this );
    }
    catch( uno::Exception& )
    {
    }
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
