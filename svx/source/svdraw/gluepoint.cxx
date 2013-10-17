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
#include "precompiled_svx.hxx"

#include <svx/gluepoint.hxx>
#include <basegfx/range/b2drange.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        GluePoint::GluePoint(
            const basegfx::B2DPoint& rUnitPosition,
            sal_uInt16 eEscapeDirections,
            Alignment eHorizontalAlignment,
            Alignment eVerticalAlignment,
            bool bRelative,
            bool bUserDefined)
        :   maUnitPosition(basegfx::B2DRange::getUnitB2DRange().clamp(rUnitPosition)),
            meEscapeDirections(eEscapeDirections),
            meHorizontalAlignment(eHorizontalAlignment),
            meVerticalAlignment(eVerticalAlignment),
            maID(0),
            mbRelative(bRelative),
            mbUserDefined(bUserDefined)
        {
        }

        basegfx::B2DPoint GluePoint::getUnitPosition() const
        {
            return basegfx::B2DRange::getUnitB2DRange().clamp(maUnitPosition);
        }

        void GluePoint::setUnitPosition(const basegfx::B2DPoint& rNew)
        {
            const basegfx::B2DPoint aClampedNew(basegfx::B2DRange::getUnitB2DRange().clamp(rNew));

            if(aClampedNew != maUnitPosition)
            {
                maUnitPosition = aClampedNew;
            }
        }

        void GluePoint::setRelative(bool bNew)
        {
            if(mbRelative != bNew)
            {
                mbRelative = bNew;

                if(mbRelative)
                {
                    // truncate UnitPosition when switching off; the non-relative mode allows
                    // values outside the UnitRange to represent positions moving outside the
                    // range
                    maUnitPosition = basegfx::B2DRange::getUnitB2DRange().clamp(maUnitPosition);
                }
            }
        }

        void GluePoint::adaptToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale)
        {
            if(!getRelative())
            {
                const bool bChangeX(!basegfx::fTools::equal(rOldScale.getX(), rNewScale.getX()));
                const bool bChangeY(!basegfx::fTools::equal(rOldScale.getY(), rNewScale.getY()));

                if(bChangeX || bChangeY)
                {
                    // do not use getUnitPosition() here, we do not want to clamp the value
                    basegfx::B2DPoint aChangedPos(maUnitPosition);

                    if(bChangeX)
                    {
                        // avoid using values below 1.0 when working with relative scales; thus even the
                        // values outside the unit range will be preserved
                        const double fCorrectedOldX(std::max(1.0, rOldScale.getX()));
                        const double fCorrectedNewX(std::max(1.0, rNewScale.getX()));

                        switch(getHorizontalAlignment())
                        {
                            case GluePoint::Alignment_Minimum:
                            {
                                // anchored left
                                aChangedPos.setX((aChangedPos.getX() * fCorrectedOldX) / fCorrectedNewX);
                                break;
                            }
                            case GluePoint::Alignment_Center:
                            {
                                aChangedPos.setX(((0.5 * fCorrectedNewX) + (fCorrectedOldX * (aChangedPos.getX() - 0.5))) / fCorrectedNewX);
                                break;
                            }
                            default: // case GluePoint::Alignment_Maximum:
                            {
                                aChangedPos.setX((fCorrectedNewX - (fCorrectedOldX * (1.0 - aChangedPos.getX()))) / fCorrectedNewX);
                                break;
                            }
                        }
                    }

                    if(bChangeY)
                    {
                        // avoid using values below 1.0 when working with relative scales; thus even the
                        // values outside the unit range will be preserved
                        const double fCorrectedOldY(std::max(1.0, rOldScale.getY()));
                        const double fCorrectedNewY(std::max(1.0, rNewScale.getY()));

                        switch(getVerticalAlignment())
                        {
                            case GluePoint::Alignment_Minimum:
                            {
                                // anchored left
                                aChangedPos.setY((aChangedPos.getY() * fCorrectedOldY) / fCorrectedNewY);
                                break;
                            }
                            case GluePoint::Alignment_Center:
                            {
                                aChangedPos.setY(((0.5 * fCorrectedNewY) + (fCorrectedOldY * (aChangedPos.getY() - 0.5))) / fCorrectedNewY);
                                break;
                            }
                            default: // case GluePoint::Alignment_Maximum:
                            {
                                aChangedPos.setY((fCorrectedNewY - (fCorrectedOldY * (1.0 - aChangedPos.getY()))) / fCorrectedNewY);
                                break;
                            }
                        }
                    }

                    if(!aChangedPos.equal(maUnitPosition))
                    {
                        // do not use setUnitPosition() here, we do not want to clamp the value
                        maUnitPosition = aChangedPos;
                    }
                }
            }
        }

        com::sun::star::drawing::GluePoint2 GluePoint::convertToGluePoint2(const basegfx::B2DVector& rAbsoluteScale) const
        {
            com::sun::star::drawing::GluePoint2 aRetval;

            // copy UserDefined
            aRetval.IsUserDefined = getUserDefined();

            // copy relative
            aRetval.IsRelative = getRelative();

            // copy position; get maUnitPosition directly, we do not want to crop here
            double fX(maUnitPosition.getX());
            double fY(maUnitPosition.getY());

            switch(getHorizontalAlignment())
            {
                case Alignment_Minimum: break;
                case Alignment_Center: fX = fX - 0.5; break;
                case Alignment_Maximum: fX = fX - 1.0; break;
            }

            switch(getVerticalAlignment())
            {
                case Alignment_Minimum: break;
                case Alignment_Center: fY = fY - 0.5; break;
                case Alignment_Maximum: fY = fY - 1.0; break;
            }

            if(aRetval.IsRelative)
            {
                fX *= 10000.0;
                fY *= 10000.0;
            }
            else
            {
                // avoid values below 1.0 to not lose relative values outside object scale
                fX *= std::max(1.0, rAbsoluteScale.getX());
                fY *= std::max(1.0, rAbsoluteScale.getY());
            }

            aRetval.Position.X = basegfx::fround(fX);
            aRetval.Position.Y = basegfx::fround(fY);

            // copy alignment
            switch(getHorizontalAlignment())
            {
                case Alignment_Minimum:
                    switch(getVerticalAlignment())
                    {
                        case Alignment_Minimum:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_TOP_LEFT;
                            break;
                        case Alignment_Maximum:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_BOTTOM_LEFT;
                            break;
                        default:
                        case Alignment_Center:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_LEFT;
                            break;
                    }
                    break;
                case Alignment_Maximum:
                    switch(getVerticalAlignment())
                    {
                        case Alignment_Minimum:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_TOP_RIGHT;
                            break;
                        case Alignment_Maximum:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_BOTTOM_RIGHT;
                            break;
                        default:
                        case Alignment_Center:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_RIGHT;
                            break;
                    }
                    break;
                default:
                case Alignment_Center:
                    switch(getVerticalAlignment())
                    {
                        case Alignment_Minimum:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_TOP;
                            break;
                        case Alignment_Maximum:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_BOTTOM;
                            break;
                        default:
                        case Alignment_Center:
                            aRetval.PositionAlignment = com::sun::star::drawing::Alignment_CENTER;
                            break;
                    }
                    break;
            }

            // copy escape directions
            switch( getEscapeDirections() )
            {
                case ESCAPE_DIRECTION_LEFT:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_LEFT;
                    break;
                case ESCAPE_DIRECTION_RIGHT:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_RIGHT;
                    break;
                case ESCAPE_DIRECTION_TOP:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_UP;
                    break;
                case ESCAPE_DIRECTION_BOTTOM:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_DOWN;
                    break;
                case ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_RIGHT:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_HORIZONTAL;
                    break;
                case ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_BOTTOM:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_VERTICAL;
                    break;

                // Unfortunately the enum EscapeDirection in the EscapeDirection.idl definition
                // is wrong in the sense that it does not reflect the possibility to define a free
                // combination of the directions left/right/up/down from which the router may choose
                // the best. Below (and in the idl file) are suggestions how this could be expanded,
                // but it would be incompatible
                //
                //case ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_TOP:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_LEFTUP;
                //    break;
                //case ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_RIGHT:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_UPRIGHT;
                //    break;
                //case ESCAPE_DIRECTION_RIGHT|ESCAPE_DIRECTION_BOTTOM:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_RIGHTDOWN;
                //    break;
                //case ESCAPE_DIRECTION_BOTTOM|ESCAPE_DIRECTION_LEFT:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_DOWNLEFT;
                //    break;
                //case ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_RIGHT:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_UPWARD;
                //    break;
                //case ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_RIGHT|ESCAPE_DIRECTION_DOWN:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_RIGHTWARD;
                //    break;
                //case ESCAPE_DIRECTION_RIGHT|ESCAPE_DIRECTION_DOWN|ESCAPE_DIRECTION_LEFT:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_DOWNWARD;
                //    break;
                //case ESCAPE_DIRECTION_DOWN|ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_UP:
                //    aRetval.Escape = com::sun::star::drawing::EscapeDirection_LEFTWARD;
                //    break;
                default:
                case ESCAPE_DIRECTION_SMART:
                    aRetval.Escape = com::sun::star::drawing::EscapeDirection_SMART;
                    break;
            }

            return aRetval;
        }

        GluePoint::GluePoint(
            const com::sun::star::drawing::GluePoint2& rGluePoint2,
            const basegfx::B2DVector& rAbsoluteScale)
        :   maUnitPosition(0.5, 0.5),
            meEscapeDirections(ESCAPE_DIRECTION_SMART),
            meHorizontalAlignment(Alignment_Center),
            meVerticalAlignment(Alignment_Center),
            maID(0),
            mbRelative(rGluePoint2.IsRelative),            // copy relative
            mbUserDefined(rGluePoint2.IsUserDefined)       // copy UserDefined
        {
            // copy alignment
            switch( rGluePoint2.PositionAlignment )
            {
                case com::sun::star::drawing::Alignment_TOP_LEFT:
                    meHorizontalAlignment = Alignment_Minimum;
                    meVerticalAlignment = Alignment_Minimum;
                    break;
                case com::sun::star::drawing::Alignment_TOP:
                    meHorizontalAlignment = Alignment_Center;
                    meVerticalAlignment = Alignment_Minimum;
                    break;
                case com::sun::star::drawing::Alignment_TOP_RIGHT:
                    meHorizontalAlignment = Alignment_Maximum;
                    meVerticalAlignment = Alignment_Minimum;
                    break;
                case com::sun::star::drawing::Alignment_RIGHT:
                    meHorizontalAlignment = Alignment_Maximum;
                    meVerticalAlignment = Alignment_Center;
                    break;
                case com::sun::star::drawing::Alignment_BOTTOM_LEFT:
                    meHorizontalAlignment = Alignment_Minimum;
                    meVerticalAlignment = Alignment_Maximum;
                    break;
                case com::sun::star::drawing::Alignment_BOTTOM:
                    meHorizontalAlignment = Alignment_Center;
                    meVerticalAlignment = Alignment_Maximum;
                    break;
                case com::sun::star::drawing::Alignment_BOTTOM_RIGHT:
                    meHorizontalAlignment = Alignment_Maximum;
                    meVerticalAlignment = Alignment_Maximum;
                    break;
                case com::sun::star::drawing::Alignment_LEFT:
                    meHorizontalAlignment = Alignment_Minimum;
                    meVerticalAlignment = Alignment_Center;
                    break;
                default:
                case com::sun::star::drawing::Alignment_CENTER:
                    meHorizontalAlignment = Alignment_Center;
                    meVerticalAlignment = Alignment_Center;
                    break;
            }

            // copy position (after alignment is computed)
            double fX(rGluePoint2.Position.X);
            double fY(rGluePoint2.Position.Y);

            if(mbRelative)
            {
                fX /= 10000.0;
                fY /= 10000.0;
            }
            else
            {
                // avoid values below 1.0 to not lose relative values outside object scale
                fX /= std::max(1.0, rAbsoluteScale.getX());
                fY /= std::max(1.0, rAbsoluteScale.getY());
            }

            switch(meHorizontalAlignment)
            {
                case Alignment_Minimum: break;
                case Alignment_Center: fX = fX + 0.5; break;
                case Alignment_Maximum: fX = fX + 1.0; break;
            }

            switch(meVerticalAlignment)
            {
                case Alignment_Minimum: break;
                case Alignment_Center: fY = fY + 0.5; break;
                case Alignment_Maximum: fY = fY + 1.0; break;
            }

            maUnitPosition = basegfx::B2DPoint(fX, fY);

            // copy escape directions
            switch( rGluePoint2.Escape )
            {
                case com::sun::star::drawing::EscapeDirection_LEFT:
                    meEscapeDirections = ESCAPE_DIRECTION_LEFT;
                    break;
                case com::sun::star::drawing::EscapeDirection_RIGHT:
                    meEscapeDirections = ESCAPE_DIRECTION_RIGHT;
                    break;
                case com::sun::star::drawing::EscapeDirection_UP:
                    meEscapeDirections = ESCAPE_DIRECTION_TOP;
                    break;
                case com::sun::star::drawing::EscapeDirection_DOWN:
                    meEscapeDirections = ESCAPE_DIRECTION_BOTTOM;
                    break;
                case com::sun::star::drawing::EscapeDirection_HORIZONTAL:
                    meEscapeDirections = ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_RIGHT;
                    break;
                case com::sun::star::drawing::EscapeDirection_VERTICAL:
                    meEscapeDirections = ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_BOTTOM;
                    break;

                // Unfortunately the enum EscapeDirection in the EscapeDirection.idl definition
                // is wrong in the sense that it does not reflect the possibility to define a free
                // combination of the directions left/right/up/down from which the router may choose
                // the best. Below (and in the idl file) are suggestions how this could be expanded,
                // but it would be incompatible
                //
                //case com::sun::star::drawing::EscapeDirection_LEFTUP:
                //    meEscapeDirections = ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_TOP;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_UPRIGHT:
                //    meEscapeDirections = ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_RIGHT;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_RIGHTDOWN:
                //    meEscapeDirections = ESCAPE_DIRECTION_RIGHT|ESCAPE_DIRECTION_BOTTOM;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_DOWNLEFT:
                //    meEscapeDirections = ESCAPE_DIRECTION_BOTTOM|ESCAPE_DIRECTION_LEFT;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_UPWARD:
                //    meEscapeDirections = ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_RIGHT;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_RIGHTWARD:
                //    meEscapeDirections = ESCAPE_DIRECTION_TOP|ESCAPE_DIRECTION_RIGHT|ESCAPE_DIRECTION_BOTTOM;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_DOWNWARD:
                //    meEscapeDirections = ESCAPE_DIRECTION_RIGHT|ESCAPE_DIRECTION_BOTTOM|ESCAPE_DIRECTION_LEFT;
                //    break;
                //case com::sun::star::drawing::EscapeDirection_LEFTWARD:
                //    meEscapeDirections = ESCAPE_DIRECTION_BOTTOM|ESCAPE_DIRECTION_LEFT|ESCAPE_DIRECTION_TOP;
                //    break;
                case com::sun::star::drawing::EscapeDirection_SMART:
                default:
                    meEscapeDirections = ESCAPE_DIRECTION_SMART;
                    break;
            }
        }
    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        GluePointProvider::GluePointProvider()
        {
        }

        GluePointProvider::~GluePointProvider()
        {
        }

        GluePointProvider::GluePointProvider(const GluePointProvider& /*rCandidate*/)
        {
        }

        GluePointProvider& GluePointProvider::operator=(const GluePointProvider& /*rCandidate*/)
        {
            return *this;
        }

        GluePointProvider* GluePointProvider::Clone() const
        {
            return new GluePointProvider(*this);
        }

        sal_uInt32 GluePointProvider::getAutoGluePointCount() const
        {
            return 4;
        }

        GluePoint GluePointProvider::getAutoGluePointByIndex(sal_uInt32 nIndex) const
        {
            // no error with indices, just repeatedly return last GluePoint as fallback
            basegfx::B2DPoint aGluePosition(0.5, 0.5);

            switch(nIndex)
            {
                case 0: // TopCenter
                {
                    aGluePosition.setY(0.0);
                    break;
                }
                case 1: // RightCenter
                {
                    aGluePosition.setX(1.0);
                    break;
                }
                case 2: // BottomCenter
                {
                    aGluePosition.setY(1.0);
                    break;
                }
                default: // case 3: // LeftCenter
                {
                    aGluePosition.setX(0.0);
                    break;
                }
            }

            // create GluePoint, need to set UserDefined to false for these default GluePoints
            return GluePoint(
                aGluePosition,
                GluePoint::ESCAPE_DIRECTION_SMART,
                GluePoint::Alignment_Center,
                GluePoint::Alignment_Center,
                true,   // mbRelative
                false); // mbUserDefined
        }

        bool GluePointProvider::allowsUserGluePoints() const
        {
            return false;
        }

        GluePoint& GluePointProvider::addUserGluePoint(GluePoint& rNew)
        {
            OSL_ENSURE(false, "Default GluePointProvider does not support UserGluePoints (!)");
            return rNew;
        }

        void GluePointProvider::removeUserGluePoint(const GluePoint& /*rNew*/)
        {
            OSL_ENSURE(false, "Default GluePointProvider does not support UserGluePoints (!)");
        }

        bool GluePointProvider::hasUserGluePoints() const
        {
            return false;
        }

        GluePoint* GluePointProvider::findUserGluePointByID(sal_uInt32 /*nID*/) const
        {
            OSL_ENSURE(false, "Default GluePointProvider does not support UserGluePoints (!)");
            return 0;
        }

        const GluePointVector GluePointProvider::getUserGluePointVector() const
        {
            // return empty vector; no need to assert, this may by default be used to check
            // sizes; it is better to use hasUserGluePoints first, but not required
            return GluePointVector();
        }

        void GluePointProvider::adaptUserGluePointsToChangedScale(const basegfx::B2DVector& /*rOldScale*/, const basegfx::B2DVector& /*rNewScale*/)
        {
            OSL_ENSURE(false, "Default GluePointProvider does not support UserGluePoints (!)");
        }

    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        bool GluePointComparator::operator()(const GluePoint& rA, const GluePoint& rB) const
        {
            // sort by ID
            return rA.getID() < rB.getID();
        }

        StandardGluePointProvider::StandardGluePointProvider()
        :   GluePointProvider(),
            maGluePointSet()
        {
        }

        StandardGluePointProvider::~StandardGluePointProvider()
        {
        }

        StandardGluePointProvider::StandardGluePointProvider(const StandardGluePointProvider& rCandidate)
        :   GluePointProvider(),
            maGluePointSet(rCandidate.maGluePointSet)
        {
        }

        GluePointProvider& StandardGluePointProvider::operator=(const GluePointProvider& rCandidate)
        {
            // call parent to copy UserGluePoints
            GluePointProvider::operator=(rCandidate);

            const StandardGluePointProvider* pSource = dynamic_cast< const StandardGluePointProvider* >(&rCandidate);

            if(pSource)
            {
                maGluePointSet = pSource->maGluePointSet;
            }

            return *this;
        }

        GluePointProvider* StandardGluePointProvider::Clone() const
        {
            return new StandardGluePointProvider(*this);
        }

        bool StandardGluePointProvider::allowsUserGluePoints() const
        {
            return true;
        }

        GluePoint& StandardGluePointProvider::addUserGluePoint(GluePoint& rNew)
        {
            OSL_ENSURE(0 == rNew.getID(), "Someone is adding a new GluePoint which already has an ID");

            if(!maGluePointSet.size())
            {
                // first GluePoint ever, add at start position with ID null
                setIdAtGluePoint(rNew, 0);
                maGluePointSet.insert(rNew);
                return *maGluePointSet.begin();
            }

            // get last GluePoint; take it's ID as current maximum ID (since it's a
            // set sorted by ID) as base for the new ID
            GluePointSet::iterator aFound(maGluePointSet.end());
            aFound--;

            // set ID and insert
            setIdAtGluePoint(rNew, aFound->getID() + 1);
            maGluePointSet.insert(rNew);

            // get from end (no find needed, we know we just added it with lastID + 1)
            aFound = maGluePointSet.end();
            aFound--;

            return *aFound;
        }

        void StandardGluePointProvider::removeUserGluePoint(const GluePoint& rNew)
        {
            const GluePointSet::const_iterator aFound(maGluePointSet.find(rNew));

            if(aFound == maGluePointSet.end())
            {
                // not a member
                OSL_ENSURE(false, "GluePoint for removal not found (!)");
                return;
            }

            maGluePointSet.erase(aFound);
        }

        bool StandardGluePointProvider::hasUserGluePoints() const
        {
            return !maGluePointSet.empty();
        }

        GluePoint* StandardGluePointProvider::findUserGluePointByID(sal_uInt32 nID) const
        {
            // prepare GluePoint to search for
            GluePoint aGluePoint;
            const_cast< StandardGluePointProvider* >(this)->setIdAtGluePoint(aGluePoint, nID);

            const GluePointSet::const_iterator aFound(maGluePointSet.find(aGluePoint));

            if(aFound == maGluePointSet.end())
            {
                return 0;
            }

            const GluePoint& rResult = *aFound;

            return &const_cast< GluePoint& >(rResult);
        }

        const GluePointVector StandardGluePointProvider::getUserGluePointVector() const
        {
            // prepare retval; we know the number of entries
            GluePointVector aRetval;
            aRetval.reserve(maGluePointSet.size());

            // need to loop here; the vector contains pointers to real instances, not the
            // instances themselves
            for(GluePointSet::const_iterator aFound(maGluePointSet.begin()); aFound != maGluePointSet.end(); aFound++)
            {
                aRetval.push_back(&const_cast< GluePoint& >(*aFound));
            }

            return aRetval;
        }

        void StandardGluePointProvider::adaptUserGluePointsToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale)
        {
            // if scale changed, adapt all registered GluePoints
            if(!rOldScale.equal(rNewScale))
            {
                for(GluePointSet::iterator aFound(maGluePointSet.begin()); aFound != maGluePointSet.end(); aFound++)
                {
                    adaptGluePointToChangedScale(*aFound, rOldScale, rNewScale);
                }
            }
        }

    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
