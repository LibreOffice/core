/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include "basmodnode.hxx"
#include "basmethnode.hxx"
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <osl/mutex.hxx>
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
        SolarMutexGuard aGuard;

        ::rtl::OUString sModuleName;
        if ( m_pModule )
            sModuleName = m_pModule->GetName();

        return sModuleName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicModuleNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

        return browse::BrowseNodeTypes::CONTAINER;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
