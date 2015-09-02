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
#include "svx/svxdllapi.h"

#include <boost/scoped_ptr.hpp>

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
    GalleryControl (
        SfxBindings* pBindings,
        vcl::Window* pParentWindow);

    bool GalleryKeyInput( const KeyEvent& rKEvt, vcl::Window* pWindow);

private:
    Gallery* mpGallery;
    VclPtr<GallerySplitter> mpSplitter;
    VclPtr<GalleryBrowser1> mpBrowser1;
    VclPtr<GalleryBrowser2> mpBrowser2;
    Size maLastSize;
    bool mbIsInitialResize;

    void InitSettings();

    virtual void Resize() SAL_OVERRIDE;
    virtual void GetFocus() SAL_OVERRIDE;

    DECL_LINK_TYPED(SplitHdl, Splitter*, void);

protected:
    void ThemeSelectionHasChanged();
    virtual ~GalleryControl();
    virtual void dispose() SAL_OVERRIDE;
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
