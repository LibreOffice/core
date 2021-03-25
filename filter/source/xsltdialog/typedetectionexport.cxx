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

#include <string_view>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>

#include "typedetectionexport.hxx"
#include "xmlfiltercommon.hxx"

#include <comphelper/attributelist.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;


TypeDetectionExporter::TypeDetectionExporter( Reference< XComponentContext > const & xContext )
: mxContext( xContext )
{
}

static OUString createRelativeURL( std::u16string_view rFilterName, const OUString& rURL )
{
    if( !rURL.isEmpty() &&
        !rURL.startsWith("http:") &&
        !rURL.startsWith("https:") &&
        !rURL.startsWith("jar:") &&
        !rURL.startsWith("ftp:") )
    {
        INetURLObject aURL( rURL );
        OUString aName(aURL.GetLastName());
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

        return OUString( OUString::Concat("vnd.sun.star.Package:") + rFilterName + "/" + aName );
    }
    else
    {
        return rURL;
    }
}

void TypeDetectionExporter::doExport( const Reference< XOutputStream >& xOS,  const std::vector<filter_info_impl*>& rFilters )
{
    try
    {
        static const OUStringLiteral sComponentData       ( u"oor:component-data" );
        static const OUStringLiteral sNode                ( u"node" );
        static const OUStringLiteral sName                ( u"oor:name" );
        static const OUStringLiteral sWhiteSpace          ( u" " );
        static const OUStringLiteral sUIName              ( u"UIName" );
        static const OUStringLiteral sComma               ( u"," );
        static const OUStringLiteral sDelim               ( u";" );
        static const OUStringLiteral sData                ( u"Data" );
        static const OUStringLiteral sDocTypePrefix       ( u"doctype:" );
        static const OUStringLiteral sFilterAdaptorService( u"com.sun.star.comp.Writer.XmlFilterAdaptor" );
        static const OUStringLiteral sXSLTFilterService   ( u"com.sun.star.documentconversion.XSLTFilter" );
        static const OUStringLiteral sCdataAttribute      ( u"CDATA" );


        // set up sax writer and connect to given output stream
        Reference< XWriter > xHandler = Writer::create( mxContext );
        xHandler->setOutputStream( xOS );

        rtl::Reference<::comphelper::AttributeList> pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( "xmlns:oor", sCdataAttribute, "http://openoffice.org/2001/registry" );
        pAttrList->AddAttribute ( "xmlns:xs", sCdataAttribute, "http://www.w3.org/2001/XMLSchema" );
        pAttrList->AddAttribute ( sName, sCdataAttribute, "TypeDetection" );
        pAttrList->AddAttribute ( "oor:package", sCdataAttribute, "org.openoffice.Office" );

        xHandler->startDocument();
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sComponentData, pAttrList );

        // export types
        {
            pAttrList = new ::comphelper::AttributeList;
            pAttrList->AddAttribute ( sName, sCdataAttribute, "Types" );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, pAttrList );

            for (auto const& filter : rFilters)
            {
                pAttrList = new ::comphelper::AttributeList;
                pAttrList->AddAttribute( sName, sCdataAttribute, filter->maType );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, pAttrList );
                OUString sValue = "0" + sComma + sComma;
                if( !filter->maDocType.isEmpty() )
                {
                    sValue += sDocTypePrefix + filter->maDocType;
                }
                sValue += sComma + sComma + filter->maExtension + sComma +
                    OUString::number( filter->mnDocumentIconID ) + sComma;

                addProperty( xHandler, sData, sValue );
                addLocaleProperty( xHandler, sUIName, filter->maInterfaceName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sNode );
            }

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sNode );
        }

        // export filters
        {
            pAttrList = new ::comphelper::AttributeList;
            pAttrList->AddAttribute ( sName, sCdataAttribute, "Filters" );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, pAttrList );

            for (auto const& filter : rFilters)
            {
                pAttrList = new ::comphelper::AttributeList;
                pAttrList->AddAttribute( sName, sCdataAttribute, filter->maFilterName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, pAttrList );
                addLocaleProperty( xHandler, sUIName, filter->maInterfaceName );

                OUStringBuffer sValue("0" +
                    sComma +
                    filter->maType +
                    sComma +
                    filter->maDocumentService +
                    sComma +
                    sFilterAdaptorService +
                    sComma +
                    OUString::number( filter->maFlags ) +
                    sComma +
                    sXSLTFilterService +
                    sDelim +
                    OUString::boolean( filter->mbNeedsXSLT2 ) +
                    sDelim);

                const application_info_impl* pAppInfo = getApplicationInfo( filter->maExportService );
                sValue.append(pAppInfo->maXMLImporter +
                    sDelim +
                    pAppInfo->maXMLExporter +
                    sDelim);

                sValue.append(createRelativeURL( filter->maFilterName, filter->maImportXSLT ));
                sValue.append(sDelim);
                sValue.append(createRelativeURL( filter->maFilterName, filter->maExportXSLT ));
                sValue.append(sDelim);
                // entry DTD obsolete and removed, but delimiter kept
                sValue.append(sDelim);
                sValue.append(filter->maComment);
                sValue.append(sComma);
                sValue.append("0");
                sValue.append(sComma);
                sValue.append(createRelativeURL( filter->maFilterName, filter->maImportTemplate ));
                addProperty( xHandler, sData, sValue.makeStringAndClear() );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sNode );
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
        TOOLS_WARN_EXCEPTION("filter.xslt", "");
    }
}

void TypeDetectionExporter::addProperty( const Reference< XWriter >& xHandler, const OUString& rName, const OUString& rValue )
{
    try
    {
        static const OUStringLiteral sCdataAttribute( u"CDATA" );
        static const OUStringLiteral sProp( u"prop" );
        static const OUStringLiteral sValue( u"value" );
        static const OUStringLiteral sWhiteSpace          ( u" " );

        rtl::Reference<::comphelper::AttributeList>pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( "oor:name", sCdataAttribute, rName );
        pAttrList->AddAttribute ( "oor:type", sCdataAttribute, "xs:string" );

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sProp, pAttrList );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sValue, pAttrList );
        xHandler->characters( rValue );
        xHandler->endElement( sValue );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sProp );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("filter.xslt", "");
    }
}

void TypeDetectionExporter::addLocaleProperty( const Reference< XWriter >& xHandler, const OUString& rName, const OUString& rValue )
{
    try
    {
        static const OUStringLiteral sCdataAttribute( u"CDATA" );
        static const OUStringLiteral sProp( u"prop" );
        static const OUStringLiteral sValue( u"value" );
        static const OUStringLiteral sWhiteSpace          ( u" " );

        rtl::Reference<::comphelper::AttributeList> pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( "oor:name", sCdataAttribute, rName );
        pAttrList->AddAttribute ( "oor:type", sCdataAttribute, "xs:string" );

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sProp, pAttrList );
        pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( "xml:lang", sCdataAttribute, "en-US" );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sValue, pAttrList );
        xHandler->characters( rValue );
        xHandler->endElement( sValue );
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sProp );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("filter.xslt", "");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
