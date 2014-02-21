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

#ifndef INCLUDED_VCL_SOURCE_FILTER_JPEG_JPEG_H
#define INCLUDED_VCL_SOURCE_FILTER_JPEG_JPEG_H

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <sal/types.h>

#include <jpeglib.h>

namespace com { namespace sun { namespace star { namespace task {
    class XStatusIndicator;
} } } }
class JPEGReader;
class JPEGWriter;
class Size;

void jpeg_svstream_src (j_decompress_ptr cinfo, void* infile);

void jpeg_svstream_dest (j_compress_ptr cinfo, void* outfile);

bool    WriteJPEG( JPEGWriter* pJPEGWriter, void* pOutputStream, long nWidth, long nHeight, bool bGreyScale,
                   long nQualityPercent, long aChromaSubsampling,
                   css::uno::Reference<css::task::XStatusIndicator> const & status);

void    ReadJPEG( JPEGReader* pJPEGReader, void* pInputStream, long* pLines,
                  Size const & previewSize );

long    Transform( void* pInputStream, void* pOutputStream, long nAngle );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
