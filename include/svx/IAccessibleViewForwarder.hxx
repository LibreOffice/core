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

#ifndef INCLUDED_SVX_IACCESSIBLEVIEWFORWARDER_HXX
#define INCLUDED_SVX_IACCESSIBLEVIEWFORWARDER_HXX

#include <sal/types.h>
#include <tools/gen.hxx>


namespace accessibility {


/** <p>This interface provides the means to transform between internal
    coordinates in 100th of mm and screen coordinates without giving direct
    access to the underlying view.  Each view forwarder represents a
    specific real or virtual window.  A call to
    <method>GetVisibleArea</method> returns the visible rectangle that
    corresponds to this window.</p>

    <p>This interface is similar to the SvxViewForwarder but
    differs in two important points: Firstly the <member>GetVisArea</member>
    method returns a rectangle in internal coordinates and secondly the
    transformation methods do not require explicit mapmodes.  These have to
    be provided implicitly by the classes that implement this
    interface.  A third, less important, difference are the additional
    transformation methods for sizes.  The reasons for their existence are
    convenience and improved performance.</p>

    @attention
        Note, that modifications of the underlying view that lead to
        different transformations between internal and screen coordinates or
        change the validity of the forwarder have to be signaled separately.
*/
class IAccessibleViewForwarder
{
public:
    IAccessibleViewForwarder() = default;
    IAccessibleViewForwarder(IAccessibleViewForwarder const &) = default;
    IAccessibleViewForwarder(IAccessibleViewForwarder &&) = default;
    IAccessibleViewForwarder & operator =(IAccessibleViewForwarder const &) = default;
    IAccessibleViewForwarder & operator =(IAccessibleViewForwarder &&) = default;

    virtual ~IAccessibleViewForwarder(){};

    /** Returns the area of the underlying document that is visible in the
    * corresponding window.

        @return
            The rectangle of the visible part of the document. The values
            are, contrary to the base class, in internal coordinates of
            100th of mm.
     */
    virtual tools::Rectangle GetVisibleArea() const = 0;

    /** Transform the specified point from internal coordinates in 100th of
        mm to an absolute screen position.

        @param rPoint
            Point in internal coordinates (100th of mm).

        @return
            The same point but in screen coordinates relative to the upper
            left corner of the (current) screen.
     */
    virtual Point LogicToPixel (const Point& rPoint) const = 0;

    /** Transform the specified size from internal coordinates in 100th of
        mm to a screen oriented pixel size.

        @param rSize
            Size in internal coordinates (100th of mm).

        @return
            The same size but in screen coordinates.
     */
    virtual Size LogicToPixel (const Size& rSize) const = 0;

};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
