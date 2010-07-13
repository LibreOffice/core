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

#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#define SD_GRAPHIC_VIEW_SHELL_HXX

#include "DrawViewShell.hxx"

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
