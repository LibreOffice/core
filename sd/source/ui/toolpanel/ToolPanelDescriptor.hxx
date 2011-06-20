/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_TOOL_PANEL_DESCRIPTOR_HXX
#define SD_TOOL_PANEL_DESCRIPTOR_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <memory>

class Window;

namespace sd { namespace toolpanel {


/** Collection of information the describes entries of the tool
    panel.  A descriptor owns the control it is associated with.
*/
class ToolPanelDescriptor
{
public:
    /** Create a new descriptor for the given control.
        @param pControl
    */
    ToolPanelDescriptor (::std::auto_ptr< ::Window> pControl,
        const String& rTitle);

    ~ToolPanelDescriptor (void);

    /** Return the height of the title bar.
        @return
            The title bar height is returned in pixels.
    */
    int GetTitleBarHeight (void) const;


    void SetPositionAndSize (const Point& aPosition,
        const Size& aSize);
    void SetPositionAndSize (const Rectangle& aBox);
    void SetWeight (double nWeight);

    Window* GetControl (void) const;
    const String& GetTitle (void) const;
    const Rectangle& GetTitleBox (void) const;
    Rectangle GetPositionAndSize (void) const;
    double GetWeight (void) const;

    int GetTotalHeight (void) const;
    int GetWindowHeight (void) const;

private:
    ::std::auto_ptr< ::Window> mpControl;
    String msTitle;
    Rectangle maTitleBox;
    double mnWeight;
    int mnTotalHeight;

    /// Do not use! Assignment operator is not supported.
    const ToolPanelDescriptor& operator= (
        const ToolPanelDescriptor& aDescriptor);
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
