/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SVX_UNOPROV_HXX
#define SVX_UNOPROV_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <svl/itemprop.hxx>
#include "svx/svxdllapi.h"

class SvxItemPropertySet;
class SfxItemPool;

/***********************************************************************
* class UHashMap                                                       *
***********************************************************************/

#define UHASHMAP_NOTFOUND sal::static_int_cast< sal_uInt32 >(~0)
class UHashMap
{
    UHashMap() {}
public:
    static sal_uInt32 getId( const ::rtl::OUString& rCompareString );
    static rtl::OUString getNameFromId (sal_uInt32 nId);
    static ::com::sun::star::uno::Sequence< rtl::OUString > getServiceNames();
};

/***********************************************************************
* Soriterer                                                            *
***********************************************************************/

#define SVXMAP_SHAPE                0
#define SVXMAP_CONNECTOR            1
#define SVXMAP_DIMENSIONING         2
#define SVXMAP_CIRCLE               3
#define SVXMAP_POLYPOLYGON          4
#define SVXMAP_POLYPOLYGONBEZIER    5
#define SVXMAP_GRAPHICOBJECT        6
#define SVXMAP_3DSCENEOBJECT        7
#define SVXMAP_3DCUBEOBJEKT         8
#define SVXMAP_3DSPHEREOBJECT       9
#define SVXMAP_3DLATHEOBJECT        10
#define SVXMAP_3DEXTRUDEOBJECT      11
#define SVXMAP_3DPOLYGONOBJECT      12
#define SVXMAP_ALL                  13
#define SVXMAP_GROUP                14
#define SVXMAP_CAPTION              15
#define SVXMAP_OLE2                 16
#define SVXMAP_PLUGIN               17
#define SVXMAP_FRAME                18
#define SVXMAP_APPLET               19
#define SVXMAP_CONTROL              20
#define SVXMAP_TEXT                 21
#define SVXMAP_CUSTOMSHAPE          22
#define SVXMAP_MEDIA                23
#define SVXMAP_TABLE                24
#define SVXMAP_PAGE                 25
#define SVXMAP_END                  26  // last+1 !
/***********************************************************************
* SvxUnoPropertyMapProvider                                            *
***********************************************************************/
class SVX_DLLPUBLIC SvxUnoPropertyMapProvider
{
    SfxItemPropertyMapEntry* aMapArr[SVXMAP_END];
    SvxItemPropertySet* aSetArr[SVXMAP_END];
//  void Sort(sal_uInt16 nId);
public:
    SvxUnoPropertyMapProvider();
    ~SvxUnoPropertyMapProvider();
    const SfxItemPropertyMapEntry* GetMap(sal_uInt16 nPropertyId);
    const SvxItemPropertySet* GetPropertySet(sal_uInt16 nPropertyId, SfxItemPool& rPool);
};

/***********************************************************************
* Globals                                                              *
***********************************************************************/

const sal_Int16 OBJ_OLE2_APPLET = 100;
const sal_Int16 OBJ_OLE2_PLUGIN = 101;

extern SvxUnoPropertyMapProvider aSvxMapProvider;

#define E3D_INVENTOR_FLAG           (0x80000000)

#include <editeng/unoipset.hxx>

/***********************************************************************
* class SvxPropertySetInfoPool                                         *
***********************************************************************/

const sal_Int32 SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS = 0;
const sal_Int32 SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER = 1;
const sal_Int32 SVXUNO_SERVICEID_LASTID = 1;

namespace comphelper { class PropertySetInfo; }

class SvxPropertySetInfoPool
{
public:
    SVX_DLLPUBLIC static comphelper::PropertySetInfo* getOrCreate( sal_Int32 nServiceId ) throw();

private:
    static comphelper::PropertySetInfo* mpInfos[SVXUNO_SERVICEID_LASTID+1];
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
