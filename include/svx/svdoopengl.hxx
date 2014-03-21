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
#include <svx/sdr/contact/viewcontactofopenglobj.hxx>

class SVX_DLLPUBLIC SdrOpenGLObj : public SdrObject
{
public:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact()
    {
        return new sdr::contact::ViewContactOfOpenGLObj(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
