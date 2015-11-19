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

#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <flyfrm.hxx>
#include <fmturl.hxx>
#include "accgraphic.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

SwAccessibleGraphic::SwAccessibleGraphic(
        SwAccessibleMap* pInitMap,
        const SwFlyFrame* pFlyFrame  ) :
    SwAccessibleNoTextFrame( pInitMap, AccessibleRole::GRAPHIC, pFlyFrame )
{
}

SwAccessibleGraphic::~SwAccessibleGraphic()
{
}

OUString SAL_CALL SwAccessibleGraphic::getImplementationName()
        throw( RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.Writer.SwAccessibleGraphic");
}

sal_Bool SAL_CALL SwAccessibleGraphic::supportsService(const OUString& sTestServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sTestServiceName);
}

Sequence< OUString > SAL_CALL SwAccessibleGraphic::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.AccessibleTextGraphicObject";
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleGraphic::getImplementationId()
        throw(RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

//  Return this object's role.
sal_Int16 SAL_CALL SwAccessibleGraphic::getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    SwFormatURL aURL( static_cast<const SwLayoutFrame*>(GetFrame())->GetFormat()->GetURL() );

    if (aURL.GetMap())
        return AccessibleRole::IMAGE_MAP;
    return AccessibleRole::GRAPHIC;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
