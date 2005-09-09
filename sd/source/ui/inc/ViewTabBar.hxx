/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewTabBar.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:20:40 $
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

#ifndef SD_VIEW_TAB_BAR_HXX
#define SD_VIEW_TAB_BAR_HXX

#include <vcl/tabctrl.hxx>

namespace sd {

class ViewShellBase;
class PaneManagerEvent;

/** Tab control for switching between views in the center pane.
*/
class ViewTabBar
    : public TabControl
{
public:
    ViewTabBar (ViewShellBase& rViewShellBase, ::Window* pParent);
    virtual ~ViewTabBar (void);

    /** The returned value is the calculated as the difference between the
        total height of the control and the heigh of its first tab page.
        This can be considered a hack.
        This procedure works only when the control is visible.  Calling this
        method when the control is not visible results in returning a
        default value.
        To be on the safe side wait for this control to become visible and
        the call this method again.
    */
    int GetHeight (void);

protected:
    virtual void ActivatePage (void);
    virtual void Paint (const Rectangle& rPaintArea);

private:
    ViewShellBase& mrViewShellBase;

       DECL_LINK(ViewShellBaseEventHandler, PaneManagerEvent*);
};

} // end of namespace sd

#endif

