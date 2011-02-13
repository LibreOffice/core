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
#ifndef _OBJECTFORMATTERTXTFRM_HXX
#define _OBJECTFORMATTERTXTFRM_HXX

#include <objectformatter.hxx>
#include <sal/types.h>

class SwTxtFrm;

// -----------------------------------------------------------------------------
// #i28701#
// Format floating screen objects, which are anchored at a given anchor text frame
// and registered at the given page frame.
// -----------------------------------------------------------------------------
class SwObjectFormatterTxtFrm : public SwObjectFormatter
{
    private:
        // anchor text frame
        SwTxtFrm& mrAnchorTxtFrm;

        // 'master' anchor text frame
        SwTxtFrm* mpMasterAnchorTxtFrm;

        SwObjectFormatterTxtFrm( SwTxtFrm& _rAnchorTxtFrm,
                                 const SwPageFrm& _rPageFrm,
                                 SwTxtFrm* _pMasterAnchorTxtFrm,
                                 SwLayAction* _pLayAction );

        /** method to invalidate objects, anchored previous to given object at
           the anchor text frame

            @param _rAnchoredObj
            reference to anchored object - objects, anchored previous to
            this one will be invalidated.
        */
        void _InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj );

        /** method to invalidate objects, anchored after the given object at
           the page frame

            @param _rAnchoredObj
            reference to anchored object - objects, anchored after this one will
            be invalidated.

            @param _bInclObj
            boolean indicates, if given anchored object <_rAnchoredObj> also have
            to be invalidated.
        */
        void _InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj,
                                    const bool _bInclObj );

        /** method to determine first anchored object, whose 'anchor is moved
            forward'.

            'anchor (of an object) is moved forward', if the anchor frame
            respectively the anchor character of the object isn't on the
            proposed page frame. Instead its on a following page

            #i26945# - For at-character anchored objects,
            it has also to be checked, if the anchor character is in a follow
            text frame, which would move to the next page.

            #i43913# - add output parameter <_boInFollow>

            @param _nWrapInfluenceOnPosition
            input parameter - only object with this given wrapping style
            influence are investigated.

            @param _nFromPageNum
            input parameter - number of page frame, the 'anchor' should be

            @param _noToPageNum
            output parameter - number of page frame, the 'anchor' of the returned
            anchored object is.

            @param _boInFollow
            output parameter - boolean, indicating that anchor text frame is
            currently on the same page, but it's a follow of in a follow row,
            which will move forward. value only relevant, if method returns
            an anchored object

            @return SwAnchoredObject*
            anchored object with a 'moved forward anchor'. If NULL, no such
            anchored object is found.
        */
        SwAnchoredObject* _GetFirstObjWithMovedFwdAnchor(
                                    const sal_Int16 _nWrapInfluenceOnPosition,
                                    sal_uInt32& _noToPageNum,
                                    bool& _boInFollow );

        /** method to format the anchor frame for checking of the move forward condition

            #i40141#
        */
        void _FormatAnchorFrmForCheckMoveFwd();

        /** method to determine if at least one anchored object has state
            <temporarly consider wrapping style influence> set.

            #b6449874#
        */
        bool _AtLeastOneObjIsTmpConsiderWrapInfluence();

    protected:

        virtual SwFrm& GetAnchorFrm();

    public:
        virtual ~SwObjectFormatterTxtFrm();

        // #i40147# - add parameter <_bCheckForMovedFwd>.
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                  const bool _bCheckForMovedFwd = false );
        virtual bool DoFormatObjs();

        /** method to create an instance of <SwObjectFormatterTxtFrm> is
            necessary.
        */
        static SwObjectFormatterTxtFrm* CreateObjFormatter(
                                                SwTxtFrm& _rAnchorTxtFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction );

        /** method to format given anchor text frame and its previous frames

            #i56300#
            Usage: Needed to check, if the anchor text frame is moved forward
            due to the positioning and wrapping of its anchored objects, and
            to format the frames, which have become invalid due to the anchored
            object formatting in the iterative object positioning algorithm

            @param _rAnchorTxtFrm
            input parameter - reference to anchor text frame, which has to be
            formatted including its previous frames of the page.
        */
        static void FormatAnchorFrmAndItsPrevs( SwTxtFrm& _rAnchorTxtFrm );

        /** method to check the conditions, if 'anchor is moved forward'

            #i26945#
            #i43913# - add output parameter <_boInFollow>
            #i58182# - replace method by a corresponding static
            method, because it's needed for the iterative positioning algorithm.

            @param _rAnchoredObj
            input parameter - anchored object, for which the condition has to checked.

            @param _nFromPageNum
            input parameter - number of the page, on which the check is performed

            @param _bAnchoredAtMasterBeforeFormatAnchor
            input parameter - boolean indicating, that the given anchored object
            was anchored at the master frame before the anchor frame has been
            formatted.

            @param _noToPageNum
            output parameter - number of page frame, the 'anchor' of the returned
            anchored object is.

            @param _boInFollow
            output parameter - boolean, indicating that anchor text frame is
            currently on the same page, but it's a follow of in a follow row,
            which will move forward. value only relevant, if method return <true>.

            @return boolean
            indicating, if 'anchor is moved forward'
        */
        static bool CheckMovedFwdCondition( SwAnchoredObject& _rAnchoredObj,
                                            const sal_uInt32 _nFromPageNum,
                                            const bool _bAnchoredAtMasterBeforeFormatAnchor,
                                            sal_uInt32& _noToPageNum,
                                            bool& _boInFollow );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
