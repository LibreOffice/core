/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSBITMAPFACTORY_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSBITMAPFACTORY_HXX

#include "PreviewRenderer.hxx"
#include <memory>

class SdPage;
class Size;

namespace sd { namespace slidesorter { namespace cache {

/** This factory class creates preview bitmaps for page objects.  It is
    merely an adapter for the PreviewRenderer.
*/
class BitmapFactory
{
public:
    BitmapFactory();
    ~BitmapFactory();

    Bitmap CreateBitmap (
        const SdPage& rPage,
        const Size& rPixelSize,
        const bool bDoSuperSampling);

private:
    PreviewRenderer maRenderer;
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
