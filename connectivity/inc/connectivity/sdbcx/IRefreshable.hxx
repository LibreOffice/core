/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IRefreshable.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#define _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_

#include "connectivity/CommonTools.hxx"

namespace connectivity
{
    namespace sdbcx
    {
        class SAL_NO_VTABLE IRefreshableGroups
        {
        public:
            virtual void refreshGroups() = 0;
        };

        class SAL_NO_VTABLE IRefreshableUsers
        {
        public:
            virtual void refreshUsers() = 0;
        };

        class SAL_NO_VTABLE IRefreshableColumns
        {
        public:
            virtual void refreshColumns()   = 0;
        };
    }
}
#endif //_CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_

