/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewDefines.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:46:20 $
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
#ifndef _CHART_COMMONDEFINES_HXX
#define _CHART_COMMONDEFINES_HXX

namespace chart
{

//=========================================================================
//
//=========================================================================
#define CHART_3DOBJECT_SEGMENTCOUNT ((sal_Int32)32)
#define FIXED_SIZE_FOR_3D_CHART_VOLUME (10000.0)
//There needs to be a little distance betweengrid lines and walls in 3D, otherwise the lines are partly hidden by the walls
#define GRID_TO_WALL_DISTANCE (20.0)
#define FLOOR_THICKNESS (100.0)

const double    ZDIRECTION = 1.0;
const sal_Int32 AXIS2D_TICKLENGTH = 150;//value like in old chart
const sal_Int32 AXIS2D_TICKLABELSPACING = 100;//value like in old chart


}//end namespace chart
#endif
