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

#include <bf_svx/svdpool.hxx>
#include <bf_svtools/itemprop.hxx>
#include <tools/list.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace binfilter {

/***********************************************************************
* class UHashMap                                                       *
***********************************************************************/

#define UHASHMAP_NOTFOUND sal::static_int_cast< UINT32 >(~0)

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

#define	SVXMAP_SHAPE				0
#define	SVXMAP_CONNECTOR			1
#define	SVXMAP_DIMENSIONING			2
#define	SVXMAP_CIRCLE				3
#define	SVXMAP_POLYPOLYGON			4
#define	SVXMAP_POLYPOLYGONBEZIER	5
#define	SVXMAP_GRAPHICOBJECT		6
#define	SVXMAP_3DSCENEOBJECT		7
#define	SVXMAP_3DCUBEOBJEKT			8
#define	SVXMAP_3DSPHEREOBJECT		9
#define	SVXMAP_3DLATHEOBJECT		10
#define	SVXMAP_3DEXTRUDEOBJECT		11
#define	SVXMAP_3DPOLYGONOBJECT		12
#define	SVXMAP_ALL					13
#define SVXMAP_GROUP				14
#define SVXMAP_CAPTION				15
#define SVXMAP_OLE2					16
#define SVXMAP_PLUGIN				17
#define SVXMAP_FRAME				18
#define SVXMAP_APPLET				19
#define SVXMAP_CONTROL				20
#define SVXMAP_TEXT					21
#define	SVXMAP_END					22	// last+1 !

/***********************************************************************
* SvxUnoPropertyMapProvider											   *
***********************************************************************/
class SvxUnoPropertyMapProvider
{
    SfxItemPropertyMap* aMapArr[SVXMAP_END];
    void Sort(USHORT nId);
public:
    SvxUnoPropertyMapProvider();
    SfxItemPropertyMap* GetMap(UINT16 nPropertyId);
};

/***********************************************************************
* Globals                                                              *
***********************************************************************/

const sal_Int16 OBJ_OLE2_APPLET	= 100;
const sal_Int16 OBJ_OLE2_PLUGIN = 101;

extern SvxUnoPropertyMapProvider aSvxMapProvider;

#define SFX_METRIC_ITEM				(0x40)
#define E3D_INVENTOR_FLAG			(0x80000000)
}//end of namespace binfilter
#include <bf_svx/unoipset.hxx>
namespace binfilter {

/***********************************************************************
* class SvxServiceInfoHelper                                           *
***********************************************************************/

/** this class provides a basic helper for classes suporting the XServiceInfo Interface.
 *
 *  you can overload the <code>getSupprotedServiceNames</code> to implement a XServiceInfo.
 *  you can use the static helper methods to combine your services with that of parent
 *  or aggregatet classes.
 */
class SvxServiceInfoHelper : public ::com::sun::star::lang::XServiceInfo
{
public:
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // helper
    static void addToSequence( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq,UINT16 nServices, /* sal_Char* */... ) throw();
    static sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& SupportedServices ) throw();
};

/***********************************************************************
* class SvxPropertySetInfoPool                                         *
***********************************************************************/

const sal_Int32 SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS = 0;
const sal_Int32 SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER = 1;
const sal_Int32 SVXUNO_SERVICEID_LASTID = 1;
}//end of namespace binfilter
namespace comphelper { class PropertySetInfo; }
namespace binfilter {
class SvxPropertySetInfoPool
{
public:
    static comphelper::PropertySetInfo* getOrCreate( sal_Int32 nServiceId ) throw();

private:
    static comphelper::PropertySetInfo* mpInfos[SVXUNO_SERVICEID_LASTID+1];
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
