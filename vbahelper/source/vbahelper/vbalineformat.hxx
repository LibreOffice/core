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
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
    sal_Int32 convertLineStartEndNameToArrowheadStyle( OUString sLineName );
    OUString convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle ) throw (css::uno::RuntimeException);
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
