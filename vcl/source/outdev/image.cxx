/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <comphelper/lok.hxx>

#include <vcl/dibtools.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilterStackBlur.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/bmpfast.hxx>
#include <salgdi.hxx>
#include <salbmp.hxx>

#include <cassert>
#include <cstdlib>
#include <memory>

void OutputDevice::DrawImage(const Point& rPos, const Image& rImage, DrawImageFlags nStyle)
{
    assert(!is_double_buffered_window());

    DrawImage(rPos, Size(), rImage, nStyle);
}

void OutputDevice::DrawImage(const Point& rPos, const Size& rSize, const Image& rImage,
                             DrawImageFlags nStyle)
{
    assert(!is_double_buffered_window());

    bool bIsSizeValid = !rSize.IsEmpty();

    if (!ImplIsRecordLayout())
    {
        Image& rNonConstImage = const_cast<Image&>(rImage);
        if (bIsSizeValid)
            rNonConstImage.Draw(this, rPos, nStyle, &rSize);
        else
            rNonConstImage.Draw(this, rPos, nStyle);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
