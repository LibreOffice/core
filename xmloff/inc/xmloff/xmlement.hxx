/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlement.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:31:28 $
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

#ifndef _XMLOFF_XMLEMENT_HXX
#define _XMLOFF_XMLEMENT_HXX

#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

/** Map an XMLTokenEnum to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...)
 */
struct SvXMLEnumMapEntry
{
    ::xmloff::token::XMLTokenEnum   eToken;
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

#endif  //  _XMLOFF_XMLEMENT_HXX

