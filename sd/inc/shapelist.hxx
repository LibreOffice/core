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

        /** @return true if this list is empty */
        bool isEmpty() const;

        /** @return true if given shape is part of this list */
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
