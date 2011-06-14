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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"
#include "xmlbas_import.hxx"
#include "xmlscript/xmlns.h"
#include "xmlscript/xml_helper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <cppuhelper/implementationentry.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;


//.........................................................................
namespace xmlscript
{
//.........................................................................

    // =============================================================================
    // BasicElementBase
    // =============================================================================

    BasicElementBase::BasicElementBase( const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport )
        :m_pImport( pImport )
        ,m_pParent( pParent )
        ,m_aLocalName( rLocalName )
        ,m_xAttributes( xAttributes )
    {
        if ( m_pImport )
            m_pImport->acquire();
        if ( m_pParent )
            m_pParent->acquire();
    }

    // -----------------------------------------------------------------------------

    BasicElementBase::~BasicElementBase()
    {
        if ( m_pImport )
            m_pImport->release();
        if ( m_pParent )
            m_pParent->release();
    }

    // -----------------------------------------------------------------------------

    bool BasicElementBase::getBoolAttr( sal_Bool* pRet, const ::rtl::OUString& rAttrName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::input::XAttributes >& xAttributes,
        sal_Int32 nUid )
    {
        if ( xAttributes.is() )
        {
            ::rtl::OUString aValue( xAttributes->getValueByUidName( nUid, rAttrName ) );
            if ( aValue.getLength() )
            {
                if ( aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "true" ) ) )
                {
                    *pRet = sal_True;
                    return true;
                }
                else if ( aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "false" ) ) )
                {
                    *pRet = sal_False;
                    return true;
                }
                else
                {
                    throw xml::sax::SAXException(
                        rAttrName + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": no boolean value (true|false)!" ) ),
                        Reference< XInterface >(), Any() );
                }
            }
        }
        return false;
    }

    // -----------------------------------------------------------------------------
    // XElement
    // -----------------------------------------------------------------------------

    Reference< xml::input::XElement > BasicElementBase::getParent()
        throw (RuntimeException)
    {
        return static_cast< xml::input::XElement* >( m_pParent );
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicElementBase::getLocalName()
        throw (RuntimeException)
    {
        return m_aLocalName;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 BasicElementBase::getUid()
        throw (RuntimeException)
    {
        sal_Int32 nId = -1;
        if ( m_pImport )
            nId = m_pImport->XMLNS_UID;
        return nId;
    }

    // -----------------------------------------------------------------------------

    Reference< xml::input::XAttributes > BasicElementBase::getAttributes()
        throw (RuntimeException)
    {
        return m_xAttributes;
    }

    // -----------------------------------------------------------------------------

    Reference< xml::input::XElement > BasicElementBase::startChildElement(
        sal_Int32 /*nUid*/, const ::rtl::OUString& /*rLocalName*/,
        const Reference< xml::input::XAttributes >& /*xAttributes*/ )
        throw (xml::sax::SAXException, RuntimeException)
    {
        throw xml::sax::SAXException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "unexpected element!" ) ),
            Reference< XInterface >(), Any() );
    }

    // -----------------------------------------------------------------------------

void BasicElementBase::characters( const ::rtl::OUString& /*rChars*/ )
        throw (xml::sax::SAXException, RuntimeException)
    {
        // not used, all characters ignored
    }

    // -----------------------------------------------------------------------------

void BasicElementBase::ignorableWhitespace( const ::rtl::OUString& /*rWhitespaces*/ )
        throw (xml::sax::SAXException, RuntimeException)
    {
    }

    // -----------------------------------------------------------------------------

void BasicElementBase::processingInstruction( const ::rtl::OUString& /*rTarget*/, const ::rtl::OUString& /*rData*/ )
        throw (xml::sax::SAXException, RuntimeException)
    {
    }

    // -----------------------------------------------------------------------------

    void BasicElementBase::endElement()
        throw (xml::sax::SAXException, RuntimeException)
    {
    }


    // =============================================================================
    // BasicLibrariesElement
    // =============================================================================

    BasicLibrariesElement::BasicLibrariesElement( const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< script::XLibraryContainer2 >& rxLibContainer )
        :BasicElementBase( rLocalName, xAttributes, pParent, pImport )
        ,m_xLibContainer( rxLibContainer )
    {
    }

    // -----------------------------------------------------------------------------
    // XElement
    // -----------------------------------------------------------------------------

    Reference< xml::input::XElement > BasicLibrariesElement::startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes )
        throw (xml::sax::SAXException, RuntimeException)
    {
        Reference< xml::input::XElement > xElement;

        if ( nUid != m_pImport->XMLNS_UID )
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal namespace!" ) ),
                Reference< XInterface >(), Any() );
        }
        else if ( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "library-linked" ) ) )
        {
            if ( xAttributes.is() )
            {
                ::rtl::OUString aName = xAttributes->getValueByUidName(
                    m_pImport->XMLNS_UID,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "name" ) ) );

                ::rtl::OUString aStorageURL = xAttributes->getValueByUidName(
                    m_pImport->XMLNS_XLINK_UID,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "href" ) ) );

                sal_Bool bReadOnly = sal_False;
                getBoolAttr( &bReadOnly,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "readonly" ) ),
                    xAttributes, m_pImport->XMLNS_UID );

                if ( m_xLibContainer.is() )
                {
                    try
                    {
                        Reference< container::XNameAccess > xLib(
                            m_xLibContainer->createLibraryLink( aName, aStorageURL, bReadOnly ) );
                        if ( xLib.is() )
                            xElement.set( new BasicElementBase( rLocalName, xAttributes, this, m_pImport ) );
                    }
                    catch ( container::ElementExistException& e )
                    {
                        OSL_TRACE( "BasicLibrariesElement::startChildElement: caught ElementExceptionExist reason %s",
                            ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    }
                    catch ( lang::IllegalArgumentException& e )
                    {
                        OSL_TRACE( "BasicLibrariesElement::startChildElement: caught IllegalArgumentException reason %s",
                            ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    }
                }
            }
        }
        else if ( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "library-embedded" ) ) )
        {
            // TODO: create password protected libraries

            if ( xAttributes.is() )
            {
                ::rtl::OUString aName = xAttributes->getValueByUidName(
                    m_pImport->XMLNS_UID,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "name" ) ) );

                sal_Bool bReadOnly = sal_False;
                getBoolAttr( &bReadOnly,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "readonly" ) ),
                    xAttributes, m_pImport->XMLNS_UID );

                if ( m_xLibContainer.is() )
                {
                    try
                    {
                        Reference< container::XNameContainer > xLib;
                        if ( m_xLibContainer->hasByName( aName ) )
                        {
                            // Standard library
                            m_xLibContainer->getByName( aName ) >>= xLib;
                        }
                        else
                        {
                            xLib.set( m_xLibContainer->createLibrary( aName ) );
                        }

                        if ( xLib.is() )
                            xElement.set( new BasicEmbeddedLibraryElement( rLocalName, xAttributes, this, m_pImport, m_xLibContainer, aName, bReadOnly ) );
                    }
                    catch ( lang::IllegalArgumentException& e )
                    {
                        OSL_TRACE( "BasicLibrariesElement::startChildElement: caught IllegalArgumentException reason %s",
                            ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                    }
                }
            }
        }
        else
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "expected library-linked or library-embedded element!" ) ),
                Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    // -----------------------------------------------------------------------------

    void BasicLibrariesElement::endElement()
        throw (xml::sax::SAXException, RuntimeException)
    {
    }


    // =============================================================================
    // BasicEmbeddedLibraryElement
    // =============================================================================

    BasicEmbeddedLibraryElement::BasicEmbeddedLibraryElement( const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< script::XLibraryContainer2 >& rxLibContainer,
            const ::rtl::OUString& rLibName, bool bReadOnly )
        :BasicElementBase( rLocalName, xAttributes, pParent, pImport )
        ,m_xLibContainer( rxLibContainer )
        ,m_aLibName( rLibName )
        ,m_bReadOnly( bReadOnly )
    {
        try
        {
            if ( m_xLibContainer.is() && m_xLibContainer->hasByName( m_aLibName ) )
                m_xLibContainer->getByName( m_aLibName ) >>= m_xLib;
        }
        catch ( lang::WrappedTargetException& e )
        {
            OSL_TRACE( "BasicEmbeddedLibraryElement CTOR: caught WrappedTargetException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }

    // -----------------------------------------------------------------------------
    // XElement
    // -----------------------------------------------------------------------------

    Reference< xml::input::XElement > BasicEmbeddedLibraryElement::startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes )
        throw (xml::sax::SAXException, RuntimeException)
    {
        Reference< xml::input::XElement > xElement;

        if ( nUid != m_pImport->XMLNS_UID )
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal namespace!" ) ),
                Reference< XInterface >(), Any() );
        }
        else if ( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "module" ) ) )
        {
            if ( xAttributes.is() )
            {
                ::rtl::OUString aName = xAttributes->getValueByUidName(
                    m_pImport->XMLNS_UID,
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "name" ) ) );

                if ( m_xLib.is() && aName.getLength() )
                    xElement.set( new BasicModuleElement( rLocalName, xAttributes, this, m_pImport, m_xLib, aName ) );
            }
        }
        else
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "expected module element!" ) ),
                Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    // -----------------------------------------------------------------------------

    void BasicEmbeddedLibraryElement::endElement()
        throw (xml::sax::SAXException, RuntimeException)
    {
        if ( m_xLibContainer.is() && m_xLibContainer->hasByName( m_aLibName ) && m_bReadOnly )
            m_xLibContainer->setLibraryReadOnly( m_aLibName, m_bReadOnly );
    }


    // =============================================================================
    // BasicModuleElement
    // =============================================================================

    BasicModuleElement::BasicModuleElement( const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< container::XNameContainer >& rxLib, const ::rtl::OUString& rName )
        :BasicElementBase( rLocalName, xAttributes, pParent, pImport )
        ,m_xLib( rxLib )
        ,m_aName( rName )
    {
    }

    // -----------------------------------------------------------------------------
    // XElement
    // -----------------------------------------------------------------------------

    Reference< xml::input::XElement > BasicModuleElement::startChildElement(
            sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes )
        throw (xml::sax::SAXException, RuntimeException)
    {
        // TODO: <byte-code>

        Reference< xml::input::XElement > xElement;

        if ( nUid != m_pImport->XMLNS_UID )
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal namespace!" ) ),
                Reference< XInterface >(), Any() );
        }
        else if ( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "source-code" ) ) )
        {
            // TODO: password protected libraries

            if ( xAttributes.is() )
            {
                if ( m_xLib.is() && m_aName.getLength() )
                    xElement.set( new BasicSourceCodeElement( rLocalName, xAttributes, this, m_pImport, m_xLib, m_aName ) );
            }
        }
        else
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "expected source-code element!" ) ),
                Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    // -----------------------------------------------------------------------------

    void BasicModuleElement::endElement()
        throw (xml::sax::SAXException, RuntimeException)
    {
    }


    // =============================================================================
    // BasicSourceCodeElement
    // =============================================================================

    BasicSourceCodeElement::BasicSourceCodeElement( const ::rtl::OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< container::XNameContainer >& rxLib, const ::rtl::OUString& rName )
        :BasicElementBase( rLocalName, xAttributes, pParent, pImport )
        ,m_xLib( rxLib )
        ,m_aName( rName )
    {
    }

    // -----------------------------------------------------------------------------
    // XElement
    // -----------------------------------------------------------------------------

    void BasicSourceCodeElement::characters( const ::rtl::OUString& rChars )
        throw (xml::sax::SAXException, RuntimeException)
    {
        m_aBuffer.append( rChars );
    }

    // -----------------------------------------------------------------------------

    void BasicSourceCodeElement::endElement()
        throw (xml::sax::SAXException, RuntimeException)
    {
        try
        {
            if ( m_xLib.is() && m_aName.getLength() )
            {
                Any aElement;
                aElement <<= m_aBuffer.makeStringAndClear();
                m_xLib->insertByName( m_aName, aElement );
            }
        }
        catch ( container::ElementExistException& e )
        {
            OSL_TRACE( "BasicSourceCodeElement::endElement: caught ElementExceptionExist reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        catch ( lang::IllegalArgumentException& e )
        {
            OSL_TRACE( "BasicSourceCodeElement::endElement: caught IllegalArgumentException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        catch ( lang::WrappedTargetException& e )
        {
            OSL_TRACE( "BasicSourceCodeElement::endElement: caught WrappedTargetException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }


    // =============================================================================
    // BasicImport
    // =============================================================================

    BasicImport::BasicImport( const Reference< frame::XModel >& rxModel, sal_Bool bOasis )
        :m_xModel( rxModel )
        ,m_bOasis( bOasis )
    {
    }

    // -----------------------------------------------------------------------------

    BasicImport::~BasicImport()
    {
    }

    // -----------------------------------------------------------------------------
    // XRoot
    // -----------------------------------------------------------------------------

    void BasicImport::startDocument( const Reference< xml::input::XNamespaceMapping >& xNamespaceMapping )
        throw (xml::sax::SAXException, RuntimeException)
    {
        if ( xNamespaceMapping.is() )
        {
            ::rtl::OUString aURI;
            if ( m_bOasis )
                aURI = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_OOO_URI ) );
            else
                aURI = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_SCRIPT_URI ) );
            XMLNS_UID = xNamespaceMapping->getUidByUri( aURI );
            XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_URI ) ) );
        }
    }

    // -----------------------------------------------------------------------------

    void BasicImport::endDocument()
        throw (xml::sax::SAXException, RuntimeException)
    {
    }

    // -----------------------------------------------------------------------------

void BasicImport::processingInstruction( const ::rtl::OUString& /*rTarget*/, const ::rtl::OUString& /*rData*/ )
        throw (xml::sax::SAXException, RuntimeException)
    {
    }

    // -----------------------------------------------------------------------------

void BasicImport::setDocumentLocator( const Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw (xml::sax::SAXException, RuntimeException)
    {
    }

    // -----------------------------------------------------------------------------

    Reference< xml::input::XElement > BasicImport::startRootElement( sal_Int32 nUid, const ::rtl::OUString& rLocalName,
            Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException)
    {
        Reference< xml::input::XElement > xElement;

        if ( nUid != XMLNS_UID )
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal namespace!" ) ),
                Reference< XInterface >(), Any() );
        }
        else if ( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "libraries" ) ) )
        {
            Reference< script::XLibraryContainer2 > xLibContainer;

            // try the XEmbeddedScripts interface
            Reference< document::XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
            if ( xDocumentScripts.is() )
                xLibContainer.set( xDocumentScripts->getBasicLibraries().get() );

            if ( !xLibContainer.is() )
            {
                // try the "BasicLibraries" property (old-style, for compatibility)
                Reference< beans::XPropertySet > xPSet( m_xModel, UNO_QUERY );
                if ( xPSet.is() )
                    xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicLibraries" ) ) ) >>= xLibContainer;
            }

            OSL_ENSURE( xLibContainer.is(), "BasicImport::startRootElement: nowhere to import to!" );

            if ( xLibContainer.is() )
            {
                xElement.set( new BasicLibrariesElement( rLocalName, xAttributes, 0, this, xLibContainer ) );
            }
        }
        else
        {
            throw xml::sax::SAXException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal root element (expected libraries) given: " ) ) +
                rLocalName, Reference< XInterface >(), Any() );
        }

        return xElement;
    }


    // =============================================================================
    // component operations
    // =============================================================================

    ::rtl::OUString getImplementationName_XMLBasicImporter()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.xmlscript.XMLBasicImporter" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > getSupportedServiceNames_XMLBasicImporter()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLBasicImporter" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString getImplementationName_XMLOasisBasicImporter()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.xmlscript.XMLOasisBasicImporter" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > getSupportedServiceNames_XMLOasisBasicImporter()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLOasisBasicImporter" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }


    // =============================================================================
    // XMLBasicImporterBase
    // =============================================================================

    XMLBasicImporterBase::XMLBasicImporterBase( const Reference< XComponentContext >& rxContext, sal_Bool bOasis )
        :m_xContext( rxContext )
        ,m_bOasis( bOasis )
    {
    }

    // -----------------------------------------------------------------------------

    XMLBasicImporterBase::~XMLBasicImporterBase()
    {
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    sal_Bool XMLBasicImporterBase::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------
    // XImporter
    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::setTargetDocument( const Reference< XComponent >& rxDoc )
        throw (IllegalArgumentException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_xModel.set( rxDoc, UNO_QUERY );

        if ( !m_xModel.is() )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLBasicExporter::setTargetDocument: no document model!" ) ),
                Reference< XInterface >(), 1 );
        }

        if ( m_xContext.is() )
        {
            Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
            if ( xSMgr.is() )
            {
                Reference < xml::input::XRoot > xRoot( new BasicImport( m_xModel, m_bOasis ) );
                Sequence < Any > aArgs( 1 );
                aArgs[0] <<= xRoot;
                m_xHandler.set( xSMgr->createInstanceWithArgumentsAndContext(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.input.SaxDocumentHandler" ) ),
                    aArgs, m_xContext ), UNO_QUERY );
            }
        }
    }

    // -----------------------------------------------------------------------------
    // XDocumentHandler
    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::startDocument()
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->startDocument();
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::endDocument()
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->endDocument();
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::startElement( const ::rtl::OUString& aName,
            const Reference< xml::sax::XAttributeList >& xAttribs )
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->startElement( aName, xAttribs );
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::endElement( const ::rtl::OUString& aName )
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->endElement( aName );
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::characters( const ::rtl::OUString& aChars )
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->characters( aChars );
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->ignorableWhitespace( aWhitespaces );
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::processingInstruction( const ::rtl::OUString& aTarget,
            const ::rtl::OUString& aData )
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->processingInstruction( aTarget, aData );
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImporterBase::setDocumentLocator( const Reference< xml::sax::XLocator >& xLocator )
        throw (xml::sax::SAXException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->setDocumentLocator( xLocator );
    }


    // =============================================================================
    // XMLBasicImporter
    // =============================================================================

    XMLBasicImporter::XMLBasicImporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicImporterBase( rxContext, sal_False )
    {
    }

    // -----------------------------------------------------------------------------

    XMLBasicImporter::~XMLBasicImporter()
    {
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString XMLBasicImporter::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_XMLBasicImporter();
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > XMLBasicImporter::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_XMLBasicImporter();
    }


    // =============================================================================
    // XMLOasisBasicImporter
    // =============================================================================

    XMLOasisBasicImporter::XMLOasisBasicImporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicImporterBase( rxContext, sal_True )
    {
    }

    // -----------------------------------------------------------------------------

    XMLOasisBasicImporter::~XMLOasisBasicImporter()
    {
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString XMLOasisBasicImporter::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_XMLOasisBasicImporter();
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > XMLOasisBasicImporter::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_XMLOasisBasicImporter();
    }


    // =============================================================================
    // component operations
    // =============================================================================

    Reference< XInterface > SAL_CALL create_XMLBasicImporter(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new XMLBasicImporter( xContext ) );
    }

    // -----------------------------------------------------------------------------

    Reference< XInterface > SAL_CALL create_XMLOasisBasicImporter(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new XMLOasisBasicImporter( xContext ) );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace xmlscript
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
