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
#include "basmodnode.hxx"
#include "basmethnode.hxx"
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;


//.........................................................................
namespace basprov
{
//.........................................................................

    // =============================================================================
    // BasicModuleNodeImpl
    // =============================================================================

    BasicModuleNodeImpl::BasicModuleNodeImpl( const Reference< XComponentContext >& rxContext,
        const ::rtl::OUString& sScriptingContext, SbModule* pModule, bool isAppScript )
        :m_xContext( rxContext )
        ,m_sScriptingContext( sScriptingContext )
        ,m_pModule( pModule )
        ,m_bIsAppScript( isAppScript )
    {
    }

    // -----------------------------------------------------------------------------

    BasicModuleNodeImpl::~BasicModuleNodeImpl()
    {
    }

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicModuleNodeImpl::getName(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        ::rtl::OUString sModuleName;
        if ( m_pModule )
            sModuleName = m_pModule->GetName();

        return sModuleName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicModuleNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        Sequence< Reference< browse::XBrowseNode > > aChildNodes;

        if ( m_pModule )
        {
            SbxArray* pMethods = m_pModule->GetMethods();
            if ( pMethods )
            {
                sal_Int32 nCount = pMethods->Count();
                sal_Int32 nRealCount = 0;
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Get( static_cast< sal_uInt16 >( i ) ) );
                    if ( pMethod && !pMethod->IsHidden() )
                        ++nRealCount;
                }
                aChildNodes.realloc( nRealCount );
                Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();

                sal_Int32 iTarget = 0;
                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Get( static_cast< sal_uInt16 >( i ) ) );
                    if ( pMethod && !pMethod->IsHidden() )
                        pChildNodes[iTarget++] = static_cast< browse::XBrowseNode* >( new BasicMethodNodeImpl( m_xContext, m_sScriptingContext, pMethod, m_bIsAppScript ) );
                }
            }
        }

        return aChildNodes;
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicModuleNodeImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Bool bReturn = sal_False;
        if ( m_pModule )
        {
            SbxArray* pMethods = m_pModule->GetMethods();
            if ( pMethods && pMethods->Count() > 0 )
                bReturn = sal_True;
        }

        return bReturn;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicModuleNodeImpl::getType(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return browse::BrowseNodeTypes::CONTAINER;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
