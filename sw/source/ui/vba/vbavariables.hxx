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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAVARIABLES_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAVARIABLES_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XVariables.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/beans/XPropertyAccess.hpp>

typedef CollTestImplHelper< ooo::vba::word::XVariables > SwVbaVariables_BASE;

class SwVbaVariables : public SwVbaVariables_BASE
{
private:
    css::uno::Reference< css::beans::XPropertyAccess > mxUserDefined;

public:
    SwVbaVariables( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertyAccess >& rUserDefined );
    virtual ~SwVbaVariables() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaVariables_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XVariables
    virtual css::uno::Any SAL_CALL Add( const OUString& rName, const css::uno::Any& rValue ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAVARIABLES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
