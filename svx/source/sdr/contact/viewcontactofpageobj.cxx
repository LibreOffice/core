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

#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svx/svdopage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontactofpageobj.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

namespace sdr { namespace contact {

ViewObjectContact& ViewContactOfPageObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
    ViewObjectContact* pRetval = new ViewObjectContactOfPageObj(rObjectContact, *this);
    return *pRetval;
}

ViewContactOfPageObj::ViewContactOfPageObj(SdrPageObj& rPageObj)
:   ViewContactOfSdrObj(rPageObj)
{
}

ViewContactOfPageObj::~ViewContactOfPageObj()
{
}

// #i35972# React on changes of the object of this ViewContact
void ViewContactOfPageObj::ActionChanged()
{
    static bool bIsInActionChange(false);

    if(!bIsInActionChange)
    {
        // set recursion flag, see description in *.hxx
        bIsInActionChange = true;

        // call parent
        ViewContactOfSdrObj::ActionChanged();

        // reset recursion flag, see description in *.hxx
        bIsInActionChange = false;
    }
}

drawinglayer::primitive2d::Primitive2DContainer ViewContactOfPageObj::createViewIndependentPrimitive2DSequence() const
{
    // create graphical visualisation data. Since this is the view-independent version which should not be used,
    // create a replacement graphic visualisation here. Use GetLastBoundRect to access the model data directly
    // which is aOutRect for SdrPageObj.
    const Rectangle aModelRectangle(GetPageObj().GetLastBoundRect());
    const basegfx::B2DRange aModelRange(aModelRectangle.Left(), aModelRectangle.Top(), aModelRectangle.Right(), aModelRectangle.Bottom());
    const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aModelRange));
    const basegfx::BColor aYellow(1.0, 1.0, 0.0);
    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aYellow));

    return drawinglayer::primitive2d::Primitive2DContainer { xReference };
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
