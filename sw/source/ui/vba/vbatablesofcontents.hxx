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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATABLESOFCONTENTS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATABLESOFCONTENTS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XTablesOfContents.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <ooo/vba/word/XTableOfContents.hpp>
#include <ooo/vba/word/XRange.hpp>

typedef CollTestImplHelper< ooo::vba::word::XTablesOfContents > SwVbaTablesOfContents_BASE;

class SwVbaTablesOfContents : public SwVbaTablesOfContents_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;

public:
    SwVbaTablesOfContents( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextDocument >& xDoc ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaTablesOfContents() {}

    // Methods
    virtual css::uno::Reference< ::ooo::vba::word::XTableOfContents > SAL_CALL Add( const css::uno::Reference< ::ooo::vba::word::XRange >& Range, const css::uno::Any& UseHeadingStyles, const css::uno::Any& UpperHeadingLevel, const css::uno::Any& LowerHeadingLevel, const css::uno::Any& UseFields, const css::uno::Any& TableID, const css::uno::Any& RightAlignPageNumbers, const css::uno::Any& IncludePageNumbers, const css::uno::Any& AddedStyles, const css::uno::Any& UseHyperlinks, const css::uno::Any& HidePageNumbersInWeb, const css::uno::Any& UseOutlineLevels ) throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaTablesOfContents_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBATABLESOFCONTENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
