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
#ifndef SD_SLIDESORTER_SLOT_MANAGER_HXX
#define SD_SLIDESORTER_SLOT_MANAGER_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <tools/link.hxx>
#include <memory>
#include <queue>

class AbstractSvxNameDialog;
class SfxItemSet;
class SfxRequest;
class String;

namespace sd { namespace slidesorter {
class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace controller {

class Command;

/** This manager takes over the work of handling slot calls from the
    controller of the slide sorter.
*/
class SlotManager
{
public:
    /** Create a new slot manager that handles slot calls for the controller
        of a slide sorter.
        @param rController
            The controller for which to handle the slot calls.
    */
    SlotManager (SlideSorter& rSlideSorter);

    ~SlotManager (void);

    void FuTemporary (SfxRequest& rRequest);
    void FuPermanent (SfxRequest& rRequest);
    void FuSupport (SfxRequest& rRequest);
    void GetMenuState (SfxItemSet &rSet);
    void GetClipboardState (SfxItemSet &rSet);
    void GetStatusBarState (SfxItemSet& rSet);
    void ExecCtrl (SfxRequest& rRequest);
    void GetAttrState (SfxItemSet& rSet);

    void ExecuteCommandAsynchronously (::std::auto_ptr<Command> pCommand);

    /** Exclude or include one slide or all selected slides.
        @param rpDescriptor
            When the pointer is empty then apply the new state to all
            selected pages.  Otherwise apply the new state to just the
            specified state.
    */
    void ChangeSlideExclusionState (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bExcludeSlide);

    /** Call this after a change from normal mode to master mode or back.
        The affected slots are invalidated.
    */
    void NotifyEditModeChange (void);

private:
    /// The controller for which we manage the slot calls.
    SlideSorter& mrSlideSorter;

    typedef ::std::queue<Command*> CommandQueue;
    CommandQueue maCommandQueue;

    /** Called by FuTemporary to show the slide show.
    */
    void ShowSlideShow (SfxRequest& rRequest);

    /** The implementation is a copy of the code for SID_RENAMEPAGE in
        drviews2.cxx.
    */
    void RenameSlide (void);
    DECL_LINK(RenameSlideHdl, AbstractSvxNameDialog*);
    bool RenameSlideFromDrawViewShell( sal_uInt16 nPageId, const String& rName);

    /** Handle SID_INSERTPAGE slot calls.
    */
    void InsertSlide (SfxRequest& rRequest);

    void DuplicateSelectedSlides (SfxRequest& rRequest);

    /** Use one of several ways to determine where to insert a new page.
        This can be the current selection or the insertion indicator.
    */
    sal_Int32 GetInsertionPosition (void);

    DECL_LINK(UserEventCallback, void*);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

