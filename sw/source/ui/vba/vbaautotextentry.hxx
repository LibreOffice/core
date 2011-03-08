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
#ifndef SW_VBA_AUTOTEXTENTRY_HXX
#define SW_VBA_AUTOTEXTENTRY_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XAutoTextEntries.hpp>
#include <ooo/vba/word/XAutoTextEntry.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/word/XRange.hpp>
#include <com/sun/star/text/XAutoTextEntry.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XAutoTextEntry > SwVbaAutoTextEntry_BASE;

class SwVbaAutoTextEntry : public SwVbaAutoTextEntry_BASE
{
private:
    css::uno::Reference< css::text::XAutoTextEntry > mxEntry;

public:
    SwVbaAutoTextEntry( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XAutoTextEntry >& xEntry ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaAutoTextEntry();

    // XAutoTextEntry
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL Insert( const css::uno::Reference< ooo::vba::word::XRange >& _where, const css::uno::Any& _richtext ) throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};


/* class SwVbaAutoTextEntries */
typedef CollTestImplHelper< ooo::vba::word::XAutoTextEntries > SwVbaAutoTextEntries_BASE;

class SwVbaAutoTextEntries : public SwVbaAutoTextEntries_BASE
{
private:
    css::uno::Reference< css::container::XIndexAccess > mxAutoTextEntryAccess;

public:
    SwVbaAutoTextEntries( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess ) throw (css::uno::RuntimeException);
    virtual ~SwVbaAutoTextEntries() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaAutoTextEntries_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SW_VBA_AUTOTEXTENTRY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
