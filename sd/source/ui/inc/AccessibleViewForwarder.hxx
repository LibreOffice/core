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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX

#include <svx/IAccessibleViewForwarder.hxx>

class SdrPaintView;
class OutputDevice;

namespace accessibility {


/** <p>This class provides the means to transform between internal coordinates
    and screen coordinates without giving direct access to the underlying
    view.  It represents a certain window.  A call to
    <method>GetVisArea</method> returns the corresponding visible
    rectangle.</p>

    @attention
        Note, that modifications of the underlying view that lead to
        different transformations between internal and screen coordinates or
        change the validity of the forwarder have to be signaled seperately.
*/
class AccessibleViewForwarder
    :   public IAccessibleViewForwarder
{
public:
    //=====  internal  ========================================================

    AccessibleViewForwarder (SdrPaintView* pView, OutputDevice& rDevice);

    virtual ~AccessibleViewForwarder (void);

    //=====  IAccessibleViewforwarder  ========================================

    /** This method informs you about the state of the forwarder.  Do not
        use it when the returned value is <false/>.

        @return
            Return <true/> if the view forwarder is valid and <false/> else.
     */
    virtual sal_Bool IsValid (void) const;

    /** Returns the area of the underlying document that is visible in the
    * corresponding window.

        @return
            The rectangle of the visible part of the document.
     */
    virtual Rectangle GetVisibleArea() const;

    /** Transform the specified point from internal coordinates to an
        absolute screen position.

        @param rPoint
            Point in internal coordinates.

        @return
            The same point but in screen coordinates relative to the upper
            left corner of the (current) screen.
     */
    virtual Point LogicToPixel (const Point& rPoint) const;

    /** Transform the specified size from internal coordinates to a screen
    * position.

        @param rSize
            Size in internal coordinates.

        @return
            The same size but in screen coordinates.
     */
    virtual Size LogicToPixel (const Size& rSize) const;

    /** Transform the specified point from absolute screen coordinates to
        internal coordinates.

        @param rPoint
            Point in screen coordinates relative to the upper left corner of
            the (current) screen.

        @return
            The same point but in internal coordinates.
     */
    virtual Point PixelToLogic (const Point& rPoint) const;

    /** Transform the specified Size from screen coordinates to internal
        coordinates.

        @param rSize
            Size in screen coordinates.

        @return
            The same size but in internal coordinates.
     */
    virtual Size PixelToLogic (const Size& rSize) const;

protected:
    SdrPaintView* mpView;
    sal_uInt16 mnWindowId;
    OutputDevice& mrDevice;

private:
    AccessibleViewForwarder (AccessibleViewForwarder&);
    AccessibleViewForwarder& operator= (AccessibleViewForwarder&);
};

} // end of namespace accessibility

#endif
