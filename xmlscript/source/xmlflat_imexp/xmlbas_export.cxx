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

#include "xmlbas_export.hxx"
#include <sal/log.hxx>
#include <xmlscript/xmlns.h>
#include <xmlscript/xml_helper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace xmlscript
{
    // XMLBasicExporterBase

    XMLBasicExporterBase::XMLBasicExporterBase( bool bOasis )
        :m_bOasis( bOasis )
    {
    }

    XMLBasicExporterBase::~XMLBasicExporterBase()
    {
    }

    // XServiceInfo
    sal_Bool XMLBasicExporterBase::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    // XInitialization

    void XMLBasicExporterBase::initialize( const Sequence< Any >& aArguments )
    {
        std::scoped_lock aGuard( m_aMutex );

        if ( aArguments.getLength() != 1 )
        {
            throw RuntimeException( u"XMLBasicExporterBase::initialize: invalid number of arguments!"_ustr );
        }

        aArguments[0] >>= m_xHandler;

        if ( !m_xHandler.is() )
        {
            throw RuntimeException( u"XMLBasicExporterBase::initialize: invalid argument format!"_ustr );
        }
    }

    // XExporter

    void XMLBasicExporterBase::setSourceDocument( const Reference< XComponent >& rxDoc )
    {
        std::scoped_lock aGuard( m_aMutex );

        m_xModel.set( rxDoc, UNO_QUERY );

        if ( !m_xModel.is() )
        {
            throw IllegalArgumentException( u"XMLBasicExporter::setSourceDocument: no document model!"_ustr, Reference< XInterface >(), 1 );
        }
    }

    // XFilter

sal_Bool XMLBasicExporterBase::filter( const Sequence< beans::PropertyValue >& /*aDescriptor*/ )
    {
        std::scoped_lock aGuard( m_aMutex );

        bool bReturn = true;

        try
        {
            if ( m_xHandler.is() )
            {
                m_xHandler->startDocument();

                // ooo/script prefix and URI
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

                // ooo/script:libraries element
                OUString aLibContElementName = aPrefix + ":libraries";
                rtl::Reference<XMLElement> pLibContElement = new XMLElement( aLibContElementName );

                // ooo/script namespace attribute
                pLibContElement->addAttribute( "xmlns:" + aPrefix, aURI );

                // xlink namespace attribute
                pLibContElement->addAttribute( u"xmlns:" XMLNS_XLINK_PREFIX ""_ustr, XMLNS_XLINK_URI );

                // <ooo/script:libraries...
                m_xHandler->ignorableWhitespace( OUString() );
                m_xHandler->startElement( aLibContElementName, pLibContElement );

                Reference< script::XLibraryContainer2 > xLibContainer;

                // try the XEmbeddedScripts interface
                Reference< document::XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
                if ( xDocumentScripts.is() )
                    xLibContainer = xDocumentScripts->getBasicLibraries();

                if ( !xLibContainer.is() )
                {
                    // try the "BasicLibraries" property (old-style, for compatibility)
                    Reference< beans::XPropertySet > xPSet( m_xModel, UNO_QUERY );
                    if ( xPSet.is() )
                        xPSet->getPropertyValue(u"BasicLibraries"_ustr ) >>= xLibContainer;
                }

                SAL_WARN_IF( !xLibContainer.is(), "xmlscript.xmlflat", "XMLBasicExporterBase::filter: nowhere to export to!" );

                if ( xLibContainer.is() )
                {
                    const Sequence< OUString > aLibNames = xLibContainer->getElementNames();
                    for ( const OUString& rLibName : aLibNames )
                    {
                        if ( xLibContainer->hasByName( rLibName ) )
                        {
                            OUString aTrueStr( u"true"_ustr );

                            if ( xLibContainer->isLibraryLink( rLibName ) )
                            {
                                // ooo/script:library-linked element
                                OUString aLibElementName = aPrefix + ":library-linked";
                                rtl::Reference<XMLElement> pLibElement = new XMLElement( aLibElementName );

                                // ooo/script:name attribute
                                pLibElement->addAttribute( aPrefix + ":name", rLibName );

                                OUString aLinkURL( xLibContainer->getLibraryLinkURL( rLibName ) );
                                if ( !aLinkURL.isEmpty() )
                                {
                                    // xlink:href attribute
                                    pLibElement->addAttribute( u"" XMLNS_XLINK_PREFIX ":href"_ustr, aLinkURL );

                                    // xlink:type attribute
                                    pLibElement->addAttribute( u"" XMLNS_XLINK_PREFIX ":type"_ustr, u"simple"_ustr );
                                }

                                if ( xLibContainer->isLibraryReadOnly( rLibName ) )
                                {
                                    // ooo/script:readonly attribute
                                    pLibElement->addAttribute( aPrefix + ":readonly", aTrueStr );
                                }

                                // <ooo/script:library-linked...
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->startElement( aLibElementName, pLibElement );

                                // ...ooo/script:library-linked>
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->endElement( aLibElementName );
                            }
                            else
                            {
                                // ooo/script:library-embedded element
                                OUString aLibElementName = aPrefix + ":library-embedded";
                                rtl::Reference<XMLElement> pLibElement = new XMLElement( aLibElementName );

                                // ooo/script:name attribute
                                pLibElement->addAttribute( aPrefix + ":name", rLibName );

                                if ( xLibContainer->isLibraryReadOnly( rLibName ) )
                                {
                                    // ooo/script:readonly attribute
                                    pLibElement->addAttribute( aPrefix + ":readonly", aTrueStr );
                                }

                                // TODO: password protected libraries
                                Reference< script::XLibraryContainerPassword > xPasswd( xLibContainer, UNO_QUERY );
                                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( rLibName ) )
                                    continue;

                                // <ooo/script:library-embedded...
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->startElement( aLibElementName, pLibElement );

                                if ( !xLibContainer->isLibraryLoaded( rLibName ) )
                                    xLibContainer->loadLibrary( rLibName );

                                Reference< container::XNameContainer > xLib;
                                xLibContainer->getByName( rLibName ) >>= xLib;

                                if ( xLib.is() )
                                {
                                    const Sequence< OUString > aModNames = xLib->getElementNames();
                                    for ( const OUString& rModName : aModNames )
                                    {
                                        if ( xLib->hasByName( rModName ) )
                                        {
                                            // ooo/script:module element
                                            OUString aModElementName = aPrefix + ":module";
                                            rtl::Reference<XMLElement> pModElement = new XMLElement( aModElementName );

                                            // ooo/script:name attribute
                                            pModElement->addAttribute( aPrefix + ":name", rModName );

                                            // <ooo/script:module...
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->startElement( aModElementName, pModElement );

                                            // ooo/script:source-code element
                                            OUString aSourceElementName = aPrefix + ":source-code";
                                            rtl::Reference<XMLElement> pSourceElement = new XMLElement( aSourceElementName );

                                            // <ooo/script:source-code...
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->startElement( aSourceElementName, pSourceElement );

                                            // module data
                                            // TODO: write encrypted data for password protected libraries
                                            OUString aSource;
                                            xLib->getByName( rModName ) >>= aSource;
                                            m_xHandler->characters( aSource );

                                            // TODO: <ooo/script:byte-code>

                                            // ...ooo/script:source-code>
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->endElement( aSourceElementName );

                                            // ...ooo/script:module>
                                            m_xHandler->ignorableWhitespace( OUString() );
                                            m_xHandler->endElement( aModElementName );
                                        }
                                    }
                                }

                                // ...ooo/script:library-embedded>
                                m_xHandler->ignorableWhitespace( OUString() );
                                m_xHandler->endElement( aLibElementName );
                            }
                        }
                    }
                }

                // ...ooo/script:libraries>
                m_xHandler->ignorableWhitespace( OUString() );
                m_xHandler->endElement( aLibContElementName );

                m_xHandler->endDocument();
            }
        }
        catch ( const container::NoSuchElementException& )
        {
            TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "XMLBasicExporterBase::filter" );
            bReturn = false;
        }
        catch ( const lang::IllegalArgumentException& )
        {
            TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "XMLBasicExporterBase::filter" );
            bReturn = false;
        }
        catch ( const lang::WrappedTargetException& )
        {
            TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "XMLBasicExporterBase::filter:" );
            bReturn = false;
        }
        catch ( const xml::sax::SAXException& )
        {
            TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "XMLBasicExporterBase::filter:" );
            bReturn = false;
        }

        return bReturn;
    }

    void XMLBasicExporterBase::cancel()
    {
        // cancel export
    }

    // XMLBasicExporter

    XMLBasicExporter::XMLBasicExporter()
        :XMLBasicExporterBase( false )
    {
    }

    XMLBasicExporter::~XMLBasicExporter()
    {
    }

    // XServiceInfo

    OUString XMLBasicExporter::getImplementationName(  )
    {
        return u"com.sun.star.comp.xmlscript.XMLBasicExporter"_ustr;
    }

    Sequence< OUString > XMLBasicExporter::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.document.XMLBasicExporter"_ustr };
    }

    // XMLOasisBasicExporter

    XMLOasisBasicExporter::XMLOasisBasicExporter()
        :XMLBasicExporterBase( true )
    {
    }

    XMLOasisBasicExporter::~XMLOasisBasicExporter()
    {
    }

    // XServiceInfo

    OUString XMLOasisBasicExporter::getImplementationName(  )
    {
        return u"com.sun.star.comp.xmlscript.XMLOasisBasicExporter"_ustr;
    }

    Sequence< OUString > XMLOasisBasicExporter::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.document.XMLOasisBasicExporter"_ustr };
    }

}   // namespace xmlscript

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_xmlscript_XMLBasicExporter(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new xmlscript::XMLBasicExporter());
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_xmlscript_XMLOasisBasicExporter(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{

    return cppu::acquire(new xmlscript::XMLOasisBasicExporter());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
