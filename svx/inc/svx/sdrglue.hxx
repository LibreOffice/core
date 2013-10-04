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

#ifndef _SDRGLUE_HXX
#define _SDRGLUE_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include "svx/svxdllapi.h"
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <set>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        class SVX_DLLPUBLIC Point
        {
        public:
            // defines for GluePoint alignment (meHorizontalAlignment, meVerticalAlignment)
            enum Alignment
            {
                Alignment_Minimum,
                Alignment_Center,
                Alignment_Maximum
            };

            // defines for GluePoint escape direction (meEscapeDirections). These
            // may be or-ed together to define all allowed escape directions from
            // which the layouter may choose the best possible.
            // Or-ing all allowed values is from the meaning identical to ESCAPE_DIRECTION_SMART.
            static const sal_uInt16 ESCAPE_DIRECTION_SMART = 0;
            static const sal_uInt16 ESCAPE_DIRECTION_LEFT = 1;
            static const sal_uInt16 ESCAPE_DIRECTION_RIGHT = 2;
            static const sal_uInt16 ESCAPE_DIRECTION_TOP = 4;
            static const sal_uInt16 ESCAPE_DIRECTION_BOTTOM = 8;

        private:
            // allow class List access to setID()
            friend class List;

            // position in unit coordinates [0.0 .. 1.0] in X,Y
            basegfx::B2DPoint               maUnitPosition;

            // allowed escape directions, default is ESCAPE_DIRECTION_SMART
            sal_uInt16                      meEscapeDirections;

            // horizontal and vertical alignments. If != Alignment_None the
            // position will change as distance from the defined anchor position.
            Alignment                       meHorizontalAlignment;
            Alignment                       meVerticalAlignment;

            // unique identifier ID. All Points in one list need unique identifiers
            // and will be sorted by these. This is administrated by the List class
            sal_uInt32                      maID;

            /// bitfield
            // if true, position is just relative to unit range (default)
            // if false, position is also relative, but additionally controlled
            // by the Alignment settings when the object it belongs to is scaled
            bool                            mbRelative : 1;

            // needed to separate user-defined points from the ones from CustomShapes
            bool                            mbUserDefined : 1;

            // write access to ID is limited to list class only
            void setID(sal_uInt32 nNew) { maID = nNew; }

            // write access to scale adaption to list class only
            void adaptToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale);
        protected:
        public:
            Point(
                const basegfx::B2DPoint& rUnitPosition = basegfx::B2DPoint(0.5, 0.5),
                sal_uInt16 nEscapeDirections = ESCAPE_DIRECTION_SMART,
                Alignment eHorizontalAlignment = Alignment_Center,
                Alignment eVerticalAlignment = Alignment_Center,
                bool bRelative = true,
                bool bUserDefined = true);

            // get/set UnitPosition. Always in [0.0 .. 1.0] in Y and Y, will be truncated at set
            // and truncated at get
            basegfx::B2DPoint getUnitPosition() const;
            void setUnitPosition(const basegfx::B2DPoint& rNew);

            // get/set allowed EscapeDirections
            sal_uInt16 getEscapeDirections() const { return meEscapeDirections; }
            void setEscapeDirections(sal_uInt16 nNew) { meEscapeDirections = nNew; }

            // get/set HorizontalAlignment
            Alignment getHorizontalAlignment() const { return meHorizontalAlignment; }
            void setHorizontalAlignment(Alignment eNew) { meHorizontalAlignment = eNew; }

            // get/set VerticalAlignment
            Alignment getVerticalAlignment() const { return meVerticalAlignment; }
            void setVerticalAlignment(Alignment eNew) { meVerticalAlignment = eNew; }

            // acess to relative flag. When setting to true the UnitPostion will be
            // internally truncated since the non-relative modes allow values outside
            // the unit range
            bool getRelative() const { return mbRelative; }
            void setRelative(bool bNew);

            // access to UserDefined
            bool getUserDefined() const { return mbUserDefined; }

            // read access to ID (write is private and limitied to list class)
            sal_uInt32 getID() const { return maID; }

            // needed UNO API converters; both rely on the correct absolute scale given since the UNO API
            // definition partially uses sizes of the object the GluePoint belongs to. The converter to
            // sdr::glue::Point is implemented as constructor
            com::sun::star::drawing::GluePoint2 convertToGluePoint2(
                const basegfx::B2DVector& rAbsoluteScale) const;
            Point(
                const com::sun::star::drawing::GluePoint2& rGluePoint2,
                const basegfx::B2DVector& rAbsoluteScale);
        };
    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        // sort by ID
        struct PointComparator
        {
            bool operator()(const Point& rA, const Point& rB) const;
        };

        // typedef for point set
        typedef ::std::set< Point, PointComparator > PointSet;
        typedef ::std::vector< Point* > PointVector;

        class SVX_DLLPUBLIC List
        {
        private:
            // the GluePoint set
            PointSet                maPointSet;

        protected:
        public:
            List()
            :   maPointSet()
            {
            }

            // add new Point, it gets a new ID assigned and a reference to the
            // new instance (copied to list) is returned. It will assert when
            // already added
            Point& add(const Point& rNew);

            // remove Point (will assert if not added)
            void remove(const Point& rNew);

            // find by ID
            Point* findByID(sal_uInt32 nID) const;

            // get vector of Points (pointers to the real points)
            PointVector getVector() const;

            // adapt to changed absolute scale, e.g. when not relative and alignments have to be addressed
            void adaptToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale);
        };
    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDRGLUE_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
