/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GetMetricVal.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:52:48 $
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
#ifndef _GETMETRICVAL_HXX
#define _GETMETRICVAL_HXX

#include <tools/solar.h>

#define CM_1  0         // 1 centimeter     or 1/2 inch
#define CM_05 1         // 0.5 centimeter   or 1/4 inch
#define CM_01 2         // 0.1 centimeter   or 1/20 inch

inline USHORT GetMetricVal( int n )
{
#ifdef USE_MEASUREMENT
    USHORT nVal = MEASURE_METRIC == GetAppLocaleData().getMeasurementSystemEnum()
                    ? 567       // 1 cm
                    : 770;      // 1/2 Inch
#else
    USHORT nVal = 567;      // 1 cm
#endif

    if( CM_01 == n )
        nVal /= 10;
    else if( CM_05 == n )
        nVal /= 2;
    return nVal;
}


#endif
