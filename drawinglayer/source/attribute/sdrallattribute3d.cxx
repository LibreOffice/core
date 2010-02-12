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

#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// pointer compare define
#define pointerOrContentEqual(p, q) ((p == q) || (p && q && *p == *q))

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowAttribute::SdrLineFillShadowAttribute(
            SdrLineAttribute* pLine,
            SdrFillAttribute* pFill,
            SdrLineStartEndAttribute* pLineStartEnd,
            SdrShadowAttribute* pShadow,
            FillGradientAttribute* pFillFloatTransGradient)
        :   mpShadow(pShadow),
            mpLine(pLine),
            mpLineStartEnd(pLineStartEnd),
            mpFill(pFill),
            mpFillFloatTransGradient(pFillFloatTransGradient)
        {
        }

        SdrLineFillShadowAttribute::SdrLineFillShadowAttribute(
            const SdrLineFillShadowAttribute& rCandidate)
        :   mpShadow(0),
            mpLine(0),
            mpLineStartEnd(0),
            mpFill(0),
            mpFillFloatTransGradient(0)
        {
            *this = rCandidate;
        }

        SdrLineFillShadowAttribute::~SdrLineFillShadowAttribute()
        {
            delete mpShadow;
            delete mpLine;
            delete mpLineStartEnd;
            delete mpFill;
            delete mpFillFloatTransGradient;
        }

        SdrLineFillShadowAttribute& SdrLineFillShadowAttribute::operator=(const SdrLineFillShadowAttribute& rCandidate)
        {
            // handle mpShadow
            {
                // delete local mpShadow if necessary
                if(mpShadow)
                {
                    delete mpShadow;
                    mpShadow = 0;
                }

                // copy mpShadow if necessary
                if(rCandidate.mpShadow)
                {
                    mpShadow = new SdrShadowAttribute(*rCandidate.mpShadow);
                }
            }

            // handle mpLine
            {
                // delete local mpLine if necessary
                if(mpLine)
                {
                    delete mpLine;
                    mpLine = 0;
                }

                // copy mpLine if necessary
                if(rCandidate.mpLine)
                {
                    mpLine = new SdrLineAttribute(*rCandidate.mpLine);
                }
            }

            // handle mpLineStartEnd
            {
                // delete local mpLineStartEnd if necessary
                if(mpLineStartEnd)
                {
                    delete mpLineStartEnd;
                    mpLineStartEnd = 0;
                }

                // copy mpLineStartEnd if necessary
                if(rCandidate.mpLineStartEnd)
                {
                    mpLineStartEnd = new SdrLineStartEndAttribute(*rCandidate.mpLineStartEnd);
                }
            }

            // handle mpFill
            {
                // delete local mpFill if necessary
                if(mpFill)
                {
                    delete mpFill;
                    mpFill = 0;
                }

                // copy mpFill if necessary
                if(rCandidate.mpFill)
                {
                    mpFill = new SdrFillAttribute(*rCandidate.mpFill);
                }
            }

            // handle mpFillFloatTransGradient
            {
                // delete local mpFillFloatTransGradient if necessary
                if(mpFillFloatTransGradient)
                {
                    delete mpFillFloatTransGradient;
                    mpFillFloatTransGradient = 0;
                }

                // copy mpFillFloatTransGradient if necessary
                if(rCandidate.mpFillFloatTransGradient)
                {
                    mpFillFloatTransGradient = new FillGradientAttribute(*rCandidate.mpFillFloatTransGradient);
                }
            }

            return *this;
        }

        bool SdrLineFillShadowAttribute::operator==(const SdrLineFillShadowAttribute& rCandidate) const
        {
            // handle mpShadow
            if(!pointerOrContentEqual(mpShadow, rCandidate.mpShadow))
                return false;

            // handle mpLine
            if(!pointerOrContentEqual(mpLine, rCandidate.mpLine))
                return false;

            // handle mpLineStartEnd
            if(!pointerOrContentEqual(mpLineStartEnd, rCandidate.mpLineStartEnd))
                return false;

            // handle mpFill
            if(!pointerOrContentEqual(mpFill, rCandidate.mpFill))
                return false;

            // handle mpFillFloatTransGradient
            if(!pointerOrContentEqual(mpFillFloatTransGradient, rCandidate.mpFillFloatTransGradient))
                return false;

            return true;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
