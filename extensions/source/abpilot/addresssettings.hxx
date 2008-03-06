/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addresssettings.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:36:43 $
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

#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#define EXTENSIONS_ABP_ADDRESSSETTINGS_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef EXTENSIONS_ABP_ABPTYPES_HXX
#include "abptypes.hxx"
#endif

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

