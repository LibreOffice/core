 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ascharanchoredobjectposition.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:53:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _ASCHARANCHOREDOBJECTPOSITION_HXX
#define _ASCHARANCHOREDOBJECTPOSITION_HXX
#ifndef _ANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif

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

                @author OD

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

                OD 28.10.2003 #110978#

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

                @author OD
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

                @author OD
            */
            virtual void CalcPosition();

            /** calculated anchored position for object position type AS_CHAR

                @author OD
            */
            Point GetAnchorPos() const;

            /** calculated relative position to base line for object position type AS_CHAR

                @author OD
            */
            SwTwips GetRelPosY() const;

            /** determined object rectangle including spacing for object position type AS_CHAR

                @author OD
            */
            SwRect GetObjBoundRectInclSpacing() const;

            /** determined line alignment relative to line height

                @author OD
            */
            sal_uInt8 GetLineAlignment() const;
    };
} // namespace objectpositioning

#endif
