 /*************************************************************************
 *
 *  $RCSfile: anchoredobjectposition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-02 18:17:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _ANCHOREDOBJECTPOSITION_HXX
#define _ANCHOREDOBJECTPOSITION_HXX

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _ORNTENUM_HXX
#include <orntenum.hxx>
#endif

class SdrObject;
class SwContact;
class SwFrm;
class SwTxtFrm;
class SwLayoutFrm;
class SwFlyFrm;
class SwFlyInCntFrm;
class SwFlyLayFrm;
class SwFlyAtCntFrm;
class SwFrmFmt;
class SwFmtVertOrient;
class SwFmtHoriOrient;
class SvxLRSpaceItem;
class SvxULSpaceItem;

namespace objectpositioning
{
    enum SwObjPosType { AS_CHAR, TO_CNTNT, TO_LAYOUT };

    // flags for positioning algorithm of as-character-anchored objects
    typedef sal_uInt8 AsCharFlags;
    #define AS_CHAR_NOFLAG  0
    #define AS_CHAR_QUICK   1
    #define AS_CHAR_ULSPACE 2
    #define AS_CHAR_INIT    4
    #define AS_CHAR_ROTATE  8
    #define AS_CHAR_REVERSE 16
    #define AS_CHAR_BIDI    32

    class SwAnchoredObjectPosition
    {
        private:
            // object position type
            SwObjPosType    mTypeOfObjPos;

            // object to be positioned
            SdrObject&  mrDrawObj;

            // information about object
            // does the object represents a fly frame
            bool        mbIsObjFly;
            // frame the object belongs to; NULL, if !<mbObjIsFly>
            SwFrm*      mpFrmOfObj;
            // frame the object is anchored at
            SwFrm*      mpAnchorFrm;
            // contact object
            SwContact*  mpContact;

            // calculated data for object position type AS_CHAR
            Point       maAnchorPos;
            SwTwips     mnRelPos;
            SwRect      maObjBoundRect;

            // calculated data for object position type TO_LAYOUT
            Point       maRelPos;

            // calculated data for object position type TO_CNTNT
            const SwFrm*    mpVertPosOrientFrm;

            // data for calculation of position for object position type TO_CNTNT
            bool        mbAnchorToChar;
            SwRect*     mpToCharRect;

            /** determine information about object

                OD 30.07.2003 #110978#
                member <mbIsObjFly>, <mpFrmOfObj>, <mpAnchorFrm> and
                <mpContact> are set

                @author OD
            */
            void _GetInfoAboutObj();

        // *********************************************************************
        // helper methods for object position type AS_CHAR
        // *********************************************************************
            /** determine the relative position to base line for object position type AS_CHAR

                OD 29.07.2003 #110978#
                Note about output parameter <_onLineAlignment> -
                value gives feedback for the line formatting.
                0 - no feedback; 1|2|3 - proposed formatting of characters
                at top|at center|at bottom of line.

                @author OD

                @param _ObjBoundHeight
                height including corresponding spacing of the object, for which
                the Y-position has to be calculated.

                @param _rVert
                given vertical positioning and alignment

                @param _nLineAscent
                line ascent given by the character in the line. parameter needed
                for an alignment relative to the character height

                @param _nLineDescent
                line descent given by the character in the line. parameter needed
                for an alignment relative to the character height

                @param _nLineAscentInclObjs
                line ascent given by the character and the other objects in the
                line. parameter needed for an alignment relative to the line height

                @param _nLineDescentInclObjs
                line descent given by the character and the other objects in the
                line. parameter needed for an alignment relative to the line height

                @param _onLineAlignment
                output parameter - for alignment relative to the line height.
                following values are set:
                0 - no line alignment, 1 - at top, 2 - at center, 3 - at bottom

                @return relative position to the base line
            */
            SwTwips _GetRelPosToBase( const SwTwips          _nObjBoundHeight,
                                      const SwFmtVertOrient& _rVert,
                                      const SwTwips          _nLineAscent,
                                      const SwTwips          _nLineDescent,
                                      const SwTwips          _nLineAscentInclObjs,
                                      const SwTwips          _nLineDescentInclObjs,
                                      sal_uInt8&             _onLineAlignment );

        // *********************************************************************
        // helper methods for object position type TO_LAYOUT
        // *********************************************************************
            /** calculated relative position for object position type TO_LAYOUT

                @author OD
            */
            void _CalcPositionForToLayout();

        // *********************************************************************
        // helper methods for object position type TO_CNTNT
        // *********************************************************************
            /** calculated relative position for object position type TO_CNTNT

                @author OD
            */
            void _CalcPositionForToCntnt();

        // *********************************************************************
            /** adjust calculated vertical in order to keep object inside
                'page' alignment layout frame.

                @author OD
            */
            SwTwips _AdjustVertRelPos( const SwFrm&  _rPageAlignLayFrm,
                                       const SwFrm&  _rVertOrientFrm,
                                       const SwTwips _nProposedRelPosY ) const;

        // *********************************************************************
            /** determine frame for horizontal position for object position type TO_CNTNT

                OD 04.08.2003
                if the given proposed frame is a content frame, the proposed
                frame is returned.
                otherwise (given proposed frame is a layout frame),
                the lower content frames of the proposed frame are checked and
                the nearest by vertical position is returned. if none
                lower content frame is found, the proposed frame is returned.

                @author OD

                @param _pProposedFrm
                input parameter - proposed frame for horizontal position

                @return constant reference to <SwFrm> object, at which the
                horizontal position is determined.
            */
            const SwFrm& _GetHoriVirtualAnchor( const SwFrm& _pProposedFrm ) const;

        // *********************************************************************
            /** calculate relative horizontal position for object type position TO_CNTNT

                OD 11.08.2003 #110978#

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame the horizontal position of the object
                is oriented at.

                @param _pToCharOrientFrm
                input parameter - if object is to-character anchored (mbAnchorToChar),
                this is the frame the position is oriented at.

                @param _rHoriOrient
                input parameter - horizontal positioning and alignment, for which
                the relative position is calculated.

                @param _bFollowTextFlow
                input parameter - boolean indicating, if object has to follow
                the text flow.

                @param _rLRSpacing
                input parameter - left and right spacing of the object to the text

                @param _rULSpacing
                input parameter - upper and lower spacing of the object to the text

                @param _bObjWrapThrough
                input parameter - boolean indicating, if object has wrap mode
                'wrap through'.

                @param _nRelPosY
                input parameter - relative vertical position

                @return relative horizontal position in SwTwips
            */
            SwTwips _CalcRelPosX( const SwFrm& _rHoriOrientFrm,
                                  const SwFrm*  _pToCharOrientFrm,
                                  const SwFmtHoriOrient& _rHoriOrient,
                                  const bool _bFollowTextFlow,
                                  const SvxLRSpaceItem& _rLRSpacing,
                                  const SvxULSpaceItem& _rULSpacing,
                                  const bool _bObjWrapThrough,
                                  const SwTwips _nRelPosY
                                ) const;

        // *********************************************************************
            /** determine layout frame for positioning aligned at 'page' areas
                for object position type TO_CNTNT

                OD 22.09.2003
                If an object is aligned to a 'page area' - REL_PG_FRAME,
                REL_PG_PRTAREA, REL_PG_LEFT and REL_PG_RIGHT -, the layout frame,
                which determines the alignment area has to be determined.
                If an object doesn't follow the text flow,
                   this is the cell frame, if object is anchored inside a cell or
                   it's the fly frame, if object is anchored inside a fly frame;
                otherwise it's the page frame

                @author OD

                @param _rOrientFrm
                input parameter - frame, at which the vertical respectively
                horizontal positioning is oriented.
                Starting point for the search of the layout frame

                @param _b
                @return reference to the layout frame, which determines the
                'page area' the object will be positioned in.
            */
//            const SwLayoutFrm& _GetPageAlignmentLayoutFrm( const SwFrm& _rOrientFrm,
//                                                           const bool   _bFollowTextFlow ) const;

        // *********************************************************************
            /** determine alignment values for horizontal position for object
                position type TO_CNTNT and TO_LAYOUT

                OD 04.08.2003

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame the horizontal position of the object
                is oriented at.

                @param _rPageAlignLayFrm
                input paramter - layout frame, which determines the 'page area'
                the object has to be horizontal positioned in.

                @param _pToCharOrientFrm
                input parameter - if object is to-character anchored (mbAnchorToChar),
                this is the frame the position is oriented at.

                @param _eRelOrient
                input parameter - horizontal relative alignment, for which
                the relative position is calculated.

                @param _bToggleLeftRight
                input parameter - boolean indicating, if left/right alignments
                have to be toggled.

                @param _bObjWrapThrough
                input parameter - boolean indicating, if object has wrap mode
                'wrap through'.
                important note: value is only relevant, if _rHoriOrientFrm is
                                a text frame.

                @param _orAlignAreaWidth
                output parameter - width in SwTwips of the area the horizontal
                position is aligned to.

                @param _orAlignAreaOffset
                output parameter - offset in SwTwips of the area the horizontal
                position is aligned to. offset is given to the 'left' of the
                anchor position.

                @param _obAlignedRelToPage
                output parameter - boolean indicating, that object is aligned
                to 'page area'.
            */
            void _GetHoriAlignmentValues( const SwFrm&  _rHoriOrientFrm,
                                          const SwFrm&  _rPageAlignLayFrm,
                                          const SwFrm*  _pToCharOrientFrm,
                                          const SwRelationOrient _eRelOrient,
                                          const bool    _bObjWrapThrough,
                                          SwTwips&      _orAlignAreaWidth,
                                          SwTwips&      _orAlignAreaOffset,
                                          bool&         _obAlignedRelToPage ) const;

        // *********************************************************************
            /** toggle given horizontal orientation and relative alignment

                @author OD

                @param _bToggleLeftRight
                input parameter - boolean indicating, if horizontal orientation
                and relative alignment has to be toggled.

                @param _ioeHoriOrient
                input/output parameter - horizontal orientation, that is toggled,
                if needed.

                @param _iopeRelOrient
                optional input/output parameter (default value NULL)
                - if set, relative alignment, that is toggled, if needed.
            */
            void _ToggleHoriOrientAndAlign( const bool         _bToggleLeftRight,
                                            SwHoriOrient&      _ioeHoriOrient,
                                            SwRelationOrient*  _iopeRelOrient = 0L
                                          ) const;

        // *********************************************************************
            /** adjust calculated horizontal in order to keep object inside
                'page' alignment layout frame for object type position TO_CNTNT

                @author OD

                @param _rPageAlignLayFrm
                input paramter - layout frame, which determines the 'page area'
                the object has to be horizontal positioned in.

                @param _nProposedRelPosX
                input parameter - proposed relative horizontal position, which
                will be adjusted.

                @return adjusted relative horizontal position in SwTwips.
            */
            SwTwips _AdjustHoriRelPos( const SwFrm&  _rPageAlignLayFrm,
                                       const SwTwips _nProposedRelPosX ) const;

        // *********************************************************************
            /** adjust calculated horizontal position in order to draw object
                aside other objects with same positioning

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame the horizontal position of the object
                is oriented at.

                @param _nProposedRelPosX
                input parameter - proposed relative horizontal position, which
                will be adjusted.

                @param _nRelPosY
                input parameter - relative vertical position

                @param _eHoriOrient
                input parameter - horizontal position of object

                @param _eRelOrient
                inpt parameter - alignment of object

                @param _rLRSpacing
                input parameter - left and right spacing of the object to the text

                @param _rULSpacing
                input parameter - upper and lower spacing of the object to the text

                @param _bEvenPage
                input parameter - boolean indicating, if object is on an even page.

                @return adjusted relative horizontal position in SwTwips
            */
            SwTwips _AdjustHoriRelPosForDrawAside( const SwFrm&  _rHoriOrientFrm,
                                                   const SwTwips _nProposedRelPosX,
                                                   const SwTwips _nRelPosY,
                                                   const SwHoriOrient _eHoriOrient,
                                                   const SwRelationOrient _eRelOrient,
                                                   const SvxLRSpaceItem& _rLRSpacing,
                                                   const SvxULSpaceItem& _rULSpacing,
                                                   const bool _bEvenPage
                                                 ) const;

        // *********************************************************************
            /** detemine, if object has to draw aside given fly frame

                OD 11.08.2003 #110978#
                method used by <_AdjustHoriRelPosForDrawAside(..)>

                @author OD

                @param _pFly
                input parameter - fly frame the draw aside check is done for.

                @param _rObjRect
                input parameter - proposed object rectangle

                @param _pObjContext
                input parameter - context of the object

                @param _nObjIndex
                input parameter - index of the anchor frame of the object

                @param _bEvenPage
                input parameter - boolean indicating, if object is on an even page.

                @param _eHoriOrient
                input parameter - horizontal position of object

                @param _eRelOrient
                inpt parameter - alignment of object

                @return boolean indicating, if object has to be drawn aside
                given fly frame.
            */
            bool _DrawAsideFly( const SwFlyFrm* _pFly,
                                const SwRect&   _rObjRect,
                                const SwFrm*    _pObjContext,
                                const ULONG     _nObjIndex,
                                const bool      _bEvenPage,
                                const SwHoriOrient _eHoriOrient,
                                const SwRelationOrient _eRelOrient
                              ) const;

        // *********************************************************************
            /** determine, if object has to draw aside another object

                OD 04.08.2003
                the different alignments of the objects determines, if one has
                to draw aside another one. Thus, the given alignment are checked
                against each other, which one has to be drawn aside the other one.
                depending on parameter _bLeft check is done for left or right
                positioning.
                method used by <_DrawAsideFly(..)>

                @author OD

                @param _eRelOrient1
                input parameter - alignment 1

                @param _eRelOrient2
                input parameter - alignment 2

                @param _bLeft
                input parameter - boolean indicating, if check is done for left
                or for right positioning.

                @return boolean indicating, if an object with an alignment
                <_eRelOrient1> has to be drawn aside an object with an
                alignment <_eRelOrient2>
            */
            bool _Minor( SwRelationOrient _eRelOrient1,
                         SwRelationOrient _eRelOrient2,
                         bool             _bLeft ) const;

        // *********************************************************************
            void _GetVertAlignmentValues( const SwFrm& _rVertOrientFrm,
                                          const SwFrm& _rPageAlignLayFrm,
                                          const SwRelationOrient _eRelOrient,
                                          SwTwips&      _orAlignAreaHeight,
                                          SwTwips&      _orAlignAreaOffset ) const;

        // *********************************************************************
            SwTwips _GetVertRelPos( const SwFrm& _rVertOrientFrm,
                                    const SwFrm& _rPageAlignLayFrm,
                                    const SwVertOrient     _eVertOrient,
                                    const SwRelationOrient _eRelOrient,
                                    const SwTwips          _nVertPos,
                                    const SvxLRSpaceItem& _rLRSpacing,
                                    const SvxULSpaceItem& _rULSpacing ) const;

        // *********************************************************************
        public:
            SwAnchoredObjectPosition( SwObjPosType _TypeOfObjPos,
                                      SdrObject& _rDrawObj );
            ~SwAnchoredObjectPosition();

            SwObjPosType GetObjPosType() const;

            // accessors for object and its corresponding data/information
            // basic accessors
            SdrObject& GetObject() const;
            bool       IsObjFly() const;
            SwFrm*     GetFrmOfObj() const;
            SwFrm&     GetAnchorFrm() const;
            SwContact* GetContact() const;
            SwFrmFmt*  GetFrmFmt() const;
            // accessors for object position type AS_CHAR and TO_CNTNT
            SwTxtFrm&       GetAnchorTxtFrm() const;
            // accessors for object position type AS_CHAR
            SwFlyInCntFrm*  GetFlyInCntFrmOfObj() const;
            // accessors for object position type TO_LAYOUT
            SwFlyLayFrm*    GetFlyLayFrmOfObj() const;
            // accessors for object position type TO_CNTNT
            SwFlyAtCntFrm*  GetFlyAtCntFrmOfObj() const;

        // *********************************************************************
        // methods for object position type AS_CHAR
        // *********************************************************************
            /** calculate position for object position type AS_CHAR

                OD 30.07.2003 #110978#
                members <maAnchorPos>, <mnRelPos> and <maObjBoundRect> are calculated.
                calculated position is set at the given object.

                @param _rAnchorFrm
                paragraph frame the given object is anchored at.

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

                @param _onLineAlignment
                output parameter - for alignment relative to the line height.
                gives feedback for the line formatting.

                @author OD
            */
            void CalcPosition( const Point&      _rProposedAnchorPos,
                               const AsCharFlags _nFlags,
                               const SwTwips     _nLineAscent,
                               const SwTwips     _nLineDescent,
                               const SwTwips     _nLineAscentInclObjs,
                               const SwTwips     _nLineDescentInclObjs,
                               sal_uInt8&        _onLineAlignment );

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

        // *********************************************************************
        // methods for object position type TO_LAYOUT and TO_CNTNT
        // *********************************************************************
            /** calculate position for object position type TO_LAYOUT and TO_CNTNT

                OD 30.07.2003 #110978#

                @author OD
            */
            void CalcPosition();

            /** calculated relative position for object position type TO_LAYOUT

                @author OD
            */
            Point GetRelPos() const;

            /** frame vertical position is oriented at for object position type TO_CNTNT

                OD 06.10.2003 #110978#

                @author OD
            */
            const SwFrm& GetVertPosOrientFrm() const;

        // *********************************************************************
            /** determine environment layout frame for possible vertical object
                positions respectively for alignments to 'page areas'
                for object position type TO_CNTNT

                this is, if object has to follow the text flow:
                - cell frame, if anchored inside a cell
                - fly frame, if anchored inside a fly frame
                - header/footer frame, if anchored inside page header/footer
                - footnote frame, if anchored inside footnote
                otherwise it's the document body frame

                this is, if object hasn't to follow the text flow:
                - page frame.
                - Exception: If environment layout frame is used for page alignment,
                  it's the cell frame, if anchored inside a cell.

                @author OD

                @param _rVertOrientFrm
                input parameter - frame, at which the vertical position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @param _bFollowTextFlow
                input parameter - indicates, if object has to follow the text
                flow or not.

                @param _bForPageAlignment
                input parameter - indicates, if the environment layout frame
                for the page alignments has to be determined or not.

                @return reference to the layout frame, which determines the
                the vertical environment the object has to be positioned in.
            */
            static const SwLayoutFrm& GetVertEnvironmentLayoutFrm( const SwFrm& _rVertOrientFrm,
                                                                   const bool _bFollowTextFlow,
                                                                   const bool _bForPageAlignment );

        // *********************************************************************
            /** determine environment layout frame for possible horizontal object
                positions respectively for alignment to 'page areas'
                for object position type TO_CNTNT

                OD 08.09.2003
                this is, if object has to follow the text flow:
                - cell frame, if anchored inside a cell
                - fly frame, if anchored inside a fly frame
                otherwise it's the page frame

                this is, if object hasn't to follow the text flow:
                - page frame.
                - Exception: If environment layout frame is used for page alignment,
                  it's the cell frame, if anchored inside a cell.

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame, at which the horizontal position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @param _bFollowTextFlow
                input parameter - indicates, if object has to follow the text
                flow or not.

                @param _bForPageAlignment
                input parameter - indicates, if the environment layout frame
                for the page alignments has to be determined or not.

                @return reference to the layout frame, which determines the
                the horizontal environment the object has to be positioned in.
            */
            static const SwLayoutFrm& GetHoriEnvironmentLayoutFrm( const SwFrm& _rHoriOrientFrm,
                                                                   const bool _bFollowTextFlow,
                                                                   const bool _bForPageAlignment );
    };
};

#endif
