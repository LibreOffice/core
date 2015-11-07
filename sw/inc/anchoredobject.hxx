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
#ifndef INCLUDED_SW_INC_ANCHOREDOBJECT_HXX
#define INCLUDED_SW_INC_ANCHOREDOBJECT_HXX

#include <swtypes.hxx>
#include <swrect.hxx>
#include <libxml/xmlwriter.h>

class SdrObject;
class SwFrm;
class SwLayoutFrm;
class SwTextFrm;

class SwPageFrm;
class SwObjPositioningInProgress;

class SwFrameFormat;
class SwFormatAnchor;

/** wrapper class for the positioning of Writer fly frames and drawing objects

    Purpose of this class is to provide a unified interface for the positioning
    of Writer fly frames (derived classes of <SwFlyFrm>) and of drawing objects
    (derived classes of <SwDrawFrm>).

    @author OD
*/
class SW_DLLPUBLIC SwAnchoredObject
{
    private:
        // drawing object representing the anchored object in the drawing layer
        SdrObject* mpDrawObj;
        // frame the object is anchored at
        SwFrm* mpAnchorFrm;
        // #i28701 - page frame the object is registered at
        // note: no page frame for as-character anchored objects
        SwPageFrm* mpPageFrm;
        // current relative position (relative to anchor position of anchor frame)
        Point maRelPos;

        // for to-character anchored objects:
        // Last known anchor character rectangle.
        // Used to decide, if invalidation has to been performed, if anchor position
        // has changed, and used to position object.
        SwRect maLastCharRect;

        // for to-character anchored objects:
        // Last known top of line, in which the anchor character is in.
        // Used to decide, if invalidation has to been performed, if anchor position
        // has changed, and used to position object.
        SwTwips mnLastTopOfLine;

        // for to-paragraph and to-character anchored objects:
        // Layout frame vertical position is orient at - typically its the upper
        // of the anchor frame, but it could also by the upper of a follow or
        // a following layout frame in the text flow.
        const SwLayoutFrm* mpVertPosOrientFrm;

        // i#i28701 boolean, indicating that the object
        // positioning algorithm is in progress.
        bool mbPositioningInProgress;

        // Booleans needed for the layout process.
        // Values only of relevance for to-paragraph and to-character anchored
        // floating screen object, for whose the 'straight-forward positioning
        // process are applied
        // Otherwise value of <mbConsiderForTextWrap> is treated as <true>,
        // value of <mbPositionLocked> is treated as <false> and
        // value of <mbRestartLayoutProcess> is treated as <false>.
        // i#35911 - add boolean <mbClearEnvironment>
        // Indicates that due to its position and wrapping style its layout
        // environment is cleared - all content is moved forward.
        // Treated as <false>, if not the 'straight-forward positioning process"
        // is applied.
        bool mbConsiderForTextWrap;
        bool mbPositionLocked;
        // boolean needed to keep position of
        // anchored object locked due to special object positioning for sections.
        bool mbKeepPositionLockedForSection;

        bool mbRestartLayoutProcess;
        bool mbClearedEnvironment;

        // i#i3317 - boolean, indicating that temporarly
        // the wrapping style influence of the anchored object has to be
        // considered during its positioning.
        // This boolean is used, if compatibility option 'Consider wrapping style
        // influence on object positioning' is OFF and a positioning loop is
        // detected in method <SwFlyAtCntFrm::MakeAll()> or method
        // <SwAnchoredDrawObject::_MakeObjPosAnchoredAtPara()>.
        // The boolean is reset to <false>, when the layout process for a
        // page frame starts.
        bool mbTmpConsiderWrapInfluence;

        mutable SwRect maObjRectWithSpaces;
        mutable bool mbObjRectWithSpacesValid;
        mutable SwRect maLastObjRect;

        /** method to indicate, that positioning of anchored object is in progress

            note: method is implemented empty

            @author OD
        */
        friend class SwObjPositioningInProgress;
        inline void SetPositioningInProgress( const bool _bPosInProgress )
        {
            mbPositioningInProgress = _bPosInProgress;
        }

        /** check anchor character rectangle

            helper method for method <CheckCharRectAndTopOfLine()>
            For to-character anchored Writer fly frames the member <maLastCharRect>
            is updated. This is checked for change and depending on the applied
            positioning, it's decided, if the Writer fly frame has to be invalidated.
            improvement - add second parameter <_rAnchorCharFrm>

            @author OD

            @param _rAnch
            input parameter - reference to anchor position

            @param _rAnchorCharFrm
            input parameter - reference to the text frame containing the anchor
            character.
        */
        void _CheckCharRect( const SwFormatAnchor& _rAnch,
                             const SwTextFrm& _rAnchorCharFrm );

        /** check top of line

            helper method for method <CheckCharRectAndTopOfLine()>
            For to-character anchored Writer fly frames the member <mnLastTopOfLine>
            is updated. This is checked for change and depending on the applied
            positioning, it's decided, if the Writer fly frame has to be invalidated.

            @author OD

            @param _rAnch
            input parameter - reference to anchor position

            @param _rAnchorCharFrm
            input parameter - reference to the text frame containing the anchor
            character.
        */
        void _CheckTopOfLine( const SwFormatAnchor& _rAnch,
                              const SwTextFrm& _rAnchorCharFrm );

        // method <sw_HideObj(..)> sets needed data structure values for the
        // object positioning
        friend bool sw_HideObj( const SwTextFrm& _rFrm,
                                 const RndStdIds _eAnchorType,
                                 const sal_Int32 _nObjAnchorPos,
                                 SwAnchoredObject* _pAnchoredObj );
    protected:
        SwAnchoredObject();

        void SetVertPosOrientFrm( const SwLayoutFrm& _rVertPosOrientFrm );

        /** method to assure that anchored object is registered at the correct
            page frame

            @author OD
        */
        virtual void RegisterAtCorrectPage() = 0;

        /** method to indicate, that anchored object is attached to a anchor frame

            @author OD
        */
        virtual void ObjectAttachedToAnchorFrame();

        /** method to determine, if other anchored objects, also attached at
            to the anchor frame, have to consider its wrap influence.

            @author OD
        */
        bool ConsiderObjWrapInfluenceOfOtherObjs() const;

        /** method to apply temporary consideration of wrapping style influence
            to the anchored objects, which are anchored at the same anchor frame

            @author OD
        */
        void SetTmpConsiderWrapInfluenceOfOtherObjs( const bool bTmpConsiderWrapInfluence );

        virtual bool _SetObjTop( const SwTwips _nTop) = 0;
        virtual bool _SetObjLeft( const SwTwips _nLeft) = 0;

        virtual const SwRect GetObjBoundRect() const = 0;
    public:

        virtual ~SwAnchoredObject();

        // accessors to member <mpDrawObj>
        void SetDrawObj( SdrObject& _rDrawObj );
        const SdrObject* GetDrawObj() const { return mpDrawObj;}
        SdrObject* DrawObj() { return mpDrawObj;}

        // accessors to member <mpAnchorFrm>
        const SwFrm* GetAnchorFrm() const { return mpAnchorFrm;}
        SwFrm* AnchorFrm() { return mpAnchorFrm;}
        void ChgAnchorFrm( SwFrm* _pNewAnchorFrm );
        /** determine anchor frame containing the anchor position

            the anchor frame, which is determined, is <mpAnchorFrm>
            for an at-page, at-frame or at-paragraph anchored object
            and the anchor character frame for an at-character and as-character
            anchored object.

            @author OD
        */
        SwFrm* GetAnchorFrmContainingAnchPos();

        SwPageFrm* GetPageFrm() { return mpPageFrm;}
        const SwPageFrm* GetPageFrm() const { return mpPageFrm;}
        void SetPageFrm( SwPageFrm* _pNewPageFrm );

        /** method to determine the page frame, on which the 'anchor' of
            the given anchored object is.

            Adjust meaning of method and thus its name: If the anchored object
            or its anchor isn't correctly inserted in the layout, no page frame
            can be found. Thus, the return type changed to be a pointer and can
            be NULL.

            @author OD

            @param _rAnchoredObj
            input parameter - anchored object, for which the page frame of its
            'anchor' has to be determined.

            @return SwPageFrm&
            page frame, the 'anchor' of the given anchored object is on
        */
        SwPageFrm* FindPageFrmOfAnchor();

        /** get frame, which contains the anchor character, if the object
            is anchored at-character or as-character.

            @author OD

            @return SwTextFrm*
            text frame containing the anchor character. It's NULL, if the object
            isn't anchored at-character resp. as-character.
        */
        SwTextFrm* FindAnchorCharFrm();

        // accessors to data of position calculation:
        // frame vertical position is orient at
        const SwLayoutFrm* GetVertPosOrientFrm() const
        {
            return mpVertPosOrientFrm;
        }
        // method to clear member <mpVertPosOrientFrm>
        void ClearVertPosOrientFrm();

        /** check anchor character rectangle and top of line

            For to-character anchored Writer fly frames the members <maLastCharRect>
            and <maLastTopOfLine> are updated. These are checked for change and
            depending on the applied positioning, it's decided, if the Writer fly
            frame has to be invalidated.

            @author OD

            @param _bCheckForParaPorInf
            input parameter - boolean indicating, if check on paragraph portion
            information has to be done.
        */
        void CheckCharRectAndTopOfLine( const bool _bCheckForParaPorInf = true );

        // accessors to member <maLastCharRect>
        const SwRect& GetLastCharRect() const { return maLastCharRect;}
        SwTwips GetRelCharX( const SwFrm* pFrm ) const;
        SwTwips GetRelCharY( const SwFrm* pFrm ) const;
        void AddLastCharY( long nDiff );
        void ResetLastCharRectHeight();

        // accessor to member <nmLastTopOfLine>
        SwTwips GetLastTopOfLine() const { return mnLastTopOfLine;}
        void AddLastTopOfLineY( SwTwips _nDiff );

        /** reset members <maLastCharRect> and <mnLastTopOfLine>

            @author OD
        */
        void ClearCharRectAndTopOfLine();

        /** method to determine position for the object and set the position
            at the object

            @author OD
        */
        virtual void MakeObjPos() = 0;

        /** is positioning of anchored object in progress

            @author OD
        */
        inline bool IsPositioningInProgress() const
        {
            return mbPositioningInProgress;
        }

        /** method to determine, if invalidation of position is allowed

            @author OD
        */
        bool InvalidationOfPosAllowed() const;

        /** method to invalidate position of the anchored object

            @author OD
        */
        virtual void InvalidateObjPos() = 0;

        /** method to perform necessary invalidations for the positioning of
            objects, for whose the wrapping style influence has to be considered
            on the object positioning.

            @author OD
        */
        void InvalidateObjPosForConsiderWrapInfluence( const bool _bNotifyBackgrd );

        /** method to trigger notification of 'background'

            @author OD
        */
        virtual void NotifyBackground( SwPageFrm* _pPageFrm,
                                       const SwRect& _rRect,
                                       PrepareHint _eHint ) = 0;

        // accessors to the current relative position (relative to anchor
        // position of anchor frame)
        const Point GetCurrRelPos() const { return maRelPos;}
        void SetCurrRelPos( Point _aRelPos );

        // accessors to the format
        virtual SwFrameFormat& GetFrameFormat() = 0;
        virtual const SwFrameFormat& GetFrameFormat() const = 0;

        // accessors to the object area and its position
        virtual const SwRect GetObjRect() const = 0;

        void SetObjTop( const SwTwips _nTop);
        void SetObjLeft( const SwTwips _nLeft);

        /** method update layout direction the layout direction, the anchored
            object is assigned to

            method has typically to be called, if the anchored object gets its
            anchor frame assigned and if the anchor frame changes its layout direction

            @author OD
        */
        virtual void UpdateLayoutDir();

        /** method to determine object area inclusive its spacing

            @author OD
        */
        const SwRect& GetObjRectWithSpaces() const;

        inline void InvalidateObjRectWithSpaces() const
        {
            mbObjRectWithSpacesValid = false;
        }

        /** method to determine, if wrapping style influence of the anchored
            object has to be considered on the object positioning

            Note: result of this method also decides, if the boolean for the
            layout process are of relevance.

            @author OD
        */
        bool ConsiderObjWrapInfluenceOnObjPos() const;

        // accessors to booleans for layout process
        bool ConsiderForTextWrap() const;
        void SetConsiderForTextWrap( const bool _bConsiderForTextWrap );
        bool PositionLocked() const;
        inline void LockPosition()
        {
            mbPositionLocked = true;
        }
        inline void UnlockPosition()
        {
            if ( !mbKeepPositionLockedForSection )
            {
                mbPositionLocked = false;
            }
        }

        inline void SetKeepPosLocked( const bool _bKeepPosLocked )
        {
            mbKeepPositionLockedForSection = _bKeepPosLocked;
        }

        bool RestartLayoutProcess() const;
        void SetRestartLayoutProcess( const bool _bRestartLayoutProcess );
        // accessors for <mbClearedEnvironment>
        bool ClearedEnvironment() const;
        void SetClearedEnvironment( const bool _bClearedEnvironment );

        // reset booleans for layout process
        inline void ResetLayoutProcessBools()
        {
            mbPositioningInProgress = false;
            mbConsiderForTextWrap = false;
            mbPositionLocked = false;
            mbKeepPositionLockedForSection = false;
            mbRestartLayoutProcess = false;
            mbClearedEnvironment = false;
            mbTmpConsiderWrapInfluence = false;
        }

        /** method to determine, if due to anchored object size and wrapping
            style, its layout environment is cleared.

            @author OD
        */
        bool HasClearedEnvironment() const;

        /** method to update anchored object in the <SwSortedObjs> lists

            Method is not proposed to be called during a layout process is
            running. It has been used on the change of the anchored object
            attributes, which belongs the sort criteria of <SwSortedObjs>.
            If document compatibility option 'Consider wrapping style influence
            on object positioning' is ON, additionally all anchored objects
            at the anchor frame and all following anchored objects on the page
            frame are invalidated.

            @author OD
        */
        void UpdateObjInSortedList();

        /** method to determine, if a format on the anchored object is possible

            A format isn't possible, if anchored object is in an invisible layer.
            Note: method is virtual to refine the conditions for the sub-classes.

            @author OD
        */
        virtual bool IsFormatPossible() const;

        // accessors to member <mbTmpConsiderWrapInfluence>
        void SetTmpConsiderWrapInfluence( const bool _bTmpConsiderWrapInfluence );
        bool IsTmpConsiderWrapInfluence() const { return mbTmpConsiderWrapInfluence;}

        /** method to determine, if the anchored object is overlapping with a
            previous column

            overlapping with a previous column means, that the object overlaps
            with a column, which is a previous one of the column its anchor
            frame is in.
            Only applied for at-paragraph and at-character anchored objects.

            @author OD
        */
        bool OverlapsPrevColumn() const;

        /** method to determine position of anchored object relative to
            anchor frame

            Usage: Needed layout information for WW8 export

            @author OD

            @return Point - determined relative position
        */
        Point GetRelPosToAnchorFrm() const;

        /** method to determine position of anchored object relative to
            page frame

            Usage: Needed layout information for WW8 export

            If <_bFollowTextFlow> is set and object is anchored inside table,
            the position relative to the table cell is determined. Output
            parameter <_obRelToTableCell> reflects this situation

            @author OD

            @param _bFollowTextFlow
            input parameter - boolean indicating, if the anchored object has to
            follow the text flow.

            @param _obRelToTableCell
            output parameter - boolean indicating, the determine position is
            relative to the table cell

            @return Point - determined relative position
        */
        Point GetRelPosToPageFrm( const bool _bFollowTextFlow,
                                  bool& _obRelToTableCell ) const;

        /** method to determine position of anchored object relative to
            anchor character

            Usage: Needed layout information for WW8 export

            @author OD

            @return Point - determined relative position
        */
        Point GetRelPosToChar() const;

        /** method to determine position of anchored object relative to
            top of line

            Usage: Needed layout information for WW8 export

            @author OD

            @return Point - determined relative position
        */
        Point GetRelPosToLine() const;

        /** Dump a bunch of useful data to an XML representation to ease
            layout understanding, debugging and testing.
          */
        virtual void dumpAsXml( xmlTextWriterPtr pWriter ) const;

        /** The element name to show in the XML dump.
          */
        virtual const char* getElementName( ) const { return "SwAnchoredObject"; }
};

/// Helper class for notify that positioning of an anchored object is in progress.
class SwObjPositioningInProgress
{
    private:
        SwAnchoredObject* mpAnchoredObj;
        // boolean indicating old state
        // of anchored object regarding positioning in progress in order to
        // consider nested usage of class <SwObjPositioningInProgress>
        bool mbOldObjPositioningInProgress;

    public:
        SwObjPositioningInProgress( SdrObject& _rSdrObj );
        SwObjPositioningInProgress( SwAnchoredObject& _rAnchoredObj );
        ~SwObjPositioningInProgress();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
