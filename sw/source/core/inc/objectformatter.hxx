/*************************************************************************
 *
 *  $RCSfile: objectformatter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:06:22 $
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
#ifndef _OBJECTFORMATTER_HXX
#define _OBJECTFORMATTER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class SwFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwAnchoredObject;
class SwLayAction;
class SwPageNumOfAnchors;

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
        SwPageNumOfAnchors* mpPgNumOfAnchors;

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
        const SwPageFrm& GetPageFrm() const;
        const bool FormatOnlyAsCharAnchored() const;
        const bool ConsiderWrapOnObjPos() const;
        SwLayAction* GetLayAction();

        /** method to restrict the format of floating screen objects to
            as-character anchored ones

            @author OD
        */
        void SetFormatOnlyAsCharAnchored();

        /** performs the intrinsic format of a given floating screen object and its content.

            OD 2004-06-28 #i28701#

            @author OD
        */
        void _FormatObj( SwAnchoredObject& _rAnchoredObj );

        /** invokes the intrinsic format method for all floating screen objects,
            anchored at anchor frame on the given page frame

            OD 2004-06-28 #i28701#

            @author OD
        */
        bool _FormatObjsAtFrm();

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

        /** accessor to total number of collected anchored objects

            OD 2004-07-05 #i28701#

            @author OD
        */
        sal_uInt32 CountOfCollected();

    public:
        virtual ~SwObjectFormatter();

        /** intrinsic method to format a certain floating screen object

            @author OD
        */
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj ) = 0;

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

        /** method to format all as-character anchored floating screen objects
            at the given anchor frame

            @author OD
        */
        static bool FormatAsCharAnchoredObjsAtFrm( SwFrm& _rAnchorFrm,
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
