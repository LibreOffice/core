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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_COMBTRANSITION_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_COMBTRANSITION_HXX

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
    virtual bool operator()( double x ) override;

private:
    const ::basegfx::B2DVector maPushDirectionUnit;
    sal_Int32                  mnNumStripes;

    void renderComb( double t, const ViewEntry& rViewEntry ) const;
};

} // namespace internal
} // namespace presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_COMBTRANSITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
