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
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <flyfrm.hxx>
#include "accembedded.hxx"
#include <cntfrm.hxx>
#include <notxtfrm.hxx>
#include <ndole.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

SwAccessibleEmbeddedObject::SwAccessibleEmbeddedObject(
        std::shared_ptr<SwAccessibleMap> const& pInitMap,
        const SwFlyFrame* pFlyFrame  ) :
    SwAccessibleNoTextFrame( pInitMap, AccessibleRole::EMBEDDED_OBJECT, pFlyFrame )
{
}

SwAccessibleEmbeddedObject::~SwAccessibleEmbeddedObject()
{
}

// XInterface
css::uno::Any SAL_CALL
    SwAccessibleEmbeddedObject::queryInterface (const css::uno::Type & rType)
{
    css::uno::Any aReturn = SwAccessibleNoTextFrame::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
         static_cast< css::accessibility::XAccessibleExtendedAttributes* >(this) );
    return aReturn;
}

void SAL_CALL
    SwAccessibleEmbeddedObject::acquire()
    noexcept
{
    SwAccessibleNoTextFrame::acquire ();
}

void SAL_CALL
    SwAccessibleEmbeddedObject::release()
    noexcept
{
    SwAccessibleNoTextFrame::release ();
}

OUString SAL_CALL SwAccessibleEmbeddedObject::getImplementationName()
{
    return u"com.sun.star.comp.Writer.SwAccessibleEmbeddedObject"_ustr;
}

sal_Bool SAL_CALL SwAccessibleEmbeddedObject::supportsService(const OUString& sTestServiceName)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleEmbeddedObject::getSupportedServiceNames()
{
    return { u"com.sun.star.text.AccessibleTextEmbeddedObject"_ustr, sAccessibleServiceName };
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleEmbeddedObject::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XAccessibleExtendedAttributes
css::uno::Any SAL_CALL SwAccessibleEmbeddedObject::getExtendedAttributes()
{
    SolarMutexGuard g;

    css::uno::Any strRet;
    OUString style;
    SwFlyFrame* pFFrame = getFlyFrame();

    if( pFFrame )
    {
        style = "style:";
        SwContentFrame* pCFrame;
        pCFrame = pFFrame->ContainsContent();
        if( pCFrame )
        {
            assert(pCFrame->IsNoTextFrame());
            SwContentNode *const pCNode = static_cast<SwNoTextFrame*>(pCFrame)->GetNode();
            if( pCNode )
            {
                style += static_cast<SwOLENode*>(pCNode)->GetOLEObj().GetStyleString();
            }
        }
        style += ";";
    }
    strRet <<= style;
    return strRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
