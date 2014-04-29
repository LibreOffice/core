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
SW_DLLPUBLIC sal_uInt16     GetTabDist(const SvxTabStopItem& rTabs);

// erfrage ob im Set eine Sfx-PageDesc-Kombination vorliegt
// und setze diesen im Set und loesche die Transport Items
// (PageBreak & PageModel) aus dem Set
void SwToSfxPageDescAttr( SfxItemSet& rSet );
void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet );

SW_DLLPUBLIC FieldUnit  GetDfltMetric(sal_Bool bWeb);
void        SetDfltMetric(FieldUnit eMetric, sal_Bool bWeb);

// ListBox mit allen Zeichenvorlagen fuellen - ausser Standard!
SW_DLLPUBLIC void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, sal_Bool bSorted = sal_False, sal_Bool bWithDefault = sal_False);

//inserts a string sorted into a ListBox,
SW_DLLPUBLIC sal_uInt16 InsertStringSorted(const String& rEntry, ListBox& rToFill, sal_uInt16 nOffset);

// Tabellenbreite und Ausrichtung ermitteln
SwTwips GetTableWidth( SwFrmFmt* pFmt, SwTabCols& rCols, sal_uInt16 *pPercent,
        SwWrtShell* pSh );

String GetAppLangDateTimeString( const DateTime& );

// search for a command string within the menu structure and execute it
// at the dispatcher if there is one, if executed return true
bool ExecuteMenuCommand( PopupMenu& rMenu, SfxViewFrame& rViewFrame, sal_uInt16 nId );

#endif // _UITOOL_HXX
