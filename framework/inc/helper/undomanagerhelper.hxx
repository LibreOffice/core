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

/** === begin UNO includes === **/
#include <com/sun/star/document/XUndoManager.hpp>
/** === end UNO includes === **/

#include <boost/scoped_ptr.hpp>

namespace osl
{
    class Mutex;
}

namespace svl
{
    class IUndoManager;
}

//......................................................................................................................
namespace framework
{
//......................................................................................................................

    //==================================================================================================================
    //= IUndoManagerImplementation
    //==================================================================================================================
    class SAL_NO_VTABLE IUndoManagerImplementation
    {
    public:
        /** returns the mutex which is protecting the instance. Needed for listener administration synchronization.

            Note that the mutex will <em>not</em> be used for multi-threading safety of the UndoManagerHelper.
        */
        virtual ::osl::Mutex&           getMutex() = 0;

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
    //= IClearableInstanceLock
    //==================================================================================================================
    /** helper class for releasing a lock

        Since clients of UndoManagerHelper are responsible for locking their instance, but the UndoManagerHelper
        needs to notify its listeners, and this needs to happen without any instance lock, all affected methods
        take an IClearableInstanceLock parameter, to be able to clear the owner's lock before doing any notifications.
    */
    class SAL_NO_VTABLE IClearableInstanceLock
    {
    public:
        virtual void clear() = 0;
    };

    //==================================================================================================================
    //= UndoManagerHelper
    //==================================================================================================================
    class UndoManagerHelper_Impl;
    /** helper class for implementing an XUndoManager

        The class defines the same methods as an XUndoManager does, but lacks certain aspects of a full-blown UNO
        component. In particular, it is the responsibility of the owner of the instance to care for multi-threading
        safety, and for disposal checks.
    */
    class UndoManagerHelper
    {
    public:
        UndoManagerHelper( IUndoManagerImplementation& i_undoManagerImpl );
        ~UndoManagerHelper();

        // life time control
        void disposing();

        // XUndoManager equivalents
        void            enterUndoContext( const ::rtl::OUString& i_title, IClearableInstanceLock& i_instanceLock );
        void            enterHiddenUndoContext( IClearableInstanceLock& i_instanceLock );
        void            leaveUndoContext( IClearableInstanceLock& i_instanceLock );
        void            addUndoAction( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoAction >& i_action, IClearableInstanceLock& i_instanceLock );
        void            undo( IClearableInstanceLock& i_instanceLock );
        void            redo( IClearableInstanceLock& i_instanceLock );
        ::sal_Bool      isUndoPossible() const;
        ::sal_Bool      isRedoPossible() const;
        ::rtl::OUString getCurrentUndoActionTitle() const;
        ::rtl::OUString getCurrentRedoActionTitle() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        getAllUndoActionTitles() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        getAllRedoActionTitles() const;
        void            clear( IClearableInstanceLock& i_instanceLock );
        void            clearRedo( IClearableInstanceLock& i_instanceLock );
        void            reset( IClearableInstanceLock& i_instanceLock );
        void            addUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener );
        void            removeUndoManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManagerListener >& i_listener );

        // XLockable, base of XUndoManager, equivalents
        void            lock();
        void            unlock();
        ::sal_Bool      isLocked();

    private:
        ::boost::scoped_ptr< UndoManagerHelper_Impl >   m_pImpl;
    };

//......................................................................................................................
} // namespace framework
//......................................................................................................................

#endif // FRAMEWORK_UNDOMANAGERHELPER_HXX
