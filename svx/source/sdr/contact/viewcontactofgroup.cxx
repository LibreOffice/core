/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofgroup.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:35:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
        sal_Bool ViewContactOfGroup::ShouldPaintObject(DisplayInfo& /*rDisplayInfo*/, const ViewObjectContact& /*rAssociatedVOC*/)
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
        sal_Bool ViewContactOfGroup::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& /*rAssociatedVOC*/)
        {
            // Paint the object. If this is called, the group is empty.
            // Paint a replacement object.
            return PaintReplacementObject(rDisplayInfo, rPaintRectangle);
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
