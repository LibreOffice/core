/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleViewForwarder.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:01:30 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_IVIEW_FORWARDER_HXX
#include <svx/IAccessibleViewForwarder.hxx>
#endif

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

    AccessibleViewForwarder (SdrPaintView* pView, USHORT nWindowId);
    AccessibleViewForwarder (SdrPaintView* pView, OutputDevice& rDevice);

    virtual ~AccessibleViewForwarder (void);

    void SetView (SdrPaintView* pView);

    //=====  IAccessibleViewforwarder  ========================================

    /** This method informs you about the state of the forwarder.  Do not
        use it when the returned value is <false/>.

        @return
            Return <true/> if the view forwarder is valid and <false/> else.
     */
    virtual BOOL IsValid (void) const;

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
    USHORT mnWindowId;
    OutputDevice& mrDevice;

private:
    AccessibleViewForwarder (AccessibleViewForwarder&);
    AccessibleViewForwarder& operator= (AccessibleViewForwarder&);
};

} // end of namespace accessibility

#endif
