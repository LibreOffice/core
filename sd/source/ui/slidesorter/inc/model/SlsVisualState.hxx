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

#ifndef SD_SLIDESORTER_VISUAL_STATE_HXX
#define SD_SLIDESORTER_VISUAL_STATE_HXX

#include <sal/types.h>
#include <tools/gen.hxx>
#include <boost/function.hpp>

namespace sd { namespace slidesorter { namespace model {

class PageDescriptor;

/** This class gives access to values related to the visualization of page
    objects.  This includes animation state when blending from one state to
    another.
*/
class VisualState
{
public:
    enum State {
        VS_Selected,
        VS_Focused,
        VS_Current,
        VS_Excluded,
        VS_None };

    VisualState (const sal_Int32 nPageId);
    ~VisualState (void);

    State GetCurrentVisualState (void) const;
    State GetOldVisualState (void) const;
    void SetVisualState (const State eState);
    double GetVisualStateBlend (void) const;
    void SetVisualStateBlend (const double nBlend);

    void UpdateVisualState (const PageDescriptor& rDescriptor);

    void SetMouseOverState (const bool bIsMouseOver);

    sal_Int32 GetStateAnimationId (void) const;
    void SetStateAnimationId (const sal_Int32 nAnimationId);

    Point GetLocationOffset (void) const;
    bool SetLocationOffset (const Point& rPoint);
    sal_Int32 GetLocationAnimationId (void) const;
    void SetLocationAnimationId (const sal_Int32 nAnimationId);

    double GetButtonAlpha (void) const;
    void SetButtonAlpha (const double nAlpha);
    double GetButtonBarAlpha (void) const;
    void SetButtonBarAlpha (const double nAlpha);
    sal_Int32 GetButtonAlphaAnimationId (void) const;
    void SetButtonAlphaAnimationId (const sal_Int32 nAnimationId);

    sal_Int32 mnPageId; // For debugging

private:
    State meCurrentVisualState;
    State meOldVisualState;
    double mnVisualStateBlend;
    sal_Int32 mnStateAnimationId;
    bool mbOldMouseOverState;
    bool mbCurrentMouseOverState;

    Point maLocationOffset;
    sal_Int32 mnLocationAnimationId;

    double mnButtonAlpha;
    double mnButtonBarAlpha;
    sal_Int32 mnButtonAlphaAnimationId;
};

} } } // end of namespace ::sd::slidesorter::model

#endif
