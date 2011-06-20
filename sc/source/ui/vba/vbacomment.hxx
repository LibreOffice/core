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
#ifndef SC_VBA_COMMENT_HXX
#define SC_VBA_COMMENT_HXX

#include <cppuhelper/implbase1.hxx>

#include <ooo/vba/excel/XComment.hpp>
#include <ooo/vba/excel/XApplication.hpp>
#include <ooo/vba/msforms/XShape.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XComment > ScVbaComment_BASE;

class ScVbaComment : public ScVbaComment_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::table::XCellRange > mxRange;

private:
    css::uno::Reference< css::sheet::XSheetAnnotation > SAL_CALL getAnnotation() throw (css::uno::RuntimeException);
    css::uno::Reference< css::sheet::XSheetAnnotations > SAL_CALL getAnnotations() throw (css::uno::RuntimeException);
    sal_Int32 SAL_CALL getAnnotationIndex() throw (css::uno::RuntimeException);
    css::uno::Reference< ov::excel::XComment > SAL_CALL getCommentByIndex( sal_Int32 Index ) throw (css::uno::RuntimeException);
public:
    ScVbaComment(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::table::XCellRange >& xRange ) throw ( css::lang::IllegalArgumentException );

    virtual ~ScVbaComment() {}

    // Attributes
    virtual rtl::OUString SAL_CALL getAuthor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAuthor( const rtl::OUString& _author ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XShape > SAL_CALL getShape() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL Next() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL Previous() throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL Text( const css::uno::Any& Text, const css::uno::Any& Start, const css::uno::Any& Overwrite ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SC_VBA_COMMENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
