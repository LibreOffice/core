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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <tools/debug.hxx>
#include <unotools/charclass.hxx>
#include <swtypes.hxx>
#include <swlbox.hxx>

using namespace nsSwComboBoxStyle;


SV_IMPL_PTRARR(SwEntryLst, SwBoxEntry*)

/*--------------------------------------------------------------------
     Beschreibung: Ein ListboxElement
 --------------------------------------------------------------------*/


SwBoxEntry::SwBoxEntry() :
    bModified(sal_False),
    bNew(sal_False),
    nId(LISTBOX_APPEND)
{
}


SwBoxEntry::SwBoxEntry(const String& aNam, sal_uInt16 nIdx) :
    bModified(sal_False),
    bNew(sal_False),
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



SwComboBox::SwComboBox(Window* pParent, const ResId& rId, sal_uInt16 nStyleBits ):
    ComboBox(pParent, rId),
    nStyle(nStyleBits)
{
    // Verwaltung fuer die Stringlist aus der Resource aufbauen
    sal_uInt16 nSize = GetEntryCount();
    for( sal_uInt16 i=0; i < nSize; ++i )
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


void SwComboBox::RemoveEntry(sal_uInt16 nPos)
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

sal_uInt16 SwComboBox::GetEntryPos(const SwBoxEntry& rEntry) const
{
    return ComboBox::GetEntryPos(rEntry.aName);
}

/*--------------------------------------------------------------------
     Beschreibung: Rund um die Entries
 --------------------------------------------------------------------*/


const SwBoxEntry& SwComboBox::GetEntry(sal_uInt16 nPos) const
{
    if(nPos < aEntryLst.Count())
        return *aEntryLst[nPos];

    return aDefault;
}

/*--------------------------------------------------------------------
     Beschreibung: geloeschte Eintraege
 --------------------------------------------------------------------*/


sal_uInt16 SwComboBox::GetRemovedCount() const
{
    return aDelEntryLst.Count();
}


const SwBoxEntry& SwComboBox::GetRemovedEntry(sal_uInt16 nPos) const
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
    sal_uInt16 nPos = ComboBox::GetEntryPos(pEntry->aName);
    aEntryLst.C40_INSERT(SwBoxEntry, pEntry, nPos);
}


/*--------------------------------------------------------------------
    Beschreibung: Je nach Option bestimmte Zeichen ausblenden
 --------------------------------------------------------------------*/


void SwComboBox::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nChar = rKEvt.GetCharCode();

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

void SwComboBox::SetText( const XubString& rStr )
{
    ComboBox::SetText(rStr);
    // MT: CallEventListeners stay protected for now. Probably not needed at all, as the implementation of VCLXAccessibelEdit/ComboBox should do it on text changed nowadays anyway. Verify.
    // CallEventListeners( VCLEVENT_EDIT_MODIFY );
}
