/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATEONLINEVIEW_HXX__
#define __SFX2_TEMPLATEONLINEVIEW_HXX__

#include <sfx2/thumbnailview.hxx>

class TemplateOnlineView : public ThumbnailView
{
public:

    TemplateOnlineView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    virtual ~TemplateOnlineView ();
};

#endif // __SFX2_TEMPLATEONLINEVIEW_HXX__


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
