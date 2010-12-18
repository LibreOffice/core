/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef CONNECTIVITY_HTOOLS_HXX
#define CONNECTIVITY_HTOOLS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <rtl/ustrbuf.hxx>

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    //====================================================================
    //= HTools
    //====================================================================
    class HTools
    {
    public:
        /** appens a proper WHERE clause to the given buffer, which filters
            for a given table name

            @param _bShortForm
                <TRUE/> if the column names of the system table which is being asked
                have the short form (TABLE_CAT instead of TABLE_CATALOG, and so on)
        */
        static void appendTableFilterCrit(
            ::rtl::OUStringBuffer& _inout_rBuffer, const ::rtl::OUString& _rCatalog,
            const ::rtl::OUString _rSchema, const ::rtl::OUString _rName,
            bool _bShortForm
        );
    };

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

#endif // CONNECTIVITY_HTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
