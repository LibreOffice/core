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

#ifndef _SCH_GLOB_HXX
#define _SCH_GLOB_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

#define DIA_SLOW	0
#define DIA_MEDIUM	1
#define DIA_FAST	2


#define SCH_IF_APP              SFX_INTERFACE_LIB + 2
#define SCH_IF_CHARTVIEWSHELL   SFX_INTERFACE_LIB + 3
#define SCH_IF_DOCSHELL         SFX_INTERFACE_LIB + 4
#define SCH_IF_STANDARDOBJBAR   SFX_INTERFACE_LIB + 5

#define SCH_IF_SCHAPP SFX_INTERFACE_SCH_START + 0

// Inventor-Ids fuer StarChart

const UINT32 SchInventor = UINT32('S')*0x00000001+
                           UINT32('C')*0x00000100+
                           UINT32('H')*0x00010000+
                           UINT32('U')*0x01000000;

// Object-Ids fuer StarChart

const UINT16 SCH_OBJGROUP_ID		= 1;
const UINT16 SCH_OBJECTID_ID		= 2;
const UINT16 SCH_OBJECTADJUST_ID	= 3;
const UINT16 SCH_DATAROW_ID             = 4;
const UINT16 SCH_DATAPOINT_ID		= 5;
const UINT16 SCH_LIGHTFACTOR_ID		= 6;
const UINT16 SCH_AXIS_ID                = 7;

} //namespace binfilter
#endif // _SCH_GLOB_HXX


