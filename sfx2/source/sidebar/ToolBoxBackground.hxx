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
#ifndef SFX_SIDEBAR_TOOLBOX_BACKGROUND_HXX
#define SFX_SIDEBAR_TOOLBOX_BACKGROUND_HXX

#include "vcl/window.hxx"

#include <tools/svborder.hxx>


class ToolBox;

namespace sfx2 { namespace sidebar {

/// Draws the sidebar ToolBoxes (groups of toolbar buttons).
class ToolBoxBackground
    : public Window
{
public:
    ToolBoxBackground (
        Window* pParentWindow,
        const bool bShowBorder);
    virtual ~ToolBoxBackground (void);

    /** Call this method once to
        a) let the ToolBoxBackground object know which ToolBox to
           monitor and
        b) so that position and sizes can be set up.
        @return
            The relative position of the child.
    */
    Point SetToolBoxChild (
        ToolBox* pChild,
        long nX,
        long nY,
        long nWidth,
        long nHeight,
        sal_uInt16 nFlags);

    virtual void Paint (const Rectangle& rRect);
    virtual void DataChanged (const DataChangedEvent& rEvent);

private:
    SvBorder maPadding;

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
