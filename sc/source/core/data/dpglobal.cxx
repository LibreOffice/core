/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
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
#include "precompiled_sc.hxx"

#include "dpglobal.hxx"
#include "document.hxx"
#include "dpobject.hxx"

#include <stdio.h>

namespace ScDPGlobal
{
    Rectangle operator *( const Rectangle &rLeft, const std::pair<double,double> & rRight )
    {
        Rectangle rcResult( rLeft );
        rcResult.Bottom() = rcResult.Top() + static_cast<long>( rcResult.GetHeight() * rRight.second );
        rcResult.Right() = rcResult.Left() + static_cast<long>( rcResult.GetWidth() * rRight.first);
        return rcResult;
    }

}

ScDPItemDataPool::ScDPItemDataPool()
{
}

ScDPItemDataPool::ScDPItemDataPool(const ScDPItemDataPool& r):
    maItems(r.maItems),
    maItemIds(r.maItemIds)
{
}

ScDPItemDataPool::~ScDPItemDataPool()
{
}


const ScDPItemData* ScDPItemDataPool::getData( sal_Int32 nId )
{
    if ( nId >= static_cast<sal_Int32>(maItems.size()) )
        return NULL;
    else
        return &(maItems[nId]);
}

sal_Int32 ScDPItemDataPool::getDataId( const ScDPItemData& aData )
{
    DataHash::const_iterator itr = maItemIds.find( aData),
            itrEnd = maItemIds.end();
    if ( itr == itrEnd )
         // not exist
        return -1;

    else //exist
        return itr->second;

}

sal_Int32 ScDPItemDataPool::insertData( const ScDPItemData& aData )
{
    sal_Int32 nResult = getDataId( aData );

    if( nResult < 0 )
    {
        maItemIds.insert( DataHash::value_type( aData, nResult = maItems.size() ) );
        maItems.push_back( aData );
    }

    return nResult;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
