/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include "basscript.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/basmgr.hxx>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>

#include <map>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::document;

extern ::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );
extern void unoToSbxValue( SbxVariable* pVar, const ::com::sun::star::uno::Any& aValue );


//.........................................................................
namespace basprov
{
//.........................................................................

    typedef ::std::map< sal_Int16, Any, ::std::less< sal_Int16 > > OutParamMap;

    // =============================================================================
    // BasicScriptImpl
    // =============================================================================

    // -----------------------------------------------------------------------------

    BasicScriptImpl::BasicScriptImpl( const ::rtl::OUString& funcName, SbMethodRef xMethod )
        :m_xMethod( xMethod )
        ,m_funcName( funcName )
        ,m_documentBasicManager( NULL )
        ,m_xDocumentScriptContext()
    {
    }

    // -----------------------------------------------------------------------------

    BasicScriptImpl::BasicScriptImpl( const ::rtl::OUString& funcName, SbMethodRef xMethod,
        BasicManager& documentBasicManager, const Reference< XScriptInvocationContext >& documentScriptContext )
        :m_xMethod( xMethod )
        ,m_funcName( funcName )
        ,m_documentBasicManager( &documentBasicManager )
        ,m_xDocumentScriptContext( documentScriptContext )
    {
        StartListening( *m_documentBasicManager );
    }

    // -----------------------------------------------------------------------------
    BasicScriptImpl::~BasicScriptImpl()
    {
        if ( m_documentBasicManager )
            EndListening( *m_documentBasicManager );
    }

    // -----------------------------------------------------------------------------
    // SfxListener
    // -----------------------------------------------------------------------------
    void BasicScriptImpl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        if ( &rBC != m_documentBasicManager )
        {
            OSL_ENSURE( false, "BasicScriptImpl::Notify: where does this come from?" );
            // not interested in
            return;
        }
        const SfxSimpleHint* pSimpleHint = dynamic_cast< const SfxSimpleHint* >( &rHint );
        if ( pSimpleHint && ( pSimpleHint->GetId() == SFX_HINT_DYING ) )
        {
            m_documentBasicManager = NULL;
            EndListening( rBC );    // prevent multiple notifications
        }
    }

    // -----------------------------------------------------------------------------
    // XScript
    // -----------------------------------------------------------------------------

    Any BasicScriptImpl::invoke( const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
        throw ( provider::ScriptFrameworkErrorException, reflection::InvocationTargetException, uno::RuntimeException)
    {
        // TODO: throw CannotConvertException
        // TODO: check length of aOutParamIndex, aOutParam

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        Any aReturn;

        if ( m_xMethod )
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
                    if ( ( pParamInfo->nFlags & SBX_OPTIONAL ) != 0 )
                        ++nSbxOptional;
                    else
                        nSbxOptional = 0;
                }
                sal_Int32 nSbxCount = n - 1;
                if ( nParamsCount < nSbxCount - nSbxOptional )
                {
                    throw provider::ScriptFrameworkErrorException(
                        ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "wrong number of parameters!" ) ),
                         Reference< XInterface >(),
                         m_funcName,
                         ::rtl::OUString(
                             RTL_CONSTASCII_USTRINGPARAM( "Basic" ) ),
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
                    unoToSbxValue( static_cast< SbxVariable* >( xSbxVar ), pParams[i] );
                    xSbxParams->Put( xSbxVar, static_cast< sal_uInt16 >( i ) + 1 );

                    // Enable passing by ref
                    if ( xSbxVar->GetType() != SbxVARIANT )
                        xSbxVar->SetFlag( SBX_FIXED );
                 }
            }
            if ( xSbxParams.Is() )
                m_xMethod->SetParameters( xSbxParams );

            // call method
            SbxVariableRef xReturn = new SbxVariable;
            ErrCode nErr = SbxERR_OK;
            {
                // if it's a document-based script, temporarily reset ThisComponent to the script invocation context
                Any aOldThisComponent;
                if ( m_documentBasicManager && m_xDocumentScriptContext.is() )
                    aOldThisComponent = m_documentBasicManager->SetGlobalUNOConstant( "ThisComponent", makeAny( m_xDocumentScriptContext ) );

                nErr = m_xMethod->Call( xReturn );

                if ( m_documentBasicManager && m_xDocumentScriptContext.is() )
                    m_documentBasicManager->SetGlobalUNOConstant( "ThisComponent", aOldThisComponent );
            }
            if ( nErr != SbxERR_OK )
            {
                // TODO: throw InvocationTargetException ?
            }

            // get output parameters
            if ( xSbxParams.Is() )
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
                                aOutParamMap.insert( OutParamMap::value_type( n - 1, sbxToUnoValue( xVar ) ) );
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
            aReturn = sbxToUnoValue( xReturn );

            // reset parameters
            m_xMethod->SetParameters( NULL );
        }

        return aReturn;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
