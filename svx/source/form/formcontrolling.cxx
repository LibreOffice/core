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


#include <sal/macros.h>
#include "formcontrolling.hxx"
#include "fmurl.hxx"
#include "svx/svxids.hrc"
#include "fmprop.hrc"
#include "svx/fmtools.hxx"

#include <com/sun/star/form/runtime/FormOperations.hpp>
#include <com/sun/star/form/runtime/FormFeature.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/diagnose.h>

#include <functional>
#include <algorithm>


namespace svx
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::form::runtime::XFormController;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::form::runtime::FormOperations;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::form::runtime::FeatureState;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdb::XSQLErrorBroadcaster;
    using ::com::sun::star::sdb::SQLErrorEvent;
    using ::com::sun::star::lang::EventObject;

    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;


    //= FeatureSlotTranslation

    namespace
    {
        struct FeatureDescription
        {
            OUString sURL;           // the URL
            sal_Int32       nSlotId;        // the SFX-compatible slot ID
            sal_Int16       nFormFeature;   // the css.form.runtime.FormFeature ID
        };
        typedef ::std::vector< FeatureDescription > FeatureDescriptions;


        const FeatureDescriptions& getFeatureDescriptions()
        {
            static FeatureDescriptions s_aFeatureDescriptions;
            if ( s_aFeatureDescriptions.empty() )
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if ( s_aFeatureDescriptions.empty() )
                {
                    FeatureDescription aDescriptions[] = {
                        { OUString(FMURL_FORM_POSITION),        SID_FM_RECORD_ABSOLUTE,     FormFeature::MoveAbsolute },
                        { OUString(FMURL_FORM_RECORDCOUNT),     SID_FM_RECORD_TOTAL,        FormFeature::TotalRecords },
                        { OUString(FMURL_RECORD_MOVEFIRST),     SID_FM_RECORD_FIRST,        FormFeature::MoveToFirst },
                        { OUString(FMURL_RECORD_MOVEPREV),      SID_FM_RECORD_PREV,         FormFeature::MoveToPrevious },
                        { OUString(FMURL_RECORD_MOVENEXT),      SID_FM_RECORD_NEXT,         FormFeature::MoveToNext },
                        { OUString(FMURL_RECORD_MOVELAST),      SID_FM_RECORD_LAST,         FormFeature::MoveToLast },
                        { OUString(FMURL_RECORD_MOVETONEW),     SID_FM_RECORD_NEW,          FormFeature::MoveToInsertRow },
                        { OUString(FMURL_RECORD_SAVE),          SID_FM_RECORD_SAVE,         FormFeature::SaveRecordChanges },
                        { OUString(FMURL_RECORD_DELETE),        SID_FM_RECORD_DELETE,       FormFeature::DeleteRecord },
                        { OUString(FMURL_FORM_REFRESH),         SID_FM_REFRESH,             FormFeature::ReloadForm },
                        { OUString(FMURL_FORM_REFRESH_CURRENT_CONTROL),
                                                      SID_FM_REFRESH_FORM_CONTROL,FormFeature::RefreshCurrentControl },
                        { OUString(FMURL_RECORD_UNDO),          SID_FM_RECORD_UNDO,         FormFeature::UndoRecordChanges },
                        { OUString(FMURL_FORM_SORT_UP),         SID_FM_SORTUP,              FormFeature::SortAscending },
                        { OUString(FMURL_FORM_SORT_DOWN),       SID_FM_SORTDOWN,            FormFeature::SortDescending },
                        { OUString(FMURL_FORM_SORT),            SID_FM_ORDERCRIT,           FormFeature::InteractiveSort },
                        { OUString(FMURL_FORM_AUTO_FILTER),     SID_FM_AUTOFILTER,          FormFeature::AutoFilter },
                        { OUString(FMURL_FORM_FILTER),          SID_FM_FILTERCRIT,          FormFeature::InteractiveFilter },
                        { OUString(FMURL_FORM_APPLY_FILTER),    SID_FM_FORM_FILTERED,       FormFeature::ToggleApplyFilter },
                        { OUString(FMURL_FORM_REMOVE_FILTER),   SID_FM_REMOVE_FILTER_SORT,  FormFeature::RemoveFilterAndSort }
                    };
                    for ( size_t i=0; i<sizeof(aDescriptions)/sizeof(aDescriptions[0]); ++i )
                        s_aFeatureDescriptions.push_back( aDescriptions[i] );
                }
            };
            return s_aFeatureDescriptions;
        }
    }


    namespace
    {

        struct MatchFeatureDescriptionByURL : public ::std::unary_function< FeatureDescription, bool >
        {
            const OUString&  m_rURL;
            explicit MatchFeatureDescriptionByURL( const OUString& _rURL ) :m_rURL( _rURL ) { }

            bool operator()( const FeatureDescription& _compare )
            {
                return m_rURL == _compare.sURL;
            }
        };


        struct MatchFeatureDescriptionBySlotId : public ::std::unary_function< FeatureDescription, bool >
        {
            sal_Int32   m_nSlotId;
            explicit MatchFeatureDescriptionBySlotId( sal_Int32 _nSlotId ) :m_nSlotId( _nSlotId ) { }

            bool operator()( const FeatureDescription& _compare )
            {
                return m_nSlotId == _compare.nSlotId;
            }
        };


        struct MatchFeatureDescriptionByFormFeature : public ::std::unary_function< FeatureDescription, bool >
        {
            sal_Int32   m_nFormFeature;
            explicit MatchFeatureDescriptionByFormFeature( sal_Int32 _nFormFeature ) :m_nFormFeature( _nFormFeature ) { }

            bool operator()( const FeatureDescription& _compare )
            {
                return m_nFormFeature == _compare.nFormFeature;
            }
        };


        struct FormFeatureToSlotId : public ::std::unary_function< sal_Int16, sal_Int32 >
        {
            sal_Int32 operator()( sal_Int16 _FormFeature )
            {
                return FeatureSlotTranslation::getSlotIdForFormFeature( _FormFeature );
            }
        };
    }


    sal_Int32 FeatureSlotTranslation::getControllerFeatureSlotIdForURL( const OUString& _rMainURL )
    {
        const FeatureDescriptions& rDescriptions( getFeatureDescriptions() );
        FeatureDescriptions::const_iterator pos = ::std::find_if( rDescriptions.begin(), rDescriptions.end(), MatchFeatureDescriptionByURL( _rMainURL ) );
        return ( pos != rDescriptions.end() ) ? pos->nSlotId : -1;
    }


    sal_Int16 FeatureSlotTranslation::getFormFeatureForSlotId( sal_Int32 _nSlotId )
    {
        const FeatureDescriptions& rDescriptions( getFeatureDescriptions() );
        FeatureDescriptions::const_iterator pos = ::std::find_if( rDescriptions.begin(), rDescriptions.end(), MatchFeatureDescriptionBySlotId( _nSlotId ) );
        OSL_ENSURE( pos != rDescriptions.end(), "FeatureSlotTranslation::getFormFeatureForSlotId: not found!" );
        return ( pos != rDescriptions.end() ) ? pos->nFormFeature : -1;
    }


    sal_Int32 FeatureSlotTranslation::getSlotIdForFormFeature( sal_Int16 _nFormFeature )
    {
        const FeatureDescriptions& rDescriptions( getFeatureDescriptions() );
        FeatureDescriptions::const_iterator pos = ::std::find_if( rDescriptions.begin(), rDescriptions.end(), MatchFeatureDescriptionByFormFeature( _nFormFeature ) );
        OSL_ENSURE( pos != rDescriptions.end(), "FeatureSlotTranslation::getSlotIdForFormFeature: not found!" );
        return ( pos != rDescriptions.end() ) ? pos->nSlotId : -1;
    }

    ControllerFeatures::ControllerFeatures( IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_pInvalidationCallback( _pInvalidationCallback )
        ,m_pImpl( nullptr )
    {
    }


    ControllerFeatures::ControllerFeatures( const Reference< XFormController >& _rxController, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_pInvalidationCallback( _pInvalidationCallback )
        ,m_pImpl( nullptr )
    {
        assign( _rxController );
    }


    void ControllerFeatures::assign( const Reference< XFormController >& _rxController )
    {
        dispose();
        m_pImpl = new FormControllerHelper( _rxController, m_pInvalidationCallback );
        m_pImpl->acquire();
    }


    ControllerFeatures::~ControllerFeatures()
    {
        dispose();
    }


    void ControllerFeatures::dispose()
    {
        if ( m_pImpl )
        {
            m_pImpl->dispose();
            m_pImpl->release();
            m_pImpl = nullptr;
        }
    }

    FormControllerHelper::FormControllerHelper( const Reference< XFormController >& _rxController, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_pInvalidationCallback( _pInvalidationCallback )
    {
        osl_atomic_increment( &m_refCount );
        try
        {
            m_xFormOperations = FormOperations::createWithFormController( comphelper::getProcessComponentContext(), _rxController );
            if ( m_xFormOperations.is() )
                m_xFormOperations->setFeatureInvalidation( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        osl_atomic_decrement( &m_refCount );
    }


    FormControllerHelper::~FormControllerHelper( )
    {
        try
        {
            acquire();
            dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void FormControllerHelper::dispose()
    {
        if ( m_xFormOperations.is() )
            m_xFormOperations->dispose();
        m_xFormOperations.clear();
    }


    bool FormControllerHelper::isEnabled( sal_Int32 _nSlotId ) const
    {
        if ( !m_xFormOperations.is() )
            return false;
        return m_xFormOperations->isEnabled( FeatureSlotTranslation::getFormFeatureForSlotId( _nSlotId ) );
    }


    Reference< XRowSet > FormControllerHelper::getCursor() const
    {
        Reference< XRowSet > xCursor;
        if ( m_xFormOperations.is() )
            xCursor = m_xFormOperations->getCursor();
        return xCursor;
    }


    void FormControllerHelper::getState( sal_Int32 _nSlotId, FeatureState& _rState ) const
    {
        if ( m_xFormOperations.is() )
            _rState = m_xFormOperations->getState( FeatureSlotTranslation::getFormFeatureForSlotId( _nSlotId ) );
    }


    bool FormControllerHelper::commitCurrentControl( ) const
    {
        return impl_operateForm_nothrow( COMMIT_CONTROL );
    }


    bool FormControllerHelper::commitCurrentRecord() const
    {
        return impl_operateForm_nothrow( COMMIT_RECORD );
    }


    void FormControllerHelper::execute( sal_Int32 _nSlotId, const OUString& _rParamName, const Any& _rParamValue ) const
    {
        Sequence< NamedValue > aArguments { { _rParamName, _rParamValue } };
        impl_operateForm_nothrow( EXECUTE_ARGS, FeatureSlotTranslation::getFormFeatureForSlotId( _nSlotId ), aArguments );
    }


    bool FormControllerHelper::impl_operateForm_nothrow( const FormOperation _eWhat, const sal_Int16 _nFeature,
            const Sequence< NamedValue >& _rArguments ) const
    {
        if ( !m_xFormOperations.is() )
            return false;

        Any aError;
        bool bSuccess = false;
        const_cast< FormControllerHelper* >( this )->m_aOperationError.clear();
        try
        {
            // to prevent the controller from displaying any error messages which happen while we operate on it,
            // we add ourself as XSQLErrorListener. By contract, a FormController displays errors if and only if
            // no SQLErrorListeners are registered.
            m_xFormOperations->getController()->addSQLErrorListener( const_cast< FormControllerHelper* >(this) );

            switch ( _eWhat )
            {
            case COMMIT_CONTROL:
                bSuccess = m_xFormOperations->commitCurrentControl();
                break;

            case COMMIT_RECORD:
            {
                sal_Bool bDummy( false );
                bSuccess = m_xFormOperations->commitCurrentRecord( bDummy );
            }
            break;

            case EXECUTE:
                m_xFormOperations->execute( _nFeature );
                bSuccess = true;
                break;

            case EXECUTE_ARGS:
                m_xFormOperations->executeWithArguments( _nFeature, _rArguments );
                bSuccess = true;
                break;
            }
        }
        catch ( const SQLException& )
        {
            m_xFormOperations->getController()->removeSQLErrorListener( const_cast< FormControllerHelper* >(this) );
            aError = ::cppu::getCaughtException();
        }
        catch( const Exception& )
        {
            m_xFormOperations->getController()->removeSQLErrorListener( const_cast< FormControllerHelper* >(this) );
            SQLException aFallbackError;
            aFallbackError.Message = ::comphelper::anyToString( ::cppu::getCaughtException() );
            aError <<= aFallbackError;
        }

        if ( bSuccess )
            return true;

        // display the error. Prefer the one reported in errorOccurred over the one caught.
        if ( m_aOperationError.hasValue() )
            displayException( m_aOperationError );
        else if ( aError.hasValue() )
            displayException( aError );
        else
            OSL_FAIL( "FormControllerHelper::impl_operateForm_nothrow: no success, but no error?" );

        return false;
    }


    void FormControllerHelper::execute( sal_Int32 _nSlotId ) const
    {
        impl_operateForm_nothrow( EXECUTE, FeatureSlotTranslation::getFormFeatureForSlotId( _nSlotId ),
            Sequence< NamedValue >() );
    }


    void SAL_CALL FormControllerHelper::invalidateFeatures( const Sequence< ::sal_Int16 >& _Features ) throw (RuntimeException, std::exception)
    {
        if ( !m_pInvalidationCallback )
            // nobody's interested in ...
            return;

        ::std::vector< sal_Int32 > aFeatures( _Features.getLength() );
        ::std::transform(
            _Features.getConstArray(),
            _Features.getConstArray() + _Features.getLength(),
            aFeatures.begin(),
            FormFeatureToSlotId()
        );

        m_pInvalidationCallback->invalidateFeatures( aFeatures );
    }


    void SAL_CALL FormControllerHelper::invalidateAllFeatures() throw (RuntimeException, std::exception)
    {
        if ( !m_pInvalidationCallback )
            // nobody's interested in ...
            return;

        // actually, it's a little bit more than the supported features,
        // but on the medium term, we are to support everything listed
        // here
        ::std::vector< sal_Int32 > aSupportedFeatures;
        sal_Int32 pSupportedFeatures[] =
        {
            SID_FM_RECORD_FIRST,
            SID_FM_RECORD_NEXT,
            SID_FM_RECORD_PREV,
            SID_FM_RECORD_LAST,
            SID_FM_RECORD_NEW,
            SID_FM_RECORD_DELETE,
            SID_FM_RECORD_ABSOLUTE,
            SID_FM_RECORD_TOTAL,
            SID_FM_RECORD_SAVE,
            SID_FM_RECORD_UNDO,
            SID_FM_REMOVE_FILTER_SORT,
            SID_FM_SORTUP,
            SID_FM_SORTDOWN,
            SID_FM_ORDERCRIT,
            SID_FM_AUTOFILTER,
            SID_FM_FILTERCRIT,
            SID_FM_FORM_FILTERED,
            SID_FM_REFRESH,
            SID_FM_REFRESH_FORM_CONTROL,
            SID_FM_SEARCH,
            SID_FM_FILTER_START,
            SID_FM_VIEW_AS_GRID
        };
        sal_Int32 nFeatureCount = sizeof( pSupportedFeatures ) / sizeof( pSupportedFeatures[ 0 ] );
        aSupportedFeatures.resize( nFeatureCount );
        ::std::copy( pSupportedFeatures, pSupportedFeatures + nFeatureCount, aSupportedFeatures.begin() );

        m_pInvalidationCallback->invalidateFeatures( aSupportedFeatures );
    }


    void SAL_CALL FormControllerHelper::errorOccured( const SQLErrorEvent& _Event ) throw (RuntimeException, std::exception)
    {
        OSL_ENSURE( !m_aOperationError.hasValue(), "FormControllerHelper::errorOccurred: two errors during one operation?" );
        m_aOperationError = _Event.Reason;
    }


    void SAL_CALL FormControllerHelper::disposing( const EventObject& /*_Source*/ ) throw (RuntimeException, std::exception)
    {
        // not interested in
    }


    bool FormControllerHelper::isInsertionRow() const
    {
        bool bIs = false;
        if ( m_xFormOperations.is() )
            bIs = m_xFormOperations->isInsertionRow();
        return bIs;
    }


    bool FormControllerHelper::isModifiedRow() const
    {
        bool bIs = false;
        if ( m_xFormOperations.is() )
            bIs = m_xFormOperations->isModifiedRow();
        return bIs;
    }

    bool FormControllerHelper::canDoFormFilter() const
    {
        if ( !m_xFormOperations.is() )
            return false;

        bool bCanDo = false;
        try
        {
            Reference< XPropertySet > xCursorProperties( m_xFormOperations->getCursor(), UNO_QUERY_THROW );

            bool bEscapeProcessing( false );
            OSL_VERIFY( xCursorProperties->getPropertyValue( FM_PROP_ESCAPE_PROCESSING ) >>= bEscapeProcessing );

            OUString sActiveCommand;
            OSL_VERIFY( xCursorProperties->getPropertyValue( FM_PROP_ACTIVECOMMAND ) >>= sActiveCommand );

            bool bInsertOnlyForm( false );
            OSL_VERIFY( xCursorProperties->getPropertyValue( FM_PROP_INSERTONLY ) >>= bInsertOnlyForm );

            bCanDo = bEscapeProcessing && !sActiveCommand.isEmpty() && !bInsertOnlyForm;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bCanDo;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
