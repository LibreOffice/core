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

#ifndef SD_TASKPANE_SLIDE_SORTER_CACHE_DISPLAY_HXX
#define SD_TASKPANE_SLIDE_SORTER_CACHE_DISPLAY_HXX

// Uncomment the define below to activate the slide sorter cache display in
// the task pane.  Visible slide previews are displayed as large rectangles,
// off-screen previews as smaller rectangles.  The color shows the state:
// green for no action, different shades of yellow for a request being in
// the queue, pink for currently being rendered.  A diagonal line indicates
// that the preview is not up-to-date.
#ifdef DEBUG
//#define USE_SLIDE_SORTER_CACHE_DISPLAY
#endif


#ifdef USE_SLIDE_SORTER_CACHE_DISPLAY
#include <taskpane/TaskPaneTreeNode.hxx>

#include <map>
#include <vector>
#include <memory>

class Window;

#include "svx/svdpage.hxx"
#include "drawdoc.hxx"


namespace sd { namespace toolpanel {

class TreeNode;

/** This panel demonstrates how to create a panel for the task pane.
*/
class SlideSorterCacheDisplay
    : public TreeNode
{
public:
    SlideSorterCacheDisplay (const SdDrawDocument* pDocument);
    virtual ~SlideSorterCacheDisplay (void);

    void SetParentWindow (::Window* pParentWindow);

    virtual void Paint (const Rectangle& rBoundingBox);
    virtual void Resize (void);

    static SlideSorterCacheDisplay* Instance (const SdDrawDocument* pDocument);

    void SetPageCount (sal_Int32 nPageCount);
    enum PageStatus {
        NONE,
        IN_QUEUE_PRIORITY_0,
        IN_QUEUE_PRIORITY_1,
        IN_QUEUE_PRIORITY_2,
        RENDERING
    };
    void SetPageStatus (sal_Int32 nPageIndex, PageStatus eStatus);
    void SetPageVisibility (sal_Int32 nPageIndex, bool bVisible);
    void SetUpToDate (sal_Int32 nPageIndex, bool bUpToDate);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;
    virtual bool IsExpanded (void) const;

private:
    static ::std::map<const SdDrawDocument*, SlideSorterCacheDisplay*> maDisplays;
    static void AddInstance (const SdDrawDocument* pDocument, SlideSorterCacheDisplay* pControl);
    static void RemoveInstance (SlideSorterCacheDisplay* pControl);

    ::Window* mpWindow;
    sal_Int32 mnPageCount;
    sal_Int32 mnColumnCount;
    sal_Int32 mnRowCount;
    Size maCellSize;
    sal_Int32 mnHorizontalBorder;
    sal_Int32 mnVerticalBorder;
    sal_Int32 mnHorizontalGap;
    sal_Int32 mnVerticalGap;

    class PageDescriptor
    {
    public:
        PageStatus meStatus;
        bool mbVisible;
        bool mbUpToDate;
    };
    typedef ::std::vector<PageDescriptor> PageDescriptorList;
    PageDescriptorList maPageDescriptors;

    Rectangle GetPageBox (sal_Int32 nPageIndex);

    void ProvideSize (sal_Int32 nPageIndex);

    void PaintPage (sal_Int32 nPageIndex);
};

} } // end of namespace ::sd::toolpanel

namespace {

void SscdSetStatus (const SdrPage* pPage,
    ::sd::toolpanel::SlideSorterCacheDisplay::PageStatus eStatus)
{
    ::sd::toolpanel::SlideSorterCacheDisplay* pDisplay
        = ::sd::toolpanel::SlideSorterCacheDisplay::Instance(
        dynamic_cast<SdDrawDocument*>(pPage->GetModel()));
    if (pDisplay != NULL)
        pDisplay->SetPageStatus((pPage->GetPageNum()-1)/2, eStatus);
}

void SscdSetRequestClass (const SdrPage* pPage, sal_Int32 nClass)
{
    sd::toolpanel::SlideSorterCacheDisplay::PageStatus eStatus;
    switch (nClass)
    {
        case 0:
            eStatus = ::sd::toolpanel::SlideSorterCacheDisplay::IN_QUEUE_PRIORITY_0; break;
        case 1:
            eStatus = ::sd::toolpanel::SlideSorterCacheDisplay::IN_QUEUE_PRIORITY_1; break;
        case 2:
            eStatus = ::sd::toolpanel::SlideSorterCacheDisplay::IN_QUEUE_PRIORITY_2; break;
        default:
            eStatus = ::sd::toolpanel::SlideSorterCacheDisplay::NONE; break;
    }
    SscdSetStatus(pPage,eStatus);
}

void SscdSetVisibility (const SdrModel* pModel, sal_Int32 nIndex, bool bVisible)
{
    ::sd::toolpanel::SlideSorterCacheDisplay* pDisplay
        = ::sd::toolpanel::SlideSorterCacheDisplay::Instance(
        dynamic_cast<const SdDrawDocument*>(pModel));
    if (pDisplay != NULL)
        pDisplay->SetPageVisibility(nIndex, bVisible);
}



void SscdSetUpToDate (const SdrPage* pPage, bool bUpToDate)
{
    ::sd::toolpanel::SlideSorterCacheDisplay* pDisplay
        = ::sd::toolpanel::SlideSorterCacheDisplay::Instance(
            dynamic_cast<const SdDrawDocument*>(pPage->GetModel()));
    if (pDisplay != NULL)
        pDisplay->SetUpToDate((pPage->GetPageNum()-1)/2, bUpToDate);
}



#define SSCD_SET_REQUEST_CLASS(Page,RequestClass)    \
    SscdSetRequestClass(Page,RequestClass)
#define SSCD_SET_STATUS(RequestData,Status)     \
    SscdSetStatus(RequestData,::sd::toolpanel::SlideSorterCacheDisplay::Status)
#define SSCD_SET_VISIBILITY(Model,Index,Visible) \
    SscdSetVisibility(Model,Index,Visible)
#define SSCD_SET_UPTODATE(Page,UpToDate) \
    SscdSetUpToDate(Page,UpToDate)


}

#else

#define SSCD_SET_REQUEST_CLASS(Page,RequestClass)
#define SSCD_SET_STATUS(RequestData,Status)
#define SSCD_SET_VISIBILITY(Model,Index,Visible)
#define SSCD_SET_UPTODATE(Page,UpToDate)

#endif

#endif
