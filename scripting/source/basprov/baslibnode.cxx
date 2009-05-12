/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: baslibnode.cxx,v $
 * $Revision: 1.10 $
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
#include "baslibnode.hxx"
#include "basmodnode.hxx"
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbstar.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;


//.........................................................................
namespace basprov
{
//.........................................................................

    // =============================================================================
    // BasicLibraryNodeImpl
    // =============================================================================

    BasicLibraryNodeImpl::BasicLibraryNodeImpl( const Reference< XComponentContext >& rxContext,
         const ::rtl::OUString& sScriptingContext, BasicManager* pBasicManager,
        const Reference< script::XLibraryContainer >& xLibContainer, const ::rtl::OUString& sLibName, bool isAppScript )
        :m_xContext( rxContext )
    ,m_sScriptingContext( sScriptingContext )
        ,m_pBasicManager( pBasicManager )
        ,m_xLibContainer( xLibContainer )
        ,m_sLibName( sLibName )
        ,m_bIsAppScript( isAppScript )
    {
        if ( m_xLibContainer.is() )
        {
            Any aElement = m_xLibContainer->getByName( m_sLibName );
            aElement >>= m_xLibrary;
        }
    }

    // -----------------------------------------------------------------------------

    BasicLibraryNodeImpl::~BasicLibraryNodeImpl()
    {
    }

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicLibraryNodeImpl::getName(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return m_sLibName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicLibraryNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        Sequence< Reference< browse::XBrowseNode > > aChildNodes;

        if ( m_xLibContainer.is() && m_xLibContainer->hasByName( m_sLibName ) && !m_xLibContainer->isLibraryLoaded( m_sLibName ) )
            m_xLibContainer->loadLibrary( m_sLibName );

        if ( m_pBasicManager )
        {
            StarBASIC* pBasic = m_pBasicManager->GetLib( m_sLibName );
            if ( pBasic && m_xLibrary.is() )
            {
                Sequence< ::rtl::OUString > aNames = m_xLibrary->getElementNames();
                sal_Int32 nCount = aNames.getLength();
                const ::rtl::OUString* pNames = aNames.getConstArray();
                aChildNodes.realloc( nCount );
                Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();

                for ( sal_Int32 i = 0 ; i < nCount ; ++i )
                {
                    SbModule* pModule = pBasic->FindModule( pNames[i] );
                    if ( pModule )
                        pChildNodes[i] = static_cast< browse::XBrowseNode* >( new BasicModuleNodeImpl( m_xContext, m_sScriptingContext, pModule, m_bIsAppScript ) );
                }
            }
        }

        return aChildNodes;
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicLibraryNodeImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Bool bReturn = sal_False;
        if ( m_xLibrary.is() )
            bReturn = m_xLibrary->hasElements();

        return bReturn;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicLibraryNodeImpl::getType(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return browse::BrowseNodeTypes::CONTAINER;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
