/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#ifndef SW_VBA_CELLS_HXX
#define SW_VBA_CELLS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XCells.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextTable.hpp>

typedef CollTestImplHelper< ooo::vba::word::XCells > SwVbaCells_BASE;

class SwVbaCells : public SwVbaCells_BASE
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    sal_Int32 mnTop;
    sal_Int32 mnBottom;

public:
    SwVbaCells( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaCells() {}

    // Attributes
    virtual ::sal_Int32 SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( ::sal_Int32 _width ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( const css::uno::Any& _height ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getHeightRule() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL SetWidth( float width, sal_Int32 rulestyle ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaCells_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SW_VBA_CELLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
