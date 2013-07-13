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

#ifndef _CHART2_RELATIVEPOSITIONHELPER_HXX
#define _CHART2_RELATIVEPOSITIONHELPER_HXX

#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/drawing/Alignment.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS RelativePositionHelper
{
public:
    /** returns the upper left corner of an object that has size aObjectSize and
        where the point indicated by aAnchor has coordinates indicated by aPoint
        ( e.g. if aAnchor equals BOTTOM_LEFT, aPoint describes the
        coordinates of the bottom left corner of an object with size aObjectSize )
     */
    static ::com::sun::star::awt::Point
        getUpperLeftCornerOfAnchoredObject(
            ::com::sun::star::awt::Point aPoint,
            ::com::sun::star::awt::Size aObjectSize,
            ::com::sun::star::drawing::Alignment aAnchor );

    /** returns the center of an object that has size aObjectSize and
        where the point indicated by aAnchor has coordinates indicated by aPoint
        ( e.g. if aAnchor equals BOTTOM_LEFT, aPoint describes the
        coordinates of the bottom left corner of an object with size aObjectSize )
     */
    static ::com::sun::star::awt::Point
        getCenterOfAnchoredObject(
            ::com::sun::star::awt::Point aPoint,
            ::com::sun::star::awt::Size aUnrotatedObjectSize,
            ::com::sun::star::drawing::Alignment aAnchor,
            double fAnglePi );

    /** Returns a relative position that is the same point after the anchor has
        been changed to the given one.  The passed object size is taken into
        account for shifting the position.
     */
    SAL_DLLPRIVATE static ::com::sun::star::chart2::RelativePosition
        getReanchoredPosition(
            const ::com::sun::star::chart2::RelativePosition & rPosition,
            const ::com::sun::star::chart2::RelativeSize & rObjectSize,
            ::com::sun::star::drawing::Alignment aNewAnchor );

    /** grows a relative size about the given amount and shifts the given
        position such that the resize is relative to the former rectangle's
        center.

        @param bCheck If </sal_True>, the resize is only done, if after
               transformation, the position and size are within the bounds [0,1].

        @return </sal_True>, if changes were applied.

        <p>That means, if the position's alignment is center, the position will
        not change at all.</p>
     */
    static bool centerGrow(
        ::com::sun::star::chart2::RelativePosition & rInOutPosition,
        ::com::sun::star::chart2::RelativeSize & rInOutSize,
        double fAmountX, double fAmountY,
        bool bCheck = true );

    /** shifts a relative position about the given amount

        @param bCheck If </sal_True>, the shift is only done, if after
               transformation, the object represented by the position
               rInOutPosition and its size rObjectSize the position and size are
               within the bounds [0,1].

        @return </sal_True>, if changes were applied.
     */
    static bool moveObject(
        ::com::sun::star::chart2::RelativePosition & rInOutPosition,
        const ::com::sun::star::chart2::RelativeSize & rObjectSize,
        double fAmountX, double fAmountY,
        bool bCheck = true );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
