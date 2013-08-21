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


#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <tools/urlobj.hxx>

#include "typedetectionexport.hxx"
#include "xmlfiltersettingsdialog.hxx"

#include <comphelper/attributelist.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;


TypeDetectionExporter::TypeDetectionExporter( Reference< XComponentContext >& xContext )
: mxContext( xContext )
{
}

static OUString createRelativeURL( const OUString& rFilterName, const OUString& rURL )
{
    if( !rURL.isEmpty() &&
        !rURL.startsWith("http:") &&
        !rURL.startsWith("https:") &&
        !rURL.startsWith("jar:") &&
        !rURL.startsWith("ftp:") )
    {
        INetURLObject aURL( rURL );
        OUString aName( aURL.GetName() );
        if( aName.isEmpty() )
        {
            sal_Int32 nPos = rURL.lastIndexOf( '/' );
            if( nPos == -1 )
            {
                aName = rURL;
            }
            else
            {
                aName = rURL.copy( nPos + 1 );
            }
        }

        return OUString( "vnd.sun.star.Package:" + rFilterName + "/" + aName );
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
        const OUString sComponentData       ( "oor:component-data" );
        const OUString sNode                ( "node" );
        const OUString sName                ( "oor:name" );
        const OUString sWhiteSpace          ( " " );
        const OUString sUIName              ( "UIName" );
        const OUString sComma               ( "," );
        const OUString sDelim               ( ";" );
        const OUString sData                ( "Data" );
        const OUString sDocTypePrefix       ( "doctype:" );
        const OUString sFilterAdaptorService( "com.sun.star.comp.Writer.XmlFilterAdaptor" );
        const OUString sXSLTFilterService   ( "com.sun.star.documentconversion.XSLTFilter" );
        const OUString sCdataAttribute      ( "CDATA" );


        // set up sax writer and connect to given output stream
        Reference< XWriter > xHandler = Writer::create( mxContext );
        xHandler->setOutputStream( xOS );

        ::comphelper::AttributeList * pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( OUString( "xmlns:oor" ), sCdataAttribute, OUString( "http://openoffice.org/2001/registry" ) );
        pAttrList->AddAttribute ( OUString( "xmlns:xs" ), sCdataAttribute, OUString( "http://www.w3.org/2001/XMLSchema" ) );
        pAttrList->AddAttribute ( sName, sCdataAttribute, OUString( "TypeDetection" ) );
        pAttrList->AddAttribute ( OUString( "oor:package" ), sCdataAttribute, OUString( "org.openoffice.Office" ) );
        Reference < XAttributeList > xAttrList (pAttrList);

        xHandler->startDocument();
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sComponentData, xAttrList );

        // export types
        {
            xAttrList = pAttrList = new ::comphelper::AttributeList;
            pAttrList->AddAttribute ( sName, sCdataAttribute, OUString( "Types" ) );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, xAttrList );

            XMLFilterVector::const_iterator aIter( rFilters.begin() );
            while( aIter != rFilters.end() )
            {
                const filter_info_impl* pFilter = (*aIter);

                xAttrList = pAttrList = new ::comphelper::AttributeList;
                pAttrList->AddAttribute( sName, sCdataAttribute, pFilter->maType );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, xAttrList );
                OUString sValue("0");
                sValue += sComma;
                sValue += sComma;
                if( !pFilter->maDocType.isEmpty() )
                {
                    sValue += sDocTypePrefix;
                    sValue += pFilter->maDocType;
                }
                sValue += sComma;
                sValue += sComma;
                sValue += pFilter->maExtension;
                sValue += sComma;
                sValue += OUString::number( pFilter->mnDocumentIconID );
                sValue += sComma;

                addProperty( xHandler, sData, sValue );
                addLocaleProperty( xHandler, sUIName, pFilter->maInterfaceName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sNode );

                ++aIter;
            }

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sNode );
        }

        // export filters
        {
            xAttrList = pAttrList = new ::comphelper::AttributeList;
            pAttrList->AddAttribute ( sName, sCdataAttribute, OUString( "Filters" ) );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, xAttrList );

            XMLFilterVector::const_iterator aIter( rFilters.begin() );
            while( aIter != rFilters.end() )
            {
                const filter_info_impl* pFilter = (*aIter);

                xAttrList = pAttrList = new ::comphelper::AttributeList;
                pAttrList->AddAttribute( sName, sCdataAttribute, pFilter->maFilterName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, xAttrList );
                addLocaleProperty( xHandler, sUIName, pFilter->maInterfaceName );

                OUString sValue("0");
                sValue += sComma;
                sValue += pFilter->maType;
                sValue += sComma,
                sValue += pFilter->maDocumentService;
                sValue += sComma,
                sValue += sFilterAdaptorService;
                sValue += sComma,
                sValue += OUString::number( pFilter->maFlags );
                sValue += sComma;
                sValue += sXSLTFilterService;
                sValue += sDelim;
                sValue += OUString::boolean( pFilter->mbNeedsXSLT2 );
                sValue += sDelim;

                const application_info_impl* pAppInfo = getApplicationInfo( pFilter->maExportService );
                sValue += pAppInfo->maXMLImporter;
                sValue += sDelim;
                sValue += pAppInfo->maXMLExporter;
                sValue += sDelim;

                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maImportXSLT );
                sValue += sDelim;
                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maExportXSLT );
                sValue += sDelim;
                // entry DTD obsolete and removed, but delimiter kept
                sValue += sDelim;
                sValue += pFilter->maComment;
                sValue += sComma;
                sValue += "0";
                sValue += sComma;
                sValue += createRelativeURL( pFilter->maFilterName, pFilter->maImportTemplate );
                addProperty( xHandler, sData, sValue );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sNode );
                ++aIter;
            }

            xHandler->endElement( sNode );
        }

        // finish
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sComponentData );
        xHandler->endDocument();
    }
    catch( const Exception& )
    {
        OSL_FAIL( "TypeDetectionExporter::doExport exception catched!" );
    }
}

void TypeDetectionExporter::addProperty( Reference< XWriter > xHandler, const OUString& rName, const OUString& rValue )
{
    try
    {
        const OUString sCdataAttribute( "CDATA" );
        const OUString sProp( "prop" );
        const OUString sValue( "value" );
        const OUString sWhiteSpace          ( " " );

        ::comphelper::AttributeList * pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( OUString( "oor:name" ), sCdataAttribute, rName );
        pAttrList->AddAttribute ( OUString( "oor:type" ), sCdataAttribute, OUString( "xs:string" ) );
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
    catch( const Exception& )
    {
        OSL_FAIL( "TypeDetectionExporter::addProperty exception catched!" );
    }
}

void TypeDetectionExporter::addLocaleProperty( Reference< XWriter > xHandler, const OUString& rName, const OUString& rValue )
{
    try
    {
        const OUString sCdataAttribute( "CDATA" );
        const OUString sProp( "prop" );
        const OUString sValue( "value" );
        const OUString sWhiteSpace          ( " " );

        ::comphelper::AttributeList * pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( OUString( "oor:name" ), sCdataAttribute, rName );
        pAttrList->AddAttribute ( OUString( "oor:type" ), sCdataAttribute, OUString( "xs:string" ) );
        Reference < XAttributeList > xAttrList (pAttrList);

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sProp, xAttrList );
        xAttrList = pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( OUString( "xml:lang" ), sCdataAttribute, OUString( "en-US" ) );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sValue, xAttrList );
        xHandler->characters( rValue );
        xHandler->endElement( sValue );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sProp );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "TypeDetectionExporter::addLocaleProperty exception catched!" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
