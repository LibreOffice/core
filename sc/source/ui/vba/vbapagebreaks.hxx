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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAPAGEBREAKS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAPAGEBREAKS_HXX

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
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Before) throw ( css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;
    virtual css::uno::Any createCollectionObject(const css::uno::Any&) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
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
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Before ) throw ( css::script::BasicErrorException, css::uno::RuntimeException, std::exception ) override;

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw ( css::uno::RuntimeException ) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw ( css::uno::RuntimeException ) override;
    virtual css::uno::Any createCollectionObject( const css::uno::Any& ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
