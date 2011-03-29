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

#include "precompiled_framework.hxx"

#include "framework/documentundoguard.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace framework
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::document::XUndoManagerSupplier;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::document::XUndoManagerListener;
    using ::com::sun::star::document::UndoManagerEvent;
    using ::com::sun::star::lang::EventObject;
    /** === end UNO using === **/

    //==================================================================================================================
    //= UndoManagerContextListener
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper1 <   XUndoManagerListener
                                    >   UndoManagerContextListener_Base;
    class UndoManagerContextListener : public UndoManagerContextListener_Base
    {
    public:
        UndoManagerContextListener( const Reference< XUndoManager >& i_undoManager )
            :m_xUndoManager( i_undoManager, UNO_QUERY_THROW )
            ,m_nRelativeContextDepth( 0 )
            ,m_documentDisposed( false )
        {
            osl_incrementInterlockedCount( &m_refCount );
            {
                m_xUndoManager->addUndoManagerListener( this );
            }
            osl_decrementInterlockedCount( &m_refCount );
        }

        UndoManagerContextListener()
        {
        }

        void finish()
        {
            OSL_ENSURE( m_nRelativeContextDepth >= 0, "UndoManagerContextListener: more contexts left than entered?" );

            if ( m_documentDisposed )
                return;

            // work with a copy of m_nRelativeContextDepth, to be independent from possible bugs in the
            // listener notifications (where it would be decremented with every leaveUndoContext)
            sal_Int32 nDepth = m_nRelativeContextDepth;
            while ( nDepth-- > 0 )
            {
                m_xUndoManager->leaveUndoContext();
            }
            m_xUndoManager->removeUndoManagerListener( this );
        }

        // XUndoManagerListener
        virtual void SAL_CALL undoActionAdded( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL actionUndone( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL actionRedone( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL allActionsCleared( const EventObject& i_event ) throw (RuntimeException);
        virtual void SAL_CALL redoActionsCleared( const EventObject& i_event ) throw (RuntimeException);
        virtual void SAL_CALL resetAll( const EventObject& i_event ) throw (RuntimeException);
        virtual void SAL_CALL enteredContext( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL enteredHiddenContext( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL leftContext( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL leftHiddenContext( const UndoManagerEvent& i_event ) throw (RuntimeException);
        virtual void SAL_CALL cancelledContext( const UndoManagerEvent& i_event ) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& i_event ) throw (RuntimeException);

    private:
        Reference< XUndoManager > const m_xUndoManager;
        oslInterlockedCount             m_nRelativeContextDepth;
        bool                            m_documentDisposed;
    };

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::undoActionAdded( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        // not interested in
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::actionUndone( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        // not interested in
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::actionRedone( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        // not interested in
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::allActionsCleared( const EventObject& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        // not interested in
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::redoActionsCleared( const EventObject& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        // not interested in
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::resetAll( const EventObject& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        m_nRelativeContextDepth = 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::enteredContext( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        osl_incrementInterlockedCount( &m_nRelativeContextDepth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::enteredHiddenContext( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        osl_incrementInterlockedCount( &m_nRelativeContextDepth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::leftContext( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        osl_decrementInterlockedCount( &m_nRelativeContextDepth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::leftHiddenContext( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        osl_decrementInterlockedCount( &m_nRelativeContextDepth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::cancelledContext( const UndoManagerEvent& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        osl_decrementInterlockedCount( &m_nRelativeContextDepth );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL UndoManagerContextListener::disposing( const EventObject& i_event ) throw (RuntimeException)
    {
        (void)i_event;
        m_documentDisposed = true;
    }

    //==================================================================================================================
    //= DocumentUndoGuard_Data
    //==================================================================================================================
    struct DocumentUndoGuard_Data
    {
        Reference< XUndoManager >                       xUndoManager;
        ::rtl::Reference< UndoManagerContextListener >  pContextListener;
    };

    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        void lcl_init( DocumentUndoGuard_Data& i_data, const Reference< XInterface >& i_undoSupplierComponent )
        {
            try
            {
                Reference< XUndoManagerSupplier > xUndoSupplier( i_undoSupplierComponent, UNO_QUERY );
                if ( xUndoSupplier.is() )
                    i_data.xUndoManager.set( xUndoSupplier->getUndoManager(), UNO_QUERY_THROW );

                if ( i_data.xUndoManager.is() )
                    i_data.pContextListener.set( new UndoManagerContextListener( i_data.xUndoManager ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        void lcl_restore( DocumentUndoGuard_Data& i_data )
        {
            try
            {
                if ( i_data.pContextListener.is() )
                    i_data.pContextListener->finish();
                i_data.pContextListener.clear();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //==================================================================================================================
    //= DocumentUndoGuard
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DocumentUndoGuard::DocumentUndoGuard( const Reference< XInterface >& i_undoSupplierComponent )
        :m_pData( new DocumentUndoGuard_Data )
    {
        lcl_init( *m_pData, i_undoSupplierComponent );
    }

    DocumentUndoGuard::~DocumentUndoGuard()
    {
        lcl_restore( *m_pData );
    }

//......................................................................................................................
} // namespace framework
//......................................................................................................................
