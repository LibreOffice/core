/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestMenu.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:43:59 $
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

#ifndef SD_TASKPANE_COLOR_MENU_HXX
#define SD_TASKPANE_COLOR_MENU_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
#include <vcl/window.hxx>

#include <memory>


namespace sd { namespace toolpanel {

class ControlFactory;

/** This demo menu shows the colors that are available from the
    StyleSettings.
*/
class ColorMenu
    : public ::Window,
      public TreeNode
{
public:
    ColorMenu (TreeNode* pParent);
    virtual ~ColorMenu (void);

    static ::std::auto_ptr<ControlFactory> CreateControlFactory (void);

    // From ILayoutableWindow
    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    // From ::Window
    virtual void Resize (void);

    using Window::GetWindow;

private:
    ValueSet maSet;
    int mnPreferredColumnCount;

    /** Depending on the given number of columns and the item size
        calculate the number of rows that are necessary to display all
        items.
    */
    int CalculateRowCount (const Size& rItemSize, int nColumnCount);
    void Fill (void);
};

} } // end of namespace ::sd::toolpanel

#endif
