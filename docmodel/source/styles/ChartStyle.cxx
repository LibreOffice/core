/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/styles/ChartStyle.hxx>
#include <utility>

using namespace model;

void StyleSet::addEntry(enum StyleEntryType eType, const StyleEntry& aEntry)
{
    maEntryMap.emplace(std::make_pair(eType, aEntry));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
