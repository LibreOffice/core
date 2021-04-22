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
#ifndef INCLUDED_SW_SOURCE_CORE_LAYOUT_OBJECTFORMATTERTXTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_LAYOUT_OBJECTFORMATTERTXTFRM_HXX

#include <objectformatter.hxx>
#include <sal/types.h>

class SwTextFrame;

// #i28701#
// Format floating screen objects, which are anchored at a given anchor text frame
// and registered at the given page frame.
class SwObjectFormatterTextFrame : public SwObjectFormatter
{
    private:
        // anchor text frame
        SwTextFrame& mrAnchorTextFrame;

        // 'master' anchor text frame
        SwTextFrame* const mpMasterAnchorTextFrame;

        SwObjectFormatterTextFrame( SwTextFrame& _rAnchorTextFrame,
                                 const SwPageFrame& _rPageFrame,
                                 SwTextFrame* _pMasterAnchorTextFrame,
                                 SwLayAction* _pLayAction );

        /** method to invalidate objects, anchored previous to given object at
           the anchor text frame

            @param _rAnchoredObj
            reference to anchored object - objects, anchored previous to
            this one will be invalidated.
        */
        void InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj );

        /** method to invalidate objects, anchored after the given object at
           the page frame

            @param _rAnchoredObj
            reference to anchored object - objects, anchored after this one will
            be invalidated.
        */
        void InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj );

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
        SwAnchoredObject* GetFirstObjWithMovedFwdAnchor(
                                    const sal_Int16 _nWrapInfluenceOnPosition,
                                    sal_uInt32& _noToPageNum,
                                    bool& _boInFollow,
                                    bool& o_rbPageHasFlysAnchoredBelowThis);

        /** method to format the anchor frame for checking of the move forward condition

            #i40141#
        */
        void FormatAnchorFrameForCheckMoveFwd();

        /** method to determine if at least one anchored object has state
            <temporarily consider wrapping style influence> set.
        */
        bool AtLeastOneObjIsTmpConsiderWrapInfluence();

    protected:

        virtual SwFrame& GetAnchorFrame() override;

    public:
        virtual ~SwObjectFormatterTextFrame() override;

        // #i40147# - add parameter <_bCheckForMovedFwd>.
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                  const bool _bCheckForMovedFwd = false ) override;
        virtual bool DoFormatObjs() override;

        /** method to create an instance of <SwObjectFormatterTextFrame> is
            necessary.
        */
        static std::unique_ptr<SwObjectFormatterTextFrame> CreateObjFormatter(
                                                SwTextFrame& _rAnchorTextFrame,
                                                const SwPageFrame& _rPageFrame,
                                                SwLayAction* _pLayAction );

        /** method to format given anchor text frame and its previous frames

            #i56300#
            Usage: Needed to check, if the anchor text frame is moved forward
            due to the positioning and wrapping of its anchored objects, and
            to format the frames, which have become invalid due to the anchored
            object formatting in the iterative object positioning algorithm

            @param _rAnchorTextFrame
            input parameter - reference to anchor text frame, which has to be
            formatted including its previous frames of the page.
        */
        static void FormatAnchorFrameAndItsPrevs( SwTextFrame& _rAnchorTextFrame );

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
            @param o_rbPageHasFlysAnchoredBelowThis
            output parameter - indicates that the page has flys anchored
            somewhere below the anchor of the passed _rAnchoredObj

            @return boolean
            indicating, if 'anchor is moved forward'
        */
        static bool CheckMovedFwdCondition( SwAnchoredObject& _rAnchoredObj,
                                            const sal_uInt32 _nFromPageNum,
                                            const bool _bAnchoredAtMasterBeforeFormatAnchor,
                                            sal_uInt32& _noToPageNum,
                                            bool& _boInFollow,
                                            bool& o_rbPageHasFlysAnchoredBelowThis);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
