/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: continuousactivitybase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:35:43 $
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

#ifndef _SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX
#define _SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX

#include <simplecontinuousactivitybase.hxx>

namespace presentation
{
    namespace internal
    {
        /** Simple, continuous animation.

            This class implements a simple, continuous
            animation. Only addition to ActivityBase class is an
            explicit animation duration and a minimal number of
            frames to display.
        */
        class ContinuousActivityBase : public SimpleContinuousActivityBase
        {
        public:
            ContinuousActivityBase( const ActivityParameters& rParms );

            /** Hook for derived classes

                This method will be called from perform(), already
                equipped with the modified time (nMinNumberOfFrames, repeat,
                acceleration and deceleration taken into account).

                @param nModifiedTime
                Already accelerated/decelerated and repeated time, always
                in the [0,1] range.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( double nModifiedTime, sal_uInt32 nRepeatCount ) const = 0;

            /// From SimpleContinuousActivityBase class
            virtual void simplePerform( double      nSimpleTime,
                                        sal_uInt32  nRepeatCount ) const;
        };
    }
}

#endif /* _SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX */
