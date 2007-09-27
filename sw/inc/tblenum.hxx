/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tblenum.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:13:13 $
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
#ifndef _TBLENUM_HXX
#define _TBLENUM_HXX

//fuers aendern der Tabellen/Spalten/Zeilen Breiten/Hoehen
typedef USHORT TblChgWidthHeightType;

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
