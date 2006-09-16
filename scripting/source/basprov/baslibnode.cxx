/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baslibnode.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:25:28 $
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

#ifndef SCRIPTING_BASLIBNODE_HXX
#include "baslibnode.hxx"
#endif

#ifndef SCRIPTING_BASMODNODE_HXX
#include "basmodnode.hxx"
#endif

#ifndef _COM_SUN_STAR_SCRIPT_BROWSE_BROWSENODETYPES_HPP_
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif


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
