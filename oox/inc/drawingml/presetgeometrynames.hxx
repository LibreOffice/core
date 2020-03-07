/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_DRAWINGML_PRESETGEOMETRYNAMES_HXX
#define INCLUDED_OOX_DRAWINGML_PRESETGEOMETRYNAMES_HXX

#include <rtl/ustring.hxx>

namespace PresetGeometryTypeNames
{
OUString GetFontworkType(const OUString& rMsoType);
OUString GetMsoName(const OUString& rFontworkType);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
