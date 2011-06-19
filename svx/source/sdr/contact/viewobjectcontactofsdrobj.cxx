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

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>

#include "fmobj.hxx"

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        const SdrObject& ViewObjectContactOfSdrObj::getSdrObject() const
        {
            return static_cast< ViewContactOfSdrObj& >(GetViewContact()).GetSdrObject();
        }

        ViewObjectContactOfSdrObj::ViewObjectContactOfSdrObj(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContact(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfSdrObj::~ViewObjectContactOfSdrObj()
        {
        }

        bool ViewObjectContactOfSdrObj::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
        {
            const SdrObject& rObject = getSdrObject();

            // Test layer visibility
            if(!rDisplayInfo.GetProcessLayers().IsSet(rObject.GetLayer()))
            {
                return false;
            }

            if(GetObjectContact().isOutputToPrinter() )
            {
                // Test if print output but not printable
                if( !rObject.IsPrintable())
                    return false;
            }
            else
            {
                // test is object is not visible on screen
                if( !rObject.IsVisible() )
                    return false;
            }

            // Test for hidden object on MasterPage
            if(rDisplayInfo.GetSubContentActive() && rObject.IsNotVisibleAsMaster())
            {
                return false;
            }

            // Test for Calc object hiding (for OLE and Graphic it's extra, see there)
            const SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

            if(pSdrPageView)
            {
                const SdrView& rSdrView = pSdrPageView->GetView();
                const bool bHideOle(rSdrView.getHideOle());
                const bool bHideChart(rSdrView.getHideChart());
                const bool bHideDraw(rSdrView.getHideDraw());
                const bool bHideFormControl(rSdrView.getHideFormControl());

                if(bHideOle || bHideChart || bHideDraw || bHideFormControl)
                {
                    if(OBJ_OLE2 == rObject.GetObjIdentifier())
                    {
                        if(((SdrOle2Obj&)rObject).IsChart())
                        {
                            // chart
                            if(bHideChart)
                            {
                                return false;
                            }
                        }
                        else
                        {
                            // OLE
                            if(bHideOle)
                            {
                                return false;
                            }
                        }
                    }
                    else if(OBJ_GRAF == rObject.GetObjIdentifier())
                    {
                        // graphic handled like OLE
                        if(bHideOle)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        const bool bIsFormControl = dynamic_cast< const FmFormObj * >( &rObject ) != 0;
                        if(bIsFormControl && bHideFormControl)
                        {
                            return false;
                        }
                        // any other draw object
                        if(!bIsFormControl && bHideDraw)
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
