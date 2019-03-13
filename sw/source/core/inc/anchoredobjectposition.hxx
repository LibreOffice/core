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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_ANCHOREDOBJECTPOSITION_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ANCHOREDOBJECTPOSITION_HXX

#include <swtypes.hxx>
#include "frame.hxx"

class SdrObject;
class SwFlyFrame;
class SwContact;
class SwFrameFormat;
class SwRect;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFormatHoriOrient;
class SwAnchoredObject;

namespace objectpositioning
{
    class SwEnvironmentOfAnchoredObject;

    class SwAnchoredObjectPosition
    {
    private:
        // object to be positioned
        SdrObject& mrDrawObj;

        // does the object represents a Writer fly frame
        bool mbIsObjFly;
        // #i26791# - anchored object the object belongs to;
        SwAnchoredObject* mpAnchoredObj;
        // frame the object is anchored at
        SwFrame* mpAnchorFrame;
        // contact object
        SwContact* mpContact;
        // frame format
        const SwFrameFormat* mpFrameFormat;
        // #i62875#
        bool mbFollowTextFlow;
        bool mbLayoutInCell = false;
        // #i62875#
        // for compatibility option <DoNotCaptureDrawObjsOnPage>
        bool mbDoNotCaptureAnchoredObj;

        /** determine information about object

            member <mbIsObjFly>, <mpAnchoredObj>, <mpAnchorFrame>, <mpContact>
            and <mpFrameFormat> are set
        */
        void GetInfoAboutObj();

        // #i62875#
        // --> OD 2009-09-01 #mongolianlayout# - add parameter <bVertL2R>
        SwTwips ImplAdjustVertRelPos( const SwTwips nTopOfAnch,
                                       const bool bVert,
                                       const bool bVertL2R,
                                       const SwFrame&  rPageAlignLayFrame,
                                       const SwTwips nProposedRelPosY,
                                       const bool bFollowTextFlow,
                                       const bool bCheckBottom ) const;
        SwTwips ImplAdjustHoriRelPos( const SwFrame&  _rPageAlignLayFrame,
                                       const SwTwips _nProposedRelPosX ) const;

    protected:
        SwAnchoredObjectPosition( SdrObject& _rDrawObj );
        virtual ~SwAnchoredObjectPosition();

        // accessors for object and its corresponding data/information
        SdrObject& GetObject() const
        {
            return mrDrawObj;
        }
        bool IsObjFly() const
        {
            return mbIsObjFly;
        }
        SwAnchoredObject& GetAnchoredObj() const
        {
            return *mpAnchoredObj;
        }
        SwFrame& GetAnchorFrame() const
        {
            return *mpAnchorFrame;
        }
        const SwFrameFormat& GetFrameFormat() const
        {
            return *mpFrameFormat;
        }
        // #i62875#
        bool DoesObjFollowsTextFlow() const
        {
            return mbFollowTextFlow;
        }
        bool DoesObjLayoutInCell() const
        {
            return mbLayoutInCell;
        }

        // virtual methods providing data for to character anchored objects.
        virtual bool IsAnchoredToChar() const;
        virtual const SwFrame* ToCharOrientFrame() const;
        virtual const SwRect* ToCharRect() const;
        // #i22341#
        virtual SwTwips ToCharTopOfLine() const;

        /** helper method to determine top of a frame for the vertical object
            positioning

            #i11860#
        */
        SwTwips GetTopForObjPos( const SwFrame& _rFrame,
                                  const SwRectFn& _fnRect,
                                  const bool _bVert ) const;

        void GetVertAlignmentValues( const SwFrame& _rVertOrientFrame,
                                      const SwFrame& _rPageAlignLayFrame,
                                      const sal_Int16 _eRelOrient,
                                      SwTwips&      _orAlignAreaHeight,
                                      SwTwips&      _orAlignAreaOffset ) const;

        // #i26791# - add output parameter <_roVertOffsetToFrameAnchorPos>
        SwTwips GetVertRelPos( const SwFrame& _rVertOrientFrame,
                                const SwFrame& _rPageAlignLayFrame,
                                const sal_Int16 _eVertOrient,
                                const sal_Int16 _eRelOrient,
                                const SwTwips          _nVertPos,
                                const SvxLRSpaceItem& _rLRSpacing,
                                const SvxULSpaceItem& _rULSpacing,
                                SwTwips& _roVertOffsetToFrameAnchorPos ) const;

        /** adjust calculated vertical in order to keep object inside
            'page' alignment layout frame.

            #i31805# - add parameter <_bCheckBottom>
            #i26945# - add parameter <_bFollowTextFlow>
            #i62875# - made inline, intrinsic actions moved
            to private method <ImplAdjustVertRelPos>, which is only
            called, if <mbDoNotCaptureAnchoredObj> not set.
            OD 2009-09-01 #mongolianlayout# - add parameter <bVertL2R>

            @param nTopOfAnch
            input parameter - 'vertical' position, at which the relative
            position of the object is calculated from.

            @param bVert
            input parameter - boolean, indicating, if object is in vertical
            layout.

            @param bVertL2R
            input parameter - boolean, indicating, if object is in mongolian
            layout (vertical left-to-right layout).

            @param rPageAlignLayFrame
            input parameter - layout frame, which determines the 'page area'
            the object has to be vertical positioned in.

            @param nProposedRelPosY
            input parameter - proposed relative vertical position, which
            will be adjusted.

            @param bFollowTextFlow
            input parameter - value of attribute 'Follow text flow' of the
            anchored object.

            @param bCheckBottom
            input parameter - boolean indicating, if bottom of anchored
            object has to be checked and thus, (if needed) the proposed
            relative position has to be adjusted. default value <true>
        */
        SwTwips AdjustVertRelPos( const SwTwips nTopOfAnch,
                                          const bool bVert,
                                          const bool bVertL2R,
                                          const SwFrame& rPageAlignLayFrame,
                                          const SwTwips nProposedRelPosY,
                                          const bool bFollowTextFlow,
                                          const bool bCheckBottom = true ) const
        {
            return !mbDoNotCaptureAnchoredObj
                   ? ImplAdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                            rPageAlignLayFrame,
                                            nProposedRelPosY,
                                            bFollowTextFlow,
                                            bCheckBottom )
                   : nProposedRelPosY;
        }

        /** calculate relative horizontal position

            #i26791# - add output parameter
            <_roHoriOffsetToFrameAnchorPos>

            @param _rHoriOrientFrame
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

            @param _roHoriOffsetToFrameAnchorPos
            output parameter - 'horizontal' offset to frame anchor position
            according to the alignment

            @return relative horizontal position in SwTwips
        */
        SwTwips CalcRelPosX( const SwFrame& _rHoriOrientFrame,
                              const SwEnvironmentOfAnchoredObject& _rEnvOfObj,
                              const SwFormatHoriOrient& _rHoriOrient,
                              const SvxLRSpaceItem& _rLRSpacing,
                              const SvxULSpaceItem& _rULSpacing,
                              const bool _bObjWrapThrough,
                              const SwTwips _nRelPosY,
                              SwTwips& _roHoriOffsetToFrameAnchorPos
                            ) const;

        /** adjust calculated horizontal in order to keep object inside
            'page' alignment layout frame for object type position TO_CNTNT

            #i62875# - made inline, intrinsic actions moved
            to private method <ImplAdjustHoriRelPos>, which is only
            called, if <mbDoNotCaptureAnchoredObj> not set.

            @param _rPageAlignLayFrame
            input parameter - layout frame, which determines the 'page area'
            the object has to be horizontal positioned in.

            @param _nProposedRelPosX
            input parameter - proposed relative horizontal position, which
            will be adjusted.

            @return adjusted relative horizontal position in SwTwips.
        */
        SwTwips AdjustHoriRelPos( const SwFrame&  _rPageAlignLayFrame,
                                          const SwTwips _nProposedRelPosX ) const
        {
            return !mbDoNotCaptureAnchoredObj
                   ? ImplAdjustHoriRelPos( _rPageAlignLayFrame, _nProposedRelPosX )
                   : _nProposedRelPosX;
        }

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
        static void ToggleHoriOrientAndAlign( const bool _bToggleLeftRight,
                                        sal_Int16& _ioeHoriOrient,
                                        sal_Int16& _iopeRelOrient
                                      );

        /** determine alignment values for horizontal position of object

            @param _rHoriOrientFrame
            input parameter - frame the horizontal position of the object
            is oriented at.

            @param _rPageAlignLayFrame
            input parameter - layout frame, which determines the 'page area'
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
            important note: value is only relevant, if _rHoriOrientFrame is
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
        void GetHoriAlignmentValues( const SwFrame&  _rHoriOrientFrame,
                                      const SwFrame&  _rPageAlignLayFrame,
                                      const sal_Int16 _eRelOrient,
                                      const bool    _bObjWrapThrough,
                                      SwTwips&      _orAlignAreaWidth,
                                      SwTwips&      _orAlignAreaOffset,
                                      bool&         _obAlignedRelToPage ) const;

        /** adjust calculated horizontal position in order to draw object
            aside other objects with same positioning

            @param _rHoriOrientFrame
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
            input parameter - alignment of object

            @param _rLRSpacing
            input parameter - left and right spacing of the object to the text

            @param _rULSpacing
            input parameter - upper and lower spacing of the object to the text

            @param _bEvenPage
            input parameter - boolean indicating, if object is on an even page.

            @return adjusted relative horizontal position in SwTwips
        */
        SwTwips AdjustHoriRelPosForDrawAside( const SwFrame&  _rHoriOrientFrame,
                                               const SwTwips _nProposedRelPosX,
                                               const SwTwips _nRelPosY,
                                               const sal_Int16 _eHoriOrient,
                                               const sal_Int16 _eRelOrient,
                                               const SvxLRSpaceItem& _rLRSpacing,
                                               const SvxULSpaceItem& _rULSpacing,
                                               const bool _bEvenPage
                                             ) const;

        /** determine, if object has to draw aside given fly frame

            method used by <AdjustHoriRelPosForDrawAside(..)>

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
            input parameter - alignment of object

            @return boolean indicating, if object has to be drawn aside
            given fly frame.
        */
        bool DrawAsideFly( const SwFlyFrame* _pFly,
                            const SwRect&   _rObjRect,
                            const SwFrame*    _pObjContext,
                            const sal_uLong     _nObjIndex,
                            const bool      _bEvenPage,
                            const sal_Int16 _eHoriOrient,
                            const sal_Int16 _eRelOrient
                          ) const;

        /** determine, if object has to draw aside another object

            the different alignments of the objects determines, if one has
            to draw aside another one. Thus, the given alignment are checked
            against each other, which one has to be drawn aside the other one.
            depending on parameter _bLeft check is done for left or right
            positioning.
            method used by <DrawAsideFly(..)>

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
        static bool Minor_( sal_Int16 _eRelOrient1,
                     sal_Int16 _eRelOrient2,
                     bool             _bLeft );

    public:
        virtual void CalcPosition() = 0;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
