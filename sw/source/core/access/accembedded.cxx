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
#include <flyfrm.hxx>
#include "accembedded.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextEmbeddedObject";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleEmbeddedObject";

SwAccessibleEmbeddedObject::SwAccessibleEmbeddedObject(
        SwAccessibleMap* pInitMap,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleNoTextFrame( pInitMap, AccessibleRole::EMBEDDED_OBJECT, pFlyFrm )
{
}

SwAccessibleEmbeddedObject::~SwAccessibleEmbeddedObject()
{
}

OUString SAL_CALL SwAccessibleEmbeddedObject::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleEmbeddedObject::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleEmbeddedObject::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( sServiceName );
    pArray[1] = OUString( sAccessibleServiceName );
    return aRet;
}

namespace
{
    class theSwAccessibleEmbeddedObjectImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessibleEmbeddedObjectImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleEmbeddedObject::getImplementationId()
        throw(uno::RuntimeException)
{
    return theSwAccessibleEmbeddedObjectImplementationId::get().getSeq();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
