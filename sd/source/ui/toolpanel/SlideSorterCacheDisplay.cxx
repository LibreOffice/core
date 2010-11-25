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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "taskpane/SlideSorterCacheDisplay.hxx"

#ifdef USE_SLIDE_SORTER_CACHE_DISPLAY

#include "taskpane/ScrollPanel.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"

#include <vcl/window.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

namespace {

static const Color maBackgroundColor (255,250,245);

class PageCacheWindow : public ::Window
{
public:
    PageCacheWindow (
        ::Window* pParentWindow,
        ::sd::toolpanel::SlideSorterCacheDisplay* pDisplay)
        : ::Window(pParentWindow),
           mpDisplay(pDisplay)
    {
        SetBackground(Wallpaper(maBackgroundColor));
    }

    virtual void Paint (const Rectangle& rBoundingBox)
    { mpDisplay->Paint(rBoundingBox); ::Window::Paint(rBoundingBox); }
    virtual void Resize (void) { mpDisplay->Resize(); ::Window::Resize(); }

private:
    ::sd::toolpanel::SlideSorterCacheDisplay* mpDisplay;
};

}



namespace sd { namespace toolpanel {

::std::map<const SdDrawDocument*, SlideSorterCacheDisplay*> SlideSorterCacheDisplay::maDisplays;

SlideSorterCacheDisplay::SlideSorterCacheDisplay (const SdDrawDocument* pDocument)
    : TreeNode(NULL),
      mpWindow(NULL),
      mnPageCount(0),
      mnColumnCount(0),
      mnRowCount(0),
      maCellSize(0,0),
      mnHorizontalBorder(0),
      mnVerticalBorder(0)
{
    SlideSorterCacheDisplay::AddInstance(pDocument,this);
}




SlideSorterCacheDisplay::~SlideSorterCacheDisplay (void)
{
    if (mpWindow != NULL)
        delete mpWindow;
    SlideSorterCacheDisplay::RemoveInstance(this);
}




void SlideSorterCacheDisplay::SetParentWindow (::Window* pParentWindow)
{
    mpWindow = new PageCacheWindow(pParentWindow, this);
}




void SlideSorterCacheDisplay::Paint (const Rectangle& rBoundingBox)
{
    if (maCellSize.Width()>0 && maCellSize.Height()>0 && mpWindow!=NULL)
    {
        Color maSavedFillColor (mpWindow->GetFillColor());
        Color maSavedLineColor (mpWindow->GetLineColor());
        sal_Int32 nC0 = (rBoundingBox.Left() - mnHorizontalBorder) / maCellSize.Width();
        sal_Int32 nC1 = (rBoundingBox.Right() - mnHorizontalBorder) / maCellSize.Width();
        sal_Int32 nR0 = (rBoundingBox.Top() - mnVerticalBorder) / maCellSize.Height();
        sal_Int32 nR1 = (rBoundingBox.Bottom() - mnVerticalBorder) / maCellSize.Height();
        for (sal_Int32 nC=nC0; nC<=nC1; ++nC)
            for (sal_Int32 nR=nR0; nR<=nR1; ++nR)
            {
                sal_Int32 nPageIndex (nC + nR*mnColumnCount);
                if (nPageIndex < mnPageCount)
                {
                    Rectangle aBox (GetPageBox(nPageIndex));
                    if ( ! maPageDescriptors[nPageIndex].mbVisible)
                    {
                        mpWindow->SetLineColor();
                        mpWindow->SetFillColor(maBackgroundColor);
                        mpWindow->DrawRect(aBox);

                        aBox.Left() += maCellSize.Width()/4;
                        aBox.Right() -= maCellSize.Width()/4;
                        aBox.Top() += maCellSize.Height()/4;
                        aBox.Bottom() -= maCellSize.Height()/4;
                    }

                    switch (maPageDescriptors[nPageIndex].meStatus)
                    {
                        case NONE : mpWindow->SetFillColor (Color(95,255,128)); break;
                        case RENDERING : mpWindow->SetFillColor (Color(236,125,128)); break;
                        case IN_QUEUE_PRIORITY_0 : mpWindow->SetFillColor (Color(255,243,0)); break;
                        case IN_QUEUE_PRIORITY_1 : mpWindow->SetFillColor (Color(255,199,0)); break;
                        case IN_QUEUE_PRIORITY_2 : mpWindow->SetFillColor (Color(20,255,128)); break;
                        default : mpWindow->SetFillColor (COL_BLACK); break;
                    }
                    mpWindow->SetLineColor(COL_BLACK);
                    mpWindow->DrawRect(aBox);

                    if ( ! maPageDescriptors[nPageIndex].mbUpToDate)
                        mpWindow->DrawLine(aBox.TopLeft(), aBox.BottomRight());
                }
            }
        mpWindow->SetLineColor(maSavedLineColor);
        mpWindow->SetFillColor(maSavedFillColor);
    }
}




void SlideSorterCacheDisplay::Resize (void)
{
    if (mpWindow != NULL)
    {
        double nW = mpWindow->GetSizePixel().Width();
        double nH = mpWindow->GetSizePixel().Height();
        if (nH > 0)
        {
            double nAspect = nW / nH;
            sal_Int32 nR = 1;
            sal_Int32 nC = 1;
            while (nR * nC < mnPageCount)
            {
                if (double(nC) / double(nR) > nAspect)
                    ++nR;
                else
                    ++nC;
            }
            double nAspect2 = double(nC) / double(nR);

            mnRowCount = nR;
            mnColumnCount = nC;
            mnHorizontalGap = 2;
            mnVerticalGap = 2;
            maCellSize = Size(
                (int)((nW-(nC-1)*mnHorizontalGap) / nC),
                (int)((nH-(nR-1)*mnVerticalGap) / nR));
            mnHorizontalBorder = (int)(nW - nC*maCellSize.Width() - ((nC-1)*mnHorizontalGap))/2;
            mnVerticalBorder = (int)(nH - nR*maCellSize.Height() - ((nR-1)*mnVerticalGap))/2;
        }
    }
}




SlideSorterCacheDisplay* SlideSorterCacheDisplay::Instance (const SdDrawDocument* pDocument)
{
    SlideSorterCacheDisplay* pDisplay = NULL;
    ::std::map<const SdDrawDocument*, SlideSorterCacheDisplay*>::iterator iDisplay;
    for (iDisplay=maDisplays.begin(); iDisplay!=maDisplays.end(); ++iDisplay)
        if (iDisplay->first == pDocument)
            pDisplay = iDisplay->second;

    if (pDisplay == NULL)
    {
        pDisplay = new SlideSorterCacheDisplay(pDocument);
    }

    return pDisplay;
}




void SlideSorterCacheDisplay::SetPageCount (sal_Int32 nPageCount)
{
    mnPageCount = nPageCount;
    maPageDescriptors.resize(nPageCount);
    Resize();
    if (mpWindow != NULL)
        mpWindow->Invalidate();
}




void SlideSorterCacheDisplay::SetPageStatus (sal_Int32 nPageIndex, PageStatus eStatus)
{
    ProvideSize(nPageIndex);
    maPageDescriptors[nPageIndex].meStatus = eStatus;
    PaintPage(nPageIndex);
}




void SlideSorterCacheDisplay::SetPageVisibility (sal_Int32 nPageIndex, bool bVisible)
{
    ProvideSize(nPageIndex);
    maPageDescriptors[nPageIndex].mbVisible = bVisible;
    PaintPage(nPageIndex);
}




void SlideSorterCacheDisplay::SetUpToDate (sal_Int32 nPageIndex, bool bUpToDate)
{
    ProvideSize(nPageIndex);
    maPageDescriptors[nPageIndex].mbUpToDate = bUpToDate;
    PaintPage(nPageIndex);
}




Rectangle SlideSorterCacheDisplay::GetPageBox (sal_Int32 nPageIndex)
{
    sal_Int32 nRow = nPageIndex / mnColumnCount;
    sal_Int32 nColumn = nPageIndex % mnColumnCount;
    return Rectangle(
        Point(mnHorizontalBorder + nColumn * maCellSize.Width() + nColumn*mnHorizontalGap,
            mnVerticalBorder + nRow * maCellSize.Height() + nRow*mnVerticalGap),
        maCellSize);
}




void SlideSorterCacheDisplay::AddInstance (
    const SdDrawDocument* pDocument,
    SlideSorterCacheDisplay* pControl)
{
    maDisplays[pDocument] = pControl;
}




void SlideSorterCacheDisplay::RemoveInstance (SlideSorterCacheDisplay* pControl)
{
    ::std::map<const SdDrawDocument*, SlideSorterCacheDisplay*>::iterator iDisplay;
    for (iDisplay=maDisplays.begin(); iDisplay!=maDisplays.end(); ++iDisplay)
        if (iDisplay->second == pControl)
        {
            maDisplays.erase(iDisplay);
            break;
        }
}




void SlideSorterCacheDisplay::ProvideSize (sal_Int32 nPageIndex)
{
    if (maPageDescriptors.size() <= (sal_uInt32)nPageIndex)
        maPageDescriptors.resize(nPageIndex+1);
    if (mnPageCount <= nPageIndex)
        mnPageCount = nPageIndex;
}




Size SlideSorterCacheDisplay::GetPreferredSize (void)
{
    return Size(100,100);
}




sal_Int32 SlideSorterCacheDisplay::GetPreferredWidth (sal_Int32 nHeigh)
{
    return GetPreferredSize().Width();
}




sal_Int32 SlideSorterCacheDisplay::GetPreferredHeight (sal_Int32 nWidth)
{
    return GetPreferredSize().Height();
}



::Window* SlideSorterCacheDisplay::GetWindow (void)
{
    return mpWindow;
}




bool SlideSorterCacheDisplay::IsResizable (void)
{
    return true;
}




bool SlideSorterCacheDisplay::IsExpandable (void) const
{
    return true;
}




bool SlideSorterCacheDisplay::IsExpanded (void) const
{
    return true;
}




void SlideSorterCacheDisplay::PaintPage (sal_Int32 nPageIndex)
{
    if (mpWindow != NULL)
    {
        Paint(GetPageBox(nPageIndex));
    }
}


} } // end of namespace ::sd::toolpanel

#endif
