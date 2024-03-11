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

#ifndef INCLUDED_SVX_UNOPROV_HXX
#define INCLUDED_SVX_UNOPROV_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <svl/itemprop.hxx>
#include <svx/svxdllapi.h>
#include <editeng/unoipset.hxx>
#include <rtl/ref.hxx>
#include <memory>
#include <optional>

class SvxItemPropertySet;
class SfxItemPool;
enum class SdrObjKind : sal_uInt16;

/**
 * class UHashMap
 */
class UHashMap
{
    UHashMap() = delete;
public:
    static std::optional<SdrObjKind> getId( const OUString& rCompareString );
    static OUString getNameFromId (SdrObjKind nId);
    static css::uno::Sequence< OUString > getServiceNames();
};

/**
 * Sorter
 */

#define SVXMAP_SHAPE                0
#define SVXMAP_CONNECTOR            1
#define SVXMAP_DIMENSIONING         2
#define SVXMAP_CIRCLE               3
#define SVXMAP_POLYPOLYGON          4
#define SVXMAP_GRAPHICOBJECT        5
#define SVXMAP_3DSCENEOBJECT        6
#define SVXMAP_3DCUBEOBJECT         7
#define SVXMAP_3DSPHEREOBJECT       8
#define SVXMAP_3DLATHEOBJECT        9
#define SVXMAP_3DEXTRUDEOBJECT      10
#define SVXMAP_3DPOLYGONOBJECT      11
#define SVXMAP_ALL                  12
#define SVXMAP_GROUP                13
#define SVXMAP_CAPTION              14
#define SVXMAP_OLE2                 15
#define SVXMAP_PLUGIN               16
#define SVXMAP_FRAME                17
#define SVXMAP_APPLET               18
#define SVXMAP_CONTROL              19
#define SVXMAP_TEXT                 20
#define SVXMAP_CUSTOMSHAPE          21
#define SVXMAP_MEDIA                22
#define SVXMAP_TABLE                23
#define SVXMAP_PAGE                 24
#define SVXMAP_END                  25  // last+1 !

/**
 * SvxUnoPropertyMapProvider
 */

class UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) SvxUnoPropertyMapProvider
{
    std::span<SfxItemPropertyMapEntry const> aMapArr[SVXMAP_END];
    std::unique_ptr<SvxItemPropertySet> aSetArr[SVXMAP_END];
public:
    SvxUnoPropertyMapProvider();
    ~SvxUnoPropertyMapProvider();
    std::span<const SfxItemPropertyMapEntry> GetMap(sal_uInt16 nPropertyId);
    const SvxItemPropertySet* GetPropertySet(sal_uInt16 nPropertyId, SfxItemPool& rPool);
};

/**
 * class SvxPropertySetInfoPool
 */

namespace comphelper { class PropertySetInfo; }

namespace SvxPropertySetInfoPool
{
    SVXCORE_DLLPUBLIC rtl::Reference<comphelper::PropertySetInfo> const & getDrawingDefaults() noexcept;
    SVXCORE_DLLPUBLIC rtl::Reference<comphelper::PropertySetInfo> const & getWriterDrawingDefaults() noexcept;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
