/*************************************************************************
 *
 *  $RCSfile: uitool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-14 09:57:44 $
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

#ifndef _UITOOL_HXX
#define _UITOOL_HXX

#ifndef _SV_WINTYPES_HXX //autogen
#include <vcl/wintypes.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

class MetricFormatter;
class SfxItemSet;
class SfxMedium;
class SwPageDesc;
class Printer;
class SvxTabStopItem;
class SwWrtShell;
class ListBox;
class SwDocShell;
class SwFrmFmt;
class SwTabCols;
class DateTime;

// Umschalten einer Metric
void SetMetric(MetricFormatter& rCtrl, FieldUnit eUnit);

// BoxInfoAttribut fuellen
void PrepareBoxInfo(SfxItemSet& rSet, const SwWrtShell& rSh);

// SfxItemSets <-> PageDesc
void ItemSetToPageDesc(const SfxItemSet& rSet,
                       SwPageDesc& rPageDesc,
                       SwWrtShell* pShell );

void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet);

// Auffuellen der Tabs mit DefaultTabs
void    MakeDefTabs(SwTwips nDefDist, SvxTabStopItem& rTabs);

// DefaultTabs loeschen aus dem TabStopArray
//void  EraseDefTabs(SvxTabStopItem& rTabs);

// Abstand zwischen dem 1. und zweitem Element ermitteln
USHORT  GetTabDist(const SvxTabStopItem& rTabs);

// erfrage ob im Set eine Sfx-PageDesc-Kombination vorliegt
// und setze diesen im Set und loesche die Transport Items
// (PageBreak & PageModel) aus dem Set
void SwToSfxPageDescAttr( SfxItemSet& rSet );
void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet );

FieldUnit   GetDfltMetric(BOOL bWeb);
void        SetDfltMetric(FieldUnit eMetric, BOOL bWeb);

// Umwandlung:  Sub(LibName.ModulName) -> LibName.ModulName.Sub
String ConvertMacroFormat(const String& rInput );

// besorge einen Filenamen/Filternamen. Wird ppMed uebergeben, wird das
// erzeugte Medium returnt. Auch das FilePassword kann returnt werden.
BOOL GetFileFilterNameDlg( Window& rParent, String& rFileName,
                            String* pFilePassword =0, String* pFilterName =0,
                            SfxMedium** ppMed = 0 );

// ListBox mit allen Zeichenvorlagen fuellen - ausser Standard!
void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, BOOL bSorted = FALSE);
//inserts a string sorted into a ListBox,
USHORT InsertStringSorted(const String& rEntry, ListBox& rToFill, BOOL bHasOffset, const International& rInt);

// Tabellenbreite und Ausrichtung ermitteln
SwTwips GetTableWidth( SwFrmFmt* pFmt, SwTabCols& rCols, USHORT *pPercent,
        SwWrtShell* pSh );

String GetAppLangDateTimeString( const DateTime& );

#endif // _UITOOL_HXX
