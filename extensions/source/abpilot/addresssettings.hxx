/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: addresssettings.hxx,v $
 * $Revision: 1.11 $
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

#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#define EXTENSIONS_ABP_ADDRESSSETTINGS_HXX

#include <rtl/ustring.hxx>
#include "abptypes.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= AddressSourceType
    //=====================================================================
    enum AddressSourceType
    {
        AST_MORK,
        AST_THUNDERBIRD,
        AST_EVOLUTION,
        AST_EVOLUTION_GROUPWISE,
        AST_EVOLUTION_LDAP,
        AST_KAB,
        AST_MACAB,
        AST_LDAP,
        AST_OUTLOOK,
        AST_OE,

        AST_OTHER,

        AST_INVALID
    };

    //=====================================================================
    //= AddressSettings
    //=====================================================================
    struct AddressSettings
    {
        AddressSourceType   eType;
        ::rtl::OUString     sDataSourceName;
        ::rtl::OUString     sRegisteredDataSourceName;
        ::rtl::OUString     sSelectedTable;
        bool                bIgnoreNoTable;
        MapString2String    aFieldMapping;
        bool                bRegisterDataSource;
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ADDRESSSETTINGS_HXX

