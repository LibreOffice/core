/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrtextattribute.hxx,v $
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

#ifndef _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/xenum.hxx>
#include <tools/color.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SfxItemSet;
namespace drawinglayer { namespace attribute { class SdrFormTextOutlineAttribute; }}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFormTextAttribute
        {
        private:
            // FormText (FontWork) Attributes
            sal_Int32                                   mnFormTextDistance;     // distance from line in upright direction
            sal_Int32                                   mnFormTextStart;        // shift from polygon start
            sal_Int32                                   mnFormTextShdwXVal;     // shadow distance or 10th degrees
            sal_Int32                                   mnFormTextShdwYVal;     // shadow distance or scaling
            sal_uInt16                                  mnFormTextShdwTransp;   // shadow transparence
            XFormTextStyle                              meFormTextStyle;        // on/off and char orientation
            XFormTextAdjust                             meFormTextAdjust;       // adjustment (left/right/center) and scale
            XFormTextShadow                             meFormTextShadow;       // shadow mode
            Color                                       maFormTextShdwColor;    // shadow color

            // outline attributes; used when getFormTextOutline() is true and (for
            // shadow) when getFormTextShadow() != XFTSHADOW_NONE
            SdrFormTextOutlineAttribute*                mpOutline;
            SdrFormTextOutlineAttribute*                mpShadowOutline;

            // bitfield
            unsigned                                    mbFormTextMirror : 1;   // change orientation
            unsigned                                    mbFormTextOutline : 1;  // show contour of objects

        public:
            SdrFormTextAttribute(const SfxItemSet& rSet);
            ~SdrFormTextAttribute();

            // copy constructor and assigment operator
            SdrFormTextAttribute(const SdrFormTextAttribute& rCandidate);
            SdrFormTextAttribute& operator=(const SdrFormTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrFormTextAttribute& rCandidate) const;

            // data access
            sal_Int32 getFormTextDistance() const { return mnFormTextDistance; }
            sal_Int32 getFormTextStart() const { return mnFormTextStart; }
            sal_Int32 getFormTextShdwXVal() const { return mnFormTextShdwXVal; }
            sal_Int32 getFormTextShdwYVal() const { return mnFormTextShdwYVal; }
            sal_uInt16 getFormTextShdwTransp() const { return mnFormTextShdwTransp; }
            XFormTextStyle getFormTextStyle() const { return meFormTextStyle; }
            XFormTextAdjust getFormTextAdjust() const { return meFormTextAdjust; }
            XFormTextShadow getFormTextShadow() const { return meFormTextShadow; }
            Color getFormTextShdwColor() const { return maFormTextShdwColor; }

            const SdrFormTextOutlineAttribute* getOutline() const { return mpOutline; }
            const SdrFormTextOutlineAttribute* getShadowOutline() const { return mpShadowOutline; }

            bool getFormTextMirror() const { return mbFormTextMirror; }
            bool getFormTextOutline() const { return mbFormTextOutline; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX

// eof
