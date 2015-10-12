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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATABSTOPS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATABSTOPS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XTabStops.hpp>
#include <ooo/vba/word/XTabStop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef CollTestImplHelper< ooo::vba::word::XTabStops > SwVbaTabStops_BASE;

class SwVbaTabStops : public SwVbaTabStops_BASE
{
private:
    css::uno::Reference< css::beans::XPropertySet > mxParaProps;

public:
    SwVbaTabStops( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& xParaProps ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaTabStops() {}

    // Methods
    virtual css::uno::Reference< ::ooo::vba::word::XTabStop > SAL_CALL Add( float Position, const css::uno::Any& Alignment, const css::uno::Any& Leader )
        throw (css::script::BasicErrorException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL ClearAll(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaTabStops_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBATABSTOPS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
