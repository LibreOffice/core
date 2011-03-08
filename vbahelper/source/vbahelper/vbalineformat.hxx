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
#ifndef SC_VBA_XLINEFORMAT_HXX
#define SC_VBA_XLINEFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XLineFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::msforms::XLineFormat > ScVbaLineFormat_BASE;

class ScVbaLineFormat : public ScVbaLineFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nLineDashStyle;
    double m_nLineWeight;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    sal_Int32 convertLineStartEndNameToArrowheadStyle( rtl::OUString sLineName );
    rtl::OUString convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle ) throw (css::uno::RuntimeException);
    sal_Int32 calculateArrowheadSize() const;
public:
    ScVbaLineFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape > xShape );

    // Attributes
    virtual sal_Int32 SAL_CALL getBeginArrowheadStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBeginArrowheadStyle( sal_Int32 _beginarrowheadstyle ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBeginArrowheadLength() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBeginArrowheadLength( sal_Int32 _beginarrowheadlength ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBeginArrowheadWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBeginArrowheadWidth( sal_Int32 _beginarrowheadwidth ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndArrowheadStylel() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEndArrowheadStylel( sal_Int32 _endarrowheadstylel ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndArrowheadLength() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEndArrowheadLength( sal_Int32 _endarrowheadlength ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndArrowheadWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEndArrowheadWidth( sal_Int32 _endarrowheadwidth ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWeight( double _weight ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTransparency() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTransparency( double _transparency ) throw (css::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( sal_Int16 _style ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDashStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDashStyle( sal_Int32 _dashstyle ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Reference< ov::msforms::XColorFormat > SAL_CALL BackColor() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XColorFormat > SAL_CALL ForeColor() throw (css::uno::RuntimeException);
};

#endif//SC_VBA_XLINEFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
