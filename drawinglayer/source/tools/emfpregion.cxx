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
#include <emfpregion.hxx>

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace emfplushelper
{
    EMFPRegion::EMFPRegion()
        : parts(0)
        , combineMode(nullptr)
        , initialState(0)
        , ix(0.0)
        , iy(0.0)
        , iw(0.0)
        , ih(0.0)
    {
    }

    EMFPRegion::~EMFPRegion()
    {
        if (combineMode)
        {
            delete[] combineMode;
            combineMode = nullptr;
        }
    }

    void EMFPRegion::Read(SvStream& s)
    {
        sal_uInt32 header;
        s.ReadUInt32(header).ReadInt32(parts);
        SAL_INFO("cppcanvas.emf", "EMF+\tregion");
        SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " parts: " << parts << std::dec);

        if (parts)
        {
            if (parts<0 || sal_uInt32(parts)>SAL_MAX_INT32 / sizeof(sal_Int32))
            {
                parts = SAL_MAX_INT32 / sizeof(sal_Int32);
            }

            combineMode = new sal_Int32[parts];

            for (int i = 0; i < parts; i++)
            {
                s.ReadInt32(combineMode[i]);
                SAL_INFO("cppcanvas.emf", "EMF+\tcombine mode [" << i << "]: 0x" << std::hex << combineMode[i] << std::dec);
            }
        }

        s.ReadInt32(initialState);
        SAL_INFO("cppcanvas.emf", "EMF+\tinitial state: 0x" << std::hex << initialState << std::dec);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
