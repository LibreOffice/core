/*************************************************************************
 *
 *  $RCSfile: anchoreddrawobject.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: od $ $Date: 2004-08-03 05:50:32 $
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
        Rectangle maLastObjRect;

        // boolean, indicating that anchored drawing object hasn't been attached
        // to a anchor frame yet. Once, it is attached to a anchor frame the
        // boolean change its state.
        bool mbNotYetAttachedToAnchorFrame;

        /** method to determine positioning attributes as long as the anchored
            drawing object isn't attached to a anchor frame

            The positioning attributes are determined by the current object
            geometry.

            @author OD
        */
        void _SetPositioningAttr();

        /** method to set internal anchor position of <SdrObject> instance
            of the drawing according to its positioning alignments.

            For drawing objects the internal anchor position of the <SdrObject>
            instance has to be set. The <SdrObject> instance represents
            the drawing object in the drawing layer and is responsible for
            storing the position coordinates in the file format.
            The setting has to be performed according to its positioning
            alignments, in order to get the correct positioning coordinates
            saved in the file format.
            Note: This adjustment is not be done for as-character anchored
            drawing object - the positioning code takes care of this.

            @author OD
        */
        void _SetDrawObjAnchor( const Point _aOffsetToFrmAnchorPos );

        /** method to invalidate the given page frame

            OD 2004-07-02 #i28701#

            @author OD
        */
        void _InvalidatePage( SwPageFrm* _pPageFrm );

    protected:

        /** method to indicate, that anchored drawing object is attached to
            a anchor frame

            @author OD
        */
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
        const Rectangle& GetLastObjRect() const;
        Rectangle& LastObjRect();

        /** adjust positioning and alignment attributes for new anchor frame

            OD 2004-04-21
            Set horizontal and vertical position/alignment to manual position
            relative to anchor frame area using the anchor position of the
            new anchor frame and the current absolute drawing object position.
            Note: For correct Undo/Redo method should only be called inside a
            Undo-/Redo-action.

            @author OD
        */
        void AdjustPositioningAttr( const SwFrm* _pNewAnchorFrm );

        /** method to set positioning attributes as long as the anchored drawing
            object isn't attached to a anchor frame

            @author OD
        */
        inline void SetPositioningAttr()
        {
            if ( mbNotYetAttachedToAnchorFrame )
            {
                _SetPositioningAttr();
            }
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
