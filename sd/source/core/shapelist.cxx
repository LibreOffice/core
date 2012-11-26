/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <tools/debug.hxx>
#include <svx/svdobj.hxx>
#include "shapelist.hxx"
#include <algorithm>
#include <svx/svdmodel.hxx>

using namespace sd;

ShapeList::ShapeList()
:   SfxListener()
{
    maIter = maShapeList.end();
}

ShapeList::~ShapeList()
{
    clear();
}

/** adds the given shape to this list */
void ShapeList::addShape( const SdrObject& rObject )
{
    ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
    if( aIter == maShapeList.end() )
    {
        maShapeList.push_back(&rObject);
        StartListening(const_cast< SdrObject& >(rObject));
    }
    else
    {
        DBG_ERROR("sd::ShapeList::addShape(), given shape already part of list!");
    }
}

/** removes the given shape from this list */
const SdrObject* ShapeList::removeShape( const SdrObject& rObject )
{
    ListImpl::iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), &rObject ) );
    if( aIter != maShapeList.end() )
    {
        bool bIterErased = aIter == maIter;

        EndListening(const_cast< SdrObject& >(rObject));
        aIter = maShapeList.erase( aIter );

        if( bIterErased )
            maIter = aIter;

        if( aIter != maShapeList.end() )
            return (*aIter);
    }
    else
    {
        DBG_ERROR("sd::ShapeList::removeShape(), given shape not part of list!");
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
    {
        EndListening(const_cast< SdrObject& >(**aIter++));
    }

    maIter = aShapeList.end();
}

/** returns true if this list is empty */
bool ShapeList::isEmpty() const
{
    return maShapeList.empty();
}

/** returns true if given shape is part of this list */
bool ShapeList::hasShape( const SdrObject& rObject ) const
{
    return std::find( maShapeList.begin(), maShapeList.end(), &rObject )  != maShapeList.end();
}

const SdrObject* ShapeList::getNextShape(const SdrObject* pObj) const
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

// derived from SfxListener
void ShapeList::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrHint && HINT_SDROBJECTDYING == pSdrHint->GetSdrHintKind() && pSdrHint->GetSdrHintObject())
    {
        ListImpl::iterator aIter(std::find(maShapeList.begin(), maShapeList.end(), pSdrHint->GetSdrHintObject()));

    if( aIter != maShapeList.end() )
    {
        bool bIterErased = aIter == maIter;

        aIter = maShapeList.erase( aIter );

        if( bIterErased )
            maIter = aIter;
    }
    else
    {
        DBG_ERROR("sd::ShapeList::ObjectInDestruction(), got a call from an unknown friend!");
    }
}
}

const SdrObject* ShapeList::getNextShape()
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
