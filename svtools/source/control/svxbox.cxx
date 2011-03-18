/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <svtools/svxbox.hxx>
#include <unotools/charclass.hxx>

// -----------------------------------------------------------------------

SV_IMPL_PTRARR(SvxEntryLst, SvxBoxEntry*)

/*--------------------------------------------------------------------
     Beschreibung: Ein ListboxElement
 --------------------------------------------------------------------*/

SvxBoxEntry::SvxBoxEntry() :
    nId(LISTBOX_ENTRY_NOTFOUND),
    bModified(sal_False),
    bNew(sal_False)
{
}


SvxBoxEntry::SvxBoxEntry(const String& aNam, sal_uInt16 nIdx) :
    aName(aNam),
    nId(nIdx),
    bModified(sal_False),
    bNew(sal_False)
{
}


SvxBoxEntry::SvxBoxEntry(const SvxBoxEntry& rOld) :
    aName(rOld.aName),
    nId(rOld.nId),
    bModified(rOld.bModified),
    bNew(rOld.bNew)
{
}

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

SvxListBox::~SvxListBox()
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
    sal_uInt16 nSize = GetEntryCount();
    for(sal_uInt16 i=0; i < nSize; ++i)
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
    pNew->bNew = sal_True;
    InsertSorted(pNew);
}

/*--------------------------------------------------------------------
     Beschreibung: Eintrag in die ListBox aufnehmen
 --------------------------------------------------------------------*/

void SvxListBox::InsertEntry(const SvxBoxEntry& rEntry, sal_uInt16 nPos)
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

void SvxListBox::RemoveEntry(sal_uInt16 nPos)
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
    sal_uInt16 nPos = ListBox::GetEntryPos(rEntry.aName);
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

sal_uInt16 SvxListBox::GetEntryPos(const SvxBoxEntry& rEntry) const
{
    return ListBox::GetEntryPos(rEntry.aName);
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetSvxBoxEntry(sal_uInt16 nPos) const
{
    if(nPos < aEntryLst.Count())
        return *aEntryLst[nPos];
    else
        return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: aktullen Eintrag zurueckgeben
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetSelectSvxBoxEntry(sal_uInt16 nSelId) const
{
    String aName(ListBox::GetSelectEntry(nSelId));

    if(aName.Len() > 0)
    {
        for (sal_uInt16 i=0; i < aEntryLst.Count(); i++)
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

sal_uInt16 SvxListBox::GetModifiedCount() const
{
    sal_uInt16 nMod  = 0;
    sal_uInt16 nSize = aEntryLst.Count();
    for(sal_uInt16 i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bModified)
            nMod++;
    }
    return nMod;
}

/*--------------------------------------------------------------------
     Beschreibung: Modifizierte Eintraege behandeln
 --------------------------------------------------------------------*/

void SvxListBox::ModifyEntry(sal_uInt16 nPos, const String& rName)
{
    if(nPos >= aEntryLst.Count())
        return;

    SvxBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    aEntryLst[nPos]->aName      = rName;
    aEntryLst[nPos]->bModified  = sal_True;
    ListBox::RemoveEntry(nPos);

    InsertSorted(pEntry);
}

/*--------------------------------------------------------------------
     Beschreibung: alle modifizierten Eintraege bahandeln
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetModifiedEntry(sal_uInt16 nPos) const
{
    sal_uInt16 nSize = aEntryLst.Count();
    sal_uInt16 nMod  = 0;
    for(sal_uInt16 i=0; i < nSize; ++i)
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

sal_uInt16 SvxListBox::GetRemovedCount() const
{
    return aDelEntryLst.Count();
}


const SvxBoxEntry& SvxListBox::GetRemovedEntry(sal_uInt16 nPos) const
{
    if(nPos < aDelEntryLst.Count())
        return *aDelEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Neue Entries begutachten
 --------------------------------------------------------------------*/

sal_uInt16 SvxListBox::GetNewCount() const
{
    sal_uInt16 nNew = 0;
    sal_uInt16 nSize = aEntryLst.Count();
    for(sal_uInt16 i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bNew)
            nNew++;
    }
    return nNew;
}

/*--------------------------------------------------------------------
     Beschreibung:  Alle neuen Eintraege ueberpruefen
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxListBox::GetNewEntry(sal_uInt16 nPos) const
{
    sal_uInt16 nSize = aEntryLst.Count();
    sal_uInt16 nNew  = 0;
    for(sal_uInt16 i=0; i < nSize; ++i)
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
    sal_uInt16 nPos = ListBox::GetEntryPos(pEntry->aName);
    aEntryLst.C40_INSERT(SvxBoxEntry, pEntry, nPos);
}

/*--------------------------------------------------------------------
     Beschreibung: ComboBoxen mit Verwaltungseinheit
 --------------------------------------------------------------------*/

SvxComboBox::SvxComboBox(Window* pParent, WinBits nBits, sal_uInt16 nStyleBits) :
    ComboBox(pParent, nBits),
    nStyle(nStyleBits)
{
    InitComboBox();
}


SvxComboBox::SvxComboBox(Window* pParent, const ResId& rId, sal_uInt16 nStyleBits ):
    ComboBox(pParent, rId),
    nStyle(nStyleBits)
{
    InitComboBox();
}

/*--------------------------------------------------------------------
     Beschreibung: Basisklasse Dtor
 --------------------------------------------------------------------*/

SvxComboBox::~SvxComboBox()
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
    sal_uInt16 nSize = GetEntryCount();
    for(sal_uInt16 i=0; i < nSize; ++i)
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
    pNew->bNew = sal_True;
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

void SvxComboBox::RemoveEntry(sal_uInt16 nPos)
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
    sal_uInt16 nPos = ComboBox::GetEntryPos(rEntry.aName);
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

sal_uInt16 SvxComboBox::GetEntryPos(const SvxBoxEntry& rEntry) const
{
    return ComboBox::GetEntryPos(rEntry.aName);
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxComboBox::GetEntry(sal_uInt16 nPos) const
{
    if(nPos < aEntryLst.Count())
        return *aEntryLst[nPos];
    else
        return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: modifizierte Eintraege
 --------------------------------------------------------------------*/

sal_uInt16 SvxComboBox::GetModifiedCount() const
{
    sal_uInt16 nMod  = 0;
    sal_uInt16 nSize = aEntryLst.Count();
    for(sal_uInt16 i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bModified)
            nMod++;
    }
    return nMod;
}

/*--------------------------------------------------------------------
     Beschreibung: Modifizierte Eintraege behandeln
 --------------------------------------------------------------------*/

void SvxComboBox::ModifyEntry(sal_uInt16 nPos, const String& rName)
{
    if(nPos >= aEntryLst.Count())
        return;

    SvxBoxEntry* pEntry = aEntryLst[nPos];
    aEntryLst.Remove(nPos, 1);
    aEntryLst[nPos]->aName      = rName;
    aEntryLst[nPos]->bModified  = sal_True;
    ComboBox::RemoveEntry(nPos);

    InsertSorted(pEntry);
}

/*--------------------------------------------------------------------
     Beschreibung: alle modifizierten Eintraege bahandeln
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxComboBox::GetModifiedEntry(sal_uInt16 nPos) const
{
    sal_uInt16 nSize = aEntryLst.Count();
    sal_uInt16 nMod  = 0;
    for(sal_uInt16 i=0; i < nSize; ++i)
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

sal_uInt16 SvxComboBox::GetRemovedCount() const
{
    return aDelEntryLst.Count();
}


const SvxBoxEntry& SvxComboBox::GetRemovedEntry(sal_uInt16 nPos) const
{
    if(nPos < aDelEntryLst.Count())
        return *aDelEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: Neue Entries begutachten
 --------------------------------------------------------------------*/

sal_uInt16 SvxComboBox::GetNewCount() const
{
    sal_uInt16 nNew = 0;
    sal_uInt16 nSize = aEntryLst.Count();
    for(sal_uInt16 i=0; i < nSize; ++i)
    {   if(aEntryLst[i]->bNew)
            nNew++;
    }
    return nNew;
}

/*--------------------------------------------------------------------
     Beschreibung:  Alle neuen Eintraege ueberpruefen
 --------------------------------------------------------------------*/

const SvxBoxEntry& SvxComboBox::GetNewEntry(sal_uInt16 nPos) const
{
    sal_uInt16 nSize = aEntryLst.Count();
    sal_uInt16 nNew  = 0;
    for(sal_uInt16 i=0; i < nSize; ++i)
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
    sal_uInt16 nPos = ComboBox::GetEntryPos(pEntry->aName);
    aEntryLst.C40_INSERT(SvxBoxEntry, pEntry, nPos);
}


/*--------------------------------------------------------------------
    Beschreibung: Je nach Option bestimmte Zeichen ausblenden
 --------------------------------------------------------------------*/

void SvxComboBox::KeyInput( const KeyEvent& rKEvt )
{
    sal_Unicode cChar = rKEvt.GetCharCode();

    if(nStyle & SVX_CBS_FILENAME)
    {
#if defined UNX
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
    CharClass aCharClass( Application::GetSettings().GetLocale() );

    if(nStyle & SVX_CBS_LOWER)
        return aCharClass.lower(aTxt);

    if(nStyle & SVX_CBS_UPPER)
        return aCharClass.upper(aTxt);

    return aTxt;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
