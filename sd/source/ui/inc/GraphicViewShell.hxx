/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_GRAPHICVIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_GRAPHICVIEWSHELL_HXX

#include "DrawViewShell.hxx"

namespace vcl { class Window; }

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

    SFX_DECL_VIEWFACTORY(GraphicViewShell);
    SFX_DECL_INTERFACE(SD_IF_SDGRAPHICVIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
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
        vcl::Window* pParentWindow,
        FrameView* pFrameView = NULL);

    virtual ~GraphicViewShell();

    /** Override this method in order to have the layer mode always active.
    */
    virtual void ChangeEditMode (EditMode eMode, bool bIsLayerModeActive) override;

protected:
    void ConstructGraphicViewShell();
    virtual void ArrangeGUIElements() override;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
