/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewobjectcontactofgroup.cxx,v $
 *
 * $Revision: 1.2 $
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

#include <svx/sdr/contact/viewobjectcontactofgroup.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfGroup::ViewObjectContactOfGroup(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfGroup::~ViewObjectContactOfGroup()
        {
        }

        bool ViewObjectContactOfGroup::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
        {
            const bool bHasChildren(0 != GetViewContact().GetObjectCount());

            if(!bHasChildren && (GetObjectContact().isOutputToPrinter() || GetObjectContact().isOutputToRecordingMetaFile()))
            {
                // empty group uses fallback gray empty frame display. Do neither print nor PDF export it
                return false;
            }

            // call parent
            return ViewObjectContactOfSdrObj::isPrimitiveVisible(rDisplayInfo);
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfGroup::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

            if(nSubHierarchyCount)
            {
                const sal_Bool bDoGhostedDisplaying(
                    GetObjectContact().DoVisualizeEnteredGroup()
                    && !GetObjectContact().isOutputToPrinter()
                    && GetObjectContact().getActiveViewContact() == &GetViewContact());

                if(bDoGhostedDisplaying)
                {
                    rDisplayInfo.ClearGhostedDrawMode();
                }

                // create object hierarchy
                drawinglayer::primitive2d::Primitive2DSequence xRetval(getPrimitive2DSequenceSubHierarchy(rDisplayInfo));

                if(xRetval.hasElements())
                {
                    // get ranges
                    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                    const ::basegfx::B2DRange aObjectRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xRetval, rViewInformation2D));
                    const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());

                    // check geometrical visibility
                    if(!aViewRange.isEmpty() && !aViewRange.overlaps(aObjectRange))
                    {
                        // not visible, release
                        xRetval.realloc(0);
                    }
                }

                if(bDoGhostedDisplaying)
                {
                    rDisplayInfo.SetGhostedDrawMode();
                }

                return xRetval;
            }
            else
            {
                // draw replacement object for group. This will use ViewContactOfGroup::createViewIndependentPrimitive2DSequence
                // which creates the replacement primitives for an empty group
                return ViewObjectContactOfSdrObj::getPrimitive2DSequenceHierarchy(rDisplayInfo);
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
