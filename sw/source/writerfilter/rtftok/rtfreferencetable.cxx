/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfreferencetable.hxx"

namespace writerfilter::rtftok
{
RTFReferenceTable::RTFReferenceTable(Entries_t aEntries)
    : m_aEntries(std::move(aEntries))
{
}

RTFReferenceTable::~RTFReferenceTable() = default;

void RTFReferenceTable::resolve(Table& rHandler)
{
    for (const auto& rEntry : m_aEntries)
        rHandler.entry(rEntry.first, rEntry.second);
}

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
