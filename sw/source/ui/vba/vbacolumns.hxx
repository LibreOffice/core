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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBACOLUMNS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBACOLUMNS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XColumns.hpp>
#include <ooo/vba/word/XColumn.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/text/XTextTable.hpp>

typedef CollTestImplHelper< ooo::vba::word::XColumns > SwVbaColumns_BASE;

class SwVbaColumns : public SwVbaColumns_BASE
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    css::uno::Reference< css::table::XTableColumns > mxTableColumns;
    sal_Int32 mnStartColumnIndex;
    sal_Int32 mnEndColumnIndex;

private:
    css::uno::Reference< ooo::vba::word::XColumn > getColumnAtIndex( sal_Int32 index ) throw (css::uno::RuntimeException);

public:
    SwVbaColumns( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, const css::uno::Reference< css::table::XTableColumns >& xTableColumns ) throw ( css::uno::RuntimeException );
    SwVbaColumns( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, const css::uno::Reference< css::table::XTableColumns >& xTableColumns, sal_Int32 nStartCol, sal_Int32 nEndCol ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaColumns() {}

    virtual sal_Int32 SAL_CALL getWidth(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setWidth( sal_Int32 _width ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException, std::exception) override;

    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& /*not processed in this base class*/ ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException ) override;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaColumns_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBACOLUMNS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
