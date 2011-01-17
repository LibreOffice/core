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
#ifndef _UITOOL_HXX
#define _UITOOL_HXX

#include <tools/wintypes.hxx>
#include <vcl/field.hxx>
#include <swtypes.hxx>
#include "swdllapi.h"

class MetricFormatter;
class SfxItemSet;
class SfxMedium;
class SwPageDesc;
class SvxTabStopItem;
class SwWrtShell;
class ListBox;
class SwDocShell;
class SwFrmFmt;
class SwTabCols;
class DateTime;
class SfxViewFrame;

// Umschalten einer Metric
SW_DLLPUBLIC void SetMetric(MetricFormatter& rCtrl, FieldUnit eUnit);

// BoxInfoAttribut fuellen
SW_DLLPUBLIC void PrepareBoxInfo(SfxItemSet& rSet, const SwWrtShell& rSh);

// SfxItemSets <-> PageDesc
void ItemSetToPageDesc( const SfxItemSet& rSet, SwPageDesc& rPageDesc );
void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet);

// Auffuellen der Tabs mit DefaultTabs
SW_DLLPUBLIC void   MakeDefTabs(SwTwips nDefDist, SvxTabStopItem& rTabs);

// DefaultTabs loeschen aus dem TabStopArray
//void  EraseDefTabs(SvxTabStopItem& rTabs);

// Abstand zwischen dem 1. und zweitem Element ermitteln
SW_DLLPUBLIC USHORT     GetTabDist(const SvxTabStopItem& rTabs);

// erfrage ob im Set eine Sfx-PageDesc-Kombination vorliegt
// und setze diesen im Set und loesche die Transport Items
// (PageBreak & PageModel) aus dem Set
void SwToSfxPageDescAttr( SfxItemSet& rSet );
void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet );

SW_DLLPUBLIC FieldUnit  GetDfltMetric(BOOL bWeb);
void        SetDfltMetric(FieldUnit eMetric, BOOL bWeb);

// ListBox mit allen Zeichenvorlagen fuellen - ausser Standard!
SW_DLLPUBLIC void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, BOOL bSorted = FALSE, BOOL bWithDefault = FALSE);

//inserts a string sorted into a ListBox,
SW_DLLPUBLIC USHORT InsertStringSorted(const String& rEntry, ListBox& rToFill, USHORT nOffset);

// Tabellenbreite und Ausrichtung ermitteln
SwTwips GetTableWidth( SwFrmFmt* pFmt, SwTabCols& rCols, USHORT *pPercent,
        SwWrtShell* pSh );

String GetAppLangDateTimeString( const DateTime& );

// search for a command string withing the menu structure and execute it
// at the dispatcher if there is one, if executed return true
bool ExecuteMenuCommand( PopupMenu& rMenu, SfxViewFrame& rViewFrame, USHORT nId );

#endif // _UITOOL_HXX
