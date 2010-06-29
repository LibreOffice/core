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

#ifndef SD_TASKPANE_COLOR_MENU_HXX
#define SD_TASKPANE_COLOR_MENU_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include <svtools/valueset.hxx>
#include <vcl/window.hxx>

#include <memory>


namespace sd { namespace toolpanel {

class ControlFactory;

#ifdef SHOW_COLOR_MENU

/** This demo menu shows the colors that are available from the
    StyleSettings.
*/
class ColorMenu
    : public ::Window,
      public TreeNode
{
public:
    ColorMenu (::Window* i_pParent);
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
#endif

} } // end of namespace ::sd::toolpanel

#endif
