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

#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATABLE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATABLE_HXX
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <ooo/vba/word/XRange.hpp>
#include <ooo/vba/word/XTable.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XTable > SwVbaTable_BASE;

class SwVbaTable : public SwVbaTable_BASE
{
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextTable > mxTextTable;
public:
    /// @throws css::uno::RuntimeException
    SwVbaTable( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, css::uno::Reference< css::text::XTextDocument > xDocument, const css::uno::Reference< css::text::XTextTable >& xTextTable);
    virtual css::uno::Reference< ::ooo::vba::word::XRange > SAL_CALL Range(  ) override;
    virtual void SAL_CALL Select(  ) override;
    virtual void SAL_CALL Delete(  ) override;
    virtual OUString SAL_CALL getName(  ) override;
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& aIndex ) override;
    virtual float SAL_CALL getBottomPadding(  ) override;
    virtual void SAL_CALL setBottomPadding( float fValue ) override;
    virtual float SAL_CALL getLeftPadding(  ) override;
    virtual void SAL_CALL setLeftPadding( float fValue ) override;
    virtual float SAL_CALL getRightPadding(  ) override;
    virtual void SAL_CALL setRightPadding( float fValue ) override;
    virtual float SAL_CALL getTopPadding(  ) override;
    virtual void SAL_CALL setTopPadding( float fValue ) override;
    virtual css::uno::Any SAL_CALL Rows( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Columns( const css::uno::Any& aIndex ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
