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

#include <string.h>
#include "TagLogger.hxx"
#ifdef DBG_UTIL
#include <unotools/pathoptions.hxx>
#endif

using namespace css;

namespace writerfilter
{
    TagLogger::TagLogger()
        : pWriter( nullptr ), pName( "DOMAINMAPPER" )
    {
    }

    TagLogger::~TagLogger()
    {
        pWriter = nullptr;
        pName = nullptr;
    }

#ifdef DBG_UTIL
    void TagLogger::setFileName( const std::string & filename )
    {
        if ( pWriter )
            endDocument();

        std::string fileName;
        char * temp = getenv("TAGLOGGERTMP");

        if (temp != nullptr)
            fileName += temp;
        else
            fileName += SvtPathOptions().GetTempPath().toUtf8().getStr();

        std::string sPrefix = filename;
        size_t nLastSlash = sPrefix.find_last_of('/');
        size_t nLastBackslash = sPrefix.find_last_of('\\');
        size_t nCutPos = nLastSlash;
        if (nLastBackslash < nCutPos)
            nCutPos = nLastBackslash;
        if (nCutPos < sPrefix.size())
            sPrefix = sPrefix.substr(nCutPos + 1);

        fileName += "/";
        fileName += sPrefix;
        fileName += ".";
        fileName += pName;
        fileName += ".xml";

        pWriter = xmlNewTextWriterFilename( fileName.c_str(), 0 );
        xmlTextWriterSetIndent(pWriter,1);
        xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        xmlTextWriterSetIndent( pWriter, 4 );
    }

    void TagLogger::startDocument()
    {
        if (!pWriter)
            return;
        (void)xmlTextWriterStartDocument( pWriter, nullptr, nullptr, nullptr );
        (void)xmlTextWriterStartElement( pWriter, BAD_CAST( "root" ) );
    }

    void TagLogger::endDocument()
    {
        if (!pWriter)
            return;
        (void)xmlTextWriterEndDocument( pWriter );
        xmlFreeTextWriter( pWriter );
        pWriter = nullptr;
    }

#endif

namespace {

struct TheTagLogger:
    public rtl::Static<TagLogger, TheTagLogger>
{};

}

    TagLogger& TagLogger::getInstance()
    {
        return TheTagLogger::get();
    }

#ifdef DBG_UTIL
    void TagLogger::element(const std::string & name)
    {
        startElement(name);
        endElement();
    }

    void TagLogger::unoPropertySet(const uno::Reference<beans::XPropertySet>& rPropSet)
    {
        uno::Reference<beans::XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());
        const uno::Sequence<beans::Property> aProps(xPropSetInfo->getProperties());

        startElement( "unoPropertySet" );

        for (beans::Property const & prop : aProps)
        {
            startElement( "property" );
            OUString sName(prop.Name);

            attribute( "name", sName );
            try
            {
                attribute( "value", rPropSet->getPropertyValue( sName ) );
            }
            catch (const uno::Exception &)
            {
                startElement( "exception" );

                chars(std::string("getPropertyValue(\""));
                chars(sName);
                chars(std::string("\")"));

                endElement( );
            }
            endElement( );
        }
        endElement( );
    }

    void TagLogger::startElement(const std::string & name)
    {
        if (!pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        (void)xmlTextWriterStartElement( pWriter, xmlName );
        xmlFree( xmlName );
    }
#endif

    void TagLogger::attribute(const std::string & name, const std::string & value)
    {
        if (!pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlChar* xmlValue = xmlCharStrdup( value.c_str() );
        (void)xmlTextWriterWriteAttribute( pWriter, xmlName, xmlValue );

        xmlFree( xmlValue );
        xmlFree( xmlName );
    }

#ifdef DBG_UTIL
    void TagLogger::attribute(const std::string & name, std::u16string_view value)
    {
        attribute( name, OUStringToOString( value, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }

    void TagLogger::attribute(const std::string & name, sal_uInt32 value)
    {
        if (!pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        (void)xmlTextWriterWriteFormatAttribute( pWriter, xmlName,
               "%" SAL_PRIuUINT32, value );
        xmlFree( xmlName );
    }

    void TagLogger::attribute(const std::string & name, const uno::Any& aAny)
    {
        if (!pWriter)
            return;

        sal_Int32 nInt = 0;
        float nFloat = 0.0;
        OUString aStr;

        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        if ( aAny >>= nInt )
        {
            (void)xmlTextWriterWriteFormatAttribute( pWriter, xmlName,
                   "%" SAL_PRIdINT32, nInt );
        }
        else if ( aAny >>= nFloat )
        {
            (void)xmlTextWriterWriteFormatAttribute( pWriter, xmlName,
                   "%f", nFloat );
        }
        else if ( aAny >>= aStr )
        {
            attribute( name, aStr );
        }
        xmlFree( xmlName );
    }

    void TagLogger::chars(const std::string & rChars)
    {
        if (!pWriter)
            return;
        xmlChar* xmlChars = xmlCharStrdup( rChars.c_str() );
        (void)xmlTextWriterWriteString( pWriter, xmlChars );
        xmlFree( xmlChars );
    }

    void TagLogger::chars(std::u16string_view rChars)
    {
        chars(OUStringToOString(rChars, RTL_TEXTENCODING_ASCII_US).getStr());
    }

    void TagLogger::endElement()
    {
        if (!pWriter)
            return;
        (void)xmlTextWriterEndElement( pWriter );
    }

#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
