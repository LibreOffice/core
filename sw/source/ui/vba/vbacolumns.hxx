/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#ifndef SW_VBA_COLUMNS_HXX
#define SW_VBA_COLUMNS_HXX

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

    virtual sal_Int32 SAL_CALL getWidth(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( sal_Int32 _width ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException);

    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& /*not processed in this base class*/ ) throw ( css::uno::RuntimeException );
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaColumns_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SW_VBA_COLUMNS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
