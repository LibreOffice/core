/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fldunit.hxx,v $
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

#ifndef _VCL_FLDUNIT_HXX
#define _VCL_FLDUNIT_HXX

// --------------
// - FieldTypes -
// --------------

// By changes you must also change: rsc/vclrsc.hxx
enum FieldUnit { FUNIT_NONE, FUNIT_MM, FUNIT_CM, FUNIT_M, FUNIT_KM,
                 FUNIT_TWIP, FUNIT_POINT, FUNIT_PICA,
                 FUNIT_INCH, FUNIT_FOOT, FUNIT_MILE, FUNIT_CUSTOM,
                 FUNIT_PERCENT, FUNIT_100TH_MM };

#endif // _VCL_FLDUNIT_HXX
