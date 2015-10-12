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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBALISTLEVELS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBALISTLEVELS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XListLevels.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbalisthelper.hxx"

typedef CollTestImplHelper< ooo::vba::word::XListLevels > SwVbaListLevels_BASE;

class SwVbaListLevels : public SwVbaListLevels_BASE
{
private:
    SwVbaListHelperRef pListHelper;

public:
    SwVbaListLevels( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, SwVbaListHelperRef pHelper ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaListLevels() {}

    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& /*not processed in this base class*/ ) throw ( css::uno::RuntimeException ) override;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaListLevels_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBALISTLEVELS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
