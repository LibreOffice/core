/*************************************************************************
 *
 *  $RCSfile: basscript.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2003-09-16 15:23:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SCRIPTING_BASSCRIPT_HXX
#include "basscript.hxx"
#endif

#ifndef _SBXCLASS_HXX
#include <svtools/sbx.hxx>
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

#include <map>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;


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

    BasicScriptImpl::BasicScriptImpl( SbMethod* pMethod )
        :m_pMethod( pMethod )
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
        throw (IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
    {
        // TODO: throw CannotConvertException
        // TODO: check length of aOutParamIndex, aOutParam

        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        Any aReturn;

        if ( m_pMethod )
        {
            // check if compiled
            SbModule* pModule = (SbModule*)m_pMethod->GetParent();
            if ( pModule && !pModule->IsCompiled() )
                pModule->Compile();

            // check number of parameters
            sal_Int32 nParamsCount = aParams.getLength();
            SbxInfo* pInfo = m_pMethod->GetInfo();
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
                if ( ( nParamsCount < nSbxCount - nSbxOptional ) || ( nParamsCount > nSbxCount ) )
                {
                    throw IllegalArgumentException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicScriptImpl::invoke: wrong number of paramters!" ) ),
                        Reference< XInterface >(), 1 );
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
                    unoToSbxValue( (SbxVariable*)xSbxVar, pParams[i] );
                    xSbxParams->Put( xSbxVar, (USHORT)i + 1 );
                }
            }
            if ( xSbxParams.Is() )
                m_pMethod->SetParameters( xSbxParams );

            // call method
            SbxVariableRef xReturn = new SbxVariable;
            ErrCode nErr = m_pMethod->Call( xReturn );
            if ( nErr != SbxERR_OK )
            {
                // TODO: throw InvocationTargetException ?
            }

            // get output parameters
            if ( xSbxParams.Is() )
            {
                SbxInfo* pInfo = m_pMethod->GetInfo();
                if ( pInfo )
                {
                    OutParamMap aOutParamMap;
                    for ( USHORT n = 1, nCount = xSbxParams->Count(); n < nCount; ++n )
                    {
                        const SbxParamInfo* pParamInfo = pInfo->GetParam( n );
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
            m_pMethod->SetParameters( NULL );
        }

        return aReturn;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
