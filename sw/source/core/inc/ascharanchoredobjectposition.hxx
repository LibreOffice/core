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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_ASCHARANCHOREDOBJECTPOSITION_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ASCHARANCHOREDOBJECTPOSITION_HXX

#include <anchoredobjectposition.hxx>
#include <sal/types.h>
#include <swtypes.hxx>
#include <swrect.hxx>

class SwTextFrame;
class SwFormatVertOrient;

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
        // Proposed anchor position, starting point for the calculation
        // of the object position.
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

        // method to cast <SwAnchoredObjectPosition::GetAnchorFrame()>
        const SwTextFrame& GetAnchorTextFrame() const;

        /** determine the relative position to base line for object position

            @param _ObjBoundHeight
            height including corresponding spacing of the object, for which
            the Y-position has to be calculated.

            @param _rVert
            given vertical positioning and alignment

            @return relative position to the base line
        */
        SwTwips _GetRelPosToBase( const SwTwips          _nObjBoundHeight,
                                  const SwFormatVertOrient& _rVert );

    public:
        /** constructor; provided object to be positioned and needed data
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
        virtual void CalcPosition() override;

        // calculated anchored position for object position type AS_CHAR
        const Point& GetAnchorPos() const { return maAnchorPos;}

        // calculated relative position to base line for object position type AS_CHAR
        SwTwips GetRelPosY() const { return mnRelPos;}

        // determined object rectangle including spacing for object position type AS_CHAR
        const SwRect& GetObjBoundRectInclSpacing() const { return maObjBoundRect;}

        // determined line alignment relative to line height
        sal_uInt8 GetLineAlignment() const { return mnLineAlignment;}
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
