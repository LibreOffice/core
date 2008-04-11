/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fntstyle.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _VCL_FNTSTYLE_HXX
#define _VCL_FNTSTYLE_HXX

#include <tools/solar.h>
#include <sal/types.h>

// --------------
// - Font enums -
// --------------

#ifndef ENUM_FONTRELIEF_DECLARED
#define ENUM_FONTRELIEF_DECLARED

enum FontRelief { RELIEF_NONE, RELIEF_EMBOSSED, RELIEF_ENGRAVED, FontRelief_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// --------------
// - Font types -
// --------------

typedef BYTE FontKerning;
#define KERNING_FONTSPECIFIC        ((FontKerning)0x01)
#define KERNING_ASIAN               ((FontKerning)0x02)

#endif  // _VCL_FNTSTYLE_HXX
