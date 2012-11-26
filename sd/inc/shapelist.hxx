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



#ifndef _SHAPELIST_HXX
#define _SHAPELIST_HXX

#include <list>

namespace sd
{
    class ShapeList : public SfxListener
    {
    public:
        ShapeList();
        virtual ~ShapeList();

        /** adds the given shape to this list */
        void addShape( const SdrObject& rObject );

        /** removes the shape from this list and returns
            a pointer to the next shape in list or 0*/
        const SdrObject* removeShape( const SdrObject& rObject );

        /** removes all shapes from this list */
        void clear();

        /** returns true if this list is empty */
        bool isEmpty() const;

        /** returns true if given shape is part of this list */
        bool hasShape( const SdrObject& rObject ) const;

        /** returns the shape following the given shape in the list or 0
            returns the first shape if pObj is 0 */
        const SdrObject* getNextShape(const SdrObject* pObj) const;

        /**
        */
        const SdrObject* getNextShape();

        /**
        */
        void seekShape( sal_uInt32 nIndex );

        /**
        */
        bool hasMore() const;

        const std::list< const SdrObject* >& getList() const { return maShapeList; }

    private:
        // derived from SfxListener
        virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

        typedef std::list< const SdrObject* > ListImpl;
        ListImpl maShapeList;
        ListImpl::iterator maIter;
    };
}

#endif     // _SHAPELIST_HXX
