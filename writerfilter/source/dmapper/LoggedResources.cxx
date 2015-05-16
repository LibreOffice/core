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

#include <rtl/ustrbuf.hxx>
#include "LoggedResources.hxx"
#include <ooxml/QNameToString.hxx>

using namespace ::com::sun::star;

namespace writerfilter
{

#ifdef DEBUG_WRITERFILTER
// class: LoggedResourcesHelper

LoggedResourcesHelper::LoggedResourcesHelper(const std::string & sPrefix)
: msPrefix(sPrefix)
{
}

LoggedResourcesHelper::~LoggedResourcesHelper()
{
}

void LoggedResourcesHelper::startElement(const std::string & sElement)
{
    TagLogger::getInstance().startElement(msPrefix + "." + sElement);
}

void LoggedResourcesHelper::endElement(const std::string & /*sElement*/)
{
    TagLogger::getInstance().endElement();
}

void LoggedResourcesHelper::chars(const OUString & rChars)
{
    TagLogger::getInstance().chars(rChars);
}

void LoggedResourcesHelper::chars(const std::string & rChars)
{
    TagLogger::getInstance().chars(rChars);
}

void LoggedResourcesHelper::attribute(const std::string & rName, const std::string & rValue)
{
    TagLogger::getInstance().attribute(rName, rValue);
}

void LoggedResourcesHelper::attribute(const std::string & rName, sal_uInt32 nValue)
{
    TagLogger::getInstance().attribute(rName, nValue);
}

#endif

// class: LoggedStream

LoggedStream::LoggedStream(
#ifdef DEBUG_WRITERFILTER
    const std::string & sPrefix
) : mHelper(sPrefix)
#else
    const std::string&
)
#endif
{
}

LoggedStream::~LoggedStream()
{
}

void LoggedStream::startSectionGroup()
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("section");
#endif

    lcl_startSectionGroup();
}

void LoggedStream::endSectionGroup()
{
    lcl_endSectionGroup();

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("section");
#endif
}

void LoggedStream::startParagraphGroup()
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("paragraph");
#endif

    lcl_startParagraphGroup();
}

void LoggedStream::endParagraphGroup()
{
    lcl_endParagraphGroup();

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("paragraph");
#endif
}


void LoggedStream::startCharacterGroup()
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("charactergroup");
#endif

    lcl_startCharacterGroup();
}

void LoggedStream::endCharacterGroup()
{
    lcl_endCharacterGroup();

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("charactergroup");
#endif
}

void LoggedStream::startShape(uno::Reference<drawing::XShape> const& xShape)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("shape");
#endif

    lcl_startShape(xShape);
}

void LoggedStream::endShape()
{
    lcl_endShape();

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("shape");
#endif
}

void LoggedStream::text(const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("text");

    OUString sText( reinterpret_cast<const char*>(data), len, RTL_TEXTENCODING_MS_1252 );

    mHelper.startElement("data");
    mHelper.chars(sText);
    mHelper.endElement("data");
#endif

    lcl_text(data, len);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("text");
#endif
}

void LoggedStream::utext(const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("utext");
    mHelper.startElement("data");

    OUString sText;
    OUStringBuffer aBuffer = OUStringBuffer(sal::static_int_cast<int>(len));
    aBuffer.append( reinterpret_cast<const sal_Unicode *>(data), len);
    sText = aBuffer.makeStringAndClear();

    mHelper.chars(sText);

    mHelper.endElement("data");
#endif

    lcl_utext(data, len);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("utext");
#endif
}

void LoggedStream::positionOffset(const OUString& rText, bool bVertical)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("positionOffset");
    mHelper.attribute("vertical", static_cast<int>(bVertical));
    mHelper.chars(rText);
#endif

    lcl_positionOffset(rText, bVertical);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("positionOffset");
#endif
}

void LoggedStream::align(const OUString& rText, bool bVertical)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("align");
    mHelper.attribute("vertical", static_cast<int>(bVertical));
    mHelper.chars(rText);
#endif

    lcl_align(rText, bVertical);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("align");
#endif
}

void LoggedStream::positivePercentage(const OUString& rText)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("positivePercentage");
    mHelper.chars(rText);
#endif

    lcl_positivePercentage(rText);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("positivePercentage");
#endif
}

void LoggedStream::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("props");
#endif

    lcl_props(ref);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("props");
#endif
}

void LoggedStream::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("table");
    mHelper.attribute("name", (*QNameToString::Instance())(name));
#endif

    lcl_table(name, ref);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("table");
#endif
}

void LoggedStream::substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("substream");
    mHelper.attribute("name", (*QNameToString::Instance())(name));
#endif

    lcl_substream(name, ref);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("substream");
#endif
}

void LoggedStream::info(const std::string & _info)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("info");
    mHelper.attribute("text", _info);
#endif

    lcl_info(_info);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("info");
#endif
}

// class LoggedProperties
LoggedProperties::LoggedProperties(
#ifdef DEBUG_WRITERFILTER
    const std::string & sPrefix
) : mHelper(sPrefix)
#else
    const std::string&
)
#endif
{
}

LoggedProperties::~LoggedProperties()
{
}

void LoggedProperties::attribute(Id name, Value & val)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("attribute");
    mHelper.attribute("name", (*QNameToString::Instance())(name));
    mHelper.attribute("value", val.toString());
    mHelper.endElement("attribute");
#endif

    lcl_attribute(name, val);
}

void LoggedProperties::sprm(Sprm & rSprm)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("sprm");
    mHelper.attribute("name", (*QNameToString::Instance())(rSprm.getId()));
    mHelper.chars(rSprm.toString());
#endif

    lcl_sprm(rSprm);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("sprm");
#endif
}

LoggedTable::LoggedTable(
#ifdef DEBUG_WRITERFILTER
    const std::string & sPrefix
) : mHelper(sPrefix)
#else
    const std::string&
)
#endif
{
}

LoggedTable::~LoggedTable()
{
}

void LoggedTable::entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_WRITERFILTER
    mHelper.startElement("entry");
    mHelper.attribute("pos", pos);
#endif

    lcl_entry(pos, ref);

#ifdef DEBUG_WRITERFILTER
    mHelper.endElement("entry");
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
