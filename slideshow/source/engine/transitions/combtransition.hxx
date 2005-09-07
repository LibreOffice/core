/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: combtransition.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:51:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SLIDESHOW_COMBTRANSITION_HXX
#define _SLIDESHOW_COMBTRANSITION_HXX

#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "slidechangebase.hxx"

namespace presentation {
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
                    const SlideSharedPtr& pEnteringSlide,
                    const SoundPlayerSharedPtr& pSoundPlayer,
                    const ::basegfx::B2DVector& rPushDirection,
                    sal_Int32                   nNumStripes );

    // NumberAnimation
    virtual bool operator()( double x );

private:
    const ::basegfx::B2DVector maPushDirectionUnit;
    sal_Int32                  mnNumStripes;

    void renderComb( double t, UnoViewSharedPtr const & pView ) const;
};

} // namespace internal
} // namespace presentation

#endif /* _SLIDESHOW_COMBTRANSITION_HXX */
