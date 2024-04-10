/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <dmapper/resourcemodel.hxx>

namespace writerfilter::rtftok
{
/// Sends tables (e.g. font table) to the domain mapper.
class RTFReferenceTable : public writerfilter::Reference<Table>
{
public:
    using Entries_t = std::map<int, writerfilter::Reference<Properties>::Pointer_t>;
    using Entry_t = std::pair<int, writerfilter::Reference<Properties>::Pointer_t>;
    explicit RTFReferenceTable(Entries_t aEntries);
    ~RTFReferenceTable() override;
    void resolve(Table& rHandler) override;

private:
    Entries_t m_aEntries;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
