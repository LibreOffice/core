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
#include "Tools.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include <comphelper/property.hxx>


namespace reportdesign
{

using namespace com::sun::star;

uno::Reference< report::XSection> lcl_getSection(const uno::Reference< uno::XInterface>& _xReportComponent)
{
    uno::Reference< container::XChild> xChild(_xReportComponent,uno::UNO_QUERY);
    uno::Reference< report::XSection> xRet(_xReportComponent,uno::UNO_QUERY);
    while( !xRet.is() && xChild.is() )
    {
        uno::Reference< uno::XInterface> xTemp = xChild->getParent();
        xChild.set(xTemp,uno::UNO_QUERY);
        xRet.set(xTemp,uno::UNO_QUERY);
    }
    return xRet;
}

void throwIllegallArgumentException( const OUString& _sTypeName
                                    ,const uno::Reference< uno::XInterface >& ExceptionContext_
                                    ,const ::sal_Int16& ArgumentPosition_
                                    ,const css::uno::Reference< css::uno::XComponentContext >& Context_
                                    )
{
    OUString sErrorMessage(RPT_RESSTRING(RID_STR_ERROR_WRONG_ARGUMENT,Context_->getServiceManager()));
    sErrorMessage = sErrorMessage.replaceAt(sErrorMessage.indexOf('#'),2,_sTypeName);
    throw lang::IllegalArgumentException(sErrorMessage,ExceptionContext_,ArgumentPosition_);
}

uno::Reference< util::XCloneable > cloneObject(const uno::Reference< report::XReportComponent>& _xReportComponent
    ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
    ,const OUString& _sServiceName)
{
    OSL_ENSURE(_xReportComponent.is() && _xFactory.is() ,"reportcomponent is null -> GPF");
    uno::Reference< report::XReportComponent> xClone(_xFactory->createInstance(_sServiceName),uno::UNO_QUERY_THROW);
    ::comphelper::copyProperties(_xReportComponent.get(),xClone.get());
    return xClone.get();
}



} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
