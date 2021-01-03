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

#ifndef INCLUDED_VCL_INC_OUTDEV_H
#define INCLUDED_VCL_INC_OUTDEV_H

#include <set>
#include <vector>

#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

#include "fontinstance.hxx"
#include "impfontcache.hxx"

class Size;
namespace vcl { class Font; }
class VirtualDevice;
class PhysicalFontCollection;
enum class AddFontSubstituteFlags;

class ImplDeviceFontSizeList
{
private:
    std::vector<int>    maSizeList;

public:
                        ImplDeviceFontSizeList()
                        { maSizeList.reserve( 32 ); }
    void                Add( int nHeight )      { maSizeList.push_back( nHeight ); }
    int                 Count() const           { return maSizeList.size(); }
    int                 Get( int nIndex ) const { return maSizeList[ nIndex ]; }
};

namespace vcl { struct ControlLayoutData; }
// #i75163#
namespace basegfx { class B2DHomMatrix; }

struct ImplOutDevData
{
    VclPtr<VirtualDevice>       mpRotateDev;
    vcl::ControlLayoutData*     mpRecordLayout;
    tools::Rectangle                   maRecordRect;

    // #i75163#
    basegfx::B2DHomMatrix*      mpViewTransform;
    basegfx::B2DHomMatrix*      mpInverseViewTransform;
};

#endif // INCLUDED_VCL_INC_OUTDEV_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
