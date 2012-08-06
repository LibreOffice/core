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

#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineStartEndAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            double                                  mfWidth;                // absolute line StartEndGeometry base width
            basegfx::B2DPolyPolygon                 maPolyPolygon;          // the StartEndGeometry PolyPolygon

            // bitfield
            unsigned                                mbCentered : 1;         // use centered to ineStart/End point?

            ImpLineStartEndAttribute(
                double fWidth,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                bool bCentered)
            :   mnRefCount(0),
                mfWidth(fWidth),
                maPolyPolygon(rPolyPolygon),
                mbCentered(bCentered)
            {
            }

            // data read access
            double getWidth() const { return mfWidth; }
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            bool isCentered() const { return mbCentered; }

            bool operator==(const ImpLineStartEndAttribute& rCandidate) const
            {
                return (basegfx::fTools::equal(getWidth(), rCandidate.getWidth())
                    && getB2DPolyPolygon() == rCandidate.getB2DPolyPolygon()
                    && isCentered() == rCandidate.isCentered());
            }

            static ImpLineStartEndAttribute* get_global_default()
            {
                static ImpLineStartEndAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpLineStartEndAttribute(
                        0.0,
                        basegfx::B2DPolyPolygon(),
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        LineStartEndAttribute::LineStartEndAttribute(
            double fWidth,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            bool bCentered)
        :   mpLineStartEndAttribute(new ImpLineStartEndAttribute(
                fWidth, rPolyPolygon, bCentered))
        {
        }

        LineStartEndAttribute::LineStartEndAttribute()
        :   mpLineStartEndAttribute(ImpLineStartEndAttribute::get_global_default())
        {
            mpLineStartEndAttribute->mnRefCount++;
        }

        LineStartEndAttribute::LineStartEndAttribute(const LineStartEndAttribute& rCandidate)
        :   mpLineStartEndAttribute(rCandidate.mpLineStartEndAttribute)
        {
            mpLineStartEndAttribute->mnRefCount++;
        }

        LineStartEndAttribute::~LineStartEndAttribute()
        {
            if(mpLineStartEndAttribute->mnRefCount)
            {
                mpLineStartEndAttribute->mnRefCount--;
            }
            else
            {
                delete mpLineStartEndAttribute;
            }
        }

        bool LineStartEndAttribute::isDefault() const
        {
            return mpLineStartEndAttribute == ImpLineStartEndAttribute::get_global_default();
        }

        LineStartEndAttribute& LineStartEndAttribute::operator=(const LineStartEndAttribute& rCandidate)
        {
            if(rCandidate.mpLineStartEndAttribute != mpLineStartEndAttribute)
            {
                if(mpLineStartEndAttribute->mnRefCount)
                {
                    mpLineStartEndAttribute->mnRefCount--;
                }
                else
                {
                    delete mpLineStartEndAttribute;
                }

                mpLineStartEndAttribute = rCandidate.mpLineStartEndAttribute;
                mpLineStartEndAttribute->mnRefCount++;
            }

            return *this;
        }

        bool LineStartEndAttribute::operator==(const LineStartEndAttribute& rCandidate) const
        {
            if(rCandidate.mpLineStartEndAttribute == mpLineStartEndAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpLineStartEndAttribute == *mpLineStartEndAttribute);
        }

        double LineStartEndAttribute::getWidth() const
        {
            return mpLineStartEndAttribute->getWidth();
        }

        const basegfx::B2DPolyPolygon& LineStartEndAttribute::getB2DPolyPolygon() const
        {
            return mpLineStartEndAttribute->getB2DPolyPolygon();
        }

        bool LineStartEndAttribute::isCentered() const
        {
            return mpLineStartEndAttribute->isCentered();
        }

        bool LineStartEndAttribute::isActive() const
        {
            return (0.0 != getWidth()
                && 0 != getB2DPolyPolygon().count()
                && 0 != getB2DPolyPolygon().getB2DPolygon(0).count());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
