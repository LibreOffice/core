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
#include <rtl/ref.hxx>

namespace xmlscript
{

    // class BasicElementBase

    class BasicImport;

    typedef ::cppu::WeakImplHelper<
        css::xml::input::XElement > BasicElementBase_BASE;

    class BasicElementBase : public BasicElementBase_BASE
    {
    protected:
        rtl::Reference<BasicImport> m_xImport;
    private:
        rtl::Reference<BasicElementBase> m_xParent;
        OUString const m_aLocalName;
        css::uno::Reference< css::xml::input::XAttributes > m_xAttributes;

    protected:
        static bool getBoolAttr( bool* pRet, const OUString& rAttrName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            sal_Int32 nUid );

    public:
        BasicElementBase( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport );
        virtual ~BasicElementBase() override;

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL getParent() override;
        virtual OUString SAL_CALL getLocalName() override;
        virtual sal_Int32 SAL_CALL getUid() override;
        virtual css::uno::Reference< css::xml::input::XAttributes > SAL_CALL getAttributes() override;
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes ) override;
        virtual void SAL_CALL characters( const OUString& rChars ) override;
        virtual void SAL_CALL ignorableWhitespace(
            const OUString& rWhitespaces ) override;
        virtual void SAL_CALL processingInstruction(
            const OUString& rTarget, const OUString& rData ) override;
        virtual void SAL_CALL endElement() override;
    };

    // class BasicLibrariesElement

    class BasicLibrariesElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::script::XLibraryContainer2 > m_xLibContainer;

    public:
        BasicLibrariesElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicImport* pImport,
            const css::uno::Reference< css::script::XLibraryContainer2 >& rxLibContainer );

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes ) override;
        virtual void SAL_CALL endElement() override;
    };

    // class BasicEmbeddedLibraryElement

    class BasicEmbeddedLibraryElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::script::XLibraryContainer2 > m_xLibContainer;
        css::uno::Reference< css::container::XNameContainer > m_xLib;
        OUString const m_aLibName;
        bool const m_bReadOnly;

    public:
        BasicEmbeddedLibraryElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::script::XLibraryContainer2 >& rxLibContainer,
            const OUString& rLibName, bool bReadOnly );

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes ) override;
        virtual void SAL_CALL endElement() override;
    };

    // class BasicModuleElement

    class BasicModuleElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::container::XNameContainer > m_xLib;
        OUString const m_aName;

    public:
        BasicModuleElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::container::XNameContainer >& rxLib,
            const OUString& rName );

        // XElement
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes ) override;
        virtual void SAL_CALL endElement() override;
    };

    // class BasicSourceCodeElement

    class BasicSourceCodeElement : public BasicElementBase
    {
    private:
        css::uno::Reference< css::container::XNameContainer > m_xLib;
        OUString const m_aName;
        OUStringBuffer m_aBuffer;

    public:
        BasicSourceCodeElement( const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const css::uno::Reference< css::container::XNameContainer >& rxLib,
            const OUString& rName );

        // XElement
        virtual void SAL_CALL characters( const OUString& rChars ) override;
        virtual void SAL_CALL endElement() override;
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
        bool const m_bOasis;

    public:
        BasicImport( const css::uno::Reference< css::frame::XModel >& rxModel, bool bOasis );
        virtual ~BasicImport() override;

        // XRoot
        virtual void SAL_CALL startDocument(
            const css::uno::Reference< css::xml::input::XNamespaceMapping >& xNamespaceMapping ) override;
        virtual void SAL_CALL endDocument() override;
        virtual void SAL_CALL processingInstruction(
            const OUString& rTarget, const OUString& rData ) override;
        virtual void SAL_CALL setDocumentLocator(
            const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;
        virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startRootElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const css::uno::Reference< css::xml::input::XAttributes >& xAttributes ) override;
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
        bool const                                                m_bOasis;

    public:
        XMLBasicImporterBase(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext, bool bOasis );
        virtual ~XMLBasicImporterBase() override;

        // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

        // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& rxDoc ) override;

        // XDocumentHandler
        virtual void SAL_CALL startDocument() override;
        virtual void SAL_CALL endDocument() override;
        virtual void SAL_CALL startElement( const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;
        virtual void SAL_CALL endElement( const OUString& aName ) override;
        virtual void SAL_CALL characters( const OUString& aChars ) override;
        virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
        virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;
        virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;
    };

    // class XMLBasicImporter

    class XMLBasicImporter : public XMLBasicImporterBase
    {
    public:
        explicit XMLBasicImporter(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~XMLBasicImporter() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
    };

    // class XMLOasisBasicImporter

    class XMLOasisBasicImporter : public XMLBasicImporterBase
    {
    public:
        explicit XMLOasisBasicImporter(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~XMLOasisBasicImporter() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
    };

}   // namespace xmlscript

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_IMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
