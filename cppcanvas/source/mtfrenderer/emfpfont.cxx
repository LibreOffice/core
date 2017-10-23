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
#include <implrenderer.hxx>
#include <emfpfont.hxx>


namespace cppcanvas
{
    namespace internal
    {
        void EMFPFont::Read(SvMemoryStream &s)
        {
            sal_uInt32 header;
            sal_uInt32 reserved;
            sal_uInt32 length;

            s.ReadUInt32(header).ReadFloat(emSize).ReadUInt32(sizeUnit).ReadInt32(fontFlags).ReadUInt32(reserved).ReadUInt32(length);

            SAL_WARN_IF((header >> 12) != 0xdbc01, "cppcanvas.emf", "Invalid header - not 0xdbc01");

            SAL_INFO("cppcanvas.emf", "EMF+\tfont\nEMF+\theader: 0x" << std::hex << (header >> 12) << " version: 0x" << (header & 0x1fff) << " size: " << std::dec << emSize << " unit: 0x" << std::hex << sizeUnit << std::dec);
            SAL_INFO("cppcanvas.emf", "EMF+\tflags: 0x" << std::hex << fontFlags << " reserved: 0x" << reserved << " length: 0x" << std::hex << length << std::dec);

            if (length > 0 && length < 0x4000)
            {
                rtl_uString *pStr = rtl_uString_alloc(length);
                sal_Unicode *chars = pStr->buffer;

                for (sal_uInt32 i = 0; i < length; ++i)
                    s.ReadUtf16(chars[i]);

                family = OUString(pStr, SAL_NO_ACQUIRE);
                SAL_INFO("cppcanvas.emf", "EMF+\tfamily: " << family);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
