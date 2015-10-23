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

#ifndef INCLUDED_SVX_XPROPERTYTABLE_HXX
#define INCLUDED_SVX_XPROPERTYTABLE_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <svx/svxdllapi.h>
#include <svx/xtable.hxx>

// FIXME: should have a single factory method with an enumeration here [!]

css::uno::Reference< css::uno::XInterface > SAL_CALL SvxUnoXColorTable_createInstance( XPropertyList* pList ) throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SvxUnoXLineEndTable_createInstance( XPropertyList* pList ) throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SvxUnoXDashTable_createInstance( XPropertyList* pList ) throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SvxUnoXHatchTable_createInstance( XPropertyList* pList ) throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SvxUnoXGradientTable_createInstance( XPropertyList* pList ) throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SvxUnoXBitmapTable_createInstance( XPropertyList* pList ) throw();

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
