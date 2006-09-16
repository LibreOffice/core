/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cloneable.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:53:25 $
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
#include "precompiled_forms.hxx"

#ifndef FORMS_COMPONENT_CLONEABLE_HXX
#include "cloneable.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;

    //====================================================================
    //= OCloneableAggregation
    //====================================================================
    //------------------------------------------------------------------
    Reference< XAggregation > OCloneableAggregation::createAggregateClone( const OCloneableAggregation* _pOriginal )
    {
        Reference< XCloneable > xAggregateCloneable;    // will be the aggregate's XCloneable
        Reference< XAggregation > xAggregateClone;      // will be the aggregate's clone

        if ( query_aggregation( _pOriginal->m_xAggregate, xAggregateCloneable ) )
        {
            xAggregateClone = xAggregateClone.query( xAggregateCloneable->createClone() );
            DBG_ASSERT( xAggregateClone.is(), "OCloneableAggregation::createAggregateClone: invalid clone returned by the aggregate!" );
        }
        else
            DBG_ASSERT( !_pOriginal->m_xAggregate.is(), "OCloneableAggregation::createAggregateClone: aggregate is not cloneable!" );

        return xAggregateClone;
    }

//.........................................................................
}   // namespace frm
//.........................................................................

