/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cloneable.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_forms.hxx"
#include "cloneable.hxx"
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/uno3.hxx>
#include <tools/debug.hxx>

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

