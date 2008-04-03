/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneHider.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:10:08 $
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

#ifndef SD_PANE_HIDE_HXX
#define SD_PANE_HIDE_HXX

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATION_HPP_
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif


namespace sd {

class ViewShell;
class SlideshowImpl;

/** Hide the windows of the side panes and restore the original visibility
    later. Used by the in-window slide show in order to use the whole frame
    window for the show.
*/
class PaneHider
{
public:
    /** The constructor hides all side panes that belong to the
        ViewShellBase of the given view shell.
    */
    PaneHider (const ViewShell& rViewShell, SlideshowImpl* pSlideShow);

    /** Restore the original visibility of the side panes.
    */
    ~PaneHider (void);

private:
    const ViewShell& mrViewShell;
    /** Remember whether the visibility states of the windows of the  panes
        has been modified and have to be restored.
    */
    bool mbWindowVisibilitySaved;
    bool mbOriginalLeftPaneWindowVisibility;
    bool mbOriginalRightPaneWindowVisibility;

    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfiguration>
        mxConfiguration;
};

} // end of namespace sd

#endif
