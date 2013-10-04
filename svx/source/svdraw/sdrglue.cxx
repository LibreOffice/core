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

#include <svx/sdrglue.hxx>
#include <basegfx/range/b2drange.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace glue
    {
        Point::Point(
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

        basegfx::B2DPoint Point::getUnitPosition() const
        {
            return basegfx::B2DRange::getUnitB2DRange().clamp(maUnitPosition);
        }

        void Point::setUnitPosition(const basegfx::B2DPoint& rNew)
        {
            const basegfx::B2DPoint aClampedNew(basegfx::B2DRange::getUnitB2DRange().clamp(rNew));

            if(aClampedNew != maUnitPosition)
            {
                maUnitPosition = aClampedNew;
            }
        }

        void Point::setRelative(bool bNew)
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

        void Point::adaptToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale)
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
                            case Point::Alignment_Minimum:
                            {
                                // anchored left
                                aChangedPos.setX((aChangedPos.getX() * fCorrectedOldX) / fCorrectedNewX);
                                break;
                            }
                            case Point::Alignment_Center:
                            {
                                aChangedPos.setX(((0.5 * fCorrectedNewX) + (fCorrectedOldX * (aChangedPos.getX() - 0.5))) / fCorrectedNewX);
                                break;
                            }
                            default: // case Point::Alignment_Maximum:
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
                            case Point::Alignment_Minimum:
                            {
                                // anchored left
                                aChangedPos.setY((aChangedPos.getY() * fCorrectedOldY) / fCorrectedNewY);
                                break;
                            }
                            case Point::Alignment_Center:
                            {
                                aChangedPos.setY(((0.5 * fCorrectedNewY) + (fCorrectedOldY * (aChangedPos.getY() - 0.5))) / fCorrectedNewY);
                                break;
                            }
                            default: // case Point::Alignment_Maximum:
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

        com::sun::star::drawing::GluePoint2 Point::convertToGluePoint2(const basegfx::B2DVector& rAbsoluteScale) const
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

        Point::Point(
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
        bool PointComparator::operator()(const Point& rA, const Point& rB) const
        {
            // sort by ID
            return rA.getID() < rB.getID();
        }

        Point& List::add(const Point& rNew)
        {
            OSL_ENSURE(0 == rNew.getID(), "Someone is adding a new GluePoint which already has an ID");

            if(!maPointSet.size())
            {
                const_cast< Point& >(rNew).setID(0);
                maPointSet.insert(rNew);
                return *maPointSet.begin();
            }

            PointSet::iterator aFound(maPointSet.end());
            aFound--;
            const_cast< Point& >(rNew).setID(aFound->getID() + 1);
            maPointSet.insert(rNew);
            aFound = maPointSet.end();
            aFound--;

            return *aFound;
        }

        void List::remove(const Point& rNew)
        {
            const PointSet::const_iterator aFound(maPointSet.find(rNew));

            if(aFound == maPointSet.end())
            {
                // not a member
                OSL_ENSURE(false, "GluePoint for removal not found (!)");
                return;
            }

            maPointSet.erase(aFound);
        }

        Point* List::findByID(sal_uInt32 nID) const
        {
            Point aPoint;

            aPoint.setID(nID);

            const PointSet::const_iterator aFound(maPointSet.find(aPoint));

            if(aFound == maPointSet.end())
            {
                return 0;
            }

            const Point& rResult = *aFound;

            return &const_cast< Point& >(rResult);
        }

        PointVector List::getVector() const
        {
            PointVector aRetval;

            aRetval.reserve(maPointSet.size());

            for(PointSet::const_iterator aFound(maPointSet.begin()); aFound != maPointSet.end(); aFound++)
            {
                aRetval.push_back(&const_cast< Point& >(*aFound));
            }

            return aRetval;
        }

        void List::adaptToChangedScale(const basegfx::B2DVector& rOldScale, const basegfx::B2DVector& rNewScale)
        {
            if(!rOldScale.equal(rNewScale))
            {
                for(PointSet::iterator aFound(maPointSet.begin()); aFound != maPointSet.end(); aFound++)
                {
                    aFound->adaptToChangedScale(rOldScale, rNewScale);
                }
            }
        }

    } // end of namespace glue
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
