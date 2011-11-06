/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _OBJECTFORMATTERTXTFRM_HXX
#define _OBJECTFORMATTERTXTFRM_HXX

#include <objectformatter.hxx>
#include <sal/types.h>

class SwTxtFrm;

// -----------------------------------------------------------------------------
// OD 2004-07-01 #i28701#
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

        /* method to invalidate objects, anchored previous to given object at
           the anchor text frame

            @param _rAnchoredObj
            reference to anchored object - objects, anchored previous to
            this one will be invalidated.

            @author OD
        */
        void _InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj );

        /* method to invalidate objects, anchored after the given object at
           the page frame

            @param _rAnchoredObj
            reference to anchored object - objects, anchored after this one will
            be invalidated.

            @param _bInclObj
            boolean indicates, if given anchored object <_rAnchoredObj> also have
            to be invalidated.

            @author OD
        */
        void _InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj,
                                    const bool _bInclObj );

        /** method to determine first anchored object, whose 'anchor is moved
            forward'.

            'anchor (of an object) is moved forward', if the anchor frame
            respectively the anchor character of the object isn't on the
            proposed page frame. Instead its on a following page
            OD 2004-10-04 #i26945# - For at-character anchored objects,
            it has also to be checked, if the anchor character is in a follow
            text frame, which would move to the next page.
            OD 2005-03-30 #i43913# - add output parameter <_boInFollow>

            @author OD

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

            OD 2005-01-11 #i40141#

            @author OD
        */
        void _FormatAnchorFrmForCheckMoveFwd();

        /** method to determine if at least one anchored object has state
            <temporarly consider wrapping style influence> set.

            OD 2006-07-24 #b6449874#

            @author OD
        */
        bool _AtLeastOneObjIsTmpConsiderWrapInfluence();

    protected:

        virtual SwFrm& GetAnchorFrm();

    public:
        virtual ~SwObjectFormatterTxtFrm();

        // --> OD 2005-01-10 #i40147# - add parameter <_bCheckForMovedFwd>.
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                  const bool _bCheckForMovedFwd = false );
        // <--
        virtual bool DoFormatObjs();

        /** method to create an instance of <SwObjectFormatterTxtFrm> is
            necessary.

            @author OD
        */
        static SwObjectFormatterTxtFrm* CreateObjFormatter(
                                                SwTxtFrm& _rAnchorTxtFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction );

        /** method to format given anchor text frame and its previous frames

            OD 2005-11-17 #i56300#
            Usage: Needed to check, if the anchor text frame is moved forward
            due to the positioning and wrapping of its anchored objects, and
            to format the frames, which have become invalid due to the anchored
            object formatting in the iterative object positioning algorithm

            @author OD

            @param _rAnchorTxtFrm
            input parameter - reference to anchor text frame, which has to be
            formatted including its previous frames of the page.
        */
        static void FormatAnchorFrmAndItsPrevs( SwTxtFrm& _rAnchorTxtFrm );

        /** method to check the conditions, if 'anchor is moved forward'

            OD 2004-10-11 #i26945#
            OD 2005-03-30 #i43913# - add output parameter <_boInFollow>
            OD 2006-01-27 #i58182# - replace method by a corresponding static
            method, because it's needed for the iterative positioning algorithm.

            @author OD

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
