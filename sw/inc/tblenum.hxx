/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
