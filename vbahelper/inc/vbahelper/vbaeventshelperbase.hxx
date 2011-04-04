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

#include <deque>
#include <hash_map>
#include <map>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <cppuhelper/implbase3.hxx>
#include "vbahelper/vbahelper.hxx"

namespace com { namespace sun { namespace star {
    namespace script { namespace vba { class XVBAModuleInfo; } }
    namespace uno { class XComponentContext; }
} } }

// ============================================================================

typedef ::cppu::WeakImplHelper3<
    css::script::vba::XVBAEventProcessor,
    css::document::XEventListener,
    css::util::XChangesListener > VbaEventsHelperBase_BASE;

class VBAHELPER_DLLPUBLIC VbaEventsHelperBase : public VbaEventsHelperBase_BASE
{
public:
    VbaEventsHelperBase(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~VbaEventsHelperBase();

    // script::vba::XVBAEventProcessor
    virtual sal_Bool SAL_CALL hasVbaEventHandler( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL processVbaEvent( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException, css::util::VetoException, css::uno::RuntimeException);

    // document::XEventListener
    virtual void SAL_CALL notifyEvent( const css::document::EventObject& rEvent ) throw (css::uno::RuntimeException);

    // util::XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& rEvent ) throw (css::uno::RuntimeException);

    // lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) throw (css::uno::RuntimeException);

    // little helpers ---------------------------------------------------------

    /** Helper to execute event handlers without throwing any exceptions. */
    void processVbaEventNoThrow( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs );

    /** Throws, if the passed sequence does not contain a value at the specified index. */
    static inline void checkArgument( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) throw (css::lang::IllegalArgumentException)
        { if( (nIndex < 0) || (nIndex >= rArgs.getLength()) ) throw css::lang::IllegalArgumentException(); }

    /** Throws, if the passed sequence does not contain a value of a specific at the specified index. */
    template< typename Type >
    static inline void checkArgumentType( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) throw (css::lang::IllegalArgumentException)
        { checkArgument( rArgs, nIndex ); if( !rArgs[ nIndex ].has< Type >() ) throw css::lang::IllegalArgumentException(); }

protected:
    // ------------------------------------------------------------------------

    struct EventHandlerInfo
    {
        sal_Int32 mnEventId;
        sal_Int32 mnModuleType;
        ::rtl::OUString maMacroName;
        sal_Int32 mnCancelIndex;
        css::uno::Any maUserData;
    };

    /** Registers a supported event handler.

        @param nEventId  Event identifier from com.sun.star.script.vba.VBAEventId.
        @param nModuleType  Type of the module containing the event handler.
        @param pcMacroName  Name of the associated VBA event handler macro.
        @param nCancelIndex  0-based index of Cancel parameter, or -1.
        @param rUserData  User data for free usage in derived implementations. */
    void registerEventHandler(
            sal_Int32 nEventId,
            sal_Int32 nModuleType,
            const sal_Char* pcMacroName,
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
        event handler does not exist, or if an error occured during execution. */
    virtual void implPostProcessEvent(
        EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo,
        bool bCancel ) throw (css::uno::RuntimeException) = 0;

    /** Derived classes have to return the name of the Basic document module. */
    virtual ::rtl::OUString implGetDocumentModuleName(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) const throw (css::lang::IllegalArgumentException) = 0;

private:
    typedef ::std::map< sal_Int32, ::rtl::OUString > ModulePathMap;

    /** Starts listening at the document model. */
    void startListening();
    /** Stops listening at the document model. */
    void stopListening();

    /** Returns the event handler info struct for the specified event, or throws. */
    const EventHandlerInfo& getEventHandlerInfo( sal_Int32 nEventId ) const throw (css::lang::IllegalArgumentException);

    /** Searches the event handler in the document and returns its full script path. */
    ::rtl::OUString getEventHandlerPath(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    /** On first call, accesses the Basic library containing the VBA source code. */
    void ensureVBALibrary() throw (css::uno::RuntimeException);

    /** Returns the type of the Basic module with the specified name. */
    sal_Int32 getModuleType( const ::rtl::OUString& rModuleName ) throw (css::uno::RuntimeException);

    /** Updates the map containing paths to event handlers for a Basic module. */
    ModulePathMap& updateModulePathMap( const ::rtl::OUString& rModuleName ) throw (css::uno::RuntimeException);

protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    SfxObjectShell* mpShell;

private:
    typedef ::std::map< sal_Int32, EventHandlerInfo > EventHandlerInfoMap;
    typedef ::std::hash_map< ::rtl::OUString, ModulePathMap, ::rtl::OUStringHash > EventHandlerPathMap;

    EventHandlerInfoMap maEventInfos;
    EventHandlerPathMap maEventPaths;
    css::uno::Reference< css::script::vba::XVBAModuleInfo > mxModuleInfos;
    ::rtl::OUString maLibraryName;
    bool mbDisposed;
};

// ============================================================================

#endif
