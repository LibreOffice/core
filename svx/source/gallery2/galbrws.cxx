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
#include "GallerySplitter.hxx"

#include <boost/bind.hpp>

DBG_NAME(GalleryChildWindow)

GalleryChildWindow::GalleryChildWindow( Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    DBG_CTOR(GalleryChildWindow,NULL);

    pWindow = new GalleryBrowser( pBindings, this, _pParent, GAL_RES( RID_SVXDLG_GALLERYBROWSER ) );
    eChildAlignment = SFX_ALIGN_TOP;
    ( (GalleryBrowser*) pWindow )->Initialize( pInfo );
};

GalleryChildWindow::~GalleryChildWindow()
{
    DBG_DTOR(GalleryChildWindow,NULL);
}

SFX_IMPL_DOCKINGWINDOW_WITHID( GalleryChildWindow, SID_GALLERY )

// - GalleryBrowser -

DBG_NAME(GalleryBrowser)

GalleryBrowser::GalleryBrowser(
    SfxBindings* _pBindings,
    SfxChildWindow* pCW,
    Window* pParent,
    const ResId& rResId)
:   SfxDockingWindow(_pBindings, pCW, pParent, rResId),
    maLastSize(GetOutputSizePixel()),
    mpSplitter(0),
    mpBrowser1(0),
    mpBrowser2(0),
    mpGallery(0)
{
    DBG_CTOR(GalleryBrowser,NULL);

    mpGallery = Gallery::GetGalleryInstance();
    mpBrowser1 = new GalleryBrowser1(
        this,
        GAL_RES( GALLERY_BROWSER1 ),
        mpGallery,
        ::boost::bind(&GalleryBrowser::KeyInput,this,_1,_2),
        ::boost::bind(&GalleryBrowser::ThemeSelectionHasChanged, this));
    mpSplitter = new GallerySplitter( this, GAL_RES( GALLERY_SPLITTER ),
        ::boost::bind(&GalleryBrowser::InitSettings, this));
    mpBrowser2 = new GalleryBrowser2( this, GAL_RES( GALLERY_BROWSER2 ), mpGallery );

    FreeResource();
    SetMinOutputSizePixel(maLastSize);

    mpBrowser1->SelectTheme( 0 );
    mpBrowser1->Show( sal_True );
    mpBrowser2->Show( sal_True );

    const bool bLayoutHorizontal(maLastSize.Width() > maLastSize.Height());
    mpSplitter->SetHorizontal(bLayoutHorizontal);
    mpSplitter->SetSplitHdl( LINK( this, GalleryBrowser, SplitHdl ) );
    mpSplitter->Show( sal_True );

    InitSettings();
}

GalleryBrowser::~GalleryBrowser()
{
    delete mpBrowser2;
    delete mpSplitter;
    delete mpBrowser1;

    DBG_DTOR(GalleryBrowser,NULL);
}

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

void GalleryBrowser::Resize()
{
    // call parent
    SfxDockingWindow::Resize();

    // update hor/ver
    const Size aNewSize( GetOutputSizePixel() );
    const bool bNewLayoutHorizontal(aNewSize.Width() > aNewSize.Height());
    const bool bOldLayoutHorizontal(mpSplitter->IsHorizontal());
    const long nSplitPos( bOldLayoutHorizontal ? mpSplitter->GetPosPixel().X() : mpSplitter->GetPosPixel().Y());
    const long nSplitSize( bOldLayoutHorizontal ? mpSplitter->GetOutputSizePixel().Width() : mpSplitter->GetOutputSizePixel().Height());

    if(bNewLayoutHorizontal != bOldLayoutHorizontal)
    {
        mpSplitter->SetHorizontal(bNewLayoutHorizontal);
    }

    const long nFrameLen = LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();
    const long nFrameLen2 = nFrameLen << 1;

    if(bNewLayoutHorizontal)
    {
        mpBrowser1->SetPosSizePixel(
            Point( nFrameLen, nFrameLen ),
            Size(nSplitPos - nFrameLen, aNewSize.Height() - nFrameLen2) );

        mpSplitter->SetPosSizePixel(
            Point( nSplitPos, 0),
            Size( nSplitSize, aNewSize.Height() ) );

        mpSplitter->SetDragRectPixel(
            Rectangle(
                Point( nFrameLen2, 0 ),
                Size( aNewSize.Width() - ( nFrameLen2 << 1 ) - nSplitSize, aNewSize.Height() ) ) );

        mpBrowser2->SetPosSizePixel(
            Point( nSplitPos + nSplitSize, nFrameLen ),
            Size( aNewSize.Width() - nSplitSize - nSplitPos - nFrameLen, aNewSize.Height() - nFrameLen2 ) );
    }
    else
    {
        mpBrowser1->SetPosSizePixel(
            Point( nFrameLen, nFrameLen ),
            Size(aNewSize.Width() - nFrameLen2, nSplitPos - nFrameLen));

        mpSplitter->SetPosSizePixel(
            Point( 0, nSplitPos),
            Size( aNewSize.Width(), nSplitSize ) );

        mpSplitter->SetDragRectPixel(
            Rectangle(
                Point( 0, nFrameLen2 ),
                Size( aNewSize.Width(), aNewSize.Height() - ( nFrameLen2 << 1 ) - nSplitSize ) ));

        mpBrowser2->SetPosSizePixel(
            Point( nFrameLen, nSplitPos + nSplitSize ),
            Size( aNewSize.Width() - nFrameLen2, aNewSize.Height() - nSplitSize - nSplitPos - nFrameLen ));
    }

    maLastSize = aNewSize;
}

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

sal_Bool GalleryBrowser::Close()
{
    return SfxDockingWindow::Close();
}

void GalleryBrowser::GetFocus()
{
    SfxDockingWindow::GetFocus();
    mpBrowser1->GrabFocus();
}

void GalleryBrowser::ThemeSelectionHasChanged()
{
    mpBrowser2->SelectTheme( mpBrowser1->GetSelectedTheme() );
}

IMPL_LINK_NOARG(GalleryBrowser, SplitHdl)
{
    if(mpSplitter->IsHorizontal())
    {
        mpSplitter->SetPosPixel( Point( mpSplitter->GetSplitPosPixel(), mpSplitter->GetPosPixel().Y() ) );
    }
    else
    {
        mpSplitter->SetPosPixel( Point( mpSplitter->GetPosPixel().X(), mpSplitter->GetSplitPosPixel() ) );
    }

    Resize();

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
