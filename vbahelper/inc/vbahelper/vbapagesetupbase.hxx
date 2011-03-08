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
#ifndef VBA_PAGESETUPBASE_HXX
#define VBA_PAGESETUPBASE_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XPageSetupBase.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::XPageSetupBase > VbaPageSetupBase_BASE;

class VBAHELPER_DLLPUBLIC VbaPageSetupBase :  public VbaPageSetupBase_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageProps;
    sal_Int32 mnOrientLandscape;
    sal_Int32 mnOrientPortrait;

    VbaPageSetupBase( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
public:
    virtual ~VbaPageSetupBase(){}

    // Attribute
    virtual double SAL_CALL getTopMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTopMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getBottomMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBottomMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getRightMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRightMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getLeftMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeftMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeaderMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeaderMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getFooterMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFooterMargin( double margin ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getOrientation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (css::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
