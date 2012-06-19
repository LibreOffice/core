/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateview.hxx>

#include <sfx2/doctempl.hxx>

TemplateView::TemplateView (Window *pParent, SfxDocumentTemplates *pTemplates)
    : ThumbnailView(pParent),
      mnRegionId(0),
      mpDocTemplates(pTemplates)
{
}

TemplateView::~TemplateView ()
{
}

void TemplateView::setRegionId (const sal_uInt16 nRegionId)
{
    mnRegionId = nRegionId;
    maFolderName = mpDocTemplates->GetRegionName(nRegionId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


