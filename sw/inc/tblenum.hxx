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
#ifndef _TBLENUM_HXX
#define _TBLENUM_HXX


// For changing table columns/rows widths/heights.
typedef sal_uInt16 TblChgWidthHeightType;

namespace nsTblChgWidthHeightType
{
    const TblChgWidthHeightType WH_COL_LEFT = 0;
    const TblChgWidthHeightType WH_COL_RIGHT = 1;
    const TblChgWidthHeightType WH_ROW_TOP = 2;
    const TblChgWidthHeightType WH_ROW_BOTTOM = 3;
    const TblChgWidthHeightType WH_CELL_LEFT = 4;
    const TblChgWidthHeightType WH_CELL_RIGHT = 5;
    const TblChgWidthHeightType WH_CELL_TOP = 6;
    const TblChgWidthHeightType WH_CELL_BOTTOM = 7;


    // The following can "or"ed into.
    const TblChgWidthHeightType WH_FLAG_INSDEL  = 0x4000;           // Insert/Del-mode: the Bigger-Flag
                                                                    // tells what happens:
                                                                    // bBigger -> box gets removed.
                                                                    // !bBigger-> box gets inserted.
    const TblChgWidthHeightType WH_FLAG_BIGGER  = 0x8000;           // Box becomes larger -> else smaller.
}

enum TblChgMode
{
    TBLFIX_CHGABS,          // Table fixed width, change neighbour.
    TBLFIX_CHGPROP,         // Table fixed width, change all neighbours.
    TBLVAR_CHGABS           // Table variable, change all neighbours.
};


enum SplitTbl_HeadlineOption
{
    HEADLINE_NONE = 0,          // Leave everything in place.
    HEADLINE_BORDERCOPY,        // von der davor obere Line der vorhergehen Line
    HEADLINE_CNTNTCOPY,         // Copy 1st line with all contents.
    HEADLINE_BOXATTRCOPY,       // Copy box attributs of 1st line.
    HEADLINE_BOXATRCOLLCOPY     // Copy box attributes and paragraph styles of 1st line.
};

enum TableMergeErr
{
    TBLMERGE_OK,
    TBLMERGE_NOSELECTION,
    TBLMERGE_TOOCOMPLEX
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
