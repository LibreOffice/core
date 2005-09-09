/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ILayoutableWindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:01:44 $
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
    layouted by a ToolPanel, SubToolPanel, or ScrollablePanel object.
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
