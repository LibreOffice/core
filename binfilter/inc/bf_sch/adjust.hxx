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

#ifndef _SCH_ADJUST_HXX
#define _SCH_ADJUST_HXX

/************************************************************************/
namespace binfilter {

enum ChartAdjust
{
    CHADJUST_TOP_LEFT,
    CHADJUST_TOP_RIGHT,
    CHADJUST_TOP_CENTER,
    CHADJUST_CENTER_LEFT,
    CHADJUST_CENTER_RIGHT,
    CHADJUST_CENTER_CENTER,
    CHADJUST_BOTTOM_LEFT,
    CHADJUST_BOTTOM_CENTER,
    CHADJUST_BOTTOM_RIGHT
};

#define CHADJUST_COUNT	CHADJUST_BOTTOM_RIGHT + 1

} //namespace binfilter
#endif

