/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SIGNATUREHELPER_HXX
#define INCLUDED_SFX2_SIGNATUREHELPER_HXX

#include <sfx2/signaturestate.hxx>

class SfxViewFrame;

class SignatureHelper
{
public:
    SignatureHelper(SfxViewFrame* rViewFrame);
    /**
     * Show the appropriate infobar according to the current signature status
    */
    void updateInfoBarState(SignatureState signatureState);
private:
    SfxViewFrame* mrViewFrame;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

