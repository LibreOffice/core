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
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/gradienttools.hxx>
#include <basegfx/tools/tools.hxx>
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
#include <emfpstringformat.hxx>


namespace cppcanvas
{
    namespace internal
    {
        void EMFPStringFormat::Read(SvMemoryStream &s)
        {
            sal_uInt32 header;

            s.ReadUInt32(version).ReadUInt32(stringFormatFlags).ReadUInt32(language);
            s.ReadUInt32(stringAlignment).ReadUInt32(lineAlign).ReadUInt32(digitSubstitution).ReadUInt32(digitLanguage);
            s.ReadFloat(firstTabOffset).ReadInt32(hotkeyPrefix).ReadFloat(leadingMargin).ReadFloat(trailingMargin).ReadFloat(tracking);
            s.ReadInt32(trimming).ReadInt32(tabStopCount).ReadInt32(rangeCount);

            OSL_ASSERT((header >> 12) == 0xdbc01);

            SAL_INFO("cppcanvas.emf", "EMF+\t string format\nEMF+\theader: 0x" << std::hex << (header >> 12) << " version: 0x" << (header & 0x1fff) << " StringFormatFlags: " << std::dec << stringFormatFlags << " Language: " << language);
            SAL_INFO("cppcanvas.emf", "EMF+\t StringAlignment: " << stringAlignment << " LineAlign: " << lineAlign << " DigitSubstitution: " << digitSubstitution << " DigitLanguage: " << digitLanguage);
            SAL_INFO("cppcanvas.emf", "EMF+\t FirstTabOffset: " << firstTabOffset << " HotkeyPrefix: " << hotkeyPrefix << " LeadingMargin: " << leadingMargin << " TrailingMargin: " << trailingMargin << " Tracking: " << tracking);
            SAL_INFO("cppcanvas.emf", "EMF+\t Trimming: " << trimming << " TabStopCount: " << tabStopCount << " RangeCount: " << rangeCount);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
