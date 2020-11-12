/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_INCLUDE_SFX2_FLATPAK_HXX
#define INCLUDED_INCLUDE_SFX2_FLATPAK_HXX

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>

// Functionality related to the Flatpak version of LibreOffice.

namespace flatpak
{
bool isFlatpak();

// Must only be called with SolarMutex locked:
bool createTemporaryHtmlDirectory(OUString** url);

// Must only be called with SolarMutex locked:
SFX2_DLLPUBLIC void removeTemporaryHtmlDirectory();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
