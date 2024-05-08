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

#include <formnavigation.hxx>
#include <urltransformer.hxx>
#include <controlfeatureinterception.hxx>
#include <frm_strings.hxx>

#include <com/sun/star/form/runtime/FormFeature.hpp>

#include <comphelper/propertyvalue.hxx>
#include <tools/debug.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;

    OFormNavigationHelper::OFormNavigationHelper( const Reference< XComponentContext >& _rxORB )
        :m_xORB( _rxORB )
        ,m_aFeatureInterception( m_xORB )
        ,m_nConnectedFeatures( 0 )
    {
    }


    OFormNavigationHelper::~OFormNavigationHelper()
    {
    }


    void OFormNavigationHelper::dispose( )
    {
        m_aFeatureInterception.dispose();
        disconnectDispatchers();
    }


    void OFormNavigationHelper::interceptorsChanged( )
    {
        updateDispatches();
    }


    void OFormNavigationHelper::featureStateChanged( sal_Int16 /*_nFeatureId*/, bool /*_bEnabled*/ )
    {
        // not interested in
    }


    void OFormNavigationHelper::allFeatureStatesChanged( )
    {
        // not interested in
    }


    void SAL_CALL OFormNavigationHelper::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
    {
        m_aFeatureInterception.registerDispatchProviderInterceptor( _rxInterceptor );
        interceptorsChanged();
    }


    void SAL_CALL OFormNavigationHelper::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
    {
        m_aFeatureInterception.releaseDispatchProviderInterceptor( _rxInterceptor );
        interceptorsChanged();
    }


    void SAL_CALL OFormNavigationHelper::statusChanged( const FeatureStateEvent& _rState )
    {
        for (auto & feature : m_aSupportedFeatures)
        {
            if ( feature.second.aURL.Main == _rState.FeatureURL.Main )
            {
                if  (  ( feature.second.bCachedState != bool(_rState.IsEnabled) )
                    || ( feature.second.aCachedAdditionalState != _rState.State )
                    )
                {
                    // change the cached state
                    feature.second.bCachedState           = _rState.IsEnabled;
                    feature.second.aCachedAdditionalState = _rState.State;
                    // tell derivees what happened
                    featureStateChanged( feature.first, _rState.IsEnabled );
                }
                return;
            }
        }

        // unreachable
        OSL_FAIL( "OFormNavigationHelper::statusChanged: huh? An invalid/unknown URL?" );
    }


    void SAL_CALL OFormNavigationHelper::disposing( const EventObject& _rSource )
    {
        // was it one of our external dispatchers?
        if ( !m_nConnectedFeatures )
            return;

        for (auto & feature : m_aSupportedFeatures)
        {
            if ( feature.second.xDispatcher == _rSource.Source )
            {
                feature.second.xDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), feature.second.aURL );
                feature.second.xDispatcher = nullptr;
                feature.second.bCachedState = false;
                feature.second.aCachedAdditionalState.clear();
                --m_nConnectedFeatures;

                featureStateChanged( feature.first, false );
                break;
            }
        }
    }


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

        for (auto & feature : m_aSupportedFeatures)
        {
            xNewDispatcher = queryDispatch( feature.second.aURL );
            xCurrentDispatcher = feature.second.xDispatcher;
            if ( xNewDispatcher != xCurrentDispatcher )
            {
                // the dispatcher for this particular URL changed
                if ( xCurrentDispatcher.is() )
                    xCurrentDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), feature.second.aURL );

                xCurrentDispatcher = feature.second.xDispatcher = xNewDispatcher;

                if ( xCurrentDispatcher.is() )
                    xCurrentDispatcher->addStatusListener( static_cast< XStatusListener* >( this ), feature.second.aURL );
            }

            if ( xCurrentDispatcher.is() )
                ++m_nConnectedFeatures;
            else
                feature.second.bCachedState = false;
        }

        // notify derivee that (potentially) all features changed their state
        allFeatureStatesChanged( );
    }


    void OFormNavigationHelper::connectDispatchers()
    {
        if ( m_nConnectedFeatures )
        {   // already connected -> just do an update
            updateDispatches();
            return;
        }

        initializeSupportedFeatures();

        m_nConnectedFeatures = 0;

        for (auto & feature : m_aSupportedFeatures)
        {
            feature.second.bCachedState = false;
            feature.second.aCachedAdditionalState.clear();
            feature.second.xDispatcher = queryDispatch( feature.second.aURL );
            if ( feature.second.xDispatcher.is() )
            {
                ++m_nConnectedFeatures;
                feature.second.xDispatcher->addStatusListener( static_cast< XStatusListener* >( this ), feature.second.aURL );
            }
        }

        // notify derivee that (potentially) all features changed their state
        allFeatureStatesChanged( );
    }


    void OFormNavigationHelper::disconnectDispatchers()
    {
        if ( m_nConnectedFeatures )
        {
            for (auto & feature : m_aSupportedFeatures)
            {
                if ( feature.second.xDispatcher.is() )
                    feature.second.xDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), feature.second.aURL );

                feature.second.xDispatcher = nullptr;
                feature.second.bCachedState = false;
                feature.second.aCachedAdditionalState.clear();
            }

            m_nConnectedFeatures = 0;
        }

        // notify derivee that (potentially) all features changed their state
        allFeatureStatesChanged( );
    }


    void OFormNavigationHelper::initializeSupportedFeatures( )
    {
        if ( !m_aSupportedFeatures.empty() )
            return;

        // ask the derivee which feature ids it wants us to support
        ::std::vector< sal_Int16 > aFeatureIds;
        getSupportedFeatures( aFeatureIds );

        OFormNavigationMapper aUrlMapper( m_xORB );

        for (auto const& feature : aFeatureIds)
        {
            FeatureInfo aFeatureInfo;

            bool bKnownId =
                aUrlMapper.getFeatureURL( feature, aFeatureInfo.aURL );
            DBG_ASSERT( bKnownId, "OFormNavigationHelper::initializeSupportedFeatures: unknown feature id!" );

            if ( bKnownId )
                // add to our map
                m_aSupportedFeatures.emplace( feature, aFeatureInfo );
        }
    }


    Reference< XDispatch > OFormNavigationHelper::queryDispatch( const URL& _rURL )
    {
        return m_aFeatureInterception.queryDispatch( _rURL );
    }


    void OFormNavigationHelper::dispatchWithArgument( sal_Int16 _nFeatureId, const OUString& _pParamAsciiName,
        const Any& _rParamValue ) const
    {
        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
        {
            if ( aInfo->second.xDispatcher.is() )
            {
                Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
                    _pParamAsciiName, _rParamValue) };

                aInfo->second.xDispatcher->dispatch( aInfo->second.aURL, aArgs );
            }
        }
    }


    void OFormNavigationHelper::dispatch( sal_Int16 _nFeatureId ) const
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


    bool OFormNavigationHelper::isEnabled( sal_Int16 _nFeatureId ) const
    {
        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            return aInfo->second.bCachedState;

        return false;
    }


    bool OFormNavigationHelper::getBooleanState( sal_Int16 _nFeatureId ) const
    {
        bool bState = false;

        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            aInfo->second.aCachedAdditionalState >>= bState;

        return bState;
    }


    OUString OFormNavigationHelper::getStringState( sal_Int16 _nFeatureId ) const
    {
        OUString sState;

        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            aInfo->second.aCachedAdditionalState >>= sState;

        return sState;
    }


    sal_Int32 OFormNavigationHelper::getIntegerState( sal_Int16 _nFeatureId ) const
    {
        sal_Int32 nState = 0;

        FeatureMap::const_iterator aInfo = m_aSupportedFeatures.find( _nFeatureId );
        if ( m_aSupportedFeatures.end() != aInfo )
            aInfo->second.aCachedAdditionalState >>= nState;

        return nState;
    }


    void OFormNavigationHelper::invalidateSupportedFeaturesSet()
    {
        disconnectDispatchers( );
        // no supported features anymore:
        FeatureMap().swap(m_aSupportedFeatures);
    }

    OFormNavigationMapper::OFormNavigationMapper( const Reference< XComponentContext >& _rxORB )
    {
        m_pUrlTransformer.reset( new UrlTransformer( _rxORB ) );
    }


    OFormNavigationMapper::~OFormNavigationMapper( )
    {
    }


    bool OFormNavigationMapper::getFeatureURL( sal_Int16 _nFeatureId, URL& /* [out] */ _rURL )
    {
        // get the ascii version of the URL
        std::optional<OUString> pAsciiURL = getFeatureURL( _nFeatureId );
        if ( pAsciiURL )
            _rURL = m_pUrlTransformer->getStrictURL( *pAsciiURL );

        return pAsciiURL.has_value();
    }


    namespace
    {
        struct FeatureURL
        {
            const sal_Int16 nFormFeature;
            OUString   aAsciiURL;
        };
        constexpr FeatureURL s_aFeatureURLs[]
        {
            { FormFeature::MoveAbsolute,            u".uno:FormController/positionForm"_ustr },
            { FormFeature::TotalRecords,            u".uno:FormController/RecordCount"_ustr },
            { FormFeature::MoveToFirst,             u".uno:FormController/moveToFirst"_ustr },
            { FormFeature::MoveToPrevious,          u".uno:FormController/moveToPrev"_ustr },
            { FormFeature::MoveToNext,              u".uno:FormController/moveToNext"_ustr },
            { FormFeature::MoveToLast,              u".uno:FormController/moveToLast"_ustr },
            { FormFeature::SaveRecordChanges,       u".uno:FormController/saveRecord"_ustr },
            { FormFeature::UndoRecordChanges,       u".uno:FormController/undoRecord"_ustr },
            { FormFeature::MoveToInsertRow,         u".uno:FormController/moveToNew"_ustr },
            { FormFeature::DeleteRecord,            u".uno:FormController/deleteRecord"_ustr },
            { FormFeature::ReloadForm,              u".uno:FormController/refreshForm"_ustr },
            { FormFeature::RefreshCurrentControl,   u".uno:FormController/refreshCurrentControl"_ustr },
            { FormFeature::SortAscending,           u".uno:FormController/sortUp"_ustr },
            { FormFeature::SortDescending,          u".uno:FormController/sortDown"_ustr },
            { FormFeature::InteractiveSort,         u".uno:FormController/sort"_ustr },
            { FormFeature::AutoFilter,              u".uno:FormController/autoFilter"_ustr },
            { FormFeature::InteractiveFilter,       u".uno:FormController/filter"_ustr },
            { FormFeature::ToggleApplyFilter,       u".uno:FormController/applyFilter"_ustr },
            { FormFeature::RemoveFilterAndSort,     u".uno:FormController/removeFilterOrder"_ustr },
        };
    }


    std::optional<OUString> OFormNavigationMapper::getFeatureURL( sal_Int16 _nFeatureId )
    {
        for (const FeatureURL& rFeature : s_aFeatureURLs)
        {
            if ( rFeature.nFormFeature == _nFeatureId )
                return rFeature.aAsciiURL;
        }
        return std::nullopt;
    }


    sal_Int16 OFormNavigationMapper::getFeatureId( std::u16string_view _rCompleteURL )
    {
        for (const FeatureURL& rFeature : s_aFeatureURLs)
        {
            if ( rFeature.aAsciiURL == _rCompleteURL )
                return rFeature.nFormFeature;
        }
        return -1;
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
