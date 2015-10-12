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
#ifndef INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBAPICTUREFORMAT_HXX
#define INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBAPICTUREFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XPictureFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::msforms::XPictureFormat > ScVbaPictureFormat_BASE;

class ScVbaPictureFormat : public ScVbaPictureFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
private:
    static void checkParameterRangeInDouble( double nRange, double nMin, double nMax ) throw (css::uno::RuntimeException);
public:
    ScVbaPictureFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::drawing::XShape > xShape )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    // Attributes
    virtual double SAL_CALL getBrightness() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBrightness( double _brightness ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getContrast() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setContrast( double _contrast ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual void SAL_CALL IncrementBrightness( double increment ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL IncrementContrast( double increment ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBAPICTUREFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
