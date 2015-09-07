/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <sdr/contact/viewobjectcontactofopenglobj.hxx>
#include <sdr/contact/viewcontactofopenglobj.hxx>

#include <svdoopengl.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

using namespace sdr::contact;


ViewObjectContactOfOpenGLObj::ViewObjectContactOfOpenGLObj(
    ObjectContact& rObjectContact, ViewContact& rViewContact )
    : ViewObjectContactOfSdrObj( rObjectContact, rViewContact )
{
#if HAVE_FEATURE_DESKTOP
    rtl::Reference<OpenGLContext> pContext = static_cast<SdrOpenGLObj&>(static_cast<ViewContactOfSdrObj&>(rViewContact).GetSdrObject()).getOpenGLContext();
    if (pContext.is())
        pContext->init(getWindow());
#endif
}

ViewObjectContactOfOpenGLObj::~ViewObjectContactOfOpenGLObj()
{
}

vcl::Window* ViewObjectContactOfOpenGLObj::getWindow() const
{
    vcl::Window* pRetval = 0;

    boost::optional<const OutputDevice&> oPageOutputDev = getPageViewOutputDevice();
    if( oPageOutputDev )
    {
        if(OUTDEV_WINDOW == oPageOutputDev->GetOutDevType())
        {
            pRetval = static_cast< vcl::Window* >(&const_cast<OutputDevice&>(oPageOutputDev.get()));
        }
    }

    return pRetval;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
