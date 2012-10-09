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

#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            basegfx::BColor                         maColor;                // color
            double                                  mfWidth;                // absolute line width
            basegfx::B2DLineJoin                    meLineJoin;             // type of LineJoin
            com::sun::star::drawing::LineCap        meLineCap;              // BUTT, ROUND, or SQUARE

            ImpLineAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::B2DLineJoin aB2DLineJoin,
                com::sun::star::drawing::LineCap aLineCap)
            :   mnRefCount(0),
                maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin),
                meLineCap(aLineCap)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }
            com::sun::star::drawing::LineCap getLineCap() const { return meLineCap; }

            bool operator==(const ImpLineAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getWidth() == rCandidate.getWidth()
                    && getLineJoin() == rCandidate.getLineJoin()
                    && getLineCap() == rCandidate.getLineCap());
            }

            static ImpLineAttribute* get_global_default()
            {
                static ImpLineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpLineAttribute(
                        basegfx::BColor(),
                        0.0,
                        basegfx::B2DLINEJOIN_ROUND,
                        com::sun::star::drawing::LineCap_BUTT);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        LineAttribute::LineAttribute(
            const basegfx::BColor& rColor,
            double fWidth,
            basegfx::B2DLineJoin aB2DLineJoin,
            com::sun::star::drawing::LineCap aLineCap)
        :   mpLineAttribute(
                new ImpLineAttribute(
                    rColor,
                    fWidth,
                    aB2DLineJoin,
                    aLineCap))
        {
        }

        LineAttribute::LineAttribute()
        :   mpLineAttribute(ImpLineAttribute::get_global_default())
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::LineAttribute(const LineAttribute& rCandidate)
        :   mpLineAttribute(rCandidate.mpLineAttribute)
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::~LineAttribute()
        {
            if(mpLineAttribute->mnRefCount)
            {
                mpLineAttribute->mnRefCount--;
            }
            else
            {
                delete mpLineAttribute;
            }
        }

        bool LineAttribute::isDefault() const
        {
            return mpLineAttribute == ImpLineAttribute::get_global_default();
        }

        LineAttribute& LineAttribute::operator=(const LineAttribute& rCandidate)
        {
            if(rCandidate.mpLineAttribute != mpLineAttribute)
            {
                if(mpLineAttribute->mnRefCount)
                {
                    mpLineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpLineAttribute;
                }

                mpLineAttribute = rCandidate.mpLineAttribute;
                mpLineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool LineAttribute::operator==(const LineAttribute& rCandidate) const
        {
            if(rCandidate.mpLineAttribute == mpLineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpLineAttribute == *mpLineAttribute);
        }

        const basegfx::BColor& LineAttribute::getColor() const
        {
            return mpLineAttribute->getColor();
        }

        double LineAttribute::getWidth() const
        {
            return mpLineAttribute->getWidth();
        }

        basegfx::B2DLineJoin LineAttribute::getLineJoin() const
        {
            return mpLineAttribute->getLineJoin();
        }

        com::sun::star::drawing::LineCap LineAttribute::getLineCap() const
        {
            return mpLineAttribute->getLineCap();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
