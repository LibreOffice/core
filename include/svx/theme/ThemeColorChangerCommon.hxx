/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <set>
#include <svx/svxdllapi.h>
#include <docmodel/theme/ColorSet.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svl/undo.hxx>

namespace svx
{
namespace theme
{
SVXCORE_DLLPUBLIC void updateSdrObject(model::ColorSet const& rColorSet, SdrObject* pObject,
                                       SdrView* pView, SfxUndoManager* pUndoManager = nullptr);

/// Sends to the LOK updated palettes
SVXCORE_DLLPUBLIC void notifyLOK(std::shared_ptr<model::ColorSet> const& pColorSet,
                                 const std::set<Color>& rDocumentColors);
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
