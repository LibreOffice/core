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
#ifndef _SVX_ANCHORID_HXX
#define _SVX_ANCHORID_HXX


enum SvxAnchorIds
{
    SVX_OBJ_AT_CNTNT    = 0x01,     //Absatzgebundener Rahmen
    SVX_OBJ_IN_CNTNT    = 0x02,     //Zeichengebundener Rahmen
    SVX_OBJ_PAGE        = 0x04,     //Seitengebundener Rahmen
    SVX_OBJ_AT_FLY      = 0x08      //Rahmengebundener Rahmen
//  SVX_OBJ_AUTO_CNTNT  = 0x10      //Automatisch positionierter, absatzgebundener Rahmen
};



#endif
