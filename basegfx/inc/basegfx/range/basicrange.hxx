/*************************************************************************
 *
 *  $RCSfile: basicrange.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-26 14:30:28 $
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

#ifndef _BGFX_RANGE_BASICRANGE_HXX
#define _BGFX_RANGE_BASICRANGE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _INC_FLOAT
#include <float.h>
#endif

#define START_MINIMUM_VALUE                     (DBL_MAX)
#define START_MAXIMUM_VALUE                     (DBL_MIN)

namespace basegfx
{
    namespace range
    {
        class BasicRange
        {
        protected:
            double                                      mfMinimum;
            double                                      mfMaximum;

        public:
            void reset()
            {
                mfMinimum = START_MINIMUM_VALUE;
                mfMaximum = START_MAXIMUM_VALUE;
            }

            sal_Bool isEmpty() const
            {
                return sal_Bool(START_MINIMUM_VALUE == mfMinimum && START_MAXIMUM_VALUE == mfMaximum);
            }

            double getMinimum() const { return mfMinimum; }
            double getMaximum() const { return mfMaximum; }

            double getRange() const
            {
                return (mfMaximum - mfMinimum);
            }

            double getCenter() const
            {
                return ((mfMaximum + mfMinimum) / 2.0);
            }

            sal_Bool isInside(double fValue) const
            {
                return sal_Bool((fValue >= mfMinimum) && (fValue <= mfMaximum));
            }

            sal_Bool isInside(const BasicRange& rRange) const
            {
                return sal_Bool((rRange.getMinimum() >= mfMinimum) && (rRange.getMaximum() <= mfMaximum));
            }

            sal_Bool overlaps(const BasicRange& rRange) const
            {
                return !sal_Bool((rRange.getMaximum() < mfMinimum) || (rRange.getMinimum() > mfMaximum));
            }

            BasicRange()
            :   mfMinimum(START_MINIMUM_VALUE),
                mfMaximum(START_MAXIMUM_VALUE)
            {
            }
            BasicRange(double fStartValue)
            :   mfMinimum(fStartValue),
                mfMaximum(fStartValue)
            {
            }
            BasicRange(const BasicRange& rRange)
            :   mfMinimum(rRange.getMinimum()),
                mfMaximum(rRange.getMaximum())
            {
            }

            void operator=(const BasicRange& rRange)
            {
                mfMinimum = rRange.getMinimum();
                mfMaximum = rRange.getMaximum();
            }

            void expand(double fValue)
            {
                if(fValue < mfMinimum)
                {
                    mfMinimum = fValue;
                }

                if(fValue > mfMaximum)
                {
                    mfMaximum = fValue;
                }
            }

            void expand(const BasicRange& rRange)
            {
                if(rRange.getMinimum() < mfMinimum)
                {
                    mfMinimum = rRange.getMinimum();
                }

                if(rRange.getMaximum() > mfMaximum)
                {
                    mfMaximum = rRange.getMaximum();
                }
            }
        };
    } // end of namespace range
} // end of namespace basegfx

#endif _BGFX_RANGE_BASICRANGE_HXX
