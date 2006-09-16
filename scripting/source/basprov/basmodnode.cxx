/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basmodnode.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:25:57 $
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

#ifndef SCRIPTING_BASMODNODE_HXX
#include "basmodnode.hxx"
#endif

#ifndef SCRIPTING_BASMETHNODE_HXX
#include "basmethnode.hxx"
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
                aChildNodes.realloc( nCount );
                Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();

                for ( sal_Int32 i = 0; i < nCount; ++i )
                {
                    SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Get( static_cast< USHORT >( i ) ) );
                    if ( pMethod )
                        pChildNodes[i] = static_cast< browse::XBrowseNode* >( new BasicMethodNodeImpl( m_xContext, m_sScriptingContext, pMethod, m_bIsAppScript ) );
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
