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
#ifndef INCLUDED_SVX_VERTTEXTTBXCTRL_HXX
#define INCLUDED_SVX_VERTTEXTTBXCTRL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <svtools/toolboxcontroller.hxx>

#include <svx/svxdllapi.h>

//HACK to avoid duplicate ImplInheritanceHelper symbols with MSVC:
class SAL_DLLPUBLIC_TEMPLATE SvxVertCTLTextTbxCtrl_Base:
    public cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo>
{
    using ImplInheritanceHelper::ImplInheritanceHelper;
};

/*
  control to remove/insert cjk settings dependent vertical text toolbox item
 */
class SvxVertCTLTextTbxCtrl : public SvxVertCTLTextTbxCtrl_Base
{
    bool m_bVisible;
public:
    explicit SvxVertCTLTextTbxCtrl(const css::uno::Reference<css::uno::XComponentContext>& rContext);

    virtual ~SvxVertCTLTextTbxCtrl() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;
};

class SvxCTLTextTbxCtrl final : public SvxVertCTLTextTbxCtrl
{
public:
    SvxCTLTextTbxCtrl(const css::uno::Reference<css::uno::XComponentContext>& rContext);

    virtual OUString SAL_CALL getImplementationName() override;
};

class SvxVertTextTbxCtrl final : public SvxVertCTLTextTbxCtrl
{
public:
    SvxVertTextTbxCtrl(const css::uno::Reference<css::uno::XComponentContext>& rContext);

    virtual OUString SAL_CALL getImplementationName() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
