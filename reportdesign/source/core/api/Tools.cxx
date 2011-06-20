/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_reportdesign.hxx"
#include "Tools.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include <comphelper/property.hxx>

// =============================================================================
namespace reportdesign
{
// =============================================================================
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
// -----------------------------------------------------------------------------
void throwIllegallArgumentException( const ::rtl::OUString& _sTypeName
                                    ,const uno::Reference< uno::XInterface >& ExceptionContext_
                                    ,const ::sal_Int16& ArgumentPosition_
                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context_
                                    )
{
    ::rtl::OUString sErrorMessage(RPT_RESSTRING(RID_STR_ERROR_WRONG_ARGUMENT,Context_->getServiceManager()));
    sErrorMessage = sErrorMessage.replaceAt(sErrorMessage.indexOf('#'),2,_sTypeName);
    throw lang::IllegalArgumentException(sErrorMessage,ExceptionContext_,ArgumentPosition_);
}
// -----------------------------------------------------------------------------
uno::Reference< util::XCloneable > cloneObject(const uno::Reference< report::XReportComponent>& _xReportComponent
    ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
    ,const ::rtl::OUString& _sServiceName)
{
    OSL_ENSURE(_xReportComponent.is() && _xFactory.is() ,"reportcomponent is null -> GPF");
    uno::Reference< report::XReportComponent> xClone(_xFactory->createInstance(_sServiceName),uno::UNO_QUERY_THROW);
    ::comphelper::copyProperties(_xReportComponent.get(),xClone.get());
    return xClone.get();
}
// -----------------------------------------------------------------------------

// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
