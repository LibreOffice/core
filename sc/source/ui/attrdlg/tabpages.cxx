/*************************************************************************
 *
 *  $RCSfile: tabpages.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

#include "global.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include "attrdlg.hrc"
#include "tabpages.hxx"

// STATIC DATA -----------------------------------------------------------

static USHORT pProtectionRanges[] =
{
    SID_SCATTR_PROTECTION,
    SID_SCATTR_PROTECTION,
    0
};

SEG_EOFGLOBALS()

//========================================================================
// Zellschutz-Tabpage:
//========================================================================
#pragma SEG_FUNCDEF(tabpages_18)

ScTabPageProtection::ScTabPageProtection( Window*           pParent,
                                          const SfxItemSet& rCoreAttrs )
    :   SfxTabPage          ( pParent,
                              ScResId( RID_SCPAGE_PROTECTION ),
                              rCoreAttrs ),
        //
        aBtnProtect         ( this, ScResId( BTN_PROTECTED      ) ),
        aBtnHideCell        ( this, ScResId( BTN_HIDE_ALL       ) ),
        aBtnHideFormula     ( this, ScResId( BTN_HIDE_FORMULAR  ) ),
        aTxtHint            ( this, ScResId( FT_HINT            ) ),
        aGbProtect          ( this, ScResId( GB_PROTECTION      ) ),
        aBtnHidePrint       ( this, ScResId( BTN_HIDE_PRINT     ) ),
        aTxtHint2           ( this, ScResId( FT_HINT2           ) ),
        aGbPrint            ( this, ScResId( GB_PRINT           ) )
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    //  States werden in Reset gesetzt
    bTriEnabled = bDontCare = bProtect = bHideForm = bHideCell = bHidePrint = FALSE;

    aBtnProtect.SetClickHdl(     LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    aBtnHideCell.SetClickHdl(    LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    aBtnHideFormula.SetClickHdl( LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    aBtnHidePrint.SetClickHdl(   LINK( this, ScTabPageProtection, ButtonClickHdl ) );

    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_19)

__EXPORT ScTabPageProtection::~ScTabPageProtection()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_20)

USHORT* __EXPORT ScTabPageProtection::GetRanges()
{
    return pProtectionRanges;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_1a)

SfxTabPage* __EXPORT ScTabPageProtection::Create( Window*           pParent,
                                                  const SfxItemSet& rAttrSet )
{
    return ( new ScTabPageProtection( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_1c)

void __EXPORT ScTabPageProtection::Reset( const SfxItemSet& rCoreAttrs )
{
    //  Variablen initialisieren

    USHORT nWhich = GetWhich( SID_SCATTR_PROTECTION );
    const ScProtectionAttr* pProtAttr = NULL;
    SfxItemState eItemState = rCoreAttrs.GetItemState( nWhich, FALSE,
                                          (const SfxPoolItem**)&pProtAttr );

    // handelt es sich um ein Default-Item?
    if ( eItemState == SFX_ITEM_DEFAULT )
        pProtAttr = (const ScProtectionAttr*)&(rCoreAttrs.Get(nWhich));
    // bei SFX_ITEM_DONTCARE auf 0 lassen

    bTriEnabled = ( pProtAttr == NULL );                // TriState, wenn DontCare
    bDontCare = bTriEnabled;
    if (bTriEnabled)
    {
        //  Defaults, die erscheinen wenn ein TriState weggeklickt wird:
        //  (weil alles zusammen ein Attribut ist, kann auch nur alles zusammen
        //  auf DontCare stehen - #38543#)
        bProtect = TRUE;
        bHideForm = bHideCell = bHidePrint = FALSE;
    }
    else
    {
        bProtect = pProtAttr->GetProtection();
        bHideCell = pProtAttr->GetHideCell();
        bHideForm = pProtAttr->GetHideFormula();
        bHidePrint = pProtAttr->GetHidePrint();
    }

    //  Controls initialisieren

    aBtnProtect     .EnableTriState( bTriEnabled );
    aBtnHideCell    .EnableTriState( bTriEnabled );
    aBtnHideFormula .EnableTriState( bTriEnabled );
    aBtnHidePrint   .EnableTriState( bTriEnabled );

    UpdateButtons();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_1b)

BOOL __EXPORT ScTabPageProtection::FillItemSet( SfxItemSet& rCoreAttrs )
{
    BOOL                bAttrsChanged   = FALSE;
    USHORT              nWhich          = GetWhich( SID_SCATTR_PROTECTION );
    const SfxPoolItem*  pOldItem        = GetOldItem( rCoreAttrs, SID_SCATTR_PROTECTION );
    const SfxItemSet&   rOldSet         = GetItemSet();
    SfxItemState        eItemState      = rOldSet.GetItemState( nWhich, FALSE );
    ScProtectionAttr    aProtAttr;

    if ( !bDontCare )
    {
        aProtAttr.SetProtection( bProtect );
        aProtAttr.SetHideCell( bHideCell );
        aProtAttr.SetHideFormula( bHideForm );
        aProtAttr.SetHidePrint( bHidePrint );

        if ( bTriEnabled )
            bAttrsChanged = TRUE;                   // DontCare -> richtiger Wert
        else
            bAttrsChanged = !pOldItem || !( aProtAttr == *(const ScProtectionAttr*)pOldItem );
    }

    //--------------------------------------------------

    if ( bAttrsChanged )
        rCoreAttrs.Put( aProtAttr );
    else if ( eItemState == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    return bAttrsChanged;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_22)

int __EXPORT ScTabPageProtection::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_1d)

IMPL_LINK( ScTabPageProtection, ButtonClickHdl, TriStateBox*, pBox )
{
    TriState eState = pBox->GetState();
    if ( eState == STATE_DONTKNOW )
        bDontCare = TRUE;                           // alles zusammen auf DontCare
    else
    {
        bDontCare = FALSE;                          // DontCare ueberall aus
        BOOL bOn = ( eState == STATE_CHECK );       // ausgewaehlter Wert

        if ( pBox == &aBtnProtect )
            bProtect = bOn;
        else if ( pBox == &aBtnHideCell )
            bHideCell = bOn;
        else if ( pBox == &aBtnHideFormula )
            bHideForm = bOn;
        else if ( pBox == &aBtnHidePrint )
            bHidePrint = bOn;
        else
            DBG_ERRORFILE("falscher Button");
    }

    UpdateButtons();        // TriState und Enable-Logik

    return 0;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tabpages_23)

void ScTabPageProtection::UpdateButtons()
{
    if ( bDontCare )
    {
        aBtnProtect.SetState( STATE_DONTKNOW );
        aBtnHideCell.SetState( STATE_DONTKNOW );
        aBtnHideFormula.SetState( STATE_DONTKNOW );
        aBtnHidePrint.SetState( STATE_DONTKNOW );
    }
    else
    {
        aBtnProtect.SetState( bProtect ? STATE_CHECK : STATE_NOCHECK );
        aBtnHideCell.SetState( bHideCell ? STATE_CHECK : STATE_NOCHECK );
        aBtnHideFormula.SetState( bHideForm ? STATE_CHECK : STATE_NOCHECK );
        aBtnHidePrint.SetState( bHidePrint ? STATE_CHECK : STATE_NOCHECK );
    }

    BOOL bEnable = ( aBtnHideCell.GetState() != STATE_CHECK );
    {
        aBtnProtect.Enable( bEnable );
        aBtnHideFormula.Enable( bEnable );
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.73  2000/09/17 14:08:55  willem.vandorp
    OpenOffice header added.

    Revision 1.72  2000/08/31 16:38:19  willem.vandorp
    Header and footer replaced

    Revision 1.71  1997/12/05 19:05:16  ANK
    Includes geaendert


      Rev 1.70   05 Dec 1997 20:05:16   ANK
   Includes geaendert

      Rev 1.69   08 Apr 1997 18:53:42   NN
   #38543# DontCare fuer alle Flags zusammen ein/aus

      Rev 1.68   29 Oct 1996 14:04:38   NN
   ueberall ScResId statt ResId

      Rev 1.67   23 Jan 1996 11:53:46   MO
   GetOldItem() fuer Standard-Button

      Rev 1.66   04 Dec 1995 10:09:28   MO
   neuer Link

      Rev 1.65   28 Nov 1995 19:02:40   MO
   303a: HasExchangeSupport entfernt

      Rev 1.64   08 Nov 1995 13:04:38   MO
   301-Aenderungen

      Rev 1.63   12 Sep 1995 10:56:14   MO
   FixedText jetzt FixedInfo

      Rev 1.62   01 Aug 1995 20:23:04   MO
   Reset-Enable/Disable korrigiert

      Rev 1.61   28 Jul 1995 11:40:34   MO
   FillItemSet korrigiert

      Rev 1.60   24 Jul 1995 14:00:58   MO
   EXPORT

      Rev 1.59   05 Jul 1995 22:24:42   MO
   ExchangeSupport

      Rev 1.58   30 Jun 1995 17:56:00   HJS
   exports fuer create

      Rev 1.57   22 May 1995 17:33:12   MO
   Schuetzen/Ausgabe Gruppen

      Rev 1.56   22 May 1995 13:03:40   MO
   InfoText in BoldLight

      Rev 1.55   18 May 1995 14:13:24   MO
   Alignment Seite entfernt

      Rev 1.54   21 Apr 1995 18:48:40   MO
   ImageButtons

      Rev 1.53   15 Mar 1995 11:41:20   NN
   SaveValue-Aufruf am TriState-Button bei Alignment::Reset

      Rev 1.52   13 Mar 1995 16:27:08   MO
   * x10-Korrektur fuer MetricFields entfernt

      Rev 1.51   23 Feb 1995 08:27:44   KH
   Wer braucht "\" in Funktionsaufrufen ?

      Rev 1.50   07 Feb 1995 09:21:00   MO
   * Fix beim Normalize der Metrikfeldwerte bis CL es in SV fixed.

      Rev 1.49   04 Feb 1995 13:58:02   MO
   * Reset()-Handler aufgeraeumt
   * Suche, ob Einstellungen verloren gehen (noch nicht beendet!)

      Rev 1.48   27 Jan 1995 13:06:54   TRI
   __EXPORT bei virtuellen Methoden eingebaut

      Rev 1.47   27 Jan 1995 10:29:08   MO
   * NumberFormat-Seite entfernt (jetzt Svx-Page)
   + GetRanges()-Methode

      Rev 1.46   22 Jan 1995 15:09:14   SC
   2.37 Anpassung

      Rev 1.45   19 Jan 1995 15:19:24   TRI
   __EXPORT vor verschiedene LinkHandler gesetzt

      Rev 1.44   18 Jan 1995 13:07:42   TRI
   Pragmas zur Segementierung eingebaut

      Rev 1.43   18 Jan 1995 09:56:48   MO
   Bedingte Auswertung in ScTabPageNumber::Reset() durch if-else ersetzt

      Rev 1.42   10 Jan 1995 11:37:46   MO
   Erste verallgemeinerungs-faehige Version der Zahlenformat-Page.
   ScNumberInfoItem ersetzt ScDocumentItem und ScCellItem.
   Core wird nicht mehr ueber das Dokument aktualisiert,
   sondern die zu aktualisierenden Daten werden ueber das InfoItem
   wieder in den Set gesteckt.

      Rev 1.41   22 Dec 1994 15:52:20   MO
   switches haben jetzt alle ein default (Mac-Compiler)

      Rev 1.40   21 Dec 1994 17:56:50   MO
   SVX_ORIENTATION_STACKED aktiviert

      Rev 1.39   14 Dec 1994 13:50:30   MO
   Globale ResIds jetzt in sc.hrc

      Rev 1.38   07 Dec 1994 09:54:08   MO
   SfxMap -> SfxWhichMap (Sfx 233.b)

      Rev 1.37   21 Nov 1994 15:56:38   MO
   FontTabpage entfernt

      Rev 1.36   10 Nov 1994 20:45:02   MO
   EditHdl-Aufruf nur mit Edit-Objekt
   0 = UpdateOptions nicht ausfuehren

      Rev 1.35   10 Nov 1994 17:24:52   MO
   Zahlenformate nur bei Aenderungen schreiben

      Rev 1.34   09 Nov 1994 17:55:34   MO

      Rev 1.33   09 Nov 1994 11:16:04   MO
   Options-Update aus Format-String

      Rev 1.32   09 Nov 1994 09:27:18   MO
   Zeilenumbruch-Attribut wird jetzt wieder geschrieben

      Rev 1.31   08 Nov 1994 19:44:00   MO

      Rev 1.30   07 Nov 1994 19:17:18   MO

      Rev 1.28   07 Nov 1994 09:30:22   MO
   Zahlenformat in Arbeit

      Rev 1.27   03 Nov 1994 15:14:12   MO
   Fehler beim Init behoben

      Rev 1.26   02 Nov 1994 21:13:48   MO
   Zahlenformatseite ohne Hinzufuegen/Entfernen

      Rev 1.25   27 Oct 1994 14:35:04   MO
   Attribute in die Core und umgekehrt

      Rev 1.24   27 Oct 1994 10:47:32   MO
   Attribute aus der Core lesen

      Rev 1.23   25 Oct 1994 11:46:16   MO
   Umrandungs-Vorgaben eingeaut

      Rev 1.22   21 Oct 1994 17:03:30   MO
   Neuen FrameSelektor eingebaut

      Rev 1.21   19 Oct 1994 10:59:54   MO
   Voreinstellungen und Schatten anwaehlbar

      Rev 1.20   17 Oct 1994 17:31:46   MO

      Rev 1.19   17 Oct 1994 13:23:56   MO
   Neuer FrameSelektor

      Rev 1.18   14 Oct 1994 09:26:44   MO
   'Reanpassung' SV229

      Rev 1.17   13 Oct 1994 12:05:24   MO
   FramSelector in Umrandungs-Seite eingebaut

      Rev 1.16   12 Oct 1994 18:14:12   NN
   MarginAttr-Werte als Twips interpretiert

      Rev 1.15   06 Oct 1994 11:43:52   MO
   Anpassungen an Sfx 228g

      Rev 1.14   05 Oct 1994 19:59:34   MO
   BitmapRadioButtons fuer Ausrichtungs-Seite

      Rev 1.13   05 Oct 1994 12:23:44   MO
   Sfx-Anpassungen (228f)

      Rev 1.12   04 Oct 1994 15:56:58   MO
   Umstellung auf allgemeine Sfx-Loesung

      Rev 1.11   27 Sep 1994 16:10:58   MO

      Rev 1.9   22 Sep 1994 15:31:26   MO
   Neuer Zahlenformat-Entwurf

      Rev 1.8   20 Sep 1994 19:53:46   MO
   No change.

      Rev 1.7   20 Sep 1994 11:33:00   MO
   aFont-Problem gefixed

      Rev 1.6   19 Sep 1994 13:18:20   MO

      Rev 1.5   14 Sep 1994 12:18:22   MO

      Rev 1.4   13 Sep 1994 16:31:00   MO
   Zellschutzseite implementiert

      Rev 1.3   12 Sep 1994 10:05:20   MO
   Fontdialog arbeitet mit Fonthoehe in Twips

      Rev 1.2   09 Sep 1994 12:57:00   MO
   Hintergrund- und Zeichenseiten rudimentaer implementiert

      Rev 1.0   01 Sep 1994 12:45:24   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

