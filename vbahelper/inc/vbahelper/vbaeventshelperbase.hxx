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

#ifndef VBAHELPER_VBAEVENTSHELPERBASE_HXX
#define VBAHELPER_VBAEVENTSHELPERBASE_HXX

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <cppuhelper/implbase2.hxx>
#include <map>
#include <deque>
#include "vbahelper/vbahelper.hxx"

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
} } }

// ============================================================================

typedef ::cppu::WeakImplHelper2< css::script::vba::XVBAEventProcessor, css::lang::XEventListener > VbaEventsHelperBase_BASE;

class VBAHELPER_DLLPUBLIC VbaEventsHelperBase : public VbaEventsHelperBase_BASE
{
public:
    VbaEventsHelperBase(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~VbaEventsHelperBase();

    // XVBAEventProcessor
    virtual sal_Bool SAL_CALL hasVbaEventHandler( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual void SAL_CALL processVbaEvent( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException, css::script::provider::ScriptFrameworkErrorException, css::util::VetoException, css::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& aSource ) throw (css::uno::RuntimeException);

    // little helpers ---------------------------------------------------------

    /** Throws, if the passed sequence does not contain a value at the specified index. */
    static inline void checkArgument( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) throw (css::lang::IllegalArgumentException)
        { if( (nIndex < 0) || (nIndex >= rArgs.getLength()) ) throw css::lang::IllegalArgumentException(); }

    /** Throws, if the passed sequence does not contain a value of a specific at the specified index. */
    template< typename Type >
    static inline void checkArgumentType( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) throw (css::lang::IllegalArgumentException)
        { checkArgument( rArgs, nIndex ); if( !rArgs[ nIndex ].has< Type >() ) throw css::lang::IllegalArgumentException(); }

protected:
    // ------------------------------------------------------------------------

    enum EventHandlerType { EVENTHANDLER_GLOBAL, EVENTHANDLER_DOCUMENT };
    struct EventHandlerInfo
    {
        sal_Int32 mnEventId;
        ::rtl::OUString maMacroName;
        EventHandlerType meType;
        sal_Int32 mnCancelIndex;
        css::uno::Any maUserData;
    };

    /** Registers a supported event handler.

        @param nEventId  Event identifier from com.sun.star.script.vba.VBAEventId.
        @param pcMacroName  Name of the associated VBA event handler macro.
        @param eType  Document event or global event.
        @param nCancelIndex  0-based index of Cancel parameter, or -1.
        @param rUserData  User data for free usage in derived implementations. */
    void registerEventHandler(
            sal_Int32 nEventId,
            const sal_Char* pcMacroName,
            EventHandlerType eType = EVENTHANDLER_DOCUMENT,
            sal_Int32 nCancelIndex = -1,
            const css::uno::Any& rUserData = css::uno::Any() );

    // ------------------------------------------------------------------------

    struct EventQueueEntry
    {
        sal_Int32 mnEventId;
        css::uno::Sequence< css::uno::Any > maArgs;
        inline /*implicit*/ EventQueueEntry( sal_Int32 nEventId ) : mnEventId( nEventId ) {}
        inline EventQueueEntry( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) : mnEventId( nEventId ), maArgs( rArgs ) {}
    };
    typedef ::std::deque< EventQueueEntry > EventQueue;

    /** Derived classes do additional prpeparations and return whether the
        event handler has to be called. */
    virtual bool implPrepareEvent(
        EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::uno::RuntimeException) = 0;

    /** Derived classes have to return the argument list for the specified VBA event handler. */
    virtual css::uno::Sequence< css::uno::Any > implBuildArgumentList(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException) = 0;

    /** Derived classes may do additional postprocessing. Called even if the
        event handler does not exist, or if an error occurred during execution. */
    virtual void implPostProcessEvent(
        EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo,
        bool bSuccess,
        bool bCancel ) throw (css::uno::RuntimeException) = 0;

    /** Derived classes have to return the name of the Basic document module. */
    virtual ::rtl::OUString implGetDocumentModuleName(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) const throw (css::lang::IllegalArgumentException) = 0;

private:
    /** Returns the event handler info struct for the specified event, or throws. */
    const EventHandlerInfo& getEventHandlerInfo( sal_Int32 nEventId ) const throw (css::lang::IllegalArgumentException);

    /** Searches the event handler in the document and returns its full script path. */
    ::rtl::OUString getEventHandlerPath(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) const throw (css::lang::IllegalArgumentException);

    /** Removes this instance from all broadcasters. */
    void stopListening();

protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    SfxObjectShell* mpShell;

private:
    typedef ::std::map< sal_Int32, EventHandlerInfo > EventHandlerMap;

    EventHandlerMap maEvents;
    bool mbDisposed;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
