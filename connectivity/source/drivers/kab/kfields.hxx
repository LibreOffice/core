/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: kfields.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _CONNECTIVITY_KAB_FIELDS_HXX_
#define _CONNECTIVITY_KAB_FIELDS_HXX_

#include <vcl/kde_headers.h>
#include <connectivity/dbexception.hxx>
#include <rtl/ustring.hxx>

#define KAB_FIELD_REVISION  0
#define KAB_DATA_FIELDS     1

namespace connectivity
{
    namespace kab
    {
        QString valueOfKabField(const ::KABC::Addressee &aAddressee, sal_Int32 nFieldNumber);
        sal_uInt32 findKabField(const ::rtl::OUString& columnName) throw(::com::sun::star::sdbc::SQLException);
    }
}

#endif
