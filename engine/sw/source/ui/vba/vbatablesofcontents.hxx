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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATABLESOFCONTENTS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATABLESOFCONTENTS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XTablesOfContents.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <ooo/vba/word/XTableOfContents.hpp>
#include <ooo/vba/word/XRange.hpp>
#include <rtl/ref.hxx>

class SwXTextDocument;

typedef CollTestImplHelper< ooo::vba::word::XTablesOfContents > SwVbaTablesOfContents_BASE;

class SwVbaTablesOfContents : public SwVbaTablesOfContents_BASE
{
private:
    rtl::Reference< SwXTextDocument > mxTextDocument;

public:
    /// @throws css::uno::RuntimeException
    SwVbaTablesOfContents( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const rtl::Reference< SwXTextDocument >& xDoc );

    // Methods
    virtual css::uno::Reference< ::ooo::vba::word::XTableOfContents > SAL_CALL Add( const css::uno::Reference< ::ooo::vba::word::XRange >& Range, const cpo::uno::Any& UseHeadingStyles, const cpo::uno::Any& UpperHeadingLevel, const cpo::uno::Any& LowerHeadingLevel, const cpo::uno::Any& UseFields, const cpo::uno::Any& TableID, const cpo::uno::Any& RightAlignPageNumbers, const cpo::uno::Any& IncludePageNumbers, const cpo::uno::Any& AddedStyles, const cpo::uno::Any& UseHyperlinks, const cpo::uno::Any& HidePageNumbersInWeb, const cpo::uno::Any& UseOutlineLevels ) override;

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // SwVbaTablesOfContents_BASE
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBATABLESOFCONTENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
