/*************************************************************************
 *
 *  $RCSfile: swlbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWLBOX_HXX
#include <swlbox.hxx>
#endif


SV_IMPL_PTRARR(SwEntryLst, SwBoxEntry*)

/*--------------------------------------------------------------------
     Beschreibung: Ein ListboxElement
 --------------------------------------------------------------------*/


SwBoxEntry::SwBoxEntry() :
    bModified(FALSE),
    bNew(FALSE),
    nId(LISTBOX_APPEND)
{
}


SwBoxEntry::SwBoxEntry(const String& aNam, USHORT nIdx) :
    bModified(FALSE),
    bNew(FALSE),
    aName(aNam),
    nId(nIdx)
{
}


SwBoxEntry::SwBoxEntry(const SwBoxEntry& rOld) :
    aName(rOld.aName),
    nId(rOld.nId),
    bNew(rOld.bNew),
    bModified(rOld.bModified)
{

}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/


/*SwListBox::SwListBox(Window* pParent, const ResId& rId):
    ListBox(pParent, rId)
{
    DBG_ASSERT( 0 == (ListBox::GetStyle() & WB_SORT), "NIE sortiert aus der Resource lesen!" );
    // falls eine Liste ueber die Resource gelesen wurde, die interne
    // entsprechend updaten
    USHORT nCnt = ListBox::GetEntryCount();
    for( USHORT n = 0; n < nCnt; ++n )
    {
        const SwBoxEntry* pTmp = new SwBoxEntry( ListBox::GetEntry( n ), n );
        aEntryLst.Insert( pTmp, n );
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Basisklasse Dtor
 --------------------------------------------------------------------*/


/*SwListBox::~SwListBox()
{
}

/*--------------------------------------------------------------------
     Beschreibung: Listen loeschen und Anzeige loeschen
 --------------------------------------------------------------------*/


/*void SwListBox::Clear()
{
    ListBox::Clear();
    aEntryLst.DeleteAndDestroy( 0, aEntryLst.Count() );
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/


/*const SwBoxEntry& SwListBox::GetEntry(USHORT nPos) const
{
    if( nPos < aEntryLst.Count() )
        return *aEntryLst[ nPos ];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: aktullen Eintrag zurueckgeben
 --------------------------------------------------------------------*/


/*const SwBoxEntry& SwListBox::GetSelectEntry() const
{
    USHORT nPos = ListBox::GetSelectEntryPos();
    if( nPos < aEntryLst.Count() )
        return *aEntryLst[ nPos ];

    return aDefault;
}


void SwListBox::RemoveEntry( USHORT nPos )
{
    if( nPos < aEntryLst.Count() )
    {
        aEntryLst.DeleteAndDestroy( nPos, 1 );
        ListBox::RemoveEntry( nPos );
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag in die ListBox aufnehmen
 --------------------------------------------------------------------*/


/*void SwListBox::InsertEntry( const SwBoxEntry& rEntry, USHORT nPos )
{
    if( nPos >= aEntryLst.Count() )
        nPos = aEntryLst.Count();

    SwBoxEntry* pEntry = new SwBoxEntry( rEntry );
    ListBox::InsertEntry( pEntry->aName, nPos );
    aEntryLst.C40_INSERT( SwBoxEntry, pEntry, nPos );
}

/*--------------------------------------------------------------------
     Beschreibung: Sortiert einfuegen
 --------------------------------------------------------------------*/


/*void SwListBox::InsertEntrySort( const SwBoxEntry& rEntry )
{
    USHORT nPos;
    if( !SeekEntry( rEntry, &nPos ) )
    {
        SwBoxEntry* pEntry = new SwBoxEntry( rEntry );
        ListBox::InsertEntry( pEntry->aName, nPos );
        aEntryLst.C40_INSERT( SwBoxEntry, pEntry, nPos );
    }
}


BOOL SwListBox::SeekEntry( const SwBoxEntry& rEntry, USHORT* pPos )
{
    register USHORT nO = aEntryLst.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = aEntryLst[ nM ]->aName.ICompare( rEntry.aName );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pPos ) *pPos = nM;
                return TRUE;
            }
            else if( COMPARE_GREATER == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
                break;
            else
                nO = nM - 1;
        }
    }
    if( pPos ) *pPos = nU;
    return FALSE;
}

/*  */


SwComboBox::SwComboBox(Window* pParent, const ResId& rId, USHORT nStyleBits ):
    ComboBox(pParent, rId),
    nStyle(nStyleBits)
{
    // Verwaltung fuer die Stringlist aus der Resource aufbauen
    USHORT nSize = GetEntryCount();
    for( USHORT i=0; i < nSize; ++i )
    {
        const SwBoxEntry* pTmp = new SwBoxEntry(ComboBox::GetEntry(i), i);
        aEntryLst.Insert(pTmp, aEntryLst.Count() );
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Basisklasse Dtor
 --------------------------------------------------------------------*/


SwComboBox::~SwComboBox()
{
// das erledigen die Listen doch schon selbst im DTOR!
//  aEntryLst.DeleteAndDestroy(0,   aEntryLst.Count());
//  aDelEntryLst.DeleteAndDestroy(0, aDelEntryLst.Count());
}

/*--------------------------------------------------------------------
     Beschreibung: neue Eintraege verwalten
 --------------------------------------------------------------------*/


void SwComboBox::InsertNewEntry(const SwBoxEntry& rEntry)
{
    SwBoxEntry* pNew = new SwBoxEntry(rEntry);
    pNew->bNew = TRUE;
    InsertSorted(pNew);
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag in die ComboBox aufnehmen
 --------------------------------------------------------------------*/


void SwComboBox::InsertEntry(const SwBoxEntry& rEntry)
{
    InsertSorted(new SwBoxEntry(rEntry));
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag aus der Liste loeschen
 --------------------------------------------------------------------*/


void SwComboBox::RemoveEntry(USHORT nPos)
{
    if(nPos >= aEntryLst.Count())
        return;

    // Altes Element austragen
    SwBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    ComboBox::RemoveEntry(nPos);

    // keine neuen Eintraege in die Liste mit aufnehmen
    if(pEntry->bNew)
        return;

    // in DeleteListe eintragen
    aDelEntryLst.C40_INSERT(SwBoxEntry, pEntry, aDelEntryLst.Count());
}



/*--------------------------------------------------------------------
     Beschreibung: Position by Name
 --------------------------------------------------------------------*/

USHORT SwComboBox::GetEntryPos(const SwBoxEntry& rEntry) const
{
    return ComboBox::GetEntryPos(rEntry.aName);
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/


const SwBoxEntry& SwComboBox::GetEntry(USHORT nPos) const
{
    if(nPos < aEntryLst.Count())
        return *aEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: geloeschte Eintraege
 --------------------------------------------------------------------*/


USHORT SwComboBox::GetRemovedCount() const
{
    return aDelEntryLst.Count();
}


const SwBoxEntry& SwComboBox::GetRemovedEntry(USHORT nPos) const
{
    if(nPos < aDelEntryLst.Count())
        return *aDelEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Neue Entries begutachten
 --------------------------------------------------------------------*/


USHORT SwComboBox::GetNewCount() const
{
    USHORT nNew = 0;
    USHORT nSize = aEntryLst.Count();
    for(USHORT i=0; i < nSize; ++i)
        if(aEntryLst[i]->bNew)
            nNew++;

    return nNew;
}

/*--------------------------------------------------------------------
     Beschreibung:  Alle neuen Eintraege ueberpruefen
 --------------------------------------------------------------------*/


const SwBoxEntry& SwComboBox::GetNewEntry(USHORT nPos) const
{
    USHORT nSize = aEntryLst.Count();
    USHORT nNew  = 0;

    for(USHORT i=0; i < nSize; ++i)
        if( aEntryLst[i]->bNew && nNew++ == nPos )
            return *aEntryLst[i];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Sortiert einfuegen
 --------------------------------------------------------------------*/


void SwComboBox::InsertSorted(SwBoxEntry* pEntry)
{
    ComboBox::InsertEntry(pEntry->aName);
    USHORT nPos = ComboBox::GetEntryPos(pEntry->aName);
    aEntryLst.C40_INSERT(SwBoxEntry, pEntry, nPos);
}


/*--------------------------------------------------------------------
    Beschreibung: Je nach Option bestimmte Zeichen ausblenden
 --------------------------------------------------------------------*/


void SwComboBox::KeyInput( const KeyEvent& rKEvt )
{
    USHORT nChar = rKEvt.GetCharCode();

    if(nStyle & CBS_FILENAME)
    {
#ifdef MAC
        if(nChar == ':')
            return;
#elif defined UNX
        if(nChar == '/' || nChar == ' ' )
            return;
#else
        if(nChar == ':' || nChar == '\\' || nChar == '.' || nChar == ' ')
            return;
#endif
    }
    ComboBox::KeyInput(rKEvt);
}



/*--------------------------------------------------------------------
    Beschreibung: Text nach Option konvertieren
 --------------------------------------------------------------------*/


String SwComboBox::GetText() const
{
    String aTxt( ComboBox::GetText() );

    if(nStyle & CBS_LOWER)
        GetAppCharClass().toLower( aTxt );
    else if( nStyle & CBS_UPPER )
        GetAppCharClass().toUpper( aTxt );

    return aTxt;
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.52  2000/09/18 16:05:13  willem.vandorp
    OpenOffice header added.

    Revision 1.51  2000/08/30 16:45:51  jp
    use CharClass instead of international

    Revision 1.50  2000/04/11 08:03:01  os
    UNICODE

    Revision 1.49  2000/02/11 14:43:21  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.48  1997/11/03 12:09:46  MA
    precomp entfernt


      Rev 1.47   03 Nov 1997 13:09:46   MA
   precomp entfernt

      Rev 1.46   08 Aug 1997 17:29:48   OM
   Headerfile-Umstellung

      Rev 1.45   06 Aug 1997 11:28:38   TRI
   GetpApp statt pApp

      Rev 1.44   23 Apr 1997 10:39:28   OS
   ResId const

      Rev 1.43   07 Nov 1996 11:47:52   JP
   ASSERT korrigiert

      Rev 1.42   05 Nov 1996 15:36:36   JP
   SwListBox: Sortierung selbst uebernommen; ueberfluessiges entfernt/aufgeraeumt

      Rev 1.41   28 Aug 1996 08:23:42   OS
   includes

      Rev 1.40   16 Aug 1996 13:24:00   TRI
   C40_INSERT statt Insert

      Rev 1.39   25 Jun 1996 17:46:54   HJS
   include tlintl.hxx

      Rev 1.38   16 Apr 1996 16:46:10   OM
   SwEventListBox entfernt

      Rev 1.37   24 Nov 1995 16:57:38   OM
   PCH->PRECOMPILED

      Rev 1.36   17 Nov 1995 13:21:36   MA
   Segmentierung

      Rev 1.35   27 Jul 1995 10:44:42   mk
   an SCC4.0.1a angepasst (MDA)

      Rev 1.34   20 Mar 1995 19:03:20   OS
   unbenutzte Funktionen entfernt

      Rev 1.33   15 Dec 1994 20:21:28   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.32   25 Oct 1994 14:55:22   ER
   add: PCH

      Rev 1.31   10 May 1994 13:48:44   MS
   boeser Fehler! bei sortierten SwlBoxen kam der Zugriff auf die EntryListe durcheinander

      Rev 1.30   06 May 1994 13:17:28   MS
   GetSelectEntryPos geandert

      Rev 1.29   02 Mar 1994 17:22:56   MS
   mit EntryPos

      Rev 1.28   02 Mar 1994 15:46:52   VB
   ListBox mit MouseButtonDown-Handler

      Rev 1.27   26 Feb 1994 02:59:36   ER
   virt. dtor mit export

      Rev 1.26   23 Feb 1994 12:07:04   ER
   SEXPORT

      Rev 1.25   17 Feb 1994 08:35:06   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.24   16 Feb 1994 19:56:36   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.23   16 Feb 1994 15:41:54   MI
   Pragmas zurechtgerueckt

      Rev 1.22   28 Jan 1994 11:36:26   MI
   TCOV() entfernt, SW_... nach SEG_... umbenannt

      Rev 1.21   13 Jan 1994 08:33:26   MI
   Segmentierung per #define ermoeglicht

      Rev 1.20   10 Jan 1994 09:38:36   MS
   UNX - Fehler behoben

      Rev 1.19   09 Jan 1994 21:06:46   MI
   Provisorische Segmentierung

      Rev 1.18   06 Jan 1994 13:14:38   MS
   Default-Options geaendert

      Rev 1.17   08 Dec 1993 11:21:38   MS
   SwBoxEntry() setz nId auf LISTBOX_ENTRY_NOTFOUND

      Rev 1.16   06 Dec 1993 11:47:10   VB
   Upper/Lower konvertieren ueber International

      Rev 1.15   24 Sep 1993 18:50:02   VB
   Segmentierung

      Rev 1.14   24 Sep 1993 08:59:44   MS
   Beschraenkung der Eingabe geaendert

      Rev 1.13   23 Sep 1993 17:33:46   MS
   ComboBox Namen im Edit begrenzen

      Rev 1.12   27 Apr 1993 08:18:20   OK
   NEU: #pragma hdrstop

      Rev 1.11   16 Feb 1993 08:52:18   MS
   fuer m800

      Rev 1.10   10 Feb 1993 19:42:08   VB
   Umstellung SV21

      Rev 1.9   06 Jan 1993 09:15:30   OK
   Anpassung an M70

      Rev 1.8   02 Dec 1992 17:39:46   MS
   neu SwMultiComboBox

      Rev 1.7   27 Nov 1992 16:04:52   MS
   RemoveEntry() korrigiert

      Rev 1.6   27 Nov 1992 15:55:48   MS
   mit InitComboBox

      Rev 1.5   27 Nov 1992 15:42:52   MS
   Init erweitert

      Rev 1.4   27 Nov 1992 14:42:38   MS
   mit const SwBoxEntr& Returns

      Rev 1.3   27 Nov 1992 13:43:54   MS
   GetCurEntry gerichtet

      Rev 1.2   27 Nov 1992 11:40:52   MS
   neu GetEntryPos

      Rev 1.1   27 Nov 1992 11:30:08   MS
   neu InitFromResource

      Rev 1.0   27 Nov 1992 10:49:58   MS
   Initial revision.

 ------------------------------------------------------------------------*/

