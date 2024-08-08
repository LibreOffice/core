/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sddllapi.h>
#include <tools/gen.hxx>
#include <rtl/string.hxx>

class SdrPage;
class BitmapEx;
class Size;

namespace sd
{
class SD_DLLPUBLIC SlideshowLayerRenderer
{
    SdrPage* mpPage;
    Size maSlideSize;
    bool bRenderDone = false;

public:
    SlideshowLayerRenderer(SdrPage* pPage);
    Size calculateAndSetSizePixel(Size const& rDesiredSizePixel);
    bool render(unsigned char* pBuffer, OString& rJsonMsg);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
