/*************************************************************************
 *
 *  $RCSfile: LayoutMenu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:33:38 $
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

#ifndef SD_TOOLPANEL_LAYOUT_MENU_HXX
#define SD_TOOLPANEL_LAYOUT_MENU_HXX

#include "TaskPaneTreeNode.hxx"

#include "glob.hxx"
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
#include <svtools/transfer.hxx>
#include <sfx2/shell.hxx>


class SfxModule;

namespace sd {

class DrawDocShell;
class PaneManagerEvent;
class ViewShellBase;

namespace toolpanel {


class LayoutMenu
    : public ValueSet,
      public TreeNode,
      public SfxShell,
      public DragSourceHelper,
      public DropTargetHelper
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDLAYOUTMENU);

    /** Create a new layout menu.  Depending on the given flag it
        displays its own scroll bar or lets a surrounding window
        handle that.
        @param rDocumentShell
            Used to determine writing direction.
        @param rViewShellBase
            Gives access to the view shell at whose active page the
            layout will be set.
        @param bUseOwnScrollBar
            When <TRUE/> then we will show our own scroll bar when not
            all icons can be displayed in the visible window area.
            When <FALSE/> then rely on an outer scroll bar.  In this
            case we will set the height of the window so that all
            icons are visible.
    */
    LayoutMenu (
        TreeNode* pParent,
        DrawDocShell& rDocumentShell,
        ViewShellBase& rViewShellBase,
        bool bUseOwnScrollBar);
    virtual ~LayoutMenu (void);

    /** Return the name of the currently selected layout.
    */
    String GetSelectedLayoutName (void);

    /** Return a numerical value representing the currently selected
        layout.
    */
    AutoLayout GetSelectedAutoLayout (void);


    // From ILayoutableWindow
    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual sal_Int32 GetMinimumWidth (void);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    // From ::Window
    virtual void Paint (const Rectangle& rRect);
    virtual void Resize (void);

    /** Show a context menu when the right mouse button is pressed.
    */
    virtual void MouseButtonDown (const MouseEvent& rEvent);

    void Execute (SfxRequest& rRequest);
    void GetState (SfxItemSet& rItemSet);

    // DragSourceHelper
    virtual void StartDrag (sal_Int8 nAction, const Point& rPosPixel);

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop (const AcceptDropEvent& rEvent);
    virtual sal_Int8 ExecuteDrop (const ExecuteDropEvent& rEvent);

private:
    ViewShellBase& mrBase;

    /** Do we use our own scroll bar or is viewport handling done by
        our parent?
    */
    bool mbUseOwnScrollBar;

    /** If we are asked for the preferred window size, then use this
        many columns for the calculation.
    */
    const int mnPreferredColumnCount;

    /** Depending on the given number of columns and the item size
        calculate the number of rows that are necessary to display all
        items.
    */
    int CalculateRowCount (const Size& rItemSize, int nColumnCount);
    void Fill (void);
    void Clear (void);

    /** Assign the given layout to all selected slides of a slide sorter.
        If no slide sorter is active then this call is ignored.  The slide
        sorter in the center pane is preferred if the choice exists.
    */
    void AssignLayoutToSelectedSlides (AutoLayout aLayout);

    /** Insert a new page with the given layout.  The page is inserted via
        the main view shell, i.e. its SID_INSERTPAGE slot is called.  It it
        does not support this slot then inserting a new page does not take
        place.  The new page is inserted after the currently active one (the
        one returned by ViewShell::GetActualPage().)
    */
    void InsertPageWithLayout (AutoLayout aLayout);

    SfxRequest CreateRequest (
        USHORT nSlotId,
        AutoLayout aLayout);

    /** When clicked then set the current page of the view in the center pane.
    */
    DECL_LINK(ClickHandler, ValueSet*);
    DECL_LINK(ViewShellChangeCallback, PaneManagerEvent*);
    DECL_LINK(RightClickHandler, MouseEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif
