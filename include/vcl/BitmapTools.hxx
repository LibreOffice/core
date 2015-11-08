/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BITMAP_TOOLS_HXX
#define INCLUDED_VCL_BITMAP_TOOLS_HXX

#include <vcl/bitmapex.hxx>
#include <tools/stream.hxx>

namespace vcl
{

class VCL_DLLPUBLIC BitmapTools
{
public:
    static void loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx,
                            double fScaleFactor = 1.0, const Size& aSize = Size());
};

}

#endif // INCLUDED_VCL_BITMAP_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
