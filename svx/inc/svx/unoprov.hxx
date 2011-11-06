/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SVX_UNOPROV_HXX
#define SVX_UNOPROV_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <svl/itemprop.hxx>
#include <tools/list.hxx>
#include "svx/svxdllapi.h"

class SvxItemPropertySet;
class SfxItemPool;

/***********************************************************************
* class UHashMap                                                       *
***********************************************************************/

struct UHashMapEntry
{
    ::rtl::OUString aIdentifier;
    sal_uInt32 nId;

    UHashMapEntry(const sal_Char * value, sal_Int32 length, sal_uInt32 _nId) : aIdentifier(value,length,RTL_TEXTENCODING_ASCII_US), nId(_nId) { }
};

DECLARE_LIST( UHashMapEntryList, UHashMapEntry* )

#define HASHARRAYSIZE   0x10
#define UHASHMAP_NOTFOUND sal::static_int_cast< sal_uInt32 >(~0)

class UHashMap
{
private:
    UHashMapEntryList m_aHashList[HASHARRAYSIZE];

public:
    UHashMap( UHashMapEntry* pMap );
    ~UHashMap() {};

    sal_uInt32 getId( const ::rtl::OUString& rCompareString );
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
extern UHashMapEntry pSdrShapeIdentifierMap[];
extern UHashMap aSdrShapeIdentifierMap;

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

