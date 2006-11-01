/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapelist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 14:14:30 $
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

#ifndef _SHAPELIST_HXX
#define _SHAPELIST_HXX

#ifndef _SDR_OBJECTUSER_HXX
#include <svx/sdrobjectuser.hxx>
#endif

#include <list>

namespace sd
{
    class ShapeList : public sdr::ObjectUser
    {
    public:
        ShapeList();
        virtual ~ShapeList();

        /** adds the given shape to this list */
        void addShape( SdrObject& rObject );

        /** removes the shape from this list and returns
            a pointer to the next shape in list or 0*/
        SdrObject* removeShape( SdrObject& rObject );

        /** removes all shapes from this list */
        void clear();

        /** returns true if this list is empty */
        bool isEmpty() const;

        /** returns true if given shape is part of this list */
        bool hasShape( SdrObject& rObject ) const;

        /** returns the shape following the given shape in the list or 0
            returns the first shape if pObj is 0 */
        SdrObject* getNextShape(SdrObject* pObj) const;

        /** returns the shape prior to the given shape in the list or 0
            returns the last shape if pObj is 0 */
        SdrObject* getPreviousShape( SdrObject* pObj ) const;

        /**
        */
        SdrObject* getNextShape();

        /**
        */
        void seekShape( sal_uInt32 nIndex );

        /**
        */
        bool hasMore() const;

    private:
        virtual void ObjectInDestruction(const SdrObject& rObject);

        typedef std::list< SdrObject* > ListImpl;
        ListImpl maShapeList;
        ListImpl::iterator maIter;
    };
}

#endif     // _SHAPELIST_HXX
