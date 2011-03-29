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

#ifndef SD_SLIDESORTER_CURRENT_SLIDE_MANAGER_HXX
#define SD_SLIDESORTER_CURRENT_SLIDE_MANAGER_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>

class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace controller {

/** Manage the current slide.  This includes setting the according flags at
    the PageDescriptor objects and setting the current slide at the main
    view shell.

    Switching pages is triggered only after a little delay.  This allows
    fast travelling through a larger set of slides without having to wait
    for the edit view to update its content after every slide change.
*/
class CurrentSlideManager
{
public:
    /** Create a new CurrentSlideManager object that manages the current
        slide for the given SlideSorter.
    */
    CurrentSlideManager (SlideSorter& rSlideSorter);

    ~CurrentSlideManager (void);

    /** Call this when the current page of the main view shell has been
        switched.  Use SwitchCurrentSlide() to initiate such a switch.
    */
    void NotifyCurrentSlideChange (const sal_Int32 nSlideIndex);
    void NotifyCurrentSlideChange (const SdPage* pPage);

    /** Call this method to switch the current page of the main view shell
        to the given slide.  Use CurrentSlideHasChanged() when the current
        slide change has been initiated by someone else.
        @param nSlideIndex
            Zero based index in the range [0,number-of-slides).
        @param bUpdateSelection
            When <TRUE/> then the page selection is cleared and only the new
            current slide is selected.
    */
    void SwitchCurrentSlide (
        const sal_Int32 nSlideIndex,
        const bool bUpdateSelection = false);
    void SwitchCurrentSlide (
        const model::SharedPageDescriptor& rpSlide,
        const bool bUpdateSelection = false);

    /** Return the page descriptor for the current slide.  Note, that when
        there is no current slide then the returned pointer is empty.
    */
    model::SharedPageDescriptor GetCurrentSlide (void);

    /** Release all references to model data.
    */
    void PrepareModelChange (void);

    /** Modify inner state in reaction to a change of the SlideSorterModel.
    */
    void HandleModelChange (void);

private:
    SlideSorter& mrSlideSorter;
    sal_Int32 mnCurrentSlideIndex;
    model::SharedPageDescriptor mpCurrentSlide;
    /** Timer to control the delay after which to ask
        XController/ViewShellBase to switch to another slide.
    */
    Timer maSwitchPageDelayTimer;

    bool IsCurrentSlideIsValid (void);
    void SetCurrentSlideAtViewShellBase (const model::SharedPageDescriptor& rpSlide);
    void SetCurrentSlideAtTabControl (const model::SharedPageDescriptor& rpSlide);
    void SetCurrentSlideAtXController (const model::SharedPageDescriptor& rpSlide);

    /** When switching from one slide to a new current slide then this
        method releases all ties to the old slide.
    */
    void ReleaseCurrentSlide (void);

    /** When switching from one slide to a new current slide then this
        method connects to the new current slide.
    */
    void AcquireCurrentSlide (const sal_Int32 nSlideIndex);

    DECL_LINK(SwitchPageCallback,void*);
};


} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
