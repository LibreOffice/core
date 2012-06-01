/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef THUMBNAILVIEW_HXX
#define THUMBNAILVIEW_HXX

#include <svtools/valueset.hxx>

/**
 *
 *  Class to display thumbnails with their names below their respective icons
 *
 **/

class SVT_DLLPUBLIC ThumbnailView : public ValueSet
{
public:

    ThumbnailView ( Window* pParent, WinBits nWinStyle = WB_ITEMBORDER, bool bDisableTransientChildren = false );

    ThumbnailView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );

    virtual ~ThumbnailView ();
};

#endif // THUMBNAILVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
