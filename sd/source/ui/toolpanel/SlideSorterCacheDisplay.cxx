/*************************************************************************
 *
 *  $RCSfile: SlideSorterCacheDisplay.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:45:09 $
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

/** This factory class is used to create instances of TestPanel.  It can be
    extended so that its constructor stores arguments that later are passed
    to new TestPanel objects.
*/
class SlideSorterCacheDisplayFactory
    : public ControlFactory
{
public:
    SlideSorterCacheDisplayFactory (const SdDrawDocument* pDocument)
        : mpDocument(pDocument)
    {
    }

protected:
    virtual TreeNode* InternalCreateControl (TreeNode* pTreeNode)
    {
        SlideSorterCacheDisplay* pDisplay = SlideSorterCacheDisplay::Instance(mpDocument);
        pDisplay->SetParentWindow(pTreeNode->GetWindow());
        pDisplay->SetParentNode(pTreeNode);
        return pDisplay;
    }

private:
    const SdDrawDocument* mpDocument;
};







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




std::auto_ptr<ControlFactory> SlideSorterCacheDisplay::CreateControlFactory (
    const SdDrawDocument* pDocument)
{
    return std::auto_ptr<ControlFactory>(new SlideSorterCacheDisplayFactory(pDocument));
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
