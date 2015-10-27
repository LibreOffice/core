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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAWORKBOOK_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAWORKBOOK_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XWorkbook.hpp>
#include <cppuhelper/implbase.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadocumentbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaDocumentBase, ov::excel::XWorkbook > ScVbaWorkbook_BASE;

class ScVbaWorkbook : public ScVbaWorkbook_BASE
{
    static css::uno::Sequence< sal_Int32 > ColorData;
    static bool setFilterPropsFromFormat( sal_Int32 nFormat, css::uno::Sequence< css::beans::PropertyValue >& rProps );
    static void initColorData( const css::uno::Sequence< sal_Int32 >& sColors );
    void init();

public:
    ScVbaWorkbook(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
            css::uno::Reference< css::frame::XModel > xModel );
    ScVbaWorkbook(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~ScVbaWorkbook() {}

    // Attributes
    virtual sal_Bool SAL_CALL getProtectStructure() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getPrecisionAsDisplayed() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrecisionAsDisplayed( sal_Bool _precisionAsDisplayed ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Sheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Protect( const css::uno::Any & aPassword ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL SaveAs( const css::uno::Any& FileName, const css::uno::Any& FileFormat, const css::uno::Any& Password, const css::uno::Any& WriteResPassword, const css::uno::Any& ReadOnlyRecommended, const css::uno::Any& CreateBackup, const css::uno::Any& AccessMode, const css::uno::Any& ConflictResolution, const css::uno::Any& AddToMru, const css::uno::Any& TextCodepage, const css::uno::Any& TextVisualLayout, const css::uno::Any& Local ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL Styles( const css::uno::Any& Item ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ResetColors(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Colors( const css::uno::Any& Index ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getFileFormat(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL SaveCopyAs( const OUString& Filename ) throw ( css::uno::RuntimeException, std::exception) override;

    // code name
    virtual OUString SAL_CALL getCodeName() throw ( css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAWORKBOOK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
