/*************************************************************************
 *
 *  $RCSfile: anchoreddrawobject.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-27 12:29:49 $
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
#ifndef _ANCHOREDDRAWOBJECT_HXX
#define _ANCHOREDDRAWOBJECT_HXX

#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

/** class for the positioning of drawing objects

    OD 2004-03-25 #i26791#

    @author OD
*/
class SwAnchoredDrawObject : public SwAnchoredObject
{
    private:
        // boolean, indicating that the object position has been invalidated
        // and that a positioning has to be performed.
        bool mbValidPos;

        // rectangle, keeping the last object rectangle after the postioning
        // --> OD 2004-09-29 #i34748# - change <maLastObjRect> to a pointer
        Rectangle* mpLastObjRect;

        // boolean, indicating that anchored drawing object hasn't been attached
        // to a anchor frame yet. Once, it is attached to a anchor frame the
        // boolean changes its state.
        bool mbNotYetAttachedToAnchorFrame;

        // --> OD 2004-08-09 #i28749# - boolean, indicating that anchored
        // drawing object hasn't been positioned yet. Once, it's positioned the
        // boolean changes its state.
        bool mbNotYetPositioned;

        /** method for the intrinsic positioning of a at-paragraph|at-character
            anchored drawing object

            OD 2004-08-12 #i32795# - helper method for method <MakeObjPos>

            @author OD
        */
        void _MakeObjPosAnchoredAtPara();

        /** method for the intrinsic positioning of a at-page|at-frame anchored
            drawing object

            OD 2004-08-12 #i32795# - helper method for method <MakeObjPos>

            @author OD
        */
        void _MakeObjPosAnchoredAtLayout();

            /** method to set positioning attributes (not for as-character anchored)

            OD 2004-10-20 #i35798#
            During load the positioning attributes aren't set.
            Thus, the positioning attributes are set by the current object geometry.
            This method is also used for the conversion for drawing objects
            (not anchored as-character) imported from OpenOffice.org file format
            once and directly before the first positioning.

            @author OD
        */
        void _SetPositioningAttr();

        /** method to set internal anchor position of <SdrObject> instance
            of the drawing object

            For drawing objects the internal anchor position of the <SdrObject>
            instance has to be set.
            Note: This adjustment is not be done for as-character anchored
            drawing object - the positioning code takes care of this.
            OD 2004-07-29 #i31698# - API for drawing objects in Writer has
            been adjusted. Thus, this method will only set the internal anchor
            position of the <SdrObject> instance to the anchor position given
            by its anchor frame.

            @author OD
        */
        void _SetDrawObjAnchor();

        /** method to invalidate the given page frame

            OD 2004-07-02 #i28701#

            @author OD
        */
        void _InvalidatePage( SwPageFrm* _pPageFrm );

    protected:
        virtual void ObjectAttachedToAnchorFrame();

        /** method to assure that anchored object is registered at the correct
            page frame

            OD 2004-07-02 #i28701#

            @author OD
        */
        virtual void RegisterAtCorrectPage();

    public:
        TYPEINFO();

        SwAnchoredDrawObject();
        virtual ~SwAnchoredDrawObject();

        // declaration of pure virtual methods of base class <SwAnchoredObject>
        virtual void MakeObjPos();
        virtual void InvalidateObjPos();

        // accessors to the format
        virtual SwFrmFmt& GetFrmFmt();
        virtual const SwFrmFmt& GetFrmFmt() const;

        // accessors to the object area and its position
        virtual const SwRect GetObjRect() const;
        virtual void SetObjTop( const SwTwips _nTop);
        virtual void SetObjLeft( const SwTwips _nLeft);
        // --> OD 2004-09-29 #i34748# - change return type to a pointer.
        // Return value can be NULL.
        const Rectangle* GetLastObjRect() const;
        // <--
        // --> OD 2004-09-29 #i34748# - change method
        void SetLastObjRect( const Rectangle& _rNewObjRect );
        // <--

        /** adjust positioning and alignment attributes for new anchor frame

            OD 2004-04-21
            Set horizontal and vertical position/alignment to manual position
            relative to anchor frame area using the anchor position of the
            new anchor frame and the current absolute drawing object position.
            Note: For correct Undo/Redo method should only be called inside a
            Undo-/Redo-action.
            OD 2004-08-24 #i33313# - add second optional parameter <_pNewObjRect>

            @author OD

            @param <_pNewAnchorFrm>
            input parameter - new anchor frame for the anchored object.

            @param <_pNewObjRect>
            optional input parameter - proposed new object rectangle. If not
            provided the current object rectangle is taken.
        */
        void AdjustPositioningAttr( const SwFrm* _pNewAnchorFrm,
                                    const SwRect* _pNewObjRect = 0L );

        /** anchored drawing object not yet attached to a anchor frame

            OD 2004-08-04 #i31698#

            @author OD
        */
        inline bool NotYetAttachedToAnchorFrm() const
        {
            return mbNotYetAttachedToAnchorFrame;
        }

        /** method to notify background of drawing object

            OD 2004-06-30 #i28701#

            @author OD
        */
        virtual void NotifyBackground( SwPageFrm* _pPageFrm,
                                       const SwRect& _rRect,
                                       PrepareHint _eHint );
};

#endif
