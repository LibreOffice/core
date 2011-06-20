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
#ifndef _ASCHARANCHOREDOBJECTPOSITION_HXX
#define _ASCHARANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#include <sal/types.h>
#include <swtypes.hxx>
#include <swrect.hxx>

class SwTxtFrm;
class SwFmtVertOrient;

namespace objectpositioning
{
    // flags for positioning algorithm of as-character-anchored objects
    typedef sal_uInt8 AsCharFlags;
    #define AS_CHAR_NOFLAG  0
    #define AS_CHAR_QUICK   1
    #define AS_CHAR_ULSPACE 2
    #define AS_CHAR_INIT    4
    #define AS_CHAR_ROTATE  8
    #define AS_CHAR_REVERSE 16
    #define AS_CHAR_BIDI    32

    class SwAsCharAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
         // data to calculate object position
            // proposed anchor position, starting point for the calculation
            // of the object position
            const Point& mrProposedAnchorPos;
            // flags that influences the calculation of the anchor position
            // AS_CHAR_QUICK   : quick formatting - calculated position not set at object
            // AS_CHAR_ULSPACE : consider upper/lower spacing - adjustment of anchor position
            // AS_CHAR_INIT    : initial calculation
            // AS_CHAR_ROTATE  : object is rotated by 90 degrees
            // AS_CHAR_REVERSE : object is reversed (rotated by 270 degrees)
            // AS_CHAR_BIDI    : object belongs to a BIDI-multi-portion
            const AsCharFlags mnFlags;
            // needed line values for the different alignments.
            const SwTwips mnLineAscent;
            const SwTwips mnLineDescent;
            const SwTwips mnLineAscentInclObjs;
            const SwTwips mnLineDescentInclObjs;

         // calculated data for object position
            Point       maAnchorPos;
            SwTwips     mnRelPos;
            SwRect      maObjBoundRect;
            // line alignment relative to line height; gives feedback for line formatting
            // 0 - no line alignment, 1 - at top, 2 - at center, 3 - at bottom
            sal_uInt8   mnLineAlignment;

            // method to cast <SwAnchoredObjectPosition::GetAnchorFrm()>
            const SwTxtFrm& GetAnchorTxtFrm() const;

            /** determine the relative position to base line for object position

                @param _ObjBoundHeight
                height including corresponding spacing of the object, for which
                the Y-position has to be calculated.

                @param _rVert
                given vertical positioning and alignment

                @return relative position to the base line
            */
            SwTwips _GetRelPosToBase( const SwTwips          _nObjBoundHeight,
                                      const SwFmtVertOrient& _rVert );

        // *********************************************************************
        public:
            /** construtor; provided object to be positioned and needed data
                for calculation of the object position

                @param _rDrawObj
                input parameter - object, that is be positioned.

                @param _rProposedAnchorPos
                proposed anchor position; starting point for the calculation
                of the anchor position

                @param _nFlags
                flags that influences the calculation of the anchor position
                AS_CHAR_QUICK   : quick formatting - calculated position not set at object
                AS_CHAR_ULSPACE : consider upper/lower spacing - adjustment of anchor position
                AS_CHAR_INIT    : initial calculation
                AS_CHAR_ROTATE  : object is rotated by 90 degrees
                AS_CHAR_REVERSE : object is reversed (rotated by 270 degrees)
                AS_CHAR_BIDI    : object belongs to a BIDI-multi-portion

                @param _nLineAscent, _nLineDescent, _nLineAscentInclObjs,
                _nLineDescentInclObjs - needed line values for the different
                alignments.
            */
            SwAsCharAnchoredObjectPosition( SdrObject&      _rDrawObj,
                                            const Point&    _rProposedAnchorPos,
                                            const AsCharFlags _nFlags,
                                            const SwTwips     _nLineAscent,
                                            const SwTwips     _nLineDescent,
                                            const SwTwips     _nLineAscentInclObjs,
                                            const SwTwips     _nLineDescentInclObjs );
            virtual ~SwAsCharAnchoredObjectPosition();

            /** calculate position for object position

                members <maAnchorPos>, <mnRelPos>, <maObjBoundRect> and
                <mnLineAlignment> are calculated.
                calculated position is set at the given object.
            */
            virtual void CalcPosition();

            // calculated anchored position for object position type AS_CHAR

            Point GetAnchorPos() const;

           // calculated relative position to base line for object position type AS_CHAR

            SwTwips GetRelPosY() const;

          // determined object rectangle including spacing for object position type AS_CHAR

            SwRect GetObjBoundRectInclSpacing() const;

          // determined line alignment relative to line height

            sal_uInt8 GetLineAlignment() const;
    };
} // namespace objectpositioning

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
