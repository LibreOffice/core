/*************************************************************************
 *
 *  $RCSfile: svxbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:12 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#pragma hdrstop

#include "svxbox.hxx"

// -----------------------------------------------------------------------

SV_IMPL_PTRARR(SvxEntryLst, SvxBoxEntry*)

/*--------------------------------------------------------------------
     Beschreibung: Ein ListboxElement
 --------------------------------------------------------------------*/

SvxBoxEntry::SvxBoxEntry() :
    bModified(FALSE),
    bNew(FALSE),
    nId(LISTBOX_ENTRY_NOTFOUND)
{
}


SvxBoxEntry::SvxBoxEntry(const String& aNam, USHORT nIdx) :
    bModified(FALSE),
    bNew(FALSE),
    aName(aNam),
    nId(nIdx)
{
}


SvxBoxEntry::SvxBoxEntry(const SvxBoxEntry& rOld) :
    aName(rOld.aName),
    nId(rOld.nId),
    bNew(rOld.bNew),
    bModified(rOld.bModified)
{
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

SvxListBox::SvxListBox(Window* pParent, WinBits nBits) :
    ListBox(pParent, nBits)
{
    InitListBox();
}


SvxListBox::SvxListBox(Window* pParent, const ResId& rId):
    ListBox(pParent, rId)
{
    InitListBox();
}

/*--------------------------------------------------------------------
     Beschreibung: Basisklasse Dtor
 --------------------------------------------------------------------*/

__EXPORT SvxListBox::~SvxListBox()
{
    aEntryLst.DeleteAndDestroy(0,   aEntryLst.Count());
    aDelEntryLst.DeleteAndDestroy(0, aDelEntryLst.Count());
}

/*--------------------------------------------------------------------
     Beschreibung: Evtl. Liste aus der Ressource beachten
 --------------------------------------------------------------------*/

void SvxListBox::InitListBox()
{
    // Verwaltung fuer die Stringlist aus der Resource aufbauen
    USHORT nSize = GetEntryCount();
    for(USHORT i=0; i < nSize; ++i)
    {   const SvxBoxEntry* pTmp = new SvxBoxEntry(ListBox::GetEntry(i), i);
        const SvxBoxEntry* &rpTmp = pTmp;
        aEntryLst.Insert(rpTmp, aEntryLst.Count());
    }
}

/*--------------------------------------------------------------------
     Beschreibung: neue Eintraege verwalten
 --------------------------------------------------------------------*/

void SvxListBox::InsertNewEntry(const SvxBoxEntry& rEntry)
{
    SvxBoxEntry* pNew = new SvxBoxEntry(rEntry);
    pNew->bNew = TRUE;
    InsertSorted(pNew);
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag in die ListBox aufnehmen
 --------------------------------------------------------------------*/

void SvxListBox::InsertEntry(const SvxBoxEntry& rEntry, USHORT nPos)
{
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        SvxBoxEntry* pEntry = new SvxBoxEntry(rEntry);
        ListBox::InsertEntry(pEntry->aName, nPos);
        //const SvxBoxEntry* &rpEntry = pEntry;
        aEntryLst.C40_INSERT(SvxBoxEntry, pEntry, nPos);
    }
    else
        InsertSorted(new SvxBoxEntry(rEntry));
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag aus der Liste loeschen
 --------------------------------------------------------------------*/

void SvxListBox::RemoveEntry(USHORT nPos)
{
    if(nPos >= aEntryLst.Count())
        return;

    // Altes Element austragen
    SvxBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    ListBox::RemoveEntry(nPos);

    // keine neuen Eintraege in die Liste mit aufnehmen
    if(pEntry->bNew)
        return;

    // in DeleteListe eintragen
    aDelEntryLst.C40_INSERT(SvxBoxEntry, pEntry, aDelEntryLst.Count());
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag ueber konkretes Obkjekt loeschen
 --------------------------------------------------------------------*/

void SvxListBox::RemoveEntry(const SvxBoxEntry& rEntry)
{
    USHORT nPos = ListBox::GetEntryPos(rEntry.aName);
    RemoveEntry(nPos);
}

/*--------------------------------------------------------------------
     Beschreibung: Listen loeschen und Anzeige loeschen
 --------------------------------------------------------------------*/

void SvxListBox::Clear()
{
    ListBox::Clear();
    aEntryLst.DeleteAndDestroy(0, aEntryLst.Count());
    aDelEntryLst.DeleteAndDestroy(0, aDelEntryLst.Count());
}

/*--------------------------------------------------------------------
     Beschreibung: Position by Name
 --------------------------------------------------------------------*/

USHORT SvxListBox::GetEntryPos(const SvxBoxEntry& rEntry) const
{
    return ListBox::GetEntryPos(rEntry.aName);
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetEntry(USHORT nPos) const
{
    if(nPos < aEntryLst.Count())
        return *aEntryLst[nPos];
    else
        return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: aktullen Eintrag zurueckgeben
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetSelectEntry(USHORT nSelId) const
{
    String aName(ListBox::GetSelectEntry(nSelId));

    if(aName.Len() > 0)
    {
        for (USHORT i=0; i < aEntryLst.Count(); i++)
        {
            if(aEntryLst[i]->aName == aName )
                return *aEntryLst[i];
        }
    }
    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: modifizierte Eintraege
 --------------------------------------------------------------------*/

USHORT SvxListBox::GetModifiedCount() const
{
    USHORT nMod  = 0;
    USHORT nSize = aEntryLst.Count();
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bModified)
            nMod++;
    }
    return nMod;
}

/*--------------------------------------------------------------------
     Beschreibung: Modifizierte Eintraege behandeln
 --------------------------------------------------------------------*/

void SvxListBox::ModifyEntry(USHORT nPos, const String& rName)
{
    if(nPos >= aEntryLst.Count())
        return;

    SvxBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    aEntryLst[nPos]->aName      = rName;
    aEntryLst[nPos]->bModified  = TRUE;
    ListBox::RemoveEntry(nPos);

    InsertSorted(pEntry);
}

/*--------------------------------------------------------------------
     Beschreibung: alle modifizierten Eintraege bahandeln
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetModifiedEntry(USHORT nPos) const
{
    USHORT nSize = aEntryLst.Count();
    USHORT nMod  = 0;
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bModified)
        {   if(nMod == nPos)
                return *aEntryLst[i];
            nMod++;
        }
    }
    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: geloeschte Eintraege
 --------------------------------------------------------------------*/

USHORT SvxListBox::GetRemovedCount() const
{
    return aDelEntryLst.Count();
}


const SvxBoxEntry& SvxListBox::GetRemovedEntry(USHORT nPos) const
{
    if(nPos < aDelEntryLst.Count())
        return *aDelEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Neue Entries begutachten
 --------------------------------------------------------------------*/

USHORT SvxListBox::GetNewCount() const
{
    USHORT nNew = 0;
    USHORT nSize = aEntryLst.Count();
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bNew)
            nNew++;
    }
    return nNew;
}

/*--------------------------------------------------------------------
     Beschreibung:  Alle neuen Eintraege ueberpruefen
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetNewEntry(USHORT nPos) const
{
    USHORT nSize = aEntryLst.Count();
    USHORT nNew  = 0;
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bNew)
        {   if(nNew == nPos)
                return *aEntryLst[i];
            nNew++;
        }
    }
    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Sortiert einfuegen
 --------------------------------------------------------------------*/

void SvxListBox::InsertSorted(SvxBoxEntry* pEntry)
{
    ListBox::InsertEntry(pEntry->aName);
    USHORT nPos = ListBox::GetEntryPos(pEntry->aName);
    aEntryLst.C40_INSERT(SvxBoxEntry, pEntry, nPos);
}

/*--------------------------------------------------------------------
     Beschreibung: ComboBoxen mit Verwaltungseinheit
 --------------------------------------------------------------------*/

SvxComboBox::SvxComboBox(Window* pParent, WinBits nBits, USHORT nStyleBits) :
    ComboBox(pParent, nBits),
    nStyle(nStyleBits)
{
    InitComboBox();
}


SvxComboBox::SvxComboBox(Window* pParent, const ResId& rId, USHORT nStyleBits ):
    ComboBox(pParent, rId),
    nStyle(nStyleBits)
{
    InitComboBox();
}

/*--------------------------------------------------------------------
     Beschreibung: Basisklasse Dtor
 --------------------------------------------------------------------*/

__EXPORT SvxComboBox::~SvxComboBox()
{
    aEntryLst.DeleteAndDestroy(0,   aEntryLst.Count());
    aDelEntryLst.DeleteAndDestroy(0, aDelEntryLst.Count());
}

/*--------------------------------------------------------------------
     Beschreibung: Evtl. Liste aus der Ressource beachten
 --------------------------------------------------------------------*/

void SvxComboBox::InitComboBox()
{
    // Verwaltung fuer die Stringlist aus der Resource aufbauen
    USHORT nSize = GetEntryCount();
    for(USHORT i=0; i < nSize; ++i)
    {   const SvxBoxEntry* pTmp = new SvxBoxEntry(ComboBox::GetEntry(i), i);
        const SvxBoxEntry* &rpTmp = pTmp;
        aEntryLst.Insert(rpTmp, aEntryLst.Count());
    }
}

/*--------------------------------------------------------------------
     Beschreibung: neue Eintraege verwalten
 --------------------------------------------------------------------*/

void SvxComboBox::InsertNewEntry(const SvxBoxEntry& rEntry)
{
    SvxBoxEntry* pNew = new SvxBoxEntry(rEntry);
    pNew->bNew = TRUE;
    InsertSorted(pNew);
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag in die ComboBox aufnehmen
 --------------------------------------------------------------------*/

void SvxComboBox::InsertEntry(const SvxBoxEntry& rEntry)
{
    InsertSorted(new SvxBoxEntry(rEntry));
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag aus der Liste loeschen
 --------------------------------------------------------------------*/

void SvxComboBox::RemoveEntry(USHORT nPos)
{
    if(nPos >= aEntryLst.Count())
        return;

    // Altes Element austragen
    SvxBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    ComboBox::RemoveEntry(nPos);

    // keine neuen Eintraege in die Liste mit aufnehmen
    if(pEntry->bNew)
        return;

    // in DeleteListe eintragen
    aDelEntryLst.C40_INSERT(SvxBoxEntry, pEntry, aDelEntryLst.Count());
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag ueber konkretes Obkjekt loeschen
 --------------------------------------------------------------------*/

void SvxComboBox::RemoveEntry(const SvxBoxEntry& rEntry)
{
    USHORT nPos = ComboBox::GetEntryPos(rEntry.aName);
    RemoveEntry(nPos);
}

/*--------------------------------------------------------------------
     Beschreibung: Listen loeschen und Anzeige loeschen
 --------------------------------------------------------------------*/

void SvxComboBox::Clear()
{
    ComboBox::Clear();
    aEntryLst.DeleteAndDestroy(0, aEntryLst.Count());
    aDelEntryLst.DeleteAndDestroy(0, aDelEntryLst.Count());
}


/*--------------------------------------------------------------------
     Beschreibung: Position by Name
 --------------------------------------------------------------------*/

USHORT SvxComboBox::GetEntryPos(const SvxBoxEntry& rEntry) const
{
    return ComboBox::GetEntryPos(rEntry.aName);
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxComboBox::GetEntry(USHORT nPos) const
{
    if(nPos < aEntryLst.Count())
        return *aEntryLst[nPos];
    else
        return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: modifizierte Eintraege
 --------------------------------------------------------------------*/

USHORT SvxComboBox::GetModifiedCount() const
{
    USHORT nMod  = 0;
    USHORT nSize = aEntryLst.Count();
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bModified)
            nMod++;
    }
    return nMod;
}

/*--------------------------------------------------------------------
     Beschreibung: Modifizierte Eintraege behandeln
 --------------------------------------------------------------------*/

void SvxComboBox::ModifyEntry(USHORT nPos, const String& rName)
{
    if(nPos >= aEntryLst.Count())
        return;

    SvxBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    aEntryLst[nPos]->aName      = rName;
    aEntryLst[nPos]->bModified  = TRUE;
    ComboBox::RemoveEntry(nPos);

    InsertSorted(pEntry);
}

/*--------------------------------------------------------------------
     Beschreibung: alle modifizierten Eintraege bahandeln
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxComboBox::GetModifiedEntry(USHORT nPos) const
{
    USHORT nSize = aEntryLst.Count();
    USHORT nMod  = 0;
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bModified)
        {   if(nMod == nPos)
                return *aEntryLst[i];
            nMod++;
        }
    }
    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: geloeschte Eintraege
 --------------------------------------------------------------------*/

USHORT SvxComboBox::GetRemovedCount() const
{
    return aDelEntryLst.Count();
}


const SvxBoxEntry& SvxComboBox::GetRemovedEntry(USHORT nPos) const
{
    if(nPos < aDelEntryLst.Count())
        return *aDelEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Neue Entries begutachten
 --------------------------------------------------------------------*/

USHORT SvxComboBox::GetNewCount() const
{
    USHORT nNew = 0;
    USHORT nSize = aEntryLst.Count();
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bNew)
            nNew++;
    }
    return nNew;
}

/*--------------------------------------------------------------------
     Beschreibung:  Alle neuen Eintraege ueberpruefen
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxComboBox::GetNewEntry(USHORT nPos) const
{
    USHORT nSize = aEntryLst.Count();
    USHORT nNew  = 0;
    for(USHORT i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bNew)
        {   if(nNew == nPos)
                return *aEntryLst[i];
            nNew++;
        }
    }
    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Sortiert einfuegen
 --------------------------------------------------------------------*/

void SvxComboBox::InsertSorted(SvxBoxEntry* pEntry)
{
    ComboBox::InsertEntry(pEntry->aName);
    USHORT nPos = ComboBox::GetEntryPos(pEntry->aName);
    aEntryLst.C40_INSERT(SvxBoxEntry, pEntry, nPos);
}


/*--------------------------------------------------------------------
    Beschreibung: Je nach Option bestimmte Zeichen ausblenden
 --------------------------------------------------------------------*/

void __EXPORT SvxComboBox::KeyInput( const KeyEvent& rKEvt )
{
    sal_Unicode cChar = rKEvt.GetCharCode();

    if(nStyle & SVX_CBS_FILENAME)
    {
#ifdef MAC
        if( cChar == sal_Unicode( ':' ) )
            return;
#elif defined UNX
        if( cChar == sal_Unicode( '/' ) || cChar == sal_Unicode( ' ' ) )
            return;
#else
        if( cChar == sal_Unicode( ':' ) || cChar == sal_Unicode( '\\' ) ||
            cChar == sal_Unicode( '.' ) || cChar == sal_Unicode( ' ' ) )
            return;
#endif
    }
    ComboBox::KeyInput(rKEvt);
}

/*--------------------------------------------------------------------
    Beschreibung: Text nach Option konvertieren
 --------------------------------------------------------------------*/

String SvxComboBox::GetText() const
{
    String aTxt(ComboBox::GetText());

    if(nStyle & SVX_CBS_LOWER)
        return GetpApp()->GetAppInternational().Lower(aTxt);

    if(nStyle & SVX_CBS_UPPER)
        return GetpApp()->GetAppInternational().Upper(aTxt);

    return aTxt;
}


