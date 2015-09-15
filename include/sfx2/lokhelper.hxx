/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/dllapi.h>

class SfxViewShell;

class SFX2_DLLPUBLIC SfxLokHelper
{
public:
    /// Create a new view shell for pViewShell's object shell.
    static int createView(SfxViewShell* pViewShell);
    /// Destroy a view shell from the global shell list.
    static void destroyView(size_t nId);

    /// Total number of view shells.
    static int getViews();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
