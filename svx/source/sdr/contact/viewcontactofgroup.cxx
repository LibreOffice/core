/*************************************************************************
 *
 *  $RCSfile: viewcontactofgroup.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-11-24 16:45:24 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFGROUP_HXX
#include <svx/sdr/contact/viewcontactofgroup.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfGroup::CalcPaintRectangle()
        {
            maPaintRectangle = GetSdrObjGroup().GetCurrentBoundRect();
        }

        ViewContactOfGroup::ViewContactOfGroup(SdrObjGroup& rGroup)
        :   ViewContactOfSdrObj(rGroup)
        {
        }

        ViewContactOfGroup::~ViewContactOfGroup()
        {
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfGroup::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Do not paint groups themthelves only when they are empty
            if(!GetSdrObjGroup().GetSubList() || !GetSdrObjGroup().GetSubList()->GetObjCount())
            {
                // Paint empty group to get a replacement visualisation
                return sal_True;
            }

            return sal_False;
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfGroup::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            // Paint the object. If this is called, the group is empty.
            // Paint a replacement object.
            return PaintReplacementObject(rDisplayInfo, rPaintRectangle);
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
