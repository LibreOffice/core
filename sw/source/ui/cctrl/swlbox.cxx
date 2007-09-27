/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swlbox.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:18:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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

using namespace nsSwComboBoxStyle;


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
    bModified(rOld.bModified),
    bNew(rOld.bNew),
    aName(rOld.aName),
    nId(rOld.nId)
{

}



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
#if defined UNX
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



