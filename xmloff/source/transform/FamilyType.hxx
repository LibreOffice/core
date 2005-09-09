/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FamilyType.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:43:37 $
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

#ifndef _XMLOFF_FAMILYTYPE_HXX
#define _XMLOFF_FAMILYTYPE_HXX

enum XMLFamilyType
{
    XML_FAMILY_TYPE_GRAPHIC,
    XML_FAMILY_TYPE_PRESENTATION,
    XML_FAMILY_TYPE_DRAWING_PAGE,
    XML_FAMILY_TYPE_MASTER_PAGE,
    XML_FAMILY_TYPE_PAGE_LAYOUT,
    XML_FAMILY_TYPE_HEADER_FOOTER,
    XML_FAMILY_TYPE_TEXT,
    XML_FAMILY_TYPE_PARAGRAPH,
    XML_FAMILY_TYPE_RUBY,
    XML_FAMILY_TYPE_SECTION,
    XML_FAMILY_TYPE_TABLE,
    XML_FAMILY_TYPE_TABLE_COLUMN,
    XML_FAMILY_TYPE_TABLE_ROW,
    XML_FAMILY_TYPE_TABLE_CELL,
    XML_FAMILY_TYPE_LIST,
    XML_FAMILY_TYPE_CHART,
    XML_FAMILY_TYPE_DATA,
    XML_FAMILY_TYPE_GRADIENT,
    XML_FAMILY_TYPE_HATCH,
    XML_FAMILY_TYPE_FILL_IMAGE,
    XML_FAMILY_TYPE_STROKE_DASH,
    XML_FAMILY_TYPE_MARKER,
    XML_FAMILY_TYPE_PRESENTATION_PAGE_LAYOUT,
    XML_FAMILY_TYPE_END
};

#endif  //  _XMLOFF_FAMILYTYPE_HXX

