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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXPOINTER_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXPOINTER_HXX


#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

#include <vcl/ptrstyle.hxx>


//  class VCLXPointer


class VCLXPointer final : public cppu::WeakImplHelper<
    css::awt::XPointer, css::lang::XUnoTunnel, css::lang::XServiceInfo>
{
    ::osl::Mutex    maMutex;
    PointerStyle    maPointer;

    ::osl::Mutex&   GetMutex() { return maMutex; }

public:
    VCLXPointer();
    virtual ~VCLXPointer() override;

    PointerStyle GetPointer() const { return maPointer; }

    // css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXPointer*                                         GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) override;

    // css::awt::XPointer
    void SAL_CALL setType( sal_Int32 nType ) override;
    sal_Int32 SAL_CALL getType(  ) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


#endif // INCLUDED_TOOLKIT_AWT_VCLXPOINTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
