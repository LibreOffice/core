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

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <rtl/ref.hxx>
#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/fldunit.hxx>
#include <tools/mapunit.hxx>

class SvxShape;
class SdrObject;
class SdrPage;
class SvxNumBulletItem;
class SfxItemPool;
enum class SdrInventor : sal_uInt32;

/**
 * Creates a StarOffice API wrapper with the given type and inventor
 * Deprecated: This will be replaced with a function returning XShape.
 *
 * @throws css::uno::RuntimeException
 */
SVXCORE_DLLPUBLIC rtl::Reference<SvxShape> CreateSvxShapeByTypeAndInventor(sal_uInt16 nType, SdrInventor nInventor, OUString const & referer);

/** Returns a StarOffice API wrapper for the given SdrObject */
SVXCORE_DLLPUBLIC css::uno::Reference< css::drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ();

/** Returns the SdrObject from the given StarOffice API wrapper */
SVXCORE_DLLPUBLIC SdrObject* GetSdrObjectFromXShape( const css::uno::Reference< css::drawing::XShape >& xShape ) throw() ;

/** Returns a StarOffice API wrapper for the given SdrPage */
SVXCORE_DLLPUBLIC css::uno::Reference< css::drawing::XDrawPage > GetXDrawPageForSdrPage( SdrPage* pPage ) throw ();

/** Returns the SdrPage from the given StarOffice API wrapper */
SVXCORE_DLLPUBLIC SdrPage* GetSdrPageFromXDrawPage( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage ) throw() ;

/**
 * Maps the vcl MapUnit enum to an API constant MeasureUnit.
 * Returns false if conversion is not supported.
 */
SVXCORE_DLLPUBLIC bool SvxMapUnitToMeasureUnit( const MapUnit nVcl, short& eApi ) throw();

/**
 * Maps the API constant MeasureUnit to a vcl MapUnit enum.
 * Returns false if conversion is not supported.
 */
SVXCORE_DLLPUBLIC bool SvxMeasureUnitToFieldUnit( const short eApi, FieldUnit& nVcl ) throw();

/**
 * Maps the vcl MapUnit enum to an API constant MeasureUnit.
 * Returns false if conversion is not supported.
 */
SVXCORE_DLLPUBLIC bool SvxFieldUnitToMeasureUnit( const FieldUnit nVcl, short& eApi ) throw();

/**
 * If the given name is a predefined name for the current language it is replaced by
 * the corresponding API name.
 *
 * @throws std::exception
*/
[[nodiscard]] SVXCORE_DLLPUBLIC OUString
    SvxUnogetApiNameForItem(const sal_uInt16 nWhich, const OUString& rInternalName);

/**
 * If the given name is a predefined API name it is replaced by the predefined name
 * for the current language.
 *
 * @throws std::exception
*/
[[nodiscard]] OUString
    SvxUnogetInternalNameForItem(const sal_uInt16 nWhich, const OUString& rApiName);

#endif // INCLUDED_SVX_UNOAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
