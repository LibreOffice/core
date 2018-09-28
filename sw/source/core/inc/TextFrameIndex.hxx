/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_CORE_INC_TEXTFRAMEINDEX_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TEXTFRAMEINDEX_HXX

#include <sal/types.h>
#include <o3tl/strong_int.hxx>

#if 0
typedef o3tl::strong_int<sal_Int32, struct Tag_TextFrameIndex> TextFrameIndex;
#else
/**
 * Denotes a character index in a text frame at a layout level, after extent
 * mapping from a text node at a document model level.
 *
 * @see SwTextFrame::MapViewToModelPos().
 */
typedef sal_Int32 TextFrameIndex;
#endif

#endif // INCLUDED_SW_SOURCE_CORE_INC_TEXTFRAMEINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
