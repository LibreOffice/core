/*************************************************************************
 *
 *  $RCSfile: combtransition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:04:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_COMBTRANSITION_HXX
#define _SLIDESHOW_COMBTRANSITION_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <slidechangeanimation.hxx>
#include <slidebitmap.hxx>
#include <soundplayer.hxx>


namespace presentation
{
    namespace internal
    {
        /** Comb transition class.

            This class provides a SlideChangeAnimation, showing a
            comb-like effect (stripes of alternating push effects).
        */
        class CombTransition : public SlideChangeAnimation
        {
        public:
            /** Create the comb transition effect.

                @param rLeavingBitmap
                Bitmap of slide which leaves

                @param rEnteringBitmap
                Bitmap of slide which enters

                @param nNumStripes
                Number of comb-like stripes to show in this effect

                @param rSoundPlayer
                Sound to play, while transition is running. Use NULL
                for no sound.
             */
            CombTransition( const SlideBitmapSharedPtr& rLeavingBitmap,
                            const SlideBitmapSharedPtr& rEnteringBitmap,
                            const ::basegfx::B2DVector& rPushDirection,
                            sal_Int32                   nNumStripes,
                            const SoundPlayerSharedPtr& rSoundPlayer );

            // NumberAnimation
            virtual bool operator()( double x );
            virtual double getUnderlyingValue() const;

            // Animation
            virtual void start( const AnimatableShapeSharedPtr&,
                                const ShapeAttributeLayerSharedPtr& );
            virtual void end();

            // SlideChangeAnimation
            virtual void addView( const ViewSharedPtr& rView );
            virtual bool removeView( const ViewSharedPtr& rView );

        private:
            /** Query the size of the bitmaps in device pixel
             */
            ::basegfx::B2DSize getBitmapSize() const;

            ViewVector                      maViews;

            SlideBitmapSharedPtr            mpLeavingBitmap;
            SlideBitmapSharedPtr            mpEnteringBitmap;

            const ::basegfx::B2DSize        maBitmapSize;

            const ::basegfx::B2DPolyPolygon maClipPolygon1;
            const ::basegfx::B2DPolyPolygon maClipPolygon2;

            const ::basegfx::B2DVector      maPushDirection;

            SoundPlayerSharedPtr            mpSoundPlayer;
        };
    }
}

#endif /* _SLIDESHOW_COMBTRANSITION_HXX */
