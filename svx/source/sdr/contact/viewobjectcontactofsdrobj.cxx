/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <sdr/contact/objectcontactofpageview.hxx>
#include <sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <vcl/outdev.hxx>

#include "fmobj.hxx"

namespace sdr { namespace contact {

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

bool ViewObjectContactOfSdrObj::isPrimitiveVisibleOnAnyLayer(const SetOfByte& aLayers) const
{
    return aLayers.IsSet(getSdrObject().GetLayer());
}

bool ViewObjectContactOfSdrObj::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    const SdrObject& rObject = getSdrObject();

    // Test layer visibility
    if(!isPrimitiveVisibleOnAnyLayer(rDisplayInfo.GetProcessLayers()))
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
                if(static_cast<const SdrOle2Obj&>(rObject).IsChart())
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
                const bool bIsFormControl = dynamic_cast< const FmFormObj * >( &rObject ) != nullptr;
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

    // Check if this object is in the visible range.
    const drawinglayer::geometry::ViewInformation2D& rViewInfo = GetObjectContact().getViewInformation2D();
    basegfx::B2DRange aObjRange = GetViewContact().getRange(rViewInfo);
    if (!aObjRange.isEmpty())
    {
        const basegfx::B2DRange& rViewRange = rViewInfo.getViewport();
        bool bVisible = rViewRange.isEmpty() || rViewRange.overlaps(aObjRange);
        if (!bVisible)
            return false;
    }

    return true;
}

boost::optional<const OutputDevice&> ViewObjectContactOfSdrObj::getPageViewOutputDevice() const
{
    ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &GetObjectContact() );
    if ( pPageViewContact )
    {
        // if the PageWindow has a patched PaintWindow, use the original PaintWindow
        // this ensures that our control is _not_ re-created just because somebody
        // (temporarily) changed the window to paint onto.
        // #i72429# / 2007-02-20 / frank.schoenheit (at) sun.com
        SdrPageWindow& rPageWindow( pPageViewContact->GetPageWindow() );
        if ( rPageWindow.GetOriginalPaintWindow() )
            return rPageWindow.GetOriginalPaintWindow()->GetOutputDevice();

        return rPageWindow.GetPaintWindow().GetOutputDevice();
    }
    return boost::optional<const OutputDevice&>();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
