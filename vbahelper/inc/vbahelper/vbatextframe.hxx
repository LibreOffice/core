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
#ifndef VBA_TEXTFRAME_HXX
#define VBA_TEXTFRAME_HXX
#include <ooo/vba/msforms/XTextFrame.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::msforms::XTextFrame > VbaTextFrame_BASE;

class VBAHELPER_DLLPUBLIC VbaTextFrame : public VbaTextFrame_BASE
{
protected:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    virtual void setAsMSObehavior();
    sal_Int32 getMargin( rtl::OUString sMarginType );
    void setMargin( rtl::OUString sMarginType, float fMargin );
public:
    VbaTextFrame( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext , css::uno::Reference< css::drawing::XShape > xShape);
    virtual ~VbaTextFrame() {}
    // Attributes
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoSize( sal_Bool _autosize ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginBottom() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginBottom( float _marginbottom ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginTop( float _margintop ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginLeft( float _marginleft ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginRight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginRight( float _marginright ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Characters(  ) throw (css::uno::RuntimeException);

};

#endif//SC_VBA_TEXTFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
