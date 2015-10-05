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
    css::uno::Reference< css::text::XAutoTextEntry > mxEntry;

public:
    SwVbaAutoTextEntry( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XAutoTextEntry >& xEntry ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaAutoTextEntry();

    // XAutoTextEntry
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL Insert( const css::uno::Reference< ooo::vba::word::XRange >& _where, const css::uno::Any& _richtext ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

typedef CollTestImplHelper< ooo::vba::word::XAutoTextEntries > SwVbaAutoTextEntries_BASE;

class SwVbaAutoTextEntries : public SwVbaAutoTextEntries_BASE
{
private:
    css::uno::Reference< css::container::XIndexAccess > mxAutoTextEntryAccess;

public:
    SwVbaAutoTextEntries( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess ) throw (css::uno::RuntimeException);
    virtual ~SwVbaAutoTextEntries() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) SAL_OVERRIDE;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) SAL_OVERRIDE;

    // SwVbaAutoTextEntries_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) SAL_OVERRIDE;
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAAUTOTEXTENTRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
