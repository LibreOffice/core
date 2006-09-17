/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formcontrolling.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:12:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_FORMCONTROLLING_HXX
#include "formcontrolling.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMURL_HXX
#include "fmurl.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETELISTENER_HPP_
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEEVENT_HPP_
#include <com/sun/star/sdb/RowChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEACTION_HPP_
#include <com/sun/star/sdb/RowChangeAction.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRID_HPP_
#include <com/sun/star/form/XGrid.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::svxform;

    //====================================================================
    //= FeatureSlotTranslation
    //====================================================================
    namespace
    {
        //................................................................
        void    getTranslationTables( ::rtl::OUString const** _ppURLs, ::rtl::OUString const** _ppURLsEnd,
            sal_Int32 const** _ppIds, sal_Int32 const** _ppIdsEnd )
        {
            static const ::rtl::OUString aURLs[] =
            {
                FMURL_FORM_POSITION,
                FMURL_FORM_RECORDCOUNT,
                FMURL_RECORD_MOVEFIRST,
                FMURL_RECORD_MOVEPREV,
                FMURL_RECORD_MOVENEXT,
                FMURL_RECORD_MOVELAST,
                FMURL_RECORD_MOVETONEW,
                FMURL_RECORD_SAVE,
                FMURL_RECORD_DELETE,
                FMURL_FORM_REFRESH,
                FMURL_RECORD_UNDO,
                FMURL_FORM_SORT_UP,
                FMURL_FORM_SORT_DOWN,
                FMURL_FORM_SORT,
                FMURL_FORM_AUTO_FILTER,
                FMURL_FORM_FILTER,
                FMURL_FORM_APPLY_FILTER,
                FMURL_FORM_REMOVE_FILTER
            };
            static const sal_Int32 aSlots[] =
            {
                SID_FM_RECORD_ABSOLUTE,
                SID_FM_RECORD_TOTAL,
                SID_FM_RECORD_FIRST,
                SID_FM_RECORD_PREV,
                SID_FM_RECORD_NEXT,
                SID_FM_RECORD_LAST,
                SID_FM_RECORD_NEW,
                SID_FM_RECORD_SAVE,
                SID_FM_RECORD_DELETE,
                SID_FM_REFRESH,
                SID_FM_RECORD_UNDO,
                SID_FM_SORTUP,
                SID_FM_SORTDOWN,
                SID_FM_ORDERCRIT,
                SID_FM_AUTOFILTER,
                SID_FM_FILTERCRIT,
                SID_FM_FORM_FILTERED,
                SID_FM_REMOVE_FILTER_SORT
            };
            if ( _ppURLs )    *_ppURLs    = aURLs;
            if ( _ppURLsEnd ) *_ppURLsEnd = aURLs + ( sizeof( aURLs ) / sizeof( aURLs[0] ) );
            if ( _ppIds )     *_ppIds     = aSlots;
            if ( _ppIdsEnd )  *_ppIdsEnd  = aSlots + ( sizeof( aSlots ) / sizeof( aSlots[0] ) );

            OSL_ENSURE( ( sizeof( aURLs ) / sizeof( aURLs[0] ) ) == ( sizeof( aSlots ) / sizeof( aSlots[0] ) ),
                "FeatureSlotTranslation::getTranslationTables: inconsistence!" );
        }
    }

    //--------------------------------------------------------------------
    sal_Int32 FeatureSlotTranslation::getControllerFeatureIdForURL( const ::rtl::OUString& _rMainURL )
    {
        const ::rtl::OUString* pURLs    = NULL;
        const ::rtl::OUString* pURLsEnd = NULL;
        const sal_Int32*       pIds     = NULL;
        getTranslationTables( &pURLs, &pURLsEnd, &pIds, NULL );

        for ( ; pURLs < pURLsEnd; ++pURLs, ++pIds )
        {
            if ( _rMainURL.equals( *pURLs ) )
                return *pIds;
        }
        return -1;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString  FeatureSlotTranslation::getControllerFeatureURLForId( sal_Int32 _nId )
    {
        const ::rtl::OUString* pURLs    = NULL;
        const sal_Int32*       pIds     = NULL;
        const sal_Int32*       pIdsEnd  = NULL;
        getTranslationTables( &pURLs, NULL, &pIds, &pIdsEnd );

        for ( ; pIds < pIdsEnd; ++pURLs, ++pIds )
        {
            if ( _nId == *pIds )
                return *pURLs;
        }
        return ::rtl::OUString();
    }

    //--------------------------------------------------------------------
    sal_Bool FeatureSlotTranslation::isFeatureURL( const ::rtl::OUString& _rMainURL )
    {
        return ( _rMainURL.indexOf( FMURL_FORMSLOTS_PREFIX ) == 0 );
    }

    //====================================================================
    //= ControllerFeatures
    //====================================================================
    //--------------------------------------------------------------------
    ControllerFeatures::ControllerFeatures( const Reference< XMultiServiceFactory >& _rxORB, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_xORB( _rxORB )
        ,m_pInvalidationCallback( _pInvalidationCallback )
        ,m_pImpl( NULL )
    {
    }

    //--------------------------------------------------------------------
    ControllerFeatures::ControllerFeatures( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XFormController >& _rxController, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_xORB( _rxORB )
        ,m_pInvalidationCallback( _pInvalidationCallback )
        ,m_pImpl( NULL )
    {
        assign( _rxController );
    }

    //--------------------------------------------------------------------
    ControllerFeatures::ControllerFeatures( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XForm >& _rxForm, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_xORB( _rxORB )
        ,m_pInvalidationCallback( _pInvalidationCallback )
        ,m_pImpl( NULL )
    {
        assign( _rxForm );
    }

    //--------------------------------------------------------------------
    void ControllerFeatures::assign( const Reference< XFormController >& _rxController )
    {
        dispose();
        m_pImpl = new FormControllerHelper( m_xORB, _rxController, m_pInvalidationCallback );
        m_pImpl->acquire();
    }

    //--------------------------------------------------------------------
    void ControllerFeatures::assign( const Reference< XForm >& _rxForm )
    {
        dispose();
        m_pImpl = new FormControllerHelper( m_xORB, _rxForm, m_pInvalidationCallback );
        m_pImpl->acquire();
    }

    //--------------------------------------------------------------------
    ControllerFeatures::~ControllerFeatures()
    {
        dispose();
    }

    //--------------------------------------------------------------------
    void ControllerFeatures::dispose()
    {
        if ( m_pImpl )
        {
            m_pImpl->dispose();
            m_pImpl->release();
            m_pImpl = NULL;
        }
    }

    //====================================================================
    //= FormControllerHelper
    //====================================================================
    //--------------------------------------------------------------------
    FormControllerHelper::FormControllerHelper( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XFormController >& _rxController, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_xORB( _rxORB )
        ,m_bInitializedParser( sal_False )
        ,m_bActiveControlModified( sal_False )
        ,m_pInvalidationCallback( _pInvalidationCallback )
    {
        OSL_ENSURE( m_xORB.is(), "FormControllerHelper::FormControllerHelper: gimme a service factory!" );

        initController( _rxController );

        // TODO: add as dispose listener to the controller/form
        // Or should we define that this is the responsibility of our owner?

        m_pDbTools = new ::svxform::OStaticDataAccessTools;
    }

    //--------------------------------------------------------------------
    FormControllerHelper::FormControllerHelper( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XForm >& _rxForm, IControllerFeatureInvalidation* _pInvalidationCallback )
        :m_xORB( _rxORB )
        ,m_bInitializedParser( sal_False )
        ,m_bActiveControlModified( sal_False )
        ,m_pInvalidationCallback( _pInvalidationCallback )
    {
        OSL_ENSURE( m_xORB.is(), "FormControllerHelper::FormControllerHelper: gimme a service factory!" );

        initCursor( _rxForm.get() );

        // TODO: add as dispose listener to the form
        // Or should we define that this is the responsibility of our owner?

        m_pDbTools = new ::svxform::OStaticDataAccessTools;
    }

    //--------------------------------------------------------------------
    FormControllerHelper::~FormControllerHelper( )
    {
        if ( m_xController.is() || hasCursor() )
        {   // not yet disposed
            acquire();
            dispose();
        }

        if ( m_pDbTools )
        {
            delete m_pDbTools;
            m_pDbTools = NULL;
        }
    }

    //------------------------------------------------------------------------------
    void SAL_CALL FormControllerHelper::cursorMoved(const EventObject& /*event*/) throw( RuntimeException )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        OSL_ENSURE( m_xCursor.is(), "FormControllerHelper::cursorMoved: already disposed!" );
        m_bActiveControlModified = sal_False;

        invalidateAllSupportedFeatures( aGuard );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormControllerHelper::rowChanged( const EventObject& /*event*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormControllerHelper::rowSetChanged( const EventObject& /*event*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormControllerHelper::modified( const EventObject& /*_rSource*/ ) throw( RuntimeException )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        OSL_ENSURE( m_xCursor.is(), "FormControllerHelper::modified: already disposed!" );
        if ( !m_bActiveControlModified )
        {
            m_bActiveControlModified = sal_True;
            invalidateModifyDependentFeatures( aGuard );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormControllerHelper::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        if ( m_xCursor.is() && ( m_xCursor == _rEvent.Source ) )
        {
            sal_Bool bIs = sal_False;
            if  ( ( _rEvent.PropertyName == FM_PROP_ISMODIFIED )
               || ( _rEvent.PropertyName == FM_PROP_ISNEW )
                )
            {
                if ( ( _rEvent.NewValue >>= bIs ) && !bIs )
                    m_bActiveControlModified = sal_False;
            }
            invalidateAllSupportedFeatures( aGuard );
        }
        if ( m_xParser.is() && ( m_xCursor == _rEvent.Source ) )
        {
            try
            {
                ::rtl::OUString sNewValue;
                _rEvent.NewValue >>= sNewValue;
                if ( _rEvent.PropertyName == FM_PROP_ACTIVECOMMAND )
                {
                    m_xParser->setElementaryQuery( sNewValue );
                }
                else if ( _rEvent.PropertyName == FM_PROP_FILTER_CRITERIA )
                {
                    if ( m_xParser->getFilter() != sNewValue )
                        m_xParser->setFilter( sNewValue );
                }
                else if ( _rEvent.PropertyName == FM_PROP_SORT )
                {
                    _rEvent.NewValue >>= sNewValue;
                    if ( m_xParser->getOrder() != sNewValue )
                        m_xParser->setOrder( sNewValue );
                }

                invalidateAllSupportedFeatures( aGuard );
            }
            catch( Exception& )
            {
                OSL_ENSURE( sal_False, "FormControllerHelper::propertyChange: caught an exception while updating the parser!" );
            }

        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormControllerHelper::disposing( const EventObject& /*_rSource*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        disposeParser();

        // revoke various listeners
        if ( m_xCursor.is() )
            m_xCursor->removeRowSetListener( this );

        if ( m_xCursorProperties.is() )
        {
            m_xCursorProperties->removePropertyChangeListener( FM_PROP_ISMODIFIED,this );
            m_xCursorProperties->removePropertyChangeListener( FM_PROP_ISNEW, this );
        }

        Reference< XModifyBroadcaster > xBroadcaster( m_xController, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->removeModifyListener( this );

        m_xController.clear();
        m_xCursor.clear();
        m_xUpdateCursor.clear();
        m_xCursorProperties.clear();
        m_xLoadableForm.clear();

        m_bActiveControlModified = sal_True;
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::initController( const Reference< XFormController >& _rxController )
    {
        m_xController = _rxController;
        OSL_ENSURE( m_xController.is(), "FormControllerHelper::initController: invalid controller!" );
        if ( m_xController.is() )
            initCursor( m_xController->getModel().get() );

        Reference< XModifyBroadcaster > xBroadcaster( m_xController, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->addModifyListener( this );
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::initCursor( const Reference< XInterface >& _rxForm )
    {
        m_xCursor           = m_xCursor.query           ( _rxForm );
        m_xCursorProperties = m_xCursorProperties.query ( m_xCursor );
        m_xUpdateCursor     = m_xUpdateCursor.query     ( m_xCursor );
        m_xLoadableForm     = m_xLoadableForm.query     ( m_xCursor );

        OSL_ENSURE( m_xCursor.is() && m_xCursorProperties.is() && m_xLoadableForm.is(),
            "FormControllerHelper::initCursor: could not obtain necessary cursor interfaces!" );

        if ( m_xCursorProperties.is() )
        {
            m_xCursorProperties->addPropertyChangeListener( FM_PROP_ISMODIFIED,this );
            m_xCursorProperties->addPropertyChangeListener( FM_PROP_ISNEW, this );
        }

        if ( m_xCursor.is() )
            m_xCursor->addRowSetListener( this );
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::getSimpleState( sal_Int32 _nFeatureId ) const
    {
        ControllerFeatureState aState;
        getState( _nFeatureId, aState );
        return aState.bEnabled;
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::getState( sal_Int32 _nFeatureId, ControllerFeatureState& _rState ) const
    {
        _rState.aState.clear();
        _rState.bEnabled = sal_False;

        ::osl::MutexGuard aGuard( m_aMutex );
        try
        {
            // some checks for basic pre-requisites
            if  (   !m_xLoadableForm.is()
                ||  !m_xLoadableForm->isLoaded()
                ||  !m_xCursorProperties.is()
                )
            {
                return;
            }

            switch ( _nFeatureId )
            {
            case SID_FM_RECORD_FIRST:
            case SID_FM_RECORD_PREV:
                _rState.bEnabled = canMoveLeft( );
                break;

            case SID_FM_RECORD_NEXT:
                if ( isNewRecord() && m_bActiveControlModified )
                    // if the form is on the insert row, and the current
                    // control is modified, then the slot is enabled, too
                    _rState.bEnabled = sal_True;
                else
                    _rState.bEnabled = canMoveRight();
                break;

            case SID_FM_RECORD_LAST:
                _rState.bEnabled = getRecordCount() && ( !m_xCursor->isLast() || isNewRecord() );
                break;

            case SID_FM_RECORD_DELETE:
                // already deleted ?
                if ( m_xCursor->rowDeleted() )
                    _rState.bEnabled = sal_False;
                else
                {
                    // allowed to delete the row ?
                    _rState.bEnabled = !isNewRecord() && m_pDbTools->canDelete( m_xCursorProperties );
                }
                break;

            case SID_FM_RECORD_NEW:
                // if we are inserting we can move to the next row if the current record or control is modified
                _rState.bEnabled = isNewRecord()
                                ?  isModifiedRecord() || m_bActiveControlModified
                                :  m_pDbTools->canInsert( m_xCursorProperties );
                break;

            case SID_FM_REFRESH:
            {
                // there must be an active connection
                Reference< XRowSet > xCursorRowSet( m_xCursor, UNO_QUERY );
                _rState.bEnabled = m_pDbTools->getRowSetConnection( xCursorRowSet ).is();

                // and an active command
                ::rtl::OUString sActiveCommand;
                m_xCursorProperties->getPropertyValue( FM_PROP_ACTIVECOMMAND ) >>= sActiveCommand;
                _rState.bEnabled &= sActiveCommand.getLength() > 0;
            }
            break;

            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
                _rState.bEnabled = isModifiedRecord() || m_bActiveControlModified;
                break;

            case SID_FM_REMOVE_FILTER_SORT:
                if ( isParsable() && hasFilterOrOrder() )
                {
                    _rState.bEnabled = !isInsertOnlyForm();
                }
                break;

            case SID_FM_SORTUP:
            case SID_FM_SORTDOWN:
            case SID_FM_AUTOFILTER:
                if ( m_xController.is() && isParsable() )
                {
                    sal_Bool bIsDeleted = m_xCursor->rowDeleted();

                    if ( !bIsDeleted && !isInsertOnlyForm() )
                    {
                        Reference< XPropertySet > xBoundField = getCurrentBoundField( );
                        if ( xBoundField.is() )
                            xBoundField->getPropertyValue( FM_PROP_SEARCHABLE ) >>= _rState.bEnabled;
                    }
                }
                break;

            case SID_FM_ORDERCRIT:
            case SID_FM_FILTERCRIT:
                if ( isParsable() )
                {
                    _rState.bEnabled = !isInsertOnlyForm();
                }
                break;

            case SID_FM_FORM_FILTERED:
            {
                ::rtl::OUString sFilter;
                m_xCursorProperties->getPropertyValue( FM_PROP_FILTER_CRITERIA ) >>= sFilter;
                if ( sFilter.getLength() )
                {
                    _rState.aState = m_xCursorProperties->getPropertyValue( FM_PROP_APPLYFILTER );
                    _rState.bEnabled = !isInsertOnlyForm();
                }
                else
                    _rState.aState <<= (sal_Bool)sal_False;
            }
            break;

            case SID_FM_RECORD_ABSOLUTE:
            {
                sal_Int32 nPosition   = m_xCursor->getRow();
                sal_Bool  bIsNew      = isNewRecord();
                sal_Int32 nCount      = getRecordCount();
                sal_Bool  bFinalCount = isRecordCountFinal();

                if ( ( nPosition >= 0 ) || bIsNew )
                {
                    if ( bFinalCount )
                    {
                        // special case: there are no records at all, and we
                        // can't insert records -> disabled
                        if ( !nCount && !m_pDbTools->canInsert( m_xCursorProperties ) )
                        {
                            _rState.bEnabled = sal_False;
                        }
                        else
                        {
                            if ( bIsNew )
                                nPosition = ++nCount;
                            _rState.aState <<= (sal_Int32)nPosition;
                            _rState.bEnabled = sal_True;
                        }
                    }
                    else
                    {
                        _rState.aState <<= (sal_Int32)nPosition;
                        _rState.bEnabled = sal_True;
                    }
                }
            }
            break;

            case SID_FM_RECORD_TOTAL:
            {
                sal_Bool  bIsNew      = isNewRecord();
                sal_Int32 nCount      = getRecordCount();
                sal_Bool  bFinalCount = isRecordCountFinal();

                if ( bIsNew )
                    ++nCount;

                ::rtl::OUString sValue = ::rtl::OUString::valueOf( sal_Int32( nCount ) );
                if ( !bFinalCount )
                    sValue += String::CreateFromAscii(" *");

                _rState.aState <<= sValue;
                _rState.bEnabled = sal_True;
            }
            break;

            default:
                OSL_ENSURE( sal_False, "FormControllerHelper::getState: unknown feature id!" );
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormControllerHelper::getState: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------------
    void FormControllerHelper::appendOrderByColumn( const void* _pActionParam ) const
    {
        const param_appendOrderByColumn* pParam = static_cast< const param_appendOrderByColumn* >( _pActionParam );
        m_xParser->appendOrderByColumn( pParam->xField, pParam->bUp );
    }

    //------------------------------------------------------------------------------
    void FormControllerHelper::appendFilterByColumn( const void* _pActionParam ) const
    {
        const param_appendFilterByColumn* pParam = static_cast< const param_appendFilterByColumn* >( _pActionParam );
        m_xParser->appendFilterByColumn( pParam->xField, sal_True );
    }

    //------------------------------------------------------------------------------
    sal_Bool FormControllerHelper::doActionReportError( Action _pAction, const void* _pParam, sal_uInt16 _nErrorResourceId ) const
    {
        sal_Bool bSuccess = sal_False;
        try
        {
            (this->*_pAction)( _pParam );
            bSuccess = sal_True;
        }
        catch( SQLException e )
        {
            if ( _nErrorResourceId )
            {
                String aAdditionalError( SVX_RES( _nErrorResourceId ) );
                SQLContext aExtendedInfo( m_pDbTools->prependContextInfo( e, NULL, aAdditionalError, ::rtl::OUString() ) );

                displayException( aExtendedInfo );
            }
            else
                displayException( e );
        }
        catch( Exception& )
        {
            OSL_ENSURE( sal_False, "FormControllerHelper::doActionReportError: caught a non-SQL exception!" );
        }

        return bSuccess;
    }

    //------------------------------------------------------------------------------
    void FormControllerHelper::executeAutoSort( sal_Bool _bUp ) const
    {
        OSL_PRECOND( m_xController.is(), "FormControllerHelper::executeAutoSort: need a controller for this!" );
        OSL_PRECOND( hasCursor(), "FormControllerHelper::executeAutoSort: need a cursor for this!" );
        OSL_PRECOND( isParsable(), "FormControllerHelper::executeAutoSort: need a parseable statement for this!" );
        if ( !m_xController.is() || !hasCursor() || !isParsable() )
            return;

        Reference< XControl > xControl = m_xController->getCurrentControl();
        if ( xControl.is() && commitCurrentControl() && commitCurrentRecord() )
        {
            Reference< XPropertySet > xBoundField( getCurrentBoundField() );
            if ( xBoundField.is() )
            {
                ::rtl::OUString sOriginalSort;
                m_xCursorProperties->getPropertyValue( FM_PROP_SORT ) >>= sOriginalSort;

                // automatic sort by field always resets the previous sort order
                try
                {
                    m_xParser->setOrder( ::rtl::OUString() );
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "FormControllerHelper::executeAutoFilter: could not reset the parser's order!" );
                }

                param_appendOrderByColumn aParam;
                aParam.xField = xBoundField;
                aParam.bUp = _bUp;
                if ( doActionReportError( (Action)&FormControllerHelper::appendOrderByColumn, static_cast< const void* >( &aParam ), (sal_uInt16)RID_STR_COULDNOTSETORDER ) )
                {
                    WaitObject aWO( NULL );
                    try
                    {
                        m_xCursorProperties->setPropertyValue( FM_PROP_SORT, makeAny( m_xParser->getOrder() ) );
                        m_xLoadableForm->reload();
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "FormControllerHelper::executeAutoSort: caught an exception while setting the parser properties!" );
                    }


                    if ( !m_xLoadableForm->isLoaded() )
                    {   // something went wrong -> restore the original state
                        try
                        {
                            m_xParser->setOrder( sOriginalSort );
                            m_xCursorProperties->setPropertyValue( FM_PROP_SORT, makeAny( m_xParser->getOrder() ) );
                            m_xLoadableForm->reload();
                        }
                        catch( const Exception& )
                        {
                            OSL_ENSURE( sal_False, "FormControllerHelper::executeAutoSort: could not reset the form to it's original state!" );
                        }

                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    void FormControllerHelper::executeAutoFilter( ) const
    {
        OSL_PRECOND( m_xController.is(), "FormControllerHelper::executeAutoFilter: need a controller for this!" );
        OSL_PRECOND( hasCursor(), "FormControllerHelper::executeAutoFilter: need a cursor for this!" );
        OSL_PRECOND( isParsable(), "FormControllerHelper::executeAutoFilter: need a parseable statement for this!" );
        if ( !m_xController.is() || !hasCursor() || !isParsable() )
            return;

        Reference< XControl > xControl = m_xController->getCurrentControl();
        if ( xControl.is() && commitCurrentControl() && commitCurrentRecord() )
        {
            Reference< XPropertySet > xBoundField( getCurrentBoundField() );
            if ( xBoundField.is() )
            {
                ::rtl::OUString sOriginalFilter;
                m_xCursorProperties->getPropertyValue( FM_PROP_FILTER_CRITERIA ) >>= sOriginalFilter;
                sal_Bool bApplied = sal_True;
                m_xCursorProperties->getPropertyValue( FM_PROP_APPLYFILTER ) >>= bApplied;

                // if we have a filter, but it's not applied, then we have to overwrite it, else append one
                if ( !bApplied )
                {
                    try
                    {
                        m_xParser->setFilter( ::rtl::OUString() );
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "FormControllerHelper::executeAutoFilter: could not reset the parser's filter!" );
                    }
                }

                param_appendFilterByColumn aParam;
                aParam.xField = xBoundField;
                if ( doActionReportError( (Action)&FormControllerHelper::appendFilterByColumn, static_cast< const void* >( &aParam ), (sal_uInt16)RID_STR_COULDNOTSETFILTER ) )
                {
                    WaitObject aWO( NULL );
                    try
                    {
                        m_xCursorProperties->setPropertyValue( FM_PROP_FILTER_CRITERIA, makeAny( m_xParser->getFilter() ) );
                        m_xCursorProperties->setPropertyValue( FM_PROP_APPLYFILTER, makeAny( (sal_Bool)sal_True ) );

                        m_xLoadableForm->reload();
                    }
                    catch( const Exception& )
                    {
                        OSL_ENSURE( sal_False, "FormControllerHelper::executeAutoFilter: caught an exception while setting the parser properties!" );
                    }


                    if ( !m_xLoadableForm->isLoaded() )
                    {   // something went wrong -> restore the original state
                        try
                        {
                            m_xParser->setOrder( sOriginalFilter );
                            m_xCursorProperties->setPropertyValue( FM_PROP_APPLYFILTER, makeAny( (sal_Bool)bApplied ) );
                            m_xCursorProperties->setPropertyValue( FM_PROP_FILTER_CRITERIA, makeAny( m_xParser->getFilter() ) );
                            m_xLoadableForm->reload();
                        }
                        catch( const Exception& )
                        {
                            OSL_ENSURE( sal_False, "FormControllerHelper::executeAutoFilter: could not reset the form to it's original state!" );
                        }

                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    Reference< XPropertySet > FormControllerHelper::getCurrentBoundField( ) const
    {
        OSL_PRECOND( m_xController.is(), "FormControllerHelper::getCurrentBoundField: no controller -> no control!" );
        if ( !m_xController.is() )
            return NULL;

        Reference< XControl > xControl( m_xController->getCurrentControl() );

        // special handling for grid controls
        Reference< XGrid > xGrid( xControl, UNO_QUERY );
        Reference< XPropertySet > xControlModel;

        if ( xGrid.is() )
        {
            Reference< XIndexAccess > xColumns( xControl->getModel(), UNO_QUERY );
            sal_Int16 nCurrentPos = xGrid->getCurrentColumnPosition();
            nCurrentPos = GridView2ModelPos( xColumns, nCurrentPos );

            if ( nCurrentPos != (sal_Int16)-1 )
                 xColumns->getByIndex( nCurrentPos ) >>= xControlModel;
        }
        else if ( xControl.is() )
        {
            xControlModel = xControlModel.query( xControl->getModel() );
        }

        Reference< XPropertySet > xField;
        if ( xControlModel.is() && ::comphelper::hasProperty( FM_PROP_BOUNDFIELD, xControlModel ) )
            xControlModel->getPropertyValue( FM_PROP_BOUNDFIELD ) >>= xField;

        return xField;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::commitCurrentControl( ) const
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xController.is(), "FormControllerHelper::commitCurrentControl: no controller!" );
        if ( !m_xController.is() )
            return sal_False;

        Reference< XControl > xCurrentControl( m_xController->getCurrentControl() );

        // check whether the control is locked
        Reference< XBoundControl > xCheckLock( xCurrentControl, UNO_QUERY );
        sal_Bool bControlIsLocked = xCheckLock.is() && xCheckLock->getLock();

        // commit if necessary
        sal_Bool bSuccess = sal_True;
        if ( xCurrentControl.is() && !bControlIsLocked )
        {
            // both the control and it's model can be committable, so try both
            Reference< XBoundComponent > xBound( xCurrentControl, UNO_QUERY );
            if ( !xBound.is() )
                xBound = xBound.query( xCurrentControl->getModel() );
            // and now really commit
            if ( xBound.is() )
            {
                try
                {
                    aGuard.clear();
                    bSuccess = xBound->commit();
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "FormControllerHelper::commitCurrentControl: caught an exception!" );
                }
            }
        }
        return bSuccess;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::commitCurrentRecord( sal_Bool* _pRecordInserted ) const
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( _pRecordInserted )
            *_pRecordInserted = sal_False;

        if ( !hasCursor() )
            return sal_False;

        // nothing to do if the record is not modified
        sal_Bool bResult = !isModifiedRecord();
        if ( !bResult )
        {
            bResult = sal_False;
            try
            {
                // clear our mutex before calling into a foreign component
                Reference< XResultSetUpdate > xUpdate( m_xUpdateCursor );
                aGuard.clear();

                // insert respectively update the row
                if ( isNewRecord() )
                {
                    xUpdate->insertRow();
                    if ( _pRecordInserted )
                        *_pRecordInserted = sal_True;
                }
                else
                    xUpdate->updateRow();
                bResult = sal_True;
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "FormControllerHelper::commitCurrentRecord: caught an exception!" );
            }
        }
        return bResult;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::moveRight( ) const
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        OSL_PRECOND( hasCursor(), "FormControllerHelper::moveRight: no cursor!" );
        if ( !hasCursor() )
            return sal_False;

        sal_Bool bRecordInserted = sal_False;
        sal_Bool bSuccess = commitCurrentRecord( &bRecordInserted );

        if ( bSuccess )
        {
            try
            {
                // clear our mutex before calling into a foreign component
                Reference< XRowSet >          xCursor( m_xCursor       );
                Reference< XResultSetUpdate > xUpdate( m_xUpdateCursor );
                aGuard.clear();

                if ( bRecordInserted )
                {
                    // go to insert row
                    xUpdate->moveToInsertRow();
                }
                else
                {
                    if ( xCursor->isLast() )
                        xUpdate->moveToInsertRow();
                    else
                        xCursor->next();
                }
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "FormControllerHelper::moveRight: caught an exception!");
                bSuccess = sal_False;
            }

        }
        return bSuccess;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::moveLeft( ) const
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        OSL_PRECOND( hasCursor(), "FormControllerHelper::moveLeft: no cursor!" );
        if ( !hasCursor() )
            return sal_False;

        sal_Bool bRecordInserted = sal_False;
        sal_Bool bSuccess = commitCurrentRecord( &bRecordInserted );

        if ( bSuccess )
        {
            try
            {
                if ( bRecordInserted )
                {
                    // retrieve the bookmark of the new record and move previous to that bookmark
                    Reference< XRowLocate > xLocate( m_xCursor, UNO_QUERY );
                    OSL_ENSURE( xLocate.is(), "FormControllerHelper::moveLeft: no XRowLocate!" );
                    aGuard.clear();

                    if ( xLocate.is() )
                        xLocate->moveRelativeToBookmark( xLocate->getBookmark(), -1 );
                }
                else
                {
                    // clear our mutex before calling into a foreign component
                    Reference< XRowSet >          xCursor( m_xCursor       );
                    aGuard.clear();

                    if ( isNewRecord() )
                    {
                        // we assume that the inserted record is now the last record in the
                        // result set
                        xCursor->last();
                    }
                    else
                        xCursor->previous();
                }
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "FormControllerHelper::moveLeft: caught an exception!" );
                bSuccess = sal_False;
            }

        }
        return bSuccess;
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::execute( sal_Int32 _nFeatureId, const ::rtl::OUString& _rParamName,
            const Any& _rParamValue ) const
    {
        Reference< XInterface > xKeepMeAlive( *const_cast< FormControllerHelper* >( this ) );

        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        OSL_ENSURE( hasCursor(), "FormControllerHelper::execute: no cursor!" );
        if ( !hasCursor() )
            return;

        // at the moment we have only one feature which supports execution parameters
        OSL_ENSURE( _nFeatureId == SID_FM_RECORD_ABSOLUTE, "FormControllerHelper::execute: wrong id!" );
        if ( _nFeatureId == SID_FM_RECORD_ABSOLUTE )
        {
            sal_Int32 nPosition = -1;

            OSL_ENSURE( _rParamName.equalsAscii( "Position" ), "FormControllerHelper::execute: invalid parameter!" );
            if ( _rParamName.equalsAscii( "Position" ) )
            {
                _rParamValue >>= nPosition;
                if ( nPosition < 1 )
                    nPosition = 1;
            }

            if ( nPosition != -1 )
            {
                try
                {
                    // commit before doing anything else
                    if ( m_xController.is() && !commitCurrentControl() )
                        return;
                    if ( !commitCurrentRecord() )
                        return;

                    sal_Int32 nCount      = getRecordCount();
                    sal_Bool  bFinalCount = isRecordCountFinal();

                    if ( bFinalCount && ( (sal_Int32)nPosition > nCount ) )
                        nPosition = nCount;

                    m_xCursor->absolute( nPosition );
                }
                catch( const SQLException& )
                {
                    // no need to assert. See the similar block in the other
                    // execute method for an explanation
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "FormControllerHelper::execute: caught an exception!" );
                }
            }
        }
    }

    //--------------------------------------------------------------------
    namespace
    {
        static bool needConfirmCommit( sal_Int32 _nFeatureId )
        {
            return ( ( _nFeatureId == SID_FM_REFRESH )
                  || ( _nFeatureId == SID_FM_REMOVE_FILTER_SORT )
                  || ( _nFeatureId == SID_FM_FORM_FILTERED )
                  || ( _nFeatureId == SID_FM_SORTUP )
                  || ( _nFeatureId == SID_FM_SORTDOWN )
                  || ( _nFeatureId == SID_FM_AUTOFILTER )
                  || ( _nFeatureId == SID_FM_ORDERCRIT )
                  || ( _nFeatureId == SID_FM_FILTERCRIT )
                   );
        }
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::execute( sal_Int32 _nFeatureId ) const
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        OSL_ENSURE( hasCursor(), "FormControllerHelper::execute: no cursor!" );
        if ( !hasCursor() )
            return;

        if ( ( _nFeatureId != SID_FM_RECORD_DELETE ) && ( _nFeatureId != SID_FM_RECORD_UNDO ) )
        {
            // if we have a controller, commit the current control
            if ( m_xController.is() )
                if ( !commitCurrentControl() )
                    return;

            // commit the current record
            bool bCommitCurrentRecord = true;
            // (but before, let the user confirm if necessary)
            if ( isModifiedRecord() )
            {
                if ( needConfirmCommit( _nFeatureId ) )
                {
                    QueryBox aQuery( NULL, SVX_RES( RID_QRY_SAVEMODIFIED ) );
                    switch ( aQuery.Execute() )
                    {
                    case RET_NO: bCommitCurrentRecord = false; break;
                    case RET_CANCEL: return;
                    }
                }
            }
            if ( bCommitCurrentRecord && !commitCurrentRecord() )
                return;
        }

        try
        {
            switch ( _nFeatureId )
            {
            case SID_FM_RECORD_FIRST:
                // move
                m_xCursor->first();
                break;

            case SID_FM_RECORD_NEXT:
            case SID_FM_RECORD_PREV:
                if ( SID_FM_RECORD_PREV == _nFeatureId )
                    moveLeft( );
                else
                    moveRight( );
                // note that moveXXX also does a commitCurrentControl
                break;

            case SID_FM_RECORD_LAST:
            {
/*
                // TODO: re-implement this .....
                // run in an own thread if ...
                // ... the data source is thread safe ...
                sal_Bool bAllowOwnThread = sal_False;
                if ( ::comphelper::hasProperty( FM_PROP_THREADSAFE, m_xCursorProperties ) )
                    m_xCursorProperties->getPropertyValue( FM_PROP_THREADSAFE ) >>= bAllowOwnThread;

                // ... the record count is unknown
                sal_Bool bNeedOwnThread sal_False;
                if ( ::comphelper::hasProperty( FM_PROP_ROWCOUNTFINAL, m_xCursorProperties ) )
                    m_xCursorProperties->getPropertyValue( FM_PROP_ROWCOUNTFINAL ) >>= bNeedOwnThread;

                if ( bNeedOwnThread && bAllowOwnThread )
                    TODO
                else
*/
                    m_xCursor->last();
            }
            break;

            case SID_FM_REFRESH:
                if ( m_xLoadableForm.is() )
                {
                    WaitObject aWO( NULL );
                    m_xLoadableForm->reload();
                }
                break;

            case SID_FM_RECORD_DELETE:
            {
                sal_uInt32 nCount = getRecordCount();

                // next position
                sal_Bool bLeft = m_xCursor->isLast() && ( nCount > 1 );
                sal_Bool bRight= !m_xCursor->isLast();
                sal_Bool bSuccess = sal_False;
                try
                {
                    // ask for confirmation
                    Reference< XConfirmDeleteListener > xConfirmDelete( m_xController, UNO_QUERY );

                    if ( xConfirmDelete.is() )
                    {
                        RowChangeEvent aEvent;
                        aEvent.Source = Reference< XInterface >( m_xCursor, UNO_QUERY );
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
                    bSuccess = sal_False;
                }

                if ( bSuccess )
                {
                    if ( bLeft || bRight )
                        m_xCursor->relative( bRight ? 1 : -1 );
                    else
                    {
                        sal_Bool bCanInsert = m_pDbTools->canInsert( m_xCursorProperties );
                        // is it possible to insert another record?
                        if ( bCanInsert )
                            m_xUpdateCursor->moveToInsertRow();
                        else
                            // move record to update stati
                            m_xCursor->first();
                    }
                }
            }
            break;

            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
            {
                sal_Bool bInserting = isNewRecord();

                if ( SID_FM_RECORD_UNDO == _nFeatureId )
                {
                    if ( !bInserting )
                        m_xUpdateCursor->cancelRowUpdates();

                    // reset all controls for this form
                    resetAllControls( );

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

            case SID_FM_RECORD_NEW:
                // move to the last row before moving to the insert row
                // 21.01.2002 - 96480 - fs@openoffice.org
                m_xCursor->last();
                m_xUpdateCursor->moveToInsertRow();
                break;

            case SID_FM_REMOVE_FILTER_SORT:
            {
                // simultaneously reset Filter and Order property
                Reference< XMultiPropertySet > xProperties( m_xCursorProperties, UNO_QUERY );
                OSL_ENSURE( xProperties.is(), "FormControllerHelper::execute: no multi property access!" );
                if ( xProperties.is() )
                {
                    Sequence< ::rtl::OUString > aNames( 2 );
                    aNames[0] = FM_PROP_FILTER_CRITERIA;
                    aNames[1] = FM_PROP_SORT;

                    Sequence< Any> aValues( 2 );
                    aValues[0] <<= ::rtl::OUString();
                    aValues[1] <<= ::rtl::OUString();

                    WaitObject aWO( NULL );
                    xProperties->setPropertyValues( aNames, aValues );

                    if ( m_xLoadableForm.is() )
                        m_xLoadableForm->reload();
                }
            }
            break;

            case SID_FM_FORM_FILTERED:
                if ( commitCurrentControl() && commitCurrentRecord() )
                {
                    // simply toggle the value
                    sal_Bool bApplied = sal_False;
                    m_xCursorProperties->getPropertyValue( FM_PROP_APPLYFILTER ) >>= bApplied;
                    m_xCursorProperties->setPropertyValue( FM_PROP_APPLYFILTER, makeAny( (sal_Bool)!bApplied ) );

                    // and reload
                    WaitObject aWO( NULL );
                    m_xLoadableForm->reload();
                }
                break;

            case SID_FM_SORTUP:
                executeAutoSort( sal_True );
                break;

            case SID_FM_SORTDOWN:
                executeAutoSort( sal_False );
                break;

            case SID_FM_AUTOFILTER:
                executeAutoFilter();
                break;

            case SID_FM_ORDERCRIT:
                executeFilterOrSort( false );
                break;

            case SID_FM_FILTERCRIT:
                executeFilterOrSort( true );
                break;
            }
        }
        catch( const SQLException& )
        {
            // silent this. SQL exceptions, when they happen, are to be reported
            // to all XSQLErrorListeners of the form. One of these listeners is
            // the form controller, which displays an error message.
            // So there's no reason that we assert here.
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormControllerHelper::execute: caught an exception!" );
        }

        invalidateAllSupportedFeatures( aGuard );
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::executeFilterOrSort( bool _bFilter ) const
    {
        OSL_PRECOND( m_xController.is(), "FormControllerHelper::executeFilterOrSort: need a controller for this!" );
        OSL_PRECOND( hasCursor(),        "FormControllerHelper::executeFilterOrSort: need a cursor for this!" );
        OSL_PRECOND( isParsable(),       "FormControllerHelper::executeFilterOrSort: need a parseable statement for this!" );
        if ( !m_xController.is() || !hasCursor() || !isParsable() )
            return;

        if ( commitCurrentControl() && commitCurrentRecord() && m_xORB.is() )
        {
            try
            {
                PropertyValue aFirst;
                aFirst.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryComposer" ) );
                aFirst.Value <<= m_xParser;

                PropertyValue aSecond;
                aSecond.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowSet" ) );
                aSecond.Value <<= m_xCursorProperties;

                Sequence<Any> aInit(2);
                aInit[0] <<= aFirst;
                aInit[1] <<= aSecond;

                ::rtl::OUString sDialogServiceName;
                if ( _bFilter )
                    sDialogServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.FilterDialog" ) );
                else
                    sDialogServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.OrderDialog" ) );

                Reference< XExecutableDialog> xDialog(
                    m_xORB->createInstanceWithArguments( sDialogServiceName, aInit ), UNO_QUERY
                );

                if ( !xDialog.is() )
                {
                    ShowServiceNotAvailableError( NULL, sDialogServiceName, sal_True );
                    return;
                }

                if ( RET_OK == xDialog->execute() )
                {
                    WaitObject aWO( NULL );
                    if ( _bFilter )
                        m_xCursorProperties->setPropertyValue( FM_PROP_FILTER_CRITERIA, makeAny( m_xParser->getFilter() ) );
                    else
                        m_xCursorProperties->setPropertyValue( FM_PROP_SORT, makeAny( m_xParser->getOrder() ) );
                    m_xLoadableForm->reload();
                }

            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "FormControllerHelper::executeFilterOrSort: caught an exception!" );
            }
        }
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::canMoveRight( ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasCursor() )
        {
            sal_Bool bIsNew = isNewRecord();

            if ( getRecordCount() && !m_xCursor->isLast() && !bIsNew )
                return sal_True;

            if ( m_pDbTools->canInsert( m_xCursorProperties ) )
                if ( !bIsNew || isModifiedRecord() )
                   return sal_True;
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::canMoveLeft( ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasCursor() )
        {
            return getRecordCount() && ( !m_xCursor->isFirst() || isNewRecord() );
        }
        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::isNewRecord() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xCursorProperties.is(), "FormControllerHelper::isNewRecord: no cursor (already disposed?)!" );
        sal_Bool bIsNew = sal_False;
        if ( m_xCursorProperties.is() )
            m_xCursorProperties->getPropertyValue( FM_PROP_ISNEW ) >>= bIsNew;
        return bIsNew;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::isModifiedRecord() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xCursorProperties.is(), "FormControllerHelper::isModifiedRecord: no cursor (already disposed?)!" );
        sal_Bool bIsModified = sal_False;
        if ( m_xCursorProperties.is() )
            m_xCursorProperties->getPropertyValue( FM_PROP_ISMODIFIED ) >>= bIsModified;
        return bIsModified;
    }

    //--------------------------------------------------------------------
    sal_Int32 FormControllerHelper::getRecordCount() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xCursorProperties.is(), "FormControllerHelper::getRecordCount: no cursor (already disposed?)!" );
        sal_Int32 nRecordCount = 0;
        if ( m_xCursorProperties.is() )
            m_xCursorProperties->getPropertyValue( FM_PROP_ROWCOUNT ) >>= nRecordCount;
        return nRecordCount;
    }
    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::isRecordCountFinal() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xCursorProperties.is(), "FormControllerHelper::isRecordCountFinal: no cursor (already disposed?)!" );
        sal_Bool bIsFinal = sal_False;
        if ( m_xCursorProperties.is() )
            m_xCursorProperties->getPropertyValue( FM_PROP_ROWCOUNTFINAL ) >>= bIsFinal;
        return bIsFinal;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::isInsertOnlyForm() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xCursorProperties.is(), "FormControllerHelper::isInsertOnlyForm: no cursor (already disposed?)!" );
        sal_Bool bInsertOnly = sal_True;
        if ( m_xCursorProperties.is() )
            m_xCursorProperties->getPropertyValue( FM_PROP_INSERTONLY ) >>= bInsertOnly;
        return bInsertOnly;
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::isParsable() const
    {
        const_cast< FormControllerHelper* >( this )->ensureInitializedParser();
        return m_xParser.is() && m_xParser->getQuery().getLength();
    }

    //--------------------------------------------------------------------
    sal_Bool FormControllerHelper::hasFilterOrOrder() const
    {
        return isParsable() && ( m_xParser->getFilter().getLength() || m_xParser->getOrder().getLength() );
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::disposeParser()
    {
        try
        {
            // if we have a parser (and a cursor), then we're listening at the cursor's
            // properties to keep the parser in sync with the cursor
            if ( m_xParser.is() && m_xCursorProperties.is() )
            {
                m_xCursorProperties->removePropertyChangeListener( FM_PROP_FILTER_CRITERIA, this );
                m_xCursorProperties->removePropertyChangeListener( FM_PROP_ACTIVECOMMAND, this );
                m_xCursorProperties->removePropertyChangeListener( FM_PROP_SORT, this );
            }

            Reference< XComponent > xParserComp( m_xParser, UNO_QUERY );
            if ( xParserComp.is() )
                xParserComp->dispose();
            m_xParser.clear();

            m_bInitializedParser = sal_False;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormControllerHelper::disposeParser: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::ensureInitializedParser()
    {
        if ( m_bInitializedParser )
            return;

        try
        {
            sal_Bool bUseEscapeProcessing = sal_False;
            m_xCursorProperties->getPropertyValue( FM_PROP_ESCAPE_PROCESSING ) >>= bUseEscapeProcessing;
            if ( bUseEscapeProcessing )
            {
                Reference< XRowSet > xCursorRowSet( m_xCursor, UNO_QUERY );
                Reference< XMultiServiceFactory > xFactory( m_pDbTools->getRowSetConnection( xCursorRowSet ), UNO_QUERY );
                if ( xFactory.is() )
                {
                    m_xParser.set( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.SingleSelectQueryComposer" ) ) ), UNO_QUERY );
                    DBG_ASSERT( m_xParser.is(), "FormControllerHelper::ensureInitializedParser: factory did not create a parser for us!" );
                }
            }

            if ( m_xParser.is() )
            {
                if ( m_xLoadableForm.is() && m_xLoadableForm->isLoaded() )
                {
                    ::rtl::OUString sStatement;
                    ::rtl::OUString sFilter;
                    ::rtl::OUString sSort;

                    m_xCursorProperties->getPropertyValue( FM_PROP_ACTIVECOMMAND   ) >>= sStatement;
                    m_xCursorProperties->getPropertyValue( FM_PROP_FILTER_CRITERIA ) >>= sFilter;
                    m_xCursorProperties->getPropertyValue( FM_PROP_SORT            ) >>= sSort;

                    m_xParser->setQuery ( sStatement );
                    m_xParser->setFilter( sFilter    );
                    m_xParser->setOrder ( sSort      );
                }

                // start listening at the properties order/sort properties at the form, so
                // we can keep our parser in sync
                m_xCursorProperties->addPropertyChangeListener( FM_PROP_ACTIVECOMMAND, this );
                m_xCursorProperties->addPropertyChangeListener( FM_PROP_FILTER_CRITERIA, this );
                m_xCursorProperties->addPropertyChangeListener( FM_PROP_SORT, this );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormControllerHelper::ensureInitializedParser: caught an exception!" );
        }

        m_bInitializedParser = sal_True;
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::resetAllControls( ) const
    {
        resetAllControls( Reference< XForm >( m_xCursor, UNO_QUERY ) );
    }

    //--------------------------------------------------------------------
    void FormControllerHelper::resetAllControls( const Reference< XForm >& _rxForm )
    {
        Reference< XIndexAccess > xContainer( _rxForm, UNO_QUERY );
        if ( xContainer.is() )
        {
            Reference< XReset > xReset;
            for ( sal_Int32 i = 0; i < xContainer->getCount(); ++i )
            {
                if ( ( xContainer->getByIndex( i ) >>= xReset ) && xReset.is() )
                {
                    // no resets on sub forms
                    Reference< XForm > xAsForm( xReset, UNO_QUERY );
                    if ( !xAsForm.is() )
                        xReset->reset();
                }
            }
        }

    }

    //------------------------------------------------------------------------------
    void FormControllerHelper::invalidateAllSupportedFeatures( ::osl::ClearableMutexGuard& _rClearForCallback ) const
    {
        if ( !m_pInvalidationCallback )
            // nobody's interested in ...
            return;

        // actually, it's a little bit more than the supported features,
        // but on the medium term, we are to support everything listed
        // here
        static ::std::vector< sal_Int32 > aSupportedFeatures;
        if ( aSupportedFeatures.empty() )
        {
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
                SID_FM_SEARCH,
                SID_FM_FILTER_START,
                SID_FM_VIEW_AS_GRID
            };
            sal_Int32 nFeatureCount = sizeof( pSupportedFeatures ) / sizeof( pSupportedFeatures[ 0 ] );
            aSupportedFeatures.resize( nFeatureCount );
            ::std::copy( pSupportedFeatures, pSupportedFeatures + nFeatureCount, aSupportedFeatures.begin() );
        }

        IControllerFeatureInvalidation* pCallback = m_pInvalidationCallback;
        _rClearForCallback.clear();

        pCallback->invalidateFeatures( aSupportedFeatures );
    }

    //------------------------------------------------------------------------------
    void FormControllerHelper::invalidateModifyDependentFeatures( ::osl::ClearableMutexGuard& _rClearForCallback ) const
    {
        if ( !m_pInvalidationCallback )
            // nobody's interested in ...
            return;

        static ::std::vector< sal_Int32 > aModifyDependentFeatures;
        if ( aModifyDependentFeatures.empty() )
        {
            sal_uInt16 pModifyDependentFeatures[] = // slots des Controllers
            {
                SID_FM_RECORD_NEXT,
                SID_FM_RECORD_NEW,
                SID_FM_RECORD_SAVE,
                SID_FM_RECORD_UNDO
            };
            sal_Int32 nFeatureCount = sizeof( pModifyDependentFeatures ) / sizeof( pModifyDependentFeatures[ 0 ] );
            aModifyDependentFeatures.resize( nFeatureCount );
            ::std::copy( pModifyDependentFeatures, pModifyDependentFeatures + nFeatureCount, aModifyDependentFeatures.begin() );
        }

        IControllerFeatureInvalidation* pCallback = m_pInvalidationCallback;
        _rClearForCallback.clear();

        pCallback->invalidateFeatures( aModifyDependentFeatures );
    }

//........................................................................
}   // namespace svx
//........................................................................
