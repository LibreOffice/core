/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namespaces.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:44 $
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

#ifndef OOX_CORE_NAMESPACES_HXX
#define OOX_CORE_NAMESPACES_HXX

#include <sal/types.h>

namespace oox { namespace core {

const sal_Int32 NMSP_EXCEL = (1 << 16);
const sal_Int32 NMSP_OFFICE = (2 << 16);
const sal_Int32 NMSP_PPT = (3 << 16);
const sal_Int32 NMSP_WORD = (4 << 16);
const sal_Int32 NMSP_VML = (5 << 16);
const sal_Int32 NMSP_DRAWINGML = (6 << 16);
const sal_Int32 NMSP_RELATIONSHIPS = (7 << 16);
const sal_Int32 NMSP_PACKAGE_RELATIONSHIPS = (8 << 16);
const sal_Int32 NMSP_XML = (9 << 16);
const sal_Int32 NMSP_CONTENT_TYPES = (10 << 16);
const sal_Int32 NMSP_POWERPOINT = (11 << 16);
const sal_Int32 NMSP_ACTIVATION = (12 << 16);
const sal_Int32 NMSP_DIAGRAM = (13 << 16);
const sal_Int32 NMSP_MASK = 0xffff << 16;
} }

#endif // OOX_CORE_NAMESPACES_HXX
