/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <vcl/ITiledRenderable.hxx>

class SfxDispatcher;

namespace svx
{
/// Embeds one file of ITiledRenderable::insertFiles() through the view's
/// dispatcher, synchronously: a picture as SID_INSERT_GRAPHIC, a media file
/// as SID_INSERT_AVMEDIA with the size read from the file, so the insert
/// needs no media player. Without avmedia a media file is skipped.
SVXCORE_DLLPUBLIC void DispatchInsertFile(SfxDispatcher& rDispatcher,
                                          const vcl::InsertFile& rFile);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
