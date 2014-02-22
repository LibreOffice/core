/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "connectivity/parameters.hxx"

#include <com/sun/star/form/DatabaseParameterEvent.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <com/sun/star/sdb/ParametersRequest.hpp>

#include <connectivity/dbtools.hxx>
#include "connectivity/filtermanager.hxx"
#include "TConnection.hxx"

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/ParameterCont.hxx"
#include <rtl/ustrbuf.hxx>


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::container;

    using namespace ::comphelper;
    using namespace ::connectivity;

    
    
    
    
    ParameterManager::ParameterManager( ::osl::Mutex& _rMutex, const Reference< XComponentContext >& _rxContext )
        :m_rMutex             ( _rMutex )
        ,m_aParameterListeners( _rMutex )
        ,m_xContext           ( _rxContext  )
        ,m_pOuterParameters   ( NULL    )
        ,m_nInnerCount        ( 0       )
        ,m_bUpToDate          ( false   )
    {
        OSL_ENSURE( m_xContext.is(), "ParameterManager::ParameterManager: no service factory!" );
    }

    
    void ParameterManager::initialize( const Reference< XPropertySet >& _rxComponent, const Reference< XAggregation >& _rxComponentAggregate )
    {
        OSL_ENSURE( !m_xComponent.get().is(), "ParameterManager::initialize: already initialized!" );

        m_xComponent        = _rxComponent;
        m_xAggregatedRowSet = _rxComponentAggregate;
        if ( m_xAggregatedRowSet.is() )
            m_xAggregatedRowSet->queryAggregation( ::getCppuType( &m_xInnerParamUpdate ) ) >>= m_xInnerParamUpdate;
        OSL_ENSURE( m_xComponent.get().is() && m_xInnerParamUpdate.is(), "ParameterManager::initialize: invalid arguments!" );
        if ( !m_xComponent.get().is() || !m_xInnerParamUpdate.is() )
            return;
    }

    
    void ParameterManager::dispose( )
    {
        clearAllParameterInformation();

        m_xComposer.clear();
        m_xParentComposer.clear();
        
        m_xInnerParamUpdate.clear();
        m_xAggregatedRowSet.clear();
    }

    
    void ParameterManager::clearAllParameterInformation()
    {
       m_xInnerParamColumns.clear();
        if ( m_pOuterParameters.is() )
            m_pOuterParameters->dispose();
        m_pOuterParameters   = NULL;
        m_nInnerCount        = 0;
        ParameterInformation aEmptyInfo;
        m_aParameterInformation.swap( aEmptyInfo );
        m_aMasterFields.realloc( 0 );
        m_aDetailFields.realloc( 0 );
        m_sIdentifierQuoteString = OUString();
        ::std::vector< bool > aEmptyArray;
        m_aParametersVisited.swap( aEmptyArray );
        m_bUpToDate = false;
    }

    
    void ParameterManager::disposing( const EventObject& /*_rDisposingEvent*/ )
    {
    }

    
    void ParameterManager::setAllParametersNull() SAL_THROW( ( SQLException, RuntimeException ) )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::setAllParametersNull: not initialized, or already disposed!" );
        if ( !isAlive() )
            return;

        for ( sal_Int32 i = 1; i <= m_nInnerCount; ++i )
            m_xInnerParamUpdate->setNull( i, DataType::VARCHAR );
    }

    
    bool ParameterManager::initializeComposerByComponent( const Reference< XPropertySet >& _rxComponent )
    {
        OSL_PRECOND( _rxComponent.is(), "ParameterManager::initializeComposerByComponent: invalid !" );

        m_xComposer.clear();
        m_xInnerParamColumns.clear();
        m_nInnerCount = 0;

        
        try
        {
            
            m_xComposer.reset( getCurrentSettingsComposer( _rxComponent, m_xContext ), SharedQueryComposer::TakeOwnership );

            
            Reference< XParametersSupplier > xParamSupp( m_xComposer, UNO_QUERY );
            if ( xParamSupp.is() )
                m_xInnerParamColumns = xParamSupp->getParameters();

            if ( m_xInnerParamColumns.is() )
                m_nInnerCount = m_xInnerParamColumns->getCount();
        }
        catch( const SQLException& )
        {
        }

        return m_xInnerParamColumns.is();
    }

    
    void ParameterManager::collectInnerParameters( bool _bSecondRun )
    {
        OSL_PRECOND( m_xInnerParamColumns.is(), "ParameterManager::collectInnerParameters: missing some internal data!" );
        if ( !m_xInnerParamColumns.is() )
            return;

        
        if ( _bSecondRun )
        {
            for ( ParameterInformation::iterator aParamInfo = m_aParameterInformation.begin();
                  aParamInfo != m_aParameterInformation.end();
                  ++aParamInfo
                )
            {
                aParamInfo->second.aInnerIndexes.clear();
            }
        }

        
        
        
        Reference<XPropertySet> xParam;
        for ( sal_Int32 i = 0; i < m_nInnerCount; ++i )
        {
            try
            {
                xParam.clear();
                m_xInnerParamColumns->getByIndex( i ) >>= xParam;

                OUString sName;
                xParam->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME) ) >>= sName;

                
                ParameterInformation::iterator aExistentPos = m_aParameterInformation.find( sName );
                OSL_ENSURE( !_bSecondRun || ( aExistentPos != m_aParameterInformation.end() ),
                    "ParameterManager::collectInnerParameters: the parameter information should already exist in the second run!" );

                if ( aExistentPos == m_aParameterInformation.end() )
                {
                    aExistentPos = m_aParameterInformation.insert( ParameterInformation::value_type(
                        sName, xParam ) ).first;
                }
                else
                    aExistentPos->second.xComposerColumn = xParam;

                aExistentPos->second.aInnerIndexes.push_back( i );
            }
            catch( const Exception& )
            {
                SAL_WARN( "connectivity.commontools", "ParameterManager::collectInnerParameters: caught an exception!" );
            }
        }
    }

    
    OUString ParameterManager::createFilterConditionFromColumnLink(
        const OUString& _rMasterColumn, const OUString& _rDetailLink, OUString& _rNewParamName )
    {
        OUString sFilter;

        
        
        sFilter = quoteName( m_sIdentifierQuoteString, _rDetailLink );
        sFilter += " = :";

        
        _rNewParamName = "link_from_";
        _rNewParamName += convertName2SQLName( _rMasterColumn, m_sSpecialCharacters );
        while ( m_aParameterInformation.find( _rNewParamName ) != m_aParameterInformation.end() )
        {
            _rNewParamName += "_";
        }

        return sFilter += _rNewParamName;
    }

    
    void ParameterManager::classifyLinks( const Reference< XNameAccess >& _rxParentColumns,
        const Reference< XNameAccess >& _rxColumns, ::std::vector< OUString >& _out_rAdditionalFilterComponents ) SAL_THROW(( Exception ))
    {
        OSL_PRECOND( m_aMasterFields.getLength() == m_aDetailFields.getLength(),
            "ParameterManager::classifyLinks: master and detail fields should have the same length!" );
        OSL_ENSURE( _rxColumns.is(), "ParameterManager::classifyLinks: invalid columns!" );

        if ( !_rxColumns.is() )
            return;

        
        
        ::std::vector< OUString > aStrippedMasterFields;
        ::std::vector< OUString > aStrippedDetailFields;

        bool bNeedExchangeLinks = false;

        
        const OUString* pMasterFields = m_aMasterFields.getConstArray();
        const OUString* pDetailFields = m_aDetailFields.getConstArray();
        const OUString* pDetailFieldsEnd = pDetailFields + m_aDetailFields.getLength();
        for ( ; pDetailFields < pDetailFieldsEnd; ++pDetailFields, ++pMasterFields )
        {
            if ( pMasterFields->isEmpty() || pDetailFields->isEmpty() )
                continue;

            
            
            
            if ( !_rxParentColumns->hasByName( *pMasterFields ) )
            {
                bNeedExchangeLinks = true;
                continue;
            }

            bool bValidLink = true;

            
            
            ParameterInformation::iterator aPos = m_aParameterInformation.find( *pDetailFields );
            if ( aPos != m_aParameterInformation.end() )
            {   
                aPos->second.eType = eLinkedByParamName;
                aStrippedDetailFields.push_back( *pDetailFields );
            }
            else
            {
                
                if ( _rxColumns->hasByName( *pDetailFields ) )
                {
                    OUString sNewParamName;
                    const OUString sFilterCondition = createFilterConditionFromColumnLink( *pMasterFields, *pDetailFields, sNewParamName );
                    OSL_PRECOND( !sNewParamName.isEmpty(), "ParameterManager::classifyLinks: createFilterConditionFromColumnLink returned nonsense!" );

                    
                    ::std::pair< ParameterInformation::iterator, bool > aInsertionPos =
                        m_aParameterInformation.insert(
                            ParameterInformation::value_type( sNewParamName, ParameterMetaData( NULL ) )
                        );
                    OSL_ENSURE( aInsertionPos.second, "ParameterManager::classifyLinks: there already was a parameter with this name!" );
                    aInsertionPos.first->second.eType = eLinkedByColumnName;

                    
                    _out_rAdditionalFilterComponents.push_back( sFilterCondition );

                    
                    aStrippedDetailFields.push_back( sNewParamName );
                    bNeedExchangeLinks = true;
                }
                else
                {
                    
                    bValidLink = false;
                    bNeedExchangeLinks = true;
                }
            }

            if ( bValidLink )
                aStrippedMasterFields.push_back( *pMasterFields );
        }
        OSL_POSTCOND( aStrippedMasterFields.size() == aStrippedDetailFields.size(),
            "ParameterManager::classifyLinks: inconsistency in new link pairs!" );

        if ( bNeedExchangeLinks )
        {
            OUString *pFields = aStrippedMasterFields.empty() ? 0 : &aStrippedMasterFields[0];
            m_aMasterFields = Sequence< OUString >( pFields, aStrippedMasterFields.size() );
            pFields = aStrippedDetailFields.empty() ? 0 : &aStrippedDetailFields[0];
            m_aDetailFields = Sequence< OUString >( pFields, aStrippedDetailFields.size() );
        }
    }

    
    void ParameterManager::analyzeFieldLinks( FilterManager& _rFilterManager, bool& /* [out] */ _rColumnsInLinkDetails )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::analyzeFieldLinks: not initialized, or already disposed!" );
        if ( !isAlive() )
            return;

        _rColumnsInLinkDetails = false;
        try
        {
            
            Reference< XPropertySet > xProp = m_xComponent;
            OSL_ENSURE(xProp.is(),"Some already released my component!");
            if ( xProp.is() )
            {
                xProp->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MASTERFIELDS) ) >>= m_aMasterFields;
                xProp->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DETAILFIELDS) ) >>= m_aDetailFields;
            }

            {
                
                sal_Int32 nMasterLength = m_aMasterFields.getLength();
                sal_Int32 nDetailLength = m_aDetailFields.getLength();

                if ( nMasterLength > nDetailLength )
                    m_aMasterFields.realloc( nDetailLength );
                else if ( nDetailLength > nMasterLength )
                    m_aDetailFields.realloc( nMasterLength );
            }

            Reference< XNameAccess > xColumns;
            if ( !getColumns( xColumns, true ) )
                
                return;

            Reference< XNameAccess > xParentColumns;
            if ( !getParentColumns( xParentColumns, true ) )
                return;

            
            ::std::vector< OUString > aAdditionalFilterComponents;
            classifyLinks( xParentColumns, xColumns, aAdditionalFilterComponents );

            
            if ( !aAdditionalFilterComponents.empty() )
            {
                const static OUString s_sAnd( " AND " );
                
                OUStringBuffer sAdditionalFilter;
                for (   ::std::vector< OUString >::const_iterator aComponent = aAdditionalFilterComponents.begin();
                        aComponent != aAdditionalFilterComponents.end();
                        ++aComponent
                    )
                {
                    if ( !sAdditionalFilter.isEmpty() )
                        sAdditionalFilter.append(s_sAnd);

                    sAdditionalFilter.appendAscii("( ",((sal_Int32)(sizeof("( ")-1)));
                    sAdditionalFilter.append(*aComponent);
                    sAdditionalFilter.appendAscii(" )",((sal_Int32)(sizeof(" )")-1)));
                }

                
                _rFilterManager.setFilterComponent( FilterManager::fcLinkFilter, sAdditionalFilter.makeStringAndClear() );

                _rColumnsInLinkDetails = true;
            }
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::analyzeFieldLinks: caught an exception!" );
        }
    }

    
    void ParameterManager::createOuterParameters()
    {
        OSL_PRECOND( !m_pOuterParameters.is(), "ParameterManager::createOuterParameters: outer parameters not initialized!" );
        OSL_PRECOND( m_xInnerParamUpdate.is(), "ParameterManager::createOuterParameters: no write access to the inner parameters!" );
        if ( !m_xInnerParamUpdate.is() )
            return;

        m_pOuterParameters = new param::ParameterWrapperContainer;

#if OSL_DEBUG_LEVEL > 0
        sal_Int32 nSmallestIndexLinkedByColumnName = -1;
        sal_Int32 nLargestIndexNotLinkedByColumnName = -1;
#endif
        for ( ParameterInformation::iterator aParam = m_aParameterInformation.begin();
              aParam != m_aParameterInformation.end();
              ++aParam
            )
        {
#if OSL_DEBUG_LEVEL > 0
            if ( aParam->second.aInnerIndexes.size() )
            {
                if ( aParam->second.eType == eLinkedByColumnName )
                {
                    if ( nSmallestIndexLinkedByColumnName == -1 )
                        nSmallestIndexLinkedByColumnName = aParam->second.aInnerIndexes[ 0 ];
                }
                else
                {
                    nLargestIndexNotLinkedByColumnName = aParam->second.aInnerIndexes[ aParam->second.aInnerIndexes.size() - 1 ];
                }
            }
#endif
            if ( aParam->second.eType != eFilledExternally )
                continue;

            
            size_t nAlreadyVisited = 0;
            for (   ::std::vector< sal_Int32 >::iterator aIndex = aParam->second.aInnerIndexes.begin();
                    aIndex != aParam->second.aInnerIndexes.end();
                    ++aIndex
                )
            {
                if ( ( m_aParametersVisited.size() > (size_t)*aIndex ) && m_aParametersVisited[ *aIndex ] )
                {   
                    *aIndex = -1;
                    ++nAlreadyVisited;
                }
            }
            if ( nAlreadyVisited == aParam->second.aInnerIndexes.size() )
                continue;

            
            
            
            
            m_pOuterParameters->push_back( new param::ParameterWrapper( aParam->second.xComposerColumn, m_xInnerParamUpdate, aParam->second.aInnerIndexes ) );
        }

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( ( nSmallestIndexLinkedByColumnName == -1 ) || ( nLargestIndexNotLinkedByColumnName == -1 ) ||
            ( nSmallestIndexLinkedByColumnName > nLargestIndexNotLinkedByColumnName ),
            "ParameterManager::createOuterParameters: inconsistency!" );

        
        
        
        
        
        
#endif
    }

    
    void ParameterManager::updateParameterInfo( FilterManager& _rFilterManager )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::updateParameterInfo: not initialized, or already disposed!" );
        if ( !isAlive() )
            return;

        clearAllParameterInformation();
        cacheConnectionInfo();

        
        Reference< XPropertySet > xProp = m_xComponent;
        OSL_ENSURE(xProp.is(),"Some already released my component!");
        if ( xProp.is() )
        {
            if ( !initializeComposerByComponent( xProp ) )
            {   
                m_bUpToDate = true;
                return;
            } 
        }
        OSL_POSTCOND( m_xInnerParamColumns.is(), "ParameterManager::updateParameterInfo: initializeComposerByComponent did nonsense (1)!" );

        
        collectInnerParameters( false );

        
        bool bColumnsInLinkDetails = false;
        analyzeFieldLinks( _rFilterManager, bColumnsInLinkDetails );

        if ( bColumnsInLinkDetails )
        {
            
            
            
            Reference< XPropertySet > xDirectRowSetProps;
            m_xAggregatedRowSet->queryAggregation( ::getCppuType( &xDirectRowSetProps ) ) >>= xDirectRowSetProps;
            OSL_VERIFY( initializeComposerByComponent( xDirectRowSetProps ) );
            collectInnerParameters( true );
        }

        if ( !m_nInnerCount )
        {   
            m_bUpToDate = true;
            return;
        }

        
        
        createOuterParameters();

        m_bUpToDate = true;
    }

    
    void ParameterManager::fillLinkedParameters( const Reference< XNameAccess >& _rxParentColumns )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::fillLinkedParameters: not initialized, or already disposed!" );
        if ( !isAlive() )
            return;
        OSL_PRECOND( m_xInnerParamColumns.is(), "ParameterManager::fillLinkedParameters: no inner parameters found!"                 );
        OSL_ENSURE ( _rxParentColumns.is(),     "ParameterManager::fillLinkedParameters: invalid parent columns!"                    );

        try
        {
            
            const OUString* pMasterFields = m_aMasterFields.getConstArray();
            const OUString* pDetailFields = m_aDetailFields.getConstArray();

            sal_Int32 nMasterLen = m_aMasterFields.getLength();

            
            
            for ( sal_Int32 i = 0; i < nMasterLen; ++i, ++pMasterFields, ++pDetailFields )
            {
                
                if ( !_rxParentColumns->hasByName( *pMasterFields ) )
                {
                    SAL_WARN( "connectivity.commontools", "ParameterManager::fillLinkedParameters: invalid master names should have been stripped long before!" );
                    continue;
                }

                
                ParameterInformation::const_iterator aParamInfo = m_aParameterInformation.find( *pDetailFields );
                if  (  ( aParamInfo == m_aParameterInformation.end() )
                    || ( aParamInfo->second.aInnerIndexes.empty() )
                    )
                {
                    SAL_WARN( "connectivity.commontools", "ParameterManager::fillLinkedParameters: nothing known about this detail field!" );
                    continue;
                }

                
                Reference< XPropertySet >  xMasterField(_rxParentColumns->getByName( *pMasterFields ),UNO_QUERY);

                
                for ( ::std::vector< sal_Int32 >::const_iterator aPosition = aParamInfo->second.aInnerIndexes.begin();
                      aPosition != aParamInfo->second.aInnerIndexes.end();
                      ++aPosition
                    )
                {
                    
                    Reference< XPropertySet >  xDetailField(m_xInnerParamColumns->getByIndex( *aPosition ),UNO_QUERY);
                    OSL_ENSURE( xDetailField.is(), "ParameterManager::fillLinkedParameters: invalid detail field!" );
                    if ( !xDetailField.is() )
                        continue;

                    
                    sal_Int32 nParamType = DataType::VARCHAR;
                    OSL_VERIFY( xDetailField->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE) ) >>= nParamType );

                    sal_Int32 nScale = 0;
                    if ( xDetailField->getPropertySetInfo()->hasPropertyByName( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE) ) )
                        OSL_VERIFY( xDetailField->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE) ) >>= nScale );

                    
                    try
                    {
                        m_xInnerParamUpdate->setObjectWithInfo(
                            *aPosition + 1,                     
                            xMasterField->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_VALUE) ),
                            nParamType,
                            nScale
                        );
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                        SAL_WARN( "connectivity.commontools", "ParameterManager::fillLinkedParameters: master-detail parameter number " <<
                                  sal_Int32( *aPosition + 1 ) << " could not be filled!" );
                    }
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    
    bool ParameterManager::completeParameters( const Reference< XInteractionHandler >& _rxCompletionHandler, const Reference< XConnection > _rxConnection )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::completeParameters: not initialized, or already disposed!" );
        OSL_ENSURE ( _rxCompletionHandler.is(), "ParameterManager::completeParameters: invalid interaction handler!" );

        
        OInteractionAbort* pAbort = new OInteractionAbort;
        OParameterContinuation* pParams = new OParameterContinuation;

        
        ParametersRequest aRequest;
        aRequest.Parameters = m_pOuterParameters.get();
        aRequest.Connection = _rxConnection;
        OInteractionRequest* pRequest = new OInteractionRequest( makeAny( aRequest ) );
        Reference< XInteractionRequest > xRequest( pRequest );

        
        pRequest->addContinuation( pAbort );
        pRequest->addContinuation( pParams );

        
        try
        {
            _rxCompletionHandler->handle( xRequest );
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::completeParameters: caught an exception while calling the handler!" );
        }

        if ( !pParams->wasSelected() )
            
            return false;

        try
        {
            
            Sequence< PropertyValue > aFinalValues = pParams->getValues();
            const PropertyValue* pFinalValues = aFinalValues.getConstArray();
            for ( sal_Int32 i = 0; i < aFinalValues.getLength(); ++i, ++pFinalValues )
            {
                Reference< XPropertySet > xParamColumn(aRequest.Parameters->getByIndex( i ),UNO_QUERY);
                if ( xParamColumn.is() )
                {
            #ifdef DBG_UTIL
                    OUString sName;
                    xParamColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME) ) >>= sName;
                    OSL_ENSURE( sName == pFinalValues->Name, "ParameterManager::completeParameters: inconsistent parameter names!" );
            #endif
                    xParamColumn->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_VALUE), pFinalValues->Value );
                        
                        
                }
            }
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::completeParameters: caught an exception while propagating the values!" );
        }
        return true;
    }

    
    bool ParameterManager::consultParameterListeners( ::osl::ResettableMutexGuard& _rClearForNotifies )
    {
        bool bCanceled = false;

        sal_Int32 nParamsLeft = m_pOuterParameters->getParameters().size();
            
        if ( nParamsLeft )
        {
            ::cppu::OInterfaceIteratorHelper aIter( m_aParameterListeners );
            Reference< XPropertySet > xProp = m_xComponent;
            OSL_ENSURE(xProp.is(),"Some already released my component!");
            DatabaseParameterEvent aEvent( xProp.get(), m_pOuterParameters.get() );

            _rClearForNotifies.clear();
            while ( aIter.hasMoreElements() && !bCanceled )
                bCanceled = !static_cast< XDatabaseParameterListener* >( aIter.next() )->approveParameter( aEvent );
            _rClearForNotifies.reset();
        }

        return !bCanceled;
    }

    
    bool ParameterManager::fillParameterValues( const Reference< XInteractionHandler >& _rxCompletionHandler, ::osl::ResettableMutexGuard& _rClearForNotifies )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::fillParameterValues: not initialized, or already disposed!" );
        if ( !isAlive() )
            return true;

        if ( m_nInnerCount == 0 )
            
            return true;

        
        Reference< XNameAccess > xParentColumns;
        if ( getParentColumns( xParentColumns, false ) && xParentColumns->hasElements() && m_aMasterFields.getLength() )
            fillLinkedParameters( xParentColumns );

        
        Reference< XConnection > xConnection;
        getConnection( xConnection );

        if ( _rxCompletionHandler.is() )
            return completeParameters( _rxCompletionHandler, xConnection );

        return consultParameterListeners( _rClearForNotifies );
    }

    
    bool ParameterManager::getConnection( Reference< XConnection >& /* [out] */ _rxConnection )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::getConnection: not initialized, or already disposed!" );
        if ( !isAlive() )
            return false;

        _rxConnection.clear();
        try
        {
            Reference< XPropertySet > xProp = m_xComponent;
            OSL_ENSURE(xProp.is(),"Some already released my component!");
            if ( xProp.is() )
                xProp->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ACTIVE_CONNECTION) ) >>= _rxConnection;
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::getConnection: could not retrieve the connection of the !" );
        }
        return _rxConnection.is();
    }

    
    void ParameterManager::cacheConnectionInfo() SAL_THROW(( ))
    {
        try
        {
            Reference< XConnection > xConnection;
            getConnection( xConnection );
            Reference< XDatabaseMetaData > xMeta;
            if ( xConnection.is() )
                xMeta = xConnection->getMetaData();
            if ( xMeta.is() )
            {
                m_sIdentifierQuoteString = xMeta->getIdentifierQuoteString();
                m_sSpecialCharacters = xMeta->getExtraNameCharacters();
            }
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::cacheConnectionInfo: caught an exception!" );
        }
    }

    
    bool ParameterManager::getColumns( Reference< XNameAccess >& /* [out] */ _rxColumns, bool _bFromComposer ) SAL_THROW(( Exception ))
    {
        _rxColumns.clear();

        Reference< XColumnsSupplier > xColumnSupp;
        if ( _bFromComposer )
            xColumnSupp = xColumnSupp.query( m_xComposer );
        else
            xColumnSupp.set( m_xComponent.get(),UNO_QUERY);
        if ( xColumnSupp.is() )
            _rxColumns = xColumnSupp->getColumns();
        OSL_ENSURE( _rxColumns.is(), "ParameterManager::getColumns: could not retrieve the columns for the detail !" );

        return _rxColumns.is();
    }

    
    bool ParameterManager::getParentColumns( Reference< XNameAccess >& /* [out] */ _out_rxParentColumns, bool _bFromComposer )
    {
        OSL_PRECOND( isAlive(), "ParameterManager::getParentColumns: not initialized, or already disposed!" );

        _out_rxParentColumns.clear();
        try
        {
            
            Reference< XChild > xAsChild( m_xComponent.get(), UNO_QUERY_THROW );
            Reference< XPropertySet > xParent( xAsChild->getParent(), UNO_QUERY );
            if ( !xParent.is() )
                return false;

            
            Reference< XColumnsSupplier > xParentColSupp;
            if ( _bFromComposer )
            {
                
                
                m_xParentComposer.reset(
                    getCurrentSettingsComposer( xParent, m_xContext ),
                    SharedQueryComposer::TakeOwnership
                );
                xParentColSupp = xParentColSupp.query( m_xParentComposer );
            }
            else
                xParentColSupp = xParentColSupp.query( xParent );

            
            if ( xParentColSupp.is() )
                _out_rxParentColumns = xParentColSupp->getColumns();
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::getParentColumns: caught an exception!" );
        }
        return _out_rxParentColumns.is();
    }

    
    void ParameterManager::addParameterListener( const Reference< XDatabaseParameterListener >& _rxListener )
    {
        if ( _rxListener.is() )
            m_aParameterListeners.addInterface( _rxListener );
    }

    
    void ParameterManager::removeParameterListener( const Reference< XDatabaseParameterListener >& _rxListener )
    {
        m_aParameterListeners.removeInterface( _rxListener );
    }

    
    void ParameterManager::resetParameterValues( ) SAL_THROW(())
    {
        OSL_PRECOND( isAlive(), "ParameterManager::resetParameterValues: not initialized, or already disposed!" );
        if ( !isAlive() )
            return;

        if ( !m_nInnerCount )
            
            return;

        try
        {
            Reference< XNameAccess > xColumns;
            if ( !getColumns( xColumns, false ) )
                
                return;

            Reference< XNameAccess > xParentColumns;
            if ( !getParentColumns( xParentColumns, false ) )
                return;

            
            const OUString* pMasterFields = m_aMasterFields.getConstArray();
            const OUString* pDetailFields = m_aDetailFields.getConstArray();

            Reference< XPropertySet > xMasterField;
            Reference< XPropertySet > xDetailField;

            
            const OUString* pDetailFieldsEnd = pDetailFields + m_aDetailFields.getLength();
            for ( ; pDetailFields < pDetailFieldsEnd; ++pDetailFields, ++pMasterFields )
            {
                if ( !xParentColumns->hasByName( *pMasterFields ) )
                {
                    
                    
                    SAL_WARN( "connectivity.commontools", "ParameterManager::resetParameterValues: this should have been stripped long before!" );
                    continue;
                }

                
                
                
                ParameterInformation::const_iterator aParamInfo = m_aParameterInformation.find( *pDetailFields );
                if  (  ( aParamInfo == m_aParameterInformation.end() )
                    || ( aParamInfo->second.aInnerIndexes.empty() )
                    )
                {
                    SAL_WARN( "connectivity.commontools", "ParameterManager::resetParameterValues: nothing known about this detail field!" );
                    continue;
                }

                xParentColumns->getByName( *pMasterFields ) >>= xMasterField;
                if ( !xMasterField.is() )
                    continue;

                for ( ::std::vector< sal_Int32 >::const_iterator aPosition = aParamInfo->second.aInnerIndexes.begin();
                      aPosition != aParamInfo->second.aInnerIndexes.end();
                      ++aPosition
                    )
                {
                    Reference< XPropertySet > xInnerParameter;
                    m_xInnerParamColumns->getByIndex( *aPosition ) >>= xInnerParameter;
                    if ( !xInnerParameter.is() )
                        continue;

                    OUString sParamColumnRealName;
                    xInnerParameter->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME) ) >>= sParamColumnRealName;
                    if ( xColumns->hasByName( sParamColumnRealName ) )
                    {   
                        
                        xColumns->getByName( sParamColumnRealName ) >>= xDetailField;
                        if ( xDetailField.is() )
                            xDetailField->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_VALUE), xMasterField->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_VALUE) ) );
                    }
                }
            }
        }
        catch( const Exception& )
        {
            SAL_WARN( "connectivity.commontools", "ParameterManager::resetParameterValues: caught an exception!" );
        }

    }

    
    void ParameterManager::externalParameterVisited( sal_Int32 _nIndex )
    {
        if ( m_aParametersVisited.size() < (size_t)_nIndex )
        {
            m_aParametersVisited.reserve( _nIndex );
            for ( sal_Int32 i = m_aParametersVisited.size(); i < _nIndex; ++i )
                m_aParametersVisited.push_back( false );
        }
        m_aParametersVisited[ _nIndex - 1 ] = true;
    }

#define VISIT_PARAMETER( method ) \
        ::osl::MutexGuard aGuard( m_rMutex ); \
        OSL_ENSURE( m_xInnerParamUpdate.is(), "ParameterManager::XParameters::setXXX: no XParameters access to the RowSet!" ); \
        if ( !m_xInnerParamUpdate.is() ) \
            return; \
        m_xInnerParamUpdate->method; \
        externalParameterVisited( _nIndex ) \

    
    void ParameterManager::setNull( sal_Int32 _nIndex, sal_Int32 sqlType )
    {
        VISIT_PARAMETER( setNull( _nIndex, sqlType ) );
    }

    
    void ParameterManager::setObjectNull( sal_Int32 _nIndex, sal_Int32 sqlType, const OUString& typeName )
    {
        VISIT_PARAMETER( setObjectNull( _nIndex, sqlType, typeName ) );
    }

    
    void ParameterManager::setBoolean( sal_Int32 _nIndex, sal_Bool x )
    {
        VISIT_PARAMETER( setBoolean( _nIndex, x ) );
    }

    
    void ParameterManager::setByte( sal_Int32 _nIndex, sal_Int8 x )
    {
        VISIT_PARAMETER( setByte( _nIndex, x ) );
    }

    
    void ParameterManager::setShort( sal_Int32 _nIndex, sal_Int16 x )
    {
        VISIT_PARAMETER( setShort( _nIndex, x ) );
    }

    
    void ParameterManager::setInt( sal_Int32 _nIndex, sal_Int32 x )
    {
        VISIT_PARAMETER( setInt( _nIndex, x ) );
    }

    
    void ParameterManager::setLong( sal_Int32 _nIndex, sal_Int64 x )
    {
        VISIT_PARAMETER( setLong( _nIndex, x ) );
    }

    
    void ParameterManager::setFloat( sal_Int32 _nIndex, float x )
    {
        VISIT_PARAMETER( setFloat( _nIndex, x ) );
    }

    
    void ParameterManager::setDouble( sal_Int32 _nIndex, double x )
    {
        VISIT_PARAMETER( setDouble( _nIndex, x ) );
    }

    
    void ParameterManager::setString( sal_Int32 _nIndex, const OUString& x )
    {
        VISIT_PARAMETER( setString( _nIndex, x ) );
    }

    
    void ParameterManager::setBytes( sal_Int32 _nIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x )
    {
        VISIT_PARAMETER( setBytes( _nIndex, x ) );
    }

    
    void ParameterManager::setDate( sal_Int32 _nIndex, const ::com::sun::star::util::Date& x )
    {
        VISIT_PARAMETER( setDate( _nIndex, x ) );
    }

    
    void ParameterManager::setTime( sal_Int32 _nIndex, const ::com::sun::star::util::Time& x )
    {
        VISIT_PARAMETER( setTime( _nIndex, x ) );
    }

    
    void ParameterManager::setTimestamp( sal_Int32 _nIndex, const ::com::sun::star::util::DateTime& x )
    {
        VISIT_PARAMETER( setTimestamp( _nIndex, x ) );
    }

    
    void ParameterManager::setBinaryStream( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length )
    {
        VISIT_PARAMETER( setBinaryStream( _nIndex, x, length ) );
    }

    
    void ParameterManager::setCharacterStream( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length )
    {
        VISIT_PARAMETER( setCharacterStream( _nIndex, x, length ) );
    }

    
    void ParameterManager::setObject( sal_Int32 _nIndex, const ::com::sun::star::uno::Any& x )
    {
        VISIT_PARAMETER( setObject( _nIndex, x ) );
    }

    
    void ParameterManager::setObjectWithInfo( sal_Int32 _nIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale )
    {
        VISIT_PARAMETER( setObjectWithInfo( _nIndex, x, targetSqlType, scale ) );
    }

    
    void ParameterManager::setRef( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef>& x )
    {
        VISIT_PARAMETER( setRef( _nIndex, x ) );
    }

    
    void ParameterManager::setBlob( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob>& x )
    {
        VISIT_PARAMETER( setBlob( _nIndex, x ) );
    }

    
    void ParameterManager::setClob( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob>& x )
    {
        VISIT_PARAMETER( setClob( _nIndex, x ) );
    }

    
    void ParameterManager::setArray( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray>& x )
    {
        VISIT_PARAMETER( setArray( _nIndex, x ) );
    }

    
    void ParameterManager::clearParameters( )
    {
        if ( m_xInnerParamUpdate.is() )
            m_xInnerParamUpdate->clearParameters( );
    }

    
    
    
    
    void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw( RuntimeException )
    {
        m_aValues = _rValues;
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
