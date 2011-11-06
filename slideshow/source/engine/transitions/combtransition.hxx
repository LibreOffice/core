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



#ifndef INCLUDED_SLIDESHOW_COMBTRANSITION_HXX
#define INCLUDED_SLIDESHOW_COMBTRANSITION_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include "slidechangebase.hxx"

namespace slideshow {
namespace internal {

/** Comb transition class.

    This class provides a SlideChangeAnimation, showing a
    comb-like effect (stripes of alternating push effects).
*/
class CombTransition : public SlideChangeBase
{
public:
    /** Create the comb transition effect.

        @param nNumStripes
        Number of comb-like stripes to show in this effect
    */
    CombTransition( ::boost::optional<SlideSharedPtr> const & leavingSlide,
                    const SlideSharedPtr&                     pEnteringSlide,
                    const SoundPlayerSharedPtr&               pSoundPlayer,
                    const UnoViewContainer&                   rViewContainer,
                    ScreenUpdater&                            rScreenUpdater,
                    EventMultiplexer&                         rEventMultiplexer,
                    const ::basegfx::B2DVector&               rPushDirection,
                    sal_Int32                                 nNumStripes );

    // NumberAnimation
    virtual bool operator()( double x );

private:
    const ::basegfx::B2DVector maPushDirectionUnit;
    sal_Int32                  mnNumStripes;

    void renderComb( double t, const ViewEntry& rViewEntry ) const;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_COMBTRANSITION_HXX */
