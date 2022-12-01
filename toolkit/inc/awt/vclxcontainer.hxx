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

#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <cppuhelper/implbase.hxx>

#include <toolkit/awt/vclxwindow.hxx>

class VCLXContainer : public cppu::ImplInheritanceHelper<VCLXWindow,
                        css::awt::XVclContainer,
                        css::awt::XVclContainerPeer>
{
public:
                    VCLXContainer();
                    virtual ~VCLXContainer() override;

    // css::awt::XVclContainer
    void SAL_CALL addVclContainerListener( const css::uno::Reference< css::awt::XVclContainerListener >& l ) override;
    void SAL_CALL removeVclContainerListener( const css::uno::Reference< css::awt::XVclContainerListener >& l ) override;
    css::uno::Sequence< css::uno::Reference< css::awt::XWindow > > SAL_CALL getWindows(  ) override;

    // css::awt::XVclContainerPeer
    void SAL_CALL enableDialogControl( sal_Bool bEnable ) override;
    void SAL_CALL setTabOrder( const css::uno::Sequence< css::uno::Reference< css::awt::XWindow > >& WindowOrder, const css::uno::Sequence< css::uno::Any >& Tabs, sal_Bool GroupControl ) override;
    void SAL_CALL setGroup( const css::uno::Sequence< css::uno::Reference< css::awt::XWindow > >& Windows ) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
