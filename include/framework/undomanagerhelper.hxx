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

#ifndef INCLUDED_FRAMEWORK_UNDOMANAGERHELPER_HXX
#define INCLUDED_FRAMEWORK_UNDOMANAGERHELPER_HXX

#include <framework/fwedllapi.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <memory>

namespace com::sun::star::document { class XUndoAction; }
namespace com::sun::star::document { class XUndoManager; }
namespace com::sun::star::document { class XUndoManagerListener; }
namespace com::sun::star::util { class XModifyListener; }
namespace framework { class IMutex; }

class SfxUndoManager;


namespace framework
{


    //= IMutexGuard

    class SAL_NO_VTABLE IMutexGuard
    {
    public:
        /** clears the lock. If the guard does not currently hold the lock, nothing happens.
        */
        virtual void clear() = 0;

        /** returns the mutex guarded by the instance.

            Even if the guard currently has not a lock on the mutex, this method must succeed.
        */
        virtual IMutex& getGuardedMutex() = 0;

    protected:
        ~IMutexGuard() {}
    };


    //= IUndoManagerImplementation

    class SAL_NO_VTABLE IUndoManagerImplementation
    {
    public:
        /** returns the SfxUndoManager interface to the actual Undo stack

            @throws css::lang::DisposedException
                when the instance is already disposed, and no SfxUndoManager can be provided

            @throws css::lang::NotInitializedException
                when the instance is not initialized, yet, and no SfxUndoManager can be provided
        */
        virtual SfxUndoManager&        getImplUndoManager() = 0;

        /** provides access to an UNO interface for the XUndoManager implementation. Used when throwing exceptions.
        */
        virtual css::uno::Reference< css::document::XUndoManager >
                                        getThis() = 0;

    protected:
        ~IUndoManagerImplementation() {}
    };


    //= UndoManagerHelper

    class UndoManagerHelper_Impl;
    /** helper class for implementing an XUndoManager

        Several of the methods of the class take an IMutexGuard instance. It is assumed that this guard has a lock on
        its mutex at the moment the method is entered. The lock will be released before any notifications to the
        registered XUndoManagerListeners happen.

        The following locking strategy is used for this mutex:
        <ul><li>Any notifications to the registered XUndoManagerListeners are after the guard has been cleared. i.e.
                without the mutex being locked.</p>
            <li>Any calls into the <code>SfxUndoManager</code> implementation is made without the mutex being locked.
                Note that this implies that the <code>SfxUndoManager</code> implementation must be thread-safe in itself
                (which is true for the default implementation, SfxUndoManager).</li>
            <li>An exception to the previous item are the <member>SfxUndoManager::Undo</member> and
                <member>SfxUndoManager::Redo</member> methods: They're called with the given external mutex being
                locked.</li>
        </ul>

        The reason for the exception for SfxUndoManager::Undo and SfxUndoManager::Redo is that those are expected to
        modify the actual document which the UndoManager works for. And as long as our documents are not thread-safe,
        and as long as we do not re-fit <strong>all</strong> existing SfxUndoImplementations to <em>not</em> expect
        the dreaded SolarMutex being locked when they're called, the above behavior is a compromise between "how it should
        be" and "how it can realistically be".
    */
    class FWE_DLLPUBLIC UndoManagerHelper
    {
    public:
        UndoManagerHelper( IUndoManagerImplementation& i_undoManagerImpl );
        ~UndoManagerHelper();

        // life time control
        void disposing();

        // XUndoManager equivalents
        void            enterUndoContext( const OUString& i_title, IMutexGuard& i_instanceLock );
        void            enterHiddenUndoContext( IMutexGuard& i_instanceLock );
        void            leaveUndoContext( IMutexGuard& i_instanceLock );
        void            addUndoAction( const css::uno::Reference< css::document::XUndoAction >& i_action, IMutexGuard& i_instanceLock );
        void            undo( IMutexGuard& i_instanceLock );
        void            redo( IMutexGuard& i_instanceLock );
        bool            isUndoPossible() const;
        bool            isRedoPossible() const;
        OUString getCurrentUndoActionTitle() const;
        OUString getCurrentRedoActionTitle() const;
        css::uno::Sequence< OUString >
                        getAllUndoActionTitles() const;
        css::uno::Sequence< OUString >
                        getAllRedoActionTitles() const;
        void            clear( IMutexGuard& i_instanceLock );
        void            clearRedo( IMutexGuard& i_instanceLock );
        void            reset( IMutexGuard& i_instanceLock );
        void            addUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener );
        void            removeUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener );

        // XLockable, base of XUndoManager, equivalents
        void            lock();
        void            unlock();
        bool            isLocked();

        // XModifyBroadcaster equivalents
        void            addModifyListener( const css::uno::Reference< css::util::XModifyListener >& i_listener );
        void            removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& i_listener );

    private:
        std::unique_ptr< UndoManagerHelper_Impl >   m_xImpl;
    };


} // namespace framework


#endif // INCLUDED_FRAMEWORK_UNDOMANAGERHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
