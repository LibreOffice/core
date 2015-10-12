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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBATABLEOFCONTENTS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBATABLEOFCONTENTS_HXX

#include <ooo/vba/word/XTableOfContents.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XTableOfContents > SwVbaTableOfContents_BASE;

class SwVbaTableOfContents : public SwVbaTableOfContents_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XDocumentIndex > mxDocumentIndex;
    css::uno::Reference< css::beans::XPropertySet > mxTocProps;

public:
    SwVbaTableOfContents( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& xDoc, const css::uno::Reference< css::text::XDocumentIndex >& xDocumentIndex ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaTableOfContents();

    // Attributes
    virtual ::sal_Int32 SAL_CALL getLowerHeadingLevel() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLowerHeadingLevel( ::sal_Int32 _lowerheadinglevel ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getTabLeader() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTabLeader( ::sal_Int32 _tableader ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getUseFields() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setUseFields( sal_Bool _useFields ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getUseOutlineLevels() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setUseOutlineLevels( sal_Bool _useOutlineLevels ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Update(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBATABLEOFCONTENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
