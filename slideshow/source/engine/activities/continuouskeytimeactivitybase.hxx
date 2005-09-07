/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: continuouskeytimeactivitybase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:36:20 $
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

#ifndef _SLIDESHOW_CONTINUOUSKEYTIMEACTIVITYBASE_HXX
#define _SLIDESHOW_CONTINUOUSKEYTIMEACTIVITYBASE_HXX

#include <simplecontinuousactivitybase.hxx>

#include <vector>


namespace presentation
{
    namespace internal
    {
        /** Interpolated, key-times animation.

            This class implements an interpolated key-times
            animation, with continuous time.
        */
        class ContinuousKeyTimeActivityBase : public SimpleContinuousActivityBase
        {
        public:
            ContinuousKeyTimeActivityBase( const ActivityParameters& rParms );

            /** Hook for derived classes

                This method will be called from perform(), already
                equipped with the modified time (nMinNumberOfFrames, repeat,
                acceleration and deceleration taken into account).

                @param nIndex
                Current index of the key times/key values.

                @param nFractionalIndex
                Fractional value from the [0,1] range, specifying
                the position between nIndex and nIndex+1.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( sal_uInt32    nIndex,
                                  double        nFractionalIndex,
                                  sal_uInt32    nRepeatCount ) const = 0;

            /// From SimpleContinuousActivityBase class
            virtual void simplePerform( double      nSimpleTime,
                                        sal_uInt32  nRepeatCount ) const;

        private:
            const ::std::vector< double >   maKeyTimes;

            /// last active index in maKeyTimes (to avoid frequent searching)
            mutable ::std::size_t           mnLastIndex;
        };
    }
}

#endif /* _SLIDESHOW_CONTINUOUSKEYTIMEACTIVITYBASE_HXX */
