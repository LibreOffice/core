/*************************************************************************
 *
 *  $RCSfile: typedetectionexport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 13:58:20 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HXX
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include "typedetectionexport.hxx"
#include "xmlfiltersettingsdialog.hxx"

#include <attributelist.hxx>

using namespace rtl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;

TypeDetectionExporter::TypeDetectionExporter( Reference< XMultiServiceFactory >& xMSF )
: mxMSF( xMSF )
{
}

static OUString createRelativeURL( const OUString& rFilterName, const OUString& rURL )
{
    if( rURL.getLength() &&
        (rURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM("http:") ) != 0) &&
        (rURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM("shttp:") ) != 0) &&
        (rURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM("jar:") ) != 0) &&
        (rURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM("ftp:") ) != 0))
    {
        INetURLObject aURL( rURL );
        OUString aName( aURL.GetName() );
        if( aName.getLength() == 0 )
        {
            sal_Int32 nPos = rURL.lastIndexOf( sal_Unicode( '/' ) );
            if( nPos == -1 )
            {
                aName = rURL;
            }
            else
            {
                aName = rURL.copy( nPos + 1 );
            }
        }

        OUString aRelURL( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) );
        aRelURL += rFilterName;
        aRelURL += OUString( sal_Unicode( '/' ) );
        aRelURL += aName;
        return aRelURL;
    }
    else
    {
        return rURL;
    }
}

void TypeDetectionExporter::doExport( Reference< XOutputStream > xOS,  const XMLFilterVector& rFilters )
{
    try
    {
        const OUString sOorNode             ( RTL_CONSTASCII_USTRINGPARAM ( "oor:node" ) );
        const OUString sNode                ( RTL_CONSTASCII_USTRINGPARAM ( "node" ) );
        const OUString sName                ( RTL_CONSTASCII_USTRINGPARAM ( "oor:name" ) );
        const OUString sWhiteSpace          ( RTL_CONSTASCII_USTRINGPARAM ( " " ) );
        const OUString sUIName              ( RTL_CONSTASCII_USTRINGPARAM ( "UIName" ) );
        const OUString sComma               ( RTL_CONSTASCII_USTRINGPARAM ( "," ) );
        const OUString sDelim               ( RTL_CONSTASCII_USTRINGPARAM ( ";" ) );
        const OUString sData                ( RTL_CONSTASCII_USTRINGPARAM ( "Data" ) );
        const OUString sDocTypePrefix       ( RTL_CONSTASCII_USTRINGPARAM ( "doctype:" ) );
        const OUString sFilterAdaptorService( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.XmlFilterAdaptor" ) );
        const OUString sXSLTFilterService   ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.documentconversion.XSLTFilter" ) );
        const OUString sCdataAttribute      ( RTL_CONSTASCII_USTRINGPARAM( "CDATA" ) );


        // set up sax writer and connect to given output stream
        Reference< XDocumentHandler > xHandler( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ), UNO_QUERY );
        Reference< XActiveDataSource > xDocSrc( xHandler, UNO_QUERY );
        xDocSrc->setOutputStream( xOS );

        AttributeList * pAttrList = new AttributeList;
        pAttrList->AddAttribute ( OUString::createFromAscii( "xmlns:oor" ), sCdataAttribute, OUString::createFromAscii( "http://openoffice.org/2001/registry" ) );
        pAttrList->AddAttribute ( OUString::createFromAscii( "xmlns:xs" ), sCdataAttribute, OUString::createFromAscii( "http://www.w3.org/2001/XMLSchema" ) );
        pAttrList->AddAttribute ( sName, sCdataAttribute, OUString::createFromAscii( "TypeDetection" ) );
        pAttrList->AddAttribute ( OUString::createFromAscii( "oor:package" ), sCdataAttribute, OUString::createFromAscii( "org.openoffice.Office" ) );
        Reference < XAttributeList > xAttrList (pAttrList);

        xHandler->startDocument();
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sOorNode, xAttrList );

        // export types
        {
            xAttrList = pAttrList = new AttributeList;
            pAttrList->AddAttribute ( sName, sCdataAttribute, OUString::createFromAscii( "Types" ) );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, xAttrList );

            XMLFilterVector::const_iterator aIter( rFilters.begin() );
            while( aIter != rFilters.end() )
            {
                const filter_info_impl* pFilter = (*aIter);

                xAttrList = pAttrList = new AttributeList;
                pAttrList->AddAttribute( sName, sCdataAttribute, pFilter->maType );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, xAttrList );
                OUString sValue( sal_Unicode('0') );
                sValue += sComma;
                sValue += sComma;
                if( pFilter->maDocType.getLength() )
                {
                    sValue += sDocTypePrefix;
                    sValue += pFilter->maDocType;
                }
                sValue += sComma;
                sValue += sComma;
                sValue += pFilter->maExtension;
                sValue += sComma;
                sValue += OUString::valueOf( pFilter->mnDocumentIconID );
                sValue += sComma;

                addProperty( xHandler, sData, sValue );
                addLocaleProperty( xHandler, sUIName, pFilter->maInterfaceName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sNode );

                aIter++;
            }

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sNode );
        }

        // export filters
        {
            xAttrList = pAttrList = new AttributeList;
            pAttrList->AddAttribute ( sName, sCdataAttribute, OUString::createFromAscii( "Filters" ) );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, xAttrList );

            XMLFilterVector::const_iterator aIter( rFilters.begin() );
            while( aIter != rFilters.end() )
            {
                const filter_info_impl* pFilter = (*aIter);

                xAttrList = pAttrList = new AttributeList;
                pAttrList->AddAttribute( sName, sCdataAttribute, pFilter->maFilterName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, xAttrList );
                addLocaleProperty( xHandler, sUIName, pFilter->maInterfaceName );

                OUString sValue( sal_Unicode('0') );
                sValue += sComma;
                sValue += pFilter->maType;
                sValue += sComma,
                sValue += pFilter->maDocumentService;
                sValue += sComma,
                sValue += sFilterAdaptorService;
                sValue += sComma,
                sValue += OUString::valueOf( pFilter->maFlags );
                sValue += sComma;
                sValue += sXSLTFilterService;
                sValue += sDelim;
                sValue += sDelim;

                const application_info_impl* pAppInfo = getApplicationInfo( pFilter->maDocumentService );
                sValue += pAppInfo->maXMLImporter;
                sValue += sDelim;
                sValue += pAppInfo->maXMLExporter;
                sValue += sDelim;

                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maImportXSLT );
                sValue += sDelim;
                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maExportXSLT );
                sValue += sDelim;
                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maDTD );
                sValue += sDelim;
                sValue += pFilter->maComment;
                sValue += sComma;
                sValue += OUString( sal_Unicode('0') );
                sValue += sComma;
                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maImportTemplate );
                addProperty( xHandler, sData, sValue );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sNode );
                aIter++;
            }

            xHandler->endElement( sNode );
        }

        // finish
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sOorNode );
        xHandler->endDocument();
    }
    catch( Exception& )
    {
        DBG_ERROR( "TypeDetectionExporter::doExport exception catched!" );
    }
}

void TypeDetectionExporter::addProperty( Reference< XDocumentHandler > xHandler, const OUString& rName, const OUString& rValue )
{
    try
    {
        const OUString sCdataAttribute( RTL_CONSTASCII_USTRINGPARAM( "CDATA" ) );
        const OUString sProp( RTL_CONSTASCII_USTRINGPARAM( "prop" ) );
        const OUString sValue( RTL_CONSTASCII_USTRINGPARAM( "value" ) );
        const OUString sWhiteSpace          ( RTL_CONSTASCII_USTRINGPARAM ( " " ) );

        AttributeList * pAttrList = new AttributeList;
        pAttrList->AddAttribute ( OUString::createFromAscii( "oor:name" ), sCdataAttribute, rName );
        pAttrList->AddAttribute ( OUString::createFromAscii( "oor:type" ), sCdataAttribute, OUString::createFromAscii( "xs:string" ) );
        Reference < XAttributeList > xAttrList (pAttrList);

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sProp, xAttrList );
        xAttrList = NULL;
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sValue,xAttrList );
        xHandler->characters( rValue );
        xHandler->endElement( sValue );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sProp );
    }
    catch( Exception& )
    {
        DBG_ERROR( "TypeDetectionExporter::addProperty exception catched!" );
    }
}

void TypeDetectionExporter::addLocaleProperty( Reference< XDocumentHandler > xHandler, const OUString& rName, const OUString& rValue )
{
    try
    {
        const OUString sCdataAttribute( RTL_CONSTASCII_USTRINGPARAM( "CDATA" ) );
        const OUString sProp( RTL_CONSTASCII_USTRINGPARAM( "prop" ) );
        const OUString sValue( RTL_CONSTASCII_USTRINGPARAM( "value" ) );
        const OUString sWhiteSpace          ( RTL_CONSTASCII_USTRINGPARAM ( " " ) );

        AttributeList * pAttrList = new AttributeList;
        pAttrList->AddAttribute ( OUString::createFromAscii( "oor:name" ), sCdataAttribute, rName );
        pAttrList->AddAttribute ( OUString::createFromAscii( "oor:type" ), sCdataAttribute, OUString::createFromAscii( "xs:string" ) );
        Reference < XAttributeList > xAttrList (pAttrList);

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sProp, xAttrList );
        xAttrList = pAttrList = new AttributeList;
        pAttrList->AddAttribute ( OUString::createFromAscii( "xml:lang" ), sCdataAttribute, OUString::createFromAscii( "en-US" ) );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sValue, xAttrList );
        xHandler->characters( rValue );
        xHandler->endElement( sValue );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sProp );
    }
    catch( Exception& )
    {
        DBG_ERROR( "TypeDetectionExporter::addLocaleProperty exception catched!" );
    }
}

