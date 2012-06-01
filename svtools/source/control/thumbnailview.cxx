/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/thumbnailview.hxx>

ThumbnailView::ThumbnailView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : ValueSet(pParent,nWinStyle,bDisableTransientChildren)
{
}

ThumbnailView::ThumbnailView (Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : ValueSet(pParent,rResId,bDisableTransientChildren)
{
}

ThumbnailView::~ThumbnailView()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


