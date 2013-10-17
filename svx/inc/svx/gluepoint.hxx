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

#ifndef _GLUEPOINT_HXX
#define _GLUEPOINT_HXX

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
        // The GluePoint class describes possible GluePoints for objects which support
        // a GluePointProvider (see below). GluePoints are principally in unit coordinates
        // so that they do not have to be transformed with the object they belong to.
        // Their coordinateas will always be truncated to the unit range (0, 0, 1, 1)
        // when used, so being always inside that range and thus cannot leave the
        // object. Internally, values outside that range are used to model GluePoint
        // states when not relative and Alignment is set to keep that alignment alive
        // even when the positions are moved outside the object
        class SVX_DLLPUBLIC GluePoint
        {
        public:
            // defines for GluePoint alignment (see members meHorizontalAlignment,
            // meVerticalAlignment)
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
            // allow class GluePointProvider access to setID() in setIdAtGluePoint()
            friend class GluePointProvider;

            // position in unit coordinates [0.0 .. 1.0] in X,Y
            // may be internally outside the unit range when the GluePoint is not relative
            basegfx::B2DPoint               maUnitPosition;

            // allowed escape directions, default is ESCAPE_DIRECTION_SMART
            sal_uInt16                      meEscapeDirections;

            // horizontal and vertical alignments. If != Alignment_None the
            // position will change as distance from the defined anchor position.
            Alignment                       meHorizontalAlignment;
            Alignment                       meVerticalAlignment;

            // unique identifier ID. All GluePoints in one list need unique identifiers
            // and will be sorted by these. This is administrated by the
            // GluePointProvider class
            sal_uInt32                      maID;

            /// bitfield
            // if true, position is just relative to unit range (default)
            // if false, position is also relative, but additionally controlled
            // by the Alignment settings when the object it belongs to is scaled
            bool                            mbRelative : 1;

            // needed to separate user-defined GluePoints from the ones from
            // CustomShapes (custom-defined GluePoints)
            bool                            mbUserDefined : 1;

            // write access to ID is limited to GluePointProvider class only
            void setID(sal_uInt32 nNew) { maID = nNew; }

            // write access to scale adaption to GluePointProvider class only
            void adaptToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale);

        protected:
        public:
            GluePoint(
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

            // access to UserDefined (read only, define at construction time)
            bool getUserDefined() const { return mbUserDefined; }

            // read access to ID (write is private and limitied to list class)
            sal_uInt32 getID() const { return maID; }

            // needed UNO API converters; both rely on the correct absolute scale given since the UNO API
            // definition partially uses the sizes of the object the GluePoint belongs to. The converter to
            // sdr::glue::GluePoint is implemented as constructor
            com::sun::star::drawing::GluePoint2 convertToGluePoint2(
                const basegfx::B2DVector& rAbsoluteScale) const;
            GluePoint(
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
        // typedef for GluePointVector
        typedef ::std::vector< GluePoint* > GluePointVector;

        // This is the basic GluePoint providing interface; an instance of this class is
        // provided by objects supporting GluePoints (e.g. SdrObjects, using
        // GetGluePointProvider()).
        // There are two basic kinds of GluePoints: AutoGluePoints and UserGluePoints.
        //
        // AutoGluePoints are constant GluePoints offered by the object, e.g. the standard
        // left/right/top/back.
        //
        // UserGluePoints may be freely added/edited by the user, but there may also be
        // implementations which do not support UserGluePoints at all (SdrEdgeObj). There may
        // also be implementations which add 'fixed' UserGluePoints for which delete/edit
        // will be ignored, e.g. standard GluePoints added from a CustomShape, but not the
        // user. These will return false on getUserDefined().
        //
        // This basic implementation implements:
        // - standard AutoGluePoints (left/right/top/back)
        // - no UserGluePoints

        class SVX_DLLPUBLIC GluePointProvider
        {
        private:
        protected:
            // copy constructor and assignment are protected, use Clone() to create copies
            // with content
            GluePointProvider(const GluePointProvider& rCandidate);
            virtual GluePointProvider& operator=(const GluePointProvider& rCandidate);

            // internal access to GluePoint::setID; this is internal functionality and
            // only the basic implementations of GluePointProvider::addUserGluePoint should use it
            void setIdAtGluePoint(GluePoint& rGluePoint, sal_uInt32 nNew)
            {
                rGluePoint.setID(nNew);
            }

            // internal access to GluePoint::adaptToChangedScale; this is internal functionality and
            // only the basic implementations of GluePointProvider::adaptUserGluePointsToChangedScale
            // should use it
            void adaptGluePointToChangedScale(GluePoint& rGluePoint, const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale)
            {
                rGluePoint.adaptToChangedScale(rOldScale, rNewScale);
            }

        public:
            // construction, destruction
            GluePointProvider();
            virtual ~GluePointProvider();

            // copying
            virtual GluePointProvider* Clone() const;

            // AutoGluePoint read access (read only, the GluePoints returned are
            // usually created on the fly). The count is currently fixed to 4,
            // mainly because the UNO API implementations rely on this
            virtual sal_uInt32 getAutoGluePointCount() const;
            virtual GluePoint getAutoGluePointByIndex(sal_uInt32 nIndex) const;

            // flag if UserGluePoints are allowed (e.g. not for SdrEdgeObj), needs
            // to be checked before calling addUserGluePoint
            virtual bool allowsUserGluePoints() const;

            // add new GluePoint, it gets internally a new ID assigned and a
            // reference to the new instance (copied to list) is returned. It
            // will assert when already added or could not be added. To check
            // if it was added, compare the address of the return value with
            // the address of rNew
            virtual GluePoint& addUserGluePoint(GluePoint& rNew);

            // remove GluePoint (will assert if it was not added)
            virtual void removeUserGluePoint(const GluePoint& rNew);

            // check on content
            virtual bool hasUserGluePoints() const;

            // find UserGluePoint by ID
            virtual GluePoint* findUserGluePointByID(sal_uInt32 nID) const;

            // get vector of UserGluePoints (pointers to the real points). This
            // makes accesses easier, but should only be used as temporary data
            virtual const GluePointVector getUserGluePointVector() const;

            // adapt UserGluePoints to changed absolute scale, e.g. when not relative and alignments have to be addressed
            virtual void adaptUserGluePointsToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale);
        };
    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        // operator for ::std::set to sort GluePoints by ID
        struct GluePointComparator
        {
            bool operator()(const GluePoint& rA, const GluePoint& rB) const;
        };

        // typedef for GluePoint set
        typedef ::std::set< GluePoint, GluePointComparator > GluePointSet;

        // This derivation of GluePointProvider extends the standard one to
        // support basic UserGluePoints in all aspects (new/edit/delete)

        class SVX_DLLPUBLIC StandardGluePointProvider : public GluePointProvider
        {
        private:
            // the GluePoint set for UserGluePoints. Use a set sorted by ID
            // to be able to implement findUserGluePointByID effectively
            GluePointSet                maGluePointSet;

        protected:
            StandardGluePointProvider(const StandardGluePointProvider& rCandidate);
            virtual GluePointProvider& operator=(const GluePointProvider& rCandidate);

        public:
            // construction, destruction, copying
            StandardGluePointProvider();
            virtual ~StandardGluePointProvider();

            // copying
            virtual GluePointProvider* Clone() const;

            // flag if UserGluePoints are allowed (e.g. not for SdrEdgeObj)
            virtual bool allowsUserGluePoints() const;

            // add new GluePoint, it gets a new ID assigned and a reference to the
            // new instance (copied to list) is returned. It will assert when
            // already added
            virtual GluePoint& addUserGluePoint(GluePoint& rNew);

            // remove GluePoint (will assert if it was not added)
            virtual void removeUserGluePoint(const GluePoint& rNew);

            // check on content
            virtual bool hasUserGluePoints() const;

            // find UserGluePoint by ID
            virtual GluePoint* findUserGluePointByID(sal_uInt32 nID) const;

            // get vector of UserGluePoints (pointers to the real points)
            virtual const GluePointVector getUserGluePointVector() const;

            // adapt UserGluePoints to changed absolute scale, e.g. when not relative and alignments have to be addressed
            virtual void adaptUserGluePointsToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale);
        };
    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_GLUEPOINT_HXX

// eof
