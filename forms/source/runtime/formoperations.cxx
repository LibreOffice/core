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

#include <config_features.h>

#include "formoperations.hxx"
#include "frm_strings.hxx"
#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "services.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/form/runtime/FormFeature.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/form/XGrid.hpp>
#include <com/sun/star/form/XBoundControl.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#include <com/sun/star/sdb/RowChangeEvent.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/sdb/OrderDialog.hpp>
#include <com/sun/star/sdb/FilterDialog.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/container.hxx>
#include <comphelper/property.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <sal/macros.h>


namespace frm
{


    using ::dbtools::SQLExceptionInfo;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::sdbc::XResultSetUpdate;
    using ::com::sun::star::form::runtime::XFormController;
    using ::com::sun::star::form::runtime::XFormOperations;
    using ::com::sun::star::form::runtime::XFeatureInvalidation;
    using ::com::sun::star::form::runtime::FeatureState;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::sdbc::SQLException;
    using namespace ::com::sun::star::sdbc;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::util::XModifyBroadcaster;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::form::XGrid;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::form::XBoundControl;
    using ::com::sun::star::form::XBoundComponent;
    using ::com::sun::star::sdbcx::XRowLocate;
    using ::com::sun::star::form::XConfirmDeleteListener;
    using ::com::sun::star::sdb::RowChangeEvent;
    using namespace ::com::sun::star::sdb;
    using ::com::sun::star::form::XReset;
    using ::com::sun::star::beans::XMultiPropertySet;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::ui::dialogs::XExecutableDialog;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::util::XRefreshable;
    using ::com::sun::star::awt::XControlModel;

    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;
    namespace RowChangeAction = ::com::sun::star::sdb::RowChangeAction;

    FormOperations::FormOperations( const Reference< XComponentContext >& _rxContext )
        :FormOperations_Base( m_aMutex )
        ,m_xContext( _rxContext )
        ,m_bInitializedParser( false )
        ,m_bActiveControlModified( false )
        ,m_bConstructed( false )
    #ifdef DBG_UTIL
        ,m_nMethodNestingLevel( 0 )
    #endif
    {
    }

    FormOperations::~FormOperations()
    {
    }

    OUString FormOperations::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return OUString( "com.sun.star.comp.forms.FormOperations" );
    }

    Sequence< OUString > FormOperations::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aNames { "com.sun.star.form.runtime.FormOperations" };
        return aNames;
    }

    void SAL_CALL FormOperations::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException, std::exception)
    {
        if ( m_bConstructed )
            throw AlreadyInitializedException();

        if ( _arguments.getLength() == 1 )
        {
            Reference< XFormController > xController;
            Reference< XForm > xForm;
            if ( _arguments[0] >>= xController )
                createWithFormController( xController );
            else if ( _arguments[0] >>= xForm )
                createWithForm( xForm );
            else
                throw IllegalArgumentException( OUString(), *this, 1 );
            return;
        }

        throw IllegalArgumentException( OUString(), *this, 0 );
    }

    OUString SAL_CALL FormOperations::getImplementationName(  ) throw (RuntimeException, std::exception)
    {
        return getImplementationName_Static();
    }

    sal_Bool SAL_CALL FormOperations::supportsService( const OUString& _ServiceName ) throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _ServiceName);
    }

    Sequence< OUString > SAL_CALL FormOperations::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
    {
        return getSupportedServiceNames_Static();
    }

    Reference< XRowSet > SAL_CALL FormOperations::getCursor() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        return m_xCursor;
    }

    Reference< XResultSetUpdate > SAL_CALL FormOperations::getUpdateCursor() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        return m_xUpdateCursor;
    }


    Reference< XFormController > SAL_CALL FormOperations::getController() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        return m_xController;
    }


    Reference< XFeatureInvalidation > SAL_CALL FormOperations::getFeatureInvalidation() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        return m_xFeatureInvalidation;
    }


    void SAL_CALL FormOperations::setFeatureInvalidation( const Reference< XFeatureInvalidation > & _rxFeatureInvalidation ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        m_xFeatureInvalidation = _rxFeatureInvalidation;
    }


    FeatureState SAL_CALL FormOperations::getState( ::sal_Int16 _nFeature ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );

        FeatureState aState;
        aState.Enabled = sal_False;

        try
        {
            // some checks for basic pre-requisites
            if  (   !m_xLoadableForm.is()
                ||  !m_xLoadableForm->isLoaded()
                ||  !m_xCursorProperties.is()
                )
            {
                return aState;
            }

            switch ( _nFeature )
            {
            case FormFeature::MoveToFirst:
            case FormFeature::MoveToPrevious:
                aState.Enabled = impl_canMoveLeft_throw( );
                break;

            case FormFeature::MoveToNext:
                aState.Enabled = impl_canMoveRight_throw();
                break;

            case FormFeature::MoveToLast:
                aState.Enabled = impl_getRowCount_throw() && ( !m_xCursor->isLast() || impl_isInsertionRow_throw() );
                break;

            case FormFeature::DeleteRecord:
                // already deleted ?
                if ( m_xCursor->rowDeleted() )
                    aState.Enabled = sal_False;
                else
                {
                    // allowed to delete the row ?
                    aState.Enabled = !impl_isInsertionRow_throw() && ::dbtools::canDelete( m_xCursorProperties );
                }
                break;

            case FormFeature::MoveToInsertRow:
                // if we are inserting we can move to the next row if the current record or control is modified
                aState.Enabled =    impl_isInsertionRow_throw()
                                ?   impl_isModifiedRow_throw() || m_bActiveControlModified
                                :   ::dbtools::canInsert( m_xCursorProperties );
                break;

            case FormFeature::ReloadForm:
            {
                // there must be an active connection
                Reference< XRowSet > xCursorRowSet( m_xCursor, UNO_QUERY );
                aState.Enabled = ::dbtools::getConnection( xCursorRowSet ).is();

                // and an active command
                OUString sActiveCommand;
                m_xCursorProperties->getPropertyValue( PROPERTY_ACTIVECOMMAND ) >>= sActiveCommand;
                aState.Enabled = aState.Enabled && !sActiveCommand.isEmpty();
            }
            break;

            case FormFeature::RefreshCurrentControl:
            {
                Reference< XRefreshable > xControlModelRefresh( impl_getCurrentControlModel_throw(), UNO_QUERY );
                aState.Enabled = xControlModelRefresh.is();
            }
            break;

            case FormFeature::SaveRecordChanges:
            case FormFeature::UndoRecordChanges:
                aState.Enabled = impl_isModifiedRow_throw() || m_bActiveControlModified;
                break;

            case FormFeature::RemoveFilterAndSort:
                if ( impl_isParseable_throw() && impl_hasFilterOrOrder_throw() )
                    aState.Enabled = !impl_isInsertOnlyForm_throw();
                break;

            case FormFeature::SortAscending:
            case FormFeature::SortDescending:
            case FormFeature::AutoFilter:
                if ( m_xController.is() && impl_isParseable_throw() )
                {
                    bool bIsDeleted = m_xCursor->rowDeleted();

                    if ( !bIsDeleted && !impl_isInsertOnlyForm_throw() )
                    {
                        Reference< XPropertySet > xBoundField = impl_getCurrentBoundField_nothrow( );
                        if ( xBoundField.is() )
                            xBoundField->getPropertyValue( PROPERTY_SEARCHABLE ) >>= aState.Enabled;
                    }
                }
                break;

            case FormFeature::InteractiveSort:
            case FormFeature::InteractiveFilter:
                if ( impl_isParseable_throw() )
                    aState.Enabled = !impl_isInsertOnlyForm_throw();
                break;

            case FormFeature::ToggleApplyFilter:
            {
                OUString sFilter;
                m_xCursorProperties->getPropertyValue( PROPERTY_FILTER ) >>= sFilter;
                if ( !sFilter.isEmpty() )
                {
                    aState.State = m_xCursorProperties->getPropertyValue( PROPERTY_APPLYFILTER );
                    aState.Enabled = !impl_isInsertOnlyForm_throw();
                }
                else
                    aState.State <<= false;
            }
            break;

            case FormFeature::MoveAbsolute:
            {
                sal_Int32 nPosition   = m_xCursor->getRow();
                bool  bIsNew      = impl_isInsertionRow_throw();
                sal_Int32 nCount      = impl_getRowCount_throw();
                bool  bFinalCount = impl_isRowCountFinal_throw();

                if ( ( nPosition >= 0 ) || bIsNew )
                {
                    if ( bFinalCount )
                    {
                        // special case: there are no records at all, and we
                        // can't insert records -> disabled
                        if ( !nCount && !::dbtools::canInsert( m_xCursorProperties ) )
                        {
                            aState.Enabled = sal_False;
                        }
                        else
                        {
                            if ( bIsNew )
                                nPosition = ++nCount;
                            aState.State <<= (sal_Int32)nPosition;
                            aState.Enabled = sal_True;
                        }
                    }
                    else
                    {
                        aState.State <<= (sal_Int32)nPosition;
                        aState.Enabled = sal_True;
                    }
                }
            }
            break;

            case FormFeature::TotalRecords:
            {
                bool  bIsNew      = impl_isInsertionRow_throw();
                sal_Int32 nCount      = impl_getRowCount_throw();
                bool  bFinalCount = impl_isRowCountFinal_throw();

                if ( bIsNew )
                    ++nCount;

                OUString sValue = OUString::number( nCount );
                if ( !bFinalCount )
                    sValue += " *";

                aState.State <<= sValue;
                aState.Enabled = sal_True;
            }
            break;

            default:
                OSL_FAIL( "FormOperations::getState: unknown feature id!" );
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormOperations::getState: caught an exception!" );
        }

        return aState;
    }


    sal_Bool SAL_CALL FormOperations::isEnabled( ::sal_Int16 _nFeature ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );

        FeatureState aState( getState( _nFeature ) );
        return aState.Enabled;
    }


    namespace
    {
        static bool lcl_needConfirmCommit( sal_Int32 _nFeature )
        {
            return ( ( _nFeature == FormFeature::ReloadForm )
                  || ( _nFeature == FormFeature::RemoveFilterAndSort )
                  || ( _nFeature == FormFeature::ToggleApplyFilter )
                  || ( _nFeature == FormFeature::SortAscending )
                  || ( _nFeature == FormFeature::SortDescending )
                  || ( _nFeature == FormFeature::AutoFilter )
                  || ( _nFeature == FormFeature::InteractiveSort )
                  || ( _nFeature == FormFeature::InteractiveFilter )
                   );
        }
        static bool lcl_requiresArguments( sal_Int32 _nFeature )
        {
            return ( _nFeature == FormFeature::MoveAbsolute );
        }
        static bool lcl_isExecutableFeature( sal_Int32 _nFeature )
        {
            return ( _nFeature != FormFeature::TotalRecords );
        }

        template < typename TYPE >
        TYPE lcl_safeGetPropertyValue_throw( const Reference< XPropertySet >& _rxProperties, const OUString& _rPropertyName, TYPE _Default )
        {
            TYPE value( _Default );
            OSL_PRECOND( _rxProperties.is(), "FormOperations::<foo>: no cursor (already disposed?)!" );
            if ( _rxProperties.is() )
                OSL_VERIFY( _rxProperties->getPropertyValue( _rPropertyName ) >>= value );
            return value;
        }

        // returns false if parent should *abort* (user pressed cancel)
        bool checkConfirmation(bool &needConfirmation, bool &shouldCommit)
        {
            if(needConfirmation)
            {
                // TODO: shouldn't this be done with an interaction handler?
                ScopedVclPtrInstance< QueryBox > aQuery( nullptr, WB_YES_NO_CANCEL | WB_DEF_YES, FRM_RES_STRING( RID_STR_QUERY_SAVE_MODIFIED_ROW ) );
                switch ( aQuery->Execute() )
                {
                case RET_NO:
                    shouldCommit = false;
                    SAL_FALLTHROUGH; // don't ask again!
                case RET_YES:
                    needConfirmation = false;
                    return true;
                case RET_CANCEL:
                    return false;
                }
            }
            return true;
        }

        bool commit1Form(Reference< XFormController > xCntrl, bool &needConfirmation, bool &shouldCommit)
        {
            Reference< XFormOperations > xFrmOps(xCntrl->getFormOperations());
            if (!xFrmOps->commitCurrentControl())
                return false;

            if(xFrmOps->isModifiedRow())
            {
                if(!checkConfirmation(needConfirmation, shouldCommit))
                    return false;
                sal_Bool bTmp;
                if (shouldCommit && !xFrmOps->commitCurrentRecord(bTmp))
                    return false;
            }
            return true;
        }

        bool commitFormAndSubforms(Reference< XFormController > xCntrl, bool needConfirmation)
        {
            bool shouldCommit(true);
            assert(xCntrl.is());
            Reference< XIndexAccess > xSubForms(xCntrl, UNO_QUERY);
            assert(xSubForms.is());
            if(xSubForms.is())
            {
                const sal_Int32 cnt = xSubForms->getCount();
                for(int i=0; i < cnt; ++i)
                {
                    Reference< XFormController > xSubForm(xSubForms->getByIndex(i), UNO_QUERY);
                    assert(xSubForm.is());
                    if (xSubForm.is())
                    {
                        if (!commit1Form(xSubForm, needConfirmation, shouldCommit))
                            return false;
                    }
                }
            }

            if(!commit1Form(xCntrl, needConfirmation, shouldCommit))
                return false;

            return true;
        }

        bool commit1Form(Reference< XForm > xFrm, bool &needConfirmation, bool &shouldCommit)
        {
            Reference< XPropertySet > xProps(xFrm, UNO_QUERY_THROW);
            // nothing to do if the record is not modified
            if(!lcl_safeGetPropertyValue_throw( xProps, PROPERTY_ISMODIFIED, false ))
                return true;

            if(!checkConfirmation(needConfirmation, shouldCommit))
                return false;
            if(shouldCommit)
            {
                Reference< XResultSetUpdate > xUpd(xFrm, UNO_QUERY_THROW);
                // insert respectively update the row
                if ( lcl_safeGetPropertyValue_throw( xProps, PROPERTY_ISNEW, false ) )
                    xUpd->insertRow();
                else
                    xUpd->updateRow();
            }
            return true;
        }

        bool commitFormAndSubforms(Reference< XForm > xFrm, bool needConfirmation)
        {
            // No control...  do what we can with the models
            bool shouldCommit(true);
            Reference< XIndexAccess > xFormComps(xFrm, UNO_QUERY_THROW);
            assert( xFormComps.is() );

            const sal_Int32 cnt = xFormComps->getCount();
            for(int i=0; i < cnt; ++i)
            {
                Reference< XForm > xSubForm(xFormComps->getByIndex(i), UNO_QUERY);
                if(xSubForm.is())
                {
                    if(!commit1Form(xSubForm, needConfirmation, shouldCommit))
                        return false;
                }
            }

            if(!commit1Form(xFrm, needConfirmation, shouldCommit))
                return false;

            return true;
        }
    }

    void SAL_CALL FormOperations::execute( ::sal_Int16 _nFeature ) throw (RuntimeException, IllegalArgumentException, SQLException, WrappedTargetException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        MethodGuard aGuard( *this );

        if ( ( _nFeature != FormFeature::DeleteRecord ) && ( _nFeature != FormFeature::UndoRecordChanges ) )
        {


            if(m_xController.is())
            {
                if(!commitFormAndSubforms(m_xController, lcl_needConfirmCommit( _nFeature )))
                    return;
            }
            else if(m_xCursor.is())
            {
                Reference< XForm > xForm(m_xCursor, UNO_QUERY);
                assert(xForm.is());
                if(!commitFormAndSubforms(xForm, lcl_needConfirmCommit( _nFeature )))
                    return;
            }
            else
            {
                SAL_WARN( "forms.runtime", "No cursor, but trying to execute form operation " << _nFeature );
            }
        }

        try
        {
            switch ( _nFeature )
            {
            case FormFeature::MoveToFirst:
                m_xCursor->first();
                break;

            case FormFeature::MoveToNext:
                impl_moveRight_throw( );
                break;

            case FormFeature::MoveToPrevious:
                impl_moveLeft_throw( );
                break;

            case FormFeature::MoveToLast:
            {
/*
                // TODO: re-implement this .....
                // run in an own thread if ...
                // ... the data source is thread safe ...
                sal_Bool bAllowOwnThread = sal_False;
                if ( ::comphelper::hasProperty( PROPERTY_THREADSAFE, m_xCursorProperties ) )
                    m_xCursorProperties->getPropertyValue( PROPERTY_THREADSAFE ) >>= bAllowOwnThread;

                // ... the record count is unknown
                sal_Bool bNeedOwnThread sal_False;
                if ( ::comphelper::hasProperty( PROPERTY_ROWCOUNTFINAL, m_xCursorProperties ) )
                    m_xCursorProperties->getPropertyValue( PROPERTY_ROWCOUNTFINAL ) >>= bNeedOwnThread;

                if ( bNeedOwnThread && bAllowOwnThread )
                    ;
                else
*/
                    m_xCursor->last();
            }
            break;

            case FormFeature::ReloadForm:
                if ( m_xLoadableForm.is() )
                {
                    WaitObject aWO( nullptr );
                    m_xLoadableForm->reload();

                    // refresh all controls in the form (and sub forms) which can be refreshed
                    // #i90914#
                    ::comphelper::IndexAccessIterator aIter( m_xLoadableForm );
                    Reference< XInterface > xElement( aIter.Next() );
                    while ( xElement.is() )
                    {
                        Reference< XRefreshable > xRefresh( xElement, UNO_QUERY );
                        if ( xRefresh.is() )
                            xRefresh->refresh();
                        xElement = aIter.Next();
                    }
                }
                break;

            case FormFeature::RefreshCurrentControl:
            {
                Reference< XRefreshable > xControlModelRefresh( impl_getCurrentControlModel_throw(), UNO_QUERY );
                OSL_ENSURE( xControlModelRefresh.is(), "FormOperations::execute: how did you reach this?" );
                if ( xControlModelRefresh.is() )
                    xControlModelRefresh->refresh();
            }
            break;

            case FormFeature::DeleteRecord:
            {
                sal_uInt32 nCount = impl_getRowCount_throw();

                // next position
                bool bLeft = m_xCursor->isLast() && ( nCount > 1 );
                bool bRight= !m_xCursor->isLast();
                bool bSuccess = false;
                try
                {
                    // ask for confirmation
                    Reference< XConfirmDeleteListener > xConfirmDelete( m_xController, UNO_QUERY );

                    if ( xConfirmDelete.is() )
                    {
                        RowChangeEvent aEvent;
                        aEvent.Source.set( m_xCursor, UNO_QUERY );
                        aEvent.Action = RowChangeAction::DELETE;
                        aEvent.Rows = 1;
                        bSuccess = xConfirmDelete->confirmDelete( aEvent );
                    }

                    // delete it
                    if ( bSuccess )
                        m_xUpdateCursor->deleteRow();
                }
                catch( const Exception& )
                {
                    bSuccess = false;
                }

                if ( bSuccess )
                {
                    if ( bLeft || bRight )
                        m_xCursor->relative( bRight ? 1 : -1 );
                    else
                    {
                        bool bCanInsert = ::dbtools::canInsert( m_xCursorProperties );
                        // is it possible to insert another record?
                        if ( bCanInsert )
                            m_xUpdateCursor->moveToInsertRow();
                        else
                            // move record to update status
                            m_xCursor->first();
                    }
                }
            }
            break;

            case FormFeature::SaveRecordChanges:
            case FormFeature::UndoRecordChanges:
            {
                bool bInserting = impl_isInsertionRow_throw();

                if ( FormFeature::UndoRecordChanges == _nFeature )
                {
                    if ( !bInserting )
                        m_xUpdateCursor->cancelRowUpdates();

                    // reset all controls for this form
                    impl_resetAllControls_nothrow( );

                    if ( bInserting )   // back to insertion mode for this form
                        m_xUpdateCursor->moveToInsertRow();
                }
                else
                {
                    if  ( bInserting )
                    {
                        m_xUpdateCursor->insertRow();
                        m_xCursor->last();
                    }
                    else
                        m_xUpdateCursor->updateRow();
                }
            }
            break;

            case FormFeature::MoveToInsertRow:
                // move to the last row before moving to the insert row
                m_xCursor->last();
                m_xUpdateCursor->moveToInsertRow();
                break;

            case FormFeature::RemoveFilterAndSort:
            {
                // simultaneously reset Filter and Order property
                Reference< XMultiPropertySet > xProperties( m_xCursorProperties, UNO_QUERY );
                OSL_ENSURE( xProperties.is(), "FormOperations::execute: no multi property access!" );
                if ( xProperties.is() )
                {
                    Sequence< OUString > aNames( 2 );
                    aNames[0] = PROPERTY_FILTER;
                    aNames[1] = PROPERTY_SORT;

                    Sequence< Any> aValues( 2 );
                    aValues[0] <<= OUString();
                    aValues[1] <<= OUString();

                    WaitObject aWO( nullptr );
                    xProperties->setPropertyValues( aNames, aValues );

                    if ( m_xLoadableForm.is() )
                        m_xLoadableForm->reload();
                }
            }
            break;

            case FormFeature::ToggleApplyFilter:
                if ( impl_commitCurrentControl_throw() && impl_commitCurrentRecord_throw() )
                {
                    // simply toggle the value
                    bool bApplied = false;
                    m_xCursorProperties->getPropertyValue( PROPERTY_APPLYFILTER ) >>= bApplied;
                    m_xCursorProperties->setPropertyValue( PROPERTY_APPLYFILTER, makeAny( !bApplied ) );

                    // and reload
                    WaitObject aWO( nullptr );
                    m_xLoadableForm->reload();
                }
                break;

            case FormFeature::SortAscending:
                impl_executeAutoSort_throw( true );
                break;

            case FormFeature::SortDescending:
                impl_executeAutoSort_throw( false );
                break;

            case FormFeature::AutoFilter:
                impl_executeAutoFilter_throw();
                break;

            case FormFeature::InteractiveSort:
                impl_executeFilterOrSort_throw( false );
                break;

            case FormFeature::InteractiveFilter:
                impl_executeFilterOrSort_throw( true );
                break;

            default:
            {
                sal_uInt16 nErrorResourceId = RID_STR_FEATURE_UNKNOWN;
                if ( lcl_requiresArguments( _nFeature ) )
                    nErrorResourceId = RID_STR_FEATURE_REQUIRES_PARAMETERS;
                else if ( !lcl_isExecutableFeature( _nFeature ) )
                    nErrorResourceId = RID_STR_FEATURE_NOT_EXECUTABLE;
                throw IllegalArgumentException( FRM_RES_STRING( nErrorResourceId ), *this, 1 );
            }
            }   // switch
        }
        catch( const RuntimeException& ) { throw; }
        catch( const SQLException& ) { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( OUString(), *this, ::cppu::getCaughtException() );
        }

        impl_invalidateAllSupportedFeatures_nothrow( aGuard );
    }


    void SAL_CALL FormOperations::executeWithArguments( ::sal_Int16 _nFeature, const Sequence< NamedValue >& _rArguments ) throw (RuntimeException, IllegalArgumentException, SQLException, WrappedTargetException, std::exception)
    {
        if ( !lcl_requiresArguments( _nFeature ) )
        {
            execute( _nFeature );
            return;
        }

        SolarMutexGuard aSolarGuard;
        MethodGuard aGuard( *this );

        // at the moment we have only one feature which supports execution parameters
        if ( !lcl_isExecutableFeature( _nFeature ) )
            throw IllegalArgumentException( FRM_RES_STRING( RID_STR_FEATURE_NOT_EXECUTABLE ), *this, 1 );

        switch ( _nFeature )
        {
        case FormFeature::MoveAbsolute:
        {
            sal_Int32 nPosition = -1;

            ::comphelper::NamedValueCollection aArguments( _rArguments );
            aArguments.get_ensureType( "Position", nPosition );

            if ( nPosition < 1 )
                nPosition = 1;

            try
            {
                // commit before doing anything else
                if ( m_xController.is() && !impl_commitCurrentControl_throw() )
                    return;
                if ( !impl_commitCurrentRecord_throw() )
                    return;

                sal_Int32 nCount      = impl_getRowCount_throw();
                bool  bFinalCount = impl_isRowCountFinal_throw();

                if ( bFinalCount && ( (sal_Int32)nPosition > nCount ) )
                    nPosition = nCount;

                m_xCursor->absolute( nPosition );
            }
            catch( const RuntimeException& ) { throw; }
            catch( const SQLException& ) { throw; }
            catch( const Exception& )
            {
                throw WrappedTargetException( OUString(), *this, ::cppu::getCaughtException() );
            }
        }
        break;
        default:
            throw IllegalArgumentException( FRM_RES_STRING( RID_STR_FEATURE_UNKNOWN ), *this, 1 );
        }   // switch
    }


    sal_Bool SAL_CALL FormOperations::commitCurrentRecord( sal_Bool& _out_rRecordInserted ) throw (RuntimeException, SQLException, std::exception)
    {
        MethodGuard aGuard( *this );
        _out_rRecordInserted = sal_False;

        return impl_commitCurrentRecord_throw( &_out_rRecordInserted );
    }


    bool FormOperations::impl_commitCurrentRecord_throw( sal_Bool* _pRecordInserted ) const
    {
#ifdef DBG_UTIL
        DBG_ASSERT( m_nMethodNestingLevel, "FormOperations::impl_commitCurrentRecord_throw: to be called within a MethodGuard'ed section only!" );
#endif

        if ( !impl_hasCursor_nothrow() )
            return false;

        // nothing to do if the record is not modified
        bool bResult = !impl_isModifiedRow_throw();
        if ( !bResult )
        {
            // insert respectively update the row
            if ( impl_isInsertionRow_throw() )
            {
                m_xUpdateCursor->insertRow();
                if ( _pRecordInserted )
                    *_pRecordInserted = sal_True;
            }
            else
                m_xUpdateCursor->updateRow();
            bResult = true;
        }
        return bResult;
    }


    sal_Bool SAL_CALL FormOperations::commitCurrentControl() throw (RuntimeException, SQLException, std::exception)
    {
        MethodGuard aGuard( *this );
        return impl_commitCurrentControl_throw();
    }


    bool FormOperations::impl_commitCurrentControl_throw() const
    {
#ifdef DBG_UTIL
        DBG_ASSERT( m_nMethodNestingLevel, "FormOperations::impl_commitCurrentControl_throw: to be called within a MethodGuard'ed section only!" );
#endif
        OSL_PRECOND( m_xController.is(), "FormOperations::commitCurrentControl: no controller!" );
        if ( !m_xController.is() )
            return false;

        bool bSuccess = false;
        try
        {
            Reference< XControl > xCurrentControl( m_xController->getCurrentControl() );

            // check whether the control is locked
            Reference< XBoundControl > xCheckLock( xCurrentControl, UNO_QUERY );
            bool bControlIsLocked = xCheckLock.is() && xCheckLock->getLock();

            // commit if necessary
            bSuccess = true;
            if ( xCurrentControl.is() && !bControlIsLocked )
            {
                // both the control and its model can be committable, so try both
                Reference< XBoundComponent > xBound( xCurrentControl, UNO_QUERY );
                if ( !xBound.is() )
                    xBound.set(xCurrentControl->getModel(), css::uno::UNO_QUERY);
                // and now really commit
                if ( xBound.is() )
                    bSuccess = xBound->commit();
            }

        }
        catch( const RuntimeException& ) { throw; }
        catch( const SQLException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            bSuccess = false;
        }

        return bSuccess;
    }


    sal_Bool SAL_CALL FormOperations::isInsertionRow() throw (RuntimeException, WrappedTargetException, std::exception)
    {
        bool bIs = false;
        try
        {
            bIs = impl_isInsertionRow_throw();
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( OUString(), *this, ::cppu::getCaughtException() );
        }
        return bIs;
    }


    sal_Bool SAL_CALL FormOperations::isModifiedRow() throw (RuntimeException, WrappedTargetException, std::exception)
    {
        bool bIs = false;
        try
        {
            bIs = impl_isModifiedRow_throw();
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( OUString(), *this, ::cppu::getCaughtException() );
        }
        return bIs;
    }


    void SAL_CALL FormOperations::cursorMoved( const EventObject& /*_Event*/ ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        m_bActiveControlModified = false;

        impl_invalidateAllSupportedFeatures_nothrow( aGuard );
    }


    void SAL_CALL FormOperations::rowChanged( const EventObject& /*_Event*/ ) throw (RuntimeException, std::exception)
    {
        // not interested in
    }


    void SAL_CALL FormOperations::rowSetChanged( const EventObject& /*_Event*/ ) throw (RuntimeException, std::exception)
    {
        // not interested in
    }


    void SAL_CALL FormOperations::modified( const EventObject& /*_Source*/ ) throw( RuntimeException, std::exception )
    {
        MethodGuard aGuard( *this );

        OSL_ENSURE( m_xCursor.is(), "FormOperations::modified: already disposed!" );
        if ( !m_bActiveControlModified )
        {
            m_bActiveControlModified = true;
            impl_invalidateModifyDependentFeatures_nothrow( aGuard );
        }
    }


    void SAL_CALL FormOperations::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );

        if ( m_xCursor.is() && ( m_xCursor == _rEvent.Source ) )
        {
            if  ( ( _rEvent.PropertyName == PROPERTY_ISMODIFIED )
               || ( _rEvent.PropertyName == PROPERTY_ISNEW )
                )
            {
                bool bIs = false;
                if ( ( _rEvent.NewValue >>= bIs ) && !bIs )
                    m_bActiveControlModified = false;
            }
            impl_invalidateAllSupportedFeatures_nothrow( aGuard );
        }

        if ( m_xParser.is() && ( m_xCursor == _rEvent.Source ) )
        {
            try
            {
                OUString sNewValue;
                _rEvent.NewValue >>= sNewValue;
                if ( _rEvent.PropertyName == PROPERTY_ACTIVECOMMAND )
                {
                    m_xParser->setElementaryQuery( sNewValue );
                }
                else if ( _rEvent.PropertyName == PROPERTY_FILTER )
                {
                    if ( m_xParser->getFilter() != sNewValue )
                        m_xParser->setFilter( sNewValue );
                }
                else if ( _rEvent.PropertyName == PROPERTY_SORT )
                {
                    _rEvent.NewValue >>= sNewValue;
                    if ( m_xParser->getOrder() != sNewValue )
                        m_xParser->setOrder( sNewValue );
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "FormOperations::propertyChange: caught an exception while updating the parser!" );
            }
            impl_invalidateAllSupportedFeatures_nothrow( aGuard );
        }
    }


    void SAL_CALL FormOperations::disposing( const EventObject& /*_Source*/ ) throw (RuntimeException, std::exception)
    {
        // TODO: should we react on this? Or is this the responsibility of our owner to dispose us?
    }


    void SAL_CALL FormOperations::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        impl_disposeParser_nothrow();

        try
        {
            // revoke various listeners
            if ( m_xCursor.is() )
                m_xCursor->removeRowSetListener( this );

            if ( m_xCursorProperties.is() )
            {
                m_xCursorProperties->removePropertyChangeListener( PROPERTY_ISMODIFIED,this );
                m_xCursorProperties->removePropertyChangeListener( PROPERTY_ISNEW, this );
            }

            Reference< XModifyBroadcaster > xBroadcaster( m_xController, UNO_QUERY );
            if ( xBroadcaster.is() )
                xBroadcaster->removeModifyListener( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_xController.clear();
        m_xCursor.clear();
        m_xUpdateCursor.clear();
        m_xCursorProperties.clear();
        m_xLoadableForm.clear();
        m_xFeatureInvalidation.clear();

        m_bActiveControlModified = true;
    }


    void FormOperations::impl_checkDisposed_throw() const
    {
        if ( impl_isDisposed_nothrow() )
            throw DisposedException( OUString(), *const_cast< FormOperations* >( this ) );
    }


    void FormOperations::impl_initFromController_throw()
    {
        OSL_PRECOND( m_xController.is(), "FormOperations::impl_initFromController_throw: invalid controller!" );
        m_xCursor.set(m_xController->getModel(), css::uno::UNO_QUERY);
        if ( !m_xCursor.is() )
            throw IllegalArgumentException( OUString(), *this, 0 );

        impl_initFromForm_throw();

        Reference< XModifyBroadcaster > xBroadcaster( m_xController, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->addModifyListener( this );
    }


    void FormOperations::impl_initFromForm_throw()
    {
        OSL_PRECOND( m_xCursor.is(), "FormOperations::impl_initFromForm_throw: invalid form!" );
        m_xCursorProperties.set(m_xCursor, css::uno::UNO_QUERY);
        m_xUpdateCursor.set(m_xCursor, css::uno::UNO_QUERY);
        m_xLoadableForm.set(m_xCursor, css::uno::UNO_QUERY);

        if ( !m_xCursor.is() || !m_xCursorProperties.is() || !m_xLoadableForm.is() )
            throw IllegalArgumentException( OUString(), *this, 0 );

        m_xCursor->addRowSetListener( this );
        m_xCursorProperties->addPropertyChangeListener( PROPERTY_ISMODIFIED,this );
        m_xCursorProperties->addPropertyChangeListener( PROPERTY_ISNEW, this );
    }


    void FormOperations::createWithFormController( const Reference< XFormController >& _rxController )
    {
        m_xController = _rxController;
        if ( !m_xController.is() )
            throw IllegalArgumentException( OUString(), *this, 0 );

        impl_initFromController_throw();

        m_bConstructed = true;
    }


    void FormOperations::createWithForm( const Reference< XForm >& _rxForm )
    {
        m_xCursor.set(_rxForm, css::uno::UNO_QUERY);
        if ( !m_xCursor.is() )
            throw IllegalArgumentException( OUString(), *this, 0 );

        impl_initFromForm_throw();

        m_bConstructed = true;
    }


    void FormOperations::impl_invalidateAllSupportedFeatures_nothrow( MethodGuard& _rClearForCallback ) const
    {
        if ( !m_xFeatureInvalidation.is() )
            // nobody's interested in ...
            return;

        Reference< XFeatureInvalidation > xInvalidation = m_xFeatureInvalidation;
        _rClearForCallback.clear();
        xInvalidation->invalidateAllFeatures();
    }


    void FormOperations::impl_invalidateModifyDependentFeatures_nothrow( MethodGuard& _rClearForCallback ) const
    {
        if ( !m_xFeatureInvalidation.is() )
            // nobody's interested in ...
            return;

        static Sequence< sal_Int16 > s_aModifyDependentFeatures;
        if ( s_aModifyDependentFeatures.getLength() == 0 )
        {
            sal_Int16 pModifyDependentFeatures[] =
            {
                FormFeature::MoveToNext,
                FormFeature::MoveToInsertRow,
                FormFeature::SaveRecordChanges,
                FormFeature::UndoRecordChanges
            };
            size_t nFeatureCount = SAL_N_ELEMENTS( pModifyDependentFeatures );
            s_aModifyDependentFeatures = Sequence< sal_Int16 >( pModifyDependentFeatures, nFeatureCount );
        }

        Reference< XFeatureInvalidation > xInvalidation = m_xFeatureInvalidation;
        _rClearForCallback.clear();

        xInvalidation->invalidateFeatures( s_aModifyDependentFeatures );
    }


    void FormOperations::impl_ensureInitializedParser_nothrow()
    {
        OSL_PRECOND( m_xCursorProperties.is(), "FormOperations::impl_ensureInitializedParser_nothrow: we're disposed!" );
        if ( m_bInitializedParser )
            return;

        try
        {
            bool bUseEscapeProcessing = false;
            m_xCursorProperties->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bUseEscapeProcessing;
            if ( bUseEscapeProcessing )
            {
                Reference< XMultiServiceFactory > xFactory( ::dbtools::getConnection( m_xCursor ), UNO_QUERY );
                if ( xFactory.is() )
                {
                    m_xParser.set( xFactory->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"), UNO_QUERY );
                    OSL_ENSURE( m_xParser.is(), "FormOperations::impl_ensureInitializedParser_nothrow: factory did not create a parser for us!" );
                }
            }

            if ( m_xParser.is() )
            {
                if ( m_xLoadableForm.is() && m_xLoadableForm->isLoaded() )
                {
                    OUString sStatement;
                    OUString sFilter;
                    OUString sSort;

                    m_xCursorProperties->getPropertyValue( PROPERTY_ACTIVECOMMAND   ) >>= sStatement;
                    m_xCursorProperties->getPropertyValue( PROPERTY_FILTER          ) >>= sFilter;
                    m_xCursorProperties->getPropertyValue( PROPERTY_SORT            ) >>= sSort;

                    m_xParser->setElementaryQuery( sStatement );
                    m_xParser->setFilter         ( sFilter    );
                    m_xParser->setOrder          ( sSort      );
                }

                // start listening at the order/sort properties at the form, so
                // we can keep our parser in sync
                m_xCursorProperties->addPropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );
                m_xCursorProperties->addPropertyChangeListener( PROPERTY_FILTER, this );
                m_xCursorProperties->addPropertyChangeListener( PROPERTY_SORT, this );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormOperations::impl_ensureInitializedParser_nothrow: caught an exception!" );
        }

        m_bInitializedParser = true;
    }


    void FormOperations::impl_disposeParser_nothrow()
    {
        try
        {
            // if we have a parser (and a cursor), then we're listening at the cursor's
            // properties to keep the parser in sync with the cursor
            if ( m_xParser.is() && m_xCursorProperties.is() )
            {
                m_xCursorProperties->removePropertyChangeListener( PROPERTY_FILTER, this );
                m_xCursorProperties->removePropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );
                m_xCursorProperties->removePropertyChangeListener( PROPERTY_SORT, this );
            }

            Reference< XComponent > xParserComp( m_xParser, UNO_QUERY );
            if ( xParserComp.is() )
                xParserComp->dispose();
            m_xParser.clear();

            m_bInitializedParser = false;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormOperations::impl_disposeParser_nothrow: caught an exception!" );
        }
    }


    bool FormOperations::impl_canMoveLeft_throw( ) const
    {
        if ( !impl_hasCursor_nothrow() )
            return false;

        return impl_getRowCount_throw() && ( !m_xCursor->isFirst() || impl_isInsertionRow_throw() );
    }


    bool FormOperations::impl_canMoveRight_throw( ) const
    {
        if ( !impl_hasCursor_nothrow() )
            return false;

        bool bIsNew = impl_isInsertionRow_throw();

        if ( impl_getRowCount_throw() && !m_xCursor->isLast() && !bIsNew )
            return true;

        if ( ::dbtools::canInsert( m_xCursorProperties ) )
            if ( !bIsNew || impl_isModifiedRow_throw() )
                return true;

        if ( bIsNew && m_bActiveControlModified )
            return true;

        return false;
    }


    bool FormOperations::impl_isInsertionRow_throw() const
    {
        return lcl_safeGetPropertyValue_throw( m_xCursorProperties, PROPERTY_ISNEW, false );
    }


    sal_Int32 FormOperations::impl_getRowCount_throw() const
    {
        return lcl_safeGetPropertyValue_throw( m_xCursorProperties, PROPERTY_ROWCOUNT, (sal_Int32)0 );
    }

    bool FormOperations::impl_isRowCountFinal_throw() const
    {
        return lcl_safeGetPropertyValue_throw( m_xCursorProperties, PROPERTY_ROWCOUNTFINAL, false );
    }


    bool FormOperations::impl_isModifiedRow_throw() const
    {
        return lcl_safeGetPropertyValue_throw( m_xCursorProperties, PROPERTY_ISMODIFIED, false );
    }


    bool FormOperations::impl_isParseable_throw() const
    {
        const_cast< FormOperations* >( this )->impl_ensureInitializedParser_nothrow();
        return m_xParser.is() && !m_xParser->getQuery().isEmpty();
    }


    bool FormOperations::impl_hasFilterOrOrder_throw() const
    {
        return impl_isParseable_throw() && ( !m_xParser->getFilter().isEmpty() || !m_xParser->getOrder().isEmpty() );
    }


    bool FormOperations::impl_isInsertOnlyForm_throw() const
    {
        return lcl_safeGetPropertyValue_throw( m_xCursorProperties, PROPERTY_INSERTONLY, true );
    }


    Reference< XControlModel > FormOperations::impl_getCurrentControlModel_throw() const
    {
        Reference< XControl > xControl( m_xController->getCurrentControl() );

        // special handling for grid controls
        Reference< XGrid > xGrid( xControl, UNO_QUERY );
        Reference< XControlModel > xControlModel;

        if ( xGrid.is() )
        {
            Reference< XIndexAccess > xColumns( xControl->getModel(), UNO_QUERY_THROW );
            sal_Int16 nCurrentPos = xGrid->getCurrentColumnPosition();
            nCurrentPos = impl_gridView2ModelPos_nothrow( xColumns, nCurrentPos );

            if ( nCurrentPos != (sal_Int16)-1 )
                xColumns->getByIndex( nCurrentPos ) >>= xControlModel;
        }
        else if ( xControl.is() )
        {
            xControlModel = xControl->getModel();
        }
        return xControlModel;
    }


    Reference< XPropertySet > FormOperations::impl_getCurrentBoundField_nothrow( ) const
    {
        OSL_PRECOND( m_xController.is(), "FormOperations::impl_getCurrentBoundField_nothrow: no controller -> no control!" );
        if ( !m_xController.is() )
            return nullptr;

        Reference< XPropertySet > xField;
        try
        {
            Reference< XPropertySet > xControlModel( impl_getCurrentControlModel_throw(), UNO_QUERY );

            if ( xControlModel.is() && ::comphelper::hasProperty( PROPERTY_BOUNDFIELD, xControlModel ) )
                xControlModel->getPropertyValue( PROPERTY_BOUNDFIELD ) >>= xField;

        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return xField;
    }


    sal_Int16 FormOperations::impl_gridView2ModelPos_nothrow( const Reference< XIndexAccess >& _rxColumns, sal_Int16 _nViewPos )
    {
        OSL_PRECOND( _rxColumns.is(), "FormOperations::impl_gridView2ModelPos_nothrow: invalid columns container!" );
        try
        {
            // loop through all columns
            sal_Int16 col = 0;
            Reference< XPropertySet > xCol;
            bool bHidden( false );
            for ( col = 0; col < _rxColumns->getCount(); ++col )
            {
                _rxColumns->getByIndex( col ) >>= xCol;
                OSL_VERIFY( xCol->getPropertyValue( PROPERTY_HIDDEN ) >>= bHidden );
                if ( bHidden )
                    continue;

                // for every visible col : if nViewPos is greater zero, decrement it, else we
                // have found the model position
                if ( !_nViewPos )
                    break;
                else
                    --_nViewPos;
            }
            if ( col < _rxColumns->getCount() )
                return col;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return (sal_Int16)-1;
    }


    bool FormOperations::impl_moveLeft_throw( ) const
    {
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_moveLeft_throw: no cursor!" );
        if ( !impl_hasCursor_nothrow() )
            return false;

        sal_Bool bRecordInserted = sal_False;
        bool bSuccess = impl_commitCurrentRecord_throw( &bRecordInserted );

        if ( !bSuccess )
            return false;

        if ( bRecordInserted )
        {
            // retrieve the bookmark of the new record and move to the record preceding this bookmark
            Reference< XRowLocate > xLocate( m_xCursor, UNO_QUERY );
            OSL_ENSURE( xLocate.is(), "FormOperations::impl_moveLeft_throw: no XRowLocate!" );
            if ( xLocate.is() )
                xLocate->moveRelativeToBookmark( xLocate->getBookmark(), -1 );
        }
        else
        {
            if ( impl_isInsertionRow_throw() )
            {
                // we assume that the inserted record is now the last record in the
                // result set
                m_xCursor->last();
            }
            else
                m_xCursor->previous();
        }

        return true;
    }


    bool FormOperations::impl_moveRight_throw( ) const
    {
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_moveRight_throw: no cursor!" );
        if ( !impl_hasCursor_nothrow() )
            return false;

        sal_Bool bRecordInserted = sal_False;
        bool bSuccess = impl_commitCurrentRecord_throw( &bRecordInserted );

        if ( !bSuccess )
            return false;

        if ( bRecordInserted )
        {
            // go to insert row
            m_xUpdateCursor->moveToInsertRow();
        }
        else
        {
            if ( m_xCursor->isLast() )
                m_xUpdateCursor->moveToInsertRow();
            else
                (void)m_xCursor->next();
        }

        return true;
    }


    void FormOperations::impl_resetAllControls_nothrow() const
    {
        Reference< XIndexAccess > xContainer( m_xCursor, UNO_QUERY );
        if ( !xContainer.is() )
            return;

        try
        {
            Reference< XReset > xReset;
            sal_Int32 nCount( xContainer->getCount() );
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                if ( xContainer->getByIndex( i ) >>= xReset )
                {
                    // no resets on sub forms
                    Reference< XForm > xAsForm( xReset, UNO_QUERY );
                    if ( !xAsForm.is() )
                        xReset->reset();
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void FormOperations::impl_executeAutoSort_throw( bool _bUp ) const
    {
        OSL_PRECOND( m_xController.is(), "FormOperations::impl_executeAutoSort_throw: need a controller for this!" );
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_executeAutoSort_throw: need a cursor for this!" );
        OSL_PRECOND( impl_isParseable_throw(), "FormOperations::impl_executeAutoSort_throw: need a parseable statement for this!" );
        if ( !m_xController.is() || !impl_hasCursor_nothrow() || !impl_isParseable_throw() )
            return;

        try
        {
            Reference< XControl > xControl = m_xController->getCurrentControl();
            if ( !xControl.is() || !impl_commitCurrentControl_throw() || !impl_commitCurrentRecord_throw() )
                return;

            Reference< XPropertySet > xBoundField( impl_getCurrentBoundField_nothrow() );
            if ( !xBoundField.is() )
                return;

            OUString sOriginalSort;
            m_xCursorProperties->getPropertyValue( PROPERTY_SORT ) >>= sOriginalSort;

            // automatic sort by field is expected to always resets the previous sort order
            m_xParser->setOrder( OUString() );

            impl_appendOrderByColumn_throw aAction(this, xBoundField, _bUp);
            impl_doActionInSQLContext_throw(aAction, RID_STR_COULD_NOT_SET_ORDER );

            WaitObject aWO( nullptr );
            try
            {
                m_xCursorProperties->setPropertyValue( PROPERTY_SORT, makeAny( m_xParser->getOrder() ) );
                m_xLoadableForm->reload();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "FormOperations::impl_executeAutoSort_throw: caught an exception while setting the parser properties!" );
            }


            if ( !m_xLoadableForm->isLoaded() )
            {   // something went wrong -> restore the original state
                try
                {
                    m_xParser->setOrder( sOriginalSort );
                    m_xCursorProperties->setPropertyValue( PROPERTY_SORT, makeAny( m_xParser->getOrder() ) );
                    m_xLoadableForm->reload();
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "FormOperations::impl_executeAutoSort_throw: could not reset the form to its original state!" );
                }

            }
        }
        catch( const RuntimeException& ) { throw; }
        catch( const SQLException& ) { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( OUString(), *const_cast< FormOperations* >( this ), ::cppu::getCaughtException() );
        }
    }


    void FormOperations::impl_executeAutoFilter_throw( ) const
    {
        OSL_PRECOND( m_xController.is(), "FormOperations::impl_executeAutoFilter_throw: need a controller for this!" );
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_executeAutoFilter_throw: need a cursor for this!" );
        OSL_PRECOND( impl_isParseable_throw(), "FormOperations::impl_executeAutoFilter_throw: need a parseable statement for this!" );
        if ( !m_xController.is() || !impl_hasCursor_nothrow() || !impl_isParseable_throw() )
            return;

        try
        {
            Reference< XControl > xControl = m_xController->getCurrentControl();
            if ( !xControl.is() || !impl_commitCurrentControl_throw() || !impl_commitCurrentRecord_throw() )
                return;

            Reference< XPropertySet > xBoundField( impl_getCurrentBoundField_nothrow() );
            if ( !xBoundField.is() )
                return;

            OUString sOriginalFilter;
            m_xCursorProperties->getPropertyValue( PROPERTY_FILTER ) >>= sOriginalFilter;
            bool bApplied = true;
            m_xCursorProperties->getPropertyValue( PROPERTY_APPLYFILTER ) >>= bApplied;

            // if we have a filter, but it's not applied, then we have to overwrite it, else append one
            if ( !bApplied )
                m_xParser->setFilter( OUString() );

            impl_appendFilterByColumn_throw aAction(this, xBoundField);
            impl_doActionInSQLContext_throw( aAction, RID_STR_COULD_NOT_SET_FILTER );

            WaitObject aWO( nullptr );
            try
            {
                m_xCursorProperties->setPropertyValue( PROPERTY_FILTER, makeAny( m_xParser->getFilter() ) );
                m_xCursorProperties->setPropertyValue( PROPERTY_APPLYFILTER, makeAny( true ) );

                m_xLoadableForm->reload();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "FormOperations::impl_executeAutoFilter_throw: caught an exception while setting the parser properties!" );
            }


            if ( !m_xLoadableForm->isLoaded() )
            {   // something went wrong -> restore the original state
                try
                {
                    m_xParser->setOrder( sOriginalFilter );
                    m_xCursorProperties->setPropertyValue( PROPERTY_APPLYFILTER, makeAny( bApplied ) );
                    m_xCursorProperties->setPropertyValue( PROPERTY_FILTER, makeAny( m_xParser->getFilter() ) );
                    m_xLoadableForm->reload();
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "FormOperations::impl_executeAutoFilter_throw: could not reset the form to its original state!" );
                }

            }
        }
        catch( const RuntimeException& ) { throw; }
        catch( const SQLException& ) { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( OUString(), *const_cast< FormOperations* >( this ), ::cppu::getCaughtException() );
        }
    }


    void FormOperations::impl_executeFilterOrSort_throw( bool _bFilter ) const
    {
        OSL_PRECOND( m_xController.is(), "FormOperations::impl_executeFilterOrSort_throw: need a controller for this!" );
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_executeFilterOrSort_throw: need a cursor for this!" );
        OSL_PRECOND( impl_isParseable_throw(), "FormOperations::impl_executeFilterOrSort_throw: need a parseable statement for this!" );
        if ( !m_xController.is() || !impl_hasCursor_nothrow() || !impl_isParseable_throw() )
            return;

        if ( !impl_commitCurrentControl_throw() || !impl_commitCurrentRecord_throw() )
            return;
        try
        {
            Reference< XExecutableDialog> xDialog;
            if ( _bFilter )
            {
                xDialog = css::sdb::FilterDialog::createWithQuery(m_xContext, m_xParser, m_xCursor,
                              Reference<css::awt::XWindow>());
            }
            else
            {
                xDialog = css::sdb::OrderDialog::createWithQuery(m_xContext, m_xParser, m_xCursorProperties);
            }


            if ( RET_OK == xDialog->execute() )
            {
                WaitObject aWO( nullptr );
                if ( _bFilter )
                    m_xCursorProperties->setPropertyValue( PROPERTY_FILTER, makeAny( m_xParser->getFilter() ) );
                else
                    m_xCursorProperties->setPropertyValue( PROPERTY_SORT, makeAny( m_xParser->getOrder() ) );
                m_xLoadableForm->reload();
            }

        }
        catch( const RuntimeException& ) { throw; }
        catch( const SQLException& ) { throw; }
        catch( const Exception& )
        {
            throw WrappedTargetException( OUString(), *const_cast< FormOperations* >( this ), ::cppu::getCaughtException() );
        }
    }


    template < typename FunctObj >
    void FormOperations::impl_doActionInSQLContext_throw( FunctObj f, sal_uInt16 _nErrorResourceId ) const
    {
        try
        {
            f();
        }
#if HAVE_FEATURE_DBCONNECTIVITY
        catch( const SQLException& e )
        {
            (void)e;
            if ( !_nErrorResourceId )
                // no information to prepend
                throw;

            SQLExceptionInfo aInfo( ::cppu::getCaughtException() );
            OUString sAdditionalError( FRM_RES_STRING( _nErrorResourceId ) );
            aInfo.prepend( sAdditionalError );
            aInfo.doThrow();
        }
#endif
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            OUString sAdditionalError( FRM_RES_STRING( _nErrorResourceId ) );
            throw WrappedTargetException( sAdditionalError, *const_cast< FormOperations* >( this ), ::cppu::getCaughtException() );
        }
    }


} // namespace frm


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_forms_FormOperations_get_implementation(css::uno::XComponentContext* context,
                                                          css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::FormOperations(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
