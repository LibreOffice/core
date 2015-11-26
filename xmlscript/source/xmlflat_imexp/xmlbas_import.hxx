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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_IMPORT_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_IMPORT_HXX

#include <com/sun/star/document/XXMLOasisBasicImporter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>

namespace xmlscript
{

    // class BasicElementBase

    class BasicImport;

    typedef ::cppu::WeakImplHelper<
        css::xml::input::XElement > BasicElementBase_BASE;

    class BasicElementBase : public BasicElementBase_BASE
    {
    protected:
        BasicImport* m_pImport;
        BasicElementBase* m_pParent;
        OUString m_aLocalName;
        css::uno::Reference< css::xml::input::XAttributes > m_xAttributes;

        static bool getBoolAttr( sal_Bool* pRet, const OUString& rAttrName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            sal_Int32 nUid );

    public:
        BasicElementBase( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport );
        virtual ~BasicElementBase();

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL getParent()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getLocalName()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getUid()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::xml::input::XAttributes > SAL_CALL getAttributes()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL characters( const OUString& rChars )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL ignorableWhitespace(
            const OUString& rWhitespaces )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL processingInstruction(
            const OUString& rTarget, const OUString& rData )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endElement()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class BasicLibrariesElement

    class BasicLibrariesElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::script::XLibraryContainer2 > m_xLibContainer;

    public:
        BasicLibrariesElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::script::XLibraryContainer2 >& rxLibContainer );

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endElement()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class BasicEmbeddedLibraryElement

    class BasicEmbeddedLibraryElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::script::XLibraryContainer2 > m_xLibContainer;
        css::uno::Reference< css::container::XNameContainer > m_xLib;
        OUString m_aLibName;
        bool m_bReadOnly;

    public:
        BasicEmbeddedLibraryElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::script::XLibraryContainer2 >& rxLibContainer,
            const OUString& rLibName, bool bReadOnly );

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endElement()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class BasicModuleElement

    class BasicModuleElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::container::XNameContainer > m_xLib;
        OUString m_aName;

    public:
        BasicModuleElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::container::XNameContainer >& rxLib,
            const OUString& rName );

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endElement()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class BasicSourceCodeElement

    class BasicSourceCodeElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::container::XNameContainer > m_xLib;
        OUString m_aName;
        OUStringBuffer m_aBuffer;

    public:
        BasicSourceCodeElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::container::XNameContainer >& rxLib,
            const OUString& rName );

        // XElement
        virtual void SAL_CALL characters( const OUString& rChars )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endElement()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class BasicImport

    typedef ::cppu::WeakImplHelper<
        css::xml::input::XRoot > BasicImport_BASE;

    class BasicImport : public BasicImport_BASE
    {
        friend class BasicElementBase;
        friend class BasicLibrariesElement;
        friend class BasicEmbeddedLibraryElement;
        friend class BasicModuleElement;

    private:
        sal_Int32 XMLNS_UID;
        sal_Int32 XMLNS_XLINK_UID;
        css::uno::Reference< css::frame::XModel > m_xModel;
        bool m_bOasis;

    public:
        BasicImport( const css::uno::Reference< css::frame::XModel >& rxModel, bool bOasis );
        virtual ~BasicImport();

        // XRoot
        virtual void SAL_CALL startDocument(
            const css::uno::Reference< css::xml::input::XNamespaceMapping >& xNamespaceMapping )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endDocument()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL processingInstruction(
            const OUString& rTarget, const OUString& rData )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDocumentLocator(
            const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startRootElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class XMLBasicImporterBase

    typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::document::XXMLOasisBasicImporter > XMLBasicImporterBase_BASE;

    class XMLBasicImporterBase : public XMLBasicImporterBase_BASE
    {
    private:
        ::osl::Mutex                                              m_aMutex;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::xml::sax::XDocumentHandler >    m_xHandler;
        css::uno::Reference< css::frame::XModel >                 m_xModel;
        bool                                                      m_bOasis;

    public:
        XMLBasicImporterBase(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext, bool bOasis );
        virtual ~XMLBasicImporterBase();

        // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (css::uno::RuntimeException, std::exception) override;

        // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& rxDoc )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endDocument()
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL startElement( const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL endElement( const OUString& aName )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL characters( const OUString& aChars )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };

    // class XMLBasicImporter

    class XMLBasicImporter : public XMLBasicImporterBase
    {
    public:
        explicit XMLBasicImporter(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~XMLBasicImporter();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (css::uno::RuntimeException, std::exception) override;
    };

    // class XMLOasisBasicImporter

    class XMLOasisBasicImporter : public XMLBasicImporterBase
    {
    public:
        explicit XMLOasisBasicImporter(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~XMLOasisBasicImporter();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (css::uno::RuntimeException, std::exception) override;
    };

}   // namespace xmlscript

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_IMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
