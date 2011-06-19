/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef VBA_GLOBAL_BASE_HXX
#define VBA_GLOBAL_BASE_HXX

#include "vbahelperinterface.hxx"
#include <ooo/vba/XGlobalsBase.hpp>

typedef InheritedHelperInterfaceImpl1< ov::XGlobalsBase > Globals_BASE;
class VBAHELPER_DLLPUBLIC VbaGlobalsBase : public Globals_BASE
{
protected:
    rtl::OUString msDocCtxName;

    bool hasServiceName( const rtl::OUString& serviceName );
    void init(  const css::uno::Sequence< css::beans::PropertyValue >& aInitArgs );

public:
    VbaGlobalsBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const rtl::OUString& sDocCtxName );
    virtual ~VbaGlobalsBase();
    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw (css::uno::Exception, css::uno::RuntimeException);
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw (css::uno::Exception, css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw (css::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
