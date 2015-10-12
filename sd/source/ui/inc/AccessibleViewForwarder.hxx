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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEVIEWFORWARDER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEVIEWFORWARDER_HXX

#include <svx/IAccessibleViewForwarder.hxx>

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
        change the validity of the forwarder have to be signaled separately.
*/
class AccessibleViewForwarder
    :   public IAccessibleViewForwarder
{
public:
    //=====  internal  ========================================================

    AccessibleViewForwarder (SdrPaintView* pView, OutputDevice& rDevice);

    virtual ~AccessibleViewForwarder();

    //=====  IAccessibleViewforwarder  ========================================

    /** Returns the area of the underlying document that is visible in the
    * corresponding window.

        @return
            The rectangle of the visible part of the document.
     */
    virtual Rectangle GetVisibleArea() const SAL_OVERRIDE;

    /** Transform the specified point from internal coordinates to an
        absolute screen position.

        @param rPoint
            Point in internal coordinates.

        @return
            The same point but in screen coordinates relative to the upper
            left corner of the (current) screen.
     */
    virtual Point LogicToPixel (const Point& rPoint) const SAL_OVERRIDE;

    /** Transform the specified size from internal coordinates to a screen
    * position.

        @param rSize
            Size in internal coordinates.

        @return
            The same size but in screen coordinates.
     */
    virtual Size LogicToPixel (const Size& rSize) const SAL_OVERRIDE;

protected:
    SdrPaintView* mpView;
    sal_uInt16 mnWindowId;
    OutputDevice& mrDevice;

private:
    AccessibleViewForwarder (AccessibleViewForwarder&) = delete;
    AccessibleViewForwarder& operator= (AccessibleViewForwarder&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
