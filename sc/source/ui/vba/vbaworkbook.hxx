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
#ifndef SC_VBA_WORKBOOK_HXX
#define SC_VBA_WORKBOOK_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XWorkbook.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadocumentbase.hxx>

typedef cppu::ImplInheritanceHelper1< VbaDocumentBase, ov::excel::XWorkbook > ScVbaWorkbook_BASE;

class ScVbaWorkbook : public ScVbaWorkbook_BASE
{
    static css::uno::Sequence< sal_Int32 > ColorData;
    void initColorData( const css::uno::Sequence< sal_Int32 >& sColors );
    void init();

public:
    ScVbaWorkbook(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
            css::uno::Reference< css::frame::XModel > xModel );
    ScVbaWorkbook(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~ScVbaWorkbook() {}

    // Attributes
    virtual ::sal_Bool SAL_CALL getProtectStructure() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getPrecisionAsDisplayed() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrecisionAsDisplayed( sal_Bool _precisionAsDisplayed ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Protect( const css::uno::Any & aPassword ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL Styles( const css::uno::Any& Item ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ResetColors(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Colors( const css::uno::Any& Index ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getFileFormat(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL SaveCopyAs( const OUString& Filename ) throw ( css::uno::RuntimeException);

    // code name
    virtual OUString SAL_CALL getCodeName() throw ( css::uno::RuntimeException);

    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8 >& rId ) throw(css::uno::RuntimeException);
};

#endif /* SC_VBA_WORKBOOK_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
