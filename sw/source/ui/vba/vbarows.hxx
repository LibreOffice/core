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
#ifndef SW_VBA_ROWS_HXX
#define SW_VBA_ROWS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XRows.hpp>
#include <ooo/vba/word/XColumns.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/text/XTextTable.hpp>

typedef CollTestImplHelper< ooo::vba::word::XRows > SwVbaRows_BASE;

class SwVbaRows : public SwVbaRows_BASE
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    css::uno::Reference< css::table::XTableRows > mxTableRows;
    sal_Int32 mnStartRowIndex;
    sal_Int32 mnEndRowIndex;

private:
    void setIndentWithAdjustNone( sal_Int32 indent ) throw (css::uno::RuntimeException);
    void setIndentWithAdjustFirstColumn( const css::uno::Reference< ooo::vba::word::XColumns >& xColumns, sal_Int32 indent ) throw (css::uno::RuntimeException);
    void setIndentWithAdjustProportional( const css::uno::Reference< ooo::vba::word::XColumns >& xColumns, sal_Int32 indent ) throw (css::uno::RuntimeException);
    void setIndentWithAdjustSameWidth( const css::uno::Reference< ooo::vba::word::XColumns >& xColumns, sal_Int32 indent ) throw (css::uno::RuntimeException);

public:
    SwVbaRows( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, const css::uno::Reference< css::table::XTableRows >& xTableRows ) throw ( css::uno::RuntimeException );
    SwVbaRows( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, const css::uno::Reference< css::table::XTableRows >& xTableRows, sal_Int32 nStarIndex, sal_Int32 nEndIndex ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaRows() {}

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getAllowBreakAcrossPages() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAllowBreakAcrossPages( const css::uno::Any& _allowbreakacrosspages ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getSpaceBetweenColumns() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSpaceBetweenColumns( float _spacebetweencolumns ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL SetLeftIndent( float LeftIndent, ::sal_Int32 RulerStyle ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException);

    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& /*not processed in this base class*/ ) throw ( css::uno::RuntimeException );
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaRows_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SW_VBA_ROWS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
