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

#include <svx/svdobj.hxx>
#include <osl/diagnose.h>
#include <shapelist.hxx>

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
    if( maShapeSet.insert(&rObject).second )
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
void ShapeList::removeShape( SdrObject& rObject )
{
    if( maShapeSet.erase(&rObject) )
    {
        ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
        bool bIterErased = aIter == maIter;

        (*aIter)->RemoveObjectUser(*this);
        aIter = maShapeList.erase( aIter );

        if( bIterErased )
            maIter = aIter;
    }
    else
    {
        OSL_FAIL("sd::ShapeList::removeShape(), given shape not part of list!");
    }
}

/** removes all shapes from this list
    NOTE: iterators will become invalid */
void ShapeList::clear()
{
    ListImpl aShapeList;
    aShapeList.swap( maShapeList );
    maShapeSet.clear();

    for( auto& rpShape : aShapeList )
        rpShape->RemoveObjectUser(*this);

    maIter = maShapeList.end();
}

/** returns true if this list is empty */
bool ShapeList::isEmpty() const
{
    return maShapeList.empty();
}

/** returns true if given shape is part of this list */
bool ShapeList::hasShape( SdrObject& rObject ) const
{
    return maShapeSet.contains( &rObject );
}

void ShapeList::ObjectInDestruction(const SdrObject& rObject)
{
    if( maShapeSet.erase(&rObject) )
    {
        ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
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
        return nullptr;
    }
}

void ShapeList::seekShape( sal_uInt32 nIndex )
{
    maIter = maShapeList.begin();
    nIndex = std::min(nIndex, static_cast<sal_uInt32>(maShapeList.size()));
    std::advance(maIter, nIndex);
}

bool ShapeList::hasMore() const
{
    return maIter != maShapeList.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
