/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATESEARCHVIEW_HXX__
#define __SFX2_TEMPLATESEARCHVIEW_HXX__

#include <sfx2/thumbnailview.hxx>

class TemplateSearchView : public ThumbnailView
{
public:

    TemplateSearchView ( Window* pParent, WinBits nWinStyle = WB_TABSTOP | WB_VSCROLL);

    virtual ~TemplateSearchView();

    void AppendItem(sal_uInt16 nItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                    const OUString &rTitle, const OUString &rSubtitle,
                    const OUString &rPath, const BitmapEx &rImage );
};

#endif // __SFX2_TEMPLATESEARCHVIEW_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
