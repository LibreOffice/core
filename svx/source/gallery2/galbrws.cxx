/*************************************************************************
 *
 *  $RCSfile: galbrws.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-14 13:23:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <vcl/split.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include "gallery.hrc"
#include "galmisc.hxx"
#include "gallery1.hxx"
#include "galbrws1.hxx"
#include "galbrws2.hxx"
#include "galbrws.hxx"

// -------------------------
// - SvxGalleryChildWindow -
// -------------------------

GalleryChildWindow::GalleryChildWindow( Window* pParent, USHORT nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParent, nId )
{
    pWindow = new GalleryBrowser( pBindings, this, pParent, GAL_RESID( RID_SVXDLG_GALLERYBROWSER ) );
    eChildAlignment = SFX_ALIGN_TOP;
    ( (GalleryBrowser*) pWindow )->Initialize( pInfo );
};

// -----------------------------------------------------------------------------

GalleryChildWindow::~GalleryChildWindow()
{
}

// -----------------------------------------------------------------------------

SFX_IMPL_DOCKINGWINDOW( GalleryChildWindow, SID_GALLERY )

// ------------------
// - GalleryBrowser -
// ------------------

GalleryBrowser::GalleryBrowser( SfxBindings* pBindings, SfxChildWindow* pCW,
                                Window* pParent, const ResId& rResId ) :
    SfxDockingWindow( pBindings, pCW, pParent, rResId )
{
    mpGallery = Gallery::AcquireGallery( SvtPathOptions().GetGalleryPath() );
    mpBrowser1 = new GalleryBrowser1( this, GAL_RESID( GALLERY_BROWSER1 ), mpGallery );
    mpSplitter = new Splitter( this, GAL_RESID( GALLERY_SPLITTER ) );
    mpBrowser2 = new GalleryBrowser2( this, GAL_RESID( GALLERY_BROWSER2 ), mpGallery );

    FreeResource();
    SetMinOutputSizePixel( maLastSize = GetOutputSizePixel() );

    mpBrowser1->SelectTheme( 0 );
    mpBrowser1->Show( TRUE );

    mpBrowser2->Show( TRUE );

    mpSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    mpSplitter->SetSplitHdl( LINK( this, GalleryBrowser, SplitHdl ) );
    mpSplitter->Show( TRUE );
}

// -----------------------------------------------------------------------------

GalleryBrowser::~GalleryBrowser()
{
    delete mpBrowser2;
    delete mpSplitter;
    delete mpBrowser1;
    Gallery::ReleaseGallery( mpGallery );
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

    if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aNewSize.Width() - nFrameWidth ) )
        aSplitPos.X() = aNewSize.Width() - nFrameWidth - aSplitSize.Width();

    mpBrowser1->SetPosSizePixel( Point( nFrameWidth, nFrameWidth ),
                                 Size( aSplitPos.X() - nFrameWidth, aNewSize.Height() - nFrameWidth2 ) );

    mpSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aNewSize.Height() ) );
    mpSplitter->SetDragRectPixel( Rectangle( Point( nFrameWidth, 0 ), Size( aNewSize.Width() - nFrameWidth2, aNewSize.Height() ) ) );

    mpBrowser2->SetPosSizePixel( Point( aSplitPos.X() + aSplitSize.Width(), nFrameWidth ),
                                 Size( aNewSize.Width() - aSplitSize.Width() - aSplitPos.X() - nFrameWidth, aNewSize.Height() - nFrameWidth2 ) );

    maLastSize = aNewSize;
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser::Close()
{
    return SfxDockingWindow::Close();
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

BOOL GalleryBrowser::GetVCDrawModel( FmFormModel& rModel ) const
{
    return mpBrowser2->GetVCDrawModel( rModel );
}

// -----------------------------------------------------------------------------

BOOL GalleryBrowser::IsLinkage() const
{
    return mpBrowser2->IsLinkage();
}

// -----------------------------------------------------------------------------

IMPL_LINK( GalleryBrowser, SplitHdl, void*, p )
{
    mpSplitter->SetPosPixel( Point( mpSplitter->GetSplitPosPixel(), mpSplitter->GetPosPixel().Y() ) );
    Resize();

    return 0L;
}
