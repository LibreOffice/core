/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtfreferencetable.hxx>

namespace writerfilter
{
namespace rtftok
{

RTFReferenceTable::RTFReferenceTable(Entries_t const& rEntries)
    : m_aEntries(rEntries)
{
}

RTFReferenceTable::~RTFReferenceTable()
{
}

void RTFReferenceTable::resolve(Table& rHandler)
{
    for (Entries_t::const_iterator i = m_aEntries.begin(); i != m_aEntries.end(); ++i)
        rHandler.entry(i->first, i->second);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
