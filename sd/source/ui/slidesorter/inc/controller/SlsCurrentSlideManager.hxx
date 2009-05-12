/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsCurrentSlideManager.hxx,v $
 *
 * $Revision: 1.3 $
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
#include <com/sun/star/drawing/XDrawPage.hpp>

class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace controller {

/** Manage the current slide.  This includes setting the according flags at
    the PageDescriptor objects and setting the current slide at the main
    view shell.
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
    void CurrentSlideHasChanged (const sal_Int32 nSlideIndex);

    /** Call this method to switch the current page of the main view shell
        to the given slide.  Use CurrentSlideHasChanged() when the current
        slide change has been initiated by someone else.
    */
    void SwitchCurrentSlide (const sal_Int32 nSlideIndex);
    void SwitchCurrentSlide (const model::SharedPageDescriptor& rpSlide);

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

    bool IsCurrentSlideIsValid (void);
    void SetCurrentSlideAtViewShellBase (const model::SharedPageDescriptor& rpSlide);
    void SetCurrentSlideAtXController (const model::SharedPageDescriptor& rpSlide);

    /** When switching from one slide to a new current slide then this
        method releases all ties to the old slide.
    */
    void ReleaseCurrentSlide (void);

    /** When switching from one slide to a new current slide then this
        method connects to the new current slide.
    */
    void AcquireCurrentSlide (const sal_Int32 nSlideIndex);
};


} } } // end of namespace ::sd::slidesorter::controller

#endif
