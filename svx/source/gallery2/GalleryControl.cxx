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

#include <svx/galmisc.hxx>
#include <svx/gallery1.hxx>
#include "galbrws1.hxx"
#include <galbrws2.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sfx2/sidebar/Theme.hxx>

namespace svx::sidebar {

GalleryControl::GalleryControl(vcl::Window* pParent)
    : PanelLayout(pParent, "GalleryPanel", "svx/ui/sidebargallery.ui", nullptr, true)
    , mpGallery(Gallery::GetGalleryInstance())
    , mxBrowser1(new GalleryBrowser1(
              *m_xBuilder,
              mpGallery,
#if 0
              [this] (KeyEvent const& rEvent, vcl::Window *const /*pWindow*/)
                  { return this->GalleryKeyInput(rEvent); },
#endif
              [this] ()
                  { return mxBrowser2->SelectTheme(mxBrowser1->GetSelectedTheme()); }))
    , mxBrowser2(new GalleryBrowser2(*m_xBuilder, mpGallery))
{
    mxBrowser1->SelectTheme(0);
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

#if 0
void GalleryControl::Resize()
{
    // call parent
    Window::Resize();

    // update hor/ver
    const Size aNewSize( GetOutputSizePixel() );
    if (aNewSize.Width()<=0 || aNewSize.Height()<=0)
        return;

    const bool bNewLayoutHorizontal(aNewSize.Width() > aNewSize.Height());
    const bool bOldLayoutHorizontal(mpSplitter->IsHorizontal());
    long nSplitPos( bOldLayoutHorizontal ? mpSplitter->GetPosPixel().X() : mpSplitter->GetPosPixel().Y());
    const long nSplitSize( bOldLayoutHorizontal ? mpSplitter->GetOutputSizePixel().Width() : mpSplitter->GetOutputSizePixel().Height());

    if(bNewLayoutHorizontal != bOldLayoutHorizontal)
    {
        mpSplitter->SetHorizontal(bNewLayoutHorizontal);
    }
    else
    {
    }

    const long nFrameLen = LogicToPixel(Size(3, 0), MapMode(MapUnit::MapAppFont)).Width();
    const long nFrameLen2 = nFrameLen << 1;

    if(bNewLayoutHorizontal)
    {
        mxBrowser1->SetPosSizePixel(
            Point( nFrameLen, nFrameLen ),
            Size(nSplitPos - nFrameLen, aNewSize.Height() - nFrameLen2) );

        mpSplitter->SetPosSizePixel(
            Point( nSplitPos, 0),
            Size( nSplitSize, aNewSize.Height() ) );

        mpSplitter->SetDragRectPixel(
            tools::Rectangle(
                Point( nFrameLen2, 0 ),
                Size( aNewSize.Width() - ( nFrameLen2 << 1 ) - nSplitSize, aNewSize.Height() ) ) );

        mxBrowser2->SetPosSizePixel(
            Point( nSplitPos + nSplitSize, nFrameLen ),
            Size( aNewSize.Width() - nSplitSize - nSplitPos - nFrameLen, aNewSize.Height() - nFrameLen2 ) );
    }
    else
    {
        mxBrowser1->SetPosSizePixel(
            Point( nFrameLen, nFrameLen ),
            Size(aNewSize.Width() - nFrameLen2, nSplitPos - nFrameLen));

        mpSplitter->SetPosSizePixel(
            Point( 0, nSplitPos),
            Size( aNewSize.Width(), nSplitSize ) );

        mpSplitter->SetDragRectPixel(
            tools::Rectangle(
                Point( 0, nFrameLen2 ),
                Size( aNewSize.Width(), aNewSize.Height() - ( nFrameLen2 << 1 ) - nSplitSize ) ));

        mxBrowser2->SetPosSizePixel(
            Point( nFrameLen, nSplitPos + nSplitSize ),
            Size( aNewSize.Width() - nFrameLen2, aNewSize.Height() - nSplitSize - nSplitPos - nFrameLen ));
    }
}
#endif

#if 0
bool GalleryControl::GalleryKeyInput( const KeyEvent& rKEvt )
{
    const sal_uInt16    nCode = rKEvt.GetKeyCode().GetCode();
    bool            bRet = ( !rKEvt.GetKeyCode().IsMod1() &&
                           ( ( KEY_TAB == nCode ) || ( KEY_F6 == nCode && rKEvt.GetKeyCode().IsMod2() ) ) );

    if( bRet )
    {
        if( !rKEvt.GetKeyCode().IsShift() )
        {
            if( mxBrowser1->maNewTheme->HasFocus() )
                mxBrowser1->mpThemes->GrabFocus();
            else if( mxBrowser1->mpThemes->HasChildPathFocus( true ) )
                mxBrowser2->maViewBox->GrabFocus();
            else if( mxBrowser2->maViewBox->HasFocus() )
                mxBrowser2->GetViewWindow()->GrabFocus();
            else
            {
                if( mxBrowser1->maNewTheme->IsEnabled() )
                    mxBrowser1->maNewTheme->GrabFocus();
                else
                    mxBrowser1->mpThemes->GrabFocus();
            }
        }
        else
        {
            if( mxBrowser2->GetViewWindow()->HasFocus() )
                mxBrowser2->maViewBox->GrabFocus();
            else if( mxBrowser2->maViewBox->HasFocus() )
                mxBrowser1->mpThemes->GrabFocus();
            else if( mxBrowser1->mpThemes->HasChildPathFocus( true ) )
            {
                if( mxBrowser1->maNewTheme->IsEnabled() )
                    mxBrowser1->maNewTheme->GrabFocus();
                else
                    mxBrowser2->GetViewWindow()->GrabFocus();
            }
            else
                mxBrowser2->GetViewWindow()->GrabFocus();
        }
    }

    return bRet;
}

void GalleryControl::GetFocus()
{
    Window::GetFocus();
    if (mxBrowser1)
        mxBrowser1->GrabFocus();
}

IMPL_LINK_NOARG( GalleryControl, SplitHdl, Splitter*, void )
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
}

#endif

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
