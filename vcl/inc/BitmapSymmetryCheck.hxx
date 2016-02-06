/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_BITMAPSYMMETRYCHECK_HXX
#define INCLUDED_VCL_INC_BITMAPSYMMETRYCHECK_HXX

#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>

class VCL_DLLPUBLIC BitmapSymmetryCheck
{
public:
    BitmapSymmetryCheck();
    virtual ~BitmapSymmetryCheck();

    static bool check(Bitmap& rBitmap);

protected:
    static bool checkImpl(BitmapReadAccess* pReadAccess);
};

#endif // INCLUDED_VCL_INC_BITMAPSYMMETRYCHECK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
