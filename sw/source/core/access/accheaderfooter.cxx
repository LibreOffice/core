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

#include <osl/mutex.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/svapp.hxx>
#include <hffrm.hxx>
#include "accheaderfooter.hxx"
#include "access.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

const sal_Char sServiceNameHeader[] = "com.sun.star.text.AccessibleHeaderView";
const sal_Char sServiceNameFooter[] = "com.sun.star.text.AccessibleFooterView";
const sal_Char sImplementationNameHeader[] = "com.sun.star.comp.Writer.SwAccessibleHeaderView";
const sal_Char sImplementationNameFooter[] = "com.sun.star.comp.Writer.SwAccessibleFooterView";

SwAccessibleHeaderFooter::SwAccessibleHeaderFooter(
        SwAccessibleMap* pInitMap,
        const SwHeaderFrm* pHdFrm    ) :
    SwAccessibleContext( pInitMap, AccessibleRole::HEADER, pHdFrm )
{
    SolarMutexGuard aGuard;

    OUString sArg( OUString::number( pHdFrm->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_HEADER_NAME, &sArg ) );
}

SwAccessibleHeaderFooter::SwAccessibleHeaderFooter(
        SwAccessibleMap* pInitMap,
        const SwFooterFrm* pFtFrm    ) :
    SwAccessibleContext( pInitMap, AccessibleRole::FOOTER, pFtFrm )
{
    SolarMutexGuard aGuard;

    OUString sArg( OUString::number( pFtFrm->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_FOOTER_NAME, &sArg ) );
}

SwAccessibleHeaderFooter::~SwAccessibleHeaderFooter()
{
}

OUString SAL_CALL SwAccessibleHeaderFooter::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    sal_uInt16 nResId = AccessibleRole::HEADER == GetRole()
        ? STR_ACCESS_HEADER_DESC
        : STR_ACCESS_FOOTER_DESC ;

    OUString sArg( GetFormattedPageNumber() );

    return GetResource( nResId, &sArg );
}

OUString SAL_CALL SwAccessibleHeaderFooter::getImplementationName()
        throw( RuntimeException )
{
    if( AccessibleRole::HEADER == GetRole() )
        return OUString(sImplementationNameHeader);
    else
        return OUString(sImplementationNameFooter);
}

sal_Bool SAL_CALL SwAccessibleHeaderFooter::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    if( sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                       sizeof(sAccessibleServiceName)-1 ) )
        return sal_True;
    else if( AccessibleRole::HEADER == GetRole() )
        return sTestServiceName.equalsAsciiL( sServiceNameHeader, sizeof(sServiceNameHeader)-1 );
    else
        return sTestServiceName.equalsAsciiL( sServiceNameFooter, sizeof(sServiceNameFooter)-1 );

}

Sequence< OUString > SAL_CALL SwAccessibleHeaderFooter::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    if( AccessibleRole::HEADER == GetRole() )
        pArray[0] = OUString( sServiceNameHeader );
    else
        pArray[0] = OUString( sServiceNameFooter );
    pArray[1] = OUString( sAccessibleServiceName );
    return aRet;
}

namespace
{
    class theSwAccessibleHeaderFooterImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessibleHeaderFooterImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleHeaderFooter::getImplementationId()
        throw(RuntimeException)
{
    return theSwAccessibleHeaderFooterImplementationId::get().getSeq();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
