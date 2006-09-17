/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlbasici.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:41:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

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
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLOasisBasicImporter" ) ) ),
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
    const Reference< xml::sax::XAttributeList >& )
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
    const Reference< xml::sax::XAttributeList >& )
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
