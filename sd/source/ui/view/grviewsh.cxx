/*************************************************************************
 *
 *  $RCSfile: grviewsh.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:59:19 $
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

#include "GraphicViewShell.hxx"
#include "LayerTabBar.hxx"
#include "FrameView.hxx"
#include <sfx2/viewfrm.hxx>
#include <vcl/scrbar.hxx>


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
