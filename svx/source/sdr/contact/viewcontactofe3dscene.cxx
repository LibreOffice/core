/*************************************************************************
 *
 *  $RCSfile: viewcontactofe3dscene.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:40:41 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFE3DSCENE_HXX
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#endif

#ifndef _E3D_POLYSC3D_HXX
#include <polysc3d.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

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
        sal_Bool ViewContactOfE3dScene::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
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
        sal_Bool ViewContactOfE3dScene::ShouldPaintDrawHierarchy(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // 3D Scenes do draw their hierarchy themselves, so switch off
            // painting DrawHierarchy.
            return sal_False;
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfE3dScene::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);

            if(GetE3dScene().GetSubList() && GetE3dScene().GetSubList()->GetObjCount())
            {
                if(DoDraftForCalc(rDisplayInfo))
                {
                    bRetval = PaintCalcDraftObject(rDisplayInfo, rPaintRectangle);
                }
                else
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
