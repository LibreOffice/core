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

#include <sal/config.h>

#include "unoservices.hxx"
#include "xmlbas_import.hxx"
#include <sal/log.hxx>
#include <xmlscript/xmlns.h>
#include <xmlscript/xml_helper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

    // BasicElementBase

    BasicElementBase::BasicElementBase( const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport )
        :m_xImport( pImport )
        ,m_xParent( pParent )
        ,m_aLocalName( rLocalName )
        ,m_xAttributes( xAttributes )
    {
    }

    BasicElementBase::~BasicElementBase()
    {
    }

    bool BasicElementBase::getBoolAttr( bool* pRet, const OUString& rAttrName,
        const css::uno::Reference< css::xml::input::XAttributes >& xAttributes,
        sal_Int32 nUid )
    {
        if ( xAttributes.is() )
        {
            OUString aValue( xAttributes->getValueByUidName( nUid, rAttrName ) );
            if ( !aValue.isEmpty() )
            {
                if ( aValue == "true" )
                {
                    *pRet = true;
                    return true;
                }
                else if ( aValue == "false" )
                {
                    *pRet = false;
                    return true;
                }
                else
                {
                    throw xml::sax::SAXException(rAttrName + ": no boolean value (true|false)!", Reference< XInterface >(), Any() );
                }
            }
        }
        return false;
    }

    // XElement

    Reference< xml::input::XElement > BasicElementBase::getParent()
    {
        return m_xParent.get();
    }

    OUString BasicElementBase::getLocalName()
    {
        return m_aLocalName;
    }

    sal_Int32 BasicElementBase::getUid()
    {
        sal_Int32 nId = -1;
        if ( m_xImport.is() )
            nId = m_xImport->XMLNS_UID;
        return nId;
    }

    Reference< xml::input::XAttributes > BasicElementBase::getAttributes()
    {
        return m_xAttributes;
    }

    Reference< xml::input::XElement > BasicElementBase::startChildElement(
        sal_Int32 /*nUid*/, const OUString& /*rLocalName*/,
        const Reference< xml::input::XAttributes >& /*xAttributes*/ )
    {
        throw xml::sax::SAXException("unexpected element!", Reference< XInterface >(), Any() );
    }

void BasicElementBase::characters( const OUString& /*rChars*/ )
    {
        // not used, all characters ignored
    }

void BasicElementBase::ignorableWhitespace( const OUString& /*rWhitespaces*/ )
    {
    }

void BasicElementBase::processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ )
    {
    }

    void BasicElementBase::endElement()
    {
    }

    // BasicLibrariesElement

    BasicLibrariesElement::BasicLibrariesElement( const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicImport* pImport,
            const Reference< script::XLibraryContainer2 >& rxLibContainer )
        :BasicElementBase( rLocalName, xAttributes, nullptr, pImport )
        ,m_xLibContainer( rxLibContainer )
    {
    }

    // XElement

    Reference< xml::input::XElement > BasicLibrariesElement::startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes )
    {
        Reference< xml::input::XElement > xElement;

        if ( nUid != m_xImport->XMLNS_UID )
        {
            throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
        }
        else if ( rLocalName == "library-linked" )
        {
            if ( xAttributes.is() )
            {
                OUString aName = xAttributes->getValueByUidName( m_xImport->XMLNS_UID, "name" );

                OUString aStorageURL = xAttributes->getValueByUidName(m_xImport->XMLNS_XLINK_UID, "href" );

                bool bReadOnly = false;
                getBoolAttr( &bReadOnly,"readonly", xAttributes, m_xImport->XMLNS_UID );

                if ( m_xLibContainer.is() )
                {
                    try
                    {
                        Reference< container::XNameAccess > xLib(
                            m_xLibContainer->createLibraryLink( aName, aStorageURL, bReadOnly ) );
                        if ( xLib.is() )
                            xElement.set( new BasicElementBase( rLocalName, xAttributes, this, m_xImport.get() ) );
                    }
                    catch ( const container::ElementExistException& e )
                    {
                        SAL_INFO("xmlscript.xmlflat", "BasicLibrariesElement::startChildElement: caught ElementExceptionExist reason " << e.Message );
                    }
                    catch ( const lang::IllegalArgumentException& e )
                    {
                        SAL_INFO("xmlscript.xmlflat", "BasicLibrariesElement::startChildElement: caught IllegalArgumentException reason " << e.Message );
                    }
                }
            }
        }
        else if ( rLocalName == "library-embedded" )
        {
            // TODO: create password protected libraries

            if ( xAttributes.is() )
            {
                OUString aName = xAttributes->getValueByUidName( m_xImport->XMLNS_UID, "name" );

                bool bReadOnly = false;
                getBoolAttr( &bReadOnly, "readonly", xAttributes, m_xImport->XMLNS_UID );

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
                            xElement.set( new BasicEmbeddedLibraryElement( rLocalName, xAttributes, this, m_xImport.get(), m_xLibContainer, aName, bReadOnly ) );
                    }
                    catch ( const lang::IllegalArgumentException& e )
                    {
                        SAL_INFO("xmlscript.xmlflat", "BasicLibrariesElement::startChildElement: caught IllegalArgumentException reason " << e.Message );
                    }
                }
            }
        }
        else
        {
            throw xml::sax::SAXException( "expected library-linked or library-embedded element!", Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    void BasicLibrariesElement::endElement()
    {
    }

    // BasicEmbeddedLibraryElement

    BasicEmbeddedLibraryElement::BasicEmbeddedLibraryElement( const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< script::XLibraryContainer2 >& rxLibContainer,
            const OUString& rLibName, bool bReadOnly )
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
        catch ( const lang::WrappedTargetException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "BasicEmbeddedLibraryElement CTOR: caught WrappedTargetException reason " << e.Message );
        }
    }

    // XElement

    Reference< xml::input::XElement > BasicEmbeddedLibraryElement::startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes )
    {
        Reference< xml::input::XElement > xElement;

        if ( nUid != m_xImport->XMLNS_UID )
        {
            throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
        }
        else if ( rLocalName == "module" )
        {
            if ( xAttributes.is() )
            {
                OUString aName = xAttributes->getValueByUidName(m_xImport->XMLNS_UID, "name" );

                if ( m_xLib.is() && !aName.isEmpty() )
                    xElement.set( new BasicModuleElement( rLocalName, xAttributes, this, m_xImport.get(), m_xLib, aName ) );
            }
        }
        else
        {
            throw xml::sax::SAXException( "expected module element!", Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    void BasicEmbeddedLibraryElement::endElement()
    {
        if ( m_xLibContainer.is() && m_xLibContainer->hasByName( m_aLibName ) && m_bReadOnly )
            m_xLibContainer->setLibraryReadOnly( m_aLibName, m_bReadOnly );
    }

    // BasicModuleElement

    BasicModuleElement::BasicModuleElement( const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< container::XNameContainer >& rxLib, const OUString& rName )
        :BasicElementBase( rLocalName, xAttributes, pParent, pImport )
        ,m_xLib( rxLib )
        ,m_aName( rName )
    {
    }

    // XElement

    Reference< xml::input::XElement > BasicModuleElement::startChildElement(
            sal_Int32 nUid, const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes )
    {
        // TODO: <byte-code>

        Reference< xml::input::XElement > xElement;

        if ( nUid != m_xImport->XMLNS_UID )
        {
            throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
        }
        else if ( rLocalName == "source-code" )
        {
            // TODO: password protected libraries

            if ( xAttributes.is() )
            {
                if ( m_xLib.is() && !m_aName.isEmpty() )
                    xElement.set( new BasicSourceCodeElement( rLocalName, xAttributes, this, m_xImport.get(), m_xLib, m_aName ) );
            }
        }
        else
        {
            throw xml::sax::SAXException( "expected source-code element!", Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    void BasicModuleElement::endElement()
    {
    }

    // BasicSourceCodeElement

    BasicSourceCodeElement::BasicSourceCodeElement( const OUString& rLocalName,
            const Reference< xml::input::XAttributes >& xAttributes,
            BasicElementBase* pParent, BasicImport* pImport,
            const Reference< container::XNameContainer >& rxLib, const OUString& rName )
        :BasicElementBase( rLocalName, xAttributes, pParent, pImport )
        ,m_xLib( rxLib )
        ,m_aName( rName )
    {
    }

    // XElement

    void BasicSourceCodeElement::characters( const OUString& rChars )
    {
        m_aBuffer.append( rChars );
    }

    void BasicSourceCodeElement::endElement()
    {
        try
        {
            if ( m_xLib.is() && !m_aName.isEmpty() )
            {
                Any aElement;
                aElement <<= m_aBuffer.makeStringAndClear();
                m_xLib->insertByName( m_aName, aElement );
            }
        }
        catch ( const container::ElementExistException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "BasicSourceCodeElement::endElement: caught ElementExceptionExist reason " << e.Message );
        }
        catch ( const lang::IllegalArgumentException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "BasicSourceCodeElement::endElement: caught IllegalArgumentException reason " << e.Message );
        }
        catch ( const lang::WrappedTargetException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "BasicSourceCodeElement::endElement: caught WrappedTargetException reason " << e.Message );
        }
    }

    // BasicImport

    BasicImport::BasicImport( const Reference< frame::XModel >& rxModel, bool bOasis )
        : XMLNS_UID(0)
        , XMLNS_XLINK_UID(0)
        , m_xModel(rxModel)
        , m_bOasis(bOasis)
    {
    }

    BasicImport::~BasicImport()
    {
    }

    // XRoot

    void BasicImport::startDocument( const Reference< xml::input::XNamespaceMapping >& xNamespaceMapping )
    {
        if ( xNamespaceMapping.is() )
        {
            OUString aURI;
            if ( m_bOasis )
                aURI = XMLNS_OOO_URI;
            else
                aURI = XMLNS_SCRIPT_URI;
            XMLNS_UID = xNamespaceMapping->getUidByUri( aURI );
            XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri( XMLNS_XLINK_URI );
        }
    }

    void BasicImport::endDocument()
    {
    }

void BasicImport::processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ )
    {
    }

void BasicImport::setDocumentLocator( const Reference< xml::sax::XLocator >& /*xLocator*/ )
    {
    }

    Reference< xml::input::XElement > BasicImport::startRootElement( sal_Int32 nUid, const OUString& rLocalName,
            Reference< xml::input::XAttributes > const & xAttributes )
    {
        Reference< xml::input::XElement > xElement;

        if ( nUid != XMLNS_UID )
        {
            throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
        }
        else if ( rLocalName == "libraries" )
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
                    xPSet->getPropertyValue("BasicLibraries" ) >>= xLibContainer;
            }

            SAL_WARN_IF( !xLibContainer.is(), "xmlscript.xmlflat", "BasicImport::startRootElement: nowhere to import to!" );

            if ( xLibContainer.is() )
            {
                xElement.set( new BasicLibrariesElement( rLocalName, xAttributes, this, xLibContainer ) );
            }
        }
        else
        {
            throw xml::sax::SAXException("illegal root element (expected libraries) given: " + rLocalName, Reference< XInterface >(), Any() );
        }

        return xElement;
    }

    // XMLBasicImporterBase

    XMLBasicImporterBase::XMLBasicImporterBase( const Reference< XComponentContext >& rxContext, bool bOasis )
        :m_xContext( rxContext )
        ,m_bOasis( bOasis )
    {
    }

    XMLBasicImporterBase::~XMLBasicImporterBase()
    {
    }

    // XServiceInfo
    sal_Bool XMLBasicImporterBase::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    // XImporter
    void XMLBasicImporterBase::setTargetDocument( const Reference< XComponent >& rxDoc )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_xModel.set( rxDoc, UNO_QUERY );

        if ( !m_xModel.is() )
        {
            throw IllegalArgumentException( "XMLBasicExporter::setTargetDocument: no document model!", Reference< XInterface >(), 1 );
        }

        if ( m_xContext.is() )
        {
            Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
            if ( xSMgr.is() )
            {
                Reference < xml::input::XRoot > xRoot( new BasicImport( m_xModel, m_bOasis ) );
                Sequence < Any > aArgs( 1 );
                aArgs[0] <<= xRoot;
                m_xHandler.set( xSMgr->createInstanceWithArgumentsAndContext("com.sun.star.xml.input.SaxDocumentHandler", aArgs, m_xContext ), UNO_QUERY );
            }
        }
    }

    // XDocumentHandler

    void XMLBasicImporterBase::startDocument()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->startDocument();
    }

    void XMLBasicImporterBase::endDocument()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->endDocument();
    }

    void XMLBasicImporterBase::startElement( const OUString& aName,
            const Reference< xml::sax::XAttributeList >& xAttribs )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->startElement( aName, xAttribs );
    }

    void XMLBasicImporterBase::endElement( const OUString& aName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->endElement( aName );
    }

    void XMLBasicImporterBase::characters( const OUString& aChars )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->characters( aChars );
    }

    void XMLBasicImporterBase::ignorableWhitespace( const OUString& aWhitespaces )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->ignorableWhitespace( aWhitespaces );
    }

    void XMLBasicImporterBase::processingInstruction( const OUString& aTarget,
            const OUString& aData )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->processingInstruction( aTarget, aData );
    }

    void XMLBasicImporterBase::setDocumentLocator( const Reference< xml::sax::XLocator >& xLocator )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_xHandler.is() )
            m_xHandler->setDocumentLocator( xLocator );
    }

    // XMLBasicImporter

    XMLBasicImporter::XMLBasicImporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicImporterBase( rxContext, false )
    {
    }

    XMLBasicImporter::~XMLBasicImporter()
    {
    }

    // XServiceInfo

    OUString XMLBasicImporter::getImplementationName(  )
    {
        return OUString( "com.sun.star.comp.xmlscript.XMLBasicImporter" );
    }

    Sequence< OUString > XMLBasicImporter::getSupportedServiceNames(  )
    {
        Sequence< OUString > aNames { "com.sun.star.document.XMLBasicImporter" };
        return aNames;
    }

    // XMLOasisBasicImporter

    XMLOasisBasicImporter::XMLOasisBasicImporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicImporterBase( rxContext, true )
    {
    }

    XMLOasisBasicImporter::~XMLOasisBasicImporter()
    {
    }

    // XServiceInfo

    OUString XMLOasisBasicImporter::getImplementationName(  )
    {
        return OUString( "com.sun.star.comp.xmlscript.XMLOasisBasicImporter" );
    }

    Sequence< OUString > XMLOasisBasicImporter::getSupportedServiceNames(  )
    {
        Sequence< OUString > aNames { "com.sun.star.document.XMLOasisBasicImporter" };
        return aNames;
    }

}   // namespace xmlscript

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_xmlscript_XMLBasicImporter(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new xmlscript::XMLBasicImporter(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_xmlscript_XMLOasisBasicImporter(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new xmlscript::XMLOasisBasicImporter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
