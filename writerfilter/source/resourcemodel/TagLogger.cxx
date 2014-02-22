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

#include <libxml/xmlstring.h>
#include <string.h>
#include <resourcemodel/TagLogger.hxx>
#include <resourcemodel/util.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <boost/unordered_map.hpp>

namespace writerfilter
{
    TagLogger::TagLogger(const char* name)
        : pWriter( NULL ), pName( name )
    {
    }

    TagLogger::~TagLogger()
    {
        pWriter = NULL;
        pName = NULL;
    }

#ifdef DEBUG_IMPORT
    void TagLogger::setFileName( const string & filename )
    {
        if ( pWriter )
            endDocument();

        string fileName;
        char * temp = getenv("TAGLOGGERTMP");

        if (temp != NULL)
            fileName += temp;
        else
            fileName += "/tmp";

        string sPrefix = filename;
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
        xmlTextWriterSetIndent( pWriter, 4 );
    }

    void TagLogger::startDocument()
    {
        xmlTextWriterStartDocument( pWriter, NULL, NULL, NULL );
        xmlTextWriterStartElement( pWriter, BAD_CAST( "root" ) );
    }

    void TagLogger::endDocument()
    {
        xmlTextWriterEndDocument( pWriter );
        xmlFreeTextWriter( pWriter );
        pWriter = NULL;
    }

#endif

    TagLogger::Pointer_t TagLogger::getInstance(const char * name)
    {
        typedef boost::unordered_map<string, TagLogger::Pointer_t> TagLoggerHashMap_t;
        static TagLoggerHashMap_t tagLoggers;

        TagLoggerHashMap_t::iterator aIt = tagLoggers.end();

        string sName = name;
        if (! tagLoggers.empty())
            aIt = tagLoggers.find(sName);

        if (aIt == tagLoggers.end())
        {
            TagLogger::Pointer_t pTagLogger(new TagLogger(name));
            pair<string, TagLogger::Pointer_t> entry(sName, pTagLogger);
            aIt = tagLoggers.insert(entry).first;
        }

        return aIt->second;
    }

#ifdef DEBUG_DOMAINMAPPER
    void TagLogger::element(const string & name)
    {
        startElement(name);
        endElement();
    }

    void TagLogger::unoPropertySet(uno::Reference<beans::XPropertySet> rPropSet)
    {
        uno::Reference<beans::XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());
        uno::Sequence<beans::Property> aProps(xPropSetInfo->getProperties());

        startElement( "unoPropertySet" );

        for (int i = 0; i < aProps.getLength(); ++i)
        {
            startElement( "property" );
            OUString sName(aProps[i].Name);

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

#endif

#if OSL_DEBUG_LEVEL > 1
    void TagLogger::startElement(const string & name)
    {
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlTextWriterStartElement( pWriter, xmlName );
        xmlFree( xmlName );
    }
#endif

    void TagLogger::attribute(const string & name, const string & value)
    {
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlChar* xmlValue = xmlCharStrdup( value.c_str() );
        xmlTextWriterWriteAttribute( pWriter, xmlName, xmlValue );

        xmlFree( xmlValue );
        xmlFree( xmlName );
    }

#if OSL_DEBUG_LEVEL > 1
    void TagLogger::attribute(const string & name, const OUString & value)
    {
        attribute( name, OUStringToOString( value, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }

    void TagLogger::attribute(const string & name, sal_uInt32 value)
    {
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlTextWriterWriteFormatAttribute( pWriter, xmlName,
               "%" SAL_PRIuUINT32, value );
        xmlFree( xmlName );
    }

    void TagLogger::attribute(const string & name, const uno::Any aAny)
    {
        string aTmpStrInt;
        string aTmpStrFloat;
        string aTmpStrString;

        sal_Int32 nInt = 0;
        float nFloat = 0.0;
        OUString aStr;

        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        if ( aAny >>= nInt )
        {
            xmlTextWriterWriteFormatAttribute( pWriter, xmlName,
                   "%" SAL_PRIdINT32, nInt );
        }
        else if ( aAny >>= nFloat )
        {
            xmlTextWriterWriteFormatAttribute( pWriter, xmlName,
                   "%f", nFloat );
        }
        else if ( aAny >>= aStr )
        {
            attribute( name, aStr );
        }
        xmlFree( xmlName );
    }

    void TagLogger::chars(const string & rChars)
    {
        xmlChar* xmlChars = xmlCharStrdup( rChars.c_str() );
        xmlTextWriterWriteString( pWriter, xmlChars );
        xmlFree( xmlChars );
    }

    void TagLogger::chars(const OUString & rChars)
    {
        chars(OUStringToOString(rChars, RTL_TEXTENCODING_ASCII_US).getStr());
    }

    void TagLogger::endElement()
    {
        xmlTextWriterEndElement( pWriter );
    }
#endif

#ifdef DEBUG_CONTEXT_HANDLER
    class PropertySetDumpHandler : public Properties
    {
        IdToString::Pointer_t mpIdToString;
        TagLogger* m_pLogger;

    public:
        PropertySetDumpHandler(TagLogger* pLogger,
                IdToString::Pointer_t pIdToString);
        virtual ~PropertySetDumpHandler();

        void resolve(writerfilter::Reference<Properties>::Pointer_t props);

        virtual void attribute(Id name, Value & val);
        virtual void sprm(Sprm & sprm);
    };

    PropertySetDumpHandler::PropertySetDumpHandler(TagLogger* pLogger,
            IdToString::Pointer_t pIdToString) :
        mpIdToString(pIdToString),
        m_pLogger(pLogger)
    {
    }

    PropertySetDumpHandler::~PropertySetDumpHandler()
    {
    }

    void PropertySetDumpHandler::resolve(
            writerfilter::Reference<Properties>::Pointer_t pProps)
    {
        if (pProps.get() != NULL)
            pProps->resolve( *this );
    }

    void PropertySetDumpHandler::attribute(Id name, Value & val)
    {
        m_pLogger->startElement( "attribute" );

        m_pLogger->attribute("name", (*QNameToString::Instance())(name));
        m_pLogger->attribute("value", val.toString());

        resolve(val.getProperties());

        m_pLogger->endElement();
    }

    void PropertySetDumpHandler::sprm(Sprm & rSprm)
    {
        m_pLogger->startElement( "sprm" );

        string sName;

        if (mpIdToString != IdToString::Pointer_t())
            sName = mpIdToString->toString(rSprm.getId());

        m_pLogger->attribute( "name", sName );

        m_pLogger->attribute( "id", rSprm.getId() );
        m_pLogger->attribute( "value", rSprm.getValue()->toString() );

        resolve( rSprm.getProps() );

        m_pLogger->endElement();
    }

    void TagLogger::propertySet(writerfilter::Reference<Properties>::Pointer_t props,
            IdToString::Pointer_t pIdToString)
    {
        startElement( "propertySet" );

        PropertySetDumpHandler handler( this, pIdToString );
        handler.resolve( props );

        endElement( );
    }
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
