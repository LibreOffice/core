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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAROWS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAROWS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XRows.hpp>
#include <ooo/vba/word/XColumns.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/text/XTextTable.hpp>

typedef CollTestImplHelper< ooo::vba::word::XRows > SwVbaRows_BASE;

class SwVbaRows : public SwVbaRows_BASE
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    css::uno::Reference< css::table::XTableRows > mxTableRows;
    sal_Int32 mnStartRowIndex;
    sal_Int32 mnEndRowIndex;

private:
    /// @throws css::uno::RuntimeException
    void setIndentWithAdjustNone( sal_Int32 indent );
    /// @throws css::uno::RuntimeException
    void setIndentWithAdjustFirstColumn( const css::uno::Reference< ooo::vba::word::XColumns >& xColumns, sal_Int32 indent );
    /// @throws css::uno::RuntimeException
    void setIndentWithAdjustProportional( const css::uno::Reference< ooo::vba::word::XColumns >& xColumns, sal_Int32 indent );
    /// @throws css::uno::RuntimeException
    void setIndentWithAdjustSameWidth( const css::uno::Reference< ooo::vba::word::XColumns >& xColumns, sal_Int32 indent );

public:
    /// @throws css::uno::RuntimeException
    SwVbaRows( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, const css::uno::Reference< css::table::XTableRows >& xTableRows );
    /// @throws css::uno::RuntimeException
    SwVbaRows( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, const css::uno::Reference< css::table::XTableRows >& xTableRows, sal_Int32 nStarIndex, sal_Int32 nEndIndex );

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() override;
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) override;
    virtual css::uno::Any SAL_CALL getAllowBreakAcrossPages() override;
    virtual void SAL_CALL setAllowBreakAcrossPages( const css::uno::Any& _allowbreakacrosspages ) override;
    virtual float SAL_CALL getSpaceBetweenColumns() override;
    virtual void SAL_CALL setSpaceBetweenColumns( float _spacebetweencolumns ) override;

    // Methods
    virtual void SAL_CALL Delete(  ) override;
    virtual void SAL_CALL SetLeftIndent( float LeftIndent, ::sal_Int32 RulerStyle ) override;
    virtual void SAL_CALL Select(  ) override;

    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& /*not processed in this base class*/ ) override;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // SwVbaRows_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAROWS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
