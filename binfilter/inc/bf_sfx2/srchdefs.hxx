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

#ifndef _SFX_SRCHDEFS_HXX_
#define _SFX_SRCHDEFS_HXX_
namespace binfilter {

#define SEARCH_OPTIONS_SEARCH       ((sal_uInt16)0x0001)
#define SEARCH_OPTIONS_SEARCH_ALL   ((sal_uInt16)0x0002)
#define SEARCH_OPTIONS_REPLACE      ((sal_uInt16)0x0004)
#define SEARCH_OPTIONS_REPLACE_ALL  ((sal_uInt16)0x0008)
#define SEARCH_OPTIONS_WHOLE_WORDS  ((sal_uInt16)0x0010)
#define SEARCH_OPTIONS_BACKWARDS    ((sal_uInt16)0x0020)
#define SEARCH_OPTIONS_REG_EXP      ((sal_uInt16)0x0040)
#define SEARCH_OPTIONS_EXACT        ((sal_uInt16)0x0080)
#define SEARCH_OPTIONS_SELECTION    ((sal_uInt16)0x0100)
#define SEARCH_OPTIONS_FAMILIES     ((sal_uInt16)0x0200)
#define SEARCH_OPTIONS_FORMAT       ((sal_uInt16)0x0400)
#define SEARCH_OPTIONS_MORE			((sal_uInt16)0x0800)
#define SEARCH_OPTIONS_SIMILARITY	((sal_uInt16)0x1000)
#define SEARCH_OPTIONS_CONTENT		((sal_uInt16)0x2000)

}//end of namespace binfilter
#endif

