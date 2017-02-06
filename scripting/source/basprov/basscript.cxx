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

#include "basscript.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <basic/basmgr.hxx>
#include <com/sun/star/script/provider/ScriptFrameworkErrorException.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#include "bcholder.hxx"
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <map>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;


namespace basprov
{

#define BASSCRIPT_PROPERTY_ID_CALLER         1
#define BASSCRIPT_PROPERTY_CALLER            "Caller"

#define BASSCRIPT_DEFAULT_ATTRIBS()       PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT

    typedef ::std::map< sal_Int16, Any > OutParamMap;


    // BasicScriptImpl


    BasicScriptImpl::BasicScriptImpl( const OUString& funcName, SbMethodRef const & xMethod )
        : ::scripting_helper::OBroadcastHelperHolder( m_aMutex )
        ,OPropertyContainer( GetBroadcastHelper() )
        ,m_xMethod( xMethod )
        ,m_funcName( funcName )
        ,m_documentBasicManager( nullptr )
        ,m_xDocumentScriptContext()
    {
        registerProperty( BASSCRIPT_PROPERTY_CALLER, BASSCRIPT_PROPERTY_ID_CALLER, BASSCRIPT_DEFAULT_ATTRIBS(), &m_caller, cppu::UnoType<decltype(m_caller)>::get() );
    }


    BasicScriptImpl::BasicScriptImpl( const OUString& funcName, SbMethodRef const & xMethod,
        BasicManager& documentBasicManager, const Reference< XScriptInvocationContext >& documentScriptContext ) : ::scripting_helper::OBroadcastHelperHolder( m_aMutex )
        ,OPropertyContainer( GetBroadcastHelper() )
        ,m_xMethod( xMethod )
        ,m_funcName( funcName )
        ,m_documentBasicManager( &documentBasicManager )
        ,m_xDocumentScriptContext( documentScriptContext )
    {
        StartListening( *m_documentBasicManager );
        registerProperty( BASSCRIPT_PROPERTY_CALLER, BASSCRIPT_PROPERTY_ID_CALLER, BASSCRIPT_DEFAULT_ATTRIBS(), &m_caller, cppu::UnoType<decltype(m_caller)>::get() );
    }


    BasicScriptImpl::~BasicScriptImpl()
    {
        SolarMutexGuard g;

        if ( m_documentBasicManager )
            EndListening( *m_documentBasicManager );
    }


    // SfxListener

    void BasicScriptImpl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        if ( &rBC != m_documentBasicManager )
        {
            OSL_ENSURE( false, "BasicScriptImpl::Notify: where does this come from?" );
            // not interested in
            return;
        }
        if ( rHint.GetId() == SfxHintId::Dying )
        {
            m_documentBasicManager = nullptr;
            EndListening( rBC );    // prevent multiple notifications
        }
    }


    // XInterface


    IMPLEMENT_FORWARD_XINTERFACE2( BasicScriptImpl, BasicScriptImpl_BASE, OPropertyContainer )


    // XTypeProvider


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( BasicScriptImpl, BasicScriptImpl_BASE, OPropertyContainer )


    // OPropertySetHelper


    ::cppu::IPropertyArrayHelper& BasicScriptImpl::getInfoHelper(  )
    {
        return *getArrayHelper();
    }


    // OPropertyArrayUsageHelper


    ::cppu::IPropertyArrayHelper* BasicScriptImpl::createArrayHelper(  ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }


    // XPropertySet


    Reference< XPropertySetInfo > BasicScriptImpl::getPropertySetInfo(  )
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    // XScript


    Any BasicScriptImpl::invoke( const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
    {
        // TODO: throw CannotConvertException
        // TODO: check length of aOutParamIndex, aOutParam

        SolarMutexGuard aGuard;

        Any aReturn;

        if ( m_xMethod.is() )
        {
            // check if compiled
            SbModule* pModule = static_cast< SbModule* >( m_xMethod->GetParent() );
            if ( pModule && !pModule->IsCompiled() )
                pModule->Compile();

            // check number of parameters
            sal_Int32 nParamsCount = aParams.getLength();
            SbxInfo* pInfo = m_xMethod->GetInfo();
            if ( pInfo )
            {
                sal_Int32 nSbxOptional = 0;
                sal_uInt16 n = 1;
                for ( const SbxParamInfo* pParamInfo = pInfo->GetParam( n ); pParamInfo; pParamInfo = pInfo->GetParam( ++n ) )
                {
                    if ( pParamInfo->nFlags & SbxFlagBits::Optional )
                        ++nSbxOptional;
                    else
                        nSbxOptional = 0;
                }
                sal_Int32 nSbxCount = n - 1;
                if ( nParamsCount < nSbxCount - nSbxOptional )
                {
                    throw provider::ScriptFrameworkErrorException(
                         "wrong number of parameters!",
                         Reference< XInterface >(),
                         m_funcName,
                         "Basic",
                         provider::ScriptFrameworkErrorType::NO_SUCH_SCRIPT  );
                }
            }

            // set parameters
            SbxArrayRef xSbxParams;
            if ( nParamsCount > 0 )
            {
                xSbxParams = new SbxArray;
                const Any* pParams = aParams.getConstArray();
                for ( sal_Int32 i = 0; i < nParamsCount; ++i )
                {
                    SbxVariableRef xSbxVar = new SbxVariable( SbxVARIANT );
                    unoToSbxValue( xSbxVar.get(), pParams[i] );
                    xSbxParams->Put( xSbxVar.get(), static_cast< sal_uInt16 >( i ) + 1 );

                    // Enable passing by ref
                    if ( xSbxVar->GetType() != SbxVARIANT )
                        xSbxVar->SetFlag( SbxFlagBits::Fixed );
                 }
            }
            if ( xSbxParams.is() )
                m_xMethod->SetParameters( xSbxParams.get() );

            // call method
            SbxVariableRef xReturn = new SbxVariable;
            ErrCode nErr = ERRCODE_SBX_OK;

            // if it's a document-based script, temporarily reset ThisComponent to the script invocation context
            Any aOldThisComponent;
            if ( m_documentBasicManager && m_xDocumentScriptContext.is() )
                aOldThisComponent = m_documentBasicManager->SetGlobalUNOConstant( "ThisComponent", makeAny( m_xDocumentScriptContext ) );

            if ( m_caller.getLength() && m_caller[ 0 ].hasValue()  )
            {
                SbxVariableRef xCallerVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( xCallerVar.get(), m_caller[ 0 ] );
                nErr = m_xMethod->Call( xReturn.get(), xCallerVar.get() );
            }
            else
                nErr = m_xMethod->Call( xReturn.get() );

            if ( m_documentBasicManager && m_xDocumentScriptContext.is() )
                m_documentBasicManager->SetGlobalUNOConstant( "ThisComponent", aOldThisComponent );

            if ( nErr != ERRCODE_SBX_OK )
            {
                // TODO: throw InvocationTargetException ?
            }

            // get output parameters
            if ( xSbxParams.is() )
            {
                SbxInfo* pInfo_ = m_xMethod->GetInfo();
                if ( pInfo_ )
                {
                    OutParamMap aOutParamMap;
                    for ( sal_uInt16 n = 1, nCount = xSbxParams->Count(); n < nCount; ++n )
                    {
                        const SbxParamInfo* pParamInfo = pInfo_->GetParam( n );
                        if ( pParamInfo && ( pParamInfo->eType & SbxBYREF ) != 0 )
                        {
                            SbxVariable* pVar = xSbxParams->Get( n );
                            if ( pVar )
                            {
                                SbxVariableRef xVar = pVar;
                                aOutParamMap.insert( OutParamMap::value_type( n - 1, sbxToUnoValue( xVar.get() ) ) );
                            }
                        }
                    }
                    sal_Int32 nOutParamCount = aOutParamMap.size();
                    aOutParamIndex.realloc( nOutParamCount );
                    aOutParam.realloc( nOutParamCount );
                    sal_Int16* pOutParamIndex = aOutParamIndex.getArray();
                    Any* pOutParam = aOutParam.getArray();
                    for ( OutParamMap::iterator aIt = aOutParamMap.begin(); aIt != aOutParamMap.end(); ++aIt, ++pOutParamIndex, ++pOutParam )
                    {
                        *pOutParamIndex = aIt->first;
                        *pOutParam = aIt->second;
                    }
                }
            }

            // get return value
            aReturn = sbxToUnoValue( xReturn.get() );

            // reset parameters
            m_xMethod->SetParameters( nullptr );
        }

        return aReturn;
    }


}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
