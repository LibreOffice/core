/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#define GRID_TO_WALL_DISTANCE (1.0)

const double    ZDIRECTION = 1.0;
const sal_Int32 AXIS2D_TICKLENGTH = 150;//value like in old chart
const sal_Int32 AXIS2D_TICKLABELSPACING = 100;//value like in old chart


}//end namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
