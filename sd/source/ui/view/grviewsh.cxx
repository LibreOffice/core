/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grviewsh.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:44:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "GraphicViewShell.hxx"
#include "LayerTabBar.hxx"
#include "FrameView.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/scrbar.hxx>

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

namespace sd {

static const int TABCONTROL_INITIAL_SIZE = 350;

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

GraphicViewShell::GraphicViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    FrameView* pFrameView)
    : DrawViewShell (
        pFrame,
        rViewShellBase,
        pParentWindow,
        PK_STANDARD,
        pFrameView)
{
    Construct ();
}

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

GraphicViewShell::GraphicViewShell (
    SfxViewFrame* pFrame,
    ::Window* pParentWindow,
    const DrawViewShell& rShell)
    : DrawViewShell (pFrame, pParentWindow, rShell)
{
    Construct ();
}




GraphicViewShell::~GraphicViewShell (void)
{
}




void GraphicViewShell::Construct (void)
{
    meShellType = ST_DRAW;

    mpLayerTabBar.reset (new LayerTabBar(this,GetParentWindow()));
    mpLayerTabBar->SetSplitHdl(LINK(this,GraphicViewShell,TabBarSplitHandler));

    // pb: #i67363# no layer tabbar on preview mode
    if ( !GetObjectShell()->IsPreview() )
        mpLayerTabBar->Show();
}




void GraphicViewShell::ChangeEditMode (
    EditMode eMode,
    bool bIsLayerModeActive)
{
    // There is no page tab that could be shown instead of the layer tab.
    // Therefore we have it allways visible regardless of what the caller
    // said. (We have to change the callers behaviour, of course.)
    DrawViewShell::ChangeEditMode (eMode, true);
}




void GraphicViewShell::ArrangeGUIElements (void)
{
    if (mpLayerTabBar.get()!=NULL && mpLayerTabBar->IsVisible())
    {
        Size aSize = mpLayerTabBar->GetSizePixel();
        const Size aFrameSize (
            GetViewFrame()->GetWindow().GetOutputSizePixel());

        if (aSize.Width() == 0)
        {
            if (pFrameView->GetTabCtrlPercent() == 0.0)
                aSize.Width() = TABCONTROL_INITIAL_SIZE;
            else
                aSize.Width() = FRound(aFrameSize.Width()
                    * pFrameView->GetTabCtrlPercent());
        }
        aSize.Height() = GetParentWindow()->GetSettings().GetStyleSettings()
            .GetScrollBarSize();

        Point aPos (0, aViewSize.Height() - aSize.Height());

        mpLayerTabBar->SetPosSizePixel (aPos, aSize);

        if (aFrameSize.Width() > 0)
            pFrameView->SetTabCtrlPercent (
                (double) aTabControl.GetSizePixel().Width()
                / aFrameSize.Width());
        else
            pFrameView->SetTabCtrlPercent( 0.0 );
    }

    DrawViewShell::ArrangeGUIElements();
}




IMPL_LINK(GraphicViewShell, TabBarSplitHandler, TabBar*, pTabBar)
{
    const long int nMax = aViewSize.Width()
        - aScrBarWH.Width()
        - pTabBar->GetPosPixel().X();

    Size aTabSize = pTabBar->GetSizePixel();
    aTabSize.Width() = Min(pTabBar->GetSplitSize(), (long)(nMax-1));

    pTabBar->SetSizePixel (aTabSize);

    Point aPos = pTabBar->GetPosPixel();
    aPos.X() += aTabSize.Width();

    Size aScrSize (nMax - aTabSize.Width(), aScrBarWH.Height());
    mpHorizontalScrollBar->SetPosSizePixel(aPos, aScrSize);

    return 0;
}

} // end of namespace sd
