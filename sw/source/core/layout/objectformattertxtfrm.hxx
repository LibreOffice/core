/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectformattertxtfrm.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-17 16:34:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _OBJECTFORMATTERTXTFRM_HXX
#define _OBJECTFORMATTERTXTFRM_HXX

#ifndef _OBJECTFORMATTER_HXX
#include <objectformatter.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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

        /** method to check the conditions, if 'anchor is moved forward'

            OD 2004-10-11 #i26945#
            OD 2005-03-30 #i43913# - add output parameter <_boInFollow>

            @author OD

            @param _nIdxOfCollected
            input parameter - index of collected anchored object, for which the
            conditions have to be checked.

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
        bool _CheckMovedFwdCondition( const sal_uInt32 _nIdxOfCollected,
                                      sal_uInt32& _noToPageNum,
                                      bool& _boInFollow );

        /** method to format the anchor frame for checking of the move forward condition

            OD 2005-01-11 #i40141#

            @author OD
        */
        void _FormatAnchorFrmForCheckMoveFwd();

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
};

#endif
