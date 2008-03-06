/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basscript.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:18:54 $
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
#include "precompiled_scripting.hxx"

#ifndef SCRIPTING_BASSCRIPT_HXX
#include "basscript.hxx"
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SBXCLASS_HXX
#include <basic/sbx.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMOD_HXX
#include <basic/sbmod.hxx>
#endif
#ifndef _SB_SBMETH_HXX
#include <basic/sbmeth.hxx>
#endif
#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_PROVIDER_SCRIPTFRAMEWORKERRORTYPE_HPP_
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#endif

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
    }

    // -----------------------------------------------------------------------------
    BasicScriptImpl::~BasicScriptImpl()
    {
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
                USHORT n = 1;
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
                                "wrong number of paramters!" ) ),
                         Reference< XInterface >(),
                         m_funcName,
                         ::rtl::OUString(
                             RTL_CONSTASCII_USTRINGPARAM( "Basic" ) ),
                        provider::ScriptFrameworkErrorType::UNKNOWN  );
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
                    xSbxParams->Put( xSbxVar, static_cast< USHORT >( i ) + 1 );

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
                    for ( USHORT n = 1, nCount = xSbxParams->Count(); n < nCount; ++n )
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
