/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TResultSetHelper.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONNECTIVITY_TRESULTSETHELPER_HXX
#define CONNECTIVITY_TRESULTSETHELPER_HXX

#include <sal/types.h>

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

