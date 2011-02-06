/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#if OSL_DEBUG_LEVEL > 1
#include <libxml/xmlstring.h>
#include <string.h>
#include <resourcemodel/TagLogger.hxx>
#include <resourcemodel/util.hxx>
#include <resourcemodel/QNameToString.hxx>

namespace writerfilter
{
    struct eqstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) == 0;
        }
    };

    typedef boost::unordered_map<const char *, TagLogger::Pointer_t, hash<const char *>, eqstr> TagLoggerHashMap_t;
    static TagLoggerHashMap_t * tagLoggers = NULL;

    TagLogger::TagLogger(const char* name)
    : pWriter( NULL ), pName( name )
    {
    }

    TagLogger::~TagLogger()
    {
        pWriter = NULL;
        pName = NULL;
    }

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

    TagLogger::Pointer_t TagLogger::getInstance(const char * name)
    {
        if (tagLoggers == NULL)
            tagLoggers = new TagLoggerHashMap_t();

        TagLoggerHashMap_t::iterator aIt = tagLoggers->end();

        if (! tagLoggers->empty())
            aIt = tagLoggers->find(name);

        if (aIt == tagLoggers->end())
        {
            TagLogger::Pointer_t pTagLogger(new TagLogger(name));
            pair<const char *, TagLogger::Pointer_t> entry(name, pTagLogger);
            aIt = tagLoggers->insert(entry).first;
        }

        return aIt->second;
    }

    void TagLogger::startDocument()
    {
        xmlTextWriterStartDocument( pWriter, NULL, NULL, NULL );
        xmlTextWriterStartElement( pWriter, BAD_CAST( "root" ) );
    }

    void TagLogger::element(const string & name)
    {
        startElement(name);
        endElement();
    }

    void TagLogger::startElement(const string & name)
    {
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlTextWriterStartElement( pWriter, xmlName );
        xmlFree( xmlName );
    }

    void TagLogger::attribute(const string & name, const string & value)
    {
        xmlChar* xmlName = xmlCharStrdup( name.c_str() );
        xmlChar* xmlValue = xmlCharStrdup( value.c_str() );
        xmlTextWriterWriteAttribute( pWriter, xmlName, xmlValue );

        xmlFree( xmlValue );
        xmlFree( xmlName );
    }

    void TagLogger::attribute(const string & name, const ::rtl::OUString & value)
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
        ::rtl::OUString aStr;

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

    void TagLogger::chars(const ::rtl::OUString & rChars)
    {
        chars(OUStringToOString(rChars, RTL_TEXTENCODING_ASCII_US).getStr());
    }

    void TagLogger::endElement()
    {
        xmlTextWriterEndElement( pWriter );
    }

    void TagLogger::endDocument()
    {
        xmlTextWriterEndDocument( pWriter );
        xmlFreeTextWriter( pWriter );
        pWriter = NULL;
    }


    class PropertySetDumpHandler : public Properties
    {
        IdToString::Pointer_t mpIdToString;
        TagLogger::Pointer_t m_pLogger;

    public:
        PropertySetDumpHandler(TagLogger::Pointer_t pLogger,
                IdToString::Pointer_t pIdToString);
        virtual ~PropertySetDumpHandler();

        void resolve(writerfilter::Reference<Properties>::Pointer_t props);

        virtual void attribute(Id name, Value & val);
        virtual void sprm(Sprm & sprm);
    };

    PropertySetDumpHandler::PropertySetDumpHandler(TagLogger::Pointer_t pLogger,
            IdToString::Pointer_t pIdToString) :
        mpIdToString(pIdToString)
    {
        m_pLogger = pLogger;
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

        TagLogger::Pointer_t pLogger( this );
        PropertySetDumpHandler handler( pLogger, pIdToString );
        handler.resolve( props );

        endElement( );
    }

    void TagLogger::unoPropertySet(uno::Reference<beans::XPropertySet> rPropSet)
    {
        uno::Reference<beans::XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());
        uno::Sequence<beans::Property> aProps(xPropSetInfo->getProperties());

        startElement( "unoPropertySet" );

        for (int i = 0; i < aProps.getLength(); ++i)
        {
            startElement( "property" );
            ::rtl::OUString sName(aProps[i].Name);

            attribute( "name", sName );
            try
            {
                attribute( "value", rPropSet->getPropertyValue( sName ) );
            }
            catch (uno::Exception aException)
            {
                startElement( "exception" );

                chars("getPropertyValue(\"");
                chars(sName);
                chars("\")");

                endElement( );
            }

            endElement( );
        }

        endElement( );
    }

}
#endif // DEBUG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
