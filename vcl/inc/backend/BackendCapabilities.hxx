/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_BACKENDCAPABILITIES_HXX
#define INCLUDED_VCL_INC_BACKENDCAPABILITIES_HXX

namespace vcl
{
struct BackendCapabilities
{
    // The backend is capable of handling 32bpp bitmaps (as opposed to having
    // to split them to 24bpp data and extra alpha bitmap).
    bool mbSupportsBitmap32;
    // Bitmaps should be created as 32bpp if possible, e.g. if the backend
    // is more efficient with 32bpp formats.
    bool mbPrefersBitmap32;
    BackendCapabilities()
        : mbSupportsBitmap32(false)
        , mbPrefersBitmap32(false)
    {
    }
};
}

#endif // INCLUDED_VCL_INC_BACKENDCAPABILITIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
