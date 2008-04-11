/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mapunit.hxx,v $
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

#ifndef _VCL_MAPUNIT_HXX
#define _VCL_MAPUNIT_HXX

// -----------------
// - MapMode-Types -
// -----------------

// By changes you must also change: rsc/vclrsc.hxx
enum MapUnit { MAP_100TH_MM, MAP_10TH_MM, MAP_MM, MAP_CM,
               MAP_1000TH_INCH, MAP_100TH_INCH, MAP_10TH_INCH, MAP_INCH,
               MAP_POINT, MAP_TWIP, MAP_PIXEL, MAP_SYSFONT, MAP_APPFONT,
               MAP_RELATIVE, MAP_REALAPPFONT, MAP_LASTENUMDUMMY };

#endif  // _VCL_MAPUNIT_HXX
