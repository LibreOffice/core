/*************************************************************************
 *
 *  $RCSfile: xmlbas_import.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:36:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef XMLSCRIPT_XMLBAS_IMPORT_HXX
#define XMLSCRIPT_XMLBAS_IMPORT_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_INPUT_XROOT_HPP_
#include <com/sun/star/xml/input/XRoot.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif


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
