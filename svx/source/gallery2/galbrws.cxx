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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vcl/split.hxx>
#include <vcl/ctrl.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include "gallery.hrc"
#include "svx/galmisc.hxx"
#include "svx/gallery1.hxx"
#include "galbrws1.hxx"
#include "galbrws2.hxx"
#include "svx/galbrws.hxx"

// -------------------
// - GallerySplitter -
// -------------------

class GallerySplitter : public Splitter
{
protected:

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:

                    GallerySplitter( Window* pParent, const ResId& rResId );
    virtual         ~GallerySplitter();
};

// -----------------------------------------------------------------------------
DBG_NAME(GallerySplitter)

GallerySplitter::GallerySplitter( Window* pParent, const ResId& rResId ) :
    Splitter( pParent, rResId )
{
    DBG_CTOR(GallerySplitter,NULL);

}

// -----------------------------------------------------------------------------

GallerySplitter::~GallerySplitter()
{

    DBG_DTOR(GallerySplitter,NULL);
}

// -----------------------------------------------------------------------------

void GallerySplitter::DataChanged( const DataChangedEvent& rDCEvt )
{
    Splitter::DataChanged( rDCEvt );
    static_cast< GalleryBrowser* >( GetParent() )->InitSettings();
}

// -------------------------
// - SvxGalleryChildWindow -
// -------------------------
DBG_NAME(GalleryChildWindow)

GalleryChildWindow::GalleryChildWindow( Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    DBG_CTOR(GalleryChildWindow,NULL);

    pWindow = new GalleryBrowser( pBindings, this, _pParent, GAL_RESID( RID_SVXDLG_GALLERYBROWSER ) );
    eChildAlignment = SFX_ALIGN_TOP;
    ( (GalleryBrowser*) pWindow )->Initialize( pInfo );
};

// -----------------------------------------------------------------------------

GalleryChildWindow::~GalleryChildWindow()
{

    DBG_DTOR(GalleryChildWindow,NULL);
}

// -----------------------------------------------------------------------------

SFX_IMPL_DOCKINGWINDOW( GalleryChildWindow, SID_GALLERY )

// ------------------
// - GalleryBrowser -
// ------------------
DBG_NAME(GalleryBrowser)

GalleryBrowser::GalleryBrowser( SfxBindings* _pBindings, SfxChildWindow* pCW,
                                Window* pParent, const ResId& rResId ) :
    SfxDockingWindow( _pBindings, pCW, pParent, rResId )
{
    DBG_CTOR(GalleryBrowser,NULL);

    mpGallery = Gallery::GetGalleryInstance();
    mpBrowser1 = new GalleryBrowser1( this, GAL_RESID( GALLERY_BROWSER1 ), mpGallery );
    mpSplitter = new GallerySplitter( this, GAL_RESID( GALLERY_SPLITTER ) );
    mpBrowser2 = new GalleryBrowser2( this, GAL_RESID( GALLERY_BROWSER2 ), mpGallery );

    FreeResource();
    SetMinOutputSizePixel( maLastSize = GetOutputSizePixel() );

    mpBrowser1->SelectTheme( 0 );
    mpBrowser1->Show( sal_True );
    mpBrowser2->Show( sal_True );

    mpSplitter->SetSplitHdl( LINK( this, GalleryBrowser, SplitHdl ) );
    mpSplitter->Show( sal_True );

    InitSettings();
}

// -----------------------------------------------------------------------------

GalleryBrowser::~GalleryBrowser()
{
    delete mpBrowser2;
    delete mpSplitter;
    delete mpBrowser1;

    DBG_DTOR(GalleryBrowser,NULL);
}

// -----------------------------------------------------------------------------

void GalleryBrowser::InitSettings()
{
    SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    SetControlBackground( GALLERY_DLG_COLOR );
    SetControlForeground( GALLERY_DLG_COLOR );

    mpSplitter->SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    mpSplitter->SetControlBackground( GALLERY_DLG_COLOR );
    mpSplitter->SetControlForeground( GALLERY_DLG_COLOR );

    mpBrowser1->SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    mpBrowser1->SetControlBackground( GALLERY_DLG_COLOR );
    mpBrowser1->SetControlForeground( GALLERY_DLG_COLOR );

    mpBrowser2->SetBackground( Wallpaper( GALLERY_DLG_COLOR ) );
    mpBrowser2->SetControlBackground( GALLERY_DLG_COLOR );
    mpBrowser2->SetControlForeground( GALLERY_DLG_COLOR );
}

// -----------------------------------------------------------------------------

void GalleryBrowser::Resize()
{
    SfxDockingWindow::Resize();

    const long  nFrameWidth = LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();
    const long  nFrameWidth2 = nFrameWidth << 1;
    Size        aMinSize( GetMinOutputSizePixel() );
    Size        aNewSize( GetOutputSizePixel() );
    Point       aSplitPos( mpSplitter->GetPosPixel() );
    const Size  aSplitSize( mpSplitter->GetOutputSizePixel() );

    mpBrowser1->SetPosSizePixel( Point( nFrameWidth, nFrameWidth ),
                                 Size( aSplitPos.X() - nFrameWidth, aNewSize.Height() - nFrameWidth2 ) );

    mpSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aNewSize.Height() ) );
    mpSplitter->SetDragRectPixel( Rectangle( Point( nFrameWidth2, 0 ), Size( aNewSize.Width() - ( nFrameWidth2 << 1 ) - aSplitSize.Width(), aNewSize.Height() ) ) );

    mpBrowser2->SetPosSizePixel( Point( aSplitPos.X() + aSplitSize.Width(), nFrameWidth ),
                                 Size( aNewSize.Width() - aSplitSize.Width() - aSplitPos.X() - nFrameWidth, aNewSize.Height() - nFrameWidth2 ) );

    maLastSize = aNewSize;
}

// -----------------------------------------------------------------------------

sal_Bool GalleryBrowser::KeyInput( const KeyEvent& rKEvt, Window* )
{
    const sal_uInt16    nCode = rKEvt.GetKeyCode().GetCode();
    sal_Bool            bRet = ( !rKEvt.GetKeyCode().IsMod1() &&
                           ( ( KEY_TAB == nCode ) || ( KEY_F6 == nCode && rKEvt.GetKeyCode().IsMod2() ) ) );

    if( bRet )
    {
        if( !rKEvt.GetKeyCode().IsShift() )
        {
            if( mpBrowser1->mpThemes->HasChildPathFocus( sal_True ) )
                mpBrowser2->GetViewWindow()->GrabFocus();
            else if( mpBrowser2->GetViewWindow()->HasFocus() )
                mpBrowser2->maViewBox.GrabFocus();
            else if( mpBrowser2->maViewBox.HasFocus() )
                mpBrowser1->maNewTheme.GrabFocus();
            else
                mpBrowser1->mpThemes->GrabFocus();
        }
        else
        {
            if( mpBrowser1->mpThemes->HasChildPathFocus( sal_True ) )
                mpBrowser1->maNewTheme.GrabFocus();
            else if( mpBrowser1->maNewTheme.HasFocus() )
                mpBrowser2->maViewBox.GrabFocus();
            else if( mpBrowser2->maViewBox.HasFocus() )
                mpBrowser2->GetViewWindow()->GrabFocus();
            else
                mpBrowser1->mpThemes->GrabFocus();
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GalleryBrowser::Close()
{
    return SfxDockingWindow::Close();
}

// -----------------------------------------------------------------------------

void GalleryBrowser::GetFocus()
{
    SfxDockingWindow::GetFocus();
    mpBrowser1->GrabFocus();
}

// -----------------------------------------------------------------------------

void GalleryBrowser::ThemeSelectionHasChanged()
{
    mpBrowser2->SelectTheme( mpBrowser1->GetSelectedTheme() );
}

// -----------------------------------------------------------------------------

INetURLObject GalleryBrowser::GetURL() const
{
    return mpBrowser2->GetURL();
}

// -----------------------------------------------------------------------------

String GalleryBrowser::GetFilterName() const
{
    return mpBrowser2->GetFilterName();
}

// -----------------------------------------------------------------------------

Graphic GalleryBrowser::GetGraphic() const
{
    return mpBrowser2->GetGraphic();
}

// -----------------------------------------------------------------------------

sal_Bool GalleryBrowser::GetVCDrawModel( FmFormModel& rModel ) const
{
    return mpBrowser2->GetVCDrawModel( rModel );
}

// -----------------------------------------------------------------------------

sal_Bool GalleryBrowser::IsLinkage() const
{
    return mpBrowser2->IsLinkage();
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser, SplitHdl, void*, EMPTYARG )
{
    mpSplitter->SetPosPixel( Point( mpSplitter->GetSplitPosPixel(), mpSplitter->GetPosPixel().Y() ) );
    Resize();

    return 0L;
}
