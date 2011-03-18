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
#ifndef SC_VBA_PICTUREFORMAT_HXX
#define SC_VBA_PICTUREFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XPictureFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::msforms::XPictureFormat > ScVbaPictureFormat_BASE;

class ScVbaPictureFormat : public ScVbaPictureFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
private:
    void checkParameterRangeInDouble( double nRange, double nMin, double nMax ) throw (css::uno::RuntimeException);
public:
    ScVbaPictureFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::drawing::XShape > xShape ) throw( css::lang::IllegalArgumentException );

    // Attributes
    virtual double SAL_CALL getBrightness() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBrightness( double _brightness ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getContrast() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setContrast( double _contrast ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL IncrementBrightness( double increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementContrast( double increment ) throw (css::uno::RuntimeException);
};

#endif//SC_VBA_PICTUREFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
