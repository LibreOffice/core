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

#ifndef XMLSCRIPT_XMLBAS_IMPORT_HXX
#define XMLSCRIPT_XMLBAS_IMPORT_HXX

#include <com/sun/star/document/XXMLOasisBasicImporter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>


//.........................................................................
namespace xmlscript
{
//.........................................................................

    // =============================================================================
    // class BasicElementBase
    // =============================================================================

    class BasicImport;

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::xml::input::XElement > BasicElementBase_BASE;

    class BasicElementBase : public BasicElementBase_BASE
    {
    protected:
        BasicImport* m_pImport;
        BasicElementBase* m_pParent;
        ::rtl::OUString m_aLocalName;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes > m_xAttributes;

        bool getBoolAttr( sal_Bool* pRet, const ::rtl::OUString& rAttrName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
            sal_Int32 nUid );

    public:
        BasicElementBase( const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport );
        virtual ~BasicElementBase();

        // XElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XElement > SAL_CALL getParent()
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getLocalName()
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getUid()
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes > SAL_CALL getAttributes()
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL characters( const ::rtl::OUString& rChars )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL ignorableWhitespace(
            const ::rtl::OUString& rWhitespaces )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL processingInstruction(
            const ::rtl::OUString& rTarget, const ::rtl::OUString& rData )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class BasicLibrariesElement
    // =============================================================================

    class BasicLibrariesElement : public BasicElementBase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer2 > m_xLibContainer;

    public:
        BasicLibrariesElement( const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer2 >& rxLibContainer );

        // XElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class BasicEmbeddedLibraryElement
    // =============================================================================

    class BasicEmbeddedLibraryElement : public BasicElementBase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer2 > m_xLibContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xLib;
        ::rtl::OUString m_aLibName;
        bool m_bReadOnly;

    public:
        BasicEmbeddedLibraryElement( const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer2 >& rxLibContainer,
            const ::rtl::OUString& rLibName, bool bReadOnly );

        // XElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class BasicModuleElement
    // =============================================================================

    class BasicModuleElement : public BasicElementBase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xLib;
        ::rtl::OUString m_aName;

    public:
        BasicModuleElement( const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxLib,
            const ::rtl::OUString& rName );

        // XElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XElement > SAL_CALL startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class BasicSourceCodeElement
    // =============================================================================

    class BasicSourceCodeElement : public BasicElementBase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xLib;
        ::rtl::OUString m_aName;
        ::rtl::OUStringBuffer m_aBuffer;

    public:
        BasicSourceCodeElement( const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxLib,
            const ::rtl::OUString& rName );

        // XElement
        virtual void SAL_CALL characters( const ::rtl::OUString& rChars )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class BasicImport
    // =============================================================================

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::xml::input::XRoot > BasicImport_BASE;

    class BasicImport : public BasicImport_BASE
    {
        friend class BasicElementBase;
        friend class BasicLibrariesElement;
        friend class BasicEmbeddedLibraryElement;
        friend class BasicModuleElement;

    private:
        sal_Int32 XMLNS_UID;
        sal_Int32 XMLNS_XLINK_UID;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;
        sal_Bool m_bOasis;

    public:
        BasicImport( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel, sal_Bool bOasis );
        virtual ~BasicImport();

        // XRoot
        virtual void SAL_CALL startDocument(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XNamespaceMapping >& xNamespaceMapping )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endDocument()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL processingInstruction(
            const ::rtl::OUString& rTarget, const ::rtl::OUString& rData )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDocumentLocator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XElement > SAL_CALL startRootElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLBasicImporterBase
    // =============================================================================

    typedef ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::document::XXMLOasisBasicImporter > XMLBasicImporterBase_BASE;

    class XMLBasicImporterBase : public XMLBasicImporterBase_BASE
    {
    private:
        ::osl::Mutex                                                                        m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
        sal_Bool                                                                            m_bOasis;

    public:
        XMLBasicImporterBase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, sal_Bool bOasis );
        virtual ~XMLBasicImporterBase();

        // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException);

        // XImporter
        virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxDoc )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endDocument()
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL startElement( const ::rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endElement( const ::rtl::OUString& aName )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLBasicImporter
    // =============================================================================

    class XMLBasicImporter : public XMLBasicImporterBase
    {
    public:
        XMLBasicImporter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~XMLBasicImporter();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLOasisBasicImporter
    // =============================================================================

    class XMLOasisBasicImporter : public XMLBasicImporterBase
    {
    public:
        XMLOasisBasicImporter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~XMLOasisBasicImporter();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace xmlscript
//.........................................................................

#endif // XMLSCRIPT_XMLBAS_IMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
