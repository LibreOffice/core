/*************************************************************************
 *
 *  $RCSfile: w1filter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:57 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <tools/solar.h>
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>          // class SwSection
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _W1PAR_HXX
#include <w1par.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif

#define MAX_FIELDLEN 64000


///////////////////////////////////////////////////////////////////////
//
// hier stehen die methoden operator<<, Out, Start und Stop mit
// folgender Bedeutung: wenn moeglich wird die information aus dem
// dokument per
//   operator<<()
// in die shell uebertragen. sind jedoch weitere parameter noetig
// wurde der name
//   Out()
// gewaehlt. ist ein bereich zu kennzeichnen (zB bei attributen
// von/bis), heissen die methoden
//   Start(), Stop()
// alle diese methoden stehen in diesem modul, das fuer den filter,
// jedoch nicht fuer den dumper noetig ist.  und da alle regeln ihre
// ausnahme haben: hier stehen auch methoden, die aus anderen gruenden
// fuer den dumper sinnlos sind, zB wenn sie auf sv-strukturen beruhen
// wie zB GetFont() auf SvxFontItem.
//

/////////////////////////////////////////////////////////////// Manager
Ww1Shell& operator <<(Ww1Shell& rOut, Ww1Manager& This)
{
    // verhindern, das bei rekusivem aufruf dies mehrfach passiert:
    if (!This.Pushed())
    {
        { // der wird nur temporaer gebraucht:
            This.SetInStyle( TRUE );
            Ww1StyleSheet(This.aFib).Out(rOut, This);
            This.SetInStyle( FALSE );
        }
        { // dieser auch:
            Ww1Assoc(This.aFib).Out(rOut);
        }
          // dieser nicht, der ist bereits member:
        This.aDop.Out(rOut);
          // Jetzt entscheiden, wie Seitenvorlagen erzeugt werden
        if (This.GetSep().Count() <= 1)
            rOut.SetUseStdPageDesc();
    }
    // und jetzt das eigentliche dok:
    sal_Unicode cUnknown = ' ';
    while (*This.pSeek < This.pDoc->Count())
    {
    // ausgabe des ProgressState nur, wenn im haupttext, da sonst
    // nicht bestimmt werden kann, wie weit wir sind:
        if (!This.Pushed())
            ::SetProgressState(This.Where() * 100 / This.pDoc->Count(),
             rOut.GetDoc().GetDocShell());
    // hier werden abwechselnd die attribute und die zeichen in die
    // shell gepumpt.  die positionen werden durch das lesen der
    // zeichen aus dem manager hoch- gezaehlt.  erst alle attribute:
        This.Out(rOut, cUnknown);
    // das textdocument pDoc ist ein Ww1PlainText, dessen Out()
    // methode solange ausgibt, bis entweder ein sonderzeichen
    // auftaucht oder die anzahl der auszugebenden zeichen erreicht
    // ist:
        cUnknown = This.pDoc->Out(rOut, *This.pSeek);
    }
    This.SetStopAll(TRUE);
    This.OutStop(rOut, cUnknown);   // Damit die Attribute am Ende geschlossen
    This.SetStopAll(FALSE);         // werden
    return rOut;
}

void Ww1Manager::OutStop(Ww1Shell& rOut, sal_Unicode cUnknown)
{
                        // Bookmarks brauchen nicht beendet werden ???
    if (pFld)
        pFld->Stop(rOut, *this, cUnknown);
    if (!Pushed())
        aFtn.Stop(rOut, *this, cUnknown);
    if (1)
        aChp.Stop(rOut, *this, cUnknown);
    if (1)
        aPap.Stop(rOut, *this, cUnknown);
    if (!Pushed())
        aSep.Stop(rOut, *this, cUnknown);
}

void Ww1Manager::OutStart( Ww1Shell& rOut )
{
// alle attribute, die's brauchen beginnen
    if (!Pushed())
        aSep.Start(rOut, *this);
    if (1)
        aPap.Start(rOut, *this);
    if (1)
        aChp.Start(rOut, *this);
    if (!Pushed())
        aFtn.Start(rOut, *this);
    if (pFld)
        pFld->Start(rOut, *this);
    if (!Pushed())
        aBooks.Start(rOut, *this);
// bestimmen, wo das naechste Ereigniss ist:
    ULONG ulEnd = pDoc->Count(); // spaetestens am textende
    if (!Pushed())
        if (ulEnd > aSep.Where()) // naechster Sep vorher?
            ulEnd = aSep.Where();
    if (1)
        if (ulEnd > aPap.Where()) // naechster Pap vorher?
            ulEnd = aPap.Where();
    if (1)
        if (ulEnd > aChp.Where()) // naechster Chp vorher?
            ulEnd = aChp.Where();
    if (!Pushed())
        if (ulEnd > aFtn.Where()) // naechster Ftn vorher?
            ulEnd = aFtn.Where();
    if (pFld)
        if (ulEnd > pFld->Where()) // naechster Fld vorher?
            ulEnd = pFld->Where();
    if (!Pushed())
        if (ulEnd > aBooks.Where()) // naechster Bookmark vorher?
            ulEnd = aBooks.Where();
    *pSeek = Where(); // momentane position
    if (*pSeek < ulEnd) // sind wir bereits weiter?
        *pSeek = ulEnd;
}

void Ww1Manager::Out(Ww1Shell& rOut, sal_Unicode cUnknown)
{
// Je nach modus wird hier mit feldern, fusznoten, zeichenattributen,
// absatzatributen und sektionen wie folgt verfahren: von allen wird
// zuallererst die stop-methoden gerufen. stellt das objekt fest, dasz
// etwas zu beenden ist (natuerlich nicht im ersten durchgang) beendet
// es dies, ansonsten ist der aufruf wirkungslos.  dann werden
// unbehandelte sonderzeichen augegeben. das werden genauso alle
// start-methoden gerufen und danach per where festgestellt, an
// welcher stelle die naechste aktion zu erwarten ist.
//
// ist der manager in einem ge'push'ten mode, werden bestimmte
// elemente ausgeklammert. felder werden wiederum nur in besonderen
// faellen augeklammert, wenn naemlich bereiche ausgegeben werden, die
// keine felder enthalten koennen. charakterattribute und
// paragraphenattribute werden jedoch nie ausgeklammert. die if (1)
// wurden zur verdeutlichung der einheitlichkeit eingefuegt.

// Erstmal eine Sonderbehandlung fuer Tabellen:
// die wichtigen Attribute lassen sich am Besten vor Beendigung derselben
// abfragen.
// Optimierung: Sie werden nur auf sinnvolle Werte gesetzt, wenn
// das 0x07-Zeiche ansteht.

    BOOL bLIsTtp = FALSE;
    BOOL bLHasTtp = FALSE;
    if( cUnknown == 0x07 )
    {
        bLIsTtp = IsInTtp();
        bLHasTtp = HasTtp();
    }
#if 0
    ULONG uLlMan = Where();
    BOOL bLStop = IsStopAll();
    BOOL bLIsT = rOut.IsInTable();
    BOOL bLHasT = HasInTable();
    BOOL bLIsTtp = IsInTtp();
    BOOL bLHasTtp = HasTtp();
    BOOL bLNextHasTtp = NextHasTtp();
#endif

    OutStop( rOut, cUnknown );      // Attrs ggfs. beenden

// meta-zeichen interpretieren:
    if (!Ww1PlainText::IsChar(cUnknown))
        switch (cUnknown)
        {
        case 0x02:
            // dontknow
        break;
        case 0x07: // table
#if 0
{
    ULONG ulMan = Where();
    BOOL bStop = IsStopAll();
    BOOL bIsT = rOut.IsInTable();
    BOOL bHasT = HasInTable();
    BOOL bIsTtp = IsInTtp();
    BOOL bHasTtp = HasTtp();
    BOOL bNextHasTtp = NextHasTtp();
#endif
            if (rOut.IsInTable() && HasInTable() && !bLIsTtp && !bLHasTtp){
//          if (rOut.IsInTable() && HasInTable() && !IsInTtp() && !HasTtp()){
//          if (rOut.IsInTable() && HasInTable() && !HasTtp() && !NextHasTtp()){
//              rOut << 'X';
                rOut.NextTableCell();
            }else{
//              rOut << 'O';
            }
        break;
        case 0x09: // tab
            rOut.NextTab();
        break;
        case 0x0a: // linefeed
            rOut.NextParagraph();
        break;
        case 0x0b: // linebreak
            if (rOut.IsInTable())
//              rOut.NextBand();    // war Stuss
                ;
            else
                rOut.NextLine();
        break;
        case 0x0d: // carriage return
            // ignore
        break;
        case 0x0c: // pagebreak
            rOut.NextPage();
        break;
        case 0x14: // sectionendchar
            // ignore here
        break;
        default:
        {
#ifdef DEBUG
            char s[200];
            sprintf(s, "%s(%d) assert (Ww1PlainText::IsChar(c)) failed"
             ": unknown char 0x%02x", __FILE__, __LINE__, cUnknown);
            DBG_ERROR(s);
#endif
        }
        }

    OutStart( rOut );   // Attrs ggfs. starten und Naechste Pos berechnen
}

#if 0
void Ww1Manager::Out(Ww1Shell& rOut, BYTE cUnknown)
{
// Je nach modus wird hier mit feldern, fusznoten, zeichenattributen,
// absatzatributen und sektionen wie folgt verfahren: von allen wird
// zuallererst die stop-methoden gerufen. stellt das objekt fest, dasz
// etwas zu beenden ist (natuerlich nicht im ersten durchgang) beendet
// es dies, ansonsten ist der aufruf wirkungslos.  dann werden
// unbehandelte sonderzeichen augegeben. das werden genauso alle
// start-methoden gerufen und danach per where festgestellt, an
// welcher stelle die naechste aktion zu erwarten ist.
//
// ist der manager in einem ge'push'ten mode, werden bestimmte
// elemente ausgeklammert. felder werden wiederum nur in besonderen
// faellen augeklammert, wenn naemlich bereiche ausgegeben werden, die
// keine felder enthalten koennen. charakterattribute und
// paragraphenattribute werden jedoch nie ausgeklammert. die if (1)
// wurden zur verdeutlichung der einheitlichkeit eingefuegt.
    if (pFld)
        pFld->Stop(rOut, *this, cUnknown);
    if (!Pushed())
        aFtn.Stop(rOut, *this, cUnknown);
    if (1)
        aChp.Stop(rOut, *this, cUnknown);
    if (1)
        aPap.Stop(rOut, *this, cUnknown);
    if (!Pushed())
        aSep.Stop(rOut, *this, cUnknown);
// meta-zeichen interpretieren:
    if (!Ww1PlainText::IsChar(cUnknown))
        switch (cUnknown)
        {
        case 0x02:
            // dontknow
        break;
        case 0x07: // table
//  hier muesste etwa stehen:
// ich probiere es mal in den Sprm::Stop von InTable und Ttp
//          if (IsInTable()){
//              if(IsInDummyCell())
//                  if (NextInTable())
//                      rOut.NextTableRow();
//              else
//                  rOut.NextTableCell();
//          rOut.NextTableCell();
        break;
        case 0x09: // tab
            rOut.NextTab();
        break;
        case 0x0a: // linefeed
            rOut.NextParagraph();
        break;
        case 0x0b: // linebreak
            if (IsInTable())
//              rOut.NextBand();    // war Stuss
                ;
            else
                rOut.NextLine();
        break;
        case 0x0d: // carriage return
            // ignore
        break;
        case 0x0c: // pagebreak
            rOut.NextPage();
        break;
        case 0x14: // sectionendchar
            // ignore here
        break;
        default:
        {
#ifdef DEBUG
            char s[200];
            sprintf(s, "%s(%d) assert (Ww1PlainText::IsChar(c)) failed"
             ": unknown char 0x%02x", __FILE__, __LINE__, cUnknown);
            DBG_ERROR(s);
#endif
        }
        }
// alle attribute, die's brauchen beginnen
    if (!Pushed())
        aSep.Start(rOut, *this);
    if (1)
        aPap.Start(rOut, *this);
    if (1)
        aChp.Start(rOut, *this);
    if (!Pushed())
        aFtn.Start(rOut, *this);
    if (pFld)
        pFld->Start(rOut, *this);
// bestimmen, wo das naechste Ereigniss ist:
    ULONG ulEnd = pDoc->Count(); // spaetestens am textende
    if (!Pushed())
        if (ulEnd > aSep.Where()) // naechster Sep vorher?
            ulEnd = aSep.Where();
    if (1)
        if (ulEnd > aPap.Where()) // naechster Pap vorher?
            ulEnd = aPap.Where();
    if (1)
        if (ulEnd > aChp.Where()) // naechster Chp vorher?
            ulEnd = aChp.Where();
    if (!Pushed())
        if (ulEnd > aFtn.Where()) // naechster Ftn vorher?
            ulEnd = aFtn.Where();
    if (pFld)
        if (ulEnd > pFld->Where()) // naechster Fld vorher?
            ulEnd = pFld->Where();
    *pSeek = Where(); // momentane position
    if (*pSeek < ulEnd) // sind wir bereits weiter?
        *pSeek = ulEnd;
}
#endif  // 0

SvxFontItem Ww1Manager::GetFont(USHORT nFCode)
{
    return aFonts.GetFont(nFCode);
}

void Ww1Manager::Push0(Ww1PlainText* pDoc, ULONG ulSeek, Ww1Fields* pFld)
{
    DBG_ASSERT(!Pushed(), "Ww1Manager");
    this->pDoc = pDoc;
    pSeek = new ULONG;
    *pSeek = pDoc->Where();
    aPap.Push(ulSeek);
    aChp.Push(ulSeek);
    this->pFld = pFld;
}

// ulSeek ist der FC-Abstand zwischen Hauptest-Start und Sondertext-Start
// ulSeek2 ist der Offset dieses bestimmten Sondertextes im Sondertext-Bereich,
// also z.B. der Offset des speziellen K/F-Textes
void Ww1Manager::Push1(Ww1PlainText* pDoc, ULONG ulSeek, ULONG ulSeek2,
                       Ww1Fields* pFld)
{
    DBG_ASSERT(!Pushed(), "Ww1Manager");
    this->pDoc = pDoc;
    pSeek = new ULONG;
    *pSeek = pDoc->Where();
    aPap.Push(ulSeek + ulSeek2);
    aChp.Push(ulSeek + ulSeek2);
    if( pFld )
        pFld->Seek( ulSeek2 );
    this->pFld = pFld;
}

void Ww1Manager::Pop()
{
    DBG_ASSERT(Pushed(), "Ww1Manager");
    delete pDoc;
    pDoc = &aDoc;
    delete pSeek;
    pSeek = &ulDocSeek;
    aChp.Pop();
    aPap.Pop();
    delete pFld;
    pFld = &aFld;
}

///////////////////////////////////////////////////////////// Bookmarks

void Ww1Bookmarks::Out(Ww1Shell& rOut, Ww1Manager& rMan, USHORT)
{
#ifdef DEBUG
    const String & rN = GetName();
    long nHandle = GetHandle();
#endif

    if( GetIsEnd() ){
        rOut.SetBookEnd( GetHandle() );
        return;
    }
#if 0
    eBookStatus eB = pB->GetStatus();
    if( ( eB & BOOK_IGNORE ) != 0 )
        return 0;                               // Bookmark zu ignorieren
#endif

    const String & rName = GetName();
    if( rName.EqualsAscii( "_Toc", 0, 4 ) ) // "_Toc*" ist ueberfluessig
        return;

    if( rOut.IsFlagSet( SwFltControlStack::HYPO )
        && rName.EqualsIgnoreCaseAscii( "FORMULAR" ) )
        rOut.SetProtect();

            // Fuer UEbersetzung Bookmark -> Variable setzen
    long nLen = Len();
    if( nLen > MAX_FIELDLEN )
        nLen = MAX_FIELDLEN;
                                // Lese Inhalt des Bookmark
                                // geht vermulich auch ueber Ww1PlainText
    String aVal( rMan.GetText().GetText( Where(), nLen ) );

        // in 2 Schritten, da OS/2 zu doof ist
    SwFltBookmark aBook( rName, aVal, GetHandle(), FALSE );
    rOut << aBook;
//          ( eB & BOOK_ONLY_REF ) != 0, TRUE  );

//  rOut << GetName();  // Test
}

void Ww1Bookmarks::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
#ifdef DEBUG
    ULONG ulMan = rMan.Where();
    ULONG ulBook = Where();
#endif

    if (rMan.Where() >= Where())
    {
        Out(rOut, rMan);
        (*this)++;
    }
}

///////////////////////////////////////////////////////////// Footnotes
void Ww1Footnotes::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        DBG_ASSERT(nPlcIndex < Count(), "WwFootnotes");
        sal_Unicode c;
        rMan.Fill(c);
        DBG_ASSERT(c==0x02, "Ww1Footnotes");
        if (c==0x02)
        {
            Ww1FtnText* pText = new Ww1FtnText(rMan.GetFib());
        // beginn des textes dieser fusznote:
            ULONG start = aText.Where(nPlcIndex);
            pText->Seek(start);
        // laenge des textes
            ULONG count = aText.Where(nPlcIndex+1) - start;
            pText->SetCount(count);
        // fusznotenkennung sollte das erste byte sein
            pText->Out(c);
            DBG_ASSERT(c==0x02, "Ww1Footnotes");
            count--; // fuer das eben gelesene kenn-byte
        // fusznoten mode beginnen:
            rOut.BeginFootnote();
            bStarted = TRUE;
            rMan.Push0(pText, pText->Offset(rMan.GetFib()),
             new Ww1FootnoteFields(rMan.GetFib()));
            rOut << rMan;
            rMan.Pop();
            rOut.EndFootnote();
        }
        else
            (*this)++;
    }
}

void Ww1Footnotes::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode& c)
{
    if (bStarted && rMan.Where() > Where())
    {
        DBG_ASSERT(nPlcIndex < Count(), "Ww1Footnotes");
//      DBG_ASSERT(c==0x02, "Ww1Footnotes");    // scheint Stuss zu sein
        c = ' ';
        (*this)++;
    }
}

//////////////////////////////////////////////////////////////// Fields
void Ww1Fields::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where()){
        DBG_ASSERT(nPlcIndex < Count(), "Ww1Fields");
        if (GetData()->chGet() == 19)
            Out(rOut, rMan);
        else
            (*this)++; // ignore
    }
}

void Ww1Fields::Stop( Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode& c)
{
    if (rMan.Where() >= Where())
    {
        DBG_ASSERT(nPlcIndex < Count(), "Ww1Fields");
        if (GetData()->chGet() != 19)
        {
            rMan.Fill( c );
            DBG_ASSERT(c==21, "Ww1Fields");
            (*this)++;
            c = ' ';
            if (pField)
            // haben wir ein fertiges feld da, das  eingefuegt werden soll?
            {
                rOut << *pField;
                delete pField;
                pField = 0;
            // das macht der filter so, damit attribute die ueber das feld
            // gelten auch wirklich eingelesen werden und dem feld
            // zugeordnet werden.
            }
            if (sErgebnis.Len())
                rOut << sErgebnis;
        }
    }
}

enum WWDateTime{ WW_DONTKNOW = 0x0, WW_DATE = 0x1, WW_TIME = 0x2, WW_BOTH = 0x3 };

static WWDateTime GetTimeDatePara( const String& rForm,
                                    SwTimeFormat* pTime = 0,
                                     SwDateFormat* pDate = 0 )
{
    WWDateTime eDT = WW_BOTH;
    if( STRING_NOTFOUND == rForm.Search( 'H' ))         // H    -> 24h
    {
        if( pTime )
            *pTime = TF_SSMM_24;
    }
    else if( STRING_NOTFOUND == rForm.Search( 'H' ))    // h    -> 24h
    {
        if( pTime )
            *pTime = TF_SSMM_12;
    }
    else                                    // keine Zeit
    {
        eDT = (WWDateTime)( eDT & ~(USHORT)WW_TIME );
    }

    xub_StrLen nDPos = 0;
    while( STRING_NOTFOUND != nDPos )
    {
        nDPos = rForm.Search( 'M', nDPos );     // M    -> Datum
        if( !nDPos )
            break;
        sal_Unicode cPrev = rForm.GetChar( nDPos - 1 );
        // ignoriere dabei "AM", "aM", "PM", "pM"
        if( 'a' != cPrev && 'A' != cPrev && 'p' != cPrev && 'P' != cPrev )
            break;
        // else search again
        ++nDPos;
    }

    if( STRING_NOTFOUND != nDPos )                  // Monat -> Datum ?
    {
        static SwDateFormat __READONLY_DATA aDateA[32] =
        {
            DFF_DMY, DFF_DMMY, DFF_DMYY, DFF_DMMYY,
            DFF_DMMMY, DFF_DMMMY, DFF_DMMMYY, DFF_DMMMYY,
            DFF_DDMMY, DFF_DDMMY, DFF_DDMMMYY, DFF_DDMMMYY,
            DFF_DDMMMY, DFF_DDMMMY, DFF_DDMMMYY, DFF_DDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY,
            DFF_DDDMMMY, DFF_DDDMMMY, DFF_DDDMMMYY, DFF_DDDMMMYY
        };

        BOOL bHasDay = STRING_NOTFOUND != rForm.Search( 't' ) ||
                       STRING_NOTFOUND != rForm.Search( 'T' ) ||
                       STRING_NOTFOUND != rForm.Search( 'd' ) ||
                       STRING_NOTFOUND != rForm.Search( 'D' );

        BOOL bLongDayOfWeek= STRING_NOTFOUND != rForm.SearchAscii( "tttt" ) ||
                               STRING_NOTFOUND != rForm.SearchAscii( "TTTT" ) ||
                               STRING_NOTFOUND != rForm.SearchAscii( "dddd" ) ||
                               STRING_NOTFOUND != rForm.SearchAscii( "DDDD" );

        BOOL bDayOfWeek = STRING_NOTFOUND != rForm.SearchAscii( "ttt" ) ||
                          STRING_NOTFOUND != rForm.SearchAscii( "TTT" ) ||
                          STRING_NOTFOUND != rForm.SearchAscii( "ddd" ) ||
                          STRING_NOTFOUND != rForm.SearchAscii( "DDD" );

                    //  M, MM -> numeric month
                    //  MMM, MMMM -> text. month
        BOOL bLitMonth = STRING_NOTFOUND != rForm.SearchAscii( "MMM" );
                    //  MMMM -> full month
        BOOL bFullMonth = STRING_NOTFOUND != rForm.SearchAscii( "MMMM" );
                    //  jj, JJ -> 2-col-year
                    //  jjjj, JJJJ -> 4-col-year
        BOOL bFullYear = STRING_NOTFOUND != rForm.SearchAscii( "jjj" ) ||
                         STRING_NOTFOUND != rForm.SearchAscii( "JJJ" ) ||
                         STRING_NOTFOUND != rForm.SearchAscii( "yyy" ) ||
                         STRING_NOTFOUND != rForm.SearchAscii( "YYY" );

        USHORT i = ( bLitMonth & 1 )
                   | ( ( bFullYear & 1 ) << 1 )
                   | ( ( bFullMonth & 1 ) << 2 )
                   | ( ( bDayOfWeek & 1 ) << 3 )
                   | ( ( bLongDayOfWeek & 1 ) << 4 );
        if( pDate )
        {
            if( !bHasDay && !bFullMonth )
                *pDate = DFF_MY;
            else
                *pDate = aDateA[i];
        }
    }
    else
    {
        eDT = (WWDateTime)( eDT & ~(USHORT)WW_DATE );
    }
    return eDT;
}


extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                USHORT& rSubType, ULONG &rFmt,
                                USHORT nVersion );

void Ww1Fields::Out(Ww1Shell& rOut, Ww1Manager& rMan, USHORT nDepth)
{
    String sType; // der typ als string
    String sFormel; // die formel
    String sFormat;
    String sDTFormat;   // Datum / Zeit-Format
    W1_FLD* pData = GetData(); // die an den plc gebunden daten
    DBG_ASSERT(pData->chGet()==19, "Ww1Fields"); // sollte beginn sein

    sal_Unicode c;
    rMan.Fill( c );
    DBG_ASSERT(c==19, "Ww1Fields"); // sollte auch beginn sein
    if (pData->chGet()==19 && c == 19)
    {
        String aStr;
        c = rMan.Fill( aStr, GetLength() );
        DBG_ASSERT(Ww1PlainText::IsChar(c), "Ww1Fields");
        xub_StrLen pos = aStr.Search(' ');
        // get type out of text
        sType = aStr.Copy( 0, pos );
        aStr.Erase( 0, pos );
        if ( pos != STRING_NOTFOUND )
            aStr.Erase(0, 1);
        sFormel += aStr;
        BYTE rbType = pData->fltGet();
        do {
        // solange den formelteil einlesen, bis das feld entweder
        // zuende ist oder der ergebnisteil beginnt. dabei koennen
        // natuerlich neue felder beginnen (word unterstuetzt felder,
        // die wiederum felder beinhalten).
            (*this)++;
            pData = GetData();
            if (pData->chGet()==19) // nested field
            {
                Out(rOut, rMan, nDepth+1);
                rMan.Fill(c);
                DBG_ASSERT(c==21, "Ww1PlainText");
                sFormel.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "Ww" ));
                sFormel += String::CreateFromInt32( nPlcIndex );
                c = rMan.Fill(aStr, GetLength());
                DBG_ASSERT(Ww1PlainText::IsChar(c), "Ww1PlainText");
                sFormel += aStr;
            }
        }
        while (pData->chGet()==19);

        // get format out of text
        pos = sFormel.SearchAscii( "\\*" );
        sFormat = sFormel.Copy( pos );
        sFormel.Erase( pos );

        pos = sFormel.SearchAscii( "\\@" );
        sDTFormat = sFormel.Copy( pos );
        sFormel.Erase( pos );

        // der formelteil ist zuende, kommt ein ergebnisteil?
        if( pData->chGet() == 20 )
        {
            rMan.Fill( c );
            DBG_ASSERT(c==20, "Ww1PlainText");
            c = rMan.Fill(sErgebnis, GetLength());
            if (!Ww1PlainText::IsChar(c))
                sErgebnis += c; //~ mdt: sonderzeichenbenhandlung
            (*this)++;
            pData = GetData();
        }
        DBG_ASSERT(pData->chGet()==21, "Ww1PlainText");
        BOOL bKnown = TRUE;
        DBG_ASSERT(pField==0, "Ww1PlainText");
        if (pField != 0)
        {
            rOut << *pField;
            delete pField;
            pField = 0;
        }
// naja, aber info enthaelt alle moeglichkeiten, die auch direkt da sind
oncemore:
        switch (rbType)
        {
        case 3: // bookmark reference
            rOut.ConvertUStr( sFormel );
            pField = new SwGetRefField( (SwGetRefFieldType*)
                rOut.GetSysFldType( RES_GETREFFLD ),
                sFormel,
                REF_BOOKMARK,
                0,
                REF_CONTENT );
//          pField = new SwGetExpField((SwGetExpFieldType*)
//           rOut.GetSysFldType(RES_GETEXPFLD), sFormel, GSE_STRING);
//           ,
//           GSE_STRING, VVF_SYS);
        break;
        case 6: // set command
        {
            pos = aStr.Search(' ');
            String aName( aStr.Copy( 0, pos ));
            aStr.Erase(0, pos );
            aStr.Erase(0, 1);
            if( !aName.Len() )
                break;
            rOut.ConvertUStr( aName );
            SwFieldType* pFT = rOut.GetDoc().InsertFldType(
                SwSetExpFieldType( &rOut.GetDoc(), aName, GSE_STRING ) );
            pField = new SwSetExpField((SwSetExpFieldType*)pFT, aStr);
            ((SwSetExpField*)pField)->SetSubType(SUB_INVISIBLE);
// Invisible macht in 378 AErger, soll aber demnaechst gehen

            // das Ignorieren des Bookmarks ist nicht implementiert
        }
        break;
#if 0               // noch nicht fertig, daher disabled
        case 8: // create index
        case 13: // create table of contents
        {
            TOXTypes eTox;                              // Baue ToxBase zusammen
            switch( rbType ){
                case  8: eTox = TOX_INDEX; break;
                case 13: eTox = TOX_CONTENT; break;
                default: eTox = TOX_USER; break;
            }
            USHORT nCreateOf = ( eTox == TOX_CONTENT ) ? ( TOX_MARK | TOX_OUTLINELEVEL )
                                                    : TOX_MARK;
            BYTE nNum = 0;      // wird z.Zt. nicht ausgewertet
            const SwTOXType* pType = rOut.GetDoc().GetTOXType( eTox, nNum );
            SwForm aForm( eTox );
            SwTOXBase* pBase = new SwTOXBase( pType, aForm, nCreateOf, aEmptyStr );
                                        // Name des Verzeichnisses
            switch( eTox ){
            case TOX_INDEX:   pBase->SetOptions( TOI_SAME_ENTRY | TOI_FF | TOI_CASE_SENSITIVE );
                            break;
            case TOX_CONTENT: break;
            case TOX_USER:    break;
            }                                           // ToxBase fertig

            rOut.GetDoc().SetUpdateTOX( TRUE );         // Update fuer TOX anstossen
            rOut << SwFltTOX( pBase );
            rOut.EndItem(RES_FLTR_TOX);
        }
        break;
#endif
        case 14: // info var
        {
            pos = aStr.Search(' ');
            String aSubType( aStr.Copy( 0, pos ));
            aStr.Erase(0, pos );
            aStr.Erase(0, 1);
            rOut.ConvertUStr( aSubType );


            // ganz grosze schiete: der typ 'info' kann einem der
            // typen 15..31 entsprechen. er enthaelt als formel
            // das eingentliche feld der doc-info.
            // kein ';' benutzen mit folgendem macro:
#define IS(sd, se, t) \
    if (aSubType.EqualsAscii( sd ) || aSubType.EqualsAscii( se)) \
        rbType = t; \
    else

            // deutsche bez.     englische bez.    typ-code
            IS("titel",          "title",          15)
            IS("thema",          "subject",        16)
            IS("autor",          "author",         17)
            IS("stichw?rter",    "keywords",       18) //~ mdt: umlaut
            IS("kommentar",      "comment",        19)
            IS("gespeichertvon", "lastrevisedby",  20)
            IS("ertelldat",      "creationdate",   21)
            IS("speicherdat",    "revisiondate",   22)
            IS("druckdat",       "printdate",      23)
            IS("version",        "revisionnumber", 24)
            IS("zeit",           "edittime",       25)
            IS("anzseit",        "numberofpages",  26)
            IS("anzw?rter",      "numberofwords",  27) //~ mdt: umlaut
            IS("anzzeichen",     "numberofchars",  28)
            IS("dateiname",      "filename",       29)
            IS("vorlage",        "templatename",   30)
                bKnown = FALSE;
#undef IS
            if (rbType != 14)
                goto oncemore;
        }
        break;
        case 15: // title
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_TITEL, 0);
        break;
        case 16: // subject
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_THEMA, 0);
        break;
        case 17: // author
            pField = new SwAuthorField((SwAuthorFieldType*)
             rOut.GetSysFldType(RES_AUTHORFLD), AF_NAME);
        break;
        case 18: // keywords
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_KEYS, 0);
        break;
        case 19: // comments
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_COMMENT, 0);
        break;
        case 20: // last revised by
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD), DI_CHANGE|DI_SUB_AUTHOR);
        break;
        case 21: // creation date
        case 22: // revision date
        case 23: // print date
        case 25:{// edit time
                    USHORT nSub;
                    USHORT nReg = 0;    // RegInfoFormat, DefaultFormat fuer DocInfoFelder

                    switch( rbType )
                    {
                        case 21: nSub = DI_CREATE;  nReg = DI_SUB_DATE; break;
                        case 23: nSub = DI_PRINT;   nReg = DI_SUB_DATE; break;
                        case 22: nSub = DI_CHANGE;  nReg = DI_SUB_DATE; break;
                        case 25: nSub = DI_CHANGE;  nReg = DI_SUB_TIME; break;
                    }
                    switch( GetTimeDatePara( sDTFormat ) )
                    {
                        case WW_DATE: nReg = DI_SUB_DATE; break;
                        case WW_TIME: nReg = DI_SUB_TIME; break;
                        case WW_BOTH: nReg = DI_SUB_DATE; break;
                        // WW_DONTKNOW -> Default bereits gesetzt
                    }
                    pField = new SwDocInfoField((SwDocInfoFieldType*)
                        rOut.GetSysFldType(RES_DOCINFOFLD), nSub | nReg);
                }
        break;
        case 24: // revision number
            pField = new SwDocInfoField((SwDocInfoFieldType*)
             rOut.GetSysFldType(RES_DOCINFOFLD),  DI_DOCNO, 0);
        break;
        case 26: // number of pages
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_PAGE, SVX_NUM_ARABIC);
        break;
        case 27: // number of words
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_WORD, SVX_NUM_ARABIC);
        break;
        case 28: // number of chars
            pField = new SwDocStatField((SwDocStatFieldType*)
             rOut.GetSysFldType(RES_DOCSTATFLD), DS_CHAR, SVX_NUM_ARABIC);
        break;
        case 29: // file name
            pField = new SwFileNameField((SwFileNameFieldType*)
             rOut.GetSysFldType(RES_FILENAMEFLD));
        break;
        case 30: // doc template name
            pField = new SwTemplNameField((SwTemplNameFieldType*)
             rOut.GetSysFldType(RES_TEMPLNAMEFLD), FF_NAME);
        break;
        case 31:
        case 32:{
                    SwDateFormat aDate = DF_SSYS;
                    SwTimeFormat aTime = TF_SYSTEM;

                    WWDateTime eDT = GetTimeDatePara(sDTFormat, &aTime, &aDate);
                    if( eDT == WW_DONTKNOW )        // kein D/T-Formatstring
                        eDT = ( rbType == 32 ) ? WW_TIME : WW_DATE;    // benutze ID

                    if( eDT & WW_DATE )
                    {
                        USHORT nWhich = RES_DATEFLD;
                        USHORT nSubType = DATEFLD;
                        ULONG nFormat = aDate;
                        sw3io_ConvertFromOldField( rOut.GetDoc(),
                            nWhich, nSubType, nFormat, 0x0110 );
                        pField = new SwDateTimeField((SwDateTimeFieldType*)
                            rOut.GetSysFldType(RES_DATETIMEFLD), DATEFLD, nFormat);

                        if( eDT == WW_BOTH )
                            rOut << * pField << ' ';
                                // Mogel: direkt einfuegen und Space dahinter
                    }
                    if( eDT & WW_TIME )
                    {
                        USHORT nWhich = RES_TIMEFLD;
                        USHORT nSubType = TIMEFLD;
                        ULONG nFormat = aTime;
                        sw3io_ConvertFromOldField( rOut.GetDoc(),
                            nWhich, nSubType, nFormat, 0x0110 );
                        pField = new SwDateTimeField((SwDateTimeFieldType*)
                            rOut.GetSysFldType(RES_DATETIMEFLD), TIMEFLD, nFormat);
                    }

                }
        break;
        case 33: // current page
            pField = new SwPageNumberField((SwPageNumberFieldType*)
             rOut.GetSysFldType(RES_PAGENUMBERFLD), PG_RANDOM, SVX_NUM_ARABIC);
        break;
        case 34: // evaluation exp
        {
            if (nDepth == 0)
            {
                SwGetExpFieldType* p =
                 (SwGetExpFieldType*)rOut.GetSysFldType(RES_GETEXPFLD);
                DBG_ASSERT(p!=0, "Ww1Fields");
                if (p != 0)
                    pField = new SwGetExpField(p, sFormel,
                     GSE_STRING, VVF_SYS);
            }
            else // rekursion:
            {
                String aName( String::CreateFromAscii(
                                        RTL_CONSTASCII_STRINGPARAM( "Ww" )));
                aName += String::CreateFromInt32( nPlcIndex );
                SwFieldType* pFT = rOut.GetDoc().GetFldType( RES_SETEXPFLD,
                                                            aName);
                if (pFT == 0)
                {
                    SwSetExpFieldType aS(&rOut.GetDoc(), aName, GSE_FORMULA);
                    pFT = rOut.GetDoc().InsertFldType(aS);
                }
                SwSetExpField aFld((SwSetExpFieldType*)pFT, sFormel);
                aFld.SetSubType(SUB_INVISIBLE);
                rOut << aFld;
            }
        }
        break;
        case 36: // print command, Einfuegendatei
        {
            pos = aStr.Search(' ');
            String aFName( aStr.Copy( 0, pos ));
            aStr.Erase(0, pos );
            aStr.Erase(0, 1);
            if( !aFName.Len() )
                break;
            aFName.SearchAndReplaceAscii( "\\\\", String( '\\' ));

//          char* pBook = FindNextPara( pNext, 0 );     //!! Bookmark/Feld-Name
//                                                      //!! erstmal nicht

//          ConvertFFileName( aPara, pFName );          //!! WW1 ????
            aFName = INetURLObject::RelToAbs( aFName );

            String aName( String::CreateFromAscii(
                                        RTL_CONSTASCII_STRINGPARAM( "WW" )));
            SwSection* pSection = new SwSection( FILE_LINK_SECTION,
                rOut.GetDoc().GetUniqueSectionName( &aStr ) );
            pSection->SetLinkFileName( aFName );
            pSection->SetProtect( TRUE );
            rOut << SwFltSection( pSection );
            rOut.EndItem( RES_FLTR_SECTION );
            rOut.NextParagraph();
        }
        case 37: // page ref
            pField = new SwGetRefField(
             (SwGetRefFieldType*)rOut.GetSysFldType(RES_GETREFFLD),
             sFormel, 0, 0, REF_PAGE);
        break;
        case 38: // ask command
        {
            pos = aStr.Search(' ');
            String aName( aStr.Copy( 0, pos ));
            aStr.Erase(0, pos );
            aStr.Erase(0, 1);
            if( !aName.Len() )
                break;

            SwFieldType* pFT = rOut.GetDoc().InsertFldType(
                SwSetExpFieldType( &rOut.GetDoc(), aName, GSE_STRING ) );
            pField = new SwSetExpField((SwSetExpFieldType*)pFT, aStr );
            ((SwSetExpField*)pField)->SetInputFlag( TRUE );
            ((SwSetExpField*)pField)->SetSubType(SUB_INVISIBLE);
//          pField.SetPromptText( aQ ); //!! fehlt noch
//          aFld.SetPar2( aDef );       //!! dito
            // das Ignorieren des Bookmarks ist nicht implementiert
        }
        case 39: // fillin command
            pField = new SwInputField(
                (SwInputFieldType*)rOut.GetSysFldType( RES_INPUTFLD ),
                aEmptyStr, sFormel,
                INP_TXT, 0 );       // sichtbar ( geht z.Zt. nicht anders )
        break;
        case 51: // macro button
        {
            pos = aStr.Search(' ');
            String aName( aStr.Copy( 0, pos ));
            aStr.Erase(0, pos );
            aStr.Erase(0, 1);
            if( !aName.Len() || !aStr.Len() )
                break;
            aName.InsertAscii( "StarOffice.Standard.Modul1.", 0 );

            pField = new SwMacroField( (SwMacroFieldType*)
                            rOut.GetSysFldType( RES_MACROFLD ),
                            aName, aStr );
        }
        break;
        case 55: // read tiff / or better: import anything
        {
            const sal_Unicode* pFormel = sFormel.GetBuffer();
            const sal_Unicode* pDot = 0;
            String sName;
            while (*pFormel != '\0' && *pFormel != ' ')
            {
                // ab hier koennte eine extension kommen
                if (*pFormel == '.')
                    pDot = pFormel;
                else
                    // aha: wir waren bislang noch in dirs
                    if (*pFormel == '\\')
                    {
                        pDot = 0;
                        if (pFormel[1] == '\\')
                            pFormel++;
                    }
                if (*pFormel != '\0')
                    sName += *pFormel++;
            }
            if( pDot )
            {
                String sExt;
                while( *pDot != '\0' && *pDot != ' ')
                    sExt += *pDot++;

                if( sExt.EqualsIgnoreCaseAscii( ".tiff" )
                 || sExt.EqualsIgnoreCaseAscii( ".bmp" )
                 || sExt.EqualsIgnoreCaseAscii( ".gif" )
                 || sExt.EqualsIgnoreCaseAscii( ".pcx" )
                 || sExt.EqualsIgnoreCaseAscii( ".pic" ))
                    rOut.AddGraphic( sName );
                else
                    bKnown = FALSE;
            }
            else
                bKnown = FALSE;
        }
        break;
        default: // unknown
            DBG_ASSERT(FALSE, "Ww1PlainText");
        // unsupported:
        case 1: // unknown
        case 2: // possible bookmark
        case 4: // index entry
        // wwpar5: 1351/1454
        case 5: // footnote ref
//          pField = new SwGetRefField(
//           (SwGetRefFieldType*)rDoc.GetSysFldType(RES_GETREFFLD),
//           sFormel, REF_FOOTNOTE, 0, REF_BEGIN);
        case 7: // if command
        case 8: // create index
        // wwpar5: 1351/1454
        case 9: // table of contents entry
        // wwpar5: 1351/1454
        case 10: // style ref
        case 11: // doc ref
        case 12: // seq ref
        case 13: // create table of contents
        // wwpar5: 1351/1454
        case 35: // literal text
        // print merge:
        case 40: // data command
        case 41: // next command
        case 42: // nextif command
        case 43: // skipif command
        case 44: // number of record
        //
        case 45: // dde ref
        case 46: // dde auto ref
        case 47: // glossary entry
        case 48: // print char
        case 49: // formula def
        case 50: // goto button
        case 52: // auto number outline
        case 53: // auto number legal
        case 54: // auto number arabic
            bKnown = FALSE;
        break;
        }
        if( bKnown || sErgebnis.EqualsAscii( "\0270" ))
            this->sErgebnis.Erase();
        else
            this->sErgebnis = sErgebnis;
    }
    else // oops: we are terribly wrong: skip this
        (*this)++;
}

ULONG Ww1Fields::GetLength()
{
// berechnet die laenge eines feldteiles. nicht mitgerechnet werden
// die terminierenden zeichen im text (19, 20, 21) die beginn, trenner
// und ende bedeuten.
    ULONG ulBeg = Where();
    ULONG ulEnd = Where(nPlcIndex+1);
    DBG_ASSERT(ulBeg<ulEnd, "Ww1Fields");
    return (ulEnd - ulBeg) - 1;
}

/////////////////////////////////////////////////////////////////// Sep
void Ww1Sep::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        rOut.NextSection();
        SwFrmFmt &rFmt = rOut.GetPageDesc().GetMaster();
        W1_DOP& rDOP = rMan.GetDop().GetDOP();
        rOut.GetPageDesc().SetLandscape(rDOP.fWideGet());
        SwFmtFrmSize aSz(rFmt.GetFrmSize());
        aSz.SetWidth(rDOP.xaPageGet());
        aSz.SetHeight(rDOP.yaPageGet());
        rFmt.SetAttr(aSz);
        SvxLRSpaceItem aLR(rDOP.dxaLeftGet()+rDOP.dxaGutterGet(),
         rDOP.dxaRightGet());
        rFmt.SetAttr(aLR);
        SvxULSpaceItem aUL(rDOP.dyaTopGet(), rDOP.dyaBottomGet());
        rFmt.SetAttr(aUL);
    // sobald wir mit dem lesen der zeichen soweit sind, wo sep's
    // momentanes attribut beginnt, wird dieses attribut eingefuegt.
    // diese methode ist bei den meisten start/stop methoden der
    // memberklassen des managers identisch.
        BYTE* p = GetData();
        Ww1SprmSep aSprm(rFib, SVBT32ToLong(p+2));
        aSprm.Start(rOut, rMan);
        aSprm.Stop(rOut, rMan);
        (*this)++;
        aHdd.Start(rOut, rMan);
    }
}

/////////////////////////////////////////////////////////////////// Pap
void Ww1Pap::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (rMan.Where() >= Where())
    {
        BYTE* p;
        USHORT cb;
    // bereitstellen der zu startenden attribute
        if (FillStart(p, cb))
        {
            Ww1SprmPapx aSprm(p, cb);
        // und ausgeben:
            aSprm.Start(rOut, rMan);
        }
        (*this)++;
    }
}

void Ww1Pap::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode&)
{
#ifdef DEBUG
    ULONG ulMan = rMan.Where();
    ULONG ulPap = Where();
    BOOL bIsStopAll = rMan.IsStopAll();
#endif
    if (rMan.Where() >= Where() || rMan.IsStopAll())
    {
        BYTE* p;
        USHORT cb;
        if (FillStop(p, cb)){
            Ww1SprmPapx aSprm(p, cb);
            aSprm.Stop(rOut, rMan);
        }else{
            DBG_ASSERT( !nPlcIndex || rMan.IsStopAll(), "Pap-Attribut-Stop verloren" );
//          rMan.IsStopAll() ist nicht schoen.
        }
    }
}

//////////////////////////////////////////////////////////////// W1_CHP
//
// momentan laesst sich die ausgabe von W1CHPxen nicht nur per define
// loesen....
//
void W1_CHP::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if (fBoldGet())
        rOut << SvxWeightItem(
            rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD);
    if (fItalicGet())
        rOut << SvxPostureItem(
            rOut.GetPostureItalic()?ITALIC_NONE:ITALIC_NORMAL);
    if (fStrikeGet())
        rOut << SvxCrossedOutItem(
            rOut.GetCrossedOut()?STRIKEOUT_NONE:STRIKEOUT_SINGLE);
    if (fOutlineGet())
        rOut << SvxContourItem(!rOut.GetContour());
    if (fSmallCapsGet())
        rOut << SvxCaseMapItem(
            rOut.GetCaseKapitaelchen()?SVX_CASEMAP_NOT_MAPPED:SVX_CASEMAP_KAPITAELCHEN);
    if (fCapsGet())
        rOut << SvxCaseMapItem(
            rOut.GetCaseVersalien()?SVX_CASEMAP_NOT_MAPPED:SVX_CASEMAP_VERSALIEN);
    if (fsHpsGet())
            rOut << SvxFontHeightItem(hpsGet() * 10);
    if (fsKulGet())
        switch (kulGet()) {
        case 0: {
                    rOut << SvxUnderlineItem(UNDERLINE_NONE) <<
                        SvxWordLineModeItem(FALSE);
                } break;
        default: DBG_ASSERT(FALSE, "Chpx");
        case 1: {
                    rOut << SvxUnderlineItem(UNDERLINE_SINGLE);
                } break;
        case 2: {
                    rOut << SvxUnderlineItem(UNDERLINE_SINGLE) <<
                    SvxWordLineModeItem(TRUE);
                } break;
        case 3: {
                    rOut << SvxUnderlineItem(UNDERLINE_DOUBLE);
                } break;
        case 4: {
                    rOut << SvxUnderlineItem(UNDERLINE_DOTTED);
                } break;
        }

    if (fsIcoGet())
        switch(icoGet()) {
        default: DBG_ASSERT(FALSE, "Chpx");
        case 0: { rOut.EndItem(RES_CHRATR_COLOR); } break;
        case 1: { rOut << SvxColorItem(Color(COL_BLACK)); } break;
        case 2: { rOut << SvxColorItem(Color(COL_LIGHTBLUE)); } break;
        case 3: { rOut << SvxColorItem(Color(COL_LIGHTCYAN)); } break;
        case 4: { rOut << SvxColorItem(Color(COL_LIGHTGREEN)); } break;
        case 5: { rOut << SvxColorItem(Color(COL_LIGHTMAGENTA)); } break;
        case 6: { rOut << SvxColorItem(Color(COL_LIGHTRED)); } break;
        case 7: { rOut << SvxColorItem(Color(COL_YELLOW)); } break;
        case 8: { rOut << SvxColorItem(Color(COL_WHITE)); } break;
        }
    if (fsSpaceGet()) {
        short sQps = qpsSpaceGet();
        if (sQps > 56)
            sQps = sQps - 64;
        rOut << SvxKerningItem(sQps);
    }
    if (fsPosGet())
        if (hpsPosGet() == 0)
            rOut << SvxEscapementItem(SVX_ESCAPEMENT_OFF);
        else {
            short sHps = hpsPosGet();
            if (sHps > 128)
                sHps =  sHps - 256;
            sHps *= 100;
            sHps /= 24;
            rOut << SvxEscapementItem(sHps, 100);
        }
    if (fsFtcGet()) {
        SvxFontItem aFont(rMan.GetFont(ftcGet()));
        rOut << aFont;
    }
}

/////////////////////////////////////////////////////////////////// Chp
void Ww1Chp::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
#ifdef DEBUG
    ULONG ulMan = rMan.Where();
    ULONG ulChp = Where();
#endif

    if (rMan.Where() >= Where())
    {
        W1_CHP aChpx;
        if (FillStart(aChpx))
        {
            aChpx.Out(rOut, rMan);
            if (aChpx.fcPicGet())
            {
                Ww1Picture aPic(rMan.GetFib().GetStream(),
                 aChpx.fcPicGet());
                if (!aPic.GetError())
                    aPic.Out(rOut, rMan);
            }
        }
        (*this)++;
    }
}

void Ww1Chp::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode&)
{
    if (rMan.Where() >= Where())
    {
        W1_CHP aChpx;
        if (FillStop(aChpx))
        {
        // zuerst alle toggle-flags
            if (aChpx.fBoldGet())
                rOut.EndItem(RES_CHRATR_WEIGHT);
            if (aChpx.fItalicGet())
                rOut.EndItem(RES_CHRATR_POSTURE);
            if (aChpx.fStrikeGet())
                rOut.EndItem(RES_CHRATR_CROSSEDOUT);
            if (aChpx.fOutlineGet())
                rOut.EndItem(RES_CHRATR_CONTOUR);
            if (aChpx.fSmallCapsGet() || aChpx.fCapsGet())
                rOut.EndItem(RES_CHRATR_CASEMAP);
        // dann alle zahl-werte, diese haben flags, wenn sie gesetzt
        // sind..................
            if (aChpx.fsHpsGet())
                rOut.EndItem(RES_CHRATR_FONTSIZE);
            if (aChpx.fsKulGet())
                rOut.EndItem(RES_CHRATR_UNDERLINE)
                    .EndItem(RES_CHRATR_WORDLINEMODE);
            if (aChpx.fsIcoGet())
                rOut.EndItem(RES_CHRATR_COLOR);
            if (aChpx.fsSpaceGet())
                rOut.EndItem(RES_CHRATR_KERNING);
            if (aChpx.fsPosGet())
                rOut.EndItem(RES_CHRATR_ESCAPEMENT);
            if (aChpx.fsFtcGet())
                rOut.EndItem(RES_CHRATR_FONT);
        }else{
            DBG_ASSERT( !nPlcIndex, "Chp-Attribut-Stop verloren" );
        }
    }
}

///////////////////////////////////////////////////////////////// Style
void Ww1Style::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
// Zuerst Basis, damit Attribute des Basis-Styles erkannt werden
// first: Base................................................
    if(pParent->GetStyle(stcBase).IsUsed() )    // Basis gueltig ?
        rOut.BaseStyle(stcBase);

// next of all: CHP...............................................
    aChpx.Out(rOut, rMan);
// Last: PAP.......................................................
    if (pPapx)
        pPapx->Start(rOut, rMan);
}

////////////////////////////////////////////////////////// Ww1PlainText
//
// die Out() methoden von plaintext fuer den filter geben eine anzahl
// zeichen aus auf die shell, einen string oder einen char, wieviel
// zeichen ausgegeben werden, bestimmt ulEnd, das das ende bestimmt,
// bis zudem ausgegeben wird. ausserdem beenden die methoden die
// ausgabe bei kontrollzeichen.
// diese sind definiert durch MinChar. alle zeichen mit wert darunter
// gelten als kontroll- zeichen. dafuer gibts die methode IsChar, die
// zurueckgibt, ob es sich um ein standard zeichen handelt. kommt ein
// solches zeichen, wird dieses zeichen zurueckgegeben und die methode
// beendet, auch wenn ulEnd noch nicht erreicht wurde. bei nutzung
// also beachten, dasz wenn !IsChar(Out(...)) gilt, ulEnd unter
// umstaenden nicht erreicht wurde. dann wurde das kontrollzeichen
// zwar (weg-)gelesen, jedoch noch nicht ausgegeben.
//
sal_Unicode Ww1PlainText::Out( Ww1Shell& rOut, ULONG& ulEnd )
{
// gibt die zeichen bis ulEnd aus, es sei den es kommen sonderzeichen
// die eine bedeutung haben wie absatzende oder seitenumbruch.
    if (ulEnd > Count())
        ulEnd = Count();
    while (ulSeek < ulEnd)
    {
        sal_Unicode c = (*this)[ulSeek];
        (*this)++;
        if (Ww1PlainText::IsChar(c))
            rOut << c;
        else
            return c;
    }
    return Ww1PlainText::MinChar;
}

sal_Unicode Ww1PlainText::Out( String& rStr, ULONG ulEnd )
{
// wie Out(Shell..., jedoch ausgabe auf einen string
    rStr.Erase();
    if (ulEnd > Count())
        ulEnd = Count();
    while (ulSeek < ulEnd)
    {
        sal_Unicode c = (*this)[ulSeek];
        (*this)++;
        if( Ww1PlainText::IsChar(c) )
            rStr += c;
        else
            return c;
    }
    return Ww1PlainText::MinChar;
}

//
// hier eruebrigt sich ulEnd...oder?
//
sal_Unicode Ww1PlainText::Out( sal_Unicode& rRead )
{
    rRead = (*this)[ulSeek];
    (*this)++;
    return rRead;
}

/////////////////////////////////////////////////////////// Ww1SprmPapx

void Ww1SprmPapx::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
    if( !rMan.IsInStyle() ){        // Innerhalb Style gehts ueber die
                                    // normalen Attribute
        if (!rOut.IsInFly()
            && !rOut.IsInTable()    // Nicht innerhalb Tabelle!
            && ( rMan.HasPPc() || rMan.HasPDxaAbs())){ // Fly-Start
            rOut.BeginFly();        // eAnchor );
        }
        if (!rOut.IsInTable() && rMan.HasInTable())
        {
            rOut.BeginTable();
        }
        rOut.SetStyle(aPapx.stcGet());
    }
    Ww1Sprm::Start(rOut, rMan);
}

void Ww1SprmPapx::Stop(Ww1Shell& rOut, Ww1Manager& rMan)
{
    Ww1Sprm::Stop(rOut, rMan);

    if( !rMan.IsInStyle() )         // Innerhalb Style gehts ueber die
    {                               // normalen Attribute
        if (rOut.IsInTable() &&( rMan.IsStopAll() || !rMan.HasInTable()))
            rOut.EndTable();

        if( rOut.IsInFly() &&
            ( rMan.IsStopAll()
                || ( !rMan.HasPPc() && !rMan.HasPDxaAbs()   // Fly-Ende
                    && !rOut.IsInTable())))     // Nicht innerhalb Tabelle!
            rOut.EndFly();
    }
}

///////////////////////////////////////////////////////////////// Fonts
SvxFontItem Ww1Fonts::GetFont(USHORT nFCode)
{
// erzeugen eine fonts im sw-sinne aus den word-strukturen
    FontFamily eFamily = FAMILY_DONTKNOW;
    String aName;
    FontPitch ePitch = PITCH_DONTKNOW;
    rtl_TextEncoding eCharSet = RTL_TEXTENCODING_DONTKNOW;
    switch (nFCode)
    {
// In the Winword 1.x format, the names of the first three fonts were
// omitted from the table and assumed to be "Tms Rmn" (for ftc = 0),
// "Symbol", and "Helv"
    case 0:
         eFamily = FAMILY_ROMAN;
         aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Tms Rmn" ));
         ePitch = PITCH_VARIABLE;
         eCharSet = RTL_TEXTENCODING_MS_1252;
    break;
    case 1:
         aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Symbol" ));
         ePitch = PITCH_VARIABLE;
         eCharSet = RTL_TEXTENCODING_SYMBOL;
    break;
    case 2:
         eFamily = FAMILY_SWISS;
         aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Helv" ));
         ePitch = PITCH_VARIABLE;
         eCharSet = RTL_TEXTENCODING_MS_1252;
    break;
    default:
    {
        W1_FFN* pF = GetFFN(nFCode - 3);
        if (pF != 0)
        {
        // Fontname .........................................
            aName = String( (sal_Char*)pF->szFfnGet(),
                            RTL_TEXTENCODING_MS_1252 );
        // Pitch .............................................
            static FontPitch ePitchA[] =
            {
                PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW
            };
            ePitch = ePitchA[pF->prgGet()];
        // CharSet ...........................................
            eCharSet = RTL_TEXTENCODING_MS_1252;
            if (aName.EqualsIgnoreCaseAscii("Symbol")
             || aName.EqualsIgnoreCaseAscii("Symbol Set")
             || aName.EqualsIgnoreCaseAscii("Wingdings")
             || aName.EqualsIgnoreCaseAscii("ITC Zapf Dingbats") )
                eCharSet = RTL_TEXTENCODING_SYMBOL;
        // FontFamily ........................................
            USHORT b = pF->ffGet();
            static FontFamily eFamilyA[] =
            {
                FAMILY_DONTKNOW, FAMILY_ROMAN, FAMILY_SWISS, FAMILY_MODERN,
                FAMILY_SCRIPT, FAMILY_DECORATIVE
            };
            if (b < sizeof(eFamilyA))
                eFamily = eFamilyA[b];
        }
        else
        {
            DBG_ASSERT(FALSE, "WW1Fonts::GetFont: Nicht existenter Font !");
            eFamily = FAMILY_SWISS;
             aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Helv" ));
            ePitch = PITCH_VARIABLE;
            eCharSet = RTL_TEXTENCODING_MS_1252;
        }
    }
    break;
    }
            // Extrawurst Hypo
    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO )
         && ( aName.EqualsIgnoreCaseAscii("Helv")
            || aName.EqualsIgnoreCaseAscii("Helvetica") ) )
    {
         aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Helvetica Neue" ));
        if (eFamily==FAMILY_DONTKNOW)
            eFamily = FAMILY_SWISS;
    }
    else
    {
            // VCL matcht die Fonts selber
            // allerdings passiert bei Helv, Tms Rmn und System Monospaced
            // Scheisse, so dass diese ersetzt werden muessen.
            // Nach TH sollen diese durch feste Werte ersetzt werden,
            // also nicht ueber System::GetStandardFont, damit keine
            // Namenslisten auftauchen ( Dieses koennte den User verwirren )
        if( aName.EqualsIgnoreCaseAscii("Helv"))
        {
             aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Helvetica" ));
            if (eFamily==FAMILY_DONTKNOW)
                eFamily = FAMILY_SWISS;
        }
        else if (aName.EqualsIgnoreCaseAscii("Tms Rmn"))
        {
             aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Times New Roman" ));
            if (eFamily==FAMILY_DONTKNOW)
                eFamily = FAMILY_ROMAN;
        }
        else if (aName.EqualsIgnoreCaseAscii("System Monospaced") )
        {
             aName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Courier" ));
            ePitch = PITCH_FIXED;
        }
    }
// nun koennen wir den font basteln: .........................
    return SvxFontItem(eFamily, aName, aEmptyStr, ePitch, eCharSet);
}

/////////////////////////////////////////////////////////////////// Dop
void Ww1Dop::Out(Ww1Shell& rOut)
{
    //~ mdt: fehlt
    // aDop.fWidowControlGet(); // keine Absatztrennung fuer einzelne Zeilen
    long nDefTabSiz = aDop.dxaTabGet();
    if (nDefTabSiz < 56)
        nDefTabSiz = 709;

    // wir wollen genau einen DefaultTab
    SvxTabStopItem aNewTab(1, USHORT(nDefTabSiz),SVX_TAB_ADJUST_DEFAULT);
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;
    rOut.GetDoc().GetAttrPool().SetPoolDefaultItem( aNewTab); //~ mdt: besser (GetDoc)

    SwFrmFmt &rFmt = rOut.GetPageDesc().GetMaster();
    W1_DOP& rDOP = GetDOP();
    rOut.GetPageDesc().SetLandscape(rDOP.fWideGet());
    SwFmtFrmSize aSz(rFmt.GetFrmSize());
    aSz.SetWidth(rDOP.xaPageGet());
    aSz.SetHeight(rDOP.yaPageGet());
    rFmt.SetAttr(aSz);
    SvxLRSpaceItem aLR(rDOP.dxaLeftGet()+rDOP.dxaGutterGet(),
     rDOP.dxaRightGet());
    rFmt.SetAttr(aLR);
    SvxULSpaceItem aUL(rDOP.dyaTopGet(), rDOP.dyaBottomGet());
    rFmt.SetAttr(aUL);

    SwFtnInfo aInfo;
    aInfo = rOut.GetDoc().GetFtnInfo();     // Copy-Ctor privat
                // wo positioniert ? ( 0 == Section, 1 == Page,
                // 2 == beim Text -> Page, 3 == Doc  )
    switch( rDOP.fpcGet() ){
    case 1:
    case 2: aInfo.ePos = FTNPOS_PAGE; break;
    default: aInfo.ePos = FTNPOS_CHAPTER; break;
    }
//  aInfo.eNum = ( rDOP.fFtnRestartGet() ) ? FTNNUM_CHAPTER : FTNNUM_DOC;
    // Da Sw unter Chapter anscheinend was anderes versteht als PMW
    // hier also immer Doc !
    aInfo.eNum = FTNNUM_DOC;
                            // wie neu nummerieren ?
                            // SW-UI erlaubt Nummer nur bei FTNNUM_DOC
    if( rDOP.nFtnGet() > 0 && aInfo.eNum == FTNNUM_DOC )
        aInfo.nFtnOffset = rDOP.nFtnGet() - 1;
    rOut.GetDoc().SetFtnInfo( aInfo );

}

///////////////////////////////////////////////////////////////// Assoc
void Ww1Assoc::Out(Ww1Shell& rOut)
{
    SfxDocumentInfo* pInfo;
    if (rOut.GetDoc().GetpInfo()) //~ mdt: besser (GetDoc)
        pInfo = new SfxDocumentInfo(*rOut.GetDoc().GetpInfo());
    else
        pInfo = new SfxDocumentInfo();
//~ mdt: fehlen: FileNext, Dot, DataDoc, HeaderDoc, Criteria1,
// Criteria2, Criteria3, Criteria4, Criteria5, Criteria6, Criteria7
    pInfo->SetTitle( GetStr(Title) );
    pInfo->SetTheme( GetStr(Subject) );
    pInfo->SetComment( GetStr(Comments) );
    pInfo->SetKeywords( GetStr(KeyWords) );
    pInfo->SetCreated( GetStr(Author) );
    pInfo->SetChanged( GetStr(LastRevBy) );
    rOut.GetDoc().SetInfo(*pInfo);
    delete pInfo;
}

//////////////////////////////////////////////////////////// StyleSheet
void Ww1StyleSheet::OutDefaults(Ww1Shell& rOut, Ww1Manager& rMan, USHORT stc)
{
    switch (stc){
    case 222: // Null
        rOut << SvxFontHeightItem(240);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 223: // annotation reference
        rOut << SvxFontHeightItem(160);
        break;
    case 224: // annotation text
        rOut << SvxFontHeightItem(200);
        break;
    case 225: // table of contents 8
    case 226: // table of contents 7
    case 227: // table of contents 6
    case 228: // table of contents 5
    case 229: // table of contents 4
    case 230: // table of contents 3
    case 231: // table of contents 2
    case 232: // table of contents 1
        rOut << SvxLRSpaceItem(( 232 - stc ) * 720, 720);
            // Tabulatoren fehlen noch !
        break;
    case 233: // index 7
    case 234: // und index 6
    case 235: // und index 5
    case 236: // und index 4
    case 237: // und index 3
    case 238: // und index 2
        rOut << SvxLRSpaceItem(( 239 - stc ) * 360, 0);
        break;
    case 239: // index 1
        break;
    case 240: // line number
        break;
    case 241: // index heading
        break;
    case 242:  // footer
    case 243:{ // ... und header
            SvxTabStopItem aAttr;
            SvxTabStop aTabStop;
            aTabStop.GetTabPos() = 4535;  // 8 cm
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
            aAttr.Insert( aTabStop );
            aTabStop.GetTabPos() = 9071;  // 16 cm
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
            aAttr.Insert( aTabStop );
            rOut << aAttr;
        }
        break;
    case 244: // footnote reference
        rOut << SvxFontHeightItem(160);
        rOut << SvxEscapementItem(6 * 100 / 24, 100);
        break;
    case 245: // footnote text
        rOut << SvxFontHeightItem(200);
        break;
    case 246: // heading 9
    case 247: // und heading 8
    case 248: // und heading 7
        rOut << SvxLRSpaceItem(720, 0);
        rOut << SvxPostureItem(
                    rOut.GetPostureItalic()?ITALIC_NONE:ITALIC_NORMAL);
        rOut << SvxFontHeightItem(200);
        break;
    case 249: // heading 6
        rOut << SvxLRSpaceItem(720, 0);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE);
        rOut << SvxFontHeightItem(200);
        break;
    case 250: // heading 5
        rOut << SvxLRSpaceItem(720, 0);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD);
        rOut << SvxFontHeightItem(200);
        break;
    case 251: // heading 4
        rOut << SvxLRSpaceItem(360, 0);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE);
        rOut << SvxFontHeightItem(240);
        break;
    case 252: // heading 3
        rOut << SvxLRSpaceItem(360, 0);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD);
        rOut << SvxFontHeightItem(240);
        break;
    case 253: // heading 2
        rOut << SvxULSpaceItem(120, 0);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD);
        rOut << SvxFontHeightItem(240);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 254: // heading 1
        rOut << SvxULSpaceItem(240, 0);
        rOut << SvxWeightItem(rOut.GetWeightBold()?WEIGHT_NORMAL:WEIGHT_BOLD);
        rOut << SvxUnderlineItem(UNDERLINE_SINGLE);
        rOut << SvxFontHeightItem(240);
        rOut << SvxFontItem(rMan.GetFont(2));
        break;
    case 255: // Normal indent
        rOut << SvxLRSpaceItem(720, 0);
        break;
    case 0: // Normal
        rOut << SvxFontHeightItem(200);
        break;
    default: // selbstdefiniert
        rOut << SvxFontHeightItem(200);
        break;
    }
}

void Ww1StyleSheet::OutOne(Ww1Shell& rOut, Ww1Manager& rMan, USHORT stc)
{
    const RES_POOL_COLLFMT_TYPE RES_NONE = RES_POOLCOLL_DOC_END;
    RES_POOL_COLLFMT_TYPE aType = RES_NONE;
//              aType = RES_POOLCOLL_JAKETADRESS; break;
//              aType = RES_POOLCOLL_LISTS_BEGIN; break;
//              aType = RES_POOLCOLL_SENDADRESS; break;
//              aType = RES_POOLCOLL_SIGNATURE; break;
//              aType = RES_POOLCOLL_TEXT_NEGIDENT; break;
//              aType = RES_POOLCOLL_TOX_IDXH; break;
    switch (stc)
    {
    case 222: // Null
        aType = RES_POOLCOLL_TEXT; break;   //???
        break;
    case 223: // annotation reference
        break;
    case 224: // annotation text
        break;
    case 225: // table of contents 8
        aType = RES_POOLCOLL_TOX_CNTNT8; break;
    case 226: // table of contents 7
        aType = RES_POOLCOLL_TOX_CNTNT7; break;
    case 227: // table of contents 6
        aType = RES_POOLCOLL_TOX_CNTNT6; break;
    case 228: // table of contents 5
        aType = RES_POOLCOLL_TOX_CNTNT5; break;
    case 229: // table of contents 4
        aType = RES_POOLCOLL_TOX_CNTNT4; break;
    case 230: // table of contents 3
        aType = RES_POOLCOLL_TOX_CNTNT3; break;
    case 231: // table of contents 2
        aType = RES_POOLCOLL_TOX_CNTNT2; break;
    case 232: // table of contents 1
        aType = RES_POOLCOLL_TOX_CNTNT1; break;
    case 233: // index 7
        break;
    case 234: // index 6
        break;
    case 235: // index 5
        break;
    case 236: // index 4
        break;
    case 237: // index 3
        aType = RES_POOLCOLL_TOX_IDX3; break;
    case 238: // index 2
        aType = RES_POOLCOLL_TOX_IDX2; break;
    case 239: // index 1
        aType = RES_POOLCOLL_TOX_IDX1; break;
    case 240: // line number
        break;
    case 241: // index heading
        break;
    case 242: // footer
        aType = RES_POOLCOLL_FOOTER; break;
    case 243: // header
        aType = RES_POOLCOLL_HEADER; break;
    case 244: // footnote reference
        break;
    case 245: // footnote text
        aType = RES_POOLCOLL_FOOTNOTE; break;
    case 246: // heading 9
        break;
    case 247: // heading 8
        break;
    case 248: // heading 7
        break;
    case 249: // heading 6
        break;
    case 250: // heading 5
        aType = RES_POOLCOLL_HEADLINE5; break;
    case 251: // heading 4
        aType = RES_POOLCOLL_HEADLINE4; break;
    case 252: // heading 3
        aType = RES_POOLCOLL_HEADLINE3; break;
    case 253: // heading 2
        aType = RES_POOLCOLL_HEADLINE2; break;
    case 254: // heading 1
        aType = RES_POOLCOLL_HEADLINE1; break;
    case 255: // Normal indent
        aType = RES_POOLCOLL_TEXT_IDENT; break;
    case 0: // Normal
        aType = RES_POOLCOLL_STANDARD; break;
//      aType = RES_POOLCOLL_TEXT; break;       // Das ist "textkoerper"
    }
    if (aType == RES_NONE)
        rOut.BeginStyle(stc, GetStyle(stc).GetName() );
    else
        rOut.BeginStyle(stc, aType);
    OutDefaults(rOut, rMan, stc);
    GetStyle(stc).Out(rOut, rMan);
    rOut.EndStyle();
//  rMan.SetInApo(FALSE);
}
// OutOneWithBase() liest einen Style mit OutOne() einen Style ein
// Jedoch liest er, wenn noch nicht geschehen, den Basisstyle rekursiv ein
void Ww1StyleSheet::OutOneWithBase(Ww1Shell& rOut, Ww1Manager& rMan,
                                   USHORT stc, BYTE* pbStopRecur )
{
// SH: lineares Einlesen ist Scheisse, da dann BasedOn nicht gesetzt
// werden kann und ausserdem Toggle- und Modify-Attrs (z.B. Tabs ) nicht gehen.

    Ww1Style& rSty = GetStyle(stc);
    USHORT nBase = rSty.GetnBase();
    if( nBase != stc
        && !rOut.IsStyleImported( nBase )
        && GetStyle(nBase).IsUsed()
        && !pbStopRecur[nBase] ){

        pbStopRecur[nBase] = 1;
        OutOneWithBase( rOut, rMan, nBase, pbStopRecur ); // Rekursiv
    }
    OutOne( rOut, rMan, stc );
}

void Ww1StyleSheet::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    USHORT stc;
    BYTE bStopRecur[256];
    memset( bStopRecur, FALSE, sizeof(bStopRecur) );

// 1. Durchlauf: Styles mit Basisstyles rekursiv
    for (stc=0;stc<Count();stc++)
        if (GetStyle(stc).IsUsed() && !rOut.IsStyleImported( stc ) )
            OutOneWithBase( rOut, rMan, stc, bStopRecur );

// 2. Durchlauf: Follow-Styles
    for (stc=0;stc<Count();stc++){
        Ww1Style& rSty = GetStyle(stc);
        if ( rSty.IsUsed() ){
            USHORT nNext = rSty.GetnNext();
            if( nNext != stc && GetStyle(nNext).IsUsed() )
                rOut.NextStyle( stc, nNext );
        }
    }
}

////////////////////////////////////////////////////////////// Picture
static ULONG GuessPicSize(W1_PIC* pPic)
{
    long nSize = pPic->lcbGet() - (sizeof(*pPic)-sizeof(pPic->rgb));
    register BYTE* p = pPic->rgbGet();
    USHORT maxx = pPic->mfp.xExtGet();
    USHORT padx = ((maxx + 7) / 8) * 8;
    USHORT maxy = pPic->mfp.yExtGet();
    return 120L + (ULONG)padx * maxy;
}

//
// folgende methode schreibt eine windows-.BMP-datei aus einem
// embeddeten bild in ww-1 dateien
// gelesen wird 4-bit format, geschrieben jedoch 8-bit.
//
void Ww1Picture::WriteBmp(SvStream& rOut)
{
    long nSize = pPic->lcbGet() - (sizeof(*pPic)-sizeof(pPic->rgb));
    register BYTE* p = pPic->rgbGet();
    USHORT maxx = pPic->mfp.xExtGet();
    USHORT padx = ((maxx + 7) / 8) * 8;
    USHORT maxy = pPic->mfp.yExtGet();
    USHORT unknown1 = SVBT16ToShort(p); p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
    USHORT unknown2 = SVBT16ToShort(p); p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
    USHORT x = SVBT16ToShort(p); p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
    DBG_ASSERT(x==maxx, "Ww1Picture");
    USHORT y = SVBT16ToShort(p); p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
    DBG_ASSERT(y==maxy, "Ww1Picture");
    USHORT planes = SVBT16ToShort(p); p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
    DBG_ASSERT(planes==1, "Ww1Picture");
    USHORT bitcount = SVBT16ToShort(p); p+= sizeof(SVBT16); nSize -= sizeof(SVBT16);
    DBG_ASSERT(bitcount==4, "Ww1Picture");
    DBG_ASSERT(16*3+padx*maxy/2==nSize, "Ww1Picture");
    SVBT32 tmpLong;
    SVBT16 tmpShort;
    SVBT8 tmpByte;
#define wLong(n) \
    LongToSVBT32(n, tmpLong); \
    if ((rOut.Write(tmpLong, sizeof(SVBT32))) != sizeof(SVBT32)) goto error;
#define wShort(n) \
    ShortToSVBT16(n, tmpShort); \
    if ((rOut.Write(tmpShort, sizeof(SVBT16))) != sizeof(SVBT16)) goto error;
#define wByte(n) \
    ByteToSVBT8(n, tmpByte); \
    if ((rOut.Write(tmpByte, sizeof(SVBT8))) != sizeof(SVBT8)) goto error;
    wByte('B'); wByte('M');
    wLong(54 + 4 * 16 + padx * maxy);
    wLong(0);
    wLong(54 + 4 * 16);
    wLong(40);
    wLong(maxx);
    wLong(maxy);
    wShort(1);
    wShort(8);
    wLong(0);
    wLong(0);
    wLong(0);
    wLong(0);
    wLong(16);
    wLong(16);
    register USHORT i;
    for (i=0;nSize>0&&i<16;i++)
    {
        wByte(*p);
        p++;
        nSize -= sizeof(BYTE);
        wByte(*p);
        p++;
        nSize -= sizeof(BYTE);
        wByte(*p);
        p++;
        nSize -= sizeof(BYTE);
        wByte(0);
    }
    DBG_ASSERT(padx*maxy/2==nSize, "Ww1Picture");
    register USHORT j;
#if 1
    {
        BYTE* pBuf = new BYTE[padx];
        BYTE* q = pBuf;
        for (j=0;nSize>0&&j<maxy;j++)
        {
            register BYTE* q = pBuf;
            for (i=0;nSize>0&&i<maxx;i+=2)
            {
                *q++ = *p>>4;
                *q++ = *p&0xf;
                p++;
                nSize -= sizeof(BYTE);
            }
            for (;i<padx;i+=2)
            {
                *q++ = 0;
                p++;
                nSize -= sizeof(BYTE);
            }
            if(rOut.Write(pBuf, padx) != padx){
                delete pBuf;
                goto error;
            }
        }
        delete pBuf;
    }
#else
    for (j=0;nSize>0&&j<maxy;j++)
    {
        for (i=0;nSize>0&&i<maxx;i+=2)
        {
            wByte(*p>>4);
            wByte(*p&0xf);
            p++;
            nSize -= sizeof(BYTE);
        }
        for (;i<padx;i+=2)
        {
            wByte(0);
            p++;
            nSize -= sizeof(BYTE);
        }
    }
#endif
    DBG_ASSERT(nSize==0, "Ww1Picture");
#undef wLong
#undef wShort
#undef wByte
    rOut.Seek(0);
    return;
error:
    ;
}

void Ww1Picture::Out(Ww1Shell& rOut, Ww1Manager& rMan)
{
    Graphic* pGraphic = 0;
    USHORT mm;
    switch (mm = pPic->mfp.mmGet())
    {
    case 8: // embedded metafile
    {
        SvMemoryStream aOut(8192, 8192);
        aOut.Write(pPic->rgbGet(), pPic->lcbGet() -
         (sizeof(*pPic)-sizeof(pPic->rgb)));
        aOut.Seek(0);
        GDIMetaFile aWMF;
        if (ReadWindowMetafile(aOut, aWMF) && aWMF.GetActionCount() > 0)
        {
            aWMF.SetPrefMapMode(MapMode(MAP_100TH_MM));
            Size aOldSiz(aWMF.GetPrefSize());
            Size aNewSiz(pPic->mfp.xExtGet(), pPic->mfp.yExtGet());
            Fraction aFracX(aNewSiz.Width(), aOldSiz.Width());
            Fraction aFracY(aNewSiz.Height(), aOldSiz.Height());
            aWMF.Scale(aFracX, aFracY);
            aWMF.SetPrefSize(aNewSiz);
            pGraphic = new Graphic(aWMF);
        }
        break;
    }
    case 94: // embedded name SH:??? Was denn nun ? Embeddet oder Name ?
    case 98: // TIFF-Name
    {
        String aDir( (sal_Char*)pPic->rgbGet(),
                (USHORT)(pPic->lcbGet() - (sizeof(*pPic)-sizeof(pPic->rgb))),
                RTL_TEXTENCODING_MS_1252 );
        //SvFileStream aOut(aDir, STREAM_READ|STREAM_WRITE|STREAM_TRUNC);
        rOut.AddGraphic( aDir );
    }
    break;
    case 97: // embedded bitmap
//  case 99: // SH: bei meinem BspDoc 41738.doc auch embedded Bitmap,
             // aber leider anderes Format
    {
        ULONG nSiz = GuessPicSize(pPic);
        SvMemoryStream aOut(nSiz, 8192);
        WriteBmp(aOut);
        Bitmap aBmp;
        aOut >> aBmp;
        pGraphic = new Graphic(aBmp);
    }
    default:
        DBG_ASSERT(pPic->mfp.mmGet() == 97, "Ww1Picture");
    }
    if (pGraphic)
        rOut << *pGraphic;
}

////////////////////////////////////////////////////////// HeaderFooter
void Ww1HeaderFooter::Start(Ww1Shell& rOut, Ww1Manager& rMan)
{
// wird sowieso nur bei SEPs aufgerufen, keine weitere pruefung
// noetig:
    if (!rMan.Pushed())
    {
        while ((*this)++)
            switch (eHeaderFooterMode)
            {
            case FtnSep:
            break;
            case FtnFollowSep:
            break;
            case FtnNote:
            break;
            case EvenHeadL:
            break;
            case OddHeadL:
            {
                ULONG begin;
                ULONG end;
                if (FillOddHeadL(begin, end))
                {
                    Ww1HddText* pText = new Ww1HddText(rMan.GetFib());
                    pText->Seek(begin);
                    pText->SetCount(end-begin);
                    rOut.BeginHeader();
                    rMan.Push1(pText, pText->Offset(rMan.GetFib()), begin,
                     new Ww1HeaderFooterFields(rMan.GetFib()));
                    rOut << rMan;
                    rMan.Pop();
                    rOut.EndHeaderFooter();
                    return;
                }
            }
            break;
            case EvenFootL:
            break;
            case OddFootL:
            {
                ULONG begin;
                ULONG end;
                if (FillOddFootL(begin, end))
                {
                    Ww1HddText* pText = new Ww1HddText(rMan.GetFib());
                    pText->Seek(begin);
                    pText->SetCount(end-begin);
                    rOut.BeginFooter();
                    rMan.Push1(pText, pText->Offset(rMan.GetFib()), begin,
                     new Ww1HeaderFooterFields(rMan.GetFib()));
                    rOut << rMan;
                    rMan.Pop();
                    rOut.EndHeaderFooter();
                    return;
                }
            }
            break;
            case FirstHeadL:
            break;
            }
    }
}

void Ww1HeaderFooter::Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode&)
{
    if (!rMan.Pushed() && eHeaderFooterMode != None
//   && rMan.GetText().Where() >= rMan.GetText().Count()
    )
    {
        Start(rOut, rMan);
    }
}

/***********************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww1/w1filter.cxx,v 1.1.1.1 2000-09-18 17:14:57 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.29  2000/09/18 16:04:56  willem.vandorp
      OpenOffice header added.

      Revision 1.28  2000/05/11 16:06:52  jp
      Changes for Unicode

      Revision 1.27  2000/02/11 14:39:31  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.26  1999/03/16 11:12:24  JP
      Task #63047#: keine Abfrage mehr auf ein SVX_TAB_DEFDIST


      Rev 1.25   16 Mar 1999 12:12:24   JP
   Task #63047#: keine Abfrage mehr auf ein SVX_TAB_DEFDIST

      Rev 1.24   17 Nov 1998 10:48:18   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.23   27 Feb 1998 19:29:06   HJS
   SH 27.02.98

      Rev 1.20   09 Dec 1997 17:04:38   JP
   neue InhaltsVerzeichnis Poolvorlagen -> neuer MaxLevel

      Rev 1.19   29 Nov 1997 17:37:10   MA
   includes

      Rev 1.18   21 Nov 1997 17:22:44   OM
   Feldbefehl-Umstellung: DocInfo

      Rev 1.17   14 Oct 1997 14:17:18   OM
   Feldumstellung

      Rev 1.16   09 Oct 1997 14:13:46   JP
   Aenderungen von SH

      Rev 1.15   02 Oct 1997 15:26:00   OM
   Feldumstellung

      Rev 1.14   29 Sep 1997 12:16:56   OM
   Feldumstellung

      Rev 1.13   24 Sep 1997 15:25:24   OM
   Feldumstellung

      Rev 1.12   04 Sep 1997 11:18:00   JP
   include

      Rev 1.11   03 Sep 1997 14:16:58   SH
   Flys, Tabellen ok, Style-Abhaengigkeiten u.v.a.m

      Rev 1.10   15 Aug 1997 12:50:36   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.9   12 Aug 1997 14:16:34   OS
   Header-Umstellung

      Rev 1.8   11 Aug 1997 12:51:02   SH
   Flys und einfache Tabellen und Tabulatoren

      Rev 1.7   17 Jul 1997 15:20:52   MA
   Bitmap Typ 99 geht nicht, abgeklemmt

      Rev 1.6   16 Jun 1997 13:08:56   MA
   Stand Micheal Dietrich

      Rev 1.5   24 Mar 1997 03:04:22   HJS
   fuer os2 auskommentiert

      Rev 1.4   12 Mar 1997 19:12:18   SH
   MDT: Progressbar, Pagedesc-Bug, Spalten, Anfang Tabellen u.a.

      Rev 1.3   10 Jan 1997 18:46:30   SH
   Stabiler und mehr von MDT

      Rev 1.2   28 Nov 1996 18:08:50   SH
   Schneller Schoener Weiter von MDT

      Rev 1.1   30 Sep 1996 23:23:40   SH
   neu von MDT

      Rev 1.0   14 Aug 1996 19:32:28   SH
   Initial revision.


***********************************************************************/
//]})
