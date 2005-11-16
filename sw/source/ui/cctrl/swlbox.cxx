/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swlbox.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:11 $
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



