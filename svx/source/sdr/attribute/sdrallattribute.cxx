/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrallattribute.cxx,v $
 *
 * $Revision: 1.2 $
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

#include "precompiled_svx.hxx"

#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// pointer compare define
#define pointerOrContentEqual(p, q) ((p == q) || (p && q && *p == *q))

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrShadowTextAttribute::SdrShadowTextAttribute(
            SdrShadowAttribute* pShadow,
            SdrTextAttribute* pTextAttribute)
        :   mpShadow(pShadow),
            mpTextAttribute(pTextAttribute)
        {
        }

        SdrShadowTextAttribute::SdrShadowTextAttribute(
            const SdrShadowTextAttribute& rCandidate)
        :   mpShadow(0),
            mpTextAttribute(0)
        {
            *this = rCandidate;
        }

        SdrShadowTextAttribute::~SdrShadowTextAttribute()
        {
            delete mpShadow;
            delete mpTextAttribute;
        }

        SdrShadowTextAttribute& SdrShadowTextAttribute::operator=(const SdrShadowTextAttribute& rCandidate)
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

            // handle mpTextAttribute
            {
                // delete local mpTextAttribute if necessary
                if(mpTextAttribute)
                {
                    delete mpTextAttribute;
                    mpTextAttribute = 0;
                }

                // copy mpTextAttribute if necessary
                if(rCandidate.mpTextAttribute)
                {
                    mpTextAttribute = new SdrTextAttribute(*rCandidate.mpTextAttribute);
                }
            }

            return *this;
        }

        bool SdrShadowTextAttribute::operator==(const SdrShadowTextAttribute& rCandidate) const
        {
            // handle mpShadow
            if(!pointerOrContentEqual(mpShadow, rCandidate.mpShadow))
                return false;

            // handle mpTextAttribute
            if(!pointerOrContentEqual(mpTextAttribute, rCandidate.mpTextAttribute))
                return false;

            return true;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrFillTextAttribute::SdrFillTextAttribute(
            SdrFillAttribute* pFill,
            FillGradientAttribute* pFillFloatTransGradient,
            SdrTextAttribute* pTextAttribute)
        :   mpFill(pFill),
            mpFillFloatTransGradient(pFillFloatTransGradient),
            mpTextAttribute(pTextAttribute)
        {
        }

        SdrFillTextAttribute::SdrFillTextAttribute(
            const SdrFillTextAttribute& rCandidate)
        :   mpFill(0),
            mpFillFloatTransGradient(0),
            mpTextAttribute(0)
        {
            *this = rCandidate;
        }

        SdrFillTextAttribute::~SdrFillTextAttribute()
        {
            delete mpFill;
            delete mpFillFloatTransGradient;
            delete mpTextAttribute;
        }

        SdrFillTextAttribute& SdrFillTextAttribute::operator=(const SdrFillTextAttribute& rCandidate)
        {
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
                    mpFill = new attribute::SdrFillAttribute(*rCandidate.mpFill);
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

            // handle mpTextAttribute
            {
                // delete local mpTextAttribute if necessary
                if(mpTextAttribute)
                {
                    delete mpTextAttribute;
                    mpTextAttribute = 0;
                }

                // copy mpTextAttribute if necessary
                if(rCandidate.mpTextAttribute)
                {
                    mpTextAttribute = new SdrTextAttribute(*rCandidate.mpTextAttribute);
                }
            }

            return *this;
        }

        bool SdrFillTextAttribute::operator==(const SdrFillTextAttribute& rCandidate) const
        {
            // handle mpFill
            if(!pointerOrContentEqual(mpFill, rCandidate.mpFill))
                return false;

            // handle mpFillFloatTransGradient
            if(!pointerOrContentEqual(mpFillFloatTransGradient, rCandidate.mpFillFloatTransGradient))
                return false;

            // handle mpTextAttribute
            if(!pointerOrContentEqual(mpTextAttribute, rCandidate.mpTextAttribute))
                return false;

            return true;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineShadowTextAttribute::SdrLineShadowTextAttribute(
            SdrLineAttribute* pLine,
            SdrLineStartEndAttribute* pLineStartEnd,
            SdrShadowAttribute* pShadow,
            SdrTextAttribute* pTextAttribute)
        :   SdrShadowTextAttribute(pShadow, pTextAttribute),
            mpLine(pLine),
            mpLineStartEnd(pLineStartEnd)
        {
        }

        SdrLineShadowTextAttribute::SdrLineShadowTextAttribute(
            const SdrLineShadowTextAttribute& rCandidate)
        :   SdrShadowTextAttribute(0, 0),
            mpLine(0),
            mpLineStartEnd(0)
        {
            *this = rCandidate;
        }

        SdrLineShadowTextAttribute::~SdrLineShadowTextAttribute()
        {
            delete mpLine;
            delete mpLineStartEnd;
        }

        SdrLineShadowTextAttribute& SdrLineShadowTextAttribute::operator=(const SdrLineShadowTextAttribute& rCandidate)
        {
            // call parent
            SdrShadowTextAttribute::operator=(rCandidate);

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

            return *this;
        }

        bool SdrLineShadowTextAttribute::operator==(const SdrLineShadowTextAttribute& rCandidate) const
        {
            // call parent
            if(!(SdrShadowTextAttribute::operator==(rCandidate)))
                return false;

            // handle mpLine
            if(!pointerOrContentEqual(mpLine, rCandidate.mpLine))
                return false;

            // handle mpLineStartEnd
            if(!pointerOrContentEqual(mpLineStartEnd, rCandidate.mpLineStartEnd))
                return false;

            return true;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute(
            SdrLineAttribute* pLine,
            attribute::SdrFillAttribute* pFill,
            SdrLineStartEndAttribute* pLineStartEnd,
            SdrShadowAttribute* pShadow,
            FillGradientAttribute* pFillFloatTransGradient,
            SdrTextAttribute* pTextAttribute)
        :   SdrLineShadowTextAttribute(pLine, pLineStartEnd, pShadow, pTextAttribute),
            mpFill(pFill),
            mpFillFloatTransGradient(pFillFloatTransGradient)
        {
        }

        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute(
            const SdrLineFillShadowTextAttribute& rCandidate)
        :   SdrLineShadowTextAttribute(0, 0, 0, 0),
            mpFill(0),
            mpFillFloatTransGradient(0)
        {
            *this = rCandidate;
        }

        SdrLineFillShadowTextAttribute::~SdrLineFillShadowTextAttribute()
        {
            delete mpFill;
            delete mpFillFloatTransGradient;
        }

        SdrLineFillShadowTextAttribute& SdrLineFillShadowTextAttribute::operator=(const SdrLineFillShadowTextAttribute& rCandidate)
        {
            // call parent
            SdrLineShadowTextAttribute::operator=(rCandidate);

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
                    mpFill = new attribute::SdrFillAttribute(*rCandidate.mpFill);
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

        bool SdrLineFillShadowTextAttribute::operator==(const SdrLineFillShadowTextAttribute& rCandidate) const
        {
            // call parent
            if(!(SdrLineShadowTextAttribute::operator==(rCandidate)))
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
