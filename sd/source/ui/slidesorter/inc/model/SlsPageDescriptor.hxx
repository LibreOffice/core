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

#ifndef SD_SLIDESORTER_PAGE_DESCRIPTOR_HXX
#define SD_SLIDESORTER_PAGE_DESCRIPTOR_HXX

#include "model/SlsVisualState.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <tools/gen.hxx>
#include <vcl/bitmap.hxx>
#include <sfx2/viewfrm.hxx>

#include <memory>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>


class SdPage;
class SdrPage;

namespace sd { namespace slidesorter { namespace model {

/** Each PageDescriptor object represents the preview of one draw page,
    slide, or master page of a Draw or Impress document as they are displayed
    in the slide sorter.  This class gives access to some associated
    information like prerendered preview or position on the screen.

    <p>Bounding boxes of page objects come in four varieties:
    Model and screen/pixel coordinates and the bounding boxes of the actual
    page objects and the larger bounding boxes that include page names and
    fade symbol.</p>
*/
class PageDescriptor
    : public ::boost::enable_shared_from_this<PageDescriptor>
{
public:
    /** Create a PageDescriptor for the given SdPage object.
        @param rxPage
            The page that is represented by the new PageDescriptor object.
        @param pPage
            The page pointer can in some situations not be detected from
            rxPage, e.g. after undo of page deletion.  Therefore supply it
            separately.
        @param nIndex
            This index is displayed in the view as page number.  It is not
            necessaryily the page index (not even when you add or subtract 1
            or use (x-1)/2 magic).
    */
    PageDescriptor (
        const css::uno::Reference<css::drawing::XDrawPage>& rxPage,
        SdPage* pPage,
        const sal_Int32 nIndex);

    ~PageDescriptor (void);

    /** Return the page that is represented by the descriptor as SdPage pointer .
    */
    SdPage* GetPage (void) const;

    /** Return the page that is represented by the descriptor as XDrawPage reference.
    */
    css::uno::Reference<css::drawing::XDrawPage> GetXDrawPage (void) const;

    /** Returns the index of the page as it is displayed in the view as page
        number.  The value may differ from the index returned by the
        XDrawPage when there are hidden slides and the XIndexAccess used to
        access the model filters them out.
    */
    sal_Int32 GetPageIndex (void) const;
    void SetPageIndex (const sal_Int32 nIndex);

    bool UpdateMasterPage (void);
    bool UpdateTransitionFlag (void);

    enum State { ST_Visible, ST_Selected, ST_WasSelected,
                 ST_Focused, ST_MouseOver, ST_Current, ST_Excluded };

    bool HasState (const State eState) const;

    bool SetState (const State eState, const bool bStateValue);

    /** Set the internal mbIsSelected flag to the selection state of the
        page.  Use this method to synchronize a page descriptor with the
        page it describes and determine whether a redraw to update the
        selection indicator is necessary.
        @return
            When the two selection states were different <TRUE/> is
            returned.  When they were the same this method returns
            <FALSE/>.
    */
    bool GetCoreSelection (void);

    /** Set the selection flags of the SdPage objects to the corresponding
        selection states of the page descriptors.
    */
    void SetCoreSelection (void);

    VisualState& GetVisualState (void);

    Rectangle GetBoundingBox (void) const;
    Point GetLocation (const bool bIgnoreLocation = false) const;
    void SetBoundingBox (const Rectangle& rBoundingBox);

private:
    SdPage* mpPage;
    css::uno::Reference<css::drawing::XDrawPage> mxPage;
    SdrPage const* mpMasterPage;

    /** This index is displayed as page number in the view.  It may or may
        not be the actual page index.
    */
    sal_Int32 mnIndex;

    Rectangle maBoundingBox;
    VisualState maVisualState;

    bool mbIsSelected : 1;
    bool mbWasSelected : 1;
    bool mbIsVisible : 1;
    bool mbIsFocused : 1;
    bool mbIsCurrent : 1;
    bool mbIsMouseOver : 1;
    bool mbHasTransition : 1;


    // Do not use the copy constructor operator.  It is not implemented.
    PageDescriptor (const PageDescriptor& rDescriptor);

    // Do not use the assignment operator.  It is not implemented
    // (mrPage can not be assigned).
    PageDescriptor& operator= (const PageDescriptor& rDescriptor);
};

} } } // end of namespace ::sd::slidesorter::model

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
