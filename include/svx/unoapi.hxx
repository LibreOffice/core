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

#ifndef INCLUDED_SVX_UNOAPI_HXX
#define INCLUDED_SVX_UNOAPI_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <sal/types.h>
#include <svtools/grfmgr.hxx>
#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>
#include <tools/fldunit.hxx>

class SvxShape;
class SdrObject;
class SdrPage;
class SvxNumBulletItem;
class SfxItemPool;

/**
 * Creates a StarOffice API wrapper with the given type and inventor
 * Deprecated: This will be replaced with a function returning XShape.
 */
SVX_DLLPUBLIC SvxShape* CreateSvxShapeByTypeAndInventor(sal_uInt16 nType, sal_uInt32 nInventor, OUString const & referer)
    throw (css::uno::RuntimeException, std::exception);

/** Returns a StarOffice API wrapper for the given SdrObject */
SVX_DLLPUBLIC css::uno::Reference< css::drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ();

/** Returns the SdrObject from the given StarOffice API wrapper */
SVX_DLLPUBLIC SdrObject* GetSdrObjectFromXShape( const css::uno::Reference< css::drawing::XShape >& xShape ) throw() ;

/** Returns a StarOffice API wrapper for the given SdrPage */
SVX_DLLPUBLIC css::uno::Reference< css::drawing::XDrawPage > GetXDrawPageForSdrPage( SdrPage* pPage ) throw ();

/** Returns the SdrPage from the given StarOffice API wrapper */
SVX_DLLPUBLIC SdrPage* GetSdrPageFromXDrawPage( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage ) throw() ;

/**
 * Maps the vcl MapUnit enum to a API constant MeasureUnit.
 * Returns false if conversion is not supported.
 */
SVX_DLLPUBLIC bool SvxMapUnitToMeasureUnit( const MapUnit nVcl, short& eApi ) throw();

/**
 * Maps the API constant MeasureUnit to a vcl MapUnit enum.
 * Returns false if conversion is not supported.
 */
SVX_DLLPUBLIC bool SvxMeasureUnitToFieldUnit( const short eApi, FieldUnit& nVcl ) throw();

/**
 * Maps the vcl MapUnit enum to a API constant MeasureUnit.
 * Returns false if conversion is not supported.
 */
SVX_DLLPUBLIC bool SvxFieldUnitToMeasureUnit( const FieldUnit nVcl, short& eApi ) throw();

/**
 * If the given name is a predefined name for the current language it is replaced by
 * the corresponding API name.
*/
SVX_DLLPUBLIC SAL_WARN_UNUSED_RESULT OUString
    SvxUnogetApiNameForItem(const sal_Int16 nWhich, const OUString& rInternalName) throw(std::exception);

/**
 * If the given name is a predefined API name it is replaced by the predefined name
 * for the current language.
*/
SVX_DLLPUBLIC SAL_WARN_UNUSED_RESULT OUString
    SvxUnogetInternalNameForItem(const sal_Int16 nWhich, const OUString& rApiName) throw(std::exception);

#endif // INCLUDED_SVX_UNOAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
