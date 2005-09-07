/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplecontinuousactivitybase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:37:59 $
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

#ifndef _SLIDESHOW_SIMPLECONTINUOUSACTIVITYBASE_HXX
#define _SLIDESHOW_SIMPLECONTINUOUSACTIVITYBASE_HXX

#include <activitybase.hxx>
#include <canvas/elapsedtime.hxx>

namespace presentation
{
    namespace internal
    {
        /** Simple, continuous animation.

            This class implements a simple, continuous animation
            without considering repeats or acceleration on the
            perform call. Only useful as a base class, you
            probably want to use ContinuousActivityBase.
        */
        class SimpleContinuousActivityBase : public ActivityBase
        {
        public:
            SimpleContinuousActivityBase( const ActivityParameters& rParms );

            virtual double calcTimeLag() const;

            virtual bool perform();
            virtual void dequeued();

        protected:
            /** Hook for derived classes

                This method will be called from perform().

                @param nSimpleTime
                Simple animation time, without repeat,
                acceleration or deceleration applied. This value
                is always in the [0,1] range, the repeat is
                accounted for with the nRepeatCount parameter.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void simplePerform( double nSimpleTime, sal_uInt32 nRepeatCount ) const = 0;

            virtual void startAnimation();

        private:
            /// Time elapsed since activity started
            ::canvas::tools::ElapsedTime    maTimer;

            /// Simple duration of activity
            const double                    mnMinSimpleDuration;

            /// Minimal number of frames to show (see ActivityParameters)
            const sal_uInt32                mnMinNumberOfFrames;

            /// Actual number of frames shown until now.
            sal_uInt32                      mnCurrPerformCalls;
        };
    }
}

#endif /* _SLIDESHOW_SIMPLECONTINUOUSACTIVITYBASE_HXX */
