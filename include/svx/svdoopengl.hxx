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
#include <vcl/opengl/OpenGLContext.hxx>

#include <vcl/opengl/IOpenGLRenderer.hxx>

#include <boost/scoped_ptr.hpp>

namespace sdr { namespace contact {
    class ViewContact;
} }

class IOpenGLRenderer;

class SVX_DLLPUBLIC SdrOpenGLObj : public SdrObject, public IOpenGLInfoProvider
{
public:
    virtual ~SdrOpenGLObj();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

    OpenGLContext& getOpenGLContext();

    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;

    void setRenderer(IOpenGLRenderer* pRenderer);
    IOpenGLRenderer* getRenderer();

    virtual bool isOpenGLInitialized() SAL_OVERRIDE;

private:

    OpenGLContext maContext;

    boost::scoped_ptr<IOpenGLRenderer> mpRenderer;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
