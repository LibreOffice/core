/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salcolorutils.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:14:47 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "salcolorutils.hxx"
#include "vcl/salbtype.hxx"

// =======================================================================

SalColor GetSalColor( const float* pQuartzColor )
{
    return MAKE_SALCOLOR( sal_uInt8( pQuartzColor[0] * 255.0), sal_uInt8(  pQuartzColor[1] * 255.0 ), sal_uInt8( pQuartzColor[2] * 255.0 ) );
}

void SetSalColor( const SalColor& rColor, float* pQuartzColor )
{
    pQuartzColor[0] = (float) SALCOLOR_RED(rColor) / 255.0;
    pQuartzColor[1] = (float) SALCOLOR_GREEN(rColor) / 255.0;
    pQuartzColor[2] = (float) SALCOLOR_BLUE(rColor) / 255.0;
    pQuartzColor[3] = 1.0;
}

// =======================================================================

