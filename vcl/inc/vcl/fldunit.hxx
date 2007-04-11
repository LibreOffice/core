/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldunit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:53:42 $
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
