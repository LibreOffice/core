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

#ifndef _SVX_UNOAPI_HXX_
#define _SVX_UNOAPI_HXX_


#include <com/sun/star/drawing/XShape.hpp>

#include <sal/types.h>

#include <bf_goodies/graphicobject.hxx>

#include <bf_svtools/poolitem.hxx>
class String;
namespace binfilter {

class SfxItemPool;

class SvxShape;
class SdrObject;
class SvxNumBulletItem;

/** creates a StarOffice API wrapper with the given type and inventor
    Deprecated: This will be replaced with a function returning XShape.
*/
SvxShape* CreateSvxShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor ) throw();

/** returns a StarOffice API wrapper for the given SdrObject */
::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ();

/** returns the SdrObject from the given StarOffice API wrapper */
SdrObject* GetSdrObjectFromXShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape ) throw() ;

/** returns a GraphicObject for this URL */
BfGraphicObject CreateGraphicObjectFromURL( const ::rtl::OUString &rURL ) throw() ;

/** returns the SvxNumBulletItem with the given name from the pool or a null if there is no item
    with that name
*/
SvxNumBulletItem* SvxGetNumBulletItemByName( SfxItemPool* pPool, const ::rtl::OUString& aName ) throw();

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
sal_Bool SvxMapUnitToMeasureUnit( const short nVcl, short& eApi ) throw();

/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.
*/
sal_Bool SvxMeasureUnitToFieldUnit( const short eApi, short& nVcl ) throw();

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
sal_Bool SvxFieldUnitToMeasureUnit( const short nVcl, short& eApi ) throw();

/** if the given name is a predefined name for the current language it is replaced by
    the corresponding api name.
*/
void SvxUnogetApiNameForItem( const sal_Int16 nWhich, const String& rInternalName, ::rtl::OUString& rApiName ) throw();

/** if the given name is a predefined api name it is replaced by the predefined name
    for the current	language.
*/
void SvxUnogetInternalNameForItem( const sal_Int16 nWhich, const ::rtl::OUString& rApiName, String& rInternalName ) throw();

/** converts the given any with a metric to 100th/mm if needed */
void SvxUnoConvertToMM( const SfxMapUnit eSourceMapUnit, ::com::sun::star::uno::Any & rMetric ) throw();

/** converts the given any with a metric from 100th/mm to the given metric if needed */
void SvxUnoConvertFromMM( const SfxMapUnit eDestinationMapUnit, ::com::sun::star::uno::Any & rMetric ) throw();

}//end of namespace binfilter
#endif // _SVX_UNOAPI_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
