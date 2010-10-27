/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef XMLSCRIPT_XMLBAS_IMPORT_HXX
#define XMLSCRIPT_XMLBAS_IMPORT_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
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

    typedef ::cppu::WeakImplHelper3<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::document::XImporter,
        ::com::sun::star::xml::sax::XDocumentHandler > XMLBasicImporterBase_BASE;

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
