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

#ifndef SC_OPTAB_H
#define SC_OPTAB_H

typedef void ( *OPCODE_FKT )( SvStream &aStream, sal_uInt16 nLaenge );

#define FKT_LIMIT   101

#define FKT_LIMIT123    101

#define LOTUS_EOF   0x01

#define LOTUS_FILEPASSWD 0x4b

#define LOTUS_PATTERN   0x284

#define LOTUS_FORMAT_INDEX 0x800

#define LOTUS_FORMAT_INFO 0x801

#define ROW_FORMAT_MARKER 0x106

#define COL_FORMAT_MARKER 0x107

#endif

