/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayoutMenu.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:46:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_TASKPANE_LAYOUT_MENU_HXX
#define SD_TASKPANE_LAYOUT_MENU_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#ifndef _COM_SUN_STAR_FRAME_XSTATUS_LISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

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
}


namespace sd { namespace tools {
class EventMultiplexerEvent;
} }


namespace sd { namespace toolpanel {

class ControlFactory;


class LayoutMenu
    : public ValueSet,
      public TreeNode,
      public SfxShell,
      public DragSourceHelper,
      public DropTargetHelper
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDLAYOUTMENU)

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

    static std::auto_ptr<ControlFactory> CreateControlFactory (
        ViewShellBase& rBase,
        DrawDocShell& rDocShell);

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

    /** Call this method when the set of displayed layouts is not up-to-date
        anymore.  It will re-assemple this set according to the current
        settings.
    */
    void InvalidateContent (void);

    // DragSourceHelper
    virtual void StartDrag (sal_Int8 nAction, const Point& rPosPixel);

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop (const AcceptDropEvent& rEvent);
    virtual sal_Int8 ExecuteDrop (const ExecuteDropEvent& rEvent);

    /** The context menu is requested over this Command() method.
    */
    virtual void Command (const CommandEvent& rEvent);

    /** Call Fill() when switching to or from high contrast mode so that the
        correct set of icons is displayed.
    */
    virtual void DataChanged (const DataChangedEvent& rEvent);

    using Window::GetWindow;
    using ValueSet::StartDrag;

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

    ::com::sun::star::uno::Reference<com::sun::star::frame::XStatusListener> mxListener;

    bool mbSelectionUpdatePending;

    bool mbIsMainViewChangePending;

    /** Calculate the number of displayed rows.  This depends on the given
        item size, the given number of columns, and the size of the
        control.  Note that this is not the number of rows managed by the
        valueset.  This number may be larger.  In that case a vertical
        scroll bar is displayed.
    */
    int CalculateRowCount (const Size& rItemSize, int nColumnCount);

    /** Fill the value set with the layouts that are applicable to the
        current main view shell.
    */
    void Fill (void);

    /** Remove all items from the value set.
    */
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

    /** Create a request structure that can be used with the SID_INSERTPAGE
        and SID_MODIFYPAGE slots.  The parameters are set so that the given
        layout is assigned to the current page of the main view shell.
        @param nSlotId
            Supported slots are SID_INSERTPAGE and SID_MODIFYPAGE.
        @param aLayout
            Layout of the page to insert or to assign.
    */
    SfxRequest CreateRequest (
        USHORT nSlotId,
        AutoLayout aLayout);

    /** Select the layout that is used by the current page.
    */
    void UpdateSelection (void);

    /** When clicked then set the current page of the view in the center pane.
    */
    DECL_LINK(ClickHandler, ValueSet*);
    DECL_LINK(RightClickHandler, MouseEvent*);
    DECL_LINK(StateChangeHandler, ::rtl::OUString*);
    DECL_LINK(EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif
