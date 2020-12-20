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

#include <vcl/gdimtf.hxx>

namespace tools
{
class PolyPolygon;
}

namespace vcl::Vectorizer
{
/** Produce a metafile from a bitmap

    This works by putting continuous areas of the same color into
    polygons painted in this color, by tracing the area's bounding
    line.

    @returns GDIMetaFile
    The resulting meta file

    @param rColorBmp
    The bitmap to convert to a metafile

    @param cReduce
    If non-null, minimal size of bound rects for individual polygons. Smaller ones are ignored.

    @param pProgress
    A callback for showing the progress of the vectorization
 */

GDIMetaFile VCL_DLLPUBLIC ProduceMetafile(const Bitmap& rColorBmp, sal_uInt8 cReduce,
                                          const Link<tools::Long, void>* pProgress);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
