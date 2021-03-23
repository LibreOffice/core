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

#pragma once

#include <model/SlsSharedPageDescriptor.hxx>
#include <rtl/ustring.hxx>
#include <vcl/timer.hxx>

namespace sd::slidesorter
{
class SlideSorter;
}

namespace sd::slidesorter::view
{
/** Manage the display of tool tips.  The tool tip text changes when the
    mouse is moved from slide to slide or from button to button.
    After the mouse enters a slide the first display of the tool tip is
    delayed for a short time in order to not draw attention from the slide
    or its button bar.
*/
class ToolTip
{
public:
    ToolTip(SlideSorter& rSlideSorter);
    ~ToolTip();

    /** Set a new page.  This modifies the default help text.  After a page
        change a timer is started to delay the display of the tool tip for
        the new page.
        @param rpPage
            When this is empty then the tool tip is hidden.
    */
    void SetPage(const model::SharedPageDescriptor& rpPage);

    /** Hide the tool tip.
        @return
            Returns whether the tool tip was visible at the time this method
            was called.
    */
    bool Hide();

private:
    SlideSorter& mrSlideSorter;
    model::SharedPageDescriptor mpDescriptor;
    OUString msCurrentHelpText;
    void* mnHelpWindowHandle;
    Timer maShowTimer;
    Timer maHiddenTimer;

    void DoShow();

    DECL_LINK(DelayTrigger, Timer*, void);
};

} // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
