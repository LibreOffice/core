 /*************************************************************************
 *
 *  $RCSfile: anchoredobjectposition.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:46:10 $
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
#ifndef _ORNTENUM_HXX
#include <orntenum.hxx>
#endif
// OD 2004-03-16 #i11860#
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif

class SdrObject;
class SwFrm;
class SwFlyFrm;
class SwContact;
class SwFrmFmt;
class SwRect;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFmtHoriOrient;
// OD 2004-03-23 #i26701#
class SwAnchoredObject;

namespace objectpositioning
{
    class SwEnvironmentOfAnchoredObject;

    class SwAnchoredObjectPosition
    {
        private:
            // object to be positioned
            SdrObject& mrDrawObj;

         // information about object
            // does the object represents a Writer fly frame
            bool mbIsObjFly;
            // OD 2004-03-23 #i26791# - anchored object the object belongs to;
            SwAnchoredObject* mpAnchoredObj;
            // frame the object is anchored at
            SwFrm* mpAnchorFrm;
            // contact object
            SwContact* mpContact;
            // frame format
            const SwFrmFmt* mpFrmFmt;

            /** determine information about object

                OD 30.07.2003 #110978#
                member <mbIsObjFly>, <mpAnchoredObj>, <mpAnchorFrm>, <mpContact>
                and <mpFrmFmt> are set

                @author OD
            */
            void _GetInfoAboutObj();

        protected:
            SwAnchoredObjectPosition( SdrObject& _rDrawObj );
            ~SwAnchoredObjectPosition();

         // accessors for object and its corresponding data/information
            SdrObject& GetObject() const;
            bool IsObjFly() const;
            SwAnchoredObject& GetAnchoredObj() const;
            SwFrm& GetAnchorFrm() const;
            SwContact& GetContact() const;
            const SwFrmFmt& GetFrmFmt() const;

         // virtual methods providing data for to character anchored objects.
            virtual bool IsAnchoredToChar() const;
            virtual const SwFrm* ToCharOrientFrm() const;
            virtual const SwRect* ToCharRect() const;
            // OD 12.11.2003 #i22341#
            virtual SwTwips ToCharTopOfLine() const;

        // *********************************************************************
            /** helper method to determine top of a frame for the vertical
                object positioning

                OD 2004-03-11 #i11860#

                @author OD
            */
            SwTwips _GetTopForObjPos( const SwFrm& _rFrm,
                                      const SwRectFn& _fnRect,
                                      const bool _bVert ) const;

        // *********************************************************************
            void _GetVertAlignmentValues( const SwFrm& _rVertOrientFrm,
                                          const SwFrm& _rPageAlignLayFrm,
                                          const SwRelationOrient _eRelOrient,
                                          SwTwips&      _orAlignAreaHeight,
                                          SwTwips&      _orAlignAreaOffset ) const;

        // *********************************************************************
        // --> OD 2004-06-17 #i26791# - add output parameter <_roVertOffsetToFrmAnchorPos>
            SwTwips _GetVertRelPos( const SwFrm& _rVertOrientFrm,
                                    const SwFrm& _rPageAlignLayFrm,
                                    const SwVertOrient     _eVertOrient,
                                    const SwRelationOrient _eRelOrient,
                                    const SwTwips          _nVertPos,
                                    const SvxLRSpaceItem& _rLRSpacing,
                                    const SvxULSpaceItem& _rULSpacing,
                                    SwTwips& _roVertOffsetToFrmAnchorPos ) const;

        // *********************************************************************
            /** adjust calculated vertical in order to keep object inside
                'page' alignment layout frame.

                OD 2004-07-22 #i31805# - add 3rd parameter <_bCheckBottom>

                @param _nTopOfAnch
                input parameter - 'vertical' position, at which the relative
                position of the object is calculated from.

                @param _bVert
                input parameter - boolean, indicating, if object is in vertical
                layout.

                @param _rPageAlignLayFrm
                input parameter - layout frame, which determines the 'page area'
                the object has to be vertical positioned in.

                @param _nProposedRelPosY
                input parameter - proposed relative vertical position, which
                will be adjusted.

                @param _bCheckBottom
                input parameter - boolean indicating, if bottom of anchored
                object has to be checked and thus, (if needed) the proposed
                relative position has to be adjusted. default value <true>

                @author OD
            */
            SwTwips _AdjustVertRelPos( const SwTwips _nTopOfAnch,
                                       const bool _bVert,
                                       const SwFrm&  _rPageAlignLayFrm,
                                       const SwTwips _nProposedRelPosY,
                                       const bool _bCheckBottom = true ) const;

        // *********************************************************************
            /** calculate relative horizontal position

                --> OD 2004-06-17 #i26791# - add output parameter
                <_roHoriOffsetToFrmAnchorPos>

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame the horizontal position of the object
                is oriented at.

                @param _rEnvOfObj
                input parameter - object instance to retrieve environment
                information about the object

                @param _rHoriOrient
                input parameter - horizontal positioning and alignment, for which
                the relative position is calculated.

                @param _rLRSpacing
                input parameter - left and right spacing of the object to the text

                @param _rULSpacing
                input parameter - upper and lower spacing of the object to the text

                @param _bObjWrapThrough
                input parameter - boolean indicating, if object has wrap mode
                'wrap through'.

                @param _nRelPosY
                input parameter - relative vertical position

                @param _roHoriOffsetToFrmAnchorPos
                output parameter - 'horizontal' offset to frame anchor position
                according to the alignment

                @return relative horizontal position in SwTwips
            */
            SwTwips _CalcRelPosX( const SwFrm& _rHoriOrientFrm,
                                  const SwEnvironmentOfAnchoredObject& _rEnvOfObj,
                                  const SwFmtHoriOrient& _rHoriOrient,
                                  const SvxLRSpaceItem& _rLRSpacing,
                                  const SvxULSpaceItem& _rULSpacing,
                                  const bool _bObjWrapThrough,
                                  const SwTwips _nRelPosY,
                                  SwTwips& _roHoriOffsetToFrmAnchorPos
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
                                            SwRelationOrient&  _iopeRelOrient
                                          ) const;

        // *********************************************************************
            /** determine alignment values for horizontal position of object

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame the horizontal position of the object
                is oriented at.

                @param _rPageAlignLayFrm
                input paramter - layout frame, which determines the 'page area'
                the object has to be horizontal positioned in.

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
                                          const SwRelationOrient _eRelOrient,
                                          const bool    _bObjWrapThrough,
                                          SwTwips&      _orAlignAreaWidth,
                                          SwTwips&      _orAlignAreaOffset,
                                          bool&         _obAlignedRelToPage ) const;

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

        public:
            virtual void CalcPosition() = 0;
    };
};

#endif
