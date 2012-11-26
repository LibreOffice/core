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
                    basegfx::B2DRange aBox(GetPageBox(nPageIndex));

                    if ( ! maPageDescriptors[nPageIndex].mbVisible)
                    {
                        mpWindow->SetLineColor();
                        mpWindow->SetFillColor(maBackgroundColor);
                        mpWindow->DrawRect(aBox);

                        aBox = basegfx::B2DRange(
                            aBox.getMinX() + (maCellSize.getX() * 0.25),
                            aBox.getMinY() + (maCellSize.getY() * 0.25),
                            aBox.getMaxX() - (maCellSize.getX() * 0.25),
                            aBox.getMaxY() - (maCellSize.getY() * 0.25));
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
                        mpWindow->DrawLine(aBox.getMinimum(), aBox.getMaximum());
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




basegfx::B2DRange SlideSorterCacheDisplay::GetPageBox (sal_Int32 nPageIndex)
{
    const sal_Int32 nRow(nPageIndex / mnColumnCount);
    const sal_Int32 nColumn(nPageIndex % mnColumnCount);
    const basegfx::B2DPoint aTopLeft(
        mnHorizontalBorder + nColumn * maCellSize.Width() + nColumn * mnHorizontalGap,
        mnVerticalBorder + nRow * maCellSize.Height() + nRow * mnVerticalGap);

    return basegfx::B2DRange(aTopLeft, aTopLeft + maCellSize);
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
