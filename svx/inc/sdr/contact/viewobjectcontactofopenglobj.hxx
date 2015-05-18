/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFOPENGLOBJ_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFOPENGLOBJ_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

namespace vcl { class Window; }

namespace sdr {
namespace contact {

class ViewObjectContactOfOpenGLObj : public ViewObjectContactOfSdrObj
{
public:
    ViewObjectContactOfOpenGLObj(ObjectContact& rObjectContact, ViewContact& rViewContact);
    virtual ~ViewObjectContactOfOpenGLObj();

private:
    vcl::Window* getWindow() const;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
