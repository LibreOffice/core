/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TResultSetHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:39:58 $
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

#ifndef CONNECTIVITY_TRESULTSETHELPER_HXX
#define CONNECTIVITY_TRESULTSETHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace connectivity
{
    class SAL_NO_VTABLE IResultSetHelper
    {
    public:
        enum Movement
        {
            NEXT        = 0,
            PRIOR,
            FIRST,
            LAST,
            RELATIVE,
            ABSOLUTE,
            BOOKMARK
        };
    public:
        virtual sal_Bool move(Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData) = 0;
        virtual sal_Int32 getDriverPos() const = 0;
        virtual sal_Bool deletedVisible() const = 0;
        virtual sal_Bool isRowDeleted() const = 0;
    };
}

#endif // CONNECTIVITY_TRESULTSETHELPER_HXX

