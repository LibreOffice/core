/*************************************************************************
 *
 *  $RCSfile: SlsFocusManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:19:35 $
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

#ifndef SD_SLIDESORTER_FOCUS_MANAGER_HXX
#define SD_SLIDESORTER_FOCUS_MANAGER_HXX

#include <sal/types.h>

namespace sd { namespace slidesorter { namespace model {
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

/** This class manages the focus of the slide sorter.  There is the focus
    page which is or is not focused.  Initialized to point to the first page
    it can be set to other pages by using the MoveFocus() method.  The
    focused state of the focus page can be toggled with the ToggleFocus()
    method.
*/
class FocusManager
{
public:
    /** Create a new focus manager that operates on the pages of the model
        associated with the given controller.  The focus page is set to the
        first page.  Focused state is off.
    */
    FocusManager (SlideSorterController& rController);

    ~FocusManager (void);

    enum FocusMoveDirection
    {
        FMD_NONE,
        FMD_LEFT,
        FMD_RIGHT,
        FMD_UP,
        FMD_DOWN
    };

    /** Move the focus from the currently focused page to one that is
        displayed adjacent to it, either vertically or horizontally.
        @param eDirection
            Direction in which to move the focus.  Wrap arround is done
            differently when moving vertically or horizontally.  Vertical
            wrap arround takes place in the same column, i.e. when you are
            in the top row and move up you come out in the bottom row in the
            same column.  Horizontal wrap arround moves to the next
            (FMD_RIGHT) or previous (FMD_LEFT) page.  Moving to the right
            from the last page goes to the first page and vice versa.
            When FMD_NONE is given, the current page index is checked for
            being valid.  If it is not, then it is set to the nearest valid
            page index.
    */
    void MoveFocus (FocusMoveDirection eDirection);

    /** Show the focus indicator of the current slide.
    */
    void ShowFocus (void);

    /** Hide the focus indicator.
    */
    void HideFocus (void);

    /** Toggle the focused state of the current slide.
        @return
            Returns the focused state of the focus page after the call.
    */
    bool ToggleFocus (void);

    /** Return whether the window managed by the called focus manager has
        the input focus of the application.
    */
    bool HasFocus (void) const;

    /** Return the descriptor of the page that currently has the focus.
        @return
            When there is no page that currently has the focus then NULL is
            returned.
    */
    model::PageDescriptor* GetFocusedPageDescriptor (void) const;

    /** Return the index of the page that currently has the focus as it is
        accepted by the slide sorter model.
        @return
            When there is no page that currently has the focus then -1 is
            returned.
    */
    sal_Int32 GetFocusedPageIndex (void) const;

    /** Return <TRUE/> when the focus inidcator is currently shown.  A
        prerequisite is that the window managed by this focus manager has
        the input focus as indicated by a <TRUE/> return value of
        HasFocus().  It is not necessary that the focus indicator is
        visible.  It may have been scrolled outside the visible area.
    */
    bool IsFocusShowing (void) const;

    /** Create an instance of this class to temporarily hide the focus
        indicator.  It is restored to its former visibility state when the
        FocusHider is destroyed.
    */
    class FocusHider
    {
    public:
        FocusHider (FocusManager&);
        ~FocusHider (void);
    private:
        bool mbFocusVisible;
        FocusManager& mrManager;
    };

private:
    /// The controller that is used for accessing the pages.
    SlideSorterController& mrController;

    /** Index of the page that may be focused.  It is -1 when the model
        contains no page.
    */
    sal_Int32 mnPageIndex;

    /** This flag indicates whether the page pointed to by mpFocusDescriptor
        has the focus.
    */
    bool mbPageIsFocused;

    /** Reset the focus state of the given descriptor and request a repaint
        so that the focus indicator is hidden.
        @param pDescriptor
            When NULL is given then the call is ignored.
    */
    void HideFocusIndicator (model::PageDescriptor* pDescriptor);

    /** Set the focus state of the given descriptor, scroll it into the
        visible area and request a repaint so that the focus indicator is
        made visible.
        @param pDescriptor
            When NULL is given then the call is ignored.
    */
    void ShowFocusIndicator (model::PageDescriptor* pDescriptor);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

