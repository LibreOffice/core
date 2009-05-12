/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofmasterpagedescriptor.cxx,v $
 * $Revision: 1.15 $
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
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <vcl/timer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontactofmasterpagedescriptor.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfMasterPageDescriptor::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            return *(new ViewObjectContactOfMasterPageDescriptor(rObjectContact, *this));
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfMasterPageDescriptor::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrObject* pBackgroundCandidate = GetMasterPageDescriptor().GetBackgroundObject();

            if(pBackgroundCandidate)
            {
                // build primitive from pBackgroundCandidate's attributes
                const SfxItemSet& rFillProperties = pBackgroundCandidate->GetMergedItemSet();
                drawinglayer::attribute::SdrFillAttribute* pFill = drawinglayer::primitive2d::createNewSdrFillAttribute(rFillProperties);

                if(pFill)
                {
                    if(pFill->isVisible())
                    {
                        // direct model data is the page size, get and use it
                        const SdrPage& rOwnerPage = GetMasterPageDescriptor().GetOwnerPage();
                        const basegfx::B2DRange aInnerRange(
                            rOwnerPage.GetLftBorder(), rOwnerPage.GetUppBorder(),
                            rOwnerPage.GetWdt() - rOwnerPage.GetRgtBorder(), rOwnerPage.GetHgt() - rOwnerPage.GetLwrBorder());
                        const basegfx::B2DPolygon aInnerPolgon(basegfx::tools::createPolygonFromRect(aInnerRange));
                        const basegfx::B2DHomMatrix aEmptyTransform;
                        const drawinglayer::primitive2d::Primitive2DReference xReference(drawinglayer::primitive2d::createPolyPolygonFillPrimitive(
                            basegfx::B2DPolyPolygon(aInnerPolgon), aEmptyTransform, *pFill));

                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                    }

                    delete pFill;
                }
            }

            return xRetval;
        }

        // basic constructor
        ViewContactOfMasterPageDescriptor::ViewContactOfMasterPageDescriptor(sdr::MasterPageDescriptor& rDescriptor)
        :   ViewContact(),
            mrMasterPageDescriptor(rDescriptor)
        {
        }

        // The destructor.
        ViewContactOfMasterPageDescriptor::~ViewContactOfMasterPageDescriptor()
        {
        }

        sal_uInt32 ViewContactOfMasterPageDescriptor::GetObjectCount() const
        {
            sal_uInt32 nRetval(GetMasterPageDescriptor().GetUsedPage().GetObjCount());

            if(nRetval && GetMasterPageDescriptor().GetUsedPage().GetObj(0)->IsMasterPageBackgroundObject())
            {
                nRetval--;
            }

            return nRetval;
        }

        ViewContact& ViewContactOfMasterPageDescriptor::GetViewContact(sal_uInt32 nIndex) const
        {
            if(GetMasterPageDescriptor().GetUsedPage().GetObjCount() && GetMasterPageDescriptor().GetUsedPage().GetObj(0)->IsMasterPageBackgroundObject())
            {
                nIndex++;
            }

            return GetMasterPageDescriptor().GetUsedPage().GetObj(nIndex)->GetViewContact();
        }

        ViewContact* ViewContactOfMasterPageDescriptor::GetParentContact() const
        {
            return &(GetMasterPageDescriptor().GetOwnerPage().GetViewContact());
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
