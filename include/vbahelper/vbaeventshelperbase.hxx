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

#ifndef INCLUDED_VBAHELPER_VBAEVENTSHELPERBASE_HXX
#define INCLUDED_VBAHELPER_VBAEVENTSHELPERBASE_HXX

#include <deque>
#include <map>
#include <unordered_map>

#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XChangesListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>

namespace com::sun::star {
    namespace document { struct EventObject; }
    namespace frame { class XModel; }
    namespace script::vba { class XVBAModuleInfo; }
    namespace util { struct ChangesEvent; }
}

class SfxObjectShell;

typedef ::cppu::WeakImplHelper<
    css::script::vba::XVBAEventProcessor,
    css::document::XEventListener,
    css::util::XChangesListener,
    css::lang::XServiceInfo > VbaEventsHelperBase_BASE;

class VBAHELPER_DLLPUBLIC VbaEventsHelperBase : public VbaEventsHelperBase_BASE
{
public:
    VbaEventsHelperBase(
        const css::uno::Sequence< css::uno::Any >& rArgs );
    virtual ~VbaEventsHelperBase() override;

    // script::vba::XVBAEventProcessor
    virtual sal_Bool SAL_CALL hasVbaEventHandler( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) override;
    virtual sal_Bool SAL_CALL processVbaEvent( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) override;

    // document::XEventListener
    virtual void SAL_CALL notifyEvent( const css::document::EventObject& rEvent ) override;

    // util::XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& rEvent ) override;

    // lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    // little helpers ---------------------------------------------------------

    bool hasModule(const OUString& rModuleName);

    /** Helper to execute event handlers without throwing any exceptions. */
    void processVbaEventNoThrow( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs );

    /** @throws css::lang::IllegalArgumentException if the passed sequence does not contain a value at the specified index. */
    static void checkArgument( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex )
        { if( (nIndex < 0) || (nIndex >= rArgs.getLength()) ) throw css::lang::IllegalArgumentException(); }

    /** @throws css::lang::IllegalArgumentException if the passed sequence does not contain a value of a specific at the specified index. */
    template< typename Type >
    static void checkArgumentType( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex )
        { checkArgument( rArgs, nIndex ); if( !rArgs[ nIndex ].has< Type >() ) throw css::lang::IllegalArgumentException(); }

protected:


    struct EventHandlerInfo
    {
        sal_Int32 mnEventId;
        sal_Int32 mnModuleType;
        OUString maMacroName;
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
            const char* pcMacroName,
            sal_Int32 nCancelIndex = -1,
            const css::uno::Any& rUserData = css::uno::Any() );


    struct EventQueueEntry
    {
        sal_Int32 mnEventId;
        css::uno::Sequence< css::uno::Any > maArgs;
        /*implicit*/ EventQueueEntry( sal_Int32 nEventId ) : mnEventId( nEventId ) {}
        EventQueueEntry( sal_Int32 nEventId, const css::uno::Sequence< css::uno::Any >& rArgs ) : mnEventId( nEventId ), maArgs( rArgs ) {}
    };
    typedef ::std::deque< EventQueueEntry > EventQueue;

    /** Derived classes do additional preparations and return whether the
        event handler has to be called.

        @throws css::uno::RuntimeException
    */
    virtual bool implPrepareEvent(
        EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) = 0;

    /** Derived classes have to return the argument list for the specified VBA event handler.

        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    virtual css::uno::Sequence< css::uno::Any > implBuildArgumentList(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) = 0;

    /** Derived classes may do additional postprocessing. Called even if the
        event handler does not exist, or if an error occurred during execution.

        @throws css::uno::RuntimeException
    */
    virtual void implPostProcessEvent(
        EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo,
        bool bCancel ) = 0;

    /** Derived classes have to return the name of the Basic document module.

        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    virtual OUString implGetDocumentModuleName(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs ) const = 0;

private:
    typedef ::std::map< sal_Int32, OUString > ModulePathMap;

    /** Starts listening at the document model. */
    void startListening();
    /** Stops listening at the document model. */
    void stopListening();

    /** Returns the event handler info struct for the specified event, or throws.


        @throws css::lang::IllegalArgumentException
    */
    const EventHandlerInfo& getEventHandlerInfo( sal_Int32 nEventId ) const;

    /** Searches the event handler in the document and returns its full script path.


        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    OUString getEventHandlerPath(
        const EventHandlerInfo& rInfo,
        const css::uno::Sequence< css::uno::Any >& rArgs );

    /** On first call, accesses the Basic library containing the VBA source code.

        @throws css::uno::RuntimeException
    */
    void ensureVBALibrary();

    /** Returns the type of the Basic module with the specified name.

        @throws css::uno::RuntimeException
    */
    sal_Int32 getModuleType( const OUString& rModuleName );

    /** Updates the map containing paths to event handlers for a Basic module.

        @throws css::uno::RuntimeException
    */
    ModulePathMap& updateModulePathMap( const OUString& rModuleName );

protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    SfxObjectShell* mpShell;

private:
    typedef std::map< sal_Int32, EventHandlerInfo > EventHandlerInfoMap;
    typedef std::unordered_map< OUString, ModulePathMap > EventHandlerPathMap;

    EventHandlerInfoMap maEventInfos;
    EventHandlerPathMap maEventPaths;
    css::uno::Reference< css::script::vba::XVBAModuleInfo > mxModuleInfos;
    OUString maLibraryName;
    bool mbDisposed;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
