/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAAUTOTEXTENTRY_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAAUTOTEXTENTRY_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XAutoTextEntries.hpp>
#include <ooo/vba/word/XAutoTextEntry.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <ooo/vba/word/XRange.hpp>
#include <com/sun/star/text/XAutoTextEntry.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XAutoTextEntry > SwVbaAutoTextEntry_BASE;

class SwVbaAutoTextEntry : public SwVbaAutoTextEntry_BASE
{
private:
    cpo::uno::Reference< css::text::XAutoTextEntry > mxEntry;

public:
    /// @throws cpo::uno::RuntimeException
    SwVbaAutoTextEntry( const cpo::uno::Reference< ooo::vba::XHelperInterface >& rParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& rContext, cpo::uno::Reference< css::text::XAutoTextEntry > xEntry );
    virtual ~SwVbaAutoTextEntry() override;

    // XAutoTextEntry
    virtual cpo::uno::Reference< ooo::vba::word::XRange > SAL_CALL Insert( const cpo::uno::Reference< ooo::vba::word::XRange >& _where, const cpo::uno::Any& _richtext ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

typedef CollTestImplHelper< ooo::vba::word::XAutoTextEntries > SwVbaAutoTextEntries_BASE;

class SwVbaAutoTextEntries : public SwVbaAutoTextEntries_BASE
{
public:
    /// @throws cpo::uno::RuntimeException
    SwVbaAutoTextEntries( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext, const cpo::uno::Reference< css::container::XIndexAccess >& xIndexAccess );

    // XEnumerationAccess
    virtual cpo::uno::Type SAL_CALL getElementType() override;
    virtual cpo::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // SwVbaAutoTextEntries_BASE
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAAUTOTEXTENTRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
