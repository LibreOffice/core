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

#include "cloneable.hxx"

#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/uno3.hxx>
#include <tools/debug.hxx>
namespace binfilter {

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
        Reference< XCloneable > xAggregateCloneable;	// will be the aggregates XCloneable
        Reference< XAggregation > xAggregateClone;		// will be the aggregates clone

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
}	// namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
