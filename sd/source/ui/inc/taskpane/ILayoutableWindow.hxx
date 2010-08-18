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

#ifndef SD_TOOLPANEL_I_LAYOUTABLE_WINDOW_HXX
#define SD_TOOLPANEL_I_LAYOUTABLE_WINDOW_HXX

#include <tools/gen.hxx>
#include <sal/types.h>

class Window;

namespace sd { namespace toolpanel {


class ILayouter
{
public:
    /** This method is called by layoutable controls when they have to be
        resized.
    */
    virtual void RequestResize (void) = 0;
};



/** This interface has to be implemented by windows that want to be
    layouted by a SubToolPanel or ScrollablePanel object.
*/
class ILayoutableWindow
{
public:
    virtual ~ILayoutableWindow (void) {};

    /** Return the preferred size without constraints on either the
        height or the width.
        The size the window will later be set to may but does not have
        to be equal to this size.
    */
    virtual Size GetPreferredSize (void) = 0;

    /** Return the preferred width with the constraint, that the
        window will be set to the given height.
        The width the window will later be set to may but does not have
        to be equal to this width.
    */
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight) = 0;

    /** Return the preferred height with the constraint, that the
        window will be set to the given width.
        The height the window will later be set to may but does not have
        to be equal to this height.
    */
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth) = 0;

    /** Return wether the window is generally resizable.  When used by
        a VerticalLayouter then the width will always be resized and
        the return value of this method determines whether the height
        may be modified as well.
    */
    virtual bool IsResizable (void) = 0;

    /** Return the window so that its size and position can be set.
        @return
            Returns the window which is layouted or NULL to indicate
            that the object is in an invalid state.
    */
    virtual ::Window* GetWindow (void) = 0;

    /** Return the minimal width of the window.
    */
    virtual sal_Int32 GetMinimumWidth (void) = 0;
};



} } // end of namespace ::sd::toolpanel

#endif
