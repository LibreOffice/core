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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
    class SdrFillAttribute;
    class SdrLineStartEndAttribute;
    class SdrShadowAttribute;
    class FillGradientAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineFillShadowAttribute
        {
            // shadow, line, lineStartEnd, fill, transGradient and compound3D attributes
            SdrShadowAttribute*                 mpShadow;                   // shadow attributes (if used)
            SdrLineAttribute*                   mpLine;                     // line attributes (if used)
            SdrLineStartEndAttribute*           mpLineStartEnd;             // line start end (if used)
            SdrFillAttribute*                   mpFill;                     // fill attributes (if used)
            FillGradientAttribute*              mpFillFloatTransGradient;   // fill float transparence gradient (if used)

        public:
            SdrLineFillShadowAttribute(
                SdrLineAttribute* pLine = 0,
                SdrFillAttribute* pFill = 0,
                SdrLineStartEndAttribute* pLineStartEnd = 0,
                SdrShadowAttribute* pShadow = 0,
                FillGradientAttribute* pFillFloatTransGradient = 0);
            ~SdrLineFillShadowAttribute();

            // copy constructor and assigment operator
            SdrLineFillShadowAttribute(const SdrLineFillShadowAttribute& rCandidate);
            SdrLineFillShadowAttribute& operator=(const SdrLineFillShadowAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrLineFillShadowAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (mpFill || mpLine); }

            // data access
            const SdrShadowAttribute* getShadow() const { return mpShadow; }
            const SdrLineAttribute* getLine() const { return mpLine; }
            const SdrLineStartEndAttribute* getLineStartEnd() const { return mpLineStartEnd; }
            const SdrFillAttribute* getFill() const { return mpFill; }
            const FillGradientAttribute* getFillFloatTransGradient() const { return mpFillFloatTransGradient; }
        };
    } // end of namespace overlay
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

// eof
