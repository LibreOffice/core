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

#pragma once

#include <vcl/dllapi.h>

class GDIMetaFile;
class BitmapEx;

namespace vcl
{

/** Convert the bitmap to a meta file
 *
 * This works by putting continuous areas of the same color into
 * polygons painted in this color, by tracing the area's bounding
 * line.
 *
 * @param rMetafile
 * The resulting meta file
 *
 */
class VCL_DLLPUBLIC Vectorizer
{
private:
    /** If not 0, minimal size of bound rects for individual polygons. Smaller ones are ignored. */
    sal_uInt8 mnReduce = 0;
    /** A callback for showing the progress of the vectorization */
    Link<tools::Long, void> const* mpProgress = nullptr;

public:
    Vectorizer(sal_uInt8 nReduce)
        : mnReduce(nReduce)
    {}

    bool vectorize(BitmapEx const& rBitmap, GDIMetaFile& rMetafile);

    void setProgressCallback(const Link<tools::Long,void>* pProgress)
    {
        mpProgress = pProgress;
    }

    void updateProgress(tools::Long nProgress);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
