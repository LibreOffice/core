/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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

                        SFX_DECL_CHILDWINDOW( GalleryChildWindow );
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

class SVX_DLLPUBLIC GalleryBrowser : public SfxDockingWindow
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

    /// bitfield
    void                    InitSettings();

    virtual sal_Bool        Close();
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
    sal_Bool                    GetVCDrawModel( FmFormModel& rModel ) const;
    sal_Bool                    IsLinkage() const;

    sal_Bool                    KeyInput( const KeyEvent& rKEvt, Window* pWindow );
};

#endif // _SVX_GALBRWS_HXX_
