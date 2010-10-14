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
#ifndef SW_VBA_TEMPLATE_HXX
#define SW_VBA_TEMPLATE_HXX

#include <ooo/vba/word/XTemplate.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XTemplate > SwVbaTemplate_BASE;

class SwVbaTemplate : public SwVbaTemplate_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    rtl::OUString msFullUrl;
public:
    SwVbaTemplate( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::frame::XModel >& rModel, const rtl::OUString& );
    virtual ~SwVbaTemplate();

   // XTemplate
    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getPath() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL AutoTextEntries( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_TEMPLATE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
