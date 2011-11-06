/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
