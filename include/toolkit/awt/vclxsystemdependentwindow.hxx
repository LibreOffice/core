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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXSYSTEMDEPENDENTWINDOW_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXSYSTEMDEPENDENTWINDOW_HXX


#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <cppuhelper/weak.hxx>

#include <toolkit/awt/vclxwindow.hxx>

class TOOLKIT_DLLPUBLIC VCLXSystemDependentWindow : public css::awt::XSystemDependentWindowPeer,
                                                    public VCLXWindow
{
public:
    VCLXSystemDependentWindow();
    virtual ~VCLXSystemDependentWindow() override;

    // css::uno::XInterface
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XSystemDependentWindowPeer
    css::uno::Any SAL_CALL getWindowHandle( const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) override;
};


#endif // INCLUDED_TOOLKIT_AWT_VCLXSYSTEMDEPENDENTWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
