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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <tools/debug.hxx>
#include <svx/svdobj.hxx>
#include "shapelist.hxx"

#include <algorithm>

using namespace sd;

ShapeList::ShapeList()
{
    maIter = maShapeList.end();
}

ShapeList::~ShapeList()
{
    clear();
}

/** adds the given shape to this list */
void ShapeList::addShape( SdrObject& rObject )
{
    ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
    if( aIter == maShapeList.end() )
    {
        maShapeList.push_back(&rObject);
        rObject.AddObjectUser( *this );
    }
    else
    {
        OSL_FAIL("sd::ShapeList::addShape(), given shape already part of list!");
    }
}

/** removes the given shape from this list */
SdrObject* ShapeList::removeShape( SdrObject& rObject )
{
    ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
    if( aIter != maShapeList.end() )
    {
        bool bIterErased = aIter == maIter;

        (*aIter)->RemoveObjectUser(*this);
        aIter = maShapeList.erase( aIter );

        if( bIterErased )
            maIter = aIter;

        if( aIter != maShapeList.end() )
            return (*aIter);
    }
    else
    {
        OSL_FAIL("sd::ShapeList::removeShape(), given shape not part of list!");
    }
    return 0;
}

/** removes all shapes from this list
    NOTE: iterators will become invalid */
void ShapeList::clear()
{
    ListImpl aShapeList;
    aShapeList.swap( maShapeList );

    ListImpl::iterator aIter( aShapeList.begin() );
    while( aIter != aShapeList.end() )
        (*aIter++)->RemoveObjectUser(*this);

    maIter = aShapeList.end();
}

/** returns true if this list is empty */
bool ShapeList::isEmpty() const
{
    return maShapeList.empty();
}

/** returns true if given shape is part of this list */
bool ShapeList::hasShape( SdrObject& rObject ) const
{
    return std::find( maShapeList.begin(), maShapeList.end(), &rObject )  != maShapeList.end();
}

SdrObject* ShapeList::getNextShape(SdrObject* pObj) const
{
    if( pObj )
    {
        ListImpl::const_iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), pObj ) );
        if( aIter != maShapeList.end() )
        {
            aIter++;
            if( aIter != maShapeList.end() )
            {
                return (*aIter);
            }
        }
    }
    else if( !maShapeList.empty() )
    {
        return (*maShapeList.begin());
    }

    return 0;
}

void ShapeList::ObjectInDestruction(const SdrObject& rObject)
{
    ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
    if( aIter != maShapeList.end() )
    {
        bool bIterErased = aIter == maIter;

        aIter = maShapeList.erase( aIter );

        if( bIterErased )
            maIter = aIter;
    }
    else
    {
        OSL_FAIL("sd::ShapeList::ObjectInDestruction(), got a call from an unknown friend!");
    }
}

SdrObject* ShapeList::getNextShape()
{
    if( maIter != maShapeList.end() )
    {
        return (*maIter++);
    }
    else
    {
        return 0;
    }
}

void ShapeList::seekShape( sal_uInt32 nIndex )
{
    maIter = maShapeList.begin();
    while( nIndex-- && (maIter != maShapeList.end()) )
        maIter++;
}

bool ShapeList::hasMore() const
{
    return maIter != maShapeList.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
