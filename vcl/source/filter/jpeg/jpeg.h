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
#include <basegfx/vector/b2dsize.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapwriteaccess.hxx>

#include <jpeglib.h>

namespace com { namespace sun { namespace star { namespace task {
    class XStatusIndicator;
} } } }
class JPEGReader;
class JPEGWriter;
class Size;
class SvStream;
enum class GraphicFilterImportFlags;

void jpeg_svstream_src (j_decompress_ptr cinfo, void* infile);

void jpeg_svstream_dest (j_compress_ptr cinfo, void* outfile);

bool    WriteJPEG( JPEGWriter* pJPEGWriter, void* pOutputStream,
                   long nWidth, long nHeight, basegfx::B2DSize const &  aPPI, bool bGreyScale,
                   long nQualityPercent, long aChromaSubsampling,
                   css::uno::Reference<css::task::XStatusIndicator> const & status);

void    ReadJPEG( JPEGReader* pJPEGReader, void* pInputStream, long* pLines,
                  Size const & previewSize, GraphicFilterImportFlags nImportFlags,
                  BitmapScopedWriteAccess* ppAccess );

void    Transform(void* pInputStream, void* pOutputStream, long nAngle);

/* Expanded data source object for stdio input */

struct SourceManagerStruct {
    jpeg_source_mgr pub;                /* public fields */
    SvStream*   stream;                 /* source stream */
    JOCTET*     buffer;                 /* start of buffer */
    boolean     start_of_file;          /* have we gotten any data yet? */
    boolean     no_data_available;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
