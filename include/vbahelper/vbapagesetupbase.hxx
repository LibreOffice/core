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

#include <exception>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <ooo/vba/XPageSetupBase.hpp>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace frame { class XModel; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class XHelperInterface;
} }

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::XPageSetupBase > VbaPageSetupBase_BASE;

class VBAHELPER_DLLPUBLIC VbaPageSetupBase :  public VbaPageSetupBase_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageProps;
    sal_Int32 mnOrientLandscape;
    sal_Int32 mnOrientPortrait;

    /// @throws css::uno::RuntimeException
    VbaPageSetupBase( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext );
public:

    // Attribute
    virtual double SAL_CALL getTopMargin() override;
    virtual void SAL_CALL setTopMargin( double margin ) override;
    virtual double SAL_CALL getBottomMargin() override;
    virtual void SAL_CALL setBottomMargin( double margin ) override;
    virtual double SAL_CALL getRightMargin() override;
    virtual void SAL_CALL setRightMargin( double margin ) override;
    virtual double SAL_CALL getLeftMargin() override;
    virtual void SAL_CALL setLeftMargin( double margin ) override;
    /// @throws css::uno::RuntimeException
    virtual double SAL_CALL getHeaderMargin();
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setHeaderMargin( double margin );
    /// @throws css::uno::RuntimeException
    virtual double SAL_CALL getFooterMargin();
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setFooterMargin( double margin );
    virtual sal_Int32 SAL_CALL getOrientation() override;
    virtual void SAL_CALL setOrientation( sal_Int32 orientation ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
