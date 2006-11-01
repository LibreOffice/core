/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapelist.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 14:14:58 $
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
#include "precompiled_sd.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _SHAPELIST_HXX
#include "shapelist.hxx"
#endif

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
        DBG_ERROR("sd::ShapeList::addShape(), given shape already part of list!");
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

SdrObject* ShapeList::getPreviousShape( SdrObject* pObj ) const
{
    if( pObj )
    {
        ListImpl::const_iterator aIter( std::find( maShapeList.begin(), maShapeList.end(), pObj ) );
        if( (aIter != maShapeList.end()) && (aIter != maShapeList.begin()) )
        {
            aIter--;
            return (*aIter);
        }
    }
    else if( !maShapeList.empty() )
    {
        return (*--maShapeList.end());
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
        DBG_ERROR("sd::ShapeList::ObjectInDestruction(), got a call from an unknown friend!");
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
