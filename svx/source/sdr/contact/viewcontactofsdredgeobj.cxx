/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdredgeobj.cxx,v $
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

#include <svx/sdr/contact/viewcontactofsdredgeobj.hxx>
#include <svx/svdoedge.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svx/sdr/primitive2d/sdrconnectorprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfSdrEdgeObj::ViewContactOfSdrEdgeObj(SdrEdgeObj& rEdgeObj)
        :   ViewContactOfTextObj(rEdgeObj)
        {
        }

        ViewContactOfSdrEdgeObj::~ViewContactOfSdrEdgeObj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrEdgeObj::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            ::basegfx::B2DPolygon aEdgeTrack(GetEdgeObj().getEdgeTrack());

            // base visualisation on EdgeTrack
            if(aEdgeTrack.count())
            {
                // ckeck attributes
                const SfxItemSet& rItemSet = GetEdgeObj().GetMergedItemSet();
                SdrText* pSdrText = GetEdgeObj().getText(0);

                if(pSdrText)
                {
                    drawinglayer::attribute::SdrLineShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrLineShadowTextAttribute(rItemSet, *pSdrText);

                    if(pAttribute)
                    {
                        if(pAttribute->isVisible())
                        {
                            // create primitive
                            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrConnectorPrimitive2D(*pAttribute, aEdgeTrack));
                            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                        }

                        delete pAttribute;
                    }
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
