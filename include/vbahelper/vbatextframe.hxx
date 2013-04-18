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
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
    virtual void setAsMSObehavior();
    sal_Int32 getMargin( OUString sMarginType );
    void setMargin( OUString sMarginType, float fMargin );
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
