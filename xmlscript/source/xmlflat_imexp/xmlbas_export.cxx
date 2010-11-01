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
#include "xmlbas_export.hxx"
#include "xmlscript/xmlns.h"
#include "xmlscript/xml_helper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
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
    // component operations
    // =============================================================================

    ::rtl::OUString getImplementationName_XMLBasicExporter()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.xmlscript.XMLBasicExporter" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > getSupportedServiceNames_XMLBasicExporter()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLBasicExporter" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString getImplementationName_XMLOasisBasicExporter()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.xmlscript.XMLOasisBasicExporter" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > getSupportedServiceNames_XMLOasisBasicExporter()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLOasisBasicExporter" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }


    // =============================================================================
    // XMLBasicExporterBase
    // =============================================================================

    XMLBasicExporterBase::XMLBasicExporterBase( const Reference< XComponentContext >& rxContext, sal_Bool bOasis )
        :m_xContext( rxContext )
        ,m_bOasis( bOasis )
    {
    }

    // -----------------------------------------------------------------------------

    XMLBasicExporterBase::~XMLBasicExporterBase()
    {
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    sal_Bool XMLBasicExporterBase::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void XMLBasicExporterBase::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( aArguments.getLength() == 1 )
        {
            aArguments[0] >>= m_xHandler;

            if ( !m_xHandler.is() )
            {
                throw RuntimeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLBasicExporterBase::initialize: invalid argument format!" ) ),
                    Reference< XInterface >() );
            }
        }
        else
        {
            throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLBasicExporterBase::initialize: invalid number of arguments!" ) ),
                Reference< XInterface >() );
        }
    }

    // -----------------------------------------------------------------------------
    // XExporter
    // -----------------------------------------------------------------------------

    void XMLBasicExporterBase::setSourceDocument( const Reference< XComponent >& rxDoc )
        throw (IllegalArgumentException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_xModel.set( rxDoc, UNO_QUERY );

        if ( !m_xModel.is() )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLBasicExporter::setSourceDocument: no document model!" ) ),
                Reference< XInterface >(), 1 );
        }
    }

    // -----------------------------------------------------------------------------
    // XFilter
    // -----------------------------------------------------------------------------

sal_Bool XMLBasicExporterBase::filter( const Sequence< beans::PropertyValue >& /*aDescriptor*/ )
        throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Bool bReturn = sal_True;

        try
        {
            if ( m_xHandler.is() )
            {
                m_xHandler->startDocument();

                // ooo/script prefix and URI
                ::rtl::OUString aPrefix;
                ::rtl::OUString aURI;
                if ( m_bOasis )
                {
                    aPrefix = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_OOO_PREFIX ) );
                    aURI = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_OOO_URI ) );
                }
                else
                {
                    aPrefix = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_SCRIPT_PREFIX ) );
                    aURI = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_SCRIPT_URI ) );
                }

                // ooo/script:libraries element
                ::rtl::OUString aLibContElementName( aPrefix );
                aLibContElementName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":libraries" ) );
                XMLElement* pLibContElement = new XMLElement( aLibContElementName );
                Reference< xml::sax::XAttributeList > xLibContAttribs( pLibContElement );

                // ooo/script namespace attribute
                pLibContElement->addAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "xmlns:" ) ) + aPrefix,
                    aURI );

                // xlink namespace attribute
                pLibContElement->addAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "xmlns:" XMLNS_XLINK_PREFIX ) ),
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_URI ) ) );

                // <ooo/script:libraries...
                m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                m_xHandler->startElement( aLibContElementName, xLibContAttribs );

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

                OSL_ENSURE( xLibContainer.is(), "XMLBasicExporterBase::filter: nowhere to export to!" );

                if ( xLibContainer.is() )
                {
                    Sequence< ::rtl::OUString > aLibNames = xLibContainer->getElementNames();
                    sal_Int32 nLibCount = aLibNames.getLength();
                    const ::rtl::OUString* pLibNames = aLibNames.getConstArray();
                    for ( sal_Int32 i = 0 ; i < nLibCount ; ++i )
                    {
                        ::rtl::OUString aLibName( pLibNames[i] );

                        if ( xLibContainer->hasByName( aLibName ) )
                        {
                            ::rtl::OUString aTrueStr( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

                            if ( xLibContainer->isLibraryLink( aLibName ) )
                            {
                                // ooo/script:library-linked element
                                ::rtl::OUString aLibElementName( aPrefix );
                                aLibElementName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":library-linked" ) );
                                XMLElement* pLibElement = new XMLElement( aLibElementName );
                                Reference< xml::sax::XAttributeList > xLibAttribs;
                                xLibAttribs = static_cast< xml::sax::XAttributeList* >( pLibElement );

                                // ooo/script:name attribute
                                pLibElement->addAttribute( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":name" ) ),
                                    aLibName );

                                ::rtl::OUString aLinkURL( xLibContainer->getLibraryLinkURL( aLibName ) );
                                if ( aLinkURL.getLength() )
                                {
                                    // xlink:href attribute
                                    pLibElement->addAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ":href" ) ),
                                        aLinkURL );

                                    // xlink:type attribute
                                    pLibElement->addAttribute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_XLINK_PREFIX ":type" ) ),
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "simple" ) ) );
                                }

                                if ( xLibContainer->isLibraryReadOnly( aLibName ) )
                                {
                                    // ooo/script:readonly attribute
                                    pLibElement->addAttribute( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":readonly" ) ),
                                        aTrueStr );
                                }

                                // <ooo/script:library-linked...
                                m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                m_xHandler->startElement( aLibElementName, xLibAttribs );

                                // ...ooo/script:library-linked>
                                m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                m_xHandler->endElement( aLibElementName );
                            }
                            else
                            {
                                // ooo/script:library-embedded element
                                ::rtl::OUString aLibElementName( aPrefix );
                                aLibElementName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":library-embedded" ) );
                                XMLElement* pLibElement = new XMLElement( aLibElementName );
                                Reference< xml::sax::XAttributeList > xLibAttribs;
                                xLibAttribs = static_cast< xml::sax::XAttributeList* >( pLibElement );

                                // ooo/script:name attribute
                                pLibElement->addAttribute( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":name" ) ),
                                    aLibName );

                                if ( xLibContainer->isLibraryReadOnly( aLibName ) )
                                {
                                    // ooo/script:readonly attribute
                                    pLibElement->addAttribute( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":readonly" ) ),
                                        aTrueStr );
                                }

                                // TODO: password protected libraries
                                Reference< script::XLibraryContainerPassword > xPasswd( xLibContainer, UNO_QUERY );
                                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) )
                                    continue;

                                // <ooo/script:library-embedded...
                                m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                m_xHandler->startElement( aLibElementName, xLibAttribs );

                                if ( !xLibContainer->isLibraryLoaded( aLibName ) )
                                    xLibContainer->loadLibrary( aLibName );

                                Reference< container::XNameContainer > xLib;
                                xLibContainer->getByName( aLibName ) >>= xLib;

                                if ( xLib.is() )
                                {
                                    Sequence< ::rtl::OUString > aModNames = xLib->getElementNames();
                                    sal_Int32 nModCount = aModNames.getLength();
                                    const ::rtl::OUString* pModNames = aModNames.getConstArray();
                                    for ( sal_Int32 j = 0 ; j < nModCount ; ++j )
                                    {
                                        ::rtl::OUString aModName( pModNames[j] );
                                        if ( xLib->hasByName( aModName ) )
                                        {
                                            // ooo/script:module element
                                            ::rtl::OUString aModElementName( aPrefix );
                                            aModElementName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":module" ) );
                                            XMLElement* pModElement = new XMLElement( aModElementName );
                                            Reference< xml::sax::XAttributeList > xModAttribs;
                                            xModAttribs = static_cast< xml::sax::XAttributeList* >( pModElement );

                                            // ooo/script:name attribute
                                            pModElement->addAttribute( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":name" ) ),
                                                aModName );

                                            // <ooo/script:module...
                                            m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                            m_xHandler->startElement( aModElementName, xModAttribs );

                                            // ooo/script:source-code element
                                            ::rtl::OUString aSourceElementName( aPrefix );
                                            aSourceElementName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":source-code" ) );
                                            XMLElement* pSourceElement = new XMLElement( aSourceElementName );
                                            Reference< xml::sax::XAttributeList > xSourceAttribs;
                                            xSourceAttribs = static_cast< xml::sax::XAttributeList* >( pSourceElement );

                                            // <ooo/script:source-code...
                                            m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                            m_xHandler->startElement( aSourceElementName, xSourceAttribs );

                                            // module data
                                            // TODO: write encrypted data for password protected libraries
                                            ::rtl::OUString aSource;
                                            xLib->getByName( aModName ) >>= aSource;
                                            m_xHandler->characters( aSource );

                                            // TODO: <ooo/script:byte-code>

                                            // ...ooo/script:source-code>
                                            m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                            m_xHandler->endElement( aSourceElementName );

                                            // ...ooo/script:module>
                                            m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                            m_xHandler->endElement( aModElementName );
                                        }
                                    }
                                }

                                // ...ooo/script:library-embedded>
                                m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                                m_xHandler->endElement( aLibElementName );
                            }
                        }
                    }
                }

                // ...ooo/script:libraries>
                m_xHandler->ignorableWhitespace( ::rtl::OUString() );
                m_xHandler->endElement( aLibContElementName );

                m_xHandler->endDocument();
            }
        }
        catch ( container::NoSuchElementException& e )
        {
            OSL_TRACE( "XMLBasicExporterBase::filter: caught NoSuchElementException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            bReturn = sal_False;
        }
        catch ( lang::IllegalArgumentException& e )
        {
            OSL_TRACE( "XMLBasicExporterBase::filter: caught IllegalArgumentException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            bReturn = sal_False;
        }
        catch ( lang::WrappedTargetException& e )
        {
            OSL_TRACE( "XMLBasicExporterBase::filter: caught WrappedTargetException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            bReturn = sal_False;
        }
        catch ( xml::sax::SAXException& e )
        {
            OSL_TRACE( "XMLBasicExporterBase::filter: caught SAXException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            bReturn = sal_False;
        }

        return bReturn;
    }

    // -----------------------------------------------------------------------------

    void XMLBasicExporterBase::cancel()
        throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // cancel export
    }


    // =============================================================================
    // XMLBasicExporter
    // =============================================================================

    XMLBasicExporter::XMLBasicExporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicExporterBase( rxContext, sal_False )
    {
    }

    // -----------------------------------------------------------------------------

    XMLBasicExporter::~XMLBasicExporter()
    {
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString XMLBasicExporter::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_XMLBasicExporter();
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > XMLBasicExporter::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_XMLBasicExporter();
    }


    // =============================================================================
    // XMLOasisBasicExporter
    // =============================================================================

    XMLOasisBasicExporter::XMLOasisBasicExporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicExporterBase( rxContext, sal_True )
    {
    }

    // -----------------------------------------------------------------------------

    XMLOasisBasicExporter::~XMLOasisBasicExporter()
    {
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString XMLOasisBasicExporter::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_XMLOasisBasicExporter();
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > XMLOasisBasicExporter::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_XMLOasisBasicExporter();
    }


    // =============================================================================
    // component operations
    // =============================================================================

    Reference< XInterface > SAL_CALL create_XMLBasicExporter(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new XMLBasicExporter( xContext ) );
    }

    // -----------------------------------------------------------------------------

    Reference< XInterface > SAL_CALL create_XMLOasisBasicExporter(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new XMLOasisBasicExporter( xContext ) );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace xmlscript
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
