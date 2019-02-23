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
#include <frm_strings.hxx>
#include <frm_resource.hxx>
#include <strings.hrc>
#include <services.hxx>

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/form/runtime/FormFeature.hpp>
#include <com/sun/star/frame/XFrame.hpp>
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
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/button.hxx>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/container.hxx>
#include <comphelper/property.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>


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

    void SAL_CALL FormOperations::initialize( const Sequence< Any >& _arguments )
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

    OUString SAL_CALL FormOperations::getImplementationName(  )
    {
        return OUString( "com.sun.star.comp.forms.FormOperations" );
    }

    sal_Bool SAL_CALL FormOperations::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL FormOperations::getSupportedServiceNames(  )
    {
        return { "com.sun.star.form.runtime.FormOperations" };
    }

    Reference< XRowSet > SAL_CALL FormOperations::getCursor()
    {
        MethodGuard aGuard( *this );
        return m_xCursor;
    }

    Reference< XResultSetUpdate > SAL_CALL FormOperations::getUpdateCursor()
    {
        MethodGuard aGuard( *this );
        return m_xUpdateCursor;
    }


    Reference< XFormController > SAL_CALL FormOperations::getController()
    {
        MethodGuard aGuard( *this );
        return m_xController;
    }


    Reference< XFeatureInvalidation > SAL_CALL FormOperations::getFeatureInvalidation()
    {
        MethodGuard aGuard( *this );
        return m_xFeatureInvalidation;
    }


    void SAL_CALL FormOperations::setFeatureInvalidation( const Reference< XFeatureInvalidation > & _rxFeatureInvalidation )
    {
        MethodGuard aGuard( *this );
        m_xFeatureInvalidation = _rxFeatureInvalidation;
    }


    FeatureState SAL_CALL FormOperations::getState( ::sal_Int16 _nFeature )
    {
        MethodGuard aGuard( *this );

        FeatureState aState;
        aState.Enabled = false;

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
                    aState.Enabled = false;
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
                OUString sHaving;
                m_xCursorProperties->getPropertyValue( PROPERTY_FILTER )       >>= sFilter;
                m_xCursorProperties->getPropertyValue( PROPERTY_HAVINGCLAUSE ) >>= sHaving;
                if ( ! (sFilter.isEmpty() && sHaving.isEmpty()) )
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
                            aState.Enabled = false;
                        }
                        else
                        {
                            if ( bIsNew )
                                nPosition = ++nCount;
                            aState.State <<= nPosition;
                            aState.Enabled = true;
                        }
                    }
                    else
                    {
                        aState.State <<= nPosition;
                        aState.Enabled = true;
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
                aState.Enabled = true;
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


    sal_Bool SAL_CALL FormOperations::isEnabled( ::sal_Int16 _nFeature )
    {
        MethodGuard aGuard( *this );

        FeatureState aState( getState( _nFeature ) );
        return aState.Enabled;
    }


    namespace
    {
        bool lcl_needConfirmCommit( sal_Int32 _nFeature )
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
        bool lcl_requiresArguments( sal_Int32 _nFeature )
        {
            return ( _nFeature == FormFeature::MoveAbsolute );
        }
        bool lcl_isExecutableFeature( sal_Int32 _nFeature )
        {
            return ( _nFeature != FormFeature::TotalRecords );
        }

        template < typename TYPE >
        TYPE lcl_safeGetPropertyValue_throw( const Reference< XPropertySet >& _rxProperties, const OUString& _rPropertyName, TYPE Default )
        {
            TYPE value( Default );
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
                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(nullptr,
                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                               FRM_RES_STRING(RID_STR_QUERY_SAVE_MODIFIED_ROW)));
                xQueryBox->add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
                xQueryBox->set_default_response(RET_YES);

                switch (xQueryBox->run())
                {
                    case RET_NO:
                        shouldCommit = false;
                        [[fallthrough]]; // don't ask again!
                    case RET_YES:
                        needConfirmation = false;
                        return true;
                    case RET_CANCEL:
                        return false;
                }
            }
            return true;
        }

        bool commit1Form(const Reference< XFormController >& xCntrl, bool &needConfirmation, bool &shouldCommit)
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

        bool commitFormAndSubforms(const Reference< XFormController >& xCntrl, bool needConfirmation)
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

            return commit1Form(xCntrl, needConfirmation, shouldCommit);
        }

        bool commit1Form(const Reference< XForm >& xFrm, bool &needConfirmation, bool &shouldCommit)
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

        bool commitFormAndSubforms(const Reference< XForm >& xFrm, bool needConfirmation)
        {
            // No control...  do what we can with the models
            bool shouldCommit(true);
            Reference< XIndexAccess > xFormComps(xFrm, UNO_QUERY_THROW);

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

            return commit1Form(xFrm, needConfirmation, shouldCommit);
        }
    }

    void SAL_CALL FormOperations::execute( ::sal_Int16 _nFeature )
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
                    Sequence< OUString > aNames( 3 );
                    aNames[0] = PROPERTY_FILTER;
                    aNames[1] = PROPERTY_HAVINGCLAUSE;
                    aNames[2] = PROPERTY_SORT;

                    Sequence< Any> aValues( 3 );
                    aValues[0] <<= OUString();
                    aValues[1] <<= OUString();
                    aValues[2] <<= OUString();

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
                const char* pErrorResourceId = RID_STR_FEATURE_UNKNOWN;
                if ( lcl_requiresArguments( _nFeature ) )
                    pErrorResourceId = RID_STR_FEATURE_REQUIRES_PARAMETERS;
                else if ( !lcl_isExecutableFeature( _nFeature ) )
                    pErrorResourceId = RID_STR_FEATURE_NOT_EXECUTABLE;
                throw IllegalArgumentException( FRM_RES_STRING( pErrorResourceId ), *this, 1 );
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


    void SAL_CALL FormOperations::executeWithArguments( ::sal_Int16 _nFeature, const Sequence< NamedValue >& _rArguments )
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

                if ( bFinalCount && ( nPosition > nCount ) )
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


    sal_Bool SAL_CALL FormOperations::commitCurrentRecord( sal_Bool& _out_rRecordInserted )
    {
        MethodGuard aGuard( *this );
        _out_rRecordInserted = false;

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
                    *_pRecordInserted = true;
            }
            else
                m_xUpdateCursor->updateRow();
            bResult = true;
        }
        return bResult;
    }


    sal_Bool SAL_CALL FormOperations::commitCurrentControl()
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
            DBG_UNHANDLED_EXCEPTION("forms.runtime");
            bSuccess = false;
        }

        return bSuccess;
    }


    sal_Bool SAL_CALL FormOperations::isInsertionRow()
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


    sal_Bool SAL_CALL FormOperations::isModifiedRow()
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


    void SAL_CALL FormOperations::cursorMoved( const EventObject& /*_Event*/ )
    {
        MethodGuard aGuard( *this );
        m_bActiveControlModified = false;

        impl_invalidateAllSupportedFeatures_nothrow( aGuard );
    }


    void SAL_CALL FormOperations::rowChanged( const EventObject& /*_Event*/ )
    {
        // not interested in
    }


    void SAL_CALL FormOperations::rowSetChanged( const EventObject& /*_Event*/ )
    {
        // not interested in
    }


    void SAL_CALL FormOperations::modified( const EventObject& /*_Source*/ )
    {
        MethodGuard aGuard( *this );

        OSL_ENSURE( m_xCursor.is(), "FormOperations::modified: already disposed!" );
        if ( !m_bActiveControlModified )
        {
            m_bActiveControlModified = true;
            impl_invalidateModifyDependentFeatures_nothrow( aGuard );
        }
    }


    void SAL_CALL FormOperations::propertyChange( const PropertyChangeEvent& _rEvent )
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
                else if ( _rEvent.PropertyName == PROPERTY_HAVINGCLAUSE )
                {
                    if ( m_xParser->getHavingClause() != sNewValue )
                        m_xParser->setHavingClause( sNewValue );
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


    void SAL_CALL FormOperations::disposing( const EventObject& /*_Source*/ )
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
            DBG_UNHANDLED_EXCEPTION("forms.runtime");
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
        if ( !m_xCursor.is() )
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

        static Sequence< sal_Int16 > const s_aModifyDependentFeatures
        {
            FormFeature::MoveToNext,
            FormFeature::MoveToInsertRow,
            FormFeature::SaveRecordChanges,
            FormFeature::UndoRecordChanges
        };

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
                    OUString sHaving;
                    OUString sSort;

                    m_xCursorProperties->getPropertyValue( PROPERTY_ACTIVECOMMAND   ) >>= sStatement;
                    m_xCursorProperties->getPropertyValue( PROPERTY_FILTER          ) >>= sFilter;
                    m_xCursorProperties->getPropertyValue( PROPERTY_HAVINGCLAUSE    ) >>= sHaving;
                    m_xCursorProperties->getPropertyValue( PROPERTY_SORT            ) >>= sSort;

                    m_xParser->setElementaryQuery( sStatement );
                    m_xParser->setFilter         ( sFilter    );
                    m_xParser->setHavingClause   ( sHaving    );
                    m_xParser->setOrder          ( sSort      );
                }

                // start listening at the order/sort properties at the form, so
                // we can keep our parser in sync
                m_xCursorProperties->addPropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );
                m_xCursorProperties->addPropertyChangeListener( PROPERTY_FILTER, this );
                m_xCursorProperties->addPropertyChangeListener( PROPERTY_HAVINGCLAUSE, this );
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
                m_xCursorProperties->removePropertyChangeListener( PROPERTY_HAVINGCLAUSE, this );
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
        return lcl_safeGetPropertyValue_throw( m_xCursorProperties, PROPERTY_ROWCOUNT, sal_Int32(0) );
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
        return impl_isParseable_throw() && ( !m_xParser->getFilter().isEmpty() ||
                                             !m_xParser->getHavingClause().isEmpty() ||
                                             !m_xParser->getOrder().isEmpty() );
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
            sal_Int32 nCurrentPos = impl_gridView2ModelPos_nothrow( xColumns, xGrid->getCurrentColumnPosition() );

            if ( nCurrentPos != -1 )
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
            DBG_UNHANDLED_EXCEPTION("forms.runtime");
        }

        return xField;
    }


    sal_Int32 FormOperations::impl_gridView2ModelPos_nothrow( const Reference< XIndexAccess >& _rxColumns, sal_Int16 _nViewPos )
    {
        OSL_PRECOND( _rxColumns.is(), "FormOperations::impl_gridView2ModelPos_nothrow: invalid columns container!" );
        try
        {
            // loop through all columns
            sal_Int32 col = 0;
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
            DBG_UNHANDLED_EXCEPTION("forms.runtime");
        }
        return -1;
    }


    void FormOperations::impl_moveLeft_throw( ) const
    {
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_moveLeft_throw: no cursor!" );
        if ( !impl_hasCursor_nothrow() )
            return;

        sal_Bool bRecordInserted = false;
        bool bSuccess = impl_commitCurrentRecord_throw( &bRecordInserted );

        if ( !bSuccess )
            return;

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
    }


    void FormOperations::impl_moveRight_throw( ) const
    {
        OSL_PRECOND( impl_hasCursor_nothrow(), "FormOperations::impl_moveRight_throw: no cursor!" );
        if ( !impl_hasCursor_nothrow() )
            return;

        sal_Bool bRecordInserted = false;
        bool bSuccess = impl_commitCurrentRecord_throw( &bRecordInserted );

        if ( !bSuccess )
            return;

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
            DBG_UNHANDLED_EXCEPTION("forms.runtime");
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
            OUString sOriginalHaving;
            m_xCursorProperties->getPropertyValue( PROPERTY_FILTER       ) >>= sOriginalFilter;
            m_xCursorProperties->getPropertyValue( PROPERTY_HAVINGCLAUSE ) >>= sOriginalHaving;
            bool bApplied = true;
            m_xCursorProperties->getPropertyValue( PROPERTY_APPLYFILTER ) >>= bApplied;

            // if we have a filter, but it's not applied, then we have to overwrite it, else append one
            if ( !bApplied )
            {
                m_xParser->setFilter( OUString() );
                m_xParser->setHavingClause( OUString() );
            }

            impl_appendFilterByColumn_throw aAction(this, m_xParser, xBoundField);
            impl_doActionInSQLContext_throw( aAction, RID_STR_COULD_NOT_SET_FILTER );

            WaitObject aWO( nullptr );
            try
            {
                m_xCursorProperties->setPropertyValue( PROPERTY_FILTER,       makeAny( m_xParser->getFilter() ) );
                m_xCursorProperties->setPropertyValue( PROPERTY_HAVINGCLAUSE, makeAny( m_xParser->getHavingClause() ) );
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
                    m_xParser->setFilter      ( sOriginalFilter );
                    m_xParser->setHavingClause( sOriginalHaving );
                    m_xCursorProperties->setPropertyValue( PROPERTY_APPLYFILTER, makeAny( bApplied ) );
                    m_xCursorProperties->setPropertyValue( PROPERTY_FILTER,       makeAny( m_xParser->getFilter() ) );
                    m_xCursorProperties->setPropertyValue( PROPERTY_HAVINGCLAUSE, makeAny( m_xParser->getHavingClause() ) );
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
            css::uno::Reference<css::awt::XWindow> xDialogParent;

            //tdf#122152 extract parent for dialog
            css::uno::Reference<css::awt::XTabController> xTabController(m_xController, css::uno::UNO_QUERY);
            if (xTabController.is())
            {
                css::uno::Reference<css::awt::XControl> xContainerControl(xTabController->getContainer(), css::uno::UNO_QUERY);
                if (xContainerControl.is())
                {
                    css::uno::Reference<css::awt::XWindowPeer> xContainerPeer(xContainerControl->getPeer(), css::uno::UNO_QUERY);
                    xDialogParent = css::uno::Reference<css::awt::XWindow>(xContainerPeer, css::uno::UNO_QUERY);
                }
            }

            Reference< XExecutableDialog> xDialog;
            if ( _bFilter )
            {
                xDialog = css::sdb::FilterDialog::createWithQuery(m_xContext, m_xParser, m_xCursor,
                                                                  xDialogParent);
            }
            else
            {
                xDialog = css::sdb::OrderDialog::createWithQuery(m_xContext, m_xParser, m_xCursorProperties,
                                                                 xDialogParent);
            }

            if ( RET_OK == xDialog->execute() )
            {
                WaitObject aWO( nullptr );
                if ( _bFilter )
                {
                    m_xCursorProperties->setPropertyValue( PROPERTY_FILTER,       makeAny( m_xParser->getFilter() ) );
                    m_xCursorProperties->setPropertyValue( PROPERTY_HAVINGCLAUSE, makeAny( m_xParser->getHavingClause() ) );
                }
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
    void FormOperations::impl_doActionInSQLContext_throw( FunctObj f, const char* pErrorResourceId ) const
    {
        try
        {
            f();
        }
#if HAVE_FEATURE_DBCONNECTIVITY
        catch( const SQLException& )
        {
            if (!pErrorResourceId) // no information to prepend
                throw;

            SQLExceptionInfo aInfo( ::cppu::getCaughtException() );
            OUString sAdditionalError( FRM_RES_STRING( pErrorResourceId ) );
            aInfo.prepend( sAdditionalError );
            aInfo.doThrow();
        }
#endif
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            OUString sAdditionalError( FRM_RES_STRING( pErrorResourceId ) );
            throw WrappedTargetException( sAdditionalError, *const_cast< FormOperations* >( this ), ::cppu::getCaughtException() );
        }
    }


} // namespace frm


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_forms_FormOperations_get_implementation(css::uno::XComponentContext* context,
                                                          css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::FormOperations(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
