/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XWorkbook.hpp>
#include <cppuhelper/implbase.hxx>
#include <vbahelper/vbadocumentbase.hxx>
#include <docuno.hxx>

typedef cppu::ImplInheritanceHelper< VbaDocumentBase, ov::excel::XWorkbook > ScVbaWorkbook_BASE;

class ScVbaWorkbook : public ScVbaWorkbook_BASE
{
    static css::uno::Sequence< sal_Int32 > ColorData;
    static bool setFilterPropsFromFormat( sal_Int32 nFormat, css::uno::Sequence< css::beans::PropertyValue >& rProps );
    void init();

    rtl::Reference< ScModelObj > mxModel;

protected:
    virtual ScModelObj* getModel() const override { return mxModel.get(); }

public:
    ScVbaWorkbook(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
            rtl::Reference< ScModelObj > const & xModel );
    ScVbaWorkbook(  css::uno::Sequence< cpo::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

    // Attributes
    virtual bool SAL_CALL getProtectStructure() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() override;
    virtual bool SAL_CALL getPrecisionAsDisplayed() override;
    virtual void SAL_CALL setPrecisionAsDisplayed( bool _precisionAsDisplayed ) override;
    virtual OUString SAL_CALL getAuthor() override;
    virtual void SAL_CALL setAuthor( const OUString& _author ) override;

    // Methods
    virtual cpo::uno::Any SAL_CALL Worksheets( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Sheets( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Windows( const cpo::uno::Any& aIndex ) override;
    virtual void SAL_CALL Activate() override;
    virtual void SAL_CALL Protect( const cpo::uno::Any & aPassword ) override;
    virtual void SAL_CALL SaveAs( const cpo::uno::Any& FileName, const cpo::uno::Any& FileFormat, const cpo::uno::Any& Password, const cpo::uno::Any& WriteResPassword, const cpo::uno::Any& ReadOnlyRecommended, const cpo::uno::Any& CreateBackup, const cpo::uno::Any& AccessMode, const cpo::uno::Any& ConflictResolution, const cpo::uno::Any& AddToMru, const cpo::uno::Any& TextCodepage, const cpo::uno::Any& TextVisualLayout, const cpo::uno::Any& Local ) override;
    virtual cpo::uno::Any SAL_CALL Names( const cpo::uno::Any& aIndex ) override;

    virtual cpo::uno::Any SAL_CALL Styles( const cpo::uno::Any& Item ) override;
    virtual void SAL_CALL ResetColors(  ) override;
    virtual cpo::uno::Any SAL_CALL Colors( const cpo::uno::Any& Index ) override;
    virtual ::sal_Int32 SAL_CALL getFileFormat(  ) override;
    virtual void SAL_CALL SaveCopyAs( const OUString& Filename ) override;
    virtual void SAL_CALL ExportAsFixedFormat( const cpo::uno::Any& Type, const cpo::uno::Any& FileName, const cpo::uno::Any& Quality,
        const cpo::uno::Any& IncludeDocProperties, const cpo::uno::Any& IgnorePrintAreas, const cpo::uno::Any& From,
        const cpo::uno::Any& To, const cpo::uno::Any& OpenAfterPublish, const cpo::uno::Any& FixedFormatExtClassPtr) override;

    // code name
    virtual OUString SAL_CALL getCodeName() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8 >& rId ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
