/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SVDO_OPENGL_HXX
#define INCLUDED_SVX_SVDO_OPENGL_HXX

#include <svx/svdobj.hxx>
#include <vcl/OpenGLContext.hxx>

namespace sdr { namespace contact {
    class ViewContact;
} }

class SVX_DLLPUBLIC SdrOpenGLObj : public SdrObject
{
public:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

    OpenGLContext& getOpenGLContext();

    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;

private:

    OpenGLContext maContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
