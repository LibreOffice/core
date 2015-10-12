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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <svx/svdoole2.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

class Graphic;

namespace sdr { namespace contact {

class ViewContactOfSdrOle2Obj : public ViewContactOfSdrRectObj
{
private:
    // #i123539# allow local buffering of chart data (if chart)
    drawinglayer::primitive2d::Primitive2DReference mxChartContent;

protected:
    // Create a Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something.
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

public:
    // access to SdrOle2Obj
    const SdrOle2Obj& GetOle2Obj() const
    {
        return static_cast<const SdrOle2Obj&>(GetSdrObject());
    }

    /// helper to create transformation from SdrObject
    basegfx::B2DHomMatrix createObjectTransform() const;

    // basic constructor, used from SdrObject.
    explicit ViewContactOfSdrOle2Obj(SdrOle2Obj& rOle2Obj);
    virtual ~ViewContactOfSdrOle2Obj();

    // helper for creating a OLE sequence for this object. It takes care od attributes, needed
    // scaling (e.g. for EmptyPresObj's), the correct graphic and other stuff. It is used from
    // createViewIndependentPrimitive2DSequence with false, and with evtl. HighContrast true
    // from the VOC which knows that
    drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceWithParameters() const;

    // #i123539# get rid of buffered chart content (if there) on change
    virtual void ActionChanged() override;

    virtual basegfx::B2DRange getRange( const drawinglayer::geometry::ViewInformation2D& rViewInfo2D ) const override;

protected:
    // This method is responsible for creating the graphical visualisation data
    // ONLY based on model data, just wraps to call createPrimitive2DSequenceWithParameters(false)
    virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const override;
};

}}



#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
