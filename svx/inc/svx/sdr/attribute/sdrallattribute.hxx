/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrallattribute.hxx,v $
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

#ifndef _SDR_ATTRIBUTE_SDRALLATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRALLATTRIBUTE_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
    class SdrFillAttribute;
    class SdrLineStartEndAttribute;
    class SdrShadowAttribute;
    class FillGradientAttribute;
    class SdrTextAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrShadowTextAttribute
        {
            // shadow and text attributes
            SdrShadowAttribute*                     mpShadow;                   // shadow attributes (if used)
            SdrTextAttribute*                       mpTextAttribute;            // text and text attributes (if used)

        public:
            SdrShadowTextAttribute(
                SdrShadowAttribute* pShadow = 0,
                SdrTextAttribute* pTextAttribute = 0);
            ~SdrShadowTextAttribute();

            // copy constructor and assigment operator
            SdrShadowTextAttribute(const SdrShadowTextAttribute& rCandidate);
            SdrShadowTextAttribute& operator=(const SdrShadowTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrShadowTextAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (0 != mpTextAttribute); }

            // data access
            const SdrShadowAttribute* getShadow() const { return mpShadow; }
            const SdrTextAttribute* getText() const { return mpTextAttribute; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFillTextAttribute
        {
            // shadow and text attributes
            SdrFillAttribute*                       mpFill;                     // fill attributes (if used)
            FillGradientAttribute*                  mpFillFloatTransGradient;   // fill float transparence gradient (if used)
            SdrTextAttribute*                       mpTextAttribute;            // text and text attributes (if used)

        public:
            SdrFillTextAttribute(
                SdrFillAttribute* pFill = 0,
                FillGradientAttribute* pFillFloatTransGradient = 0,
                SdrTextAttribute* pTextAttribute = 0);
            ~SdrFillTextAttribute();

            // copy constructor and assigment operator
            SdrFillTextAttribute(const SdrFillTextAttribute& rCandidate);
            SdrFillTextAttribute& operator=(const SdrFillTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrFillTextAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return ((mpTextAttribute) || (mpFill)); }

            // data access
            const SdrFillAttribute* getFill() const { return mpFill; }
            const FillGradientAttribute* getFillFloatTransGradient() const { return mpFillFloatTransGradient; }
            const SdrTextAttribute* getText() const { return mpTextAttribute; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineShadowTextAttribute : public SdrShadowTextAttribute
        {
            // line, shadow, lineStartEnd and text attributes
            SdrLineAttribute*                       mpLine;                     // line attributes (if used)
            SdrLineStartEndAttribute*               mpLineStartEnd;             // line start end (if used)

        public:
            SdrLineShadowTextAttribute(
                SdrLineAttribute* pLine = 0,
                SdrLineStartEndAttribute* pLineStartEnd = 0,
                SdrShadowAttribute* pShadow = 0,
                SdrTextAttribute* pTextAttribute = 0);
            ~SdrLineShadowTextAttribute();

            // copy constructor and assigment operator
            SdrLineShadowTextAttribute(const SdrLineShadowTextAttribute& rCandidate);
            SdrLineShadowTextAttribute& operator=(const SdrLineShadowTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrLineShadowTextAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (mpLine || (SdrShadowTextAttribute::isVisible())); }

            // data access
            const SdrLineAttribute* getLine() const { return mpLine; }
            const SdrLineStartEndAttribute* getLineStartEnd() const { return mpLineStartEnd; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineFillShadowTextAttribute : public SdrLineShadowTextAttribute
        {
            // add fill and transGradient attributes
            SdrFillAttribute*                       mpFill;                     // fill attributes (if used)
            FillGradientAttribute*                  mpFillFloatTransGradient;   // fill float transparence gradient (if used)

        public:
            SdrLineFillShadowTextAttribute(
                SdrLineAttribute* pLine = 0,
                SdrFillAttribute* pFill = 0,
                SdrLineStartEndAttribute* pLineStartEnd = 0,
                SdrShadowAttribute* pShadow = 0,
                FillGradientAttribute* pFillFloatTransGradient = 0,
                SdrTextAttribute* pTextAttribute = 0);
            ~SdrLineFillShadowTextAttribute();

            // copy constructor and assigment operator
            SdrLineFillShadowTextAttribute(const SdrLineFillShadowTextAttribute& rCandidate);
            SdrLineFillShadowTextAttribute& operator=(const SdrLineFillShadowTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrLineFillShadowTextAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (mpFill || (SdrLineShadowTextAttribute::isVisible())); }

            // data access
            const SdrFillAttribute* getFill() const { return mpFill; }
            const FillGradientAttribute* getFillFloatTransGradient() const { return mpFillFloatTransGradient; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRALLATTRIBUTE_HXX

// eof
