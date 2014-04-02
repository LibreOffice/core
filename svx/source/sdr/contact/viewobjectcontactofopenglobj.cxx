/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/sdr/contact/viewobjectcontactofopenglobj.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

using namespace sdr::contact;


ViewObjectContactOfOpenGLObj::ViewObjectContactOfOpenGLObj(
    ObjectContact& rObjectContact, ViewContact& rViewContact )
    : ViewObjectContactOfSdrObj( rObjectContact, rViewContact )
{
}

ViewObjectContactOfOpenGLObj::~ViewObjectContactOfOpenGLObj()
{
}

Window* ViewObjectContactOfOpenGLObj::getWindow() const
{
    Window* pRetval = 0;

    boost::optional<const OutputDevice&> oPageOutputDev = getPageViewOutputDevice();
    if( oPageOutputDev )
    {
        if(OUTDEV_WINDOW == oPageOutputDev->GetOutDevType())
        {
            pRetval = static_cast< Window* >(&const_cast<OutputDevice&>(oPageOutputDev.get()));
        }
    }

    return pRetval;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
