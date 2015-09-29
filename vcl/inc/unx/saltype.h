/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_VCL_INC_UNX_SALTYPE_H
#define INCLUDED_VCL_INC_UNX_SALTYPE_H

// an X11 screen index - this unpleasant construct is to allow
// us to cleanly separate the 'DisplayScreen' concept - as used
// in the public facing API, from X's idea of screen indices.
// Both of these are plain unsigned integers called 'screen'
class SalX11Screen {
    unsigned int mnXScreen;
public:
    explicit SalX11Screen(unsigned int nXScreen) : mnXScreen( nXScreen ) {}
    unsigned int getXScreen() const { return mnXScreen; }
    bool operator==(const SalX11Screen &rOther) { return rOther.mnXScreen == mnXScreen; }
    bool operator!=(const SalX11Screen &rOther) { return rOther.mnXScreen != mnXScreen; }
};

#endif // INCLUDED_VCL_INC_UNX_SALTYPE_H
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
