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

#ifndef _XMLOFF_XMLEMENT_HXX
#define _XMLOFF_XMLEMENT_HXX

#ifndef _SAL_TYPES_H 
#include <sal/types.h>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif
namespace binfilter {

/** Map an XMLTokenEnum to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...) 
 */
struct SvXMLEnumMapEntry
{
    ::binfilter::xmloff::token::XMLTokenEnum   eToken;
    sal_uInt16                      nValue;
};

#define ENUM_STRING_MAP_ENTRY(name,tok) { name, sizeof(name)-1, tok }

#define ENUM_STRING_MAP_END()           { NULL, 0, 0 }

/** Map a const sal_Char* (with length) to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...) 
 */
struct SvXMLEnumStringMapEntry
{
    const sal_Char *    pName;
    sal_Int32           nNameLength;
    sal_uInt16          nValue;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLEMENT_HXX

