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

#ifndef _SDRSELECTION_HXX
#define _SDRSELECTION_HXX

#include <sal/types.h>
#include "svx/svxdllapi.h"
#include <svx/svdobj.hxx>
#include <boost/utility.hpp>
#include <svl/lstner.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>
#include <set>
#include <map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Predefines

class SdrView;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// selection definitions

namespace sdr
{
    namespace selection
    {
        // sort by NavigationPosition
        struct SelectionComparator
        {
            bool operator()(SdrObject* pA, SdrObject* pB) const;
        };

        // sort by SdrObject memory address
        struct IndicesComparator
        {
            bool operator()(SdrObject* pA, SdrObject* pB) const;
        };

        // typedefs for point indices, 2way associative object-index-map and SelectionSet
        typedef ::std::set< sal_uInt32 > Indices;
        typedef ::std::map< SdrObject*, Indices, IndicesComparator > IndicesMap;
        typedef ::std::set< SdrObject*, SelectionComparator > SelectionSet;

        // basic selection calss with minimal interface
        class SVX_DLLPUBLIC Selection : private boost::noncopyable, public SfxListener, public Timer
        {
        private:
            // view this incarnation belongs to exclusively
            SdrView&            mrSdrView;

            // the selection itself as std::set
            SelectionSet        maSet;

            // evtl. selected points and/or gluepoints
            IndicesMap          maPointIndices;
            IndicesMap          maGlueIndices;

            // buffered: selection in vector form
            SdrObjectVector     maVector;

            // buffered: descriptions
            String              maPointDescription;
            String              maGlueDescription;

            // buffered: range of selection
            basegfx::B2DRange   maSnapRange;

            // local tooling
            void delayedSelectionChanged();
            void createIndexDescription(bool bPoints);
            void checkPointIndexCorrection();
            void checkGlueIndexCorrection();
            void resetBufferedSelectionInformation() { maVector.clear(); maSnapRange.reset(); }
            void resetBufferedPointInformation() { maPointDescription.Erase(); }
            void resetBufferedGlueInformation() { maGlueDescription.Erase(); }

            // derived from SfxListener
            virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

            // derived from Timer
            virtual void Timeout();

        public:
            /// constructor
            Selection(SdrView& rSdrView);

            /// test if selection change is pending
            bool isSelectionChangePending() const;

            /// force evtl. delayed SelectionChange to be broadcasted
            void forceSelectionChange();

            /// const accesses
            const SdrObjectVector& getVector() const;
            SdrObject* getSingle() const;
            bool empty() const;
            sal_uInt32 size() const;
            bool isSdrObject(const SdrObject& rObject) const;
            Indices getIndicesForSdrObject(const SdrObject& rObject, bool bPoints) const;
            const String& getIndexDescription(bool bPoints) const;
            bool hasIndices(bool bPoints) const;
            const basegfx::B2DRange& getSnapRange() const;

            // accesses which change the selection
            void removeSdrObject(const SdrObject& rObject);
            void addSdrObject(const SdrObject& rObject);
            void clear();
            void setSdrObjects(const SdrObjectVector& rSdrObjectVector);
            void setIndicesForSdrObject(const SdrObject& rObject, const Indices& rNew, bool bPoints);
            void clearIndicesForSdrObject(const SdrObject& rObject, bool bPoints);
        };
    } // end of namespace selection
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDRSELECTION_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
