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

#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/utils/tools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/graphicfilter.hxx>
#include <emfpimage.hxx>

namespace emfplushelper
{
    void EMFPImage::Read(SvMemoryStream &s, sal_uInt32 dataSize, bool bUseWholeStream)
    {
        sal_uInt32 header, bitmapType;
        s.ReadUInt32(header).ReadUInt32(type);
        SAL_INFO("cppcanvas.emf", "EMF+\timage\nEMF+\theader: 0x" << std::hex << header << " type: " << type << std::dec);

        if (1 == type)
        {
            // bitmap
            s.ReadInt32(width).ReadInt32(height).ReadInt32(stride).ReadInt32(pixelFormat).ReadUInt32(bitmapType);
            SAL_INFO("cppcanvas.emf", "EMF+\tbitmap width: " << width << " height: " << height << " stride: " << stride << " pixelFormat: 0x" << std::hex << pixelFormat << std::dec);

            if ((bitmapType != 0) || (width == 0))
            {
                // non native formats
                GraphicFilter filter;
                filter.ImportGraphic(graphic, OUString(), s);
                SAL_INFO("cppcanvas.emf", "EMF+\tbitmap width: " << graphic.GetBitmap().GetSizePixel().Width() << " height: " << graphic.GetBitmap().GetSizePixel().Height());
            }
        }
        else if (2 == type)
        {
            // metafile
            sal_Int32 mfType, mfSize;
            s.ReadInt32(mfType).ReadInt32(mfSize);

            if (bUseWholeStream)
                dataSize = s.remainingSize();
            else
                dataSize -= 16;

            SAL_INFO("cppcanvas.emf", "EMF+\tmetafile type: " << mfType << " dataSize: " << mfSize << " real size calculated from record dataSize: " << dataSize);

            GraphicFilter filter;
            // workaround buggy metafiles, which have wrong mfSize set (n#705956 for example)
            SvMemoryStream mfStream(const_cast<char *>(static_cast<char const *>(s.GetData()) + s.Tell()), dataSize, StreamMode::READ);
            filter.ImportGraphic(graphic, OUString(), mfStream);

            // debug code - write the stream to debug file /tmp/emf-stream.emf
#if OSL_DEBUG_LEVEL > 1
            mfStream.Seek(0);
            static sal_Int32 emfp_debug_stream_number = 0;
            OUString emfp_debug_filename = "/tmp/emf-embedded-stream" +
                OUString::number(emfp_debug_stream_number++) + ".emf";

            SvFileStream file(emfp_debug_filename, StreamMode::WRITE | StreamMode::TRUNC);

            mfStream.WriteStream(file);
            file.Flush();
            file.Close();
#endif
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
