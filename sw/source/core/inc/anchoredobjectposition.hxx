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
#ifndef _ANCHOREDOBJECTPOSITION_HXX
#define _ANCHOREDOBJECTPOSITION_HXX

#include <swtypes.hxx>
// #i11860#
#include <frame.hxx>

class SdrObject;
class SwFrm;
class SwFlyFrm;
class SwContact;
class SwFrmFmt;
class SwRect;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFmtHoriOrient;
// #i26701#
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
            // #i26791# - anchored object the object belongs to;
            SwAnchoredObject* mpAnchoredObj;
            // frame the object is anchored at
            SwFrm* mpAnchorFrm;
            // contact object
            SwContact* mpContact;
            // frame format
            const SwFrmFmt* mpFrmFmt;
            // #i62875#
            bool mbFollowTextFlow;
            // #i62875#
            // for compatibility option <DoNotCaptureDrawObjsOnPage>
            bool mbDoNotCaptureAnchoredObj;

            /** determine information about object

                member <mbIsObjFly>, <mpAnchoredObj>, <mpAnchorFrm>, <mpContact>
                and <mpFrmFmt> are set
            */
            void _GetInfoAboutObj();

            // #i62875#
            SwTwips _ImplAdjustVertRelPos( const SwTwips _nTopOfAnch,
                                           const bool _bVert,
                                           const SwFrm&  _rPageAlignLayFrm,
                                           const SwTwips _nProposedRelPosY,
                                           const bool _bFollowTextFlow,
                                           const bool _bCheckBottom = true ) const;
            SwTwips _ImplAdjustHoriRelPos( const SwFrm&  _rPageAlignLayFrm,
                                           const SwTwips _nProposedRelPosX ) const;

        protected:
            SwAnchoredObjectPosition( SdrObject& _rDrawObj );
            virtual ~SwAnchoredObjectPosition();

         // accessors for object and its corresponding data/information
            inline SdrObject& GetObject() const
            {
                return mrDrawObj;
            }
            inline bool IsObjFly() const
            {
                return mbIsObjFly;
            }
            inline SwAnchoredObject& GetAnchoredObj() const
            {
                return *mpAnchoredObj;
            }
            inline SwFrm& GetAnchorFrm() const
            {
                return *mpAnchorFrm;
            }
            inline SwContact& GetContact() const
            {
                return *mpContact;
            }
            inline const SwFrmFmt& GetFrmFmt() const
            {
                return *mpFrmFmt;
            }
            // #i62875#
            inline bool DoesObjFollowsTextFlow() const
            {
                return mbFollowTextFlow;
            }

         // virtual methods providing data for to character anchored objects.
            virtual bool IsAnchoredToChar() const;
            virtual const SwFrm* ToCharOrientFrm() const;
            virtual const SwRect* ToCharRect() const;
            // #i22341#
            virtual SwTwips ToCharTopOfLine() const;

        // *********************************************************************
            /** helper method to determine top of a frame for the vertical
                object positioning

                #i11860#
            */
            SwTwips _GetTopForObjPos( const SwFrm& _rFrm,
                                      const SwRectFn& _fnRect,
                                      const bool _bVert ) const;

        // *********************************************************************
            void _GetVertAlignmentValues( const SwFrm& _rVertOrientFrm,
                                          const SwFrm& _rPageAlignLayFrm,
                                          const sal_Int16 _eRelOrient,
                                          SwTwips&      _orAlignAreaHeight,
                                          SwTwips&      _orAlignAreaOffset ) const;

        // *********************************************************************
        // #i26791# - add output parameter <_roVertOffsetToFrmAnchorPos>
            SwTwips _GetVertRelPos( const SwFrm& _rVertOrientFrm,
                                    const SwFrm& _rPageAlignLayFrm,
                                    const sal_Int16 _eVertOrient,
                                    const sal_Int16 _eRelOrient,
                                    const SwTwips          _nVertPos,
                                    const SvxLRSpaceItem& _rLRSpacing,
                                    const SvxULSpaceItem& _rULSpacing,
                                    SwTwips& _roVertOffsetToFrmAnchorPos ) const;

        // *********************************************************************
            /** adjust calculated vertical in order to keep object inside
                'page' alignment layout frame.

                #i31805# - add parameter <_bCheckBottom>
                #i26945# - add parameter <_bFollowTextFlow>
                #i62875# - made inline, intrinsic actions moved
                to private method <_ImplAdjustVertRelPos>, which is only
                called, if <mbDoNotCaptureAnchoredObj> not set.

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

                @param _bFollowTextFlow
                input parameter - value of attribute 'Follow text flow' of the
                anchored object.

                @param _bCheckBottom
                input parameter - boolean indicating, if bottom of anchored
                object has to be checked and thus, (if needed) the proposed
                relative position has to be adjusted. default value <true>
            */
            inline SwTwips _AdjustVertRelPos( const SwTwips _nTopOfAnch,
                                              const bool _bVert,
                                              const SwFrm&  _rPageAlignLayFrm,
                                              const SwTwips _nProposedRelPosY,
                                              const bool _bFollowTextFlow,
                                              const bool _bCheckBottom = true ) const
            {
                return !mbDoNotCaptureAnchoredObj
                       ? _ImplAdjustVertRelPos( _nTopOfAnch, _bVert,
                                                _rPageAlignLayFrm,
                                                _nProposedRelPosY,
                                                _bFollowTextFlow,
                                                _bCheckBottom )
                       : _nProposedRelPosY;
            }

        // *********************************************************************
            /** calculate relative horizontal position

                #i26791# - add output parameter
                <_roHoriOffsetToFrmAnchorPos>

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

                #i62875# - made inline, intrinsic actions moved
                to private method <_ImplAdjustHoriRelPos>, which is only
                called, if <mbDoNotCaptureAnchoredObj> not set.

                @param _rPageAlignLayFrm
                input paramter - layout frame, which determines the 'page area'
                the object has to be horizontal positioned in.

                @param _nProposedRelPosX
                input parameter - proposed relative horizontal position, which
                will be adjusted.

                @return adjusted relative horizontal position in SwTwips.
            */
            inline SwTwips _AdjustHoriRelPos( const SwFrm&  _rPageAlignLayFrm,
                                              const SwTwips _nProposedRelPosX ) const
            {
                return !mbDoNotCaptureAnchoredObj
                       ? _ImplAdjustHoriRelPos( _rPageAlignLayFrm, _nProposedRelPosX )
                       : _nProposedRelPosX;
            }

        // *********************************************************************
            /** toggle given horizontal orientation and relative alignment

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
            void _ToggleHoriOrientAndAlign( const bool _bToggleLeftRight,
                                            sal_Int16& _ioeHoriOrient,
                                            sal_Int16& _iopeRelOrient
                                          ) const;

        // *********************************************************************
            /** determine alignment values for horizontal position of object

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
                                          const sal_Int16 _eRelOrient,
                                          const bool    _bObjWrapThrough,
                                          SwTwips&      _orAlignAreaWidth,
                                          SwTwips&      _orAlignAreaOffset,
                                          bool&         _obAlignedRelToPage ) const;

        // *********************************************************************
            /** adjust calculated horizontal position in order to draw object
                aside other objects with same positioning

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
                                                   const sal_Int16 _eHoriOrient,
                                                   const sal_Int16 _eRelOrient,
                                                   const SvxLRSpaceItem& _rLRSpacing,
                                                   const SvxULSpaceItem& _rULSpacing,
                                                   const bool _bEvenPage
                                                 ) const;

        // *********************************************************************
            /** detemine, if object has to draw aside given fly frame

                method used by <_AdjustHoriRelPosForDrawAside(..)>

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
                                const sal_Int16 _eHoriOrient,
                                const sal_Int16 _eRelOrient
                              ) const;

        // *********************************************************************
            /** determine, if object has to draw aside another object

                the different alignments of the objects determines, if one has
                to draw aside another one. Thus, the given alignment are checked
                against each other, which one has to be drawn aside the other one.
                depending on parameter _bLeft check is done for left or right
                positioning.
                method used by <_DrawAsideFly(..)>

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
            bool _Minor( sal_Int16 _eRelOrient1,
                         sal_Int16 _eRelOrient2,
                         bool             _bLeft ) const;

        public:
            virtual void CalcPosition() = 0;
    };
} // namespace objectpositioning

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
