/*************************************************************************
 *
 *  $RCSfile: basmodnode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:06:40 $
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

#ifndef SCRIPTING_BASMODNODE_HXX
#include "basmodnode.hxx"
#endif

#ifndef SCRIPTING_BASMETHNODE_HXX
#include "basmethnode.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_BROWSE_BROWSENODETYPES_HPP_
#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>
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


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script;


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
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        ::rtl::OUString sModuleName;
        if ( m_pModule )
            sModuleName = m_pModule->GetName();

        return sModuleName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicModuleNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

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
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

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
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return browse::BrowseNodeTypes::CONTAINER;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
