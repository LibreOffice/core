/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFOPENGLOBJ_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFOPENGLOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>

class SdrOpenGLObj;

namespace sdr {
namespace contact {

class ViewContactOfOpenGLObj : public ViewContactOfSdrObj
{
public:
    explicit ViewContactOfOpenGLObj(SdrOpenGLObj& rOpenGLObj);
    virtual ~ViewContactOfOpenGLObj();

protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

    virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const override;
};

} // namespace contact
} // namespace sdr

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
