/*************************************************************************
 *
 *  $RCSfile: simplecontinuousactivitybase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:00:43 $
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

            virtual void start();

            virtual bool perform();

        protected:
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
