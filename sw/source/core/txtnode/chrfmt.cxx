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

#include <libxml/xmlwriter.h>

#include <charfmt.hxx>
#include <charformats.hxx>
#include <doc.hxx>

void SwCharFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwCharFormat"));

    SwFormat::dumpAsXml(pWriter);

    if (mpLinkedParaFormat)
    {
        (void)xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("linked"),
            BAD_CAST(mpLinkedParaFormat->GetName().toString().toUtf8().getStr()));
    }

    (void)xmlTextWriterEndElement(pWriter);
}

void SwCharFormat::SetLinkedParaFormat(SwTextFormatColl* pLink) { mpLinkedParaFormat = pLink; }

const SwTextFormatColl* SwCharFormat::GetLinkedParaFormat() const { return mpLinkedParaFormat; }

SwCharFormat::~SwCharFormat()
{
    if (GetDoc()->IsInDtor())
    {
        return;
    }

    for (const auto& pTextFormat : *GetDoc()->GetTextFormatColls())
    {
        if (pTextFormat->GetLinkedCharFormat() == this)
        {
            pTextFormat->SetLinkedCharFormat(nullptr);
        }
    }
}

void SwCharFormats::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwCharFormats"));
    for (size_t i = 0; i < size(); ++i)
        GetFormat(i)->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

SwCharFormats::SwCharFormats()
    : m_PosIndex(m_Array.get<0>())
    , m_NameIndex(m_Array.get<1>())
{
}

SwCharFormats::~SwCharFormats()
{
    // default char format is owned by SwDoc
    DeleteAndDestroyAll(true);
}

SwCharFormats::const_iterator SwCharFormats::find(const SwCharFormat* x) const
{
    ByName::iterator it
        = m_NameIndex.find(std::make_tuple(x->GetName(), const_cast<SwCharFormat*>(x)));
    return m_Array.project<0>(it);
}

SwCharFormats::ByName::const_iterator SwCharFormats::findByName(const UIName& name) const
{
    return m_NameIndex.find(std::make_tuple(name));
}

SwCharFormat* SwCharFormats::FindFormatByName(const UIName& rName) const
{
    auto it = findByName(rName);
    if (it != m_NameIndex.end())
        return *it;
    return nullptr;
}

void SwCharFormats::DeleteAndDestroyAll(bool keepDefault)
{
    if (empty())
        return;
    const int _offset = keepDefault ? 1 : 0;
    for (const_iterator it = begin() + _offset; it != end(); ++it)
    {
        assert(!(*it)->HasName(u"Character style"));
        delete *it;
    }
    if (_offset)
        m_PosIndex.erase(begin() + _offset, end());
    else
        m_Array.clear();
}

void SwCharFormats::insert(SwCharFormat* x)
{
    assert(!ContainsFormat(x));
    m_PosIndex.push_back(x);
}

void SwCharFormats::erase(const_iterator const& position) { m_PosIndex.erase(position); }

bool SwCharFormats::ContainsFormat(const SwCharFormat* x) const { return find(x) != end(); }

/** Need to call this when the format name changes */
void SwCharFormats::SetFormatNameAndReindex(SwCharFormat* v, const UIName& sNewName)
{
    auto it = find(v);
    erase(it);
    v->SetFormatName(sNewName);
    insert(v);
}

size_t SwCharFormats::GetPos(const SwCharFormat* p) const
{
    auto it = find(p);
    return it == end() ? SIZE_MAX : it - begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
