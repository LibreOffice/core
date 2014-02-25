/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "cloneable.hxx"
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/uno3.hxx>
#include <tools/debug.hxx>


namespace frm
{


    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;


    //= OCloneableAggregation


    Reference< XAggregation > OCloneableAggregation::createAggregateClone( const OCloneableAggregation* _pOriginal )
    {
        Reference< XCloneable > xAggregateCloneable;    // will be the aggregate's XCloneable
        Reference< XAggregation > xAggregateClone;      // will be the aggregate's clone

        if ( query_aggregation( _pOriginal->m_xAggregate, xAggregateCloneable ) )
        {
            xAggregateClone = xAggregateClone.query( xAggregateCloneable->createClone() );
            DBG_ASSERT( xAggregateClone.is(), "OCloneableAggregation::createAggregateClone: invalid clone returned by the aggregate!" );
        }
        else {
            DBG_ASSERT( !_pOriginal->m_xAggregate.is(), "OCloneableAggregation::createAggregateClone: aggregate is not cloneable!" );
        }

        return xAggregateClone;
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
