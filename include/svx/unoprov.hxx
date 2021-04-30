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

#include <config_options.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <svl/itemprop.hxx>
#include <svx/svxdllapi.h>
#include <editeng/unoipset.hxx>
#include <rtl/ref.hxx>
#include <memory>


class SvxItemPropertySet;
class SfxItemPool;

/**
 * class UHashMap
 */

#define UHASHMAP_NOTFOUND sal::static_int_cast< sal_uInt32 >(~0)
class UHashMap
{
    UHashMap() = delete;
public:
    static sal_uInt32 getId( const OUString& rCompareString );
    static OUString getNameFromId (sal_uInt32 nId);
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

class SVXCORE_DLLPUBLIC SvxUnoPropertyMapProvider
{
    SfxItemPropertyMapEntry const * aMapArr[SVXMAP_END];
    std::unique_ptr<SvxItemPropertySet> aSetArr[SVXMAP_END];
public:
    SvxUnoPropertyMapProvider();
    ~SvxUnoPropertyMapProvider();
    const SfxItemPropertyMapEntry* GetMap(sal_uInt16 nPropertyId);
    const SvxItemPropertySet* GetPropertySet(sal_uInt16 nPropertyId, SfxItemPool& rPool);
};

/**
 * Globals
 */

#define E3D_INVENTOR_FLAG           (0x80000000)

/**
 * class SvxPropertySetInfoPool
 */

const sal_Int32 SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS = 0;
const sal_Int32 SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER = 1;
const sal_Int32 SVXUNO_SERVICEID_LASTID = 1;

namespace comphelper { class PropertySetInfo; }

class SvxPropertySetInfoPool
{
public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) static rtl::Reference<comphelper::PropertySetInfo> const & getOrCreate( sal_Int32 nServiceId ) noexcept;

private:
    static rtl::Reference<comphelper::PropertySetInfo> mxInfos[SVXUNO_SERVICEID_LASTID+1];
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
