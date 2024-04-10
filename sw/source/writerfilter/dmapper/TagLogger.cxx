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
        : m_pWriter( nullptr ), m_pName( "DOMAINMAPPER" )
    {
    }

    TagLogger::~TagLogger()
    {
        m_pWriter = nullptr;
        m_pName = nullptr;
    }

#ifdef DBG_UTIL
    void TagLogger::setFileName( const std::string & filename )
    {
        if ( m_pWriter )
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
        fileName += m_pName;
        fileName += ".xml";

        m_pWriter = xmlNewTextWriterFilename( fileName.c_str(), 0 );
        xmlTextWriterSetIndent(m_pWriter,1);
        (void)xmlTextWriterSetIndentString(m_pWriter, BAD_CAST("  "));
        xmlTextWriterSetIndent( m_pWriter, 4 );
    }

    void TagLogger::startDocument()
    {
        if (!m_pWriter)
            return;
        (void)xmlTextWriterStartDocument( m_pWriter, nullptr, nullptr, nullptr );
        (void)xmlTextWriterStartElement( m_pWriter, BAD_CAST( "root" ) );
    }

    void TagLogger::endDocument()
    {
        if (!m_pWriter)
            return;
        (void)xmlTextWriterEndDocument( m_pWriter );
        xmlFreeTextWriter( m_pWriter );
        m_pWriter = nullptr;
    }

#endif

    TagLogger& TagLogger::getInstance()
    {
        static TagLogger theTagLogger;
        return theTagLogger;
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
        if (!m_pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        (void)xmlTextWriterStartElement( m_pWriter, xmlName );
        xmlFree( xmlName );
    }
#endif

    void TagLogger::attribute(const std::string & name, const std::string & value)
    {
        if (!m_pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlChar* xmlValue = xmlCharStrdup( value.c_str() );
        (void)xmlTextWriterWriteAttribute( m_pWriter, xmlName, xmlValue );

        xmlFree( xmlValue );
        xmlFree( xmlName );
    }

#ifdef DBG_UTIL
    void TagLogger::attribute(const std::string & name, std::u16string_view value)
    {
        attribute( name, std::string(OUStringToOString( value, RTL_TEXTENCODING_ASCII_US )) );
    }

    void TagLogger::attribute(const std::string & name, sal_uInt32 value)
    {
        if (!m_pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        (void)xmlTextWriterWriteFormatAttribute( m_pWriter, xmlName,
               "%" SAL_PRIuUINT32, value );
        xmlFree( xmlName );
    }

    void TagLogger::attribute(const std::string & name, float value)
    {
        if (!m_pWriter)
            return;
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        (void)xmlTextWriterWriteFormatAttribute( m_pWriter, xmlName,
               "%f", value );
        xmlFree( xmlName );
    }

    void TagLogger::attribute(const std::string & name, const uno::Any& aAny)
    {
        if (!m_pWriter)
            return;

        sal_Int32 nInt = 0;
        float nFloat = 0.0;
        OUString aStr;

        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        if ( aAny >>= nInt )
        {
            (void)xmlTextWriterWriteFormatAttribute( m_pWriter, xmlName,
                   "%" SAL_PRIdINT32, nInt );
        }
        else if ( aAny >>= nFloat )
        {
            (void)xmlTextWriterWriteFormatAttribute( m_pWriter, xmlName,
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
        if (!m_pWriter)
            return;
        xmlChar* xmlChars = xmlCharStrdup( rChars.c_str() );
        (void)xmlTextWriterWriteString( m_pWriter, xmlChars );
        xmlFree( xmlChars );
    }

    void TagLogger::chars(std::u16string_view rChars)
    {
        chars(std::string(OUStringToOString(rChars, RTL_TEXTENCODING_ASCII_US)));
    }

    void TagLogger::endElement()
    {
        if (!m_pWriter)
            return;
        (void)xmlTextWriterEndElement( m_pWriter );
    }

#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
