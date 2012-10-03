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

#ifndef _COMPHELPER_OFOPXMLHELPER_HXX
#define _COMPHELPER_OFOPXMLHELPER_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{

// this helper class is designed to allow to parse ContentType- and Relationship-related information from OfficeOpenXML format
class COMPHELPER_DLLPUBLIC OFOPXMLHelper : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
    sal_uInt16 m_nFormat; // which format to parse

    // Relations info related strings
    ::rtl::OUString m_aRelListElement;
    ::rtl::OUString m_aRelElement;
    ::rtl::OUString m_aIDAttr;
    ::rtl::OUString m_aTypeAttr;
    ::rtl::OUString m_aTargetModeAttr;
    ::rtl::OUString m_aTargetAttr;

    // ContentType related strings
    ::rtl::OUString m_aTypesElement;
    ::rtl::OUString m_aDefaultElement;
    ::rtl::OUString m_aOverrideElement;
    ::rtl::OUString m_aExtensionAttr;
    ::rtl::OUString m_aPartNameAttr;
    ::rtl::OUString m_aContentTypeAttr;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > m_aResultSeq;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aElementsSeq; // stack of elements being parsed

    OFOPXMLHelper( sal_uInt16 nFormat ); // must not be created directly
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > GetParsingResult();

    static COMPHELPER_DLLPRIVATE ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL ReadSequence_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream, const ::rtl::OUString& aStringID, sal_uInt16 nFormat, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext )
    throw( ::com::sun::star::uno::Exception );

public:
    ~OFOPXMLHelper();

    // returns sequence of elements, where each element is described by sequence of tags,
    // where each tag is described by StringPair ( First - name, Second - value )
    // the first tag of each element sequence must be "Id"
    static
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >
    SAL_CALL
    ReadRelationsInfoSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
        const ::rtl::OUString aStreamName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext )
            throw( ::com::sun::star::uno::Exception );

    // returns sequence containing two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    static
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >
    SAL_CALL
    ReadContentTypeSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext )
            throw( ::com::sun::star::uno::Exception );

    // writes sequence of elements, where each element is described by sequence of tags,
    // where each tag is described by StringPair ( First - name, Second - value )
    // the first tag of each element sequence must be "Id"
    static
    void SAL_CALL WriteRelationsInfoSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >& aSequence,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // writes two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    static
    void SAL_CALL WriteContentSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutStream,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aDefaultsSequence,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aOverridesSequence,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory )
            throw( ::com::sun::star::uno::Exception );

    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
