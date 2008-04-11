/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofe3dscene.cxx,v $
 * $Revision: 1.9 $
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
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/polysc3d.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfE3dScene::CalcPaintRectangle()
        {
            maPaintRectangle = GetE3dScene().GetCurrentBoundRect();
        }

        ViewContactOfE3dScene::ViewContactOfE3dScene(E3dScene& rScene)
        :   ViewContactOfSdrObj(rScene)
        {
        }

        ViewContactOfE3dScene::~ViewContactOfE3dScene()
        {
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfE3dScene::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& /*rAssociatedVOC*/)
        {
            // Test layer visibility, force to flat SdrObject here for 3D (!)
            if(!rDisplayInfo.GetProcessLayers().IsSet(GetSdrObject().SdrObject::GetLayer()))
            {
                return sal_False;
            }

            // Test area visibility
            const Region& rRedrawArea = rDisplayInfo.GetRedrawArea();

            if(!rRedrawArea.IsEmpty() && !rRedrawArea.IsOver(GetPaintRectangle()))
            {
                return sal_False;
            }

            // Test calc hide/draft features
            if(!DoPaintForCalc(rDisplayInfo))
            {
                return sal_False;
            }

            // Always paint E3dScenes
            return sal_True;
        }

        // These methods decide which parts of the objects will be painted:
        // When ShouldPaintDrawHierarchy() returns sal_True, the DrawHierarchy of the object is painted.
        // Else, the flags and rectangles of the VOCs of the sub-hierarchy are set to the values of the
        // object's VOC.
        sal_Bool ViewContactOfE3dScene::ShouldPaintDrawHierarchy(DisplayInfo& /*rDisplayInfo*/, const ViewObjectContact& /*rAssociatedVOC*/)
        {
            // 3D Scenes do draw their hierarchy themselves, so switch off
            // painting DrawHierarchy.
            return sal_False;
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfE3dScene::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& /*rAssociatedVOC*/)
        {
            sal_Bool bRetval(sal_False);

            if(GetE3dScene().GetSubList() && GetE3dScene().GetSubList()->GetObjCount())
            {
                // copy the saved original PaintMode from the DisplayInfo to the old
                // structures so that it is available for the old 3D rendering.
                rDisplayInfo.GetPaintInfoRec()->nOriginalDrawMode = rDisplayInfo.GetOriginalDrawMode();
                rDisplayInfo.GetPaintInfoRec()->bNotActive = rDisplayInfo.IsGhostedDrawModeActive();

                // Paint the 3D scene. Just hand over to the old Paint() ATM.
                GetSdrObject().DoPaintObject(
                    *rDisplayInfo.GetExtendedOutputDevice(),
                    *rDisplayInfo.GetPaintInfoRec());

                rPaintRectangle = GetPaintRectangle();
                bRetval = sal_True;
            }
            else
            {
                // Paint a replacement object.
                bRetval = PaintReplacementObject(rDisplayInfo, rPaintRectangle);
            }

            return bRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
