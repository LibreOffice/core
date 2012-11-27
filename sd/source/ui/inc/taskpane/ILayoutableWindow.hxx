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

protected:
    ~ILayouter() {}
};



/** This interface has to be implemented by windows that want to be
    layouted by a SubToolPanel or ScrollablePanel object.
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

    /** Return whether the window is generally resizable.  When used by
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
