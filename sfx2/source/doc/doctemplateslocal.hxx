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


class DocTemplLocaleHelper : public cppu::WeakImplHelper < com::sun::star::xml::sax::XDocumentHandler >
{
    // Relations info related strings
    OUString m_aGroupListElement;
    OUString m_aGroupElement;
    OUString m_aNameAttr;
    OUString m_aUINameAttr;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > m_aResultSeq;
    ::com::sun::star::uno::Sequence< OUString > m_aElementsSeq; // stack of elements being parsed

    DocTemplLocaleHelper();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > GetParsingResult();

    static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > SAL_CALL ReadLocalizationSequence_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream, const OUString& aStringID, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext )
    throw( ::com::sun::star::uno::Exception );

public:
    virtual ~DocTemplLocaleHelper();

    // returns sequence of pairs ( GroupName, GroupUIName )
    static
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >
    ReadGroupLocalizationSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext )
            throw( ::com::sun::star::uno::Exception );

    // writes sequence of elements ( GroupName, GroupUIName )
    static
    void SAL_CALL WriteGroupLocalizationSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aSequence,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext )
            throw( ::com::sun::star::uno::Exception );

    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startElement( const OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement( const OUString& aName ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
