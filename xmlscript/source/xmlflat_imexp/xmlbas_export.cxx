/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "xmlbas_export.hxx"
#include <xmlscript/xmlns.h>
#include <xmlscript/xml_helper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

    

    OUString getImplementationName_XMLBasicExporter()
    {
        static OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static OUString aImplName( "com.sun.star.comp.xmlscript.XMLBasicExporter" );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    Sequence< OUString > getSupportedServiceNames_XMLBasicExporter()
    {
        static Sequence< OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< OUString > aNames(1);
                aNames.getArray()[0] = "com.sun.star.document.XMLBasicExporter";
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    OUString getImplementationName_XMLOasisBasicExporter()
    {
        static OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static OUString aImplName( "com.sun.star.comp.xmlscript.XMLOasisBasicExporter" );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    Sequence< OUString > getSupportedServiceNames_XMLOasisBasicExporter()
    {
        static Sequence< OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< OUString > aNames(1);
                aNames.getArray()[0] = "com.sun.star.document.XMLOasisBasicExporter";
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    

    XMLBasicExporterBase::XMLBasicExporterBase( const Reference< XComponentContext >& rxContext, bool bOasis )
        :m_xContext( rxContext )
        ,m_bOasis( bOasis )
    {
    }

    XMLBasicExporterBase::~XMLBasicExporterBase()
    {
    }

    
    sal_Bool XMLBasicExporterBase::supportsService( const OUString& rServiceName ) throw (RuntimeException)
    {
        return cppu::supportsService(this, rServiceName);
    }

    

    void XMLBasicExporterBase::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( aArguments.getLength() == 1 )
        {
            aArguments[0] >>= m_xHandler;

            if ( !m_xHandler.is() )
            {
                throw RuntimeException( "XMLBasicExporterBase::initialize: invalid argument format!", Reference< XInterface >() );
            }
        }
        else
        {
            throw RuntimeException( "XMLBasicExporterBase::initialize: invalid number of arguments!", Reference< XInterface >() );
        }
    }

    

    void XMLBasicExporterBase::setSourceDocument( const Reference< XComponent >& rxDoc )
        throw (IllegalArgumentException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_xModel.set( rxDoc, UNO_QUERY );

        if ( !m_xModel.is() )
        {
            throw IllegalArgumentException( "XMLBasicExporter::setSourceDocument: no document model!", Reference< XInterface >(), 1 );
        }
    }

    

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

                
                OUString aPrefix;
                OUString aURI;
                if ( m_bOasis )
                {
                    aPrefix = XMLNS_OOO_PREFIX;
                    aURI = XMLNS_OOO_URI;
                }
                else
                {
                    aPrefix = XMLNS_SCRIPT_PREFIX;
                    aURI = XMLNS_SCRIPT_URI;
                }

                
                OUString aLibContElementName( aPrefix );
                aLibContElementName += ":libraries";
                XMLElement* pLibContElement = new XMLElement( aLibContElementName );
                Reference< xml::sax::XAttributeList > xLibContAttribs( pLibContElement );

                
                pLibContElement->addAttribute( "xmlns:" + aPrefix, aURI );

                
                pLibContElement->addAttribute( "xmlns:" XMLNS_XLINK_PREFIX, XMLNS_XLINK_URI );

                
                m_xHandler->ignorableWhitespace( OUString() );
                m_xHandler->startElement( aLibContElementName, xLibContAttribs );

                Reference< script::XLibraryContainer2 > xLibContainer;

                
                Reference< document::XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
                if ( xDocumentScripts.is() )
                    xLibContainer.set( xDocumentScripts->getBasicLibraries().get() );

                if ( !xLibContainer.is() )
                {
                    
                    Reference< beans::XPropertySet > xPSet( m_xModel, UNO_QUERY );
                    if ( xPSet.is() )
                        xPSet->getPropertyValue("BasicLibraries" ) >>= xLibContainer;
                }

                SAL_WARN_IF( !xLibContainer.is(), "xmlscript.xmlflat", "XMLBasicExporterBase::filter: nowhere to export to!" );

                if ( xLibContainer.is() )
                {
                    Sequence< OUString > aLibNames = xLibContainer->getElementNames();
                    sal_Int32 nLibCount = aLibNames.getLength();
                    const OUString* pLibNames = aLibNames.getConstArray();
                    for ( sal_Int32 i = 0 ; i < nLibCount ; ++i )
                    {
                        OUString aLibName( pLibNames[i] );

                        if ( xLibContainer->hasByName( aLibName ) )
                        {
                            OUString aTrueStr( "true" );

                            if ( xLibContainer->isLibraryLink( aLibName ) )
                            {
                                
                                OUString aLibElementName( aPrefix );
                                aLibElementName +=  ":library-linked";
                                XMLElement* pLibElement = new XMLElement( aLibElementName );
                                Reference< xml::sax::XAttributeList > xLibAttribs;
                                xLibAttribs = static_cast< xml::sax::XAttributeList* >( pLibElement );

                                
                                pLibElement->addAttribute( aPrefix + ":name", aLibName );

                                OUString aLinkURL( xLibContainer->getLibraryLinkURL( aLibName ) );
                                if ( !aLinkURL.isEmpty() )
                                {
                                    
                                    pLibElement->addAttribute( XMLNS_XLINK_PREFIX ":href", aLinkURL );

                                    
                                    pLibElement->addAttribute( XMLNS_XLINK_PREFIX ":type", "simple" );
                                }

                                if ( xLibContainer->isLibraryReadOnly( aLibName ) )
                                {
                                    
                                    pLibElement->addAttribute( aPrefix + ":readonly", aTrueStr );
                                }

                                
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->startElement( aLibElementName, xLibAttribs );

                                
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->endElement( aLibElementName );
                            }
                            else
                            {
                                
                                OUString aLibElementName( aPrefix );
                                aLibElementName += ":library-embedded";
                                XMLElement* pLibElement = new XMLElement( aLibElementName );
                                Reference< xml::sax::XAttributeList > xLibAttribs;
                                xLibAttribs = static_cast< xml::sax::XAttributeList* >( pLibElement );

                                
                                pLibElement->addAttribute( aPrefix + ":name", aLibName );

                                if ( xLibContainer->isLibraryReadOnly( aLibName ) )
                                {
                                    
                                    pLibElement->addAttribute( aPrefix + ":readonly", aTrueStr );
                                }

                                
                                Reference< script::XLibraryContainerPassword > xPasswd( xLibContainer, UNO_QUERY );
                                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) )
                                    continue;

                                
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->startElement( aLibElementName, xLibAttribs );

                                if ( !xLibContainer->isLibraryLoaded( aLibName ) )
                                    xLibContainer->loadLibrary( aLibName );

                                Reference< container::XNameContainer > xLib;
                                xLibContainer->getByName( aLibName ) >>= xLib;

                                if ( xLib.is() )
                                {
                                    Sequence< OUString > aModNames = xLib->getElementNames();
                                    sal_Int32 nModCount = aModNames.getLength();
                                    const OUString* pModNames = aModNames.getConstArray();
                                    for ( sal_Int32 j = 0 ; j < nModCount ; ++j )
                                    {
                                        OUString aModName( pModNames[j] );
                                        if ( xLib->hasByName( aModName ) )
                                        {
                                            
                                            OUString aModElementName( aPrefix );
                                            aModElementName += ":module";
                                            XMLElement* pModElement = new XMLElement( aModElementName );
                                            Reference< xml::sax::XAttributeList > xModAttribs;
                                            xModAttribs = static_cast< xml::sax::XAttributeList* >( pModElement );

                                            
                                            pModElement->addAttribute( aPrefix + ":name", aModName );

                                            
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->startElement( aModElementName, xModAttribs );

                                            
                                            OUString aSourceElementName( aPrefix );
                                            aSourceElementName += ":source-code";
                                            XMLElement* pSourceElement = new XMLElement( aSourceElementName );
                                            Reference< xml::sax::XAttributeList > xSourceAttribs;
                                            xSourceAttribs = static_cast< xml::sax::XAttributeList* >( pSourceElement );

                                            
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->startElement( aSourceElementName, xSourceAttribs );

                                            
                                            
                                            OUString aSource;
                                            xLib->getByName( aModName ) >>= aSource;
                                            m_xHandler->characters( aSource );

                                            

                                            
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->endElement( aSourceElementName );

                                            
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->endElement( aModElementName );
                                        }
                                    }
                                }

                                
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->endElement( aLibElementName );
                            }
                        }
                    }
                }

                
                m_xHandler->ignorableWhitespace( OUString() );
                m_xHandler->endElement( aLibContElementName );

                m_xHandler->endDocument();
            }
        }
        catch ( const container::NoSuchElementException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "XMLBasicExporterBase::filter: caught NoSuchElementException reason " << e.Message );
            bReturn = sal_False;
        }
        catch ( const lang::IllegalArgumentException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "XMLBasicExporterBase::filter: caught IllegalArgumentException reason " << e.Message );
            bReturn = sal_False;
        }
        catch ( const lang::WrappedTargetException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "XMLBasicExporterBase::filter: caught WrappedTargetException reason " << e.Message );
            bReturn = sal_False;
        }
        catch ( const xml::sax::SAXException& e )
        {
            SAL_INFO("xmlscript.xmlflat", "XMLBasicExporterBase::filter: caught SAXException reason " << e.Message );
            bReturn = sal_False;
        }

        return bReturn;
    }

    void XMLBasicExporterBase::cancel()
        throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        
    }

    

    XMLBasicExporter::XMLBasicExporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicExporterBase( rxContext, false )
    {
    }

    XMLBasicExporter::~XMLBasicExporter()
    {
    }

    

    OUString XMLBasicExporter::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_XMLBasicExporter();
    }

    Sequence< OUString > XMLBasicExporter::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_XMLBasicExporter();
    }

    

    XMLOasisBasicExporter::XMLOasisBasicExporter( const Reference< XComponentContext >& rxContext )
        :XMLBasicExporterBase( rxContext, true )
    {
    }

    XMLOasisBasicExporter::~XMLOasisBasicExporter()
    {
    }

    

    OUString XMLOasisBasicExporter::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_XMLOasisBasicExporter();
    }

    Sequence< OUString > XMLOasisBasicExporter::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_XMLOasisBasicExporter();
    }

    

    Reference< XInterface > SAL_CALL create_XMLBasicExporter(
        Reference< XComponentContext > const & xContext )
        SAL_THROW(())
    {
        return static_cast< lang::XTypeProvider * >( new XMLBasicExporter( xContext ) );
    }

    Reference< XInterface > SAL_CALL create_XMLOasisBasicExporter(
        Reference< XComponentContext > const & xContext )
        SAL_THROW(())
    {
        return static_cast< lang::XTypeProvider * >( new XMLOasisBasicExporter( xContext ) );
    }

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
