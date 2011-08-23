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

#ifndef _XMLOFF_XMLBASICI_HXX
#include "xmlbasici.hxx"
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//.........................................................................
namespace binfilter
{
//.........................................................................

    // =============================================================================
    // XMLBasicImportContext
    // =============================================================================

    XMLBasicImportContext::XMLBasicImportContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
            const Reference< frame::XModel >& rxModel )
        :SvXMLImportContext( rImport, nPrfx, rLName )
        ,m_xModel( rxModel )
    {
        Reference< lang::XMultiServiceFactory > xMSF = GetImport().getServiceFactory();
        if ( xMSF.is() )
        {
            m_xHandler.set( xMSF->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLBasicImporter" ) ) ), 
                UNO_QUERY );
        }

        if ( m_xHandler.is() )
        {
            Reference< document::XImporter > xImporter( m_xHandler, UNO_QUERY );
            if ( xImporter.is() )
            {
                Reference< lang::XComponent > xComp( m_xModel, UNO_QUERY );
                xImporter->setTargetDocument( xComp );
            }
        }
    }

    // -----------------------------------------------------------------------------

    XMLBasicImportContext::~XMLBasicImportContext()
    {
    }

    // -----------------------------------------------------------------------------

    SvXMLImportContext* XMLBasicImportContext::CreateChildContext( 
        USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const Reference< xml::sax::XAttributeList >& rxAttrList )
    {
        SvXMLImportContext* pContext = 0;

        if ( m_xHandler.is() )
            pContext = new XMLBasicImportChildContext( GetImport(), nPrefix, rLocalName, m_xHandler );

        if ( !pContext )
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

        return pContext;
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImportContext::StartElement(
        const Reference< xml::sax::XAttributeList >& rxAttrList )
    {
        if ( m_xHandler.is() )
        {
            m_xHandler->startDocument();

            // copy namespace declarations
            SvXMLAttributeList* pAttrList = new SvXMLAttributeList( rxAttrList );
            Reference< xml::sax::XAttributeList > xAttrList( pAttrList );
            const SvXMLNamespaceMap& rNamespaceMap = GetImport().GetNamespaceMap();
            sal_uInt16 nPos = rNamespaceMap.GetFirstKey();
            while ( nPos != USHRT_MAX )
            {
                ::rtl::OUString aAttrName( rNamespaceMap.GetAttrNameByKey( nPos ) );
                if ( xAttrList->getValueByName( aAttrName ).getLength() == 0 )
                    pAttrList->AddAttribute( aAttrName, rNamespaceMap.GetNameByKey( nPos ) );
                nPos = rNamespaceMap.GetNextKey( nPos );
            }

            m_xHandler->startElement( 
                GetImport().GetNamespaceMap().GetQNameByKey( GetPrefix(), GetLocalName() ),
                xAttrList );
        }
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImportContext::EndElement()
    {
        if ( m_xHandler.is() )
        {
            m_xHandler->endElement( 
                GetImport().GetNamespaceMap().GetQNameByKey( GetPrefix(), GetLocalName() ) );
            m_xHandler->endDocument();
        }
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImportContext::Characters( const ::rtl::OUString& rChars )
    {
        if ( m_xHandler.is() )
            m_xHandler->characters( rChars );
    }
        

    // =============================================================================
    // XMLBasicImportChildContext
    // =============================================================================

    XMLBasicImportChildContext::XMLBasicImportChildContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
            const Reference< xml::sax::XDocumentHandler >& rxHandler )
        :SvXMLImportContext( rImport, nPrfx, rLName )
        ,m_xHandler( rxHandler )
    {
    }

    // -----------------------------------------------------------------------------

    XMLBasicImportChildContext::~XMLBasicImportChildContext()
    {
    }

    // -----------------------------------------------------------------------------

    SvXMLImportContext* XMLBasicImportChildContext::CreateChildContext( 
        USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const Reference< xml::sax::XAttributeList >& xAttrList )
    {    
        return new XMLBasicImportChildContext( GetImport(), nPrefix, rLocalName, m_xHandler );
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImportChildContext::StartElement(
        const Reference< xml::sax::XAttributeList >& xAttrList )
    {
        if ( m_xHandler.is() )
        {
            m_xHandler->startElement( 
                GetImport().GetNamespaceMap().GetQNameByKey( GetPrefix(), GetLocalName() ),
                xAttrList );
        }
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImportChildContext::EndElement()
    {
        if ( m_xHandler.is() )
        {
            m_xHandler->endElement( 
                GetImport().GetNamespaceMap().GetQNameByKey( GetPrefix(), GetLocalName() ) );
        }
    }

    // -----------------------------------------------------------------------------

    void XMLBasicImportChildContext::Characters( const ::rtl::OUString& rChars )
    {
        if ( m_xHandler.is() )
            m_xHandler->characters( rChars );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}	// namespace binfilter
//.........................................................................
