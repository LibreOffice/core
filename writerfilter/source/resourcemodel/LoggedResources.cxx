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

#include <rtl/ustrbuf.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <resourcemodel/QNameToString.hxx>

namespace writerfilter
{

// class: LoggedStream

LoggedStream::LoggedStream(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mpLogger(pLogger), msPrefix(sPrefix)
{
}

LoggedStream::~LoggedStream()
{
}

void LoggedStream::startSectionGroup()
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".section");
#endif

    lcl_startSectionGroup();
}

void LoggedStream::endSectionGroup()
{
    lcl_endSectionGroup();

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".section");
#endif
}

void LoggedStream::startParagraphGroup()
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".paragraph");
#endif

    lcl_startParagraphGroup();
}

void LoggedStream::endParagraphGroup()
{
    lcl_endParagraphGroup();

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".paragraph");
#endif
}


void LoggedStream::startCharacterGroup()
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".charactergroup");
#endif

    lcl_startCharacterGroup();
}

void LoggedStream::endCharacterGroup()
{
    lcl_endCharacterGroup();

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".charactergroup");
#endif
}

void LoggedStream::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape )
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".shape");
#endif

    lcl_startShape(xShape);
}

void LoggedStream::endShape()
{
    lcl_endShape();

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".shape");
#endif
}

void LoggedStream::text(const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".text");

    ::rtl::OUString sText( (const sal_Char*) data, len, RTL_TEXTENCODING_MS_1252 );

    mpLogger->startElement(msPrefix + ".data");
    mpLogger->chars(sText);
    mpLogger->endElement(msPrefix + ".data");
#endif

    lcl_text(data, len);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".text");
#endif
}

void LoggedStream::utext(const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".utext");
    mpLogger->startElement(msPrefix + ".data");

    ::rtl::OUString sText;
    ::rtl::OUStringBuffer aBuffer = ::rtl::OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data, len);
    sText = aBuffer.makeStringAndClear();

    mpLogger->chars(sText);

    mpLogger->endElement(msPrefix + ".data");
#endif

    lcl_utext(data, len);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".utext");
#endif
}

void LoggedStream::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".props");
#endif

    lcl_props(ref);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".props");
#endif
}

void LoggedStream::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".table");
    mpLogger->attribute("name", (*QNameToString::Instance())(name));
#endif

    lcl_table(name, ref);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".table");
#endif
}

void LoggedStream::substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".substream");
    mpLogger->attribute("name", name);
#endif

    lcl_substream(name, ref);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".substream");
#endif
}

void LoggedStream::info(const string & info)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".info");
    mpLogger->attribute("text", info);
#endif

    lcl_info(info);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".info");
#endif
}

// class LoggedProperties
LoggedProperties::LoggedProperties(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mpLogger(pLogger), msPrefix(sPrefix)
{
}

LoggedProperties::~LoggedProperties()
{
}

void LoggedProperties::attribute(Id name, Value & val)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".attribute");
    mpLogger->attribute("name", (*QNameToString::Instance())(name));
    mpLogger->attribute("value", val.toString());
    mpLogger->endElement(msPrefix + ".attribute");
#endif

    lcl_attribute(name, val);
}

void LoggedProperties::sprm(Sprm & sprm)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".sprm");
    mpLogger->chars(sprm.toString());
#endif

    lcl_sprm(sprm);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".sprm");
#endif
}

LoggedTable::LoggedTable(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mpLogger(pLogger), msPrefix(sPrefix)
{
}

LoggedTable::~LoggedTable()
{
}

void LoggedTable::entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mpLogger->startElement(msPrefix + ".entry");
    mpLogger->attribute("pos", pos);
#endif

    lcl_entry(pos, ref);

#ifdef DEBUG_LOGGING
    mpLogger->endElement(msPrefix + ".entry");
#endif
}

}
