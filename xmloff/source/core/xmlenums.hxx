/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlenums.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:36:34 $
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
#endif
