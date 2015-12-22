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

#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_URIABBREVIATION_HXX
#define INCLUDED_FRAMEWORK_INC_SERVICES_URIABBREVIATION_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XStringAbbreviation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>

namespace framework
{

class UriAbbreviation:    public ::cppu::WeakImplHelper< css::util::XStringAbbreviation, css::lang::XServiceInfo>
{
public:
    explicit UriAbbreviation(css::uno::Reference< css::uno::XComponentContext > const & context);

    DECLARE_XSERVICEINFO

    // css::util::XStringAbbreviation:
    virtual OUString SAL_CALL abbreviateString(const css::uno::Reference< css::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const OUString & aString) throw (css::uno::RuntimeException, std::exception) override;

private:
    UriAbbreviation(UriAbbreviation &) = delete;
    void operator =(UriAbbreviation &) = delete;

    virtual ~UriAbbreviation() {}
};

} //    namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_URIABBREVIATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
