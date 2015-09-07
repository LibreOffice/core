/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <svdoopengl.hxx>
#include <sdr/contact/viewcontactofopenglobj.hxx>

#include <vcl/opengl/IOpenGLRenderer.hxx>

SdrOpenGLObj::SdrOpenGLObj()
  : SdrObject(),
    IOpenGLInfoProvider(),
    mpContext(NULL)
{
#if HAVE_FEATURE_DESKTOP
    mpContext = OpenGLContext::Create();
#endif
}

SdrOpenGLObj::~SdrOpenGLObj()
{
}

sdr::contact::ViewContact* SdrOpenGLObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfOpenGLObj(*this);
}


void SdrOpenGLObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrObject::NbcResize(rRef, xFact, yFact);

    // now pass the information to the OpenGL context
    if (mpContext.is())
        mpContext->setWinSize(aOutRect.GetSize());

    SAL_WARN("svx.opengl", "resized opengl drawinglayer object");
}

void SdrOpenGLObj::setRenderer(IOpenGLRenderer* pRenderer)
{
    mpRenderer.reset(pRenderer);
    mpRenderer->setInfoProvider(this);
}

IOpenGLRenderer* SdrOpenGLObj::getRenderer()
{
    return mpRenderer.get();
}

bool SdrOpenGLObj::isOpenGLInitialized()
{
    return mpContext.is() && mpContext->isInitialized();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
