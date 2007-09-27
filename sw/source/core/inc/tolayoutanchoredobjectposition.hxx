 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tolayoutanchoredobjectposition.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:59:26 $
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
#ifndef _TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#define _TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#ifndef _ANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif

namespace objectpositioning
{
    class SwToLayoutAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
            // calculated data for object position type TO_LAYOUT
            Point       maRelPos;

            // --> OD 2004-06-17 #i26791#
            // determine offset to frame anchor position according to the
            // positioning alignments
            Point maOffsetToFrmAnchorPos;

        public:
            SwToLayoutAnchoredObjectPosition( SdrObject& _rDrawObj );
            virtual ~SwToLayoutAnchoredObjectPosition();

            /** calculate position for object

                OD 30.07.2003 #110978#

                @author OD
            */
            virtual void CalcPosition();

            /** calculated relative position for object

                @author OD
            */
            Point GetRelPos() const;
    };
} // namespace objectpositioning

#endif
