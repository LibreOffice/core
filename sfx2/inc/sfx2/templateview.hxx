/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEVIEW_HXX
#define TEMPLATEVIEW_HXX

#include <sfx2/thumbnailview.hxx>

class SfxDocumentTemplates;

class TemplateView : public ThumbnailView
{
public:

    TemplateView (Window *pParent, SfxDocumentTemplates *pTemplates);

    virtual ~TemplateView ();

private:

     SfxDocumentTemplates *mpDocTemplates;
};

#endif // TEMPLATEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

