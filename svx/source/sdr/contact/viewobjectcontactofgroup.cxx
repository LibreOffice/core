/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfGroup::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // check model-view visibility
            if(isPrimitiveVisible(rDisplayInfo))
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
                    xRetval = getPrimitive2DSequenceSubHierarchy(rDisplayInfo);

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
                }
                else
                {
                    // draw replacement object for group. This will use ViewContactOfGroup::createViewIndependentPrimitive2DSequence
                    // which creates the replacement primitives for an empty group
                    xRetval = ViewObjectContactOfSdrObj::getPrimitive2DSequenceHierarchy(rDisplayInfo);
                }
            }
            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
