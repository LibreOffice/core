/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formnavigation.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:56:20 $
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
#include "precompiled_forms.hxx"

#ifndef FORMS_FORM_NAVIGATION_HXX
#include "formnavigation.hxx"
#endif
#ifndef FORMS_SOURCE_INC_URLTRANSFORMER_HXX
#include "urltransformer.hxx"
#endif
#ifndef FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX
#include "controlfeatureinterception.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef FRM_STRINGS_HXX
#include "frm_strings.hxx"
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;

    //==================================================================
    //= OFormNavigationHelper
    //==================================================================
    DBG_NAME( OFormNavigationHelper )
    //------------------------------------------------------------------
    OFormNavigationHelper::OFormNavigationHelper( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
        ,m_nConnectedFeatures( 0 )
    {
        DBG_CTOR( OFormNavigationHelper, NULL );
        m_pFeatureInterception.reset( new ControlFeatureInterception( m_xORB ) );
    }

    //------------------------------------------------------------------
    OFormNavigationHelper::~OFormNavigationHelper()
    {
        DBG_DTOR( OFormNavigationHelper, NULL );
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormNavigationHelper::dispose( ) throw( RuntimeException )
    {
        m_pFeatureInterception->dispose();
        disconnectDispatchers();
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::interceptorsChanged( )
    {
        updateDispatches();
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::featureStateChanged( sal_Int32 /*_nFeatureId*/, sal_Bool /*_bEnabled*/ )
    {
        // not interested in
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::allFeatureStatesChanged( )
    {
        // not interested in
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormNavigationHelper::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        m_pFeatureInterception->registerDispatchProviderInterceptor( _rxInterceptor );
        interceptorsChanged();
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormNavigationHelper::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        m_pFeatureInterception->releaseDispatchProviderInterceptor( _rxInterceptor );
        interceptorsChanged();
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormNavigationHelper::statusChanged( const FeatureStateEvent& _rState ) throw (RuntimeException)
    {
        for (   FeatureMap::iterator aFeature = m_aSupportedFeatures.begin();
                aFeature != m_aSupportedFeatures.end();
                ++aFeature
            )
        {
            if ( aFeature->second.aURL.Main == _rState.FeatureURL.Main )
            {
                if  (  ( aFeature->second.bCachedState != _rState.IsEnabled )
                    || ( aFeature->second.aCachedAdditionalState != _rState.State )
                    )
                {
                    // change the cached state
                    aFeature->second.bCachedState           = _rState.IsEnabled;
                    aFeature->second.aCachedAdditionalState = _rState.State;
                    // tell derivees what happened
                    featureStateChanged( aFeature->first, _rState.IsEnabled );
                }
                return;
            }
        }

        // unreachable
        DBG_ERROR( "OFormNavigationHelper::statusChanged: huh? An invalid/unknown URL?" );
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormNavigationHelper::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        // was it one of our external dispatchers?
        if ( m_nConnectedFeatures )
        {
            for (   FeatureMap::iterator aFeature = m_aSupportedFeatures.begin();
                    aFeature != m_aSupportedFeatures.end();
                    ++aFeature
                )
            {
                if ( aFeature->second.xDispatcher == _rSource.Source )
                {
                    aFeature->second.xDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), aFeature->second.aURL );
                    aFeature->second.xDispatcher = NULL;
                    aFeature->second.bCachedState = sal_False;
                    aFeature->second.aCachedAdditionalState.clear();
                    --m_nConnectedFeatures;

                    featureStateChanged( aFeature->first, sal_False );
                    break;
                }
            }
        }
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::updateDispatches()
    {
        if ( !m_nConnectedFeatures )
        {   // we don't have any dispatchers yet -> do the initial connect
            connectDispatchers();
            return;
        }

        initializeSupportedFeatures();

        m_nConnectedFeatures = 0;

        Reference< XDispatch >  xNewDispatcher;
        Reference< XDispatch >  xCurrentDispatcher;

        for (   FeatureMap::iterator aFeature = m_aSupportedFeatures.begin();
                aFeature != m_aSupportedFeatures.end();
                ++aFeature
            )
        {
            xNewDispatcher = queryDispatch( aFeature->second.aURL );
            xCurrentDispatcher = aFeature->second.xDispatcher;
            if ( xNewDispatcher != xCurrentDispatcher )
            {
                // the dispatcher for this particular URL changed
                if ( xCurrentDispatcher.is() )
                    xCurrentDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), aFeature->second.aURL );

                xCurrentDispatcher = aFeature->second.xDispatcher = xNewDispatcher;

                if ( xCurrentDispatcher.is() )
                    xCurrentDispatcher->addStatusListener( static_cast< XStatusListener* >( this ), aFeature->second.aURL );
            }

            if ( xCurrentDispatcher.is() )
                ++m_nConnectedFeatures;
            else
                aFeature->second.bCachedState = sal_False;
        }

        // notify derivee that (potentially) all features changed their state
        allFeatureStatesChanged( );
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::connectDispatchers()
    {
        if ( m_nConnectedFeatures )
        {   // already connected -> just do an update
            updateDispatches();
            return;
        }

        initializeSupportedFeatures();

        m_nConnectedFeatures = 0;

        for (   FeatureMap::iterator aFeature = m_aSupportedFeatures.begin();
                aFeature != m_aSupportedFeatures.end();
                ++aFeature
            )
        {
            aFeature->second.bCachedState = sal_False;
            aFeature->second.aCachedAdditionalState.clear();
            aFeature->second.xDispatcher = queryDispatch( aFeature->second.aURL );
            if ( aFeature->second.xDispatcher.is() )
            {
                ++m_nConnectedFeatures;
                aFeature->second.xDispatcher->addStatusListener( static_cast< XStatusListener* >( this ), aFeature->second.aURL );
            }
        }

        // notify derivee that (potentially) all features changed their state
        allFeatureStatesChanged( );
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::disconnectDispatchers()
    {
        if ( m_nConnectedFeatures )
        {
            for (   FeatureMap::iterator aFeature = m_aSupportedFeatures.begin();
                    aFeature != m_aSupportedFeatures.end();
                    ++aFeature
                )
            {
                if ( aFeature->second.xDispatcher.is() )
                    aFeature->second.xDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), aFeature->second.aURL );

                aFeature->second.xDispatcher = NULL;
                aFeature->second.bCachedState = sal_False;
                aFeature->second.aCachedAdditionalState.clear();
            }

            m_nConnectedFeatures = 0;
        }

        // notify derivee that (potentially) all features changed their state
        allFeatureStatesChanged( );
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::initializeSupportedFeatures( )
    {
        if ( m_aSupportedFeatures.empty() )
        {
            // ask the derivee which feature ids it wants us to support
            ::std::vector< sal_Int32 > aFeatureIds;
            getSupportedFeatures( aFeatureIds );

            OFormNavigationMapper aUrlMapper( m_xORB );

            for (   ::std::vector< sal_Int32 >::const_iterator aLoop = aFeatureIds.begin();
                    aLoop != aFeatureIds.end();
                    ++aLoop
                )
            {
                FeatureInfo aFeatureInfo;

                bool bKnownId =
                    aUrlMapper.getFeatureURL( *aLoop, aFeatureInfo.aURL );
                DBG_ASSERT( bKnownId, "OFormNavigationHelper::initializeSupportedFeatures: unknown feature id!" );

                if ( bKnownId )
                    // add to our map
                    m_aSupportedFeatures.insert( FeatureMap::value_type( *aLoop, aFeatureInfo ) );
            }
        }
    }

    //------------------------------------------------------------------
    Reference< XDispatch > OFormNavigationHelper::queryDispatch( const URL& _rURL )
    {
        return m_pFeatureInterception->queryDispatch( _rURL );
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::dispatchWithArgument( sal_Int32 _nFeatureId, const sal_Char* _pParamAsciiName,
        const Any& _rParamValue ) const
    {
        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
        {
            if ( aInfo->second.xDispatcher.is() )
            {
                Sequence< PropertyValue > aArgs( 1 );
                aArgs[0].Name = ::rtl::OUString::createFromAscii( _pParamAsciiName );
                aArgs[0].Value = _rParamValue;

                aInfo->second.xDispatcher->dispatch( aInfo->second.aURL, aArgs );
            }
        }
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::dispatch( sal_Int32 _nFeatureId ) const
    {
        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
        {
            if ( aInfo->second.xDispatcher.is() )
            {
                Sequence< PropertyValue > aEmptyArgs;
                aInfo->second.xDispatcher->dispatch( aInfo->second.aURL, aEmptyArgs );
            }
        }
    }

    //------------------------------------------------------------------
    bool OFormNavigationHelper::isEnabled( sal_Int32 _nFeatureId ) const
    {
        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            return aInfo->second.bCachedState;

        return false;
    }

    //------------------------------------------------------------------
    bool OFormNavigationHelper::getBooleanState( sal_Int32 _nFeatureId ) const
    {
        sal_Bool bState = sal_False;

        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            aInfo->second.aCachedAdditionalState >>= bState;

        return (bool)bState;
    }

    //------------------------------------------------------------------
    ::rtl::OUString OFormNavigationHelper::getStringState( sal_Int32 _nFeatureId ) const
    {
        ::rtl::OUString sState;

        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            aInfo->second.aCachedAdditionalState >>= sState;

        return sState;
    }

    //------------------------------------------------------------------
    sal_Int32 OFormNavigationHelper::getIntegerState( sal_Int32 _nFeatureId ) const
    {
        sal_Int32 nState = 0;

        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            aInfo->second.aCachedAdditionalState >>= nState;

        return nState;
    }

    //------------------------------------------------------------------
    void OFormNavigationHelper::invalidateSupportedFeaturesSet()
    {
        disconnectDispatchers( );
        // no supported features anymore:
    FeatureMap aEmpty;
        m_aSupportedFeatures.swap( aEmpty );
    }

    //==================================================================
    //= OFormNavigationMapper
    //==================================================================
    //------------------------------------------------------------------
    OFormNavigationMapper::OFormNavigationMapper( const Reference< XMultiServiceFactory >& _rxORB )
    {
        m_pUrlTransformer.reset( new UrlTransformer( _rxORB ) );
    }

    //------------------------------------------------------------------
    OFormNavigationMapper::~OFormNavigationMapper( )
    {
    }

    //------------------------------------------------------------------
    bool OFormNavigationMapper::getFeatureURL( sal_Int32 _nFeatureId, URL& /* [out] */ _rURL )
    {
        // get the ascii version of the URL
        const char* pAsciiURL = getFeatureURLAscii( _nFeatureId );
        if ( pAsciiURL )
            _rURL = m_pUrlTransformer->getStrictURLFromAscii( pAsciiURL );

        return ( pAsciiURL != NULL );
    }

    //------------------------------------------------------------------
    const char* OFormNavigationMapper::getFeatureURLAscii( sal_Int32 _nFeatureId )
    {
        const char* pAsciiURL = NULL;

        switch ( _nFeatureId )
        {
        case SID_FM_RECORD_ABSOLUTE: pAsciiURL = URL_FORM_POSITION;     break;
        case SID_FM_RECORD_TOTAL   : pAsciiURL = URL_FORM_RECORDCOUNT;  break;
        case SID_FM_RECORD_FIRST   : pAsciiURL = URL_RECORD_FIRST;      break;
        case SID_FM_RECORD_PREV    : pAsciiURL = URL_RECORD_PREV;       break;
        case SID_FM_RECORD_NEXT    : pAsciiURL = URL_RECORD_NEXT;       break;
        case SID_FM_RECORD_LAST    : pAsciiURL = URL_RECORD_LAST;       break;
        case SID_FM_RECORD_SAVE    : pAsciiURL = URL_RECORD_SAVE;       break;
        case SID_FM_RECORD_UNDO    : pAsciiURL = URL_RECORD_UNDO;       break;
        case SID_FM_RECORD_NEW     : pAsciiURL = URL_RECORD_NEW;        break;
        case SID_FM_RECORD_DELETE  : pAsciiURL = URL_RECORD_DELETE;     break;
        case SID_FM_REFRESH        : pAsciiURL = URL_FORM_REFRESH;      break;

        case SID_FM_SORTUP             : pAsciiURL = URL_FORM_SORT_UP;       break;
        case SID_FM_SORTDOWN           : pAsciiURL = URL_FORM_SORT_DOWN;     break;
        case SID_FM_ORDERCRIT          : pAsciiURL = URL_FORM_SORT;          break;
        case SID_FM_AUTOFILTER         : pAsciiURL = URL_FORM_AUTO_FILTER;   break;
        case SID_FM_FILTERCRIT         : pAsciiURL = URL_FORM_FILTER;        break;
        case SID_FM_FORM_FILTERED      : pAsciiURL = URL_FORM_APPLY_FILTER;  break;
        case SID_FM_REMOVE_FILTER_SORT : pAsciiURL = URL_FORM_REMOVE_FILTER; break;
        };
        return pAsciiURL;
    }

    //------------------------------------------------------------------
    sal_Int32 OFormNavigationMapper::getFeatureId( const ::rtl::OUString& _rCompleteURL )
    {
        sal_Int32 nFeatureId = -1;

        if ( _rCompleteURL == URL_FORM_POSITION )
            nFeatureId = SID_FM_RECORD_ABSOLUTE;
        else if ( _rCompleteURL == URL_FORM_RECORDCOUNT )
            nFeatureId = SID_FM_RECORD_TOTAL;
        else if ( _rCompleteURL == URL_RECORD_FIRST )
            nFeatureId = SID_FM_RECORD_FIRST;
        else if ( _rCompleteURL == URL_RECORD_PREV )
            nFeatureId = SID_FM_RECORD_PREV;
        else if ( _rCompleteURL == URL_RECORD_NEXT )
            nFeatureId = SID_FM_RECORD_NEXT;
        else if ( _rCompleteURL == URL_RECORD_LAST )
            nFeatureId = SID_FM_RECORD_LAST;
        else if ( _rCompleteURL == URL_RECORD_SAVE )
            nFeatureId = SID_FM_RECORD_SAVE;
        else if ( _rCompleteURL == URL_RECORD_UNDO )
            nFeatureId = SID_FM_RECORD_UNDO;
        else if ( _rCompleteURL == URL_RECORD_NEW )
            nFeatureId = SID_FM_RECORD_NEW;
        else if ( _rCompleteURL == URL_RECORD_DELETE )
            nFeatureId = SID_FM_RECORD_DELETE;
        else if ( _rCompleteURL == URL_FORM_REFRESH )
            nFeatureId = SID_FM_REFRESH;
        else if ( _rCompleteURL == URL_FORM_SORT_UP )
            nFeatureId = SID_FM_SORTUP;
        else if ( _rCompleteURL == URL_FORM_SORT_DOWN )
            nFeatureId = SID_FM_SORTDOWN;
        else if ( _rCompleteURL == URL_FORM_SORT )
            nFeatureId = SID_FM_ORDERCRIT;
        else if ( _rCompleteURL == URL_FORM_AUTO_FILTER )
            nFeatureId = SID_FM_AUTOFILTER;
        else if ( _rCompleteURL == URL_FORM_FILTER )
            nFeatureId = SID_FM_FILTERCRIT;
        else if ( _rCompleteURL == URL_FORM_APPLY_FILTER )
            nFeatureId = SID_FM_FORM_FILTERED;
        else if ( _rCompleteURL == URL_FORM_REMOVE_FILTER )
            nFeatureId = SID_FM_REMOVE_FILTER_SORT;

        DBG_ASSERT( ( -1 == nFeatureId ) || _rCompleteURL.equalsAscii( getFeatureURLAscii( nFeatureId ) ),
            "OFormNavigationMapper::getFeatureId: inconsistent maps!" );

        return nFeatureId;
    }

//.........................................................................
}   // namespace frm
//.........................................................................
