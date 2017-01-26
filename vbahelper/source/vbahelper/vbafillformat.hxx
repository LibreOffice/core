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
#ifndef INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBAFILLFORMAT_HXX
#define INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBAFILLFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <ooo/vba/msforms/XFillFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::msforms::XFillFormat > ScVbaFillFormat_BASE;

class ScVbaFillFormat : public ScVbaFillFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    css::uno::Reference< ov::msforms::XColorFormat > m_xColorFormat;
    css::drawing::FillStyle m_nFillStyle;
    sal_Int32 m_nForeColor;
    sal_Int16 m_nGradientAngle;
private:
    /// @throws css::uno::RuntimeException
    void setFillStyle( css::drawing::FillStyle nFillStyle );
protected:
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

public:
    ScVbaFillFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape );

    /// @throws css::uno::RuntimeException
    void setForeColorAndInternalStyle( sal_Int32 nForeColor );
    // Attributes
    virtual sal_Bool SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) override;
    virtual double SAL_CALL getTransparency() override;
    virtual void SAL_CALL setTransparency( double _transparency ) override;

    // Methods
    virtual void SAL_CALL Solid() override;
    virtual void SAL_CALL TwoColorGradient( sal_Int32 style, sal_Int32 variant ) override;
    virtual css::uno::Reference< ov::msforms::XColorFormat > SAL_CALL BackColor() override;
    virtual css::uno::Reference< ov::msforms::XColorFormat > SAL_CALL ForeColor() override;

};

#endif // INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBAFILLFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
