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
#ifndef _OBJECTFORMATTER_HXX
#define _OBJECTFORMATTER_HXX

#include <sal/types.h>

class SwFrm;
// #i26945#
class SwTxtFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwAnchoredObject;
class SwLayAction;
// OD 2004-10-04 #i26945#
class SwPageNumAndTypeOfAnchors;

// -----------------------------------------------------------------------------
// #i28701#
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
        // #i26945#
        SwPageNumAndTypeOfAnchors* mpPgNumAndTypeOfAnchors;

        /** helper method for method <_FormatObj(..)> - performs the intrinsic
            format of the layout of the given layout frame and all its lower
            layout frames.

            #i28701#
            IMPORTANT NOTE:
            Method corresponds to methods <SwLayAction::FormatLayoutFly(..)> and
            <SwLayAction::FormatLayout(..)>. Thus, its code for the formatting have
            to be synchronised.
        */
        void _FormatLayout( SwLayoutFrm& _rLayoutFrm );

        /** helper method for method <_FormatObj(..)> - performs the intrinsic
            format of the content of the given floating screen object.

            #i28701#
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

            #i28701#
        */
        void _FormatObj( SwAnchoredObject& _rAnchoredObj );

        /** invokes the intrinsic format method for all floating screen objects,
            anchored at anchor frame on the given page frame

            #i28701#
            #i26945# - for format of floating screen objects for
            follow text frames, the 'master' text frame is passed to the method.
            Thus, the objects, whose anchor character is inside the follow text
            frame can be formatted.

            @param _pMasterTxtFrm
            input parameter - pointer to 'master' text frame. default value: NULL
        */
        bool _FormatObjsAtFrm( SwTxtFrm* _pMasterTxtFrm = 0L );

        /** accessor to collected anchored object

            #i28701#
        */
        SwAnchoredObject* GetCollectedObj( const sal_uInt32 _nIndex );

        /** accessor to 'anchor' page number of collected anchored object

            #i28701#
        */
        sal_uInt32 GetPgNumOfCollected( const sal_uInt32 _nIndex );

        /** accessor to 'anchor' type of collected anchored object

            #i26945#
        */
        bool IsCollectedAnchoredAtMaster( const sal_uInt32 _nIndex );

        /** accessor to total number of collected anchored objects

            #i28701#
        */
        sal_uInt32 CountOfCollected();

    public:
        virtual ~SwObjectFormatter();

        /** intrinsic method to format a certain floating screen object

            #i40147# - add parameter <_bCheckForMovedFwd>

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
        */
        virtual bool DoFormatObjs() = 0;

        /** method to format all floating screen objects at the given anchor frame
        */
        static bool FormatObjsAtFrm( SwFrm& _rAnchorFrm,
                                     const SwPageFrm& _rPageFrm,
                                     SwLayAction* _pLayAction = 0L );

        /** method to format a given floating screen object
        */
        static bool FormatObj( SwAnchoredObject& _rAnchoredObj,
                               SwFrm* _pAnchorFrm = 0L,
                               const SwPageFrm* _pPageFrm = 0L,
                               SwLayAction* _pLayAction = 0L );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
