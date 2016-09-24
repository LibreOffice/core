/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SOURCE_SVDRAW_SVDCONV_HXX
#define INCLUDED_SVX_SOURCE_SVDRAW_SVDCONV_HXX

#include <sal/types.h>

inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }

#endif // INCLUDED_SVX_SOURCE_SVDRAW_SVDCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
