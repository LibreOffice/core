/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"

#include <svl/itemset.hxx>

#include <optional>

class EditTextObject;
class ScDocument;

namespace sc::seclabel
{
/// Whether pArea, one area of a header or footer, holds exactly the security label marking of
/// rDocument: the marking stored with the label, or the marking derived from its policy.
SC_DLLPUBLIC bool isMarking(const ScDocument& rDocument, const EditTextObject* pArea);

/// rChanges, adjusted so that applying it to a page style whose item set was rStyleSet keeps a
/// header or footer that holds the marking as it was: switched on, with the same content sharing
/// between pages, and with the marking as its centre text. Empty when rChanges needs no
/// adjustment.
std::optional<SfxItemSet> keepMarking(const ScDocument& rDocument, const SfxItemSet& rStyleSet,
                                      const SfxItemSet& rChanges);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
