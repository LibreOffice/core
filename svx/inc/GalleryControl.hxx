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

#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <tools/urlobj.hxx>
#include <svx/svxdllapi.h>

class SfxBindings;

class Gallery;
class GallerySplitter;
class GalleryBrowser1;
class GalleryBrowser2;
class FmFormModel;
class Splitter;

namespace svx { namespace sidebar {


class SVX_DLLPUBLIC GalleryControl : public vcl::Window
{
public:
    GalleryControl( vcl::Window* pParentWindow );

    bool GalleryKeyInput( const KeyEvent& rKEvt );

private:
    Gallery* mpGallery;
    VclPtr<GallerySplitter> mpSplitter;
    VclPtr<GalleryBrowser1> mpBrowser1;
    VclPtr<GalleryBrowser2> mpBrowser2;
    bool mbIsInitialResize;

    void InitSettings();

    virtual void Resize() override;
    virtual void GetFocus() override;

    DECL_LINK(SplitHdl, Splitter*, void);

protected:
    virtual ~GalleryControl() override;
    virtual void dispose() override;
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
