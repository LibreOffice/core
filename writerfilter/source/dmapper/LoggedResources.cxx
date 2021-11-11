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

#include "LoggedResources.hxx"
#include "TagLogger.hxx"
#include <ooxml/QNameToString.hxx>

using namespace ::com::sun::star;

namespace writerfilter
{
#ifdef DBG_UTIL

LoggedResourcesHelper::LoggedResourcesHelper(const std::string& sPrefix)
    : msPrefix(sPrefix)
{
}

LoggedResourcesHelper::~LoggedResourcesHelper() {}

void LoggedResourcesHelper::startElement(const std::string& sElement)
{
    TagLogger::getInstance().startElement(msPrefix + "." + sElement);
}

void LoggedResourcesHelper::endElement() { TagLogger::getInstance().endElement(); }

void LoggedResourcesHelper::chars(std::u16string_view rChars)
{
    TagLogger::getInstance().chars(rChars);
}

void LoggedResourcesHelper::chars(const std::string& rChars)
{
    TagLogger::getInstance().chars(rChars);
}

void LoggedResourcesHelper::attribute(const std::string& rName, const std::string& rValue)
{
    TagLogger::getInstance().attribute(rName, rValue);
}

void LoggedResourcesHelper::attribute(const std::string& rName, sal_uInt32 nValue)
{
    TagLogger::getInstance().attribute(rName, nValue);
}

#endif

LoggedStream::LoggedStream(
#ifdef DBG_UTIL
    const std::string& sPrefix)
    : mHelper(sPrefix)
#else
    const std::string&)
#endif
{
}

LoggedStream::~LoggedStream() {}

void LoggedStream::startSectionGroup()
{
#ifdef DBG_UTIL
    mHelper.startElement("section");
#endif

    lcl_startSectionGroup();
}

void LoggedStream::endSectionGroup()
{
    lcl_endSectionGroup();

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::startParagraphGroup()
{
#ifdef DBG_UTIL
    mHelper.startElement("paragraph");
#endif

    lcl_startParagraphGroup();
}

void LoggedStream::endParagraphGroup()
{
    lcl_endParagraphGroup();

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::startCharacterGroup()
{
#ifdef DBG_UTIL
    mHelper.startElement("charactergroup");
#endif

    lcl_startCharacterGroup();
}

void LoggedStream::endCharacterGroup()
{
    lcl_endCharacterGroup();

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::startShape(uno::Reference<drawing::XShape> const& xShape)
{
#ifdef DBG_UTIL
    mHelper.startElement("shape");
#endif

    lcl_startShape(xShape);
}

void LoggedStream::endShape()
{
    lcl_endShape();

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::startTextBoxContent() { lcl_startTextBoxContent(); }

void LoggedStream::endTextBoxContent() { lcl_endTextBoxContent(); }

void LoggedStream::text(const sal_uInt8* data, size_t len)
{
#ifdef DBG_UTIL
    mHelper.startElement("text");

    OUString sText(reinterpret_cast<const char*>(data), len, RTL_TEXTENCODING_MS_1252);

    mHelper.startElement("data");
    LoggedResourcesHelper::chars(sText);
    LoggedResourcesHelper::endElement();
#endif

    lcl_text(data, len);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::utext(const sal_uInt8* data, size_t len)
{
#ifdef DBG_UTIL
    mHelper.startElement("utext");
    mHelper.startElement("data");

    OUString sText(reinterpret_cast<const sal_Unicode*>(data), len);

    LoggedResourcesHelper::chars(sText);

    LoggedResourcesHelper::endElement();
#endif

    lcl_utext(data, len);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::positionOffset(const OUString& rText, bool bVertical)
{
#ifdef DBG_UTIL
    mHelper.startElement("positionOffset");
    LoggedResourcesHelper::attribute("vertical", static_cast<int>(bVertical));
    LoggedResourcesHelper::chars(rText);
#endif

    lcl_positionOffset(rText, bVertical);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::align(const OUString& rText, bool bVertical)
{
#ifdef DBG_UTIL
    mHelper.startElement("align");
    LoggedResourcesHelper::attribute("vertical", static_cast<int>(bVertical));
    LoggedResourcesHelper::chars(rText);
#endif

    lcl_align(rText, bVertical);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::positivePercentage(const OUString& rText)
{
#ifdef DBG_UTIL
    mHelper.startElement("positivePercentage");
    LoggedResourcesHelper::chars(rText);
#endif

    lcl_positivePercentage(rText);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DBG_UTIL
    mHelper.startElement("props");
#endif

    lcl_props(ref);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
#ifdef DBG_UTIL
    mHelper.startElement("table");
    LoggedResourcesHelper::attribute("name", QNameToString(name));
#endif

    lcl_table(name, ref);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref)
{
#ifdef DBG_UTIL
    mHelper.startElement("substream");
    LoggedResourcesHelper::attribute("name", QNameToString(name));
#endif

    lcl_substream(name, ref);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::info(const std::string& _info)
{
#ifdef DBG_UTIL
    mHelper.startElement("info");
    LoggedResourcesHelper::attribute("text", _info);
#else
    (void)_info;
#endif

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::startGlossaryEntry()
{
#ifdef DBG_UTIL
    mHelper.startElement("startGlossaryEntry");
#endif

    lcl_startGlossaryEntry();

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

void LoggedStream::endGlossaryEntry()
{
#ifdef DBG_UTIL
    mHelper.startElement("endGlossaryEntry");
#endif

    lcl_endGlossaryEntry();

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

LoggedProperties::LoggedProperties(
#ifdef DBG_UTIL
    const std::string& sPrefix)
    : mHelper(sPrefix)
#else
    const std::string&)
#endif
{
}

LoggedProperties::~LoggedProperties() {}

void LoggedProperties::attribute(Id name, Value& val)
{
#ifdef DBG_UTIL
    mHelper.startElement("attribute");
    LoggedResourcesHelper::attribute("name", QNameToString(name));
    LoggedResourcesHelper::attribute("value", val.toString());
    LoggedResourcesHelper::endElement();
#endif

    lcl_attribute(name, val);
}

void LoggedProperties::sprm(Sprm& rSprm)
{
#ifdef DBG_UTIL
    mHelper.startElement("sprm");
    LoggedResourcesHelper::attribute("name", QNameToString(rSprm.getId()));
    LoggedResourcesHelper::chars(rSprm.toString());
#endif

    lcl_sprm(rSprm);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}

LoggedTable::LoggedTable(
#ifdef DBG_UTIL
    const std::string& sPrefix)
    : mHelper(sPrefix)
#else
    const std::string&)
#endif
{
}

LoggedTable::~LoggedTable() {}

void LoggedTable::entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DBG_UTIL
    mHelper.startElement("entry");
    LoggedResourcesHelper::attribute("pos", pos);
#else
    (void)pos;
#endif

    lcl_entry(ref);

#ifdef DBG_UTIL
    LoggedResourcesHelper::endElement();
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
