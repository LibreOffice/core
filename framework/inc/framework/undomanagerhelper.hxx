/*************************************************************************
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

#ifndef FRAMEWORK_UNDOMANAGERHELPER_HXX
#define FRAMEWORK_UNDOMANAGERHELPER_HXX

#include "framework/fwedllapi.h"
#include "framework/iguard.hxx"
#include "framework/imutex.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
/** === end UNO includes === **/

#include <boost/scoped_ptr.hpp>

namespace svl
{
    class IUndoManager;
}

//......................................................................................................................
namespace framework
{
//......................................................................................................................

    //==================================================================================================================
    //= IMutexGuard
    //==================================================================================================================
    class SAL_NO_VTABLE IMutexGuard : public IGuard
    {
    public:
        /** returns the mutex guarded by the instance.

            Even if the guard currently has not a lock on the mutex, this method must succeed.
        */
        virtual IMutex& getGuardedMutex() = 0;
    };

    //==================================================================================================================
    //= IUndoManagerImplementation
    //==================================================================================================================
    class SAL_NO_VTABLE IUndoManagerImplementation
    {
    public:
        /** returns the IUndoManager interface to the actual Undo stack

            @throws com::sun::star::lang::DisposedException
                when the instance is already disposed, and no IUndoManager can be provided

            @throws com::sun::star::lang::NotInitializedException
                when the instance is not initialized, yet, and no IUndoManager can be provided
        */
        virtual ::svl::IUndoManager&    getImplUndoManager() = 0;

        /** provides access to an UNO interface for the XUndoManager implementation. Used when throwing exceptions.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager >
                                        getThis() = 0;
    };

    //==================================================================================================================
    //= UndoManagerHelper
    //==================================================================================================================
    class UndoManagerHelper_Impl;
    /** helper class for implementing an XUndoManager

        Several of the methods of the class take an IMutexGuard instance. It is assumed that this guard has a lock on
        its mutext at the moment the method is entered. The lock will be released before any notifications to the
        registered XUndoManagerListeners happen.

        The following locking strategy is used for this mutex:
        <ul><li>Any notifications to the registered XUndoManagerListeners are after the guard has been cleared. i.e.
                without the mutex being locked.</p>
            <li>Any calls into the <code>IUndoManager</code> implementation is made without the mutex being locked.
                Note that this implies that the <code>IUndoManager</code> implementation must be thread-safe in itself
                (which is true for the default implementation, SfxUndoManager).</li>
            <li>An exception to the previous item are the <member>IUndoManager::Undo</member> and
                <member>IUndoManager::Redo</member> methods: They're called with the given external mutex being
                locked.</li>
        </ul>

        The reason for the exception for IUndoManager::Undo and IUndoManager::Redo is that those are expected to
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
        void            enterUndoContext( const ::rtl::OUString& i_title, IMutexGuard& i_instanceLock );
        void            enterHiddenUndoContext( IMutexGuard& i_instanceLock );
        void            leaveUndoContext( IMutexGuard& i_instanceLock );
        void            addUndoAction( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoAction >& i_action, IMutexGuard& i_instanceLock );
        void            undo( IMutexGuard& i_instanceLock );
        void            redo( IMutexGuard& i_instanceLock );
        ::sal_Bool      isUndoPossible() const;
        ::sal_Bool      isRedoPossible() const;
        ::rtl::OUString getCurrentUndoActionTitle() const;
        ::rtl::OUString getCurrentRedoActionTitle() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        getAllUndoActionTitles() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        getAllRedoActionTitles() const;
        void            clear( IMutexGuard& i_instanceLock );
        void            clearRedo( IMutexGuard& i_instanceLock );
        void            reset( IMutexGuard& i_instanceLock );
        void            addUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener );
        void            removeUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener );

        // XLockable, base of XUndoManager, equivalents
        void            lock();
        void            unlock();
        ::sal_Bool      isLocked();

        // XModifyBroadcaster equivalents
        void            addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& i_listener );
        void            removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& i_listener );

    private:
        ::boost::scoped_ptr< UndoManagerHelper_Impl >   m_pImpl;
    };

//......................................................................................................................
} // namespace framework
//......................................................................................................................

#endif // FRAMEWORK_UNDOMANAGERHELPER_HXX
