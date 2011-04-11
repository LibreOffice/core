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

#ifndef _SHAPELIST_HXX
#define _SHAPELIST_HXX

#include <svx/sdrobjectuser.hxx>

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

        /**
        */
        SdrObject* getNextShape();

        /**
        */
        void seekShape( sal_uInt32 nIndex );

        /**
        */
        bool hasMore() const;

        const std::list< SdrObject* >& getList() const { return maShapeList; }

    private:
        virtual void ObjectInDestruction(const SdrObject& rObject);

        typedef std::list< SdrObject* > ListImpl;
        ListImpl maShapeList;
        ListImpl::iterator maIter;
    };
}

#endif     // _SHAPELIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
