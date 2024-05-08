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
#include <comphelper/diagnose_ex.hxx>

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
        static constexpr OUString sComponentData       ( u"oor:component-data"_ustr );
        static constexpr OUString sNode                ( u"node"_ustr );
        static constexpr OUString sName                ( u"oor:name"_ustr );
        static constexpr OUString sWhiteSpace          ( u" "_ustr );
        static constexpr OUString sUIName              ( u"UIName"_ustr );
        static constexpr OUString sComma               ( u","_ustr );
        static constexpr OUString sDelim               ( u";"_ustr );
        static constexpr OUString sData                ( u"Data"_ustr );
        static constexpr OUStringLiteral sDocTypePrefix       ( u"doctype:" );
        static constexpr OUStringLiteral sFilterAdaptorService( u"com.sun.star.comp.Writer.XmlFilterAdaptor" );
        static constexpr OUStringLiteral sXSLTFilterService   ( u"com.sun.star.documentconversion.XSLTFilter" );


        // set up sax writer and connect to given output stream
        Reference< XWriter > xHandler = Writer::create( mxContext );
        xHandler->setOutputStream( xOS );

        rtl::Reference<::comphelper::AttributeList> pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( u"xmlns:oor"_ustr, u"http://openoffice.org/2001/registry"_ustr );
        pAttrList->AddAttribute ( u"xmlns:xs"_ustr, u"http://www.w3.org/2001/XMLSchema"_ustr );
        pAttrList->AddAttribute ( sName, u"TypeDetection"_ustr );
        pAttrList->AddAttribute ( u"oor:package"_ustr, u"org.openoffice.Office"_ustr );

        xHandler->startDocument();
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sComponentData, pAttrList );

        // export types
        {
            pAttrList = new ::comphelper::AttributeList;
            pAttrList->AddAttribute ( sName, u"Types"_ustr );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, pAttrList );

            for (auto const& filter : rFilters)
            {
                pAttrList = new ::comphelper::AttributeList;
                pAttrList->AddAttribute( sName, filter->maType );
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
            pAttrList->AddAttribute ( sName, u"Filters"_ustr );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sNode, pAttrList );

            for (auto const& filter : rFilters)
            {
                pAttrList = new ::comphelper::AttributeList;
                pAttrList->AddAttribute( sName, filter->maFilterName );
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sNode, pAttrList );
                addLocaleProperty( xHandler, sUIName, filter->maInterfaceName );

                const application_info_impl* pAppInfo = getApplicationInfo( filter->maExportService );
                OUString sValue =
                    "0" +
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
                    sDelim +
                    pAppInfo->maXMLImporter +
                    sDelim +
                    pAppInfo->maXMLExporter +
                    sDelim +
                    createRelativeURL( filter->maFilterName, filter->maImportXSLT ) +
                    sDelim +
                    createRelativeURL( filter->maFilterName, filter->maExportXSLT ) +
                    sDelim +
                    // entry DTD obsolete and removed, but delimiter kept
                    sDelim  +
                    filter->maComment +
                    sComma +
                    "0" +
                    sComma +
                    createRelativeURL( filter->maFilterName, filter->maImportTemplate );
                addProperty( xHandler, sData, sValue );
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
        static constexpr OUString sProp( u"prop"_ustr );
        static constexpr OUString sValue( u"value"_ustr );
        static constexpr OUString sWhiteSpace          ( u" "_ustr );

        rtl::Reference<::comphelper::AttributeList>pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( u"oor:name"_ustr, rName );
        pAttrList->AddAttribute ( u"oor:type"_ustr, u"xs:string"_ustr );

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
        static constexpr OUString sProp( u"prop"_ustr );
        static constexpr OUString sValue( u"value"_ustr );
        static constexpr OUString sWhiteSpace          ( u" "_ustr );

        rtl::Reference<::comphelper::AttributeList> pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( u"oor:name"_ustr, rName );
        pAttrList->AddAttribute ( u"oor:type"_ustr, u"xs:string"_ustr );

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sProp, pAttrList );
        pAttrList = new ::comphelper::AttributeList;
        pAttrList->AddAttribute ( u"xml:lang"_ustr, u"en-US"_ustr );
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
