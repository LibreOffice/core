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
#ifndef INCLUDED_VBAHELPER_VBAPAGESETUPBASE_HXX
#define INCLUDED_VBAHELPER_VBAPAGESETUPBASE_HXX

#include <ooo/vba/XPageSetupBase.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::XPageSetupBase > VbaPageSetupBase_BASE;

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
    virtual double SAL_CALL getTopMargin() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTopMargin( double margin ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getBottomMargin() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBottomMargin( double margin ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getRightMargin() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRightMargin( double margin ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getLeftMargin() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeftMargin( double margin ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getHeaderMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeaderMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getFooterMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFooterMargin( double margin ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getOrientation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (css::uno::RuntimeException, std::exception) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
