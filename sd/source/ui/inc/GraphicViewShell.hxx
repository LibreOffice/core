/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicViewShell.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:34:31 $
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

#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#define SD_GRAPHIC_VIEW_SHELL_HXX

#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif

class Window;

namespace sd {

/** View shell of the Draw application.

    <p>This class is an example of how not to do it: specialization by
    inheritance.  A graphic view shell is similar to a draw view shell
    but lacks some of its features.  Thus is should be at most a base
    class of DrawViewShell.  There even is special case code in
    ViewShell that turns off some of the features for GraphicViewShell
    instances.</p>
*/
class GraphicViewShell
    : public DrawViewShell
{
public:
    TYPEINFO();

    SFX_DECL_VIEWFACTORY(GraphicViewShell);
    SFX_DECL_INTERFACE(SD_IF_SDGRAPHICVIEWSHELL)

    GraphicViewShell (
        SfxViewFrame* pFrame,
        ::Window* pParentWindow,
        const DrawViewShell& rShell);

    /** Create a new view shell for the Draw application.
        @param rViewShellBase
            The new object will be stacked on this view shell base.
        @param pFrameView
            The frame view that makes it possible to pass information from
            one view shell to the next.
    */
    GraphicViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView = NULL);

    virtual ~GraphicViewShell (void);

    /** This method is overloaded in order to have the layer mode allways
        active.
    */
    virtual void ChangeEditMode (EditMode eMode, bool bIsLayerModeActive);

protected:
    void ConstructGraphicViewShell (void);
    virtual void ArrangeGUIElements (void);

private:
    DECL_LINK(TabBarSplitHandler, TabBar*);
};

} // end of namespace sd

#endif
