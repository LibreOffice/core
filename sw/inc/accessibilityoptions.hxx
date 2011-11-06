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


#ifndef _ACCESSIBILITYOPTIONS_HXX
#define _ACCESSIBILITYOPTIONS_HXX

#include <tools/solar.h>

struct SwAccessibilityOptions
{
    sal_Bool bIsAlwaysAutoColor         :1;
    sal_Bool bIsStopAnimatedText        :1;
    sal_Bool bIsStopAnimatedGraphics    :1;

    SwAccessibilityOptions() :
        bIsAlwaysAutoColor(sal_False),
        bIsStopAnimatedText(sal_False),
        bIsStopAnimatedGraphics(sal_False) {}

    inline sal_Bool IsAlwaysAutoColor() const       { return bIsAlwaysAutoColor; }
    inline void SetAlwaysAutoColor( sal_Bool b )    { bIsAlwaysAutoColor = b; }

    inline sal_Bool IsStopAnimatedGraphics() const       { return bIsStopAnimatedText;}
    inline void SetStopAnimatedGraphics( sal_Bool b )    { bIsStopAnimatedText = b; }

    inline sal_Bool IsStopAnimatedText() const       { return bIsStopAnimatedGraphics; }
    inline void SetStopAnimatedText( sal_Bool b )    { bIsStopAnimatedGraphics = b;}
};
#endif

