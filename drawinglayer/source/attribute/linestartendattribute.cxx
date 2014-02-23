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
#include <rtl/instance.hxx>



namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineStartEndAttribute
        {
        public:
            // data definitions
            double                                  mfWidth;                // absolute line StartEndGeometry base width
            basegfx::B2DPolyPolygon                 maPolyPolygon;          // the StartEndGeometry PolyPolygon

            // bitfield
            bool                                    mbCentered : 1;         // use centered to ineStart/End point?

            ImpLineStartEndAttribute(
                double fWidth,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                bool bCentered)
            :   mfWidth(fWidth),
                maPolyPolygon(rPolyPolygon),
                mbCentered(bCentered)
            {
            }

            ImpLineStartEndAttribute()
            :   mfWidth(0.0),
                maPolyPolygon(basegfx::B2DPolyPolygon()),
                mbCentered(false)
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
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< LineStartEndAttribute::ImplType, theGlobalDefault > {};
        }

        LineStartEndAttribute::LineStartEndAttribute(
            double fWidth,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            bool bCentered)
        :   mpLineStartEndAttribute(ImpLineStartEndAttribute(
                fWidth, rPolyPolygon, bCentered))
        {
        }

        LineStartEndAttribute::LineStartEndAttribute()
        :   mpLineStartEndAttribute(theGlobalDefault::get())
        {
        }

        LineStartEndAttribute::LineStartEndAttribute(const LineStartEndAttribute& rCandidate)
        :   mpLineStartEndAttribute(rCandidate.mpLineStartEndAttribute)
        {
        }

        LineStartEndAttribute::~LineStartEndAttribute()
        {
        }

        bool LineStartEndAttribute::isDefault() const
        {
            return mpLineStartEndAttribute.same_object(theGlobalDefault::get());
        }

        LineStartEndAttribute& LineStartEndAttribute::operator=(const LineStartEndAttribute& rCandidate)
        {
            mpLineStartEndAttribute = rCandidate.mpLineStartEndAttribute;
            return *this;
        }

        bool LineStartEndAttribute::operator==(const LineStartEndAttribute& rCandidate) const
        {
            return rCandidate.mpLineStartEndAttribute == mpLineStartEndAttribute;
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
