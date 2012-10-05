/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_GALBRWS_HXX_
#define _SVX_GALBRWS_HXX_

#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/graph.hxx>
#include <tools/urlobj.hxx>
#include "svx/svxdllapi.h"

// -----------
// - Defines -
// -----------

#define GALLERYBROWSER() ((GalleryBrowser*)( SfxViewFrame::Current()->GetChildWindow(GalleryChildWindow::GetChildWindowId())->GetWindow()))

// -------------------------
// - SvxGalleryChildWindow -
// -------------------------

class SVX_DLLPUBLIC GalleryChildWindow : public SfxChildWindow
{
public:
                        GalleryChildWindow( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );
                        ~GalleryChildWindow();

                        SFX_DECL_CHILDWINDOW_WITHID( GalleryChildWindow );
};

// ------------------
// - FormModel -
// ------------------

class FmFormModel;

// ------------------
// - GalleryBrowser -
// ------------------

class GalleryBrowser1;
class GalleryBrowser2;
class GallerySplitter;
class Gallery;

class GalleryBrowser : public SfxDockingWindow
{
    friend class GalleryBrowser1;
    friend class GalleryBrowser2;
    friend class GallerySplitter;
    using Window::KeyInput;

private:

    Size                    maLastSize;
    GallerySplitter*        mpSplitter;
    GalleryBrowser1*        mpBrowser1;
    GalleryBrowser2*        mpBrowser2;
    Gallery*                mpGallery;

    void                    InitSettings();

    virtual sal_Bool            Close();
    virtual void            Resize();
    virtual void            GetFocus();

                            DECL_LINK( SplitHdl, void* );

protected:

    void                    ThemeSelectionHasChanged();

public:

                            GalleryBrowser( SfxBindings* pBindings, SfxChildWindow* pCW,
                                            Window* pParent, const ResId& rResId );
                            ~GalleryBrowser();

    INetURLObject           GetURL() const;
    String                  GetFilterName() const;
    Graphic                 GetGraphic() const;
    sal_Bool                    IsLinkage() const;

    sal_Bool                    KeyInput( const KeyEvent& rKEvt, Window* pWindow );
};

#endif // _SVX_GALBRWS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
