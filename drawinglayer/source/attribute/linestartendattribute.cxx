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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
