/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <caret.hxx>

SmCaretPosGraph::SmCaretPosGraph() = default;

SmCaretPosGraph::~SmCaretPosGraph() = default;

SmCaretPosGraphEntry* SmCaretPosGraph::Add(SmCaretPos pos,
                                           SmCaretPosGraphEntry* left)
{
    assert(pos.nIndex >= 0);
    auto entry = std::make_unique<SmCaretPosGraphEntry>(pos, left, nullptr);
    SmCaretPosGraphEntry* e = entry.get();
    //Set Left and Right to point to the entry itself if they are NULL
    entry->Left = entry->Left ? entry->Left : e;
    entry->Right = entry->Right ? entry->Right : e;
    mvEntries.push_back(std::move(entry));
    return e;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
