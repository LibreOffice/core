/*************************************************************************
 *
 *  $RCSfile: anchoredobject.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: od $ $Date: 2004-08-03 05:55:53 $
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
#ifndef _ANCHOREDOBJECT_HXX
#define _ANCHOREDOBJECT_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif

class SdrObject;
class SwFrm;
class SwLayoutFrm;
// --> OD 2004-07-14 #117380#
class SwTxtFrm;
// <--
// --> OD 2004-06-30 #i28701#
class SwPageFrm;
class SwObjPositioningInProgress;
// <--
class SwFrmFmt;
class SwFmtAnchor;

/** wrapper class for the positioning of Writer fly frames and drawing objects

    OD 2004-03-22 #i26791#
    Purpose of this class is to provide a unified interface for the positioning
    of Writer fly frames (derived classes of <SwFlyFrm>) and of drawing objects
    (derived classes of <SwDrawFrm>).

    @author OD
*/
class SwAnchoredObject
{
    private:
        // drawing object representing the anchored object in the drawing layer
        SdrObject* mpDrawObj;
        // frame the object is anchored at
        SwFrm* mpAnchorFrm;
        // --> OD 2004-06-30 #i28701# - page frame the object is registered at
        // note: no page frame for as-character anchored objects
        SwPageFrm* mpPageFrm;
        // <--
        // last relative position
        Point maRelPos;

        // for to-character anchored objects:
        // Last known anchor character retangle.
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

        // --> OD 2004-06-30 #i28701# - boolean, indicating that the object
        // positioning algorithm is in progress.
        bool mbPositioningInProgress;
        // <--

        // --> OD 2004-06-29 #i28701# - Booleans needed for the layout process.
        // Values only of relevance for to-paragraph and to-character anchored
        // floating screen object, which aren't in the 'hell' layer and have
        // a wrapping style different from 'SURROUND_TRHOUGH', if document
        // compatibility option 'Consider wrapping style on object positioning' is ON.
        // Otherwise value of <mbConsiderForTextWrap> is treated as <true>,
        // value of <mbPositionLocked> is treated as <false> and
        // value of <mbRestartLayoutProcess> is treated as <false>.
        bool mbConsiderForTextWrap;
        bool mbPositionLocked;
        bool mbRestartLayoutProcess;
        // <--

        /** method to indicate, that positioning of anchored object is in progress

            note: method is implemented empty

            @author OD
        */
        friend class SwObjPositioningInProgress;
        void SetPositioningInProgress( const bool _bPosInProgress );

        /** check anchor character rectangle

            OD 2004-03-25 #i26791#
            helper method for method <CheckCharRectAndTopOfLine()>
            For to-character anchored Writer fly frames the member <maLastCharRect>
            is updated. This is checked for change and depending on the applied
            positioning, it's decided, if the Writer fly frame has to be invalidated.
            OD 2004-07-14 #117380#
            improvement - add second parameter <_rAnchorCharFrm>

            @author OD

            @param _rAnch
            input parameter - reference to anchor position

            @param _rAnchorCharFrm
            input parameter - reference to the text frame containing the anchor
            character.
        */
        void _CheckCharRect( const SwFmtAnchor& _rAnch,
                             const SwTxtFrm& _rAnchorCharFrm );

        /** check top of line

            OD 2004-03-25 #i26791#
            helper method for method <CheckCharRectAndTopOfLine()>
            For to-character anchored Writer fly frames the member <mnLastTopOfLine>
            is updated. This is checked for change and depending on the applied
            positioning, it's decided, if the Writer fly frame has to be invalidated.
            OD 2004-07-14 #117380#
            improvement - add second parameter <_rAnchorCharFrm>

            @author OD

            @param _rAnch
            input parameter - reference to anchor position

            @param _rAnchorCharFrm
            input parameter - reference to the text frame containing the anchor
            character.
        */
        void _CheckTopOfLine( const SwFmtAnchor& _rAnch,
                              const SwTxtFrm& _rAnchorCharFrm );

    protected:
        SwAnchoredObject();

        void SetVertPosOrientFrm( const SwLayoutFrm& _rVertPosOrientFrm );

        /** method to assure that anchored object is registered at the correct
            page frame

            OD 2004-07-02 #i28701#

            @author OD
        */
        virtual void RegisterAtCorrectPage() = 0;

        /** method to indicate, that anchored object is attached to a anchor frame

            @author OD
        */
        virtual void ObjectAttachedToAnchorFrame();

    public:
        TYPEINFO();

        virtual ~SwAnchoredObject();

        // accessors to member <mpDrawObj>
        void SetDrawObj( SdrObject& _rDrawObj );
        const SdrObject* GetDrawObj() const;
        SdrObject* DrawObj();

        // accessors to member <mpAnchorFrm>
        const SwFrm* GetAnchorFrm() const;
        SwFrm* AnchorFrm();
        void ChgAnchorFrm( SwFrm* _pNewAnchorFrm );

        // --> OD 2004-06-30 #i28701# - accessors to member <mpPageFrm>
        SwPageFrm* GetPageFrm();
        const SwPageFrm* GetPageFrm() const;
        void SetPageFrm( SwPageFrm* _pNewPageFrm );
        // <--

        /** method to determine the page frame, on which the 'anchor' of
            the given anchored object is.

            OD 2004-07-02 #i28701#

            @author OD

            @param _rAnchoredObj
            input parameter - anchored object, for which the page frame of its
            'anchor' has to be determined.

            @return SwPageFrm&
            page frame, the 'anchor' of the given anchored object is on
        */
        SwPageFrm& GetPageFrmOfAnchor();

        // accessors to data of position calculation:
        // frame vertical position is orient at
        const SwLayoutFrm* GetVertPosOrientFrm() const;

        /** check anchor character rectangle and top of line

            OD 2004-03-25 #i26791#
            For to-character anchored Writer fly frames the members <maLastCharRect>
            and <maLastTopOfLine> are updated. These are checked for change and
            depending on the applied positioning, it's decided, if the Writer fly
            frame has to be invalidated.
            OD 2004-07-15 #117380#
            add parameter <_bCheckForParaPorInf>, default value <true>

            @author OD

            @param _bCheckForParaPorInf
            input parameter - boolean indicating, if check on paragraph portion
            information has to be done.
        */
        void CheckCharRectAndTopOfLine( const bool _bCheckForParaPorInf = true );

        // accessors to member <maLastCharRect>
        const SwRect& GetLastCharRect() const;
        SwTwips GetLastCharX() const;
        SwTwips GetRelCharX( const SwFrm* pFrm ) const;
        SwTwips GetRelCharY( const SwFrm* pFrm ) const;
        void AddLastCharY( long nDiff );
        void ResetLastCharRectHeight();

        // accessor to member <nmLastTopOfLine>
        const SwTwips GetLastTopOfLine() const;
        // OD 2004-05-18 #i28701# - follow-up of #i22341#
        void AddLastTopOfLineY( SwTwips _nDiff );

        /** reset members <maLastCharRect> and <mnLastTopOfLine>

            OD 2004-06-29 #i27801#

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
        bool IsPositioningInProgress() const;

        /** method to determine, if invalidation of position is allowed

            OD 2004-07-01 #i28701#

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

            OD 2004-06-30 #i28701#

            @author OD
        */
        void InvalidateObjPosForConsiderWrapInfluence( const bool _bNotifyBackgrd );

        /** method to trigger notification of 'background'

            OD 2004-07-01 #i28701#

            @author OD
        */
        virtual void NotifyBackground( SwPageFrm* _pPageFrm,
                                       const SwRect& _rRect,
                                       PrepareHint _eHint ) = 0;

        // accessors to the current relative position
        const Point GetCurrRelPos() const;
        void SetCurrRelPos( Point _aRelPos );

        // accessors to the format
        virtual SwFrmFmt& GetFrmFmt() = 0;
        virtual const SwFrmFmt& GetFrmFmt() const = 0;

        // accessors to the object area and its position
        virtual const SwRect GetObjRect() const = 0;
        virtual void SetObjTop( const SwTwips _nTop) = 0;
        virtual void SetObjLeft( const SwTwips _nLeft) = 0;

        /** method to determine object area inclusive its spacing

            OD 2004-06-30 #i28701#

            @author OD
        */
        const SwRect GetObjRectWithSpaces() const;

        /** method to determine, if wrapping style influence of the anchored
            object has to be considered on the object positioning

            OD 2004-06-30 #i28701#
            Note: result of this method also decides, if the boolean for the
            layout process are of relevance.

            @author OD
        */
        bool ConsiderObjWrapInfluenceOnObjPos() const;

        // --> OD 2004-06-29 #i28701# - accessors to booleans for layout process
        bool ConsiderForTextWrap() const;
        void SetConsiderForTextWrap( const bool _bConsiderForTextWrap );
        bool PositionLocked() const;
        void LockPosition();
        void UnlockPosition();
        bool RestartLayoutProcess() const;
        void SetRestartLayoutProcess( const bool _bRestartLayoutProcess );
        // <--

        /** method to update anchored object in the <SwSortedObjs> lists

            OD 2004-07-01 #i28701#
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

            OD 2004-07-23 #i28701#
            A format is possible, if anchored object is in an invisible layer.
            Note: method is virtual to refine the conditions for the sub-classes.

            @author OD
        */
        virtual bool IsFormatPossible() const;
};

// ============================================================================
// OD 2004-04-13 #i26791#, #i28701#
// helper class for notify that positioning of an anchored object is in progress
// ============================================================================
class SwObjPositioningInProgress
{
    private:
        SwAnchoredObject* mpAnchoredObj;

    public:
        SwObjPositioningInProgress( SdrObject& _rSdrObj );
        SwObjPositioningInProgress( SwAnchoredObject& _rAnchoredObj );
        ~SwObjPositioningInProgress();
};

#endif
