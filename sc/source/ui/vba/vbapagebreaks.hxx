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
#ifndef SC_VBA_PAGEBREAKS_HXX
#define SC_VBA_PAGEBREAKS_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XHPageBreaks.hpp>
#include <ooo/vba/excel/XHPageBreak.hpp>
#include <ooo/vba/excel/XVPageBreaks.hpp>
#include <ooo/vba/excel/XVPageBreak.hpp>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#include <com/sun/star/sheet/TablePageBreakData.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XHPageBreaks > ScVbaHPageBreaks_BASE;

class ScVbaHPageBreaks : public ScVbaHPageBreaks_BASE
{
    css::uno::Reference< css::sheet::XSheetPageBreak > mxSheetPageBreak;
public:
    ScVbaHPageBreaks( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    css::uno::Reference< css::sheet::XSheetPageBreak >& xSheetPageBreak) throw (css::uno::RuntimeException);
    virtual ~ScVbaHPageBreaks(){}

    // XHPageBreaks
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Before) throw ( css::script::BasicErrorException, css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject(const css::uno::Any&);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

//VPageBreaks
typedef CollTestImplHelper< ov::excel::XVPageBreaks > ScVbaVPageBreaks_BASE;

class ScVbaVPageBreaks : public ScVbaVPageBreaks_BASE
{
    css::uno::Reference< css::sheet::XSheetPageBreak > mxSheetPageBreak;

public:
    ScVbaVPageBreaks( const css::uno::Reference< ov::XHelperInterface >& xParent,
                      const css::uno::Reference< css::uno::XComponentContext >& xContext,
                      css::uno::Reference< css::sheet::XSheetPageBreak >& xSheetPageBreak ) throw ( css::uno::RuntimeException );

    virtual ~ScVbaVPageBreaks();

    // XVPageBreaks
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Before ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw ( css::uno::RuntimeException );
    virtual css::uno::Any createCollectionObject( const css::uno::Any& );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
