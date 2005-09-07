/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: discreteactivitybase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:36:53 $
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

#ifndef _SLIDESHOW_DISCRETEACTIVITYBASE_HXX
#define _SLIDESHOW_DISCRETEACTIVITYBASE_HXX

#include <activitybase.hxx>
#include <wakeupevent.hxx>

#include <vector>


namespace presentation
{
    namespace internal
    {
        /** Specialization of ActivityBase for discrete time activities.

            A discrete time activity is one where time proceeds in
            discrete steps, i.e. at given time instants.
        */
        class DiscreteActivityBase : public ActivityBase
        {
        public:
            DiscreteActivityBase( const ActivityParameters& rParms );

            /** Hook for derived classes.

                This method is called for each discrete time
                instant, with nFrame denoting the frame number
                (starting with 0)

                @param nFrame
                Current frame number.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( sal_uInt32 nFrame, sal_uInt32 nRepeatCount ) const = 0;

            virtual void dispose();

            virtual bool perform();
            virtual void dequeued();

        protected:
            virtual void startAnimation();

            sal_uInt32 calcFrameIndex( sal_uInt32       nCurrCalls,
                                       ::std::size_t    nVectorSize ) const;

            sal_uInt32 calcRepeatCount( sal_uInt32      nCurrCalls,
                                        ::std::size_t   nVectorSize ) const;

            ::std::size_t getNumberOfKeyTimes() const { return maDiscreteTimes.size(); }

        private:
            WakeupEventSharedPtr            mpWakeupEvent;
            const ::std::vector< double >   maDiscreteTimes;
            const double                    mnSimpleDuration;
            sal_uInt32                      mnCurrPerformCalls;
        };
    }
}

#endif /* _SLIDESHOW_DISCRETEACTIVITYBASE_HXX */
