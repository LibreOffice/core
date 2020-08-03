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
#pragma once

#include <osl/conditn.hxx>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{

class ChartFrameLoader : public ::cppu::WeakImplHelper<
         css::frame::XSynchronousFrameLoader
         , css::lang::XServiceInfo
        >
{
private:
    css::uno::Reference< css::uno::XComponentContext>        m_xCC;
    bool            m_bCancelRequired;
    ::osl::Condition    m_oCancelFinished;

private:
    bool impl_checkCancel();

public:
    ChartFrameLoader() = delete;

    explicit ChartFrameLoader(css::uno::Reference< css::uno::XComponentContext > const & xContext);
    virtual ~ChartFrameLoader() override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::frame::XFrameLoader

    virtual sal_Bool SAL_CALL
        load( const css::uno::Sequence< css::beans::PropertyValue >& rMediaDescriptor
                ,const css::uno::Reference< css::frame::XFrame >& xFrame ) override;

    virtual void SAL_CALL
        cancel() override;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
