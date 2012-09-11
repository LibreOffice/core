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
#ifndef SC_VBA_NAME_HXX
#define SC_VBA_NAME_HXX

#include <ooo/vba/excel/XName.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <formula/grammar.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XName > NameImpl_BASE;

class ScVbaName : public NameImpl_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRange > mxNamedRange;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;

protected:
    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }
    virtual css::uno::Reference< ov::excel::XWorksheet > getWorkSheet() throw (css::uno::RuntimeException);
    // Get value by FormulaGrammar, such as FormulaGrammar::GRAM_NATIVE_XL_R1C1
    virtual ::rtl::OUString SAL_CALL getValue(const formula::FormulaGrammar::Grammar eGrammar) throw (css::uno::RuntimeException);

public:
    ScVbaName( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XNamedRange >& xName , const css::uno::Reference< css::sheet::XNamedRanges >& xNames , const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~ScVbaName();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameLocal() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNameLocal( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString &rValue ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersTo() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersTo( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersToLocal() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToLocal( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersToR1C1() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToR1C1( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRefersToR1C1Local() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToR1C1Local( const ::rtl::OUString &rRefersTo ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getRefersToRange() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRefersToRange( const css::uno::Reference< ov::excel::XRange > xRange ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_NAME_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
