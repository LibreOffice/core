/*************************************************************************
 *
 *  $RCSfile: symbol.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:27 $
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

#pragma hdrstop

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_INIMGR_HXX //autogen
#include <sfx2/inimgr.hxx>
#endif

#include "symbol.hxx"
#include "view.hxx"
#include "utility.hxx"
#include "dialog.hxx"
#include "config.hxx"
#include "smmod.hxx"
#include "starmath.hrc"


using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

// Das hier muss auch mal alles "uberarbeitet werden. Insbesondere die nicht
// funktionierende und bei l"oschen/"andern von Symbolen nicht gepflegte
// Hash Tabelle!!!  Diese aktualisert sich erst im Wertzuweisungsoperator
// beim Verlassen des 'SmSymDefineDialog's!

/**************************************************************************/
/*
**
**  MACRO DEFINTION
**
**/

#define SF_SM20IDENT 0x03031963L
#define SF_IDENT     0x30334D53L


/**************************************************************************/
/*
**
**  DATA DEFINITION
**
**/

long                SF_Ident = SF_IDENT;


/**************************************************************************/
/*
**
**  CLASS IMPLEMENTATION
**
**/

SmSym::SmSym() :
    Name(C2S("unknown")),
    Character('\0'),
    pHashNext(0),
    pSymSetManager(0)
{
    Face.SetTransparent(TRUE);
}

SmSym::SmSym(const SmSym& rSymbol)
{
    Name      = rSymbol.Name;
    Face      = rSymbol.Face;
    Character = rSymbol.Character;

    pHashNext      = 0;
    pSymSetManager = 0;
}

SmSym::SmSym(const String& rName, const Font& rFont, sal_Unicode aChar)
{
    Name      = rName;
    Face      = rFont;
    Character = aChar;

    pHashNext      = 0;
    pSymSetManager = 0;
}


SmSym& SmSym::operator = (const SmSym& rSymbol)
{
    Name      = rSymbol.Name;
    Face      = rSymbol.Face;
    Character = rSymbol.Character;

    pHashNext = 0;

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return *this;
}

void SmSym::SetSymbolName(const String& rName)
{
    Name    = rName;

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);
}

SvStream& operator << (SvStream& rStream, const SmSym& rSymbol)
{
    rStream.WriteByteString(ExportString(rSymbol.Name));
    rStream << rSymbol.Face;

    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
    rStream << ByteString::ConvertFromUnicode( rSymbol.Character, eEnc );

    return rStream;
}

SvStream& operator >> (SvStream& rStream, SmSym& rSymbol)
{
    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
    rStream.ReadByteString(rSymbol.Name, eEnc);
    if (SF_Ident == SF_SM20IDENT)
        ReadSM20Font(rStream, rSymbol.Face);
    else
        rStream >> rSymbol.Face;
    sal_Char cTemp;
    rStream >> cTemp;
    rSymbol.Character = ByteString::ConvertToUnicode( cTemp, eEnc );

    return rStream;
}

/**************************************************************************/

SmSymSet::SmSymSet() :
    Name(C2S("unknown")),
    pSymSetManager(0)
{
    SymbolList.Clear();
}

SmSymSet::SmSymSet(const SmSymSet& rSymbolSet)
{
    Name = rSymbolSet.Name;
    SymbolList.Clear();

    pSymSetManager = 0;

    for (int i = 0; i < rSymbolSet.GetCount(); i++)
        AddSymbol(new SmSym(rSymbolSet.GetSymbol(i)));
}

SmSymSet::SmSymSet(const String& rName)
{
    Name = rName;
    SymbolList.Clear();

    pSymSetManager = 0;
}

SmSymSet::~SmSymSet()
{
    for (int i = 0; i < GetCount(); i++)
        delete SymbolList.GetObject(i);
}

SmSymSet& SmSymSet::operator = (const SmSymSet& rSymbolSet)
{
    int i;
    for (i = 0; i < GetCount(); i++)
        delete SymbolList.GetObject(i);

    Name = rSymbolSet.Name;
    SymbolList.Clear();

    for (i = 0; i < rSymbolSet.GetCount(); i++)
        AddSymbol(new SmSym(rSymbolSet.GetSymbol(i)));

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return *this;
}

void SmSymSet::SetName(String& rName)
{
    Name = rName;

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);
}

USHORT SmSymSet::AddSymbol(SmSym* pSymbol)
{
    DBG_ASSERT(pSymbol, "Kein Symbol");

    SymbolList.Insert(pSymbol, LIST_APPEND);
    DBG_ASSERT(SymbolList.GetPos(pSymbol) == SymbolList.Count() - 1,
        "Sm : ... ergibt falschen return Wert");

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return (USHORT) SymbolList.Count() - 1;
}

void SmSymSet::DeleteSymbol(USHORT SymbolNo)
{
    delete RemoveSymbol(SymbolNo);
}

SmSym * SmSymSet::RemoveSymbol(USHORT SymbolNo)
{
    DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");

    SmSym *pSym = SymbolList.GetObject(SymbolNo);
    SymbolList.Remove(SymbolNo);

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);

    return pSym;
}


void SmSymSet::RenameSymbol(USHORT SymbolNo, String& rName)
{
    DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");

    SymbolList.GetObject(SymbolNo)->SetSymbolName(rName);

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);
}

void SmSymSet::ReplaceSymbol(USHORT SymbolNo, SmSym& rSymbol)
{
    DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");

    *SymbolList.GetObject(SymbolNo) = rSymbol;

    if (pSymSetManager)
        pSymSetManager->SetModified(TRUE);
}

USHORT SmSymSet::GetSymbolPos(const String& rName)
{
    for (USHORT i = 0; i < GetCount(); i++)
        if (SymbolList.GetObject(i)->GetName() == rName)
            return (i);

    return SYMBOL_NONE;
}

SvStream& operator << (SvStream& rStream, const SmSymSet& rSymbolSet)
{
    rStream.WriteByteString(ExportString(rSymbolSet.Name));
    rStream << rSymbolSet.GetCount();

    for (int i = 0; i < rSymbolSet.GetCount(); i++)
        rStream << rSymbolSet.GetSymbol(i);

    return rStream;
}


SvStream& operator >> (SvStream& rStream, SmSymSet& rSymbolSet)
{
    USHORT      n;
    SmSym     *pSymbol;

    rStream.ReadByteString(rSymbolSet.Name, gsl_getSystemTextEncoding());
    rStream >> n;

    for (int i = 0; i < n; i++)
    {
        if ((pSymbol = new SmSym) == 0)
            break;

        rStream >> *pSymbol;
        rSymbolSet.AddSymbol(pSymbol);
    }

    return rStream;
}

/**************************************************************************/

void SmSymSetManager::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                              const SfxHint& rHint, const TypeId& rHintType)
{
    if (((SfxSimpleHint&)rHint).GetId() == HINT_CONFIGCHANGED)
    {
        SmViewShell *pViewSh = SmGetActiveView();
        if (pViewSh)
        {
            SfxDispatcher &rDis = *pViewSh->GetViewFrame()->GetDispatcher();
            rDis.Execute(SID_SAVESYMBOLS);
            rDis.Execute(SID_LOADSYMBOLS);
        }
    }
}

UINT32 SmSymSetManager::GetHashIndex(const String& rSymbolName)
{
    UINT32 x = 0;
    for (xub_StrLen i = 0; i < rSymbolName.Len(); i++)
        x += x * rSymbolName.GetChar(i);

    return x % NoHashEntries;
}

void SmSymSetManager::EnterHashTable(SmSymSet& rSymbolSet)
{
    for (int i = 0; i < rSymbolSet.GetCount(); i++)
    {
        int j = GetHashIndex(rSymbolSet.GetSymbol(i).GetName());
        if (HashEntries[j] == 0)
            HashEntries[j] = rSymbolSet.SymbolList.GetObject(i);
        else
        {
            SmSym *p = HashEntries[j];
            while (p->pHashNext)
                p = p->pHashNext;
            p->pHashNext = rSymbolSet.SymbolList.GetObject(i);
        }
        rSymbolSet.SymbolList.GetObject(i)->pHashNext = 0;
    }
}

void SmSymSetManager::FillHashTable()
{
    if (HashEntries)
    {
        memset(HashEntries, 0, NoHashEntries * sizeof(SmSym *));

        for (int i = 0; i < NoSymbolSets; i++)
            EnterHashTable(*GetSymbolSet(i));
    }
}

void SmSymSetManager::Init()
{
    SfxModule *p = SM_MOD1();
    SmModule *pp = (SmModule *) p;
    StartListening(*pp->GetConfig());
}

void SmSymSetManager::Exit()
{
    SfxModule *p = SM_MOD1();
    SmModule *pp = (SmModule *) p;
    EndListening(*pp->GetConfig());
}

SmSymSetManager::SmSymSetManager(UINT32 HashTableSize)
{
    SymbolSets.Clear();
    NoSymbolSets = 0;
    NoHashEntries = HashTableSize;
    HashEntries = new SmSym *[NoHashEntries];
    memset(HashEntries, 0, sizeof(SmSym *) * NoHashEntries);
    Modified = FALSE;
}

SmSymSetManager::SmSymSetManager(const SmSymSetManager& rSymbolSetManager)
{
    SymbolSets.Clear();
    NoSymbolSets = 0;
    NoHashEntries = rSymbolSetManager.NoHashEntries;
    HashEntries = new SmSym *[NoHashEntries];
    memset(HashEntries, 0, sizeof(SmSym *) * NoHashEntries);

    for (int i = 0; i < rSymbolSetManager.GetCount(); i++)
        AddSymbolSet(new SmSymSet(*rSymbolSetManager.GetSymbolSet(i)));

    Modified = rSymbolSetManager.Modified;
}

SmSymSetManager::~SmSymSetManager()
{
    for (int i = 0; i< NoSymbolSets; i++)
        delete SymbolSets.Get(i);

    delete HashEntries;
}

SmSymSetManager& SmSymSetManager::operator = (const SmSymSetManager& rSymbolSetManager)
{
    int i;
    for (i = 0; i< NoSymbolSets; i++)
        delete SymbolSets.Get(i);

    SymbolSets.Clear();
    NoSymbolSets = 0;

    for (i = 0; i < rSymbolSetManager.GetCount(); i++)
        AddSymbolSet(new SmSymSet(*rSymbolSetManager.GetSymbolSet(i)));

    Modified = rSymbolSetManager.Modified;

    return *this;
}

USHORT SmSymSetManager::AddSymbolSet(SmSymSet* pSymbolSet)
{
    if (NoSymbolSets >= SymbolSets.GetSize())
        SymbolSets.SetSize(NoSymbolSets + 1);

    SymbolSets.Put(NoSymbolSets++, pSymbolSet);

    pSymbolSet->pSymSetManager = this;

    for (int i = 0; i < pSymbolSet->GetCount(); i++)
        pSymbolSet->SymbolList.GetObject(i)->pSymSetManager = this;

    FillHashTable();
    Modified = TRUE;

    return NoSymbolSets - 1;
}

void SmSymSetManager::ChangeSymbolSet(SmSymSet* pSymbolSet)
{
    if (pSymbolSet)
    {
        FillHashTable();
        Modified = TRUE;
    }
}

void SmSymSetManager::DeleteSymbolSet(USHORT SymbolSetNo)
{
    delete SymbolSets.Get(SymbolSetNo);
    NoSymbolSets--;

    for (int i = SymbolSetNo; i < NoSymbolSets; i++)
        SymbolSets.Put(i, SymbolSets.Get(i + 1));

    FillHashTable();

    Modified = TRUE;
}


USHORT SmSymSetManager::GetSymbolSetPos(const String& rSymbolSetName) const
{
    for (USHORT i = 0; i < NoSymbolSets; i++)
        if (SymbolSets.Get(i)->GetName() == rSymbolSetName)
            return (i);

    return SYMBOLSET_NONE;
}


SmSym *SmSymSetManager::GetSymbol(const String& rSymbolName)
{
    SmSym *p = HashEntries[GetHashIndex(rSymbolName)];
    while (p)
    {
        if (p->Name == rSymbolName)
            break;
        p = p->pHashNext;
    }
    return p;
}

void SmSymSetManager::Load(const String &rURL)
{
    if (aStreamName != rURL)
    {
        for (int i = 0; i< NoSymbolSets; i++)
            delete SymbolSets.Get(i);

        SymbolSets.Clear();
        NoSymbolSets = 0;

        aStreamName = rURL;

        BOOL bExist = FALSE;
        try
        {
            bExist = ::ucb::Content( aStreamName, uno::Reference< XCommandEnvironment >()).isDocument();
        }
        catch(...){}
        if (bExist)
        {
            SvFileStream aStream(aStreamName, STREAM_READ);

            if (aStream.IsOpen())
            {
                aStream >> *this;
                Modified = FALSE;
                return;
            }
        }

        SfxModule *p = SM_MOD1();
        SmModule *pp = (SmModule *) p;

        if ( pp->GetConfig()->IsWarnNoSymbols() )
        {
            ErrorBox aErrorBox( NULL, SmResId( RID_READSYMBOLERROR ) );
            String aString( aErrorBox.GetMessText() );
            String aIniFile = SFX_INIMANAGER()->SubstPathVars( aStreamName );
            aString.SearchAndReplaceAscii( "%FILE%", aIniFile );
            aErrorBox.SetMessText( aString );
            aErrorBox.Execute();

            Modified = FALSE;
            pp->GetConfig()->SetWarnNoSymbols(FALSE);
        }
    }
}

void SmSymSetManager::Save()
{
    if (Modified)
    {
        SvFileStream    aStream(aStreamName, STREAM_WRITE);

        if (aStream.IsOpen())
        {
            aStream << *this;
            Modified = FALSE;
        }
        else
        {
            ErrorBox aErrorBox( NULL, SmResId(RID_WRITESYMBOLERROR));
            String   aString (aErrorBox.GetMessText());
            USHORT   nPos = aString.SearchAscii("%FILE%");

            aString.Erase(nPos, 6);
            aString.Insert(aStreamName, nPos);
            aErrorBox.SetMessText(aString);
            aErrorBox.Execute();
        }
    }
}


SvStream& operator << (SvStream& rStream, SmSymSetManager& rSymbolSetManager)
{
    rStream << (long)SF_IDENT << (USHORT) rSymbolSetManager.NoSymbolSets;

    for (int i = 0; i < rSymbolSetManager.NoSymbolSets; i++)
        rStream << *rSymbolSetManager.GetSymbolSet(i);

    return rStream;
}

SvStream& operator >> (SvStream& rStream, SmSymSetManager& rSymbolSetManager)
{
    rStream >> SF_Ident;
    if (SF_Ident == SF_IDENT  ||  SF_Ident == SF_SM20IDENT)
    {
        USHORT n;
        rStream >> n;

        if (rSymbolSetManager.HashEntries)
            memset(rSymbolSetManager.HashEntries, 0,
                   rSymbolSetManager.NoHashEntries * sizeof(SmSym *));

        for (int i = 0; i < n; i++)
        {
            SmSymSet *pSymbolSet;

            if ((pSymbolSet = new SmSymSet) == 0)
                break;

            rStream >> *pSymbolSet;
            rSymbolSetManager.AddSymbolSet(pSymbolSet);
        }
    }

    SF_Ident = SF_IDENT;

    return rStream;
}

void ReadSM20SymSet(SvStream *pStream, SmSymSet *pSymbolSet)
{
    SF_Ident = SF_SM20IDENT;
    *pStream >> *pSymbolSet;
    SF_Ident = SF_IDENT;
}


