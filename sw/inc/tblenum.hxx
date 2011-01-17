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

//fuers aendern der Tabellen/Spalten/Zeilen Breiten/Hoehen
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


    // folgende koennen hinein verodert werden
    const TblChgWidthHeightType WH_FLAG_INSDEL  = 0x4000;           // Insert/Del-Modus: das Bigger-Flag
                                                                    // besagt was passiert:
                                                                    // bBigger -> Box wird entfernt
                                                                    // !bBigger-> Box wird eingefuegt
    const TblChgWidthHeightType WH_FLAG_BIGGER  = 0x8000;           // Box wird groesser -> sonst kleiner
}

enum TblChgMode
{
    TBLFIX_CHGABS,          // Tabelle feste Breite, den Nachbar andern
    TBLFIX_CHGPROP,         // Tabelle feste Breite, alle Nachbarn aendern
    TBLVAR_CHGABS           // Tabelle variable, alle Nachbarn aendern
};


enum SplitTbl_HeadlineOption
{
    HEADLINE_NONE = 0,          // lasse alles wie es ist
    HEADLINE_BORDERCOPY,        // von der davor obere Line der vorhergehen Line
    HEADLINE_CNTNTCOPY,         // kopiere die 1. Line komplett mit Inhalt
    HEADLINE_BOXATTRCOPY,       // kopiere von der 1. Line die Box Attribute
    HEADLINE_BOXATRCOLLCOPY     // kopiere von der 1. Line die Box Attribute
                                // und die Absatzvorlagen
};

enum TableMergeErr
{
    TBLMERGE_OK,
    TBLMERGE_NOSELECTION,
    TBLMERGE_TOOCOMPLEX
};


#endif
