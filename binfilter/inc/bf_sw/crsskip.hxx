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

#ifndef _CRSSKIP_HXX
#define _CRSSKIP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {


// define for cursor travelling normally in western text cells and chars do
// the same, but in complex text cell skip over legatures and char skip
// into it.
// These defines exist only to cut off the dependicies to I18N project.
const USHORT CRSR_SKIP_CHARS  =	0;
const USHORT CRSR_SKIP_CELLS  =	1;

} //namespace binfilter
#endif
