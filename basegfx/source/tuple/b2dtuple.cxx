/*************************************************************************
 *
 *  $RCSfile: b2dtuple.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:56 $
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

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

namespace basegfx
{
    namespace tuple
    {
        // initialize static member
        B2DTuple B2DTuple::maEmptyTuple(0.0, 0.0);

        sal_Bool B2DTuple::equalZero() const
        {
            return (this == &maEmptyTuple ||
                    (::basegfx::numeric::fTools::equalZero(mfX) && ::basegfx::numeric::fTools::equalZero(mfY)));
        }

        sal_Bool B2DTuple::equalZero(const double& rfSmallValue) const
        {
            return (this == &maEmptyTuple ||
                    (::basegfx::numeric::fTools::equalZero(mfX, rfSmallValue) && ::basegfx::numeric::fTools::equalZero(mfY, rfSmallValue)));
        }

        sal_Bool B2DTuple::equal(const B2DTuple& rTup) const
        {
            return (
                ::basegfx::numeric::fTools::equal(mfX, rTup.mfX) &&
                ::basegfx::numeric::fTools::equal(mfY, rTup.mfY));
        }

        sal_Bool B2DTuple::equal(const B2DTuple& rTup, const double& rfSmallValue) const
        {
            return (
                ::basegfx::numeric::fTools::equal(mfX, rTup.mfX, rfSmallValue) &&
                ::basegfx::numeric::fTools::equal(mfY, rTup.mfY, rfSmallValue));
        }

        void B2DTuple::correctValues(const double fCompareValue)
        {
            if(0.0 == fCompareValue)
            {
                if(::basegfx::numeric::fTools::equalZero(mfX))
                {
                    mfX = 0.0;
                }

                if(::basegfx::numeric::fTools::equalZero(mfY))
                {
                    mfY = 0.0;
                }
            }
            else
            {
                if(::basegfx::numeric::fTools::equal(mfX, fCompareValue))
                {
                    mfX = fCompareValue;
                }

                if(::basegfx::numeric::fTools::equal(mfY, fCompareValue))
                {
                    mfY = fCompareValue;
                }
            }
        }
    } // end of namespace tuple
} // end of namespace basegfx

// eof
