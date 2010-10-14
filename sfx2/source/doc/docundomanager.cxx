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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "docundomanager.hxx"
#include "sfx2/sfxbasemodel.hxx"
#include "sfx2/objsh.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svl/undo.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::util::InvalidStateException;
    using ::com::sun::star::document::XUndoAction;
    using ::com::sun::star::document::XUndoManagerSupplier;
    /** === end UNO using === **/

    //==================================================================================================================
    //= DocumentUndoManager_Data
    //==================================================================================================================
    struct DocumentUndoManager_Data
    {
        DocumentUndoManager_Data()
        {
        }
    };

    //==================================================================================================================
    namespace
    {
        SfxUndoManager& lcl_getUndoManager_throw( SfxBaseModel& i_document )
        {
            SfxObjectShell* pObjectShell = i_document.GetObjectShell();
            ENSURE_OR_THROW( pObjectShell != NULL, "internal error: disposal check should have happened before!" );
            SfxUndoManager* pUndoManager = pObjectShell->GetUndoManager();
            if ( pUndoManager == NULL )
            {
                OSL_ENSURE( false, "lcl_getUndoManager_throw: no UndoManager at the shell!" );
                throw RuntimeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "internal error: the object shell is expected to provide an UndoManager!" ) ),
                    static_cast< XUndoManagerSupplier* >( &i_document ) );
            }
            return *pUndoManager;
        }
    }

    //==================================================================================================================
    //= DocumentUndoManager
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DocumentUndoManager::DocumentUndoManager( SfxBaseModel& i_document )
        :SfxModelSubComponent( i_document )
        ,m_pData( new DocumentUndoManager_Data )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DocumentUndoManager::~DocumentUndoManager()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void DocumentUndoManager::dispose()
    {
        // TODO?
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::acquire(  ) throw ()
    {
        SfxModelSubComponent::acquire();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::release(  ) throw ()
    {
        SfxModelSubComponent::release();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::enterUndoContext( const ::rtl::OUString& i_title ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );

        (void)i_title;
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::enterHiddenUndoContext(  ) throw (InvalidStateException, RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::leaveUndoContext(  ) throw (InvalidStateException, RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::addUndoAction( const Reference< XUndoAction >& i_action ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        (void)i_action;
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::undo(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::redo(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clear(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        // TODO: place your code here
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clearRedo(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );
        // TODO: place your code here
    }


//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
