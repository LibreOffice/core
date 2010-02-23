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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <sal/types.h>
#include <svtools/grfmgr.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

class SvxShape;
class SdrObject;
class SdrPage;
class SvxNumBulletItem;
class SfxItemPool;
class String;

/** creates a StarOffice API wrapper with the given type and inventor
    Deprecated: This will be replaced with a function returning XShape.
*/
SVX_DLLPUBLIC SvxShape* CreateSvxShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor ) throw();

/** returns a StarOffice API wrapper for the given SdrObject */
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ();

/** returns the SdrObject from the given StarOffice API wrapper */
SVX_DLLPUBLIC SdrObject* GetSdrObjectFromXShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape ) throw() ;

/** returns a StarOffice API wrapper for the given SdrPage */
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > GetXDrawPageForSdrPage( SdrPage* pPage ) throw ();

/** returns the SdrPage from the given StarOffice API wrapper */
SVX_DLLPUBLIC SdrPage* GetSdrPageFromXDrawPage( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage ) throw() ;

/** returns the SvxNumBulletItem with the given name from the pool or a null if there is no item
    with that name
*/
SvxNumBulletItem* SvxGetNumBulletItemByName( SfxItemPool* pPool, const ::rtl::OUString& aName ) throw();

/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.

    @cl: for warnings01 I found out that this method never worked so I thin
         it is not used at all
sal_Bool SvxMeasureUnitToMapUnit( const short eApi, short& nVcl ) throw();
*/

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
SVX_DLLPUBLIC sal_Bool SvxMapUnitToMeasureUnit( const short nVcl, short& eApi ) throw();

/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.
*/
SVX_DLLPUBLIC sal_Bool SvxMeasureUnitToFieldUnit( const short eApi, short& nVcl ) throw();

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
SVX_DLLPUBLIC sal_Bool SvxFieldUnitToMeasureUnit( const short nVcl, short& eApi ) throw();

/** if the given name is a predefined name for the current language it is replaced by
    the corresponding api name.
*/
SVX_DLLPUBLIC void SvxUnogetApiNameForItem( const sal_Int16 nWhich, const String& rInternalName, rtl::OUString& rApiName ) throw();

/** if the given name is a predefined api name it is replaced by the predefined name
    for the current language.
*/
SVX_DLLPUBLIC void SvxUnogetInternalNameForItem( const sal_Int16 nWhich, const rtl::OUString& rApiName, String& rInternalName ) throw();

#endif // _SVX_UNOAPI_HXX_

