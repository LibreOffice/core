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
#ifndef SC_VBA_PAGEBREAK_HXX
#define SC_VBA_PAGEBREAK_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XPageBreak.hpp>
#include <ooo/vba/excel/XHPageBreak.hpp>
#include <ooo/vba/excel/XVPageBreak.hpp>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/sheet/TablePageBreakData.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

template< typename Ifc1 >
class ScVbaPageBreak : public InheritedHelperInterfaceImpl1< Ifc1 >
{
typedef InheritedHelperInterfaceImpl1< Ifc1 > ScVbaPageBreak_BASE;
protected:
    css::uno::Reference< css::beans::XPropertySet > mxRowColPropertySet;
    css::sheet::TablePageBreakData maTablePageBreakData;
public:
    ScVbaPageBreak( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    css::uno::Reference< css::beans::XPropertySet >& xProps,
                    css::sheet::TablePageBreakData aTablePageBreakData) throw (css::uno::RuntimeException);
    virtual ~ScVbaPageBreak(){}

    virtual sal_Int32 SAL_CALL getType( ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setType(sal_Int32 type) throw (css::uno::RuntimeException);

    virtual void SAL_CALL Delete() throw ( css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange> SAL_CALL Location() throw ( css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};


typedef ScVbaPageBreak < ov::excel::XHPageBreak > ScVbaHPageBreak_BASE;

class ScVbaHPageBreak :  public ScVbaHPageBreak_BASE
{
public:
    ScVbaHPageBreak( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    css::uno::Reference< css::beans::XPropertySet >& xProps,
                    css::sheet::TablePageBreakData aTablePageBreakData) throw (css::uno::RuntimeException):
              ScVbaHPageBreak_BASE( xParent,xContext,xProps,aTablePageBreakData ){}

    virtual ~ScVbaHPageBreak(){}

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

//VPageBreak
typedef ScVbaPageBreak < ov::excel::XVPageBreak > ScVbaVPageBreak_BASE;

class ScVbaVPageBreak :  public ScVbaVPageBreak_BASE
{
public:
    ScVbaVPageBreak( const css::uno::Reference< ov::XHelperInterface >& xParent,
                     const css::uno::Reference< css::uno::XComponentContext >& xContext,
                     css::uno::Reference< css::beans::XPropertySet >& xProps,
                     css::sheet::TablePageBreakData aTablePageBreakData) throw (css::uno::RuntimeException);

    virtual ~ScVbaVPageBreak();

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
