/*************************************************************************
 *
 *  $RCSfile: basmethnode.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2003-09-23 10:09:14 $
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

#ifndef SCRIPTING_BASMETHNODE_HXX
#include "basmethnode.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_BROWSE_BROWSENODETYPES_HPP_
#include <drafts/com/sun/star/script/framework/browse/BrowseNodeTypes.hpp>
#endif

#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMETH_HXX
#include <basic/sbmeth.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;


//.........................................................................
namespace basprov
{
//.........................................................................

    // =============================================================================
    // BasicMethodNodeImpl
    // =============================================================================

    BasicMethodNodeImpl::BasicMethodNodeImpl( SbMethod* pMethod )
        :m_pMethod( pMethod )
    {
    }

    // -----------------------------------------------------------------------------

    BasicMethodNodeImpl::~BasicMethodNodeImpl()
    {
    }

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicMethodNodeImpl::getName(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        ::rtl::OUString sMethodName;
        if ( m_pMethod )
            sMethodName = m_pMethod->GetName();

        return sMethodName;
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicMethodNodeImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return Sequence< Reference< browse::XBrowseNode > >();
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMethodNodeImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return sal_False;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicMethodNodeImpl::getType(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return browse::BrowseNodeTypes::SCRIPT;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................
