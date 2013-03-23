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
#pragma once
#if 1

#include <ooo/vba/excel/XName.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

#include <vbahelper/vbahelperinterface.hxx>

class ScDocument;

typedef InheritedHelperInterfaceImpl1< ov::excel::XName > NameImpl_BASE;

class ScVbaName : public NameImpl_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XNamedRange > mxNamedRange;
    css::uno::Reference< css::sheet::XNamedRanges > mxNames;

protected:
    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }
    virtual css::uno::Reference< ov::excel::XWorksheet > getWorkSheet() throw (css::uno::RuntimeException);

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
