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
#include <com/sun/star/lang/XComponent.hpp>
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
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::IllegalArgumentException;
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
    //= UndoActionWrapper
    //==================================================================================================================
    class UndoActionWrapper : public SfxUndoAction
    {
    public:
                        UndoActionWrapper(
                            Reference< XUndoAction > const& i_undoAction
                        );
        virtual         ~UndoActionWrapper();

        virtual void    Undo();
        virtual void    Redo();
        virtual BOOL    CanRepeat(SfxRepeatTarget&) const;

    private:
        const Reference< XUndoAction >  m_xUndoAction;
    };

    //------------------------------------------------------------------------------------------------------------------
    UndoActionWrapper::UndoActionWrapper( Reference< XUndoAction > const& i_undoAction )
        :SfxUndoAction()
        ,m_xUndoAction( i_undoAction )
    {
        ENSURE_OR_THROW( m_xUndoAction.is(), "illegal undo action" );
    }

    //------------------------------------------------------------------------------------------------------------------
    UndoActionWrapper::~UndoActionWrapper()
    {
        try
        {
            Reference< XComponent > xComponent( m_xUndoAction, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoActionWrapper::Undo()
    {
        m_xUndoAction->undo();
    }

    //------------------------------------------------------------------------------------------------------------------
    void UndoActionWrapper::Redo()
    {
        m_xUndoAction->redo();
    }

    //------------------------------------------------------------------------------------------------------------------
    BOOL UndoActionWrapper::CanRepeat(SfxRepeatTarget&) const
    {
        return FALSE;
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
    void DocumentUndoManager::disposing()
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

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        rUndoManager.EnterListAction( i_title, ::rtl::OUString() );
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

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        if ( !rUndoManager.IsInListAction() )
            throw InvalidStateException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no active undo context" ) ),
                static_cast< XUndoManager* >( this )
            );

        rUndoManager.LeaveListAction();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::addUndoAction( const Reference< XUndoAction >& i_action ) throw (RuntimeException, IllegalArgumentException)
    {
        SfxModelGuard aGuard( *this );

        if ( !i_action.is() )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal undo action object" ) ),
                static_cast< XUndoManager* >( this ),
                1
            );

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        rUndoManager.AddUndoAction( new UndoActionWrapper( i_action ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::undo(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        rUndoManager.Undo();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::redo(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        rUndoManager.Redo();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clear(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        rUndoManager.Clear();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DocumentUndoManager::clearRedo(  ) throw (RuntimeException)
    {
        SfxModelGuard aGuard( *this );

        SfxUndoManager& rUndoManager = lcl_getUndoManager_throw( getBaseModel() );
        rUndoManager.ClearRedo();
    }


//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
