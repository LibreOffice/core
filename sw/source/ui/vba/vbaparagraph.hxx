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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPH_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPH_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XParagraphs.hpp>
#include <ooo/vba/word/XParagraph.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XParagraph > SwVbaParagraph_BASE;

class SwVbaParagraph : public SwVbaParagraph_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextRange > mxTextRange;

public:
    SwVbaParagraph( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& xDocument, const css::uno::Reference< css::text::XTextRange >& xTextRange ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaParagraph();

    // XParagraph
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getRange() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getStyle() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setStyle( const css::uno::Any& style ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

typedef CollTestImplHelper< ooo::vba::word::XParagraphs > SwVbaParagraphs_BASE;

class SwVbaParagraphs : public SwVbaParagraphs_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
public:
    SwVbaParagraphs( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextDocument >& xDocument ) throw (css::uno::RuntimeException);
    virtual ~SwVbaParagraphs() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaParagraphs_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAPARAGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
