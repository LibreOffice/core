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
namespace binfilter {

#ifndef _XMLENUMS_HXX_
#define _XMLENUMS_HXX_

enum XMLForbiddenCharactersEnum
{
    XML_FORBIDDEN_CHARACTER_LANGUAGE,
    XML_FORBIDDEN_CHARACTER_COUNTRY,
    XML_FORBIDDEN_CHARACTER_VARIANT,
    XML_FORBIDDEN_CHARACTER_BEGIN_LINE,
    XML_FORBIDDEN_CHARACTER_END_LINE,
    XML_FORBIDDEN_CHARACTER_MAX
};

enum XMLSymbolDescriptorsEnum
{
    XML_SYMBOL_DESCRIPTOR_NAME,
    XML_SYMBOL_DESCRIPTOR_EXPORT_NAME,
    XML_SYMBOL_DESCRIPTOR_SYMBOL_SET,
    XML_SYMBOL_DESCRIPTOR_CHARACTER,
    XML_SYMBOL_DESCRIPTOR_FONT_NAME,
    XML_SYMBOL_DESCRIPTOR_CHAR_SET,
    XML_SYMBOL_DESCRIPTOR_FAMILY,
    XML_SYMBOL_DESCRIPTOR_PITCH,
    XML_SYMBOL_DESCRIPTOR_WEIGHT,
    XML_SYMBOL_DESCRIPTOR_ITALIC,
    XML_SYMBOL_DESCRIPTOR_MAX
};
}//end of namespace binfilter
#endif
