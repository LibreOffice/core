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

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <hffrm.hxx>
#include "accheaderfooter.hxx"
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

constexpr OUStringLiteral sImplementationNameHeader
    = u"com.sun.star.comp.Writer.SwAccessibleHeaderView";
constexpr OUStringLiteral sImplementationNameFooter
    = u"com.sun.star.comp.Writer.SwAccessibleFooterView";

SwAccessibleHeaderFooter::SwAccessibleHeaderFooter(
        std::shared_ptr<SwAccessibleMap> const& pInitMap,
        const SwHeaderFrame* pHdFrame    ) :
    SwAccessibleContext( pInitMap, AccessibleRole::HEADER, pHdFrame )
{
    OUString sArg( OUString::number( pHdFrame->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_HEADER_NAME, &sArg ) );
}

SwAccessibleHeaderFooter::SwAccessibleHeaderFooter(
        std::shared_ptr<SwAccessibleMap> const& pInitMap,
        const SwFooterFrame* pFtFrame    ) :
    SwAccessibleContext( pInitMap, AccessibleRole::FOOTER, pFtFrame )
{
    OUString sArg( OUString::number( pFtFrame->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_FOOTER_NAME, &sArg ) );
}

SwAccessibleHeaderFooter::~SwAccessibleHeaderFooter()
{
}

OUString SAL_CALL SwAccessibleHeaderFooter::getAccessibleDescription()
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    TranslateId pResId = AccessibleRole::HEADER == GetRole()
        ? STR_ACCESS_HEADER_DESC
        : STR_ACCESS_FOOTER_DESC ;

    OUString sArg( GetFormattedPageNumber() );

    return GetResource(pResId, &sArg);
}

OUString SAL_CALL SwAccessibleHeaderFooter::getImplementationName()
{
    if( AccessibleRole::HEADER == GetRole() )
        return sImplementationNameHeader;
    else
        return sImplementationNameFooter;
}

sal_Bool SAL_CALL SwAccessibleHeaderFooter::supportsService(const OUString& sTestServiceName)
{
    return cppu::supportsService(this, sTestServiceName);
}

Sequence< OUString > SAL_CALL SwAccessibleHeaderFooter::getSupportedServiceNames()
{
    return { (AccessibleRole::HEADER == GetRole())?u"com.sun.star.text.AccessibleHeaderView"_ustr:u"com.sun.star.text.AccessibleFooterView"_ustr,
             sAccessibleServiceName };
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleHeaderFooter::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

sal_Int32 SAL_CALL SwAccessibleHeaderFooter::getBackground()
{
    Reference< XAccessible > xParent =  getAccessibleParent();
    if (xParent.is())
    {
        Reference< XAccessibleComponent > xAccContext (xParent,UNO_QUERY);
        if(xAccContext.is())
        {
            return xAccContext->getBackground();
        }
    }
    return SwAccessibleContext::getBackground();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
