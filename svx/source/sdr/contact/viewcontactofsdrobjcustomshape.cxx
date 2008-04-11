/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrobjcustomshape.cxx,v $
 * $Revision: 1.6 $
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
#include <svx/sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrObjCustomShape::ViewContactOfSdrObjCustomShape(SdrObjCustomShape& rCustomShape)
        :   ViewContactOfTextObj(rCustomShape)
        {
        }

        ViewContactOfSdrObjCustomShape::~ViewContactOfSdrObjCustomShape()
        {
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfSdrObjCustomShape::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            // copy the saved original PaintMode from the DisplayInfo to the old
            // structures so that it is available for the old 3D rendering.
            rDisplayInfo.GetPaintInfoRec()->nOriginalDrawMode = rDisplayInfo.GetOriginalDrawMode();
            rDisplayInfo.GetPaintInfoRec()->bNotActive = rDisplayInfo.IsGhostedDrawModeActive();

            return ViewContactOfTextObj::PaintObject( rDisplayInfo, rPaintRectangle, rAssociatedVOC );
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
