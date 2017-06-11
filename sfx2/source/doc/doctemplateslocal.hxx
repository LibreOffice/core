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

#ifndef INCLUDED_SFX2_SOURCE_DOC_DOCTEMPLATESLOCAL_HXX
#define INCLUDED_SFX2_SOURCE_DOC_DOCTEMPLATESLOCAL_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <vector>


class DocTemplLocaleHelper : public cppu::WeakImplHelper < css::xml::sax::XDocumentHandler >
{
    std::vector< css::beans::StringPair > m_aResultSeq;
    std::vector< OUString > m_aElementsSeq; // stack of elements being parsed

    DocTemplLocaleHelper();
    std::vector< css::beans::StringPair > const & GetParsingResult();

    /// @throws css::uno::Exception
    static std::vector< css::beans::StringPair > SAL_CALL ReadLocalizationSequence_Impl( const css::uno::Reference< css::io::XInputStream >& xInStream, const OUString& aStringID, const css::uno::Reference< css::uno::XComponentContext >& xContext );

public:
    virtual ~DocTemplLocaleHelper() override;

    // returns sequence of pairs ( GroupName, GroupUIName )
    /// @throws css::uno::Exception
    static
    std::vector< css::beans::StringPair >
    ReadGroupLocalizationSequence(
        const css::uno::Reference< css::io::XInputStream >& xInStream,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // writes sequence of elements ( GroupName, GroupUIName )
    /// @throws css::uno::Exception
    static
    void SAL_CALL WriteGroupLocalizationSequence(
        const css::uno::Reference< css::io::XOutputStream >& xOutStream,
        const std::vector< css::beans::StringPair >& aSequence,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );

    static OUString GetStandardGroupString();

    // XDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;
    virtual void SAL_CALL endElement( const OUString& aName ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
