/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <GalleryControl.hxx>

#include <svx/gallery1.hxx>
#include "galbrws1.hxx"
#include <galbrws2.hxx>

namespace svx::sidebar {

GalleryControl::GalleryControl(vcl::Window* pParent)
    : PanelLayout(pParent, "GalleryPanel", "svx/ui/sidebargallery.ui", nullptr)
    , mpGallery(Gallery::GetGalleryInstance())
    , mxBrowser1(new GalleryBrowser1(
              *m_xBuilder,
              mpGallery,
              [this] ()
                  { return mxBrowser2->SelectTheme(mxBrowser1->GetSelectedTheme()); }))
    , mxBrowser2(new GalleryBrowser2(*m_xBuilder, mpGallery))
{
    mxBrowser1->SelectTheme(0);
    m_pInitialFocusWidget = mxBrowser1->GetInitialFocusWidget();
}

GalleryControl::~GalleryControl()
{
    disposeOnce();
}

void GalleryControl::dispose()
{
    mxBrowser2.reset();
    mxBrowser1.reset();
    PanelLayout::dispose();
}

void GalleryControl::GetFocus()
{
    Window::GetFocus();
    if (mxBrowser1)
        mxBrowser1->GrabFocus();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
