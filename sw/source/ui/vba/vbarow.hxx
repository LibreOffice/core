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
#ifndef SW_VBA_ROW_HXX
#define SW_VBA_ROW_HXX

#include <ooo/vba/word/XRow.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/text/XTextTable.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XRow > SwVbaRow_BASE;

class SwVbaRow : public SwVbaRow_BASE
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    css::uno::Reference< css::table::XTableRows > mxTableRows;
    css::uno::Reference< css::beans::XPropertySet > mxRowProps;
    sal_Int32 mnIndex;

public:
    SwVbaRow( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, sal_Int32 nIndex ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaRow();

    // Attributes
    virtual css::uno::Any SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( const css::uno::Any& _height ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getHeightRule() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException);

    static void SelectRow( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextTable >& xTextTable, sal_Int32 nStartRow, sal_Int32 nEndRow ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_ROW_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
