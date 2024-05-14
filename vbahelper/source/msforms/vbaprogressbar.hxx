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

#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAPROGRESSBAR_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAPROGRESSBAR_HXX

#include <ooo/vba/msforms/XProgressBar.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>
#include <cppuhelper/implbase.hxx>

typedef cppu::ImplInheritanceHelper< ScVbaControl, ov::msforms::XProgressBar, css::script::XDefaultProperty  > ProgressBarImpl_BASE;

class ScVbaProgressBar : public ProgressBarImpl_BASE
{
public:
    ScVbaProgressBar( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper );
   // Attributes
    virtual css::uno::Any SAL_CALL getValue() override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    // XDefaultProperty
    OUString SAL_CALL getDefaultPropertyName(  ) override { return u"Value"_ustr; }
};

#endif // INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAPROGRESSBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
