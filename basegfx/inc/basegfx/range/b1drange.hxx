/*************************************************************************
 *
 *  $RCSfile: b1drange.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:43 $
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

#ifndef _BGFX_RANGE_B1DRANGE_HXX
#define _BGFX_RANGE_B1DRANGE_HXX

#ifndef _BDRANGE_HXX
#include <BDRange.hxx>
#endif

namespace basegfx
{
    namespace range
    {
        class B1DRange
        {
            BDRange                                 maRange;

        public:
            B1DRange()
            {
            }

            B1DRange(double fStartValue)
            :   maRange(fStartValue)
            {
            }

            B1DRange(const B1DRange& rRange)
            :   maRange(rRange.maRange)
            {
            }

            sal_Bool isEmpty() const
            {
                return maRange.isEmpty();
            }

            void reset()
            {
                maRange.reset();
            }

            void operator=(const B1DRange& rRange)
            {
                maRange = rRange.maRange;
            }

            double getMinimum() const
            {
                return maRange.getMinimum();
            }

            double getMaximum() const
            {
                return maRange.getMaximum();
            }

            double getRange() const
            {
                return maRange.getRange();
            }

            sal_Bool isInside(double fValue) const
            {
                return maRange.isInside(fValue);
            }

            sal_Bool isInside(const B1DRange& rRange) const
            {
                return maRange.isInside(rRange.maRange);
            }

            void expand(double fValue)
            {
                maRange.expand(fValue);
            }

            void expand(const B1DRange& rRange)
            {
                maRange.expand(rRange.maRange);
            }
        };
    } // end of namespace range
} // end of namespace basegfx

#endif //   _BGFX_RANGE_B1DRANGE_HXX
