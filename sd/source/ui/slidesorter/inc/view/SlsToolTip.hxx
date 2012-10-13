/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_SLIDESORTER_VIEW_TOOL_TIP_HXX
#define SD_SLIDESORTER_VIEW_TOOL_TIP_HXX

#include "SlideSorter.hxx"
#include "model/SlsSharedPageDescriptor.hxx"

namespace sd { namespace slidesorter { namespace view {

/** Manage the display of tool tips.  The tool tip text changes when the
    mouse is moved from slide to slide or from button to button.
    After the mouse enters a slide the first display of the tool tip is
    delayed for a short time in order to not draw attention from the slide
    or its button bar.
*/
class ToolTip
{
public:
    ToolTip (SlideSorter& rSlideSorter);
    ~ToolTip (void);

    /** Set a new page.  This modifies the default help text.  After a page
        change a timer is started to delay the display of the tool tip for
        the new page.
        @param rpPage
            When this is empty then the tool tip is hidden.
    */
    void SetPage (const model::SharedPageDescriptor& rpPage);

    /** Hide the tool tip.
        @return
            Returns whether the tool tip was visible at the time this method
            was called.
    */
    bool Hide (void);

private:
    SlideSorter& mrSlideSorter;
    model::SharedPageDescriptor mpDescriptor;
    ::rtl::OUString msDefaultHelpText;
    ::rtl::OUString msCurrentHelpText;
    sal_uLong mnHelpWindowHandle;
    Timer maTimer;

    /** Request to show the tool tip.
        @param bForce
            When <TRUE/> then the tool tip is show right away.  Otherwise it
            is shown after a short delay.
    */
    void Show (const bool bForce);
    void DoShow (void);

    DECL_LINK(DelayTrigger, void*);
};


} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
