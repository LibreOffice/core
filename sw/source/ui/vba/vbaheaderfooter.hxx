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
#ifndef SW_VBA_HEADERFOOTER_HXX
#define SW_VBA_HEADERFOOTER_HXX

#include <ooo/vba/word/XHeaderFooter.hpp>
#include <ooo/vba/word/XRange.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XHeaderFooter > SwVbaHeaderFooter_BASE;

class SwVbaHeaderFooter : public SwVbaHeaderFooter_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageStyleProps;
    sal_Bool mbHeader;
    sal_Int32 mnIndex;

public:
    SwVbaHeaderFooter( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::beans::XPropertySet >& xProps, sal_Bool isHeader, sal_Int32 index ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaHeaderFooter(){}

    // Attributes
    virtual ::sal_Bool SAL_CALL getIsHeader() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getLinkToPrevious() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLinkToPrevious( ::sal_Bool _linktoprevious ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::word::XRange > SAL_CALL getRange() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_HEADERFOOTER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
