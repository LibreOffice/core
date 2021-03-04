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
#ifndef INCLUDED_SVX_INC_GALLERYCONTROL_HXX
#define INCLUDED_SVX_INC_GALLERYCONTROL_HXX

#include <config_options.h>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/svxdllapi.h>

class SfxBindings;

class Gallery;
class GalleryBrowser1;
class GalleryBrowser2;
class FmFormModel;
class Splitter;

namespace svx::sidebar
{
class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) GalleryControl final : public PanelLayout
{
public:
    GalleryControl(weld::Widget* pParentWindow);
    virtual ~GalleryControl() override;

private:
    Gallery* mpGallery;
    std::unique_ptr<GalleryBrowser1> mxBrowser1;
    std::unique_ptr<GalleryBrowser2> mxBrowser2;
};

} // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
