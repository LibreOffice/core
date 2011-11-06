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


#ifndef _OBJECTFORMATTER_HXX
#define _OBJECTFORMATTER_HXX

#include <sal/types.h>

class SwFrm;
// --> OD 2004-10-08 #i26945#
class SwTxtFrm;
// <--
class SwLayoutFrm;
class SwPageFrm;
class SwAnchoredObject;
class SwLayAction;
// --> OD 2004-10-04 #i26945#
class SwPageNumAndTypeOfAnchors;
// <--

// -----------------------------------------------------------------------------
// OD 2004-06-25 #i28701#
// Format floating screen objects, which are anchored at the given anchor frame
// and registered at the given page frame.
// -----------------------------------------------------------------------------
class SwObjectFormatter
{
    private:
        // page frame, at which the floating screen objects are registered.
        const SwPageFrm& mrPageFrm;

        // boolean, indicating that only as-character anchored objects have to
        // be formatted.
        bool mbFormatOnlyAsCharAnchored;

        // value of document compatibility option 'Consider wrapping style on
        // object positioning'
        const bool mbConsiderWrapOnObjPos;

        // layout action calling the format of the floating screen objects
        SwLayAction* mpLayAction;

        // data structure to collect page number of object's 'anchor'
        // --> OD 2004-10-04 #i26945#
        SwPageNumAndTypeOfAnchors* mpPgNumAndTypeOfAnchors;
        // <--

        /** helper method for method <_FormatObj(..)> - performs the intrinsic
            format of the layout of the given layout frame and all its lower
            layout frames.

            OD 2004-06-28 #i28701#
            IMPORTANT NOTE:
            Method corresponds to methods <SwLayAction::FormatLayoutFly(..)> and
            <SwLayAction::FormatLayout(..)>. Thus, its code for the formatting have
            to be synchronised.

            @author OD
        */
        void _FormatLayout( SwLayoutFrm& _rLayoutFrm );

        /** helper method for method <_FormatObj(..)> - performs the intrinsic
            format of the content of the given floating screen object.

            OD 2004-06-28 #i28701#

            @author OD
        */
        void _FormatObjCntnt( SwAnchoredObject& _rAnchoredObj );

    protected:
        SwObjectFormatter( const SwPageFrm& _rPageFrm,
                           SwLayAction* _pLayAction = 0L,
                           const bool _bCollectPgNumOfAnchors = false );

        static SwObjectFormatter* CreateObjFormatter( SwFrm& _rAnchorFrm,
                                                      const SwPageFrm& _rPageFrm,
                                                      SwLayAction* _pLayAction );

        virtual SwFrm& GetAnchorFrm() = 0;

        inline const SwPageFrm& GetPageFrm() const
        {
            return mrPageFrm;
        }

        inline bool ConsiderWrapOnObjPos() const
        {
            return mbConsiderWrapOnObjPos;
        }

        inline SwLayAction* GetLayAction()
        {
            return mpLayAction;
        }

        /** method to restrict the format of floating screen objects to
            as-character anchored ones

            @author OD
        */
        inline void SetFormatOnlyAsCharAnchored()
        {
            mbFormatOnlyAsCharAnchored = true;
        }

        inline bool FormatOnlyAsCharAnchored() const
        {
            return mbFormatOnlyAsCharAnchored;
        }

        /** performs the intrinsic format of a given floating screen object and its content.

            OD 2004-06-28 #i28701#

            @author OD
        */
        void _FormatObj( SwAnchoredObject& _rAnchoredObj );

        /** invokes the intrinsic format method for all floating screen objects,
            anchored at anchor frame on the given page frame

            OD 2004-06-28 #i28701#
            OD 2004-10-08 #i26945# - for format of floating screen objects for
            follow text frames, the 'master' text frame is passed to the method.
            Thus, the objects, whose anchor character is inside the follow text
            frame can be formatted.

            @author OD

            @param _pMasterTxtFrm
            input parameter - pointer to 'master' text frame. default value: NULL
        */
        bool _FormatObjsAtFrm( SwTxtFrm* _pMasterTxtFrm = 0L );

        /** accessor to collected anchored object

            OD 2004-07-05 #i28701#

            @author OD
        */
        SwAnchoredObject* GetCollectedObj( const sal_uInt32 _nIndex );

        /** accessor to 'anchor' page number of collected anchored object

            OD 2004-07-05 #i28701#

            @author OD
        */
        sal_uInt32 GetPgNumOfCollected( const sal_uInt32 _nIndex );

        /** accessor to 'anchor' type of collected anchored object

            OD 2004-10-04 #i26945#

            @author OD
        */
        bool IsCollectedAnchoredAtMaster( const sal_uInt32 _nIndex );

        /** accessor to total number of collected anchored objects

            OD 2004-07-05 #i28701#

            @author OD
        */
        sal_uInt32 CountOfCollected();

    public:
        virtual ~SwObjectFormatter();

        /** intrinsic method to format a certain floating screen object

            OD 2005-01-10 #i40147# - add parameter <_bCheckForMovedFwd>

            @author OD

            @param _rAnchoredObj
            input parameter - anchored object, which have to be formatted.

            @param _bCheckForMovedFwd
            input parameter - boolean indicating, that after a successful
            format of the anchored object the anchor frame has to be checked,
            if it would moved forward due to the positioning of the anchored object.
            default value: false
            value only considered, if wrapping style influence has to be
            considered for the positioning of the anchored object.
        */
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                  const bool _bCheckForMovedFwd = false ) = 0;

        /** intrinsic method to format all floating screen objects

            @author OD
        */
        virtual bool DoFormatObjs() = 0;

        /** method to format all floating screen objects at the given anchor frame

            @author OD
        */
        static bool FormatObjsAtFrm( SwFrm& _rAnchorFrm,
                                     const SwPageFrm& _rPageFrm,
                                     SwLayAction* _pLayAction = 0L );

        /** method to format a given floating screen object

            @author OD
        */
        static bool FormatObj( SwAnchoredObject& _rAnchoredObj,
                               SwFrm* _pAnchorFrm = 0L,
                               const SwPageFrm* _pPageFrm = 0L,
                               SwLayAction* _pLayAction = 0L );
};

#endif
