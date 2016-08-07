/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <rtl/ustrbuf.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <resourcemodel/QNameToString.hxx>

namespace writerfilter
{

// class: LoggedResourcesHelper

LoggedResourcesHelper::LoggedResourcesHelper(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mpLogger(pLogger), msPrefix(sPrefix)
{
}

LoggedResourcesHelper::~LoggedResourcesHelper()
{
}

void LoggedResourcesHelper::startElement(const string & sElement)
{
    mpLogger->startElement(msPrefix + "." + sElement);
}

void LoggedResourcesHelper::endElement(const string & sElement)
{
    mpLogger->endElement(msPrefix + "." + sElement);
}

void LoggedResourcesHelper::chars(const ::rtl::OUString & rChars)
{
    mpLogger->chars(rChars);
}

void LoggedResourcesHelper::chars(const string & rChars)
{
    mpLogger->chars(rChars);
}

void LoggedResourcesHelper::attribute(const string & rName, const string & rValue)
{
    mpLogger->attribute(rName, rValue);
}

void LoggedResourcesHelper::attribute(const string & rName, sal_uInt32 nValue)
{
    mpLogger->attribute(rName, nValue);
}

void LoggedResourcesHelper::setPrefix(const string & rPrefix)
{
    msPrefix = rPrefix;
}

// class: LoggedStream

LoggedStream::LoggedStream(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mHelper(pLogger, sPrefix)
{
}

LoggedStream::~LoggedStream()
{
}

void LoggedStream::startSectionGroup()
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("section");
#endif

    lcl_startSectionGroup();
}

void LoggedStream::endSectionGroup()
{
    lcl_endSectionGroup();

#ifdef DEBUG_LOGGING
    mHelper.endElement("section");
#endif
}

void LoggedStream::startParagraphGroup()
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("paragraph");
#endif

    lcl_startParagraphGroup();
}

void LoggedStream::endParagraphGroup()
{
    lcl_endParagraphGroup();

#ifdef DEBUG_LOGGING
    mHelper.endElement("paragraph");
#endif
}


void LoggedStream::startCharacterGroup()
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("charactergroup");
#endif

    lcl_startCharacterGroup();
}

void LoggedStream::endCharacterGroup()
{
    lcl_endCharacterGroup();

#ifdef DEBUG_LOGGING
    mHelper.endElement("charactergroup");
#endif
}

void LoggedStream::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape )
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("shape");
#endif

    lcl_startShape(xShape);
}

void LoggedStream::endShape()
{
    lcl_endShape();

#ifdef DEBUG_LOGGING
    mHelper.endElement("shape");
#endif
}

void LoggedStream::text(const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("text");

    ::rtl::OUString sText( (const sal_Char*) data, len, RTL_TEXTENCODING_MS_1252 );

    mHelper.startElement("data");
    mHelper.chars(sText);
    mHelper.endElement("data");
#endif

    lcl_text(data, len);

#ifdef DEBUG_LOGGING
    mHelper.endElement("text");
#endif
}

void LoggedStream::utext(const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("utext");
    mHelper.startElement("data");

    ::rtl::OUString sText;
    ::rtl::OUStringBuffer aBuffer = ::rtl::OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data, len);
    sText = aBuffer.makeStringAndClear();

    mHelper.chars(sText);

    mHelper.endElement("data");
#endif

    lcl_utext(data, len);

#ifdef DEBUG_LOGGING
    mHelper.endElement("utext");
#endif
}

void LoggedStream::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("props");
#endif

    lcl_props(ref);

#ifdef DEBUG_LOGGING
    mHelper.endElement("props");
#endif
}

void LoggedStream::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("table");
    mHelper.attribute("name", (*QNameToString::Instance())(name));
#endif

    lcl_table(name, ref);

#ifdef DEBUG_LOGGING
    mHelper.endElement("table");
#endif
}

void LoggedStream::substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("substream");
    mHelper.attribute("name", (*QNameToString::Instance())(name));
#endif

    lcl_substream(name, ref);

#ifdef DEBUG_LOGGING
    mHelper.endElement("substream");
#endif
}

void LoggedStream::info(const string & _info)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("info");
    mHelper.attribute("text", _info);
#endif

    lcl_info(_info);

#ifdef DEBUG_LOGGING
    mHelper.endElement("info");
#endif
}

// class LoggedProperties
LoggedProperties::LoggedProperties(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mHelper(pLogger, sPrefix)
{
}

LoggedProperties::~LoggedProperties()
{
}

void LoggedProperties::attribute(Id name, Value & val)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("attribute");
    mHelper.attribute("name", (*QNameToString::Instance())(name));
    mHelper.attribute("value", val.toString());
    mHelper.endElement("attribute");
#endif

    lcl_attribute(name, val);
}

void LoggedProperties::sprm(Sprm & _sprm)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("sprm");
    mHelper.attribute("name", (*QNameToString::Instance())(_sprm.getId()));
    mHelper.chars(_sprm.toString());
#endif

    lcl_sprm(_sprm);

#ifdef DEBUG_LOGGING
    mHelper.endElement("sprm");
#endif
}

LoggedTable::LoggedTable(TagLogger::Pointer_t pLogger, const string & sPrefix)
: mHelper(pLogger, sPrefix)
{
}

LoggedTable::~LoggedTable()
{
}

void LoggedTable::entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_LOGGING
    mHelper.startElement("entry");
    mHelper.attribute("pos", pos);
#endif

    lcl_entry(pos, ref);

#ifdef DEBUG_LOGGING
    mHelper.endElement("entry");
#endif
}

}
