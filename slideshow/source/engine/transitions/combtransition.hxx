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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
