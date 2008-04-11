/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofgroup.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/viewcontactofgroup.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>

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
