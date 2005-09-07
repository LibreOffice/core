/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prioritybooster.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:05:28 $
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

#ifndef _CANVAS_PRIORITYBOOSTER_HXX
#define _CANVAS_PRIORITYBOOSTER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <memory>


namespace canvas
{
    namespace tools
    {
        struct PriorityBooster_Impl;

        /** Simplistic RAII object, to temporarily boost execution
            priority for the current scope.
         */
        class PriorityBooster
        {
        public:
            /** Create booster, with given priority delta

                @param nDelta
                Difference in execution priority. Positive values
                increase prio, negative values decrease prio.
             */
            explicit PriorityBooster( sal_Int32 nDelta );
            ~PriorityBooster();

        private:
            // also disables copy constructor and assignment operator
            const ::std::auto_ptr< PriorityBooster_Impl > mpImpl;
        };
    }
}

#endif /* _CANVAS_PRIORITYBOOSTER_HXX */
// eof
