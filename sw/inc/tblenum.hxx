/*************************************************************************
 *
 *  $RCSfile: tblenum.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _TBLENUM_HXX
#define _TBLENUM_HXX

//fuers aendern der Tabellen/Spalten/Zeilen Breiten/Hoehen
enum TblChgWidthHeightType
{
    WH_COL_LEFT,
    WH_COL_RIGHT,
    WH_ROW_TOP,
    WH_ROW_BOTTOM,
    WH_CELL_LEFT,
    WH_CELL_RIGHT,
    WH_CELL_TOP,
    WH_CELL_BOTTOM,


    // folgende koennen hinein verodert werden
    WH_FLAG_INSDEL  = 0x4000,           // Insert/Del-Modus: das Bigger-Flag
                                        // besagt was passiert:
                                        // bBigger -> Box wird entfernt
                                        // !bBigger-> Box wird eingefuegt
    WH_FLAG_BIGGER  = 0x8000            // Box wird groesser -> sonst kleiner
};

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
