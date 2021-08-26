/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/PdfConfig.hxx>
#include <cstdlib>

namespace vcl::pdf
{
/// Get the default PDF rendering resolution in DPI.
double getDefaultPdfResolutionDpi()
{
    // If an overriding default is set, use it.
    const char* envar = ::getenv("PDFIMPORT_RESOLUTION_DPI");
    if (envar)
    {
        const double dpi = atof(envar);
        if (dpi > 0)
            return dpi;
    }

    // Fallback to a sensible default.
    return 96.;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
