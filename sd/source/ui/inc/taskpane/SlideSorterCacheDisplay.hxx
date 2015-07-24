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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TASKPANE_SLIDESORTERCACHEDISPLAY_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TASKPANE_SLIDESORTERCACHEDISPLAY_HXX

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

namespace vcl { class Window; }

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
    virtual ~SlideSorterCacheDisplay();

    void SetParentWindow (vcl::Window* pParentWindow);

    virtual void Paint (const Rectangle& rBoundingBox);
    virtual void Resize();

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

    virtual Size GetPreferredSize();
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual vcl::Window* GetWindow();
    virtual bool IsResizable();
    virtual bool IsExpandable() const;
    virtual bool IsExpanded() const;

private:
    static ::std::map<const SdDrawDocument*, SlideSorterCacheDisplay*> maDisplays;
    static void AddInstance (const SdDrawDocument* pDocument, SlideSorterCacheDisplay* pControl);
    static void RemoveInstance (SlideSorterCacheDisplay* pControl);

    VclPtr<vcl::Window> mpWindow;
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

}

#else

#define SSCD_SET_REQUEST_CLASS(Page,RequestClass)
#define SSCD_SET_STATUS(RequestData,Status)
#define SSCD_SET_VISIBILITY(Model,Index,Visible)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
