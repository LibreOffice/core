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

#ifdef _MSC_VER
#pragma hdrstop
#endif



#include <bf_sfx2/docfile.hxx>

#include "config.hxx"
#include "starmath.hrc"
namespace binfilter {


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
**	MACRO DEFINTION
**
**/

#define SF_SM20IDENT 0x03031963L
#define SF_IDENT	 0x30334D53L



/**************************************************************************/
/*
**
**	DATA DEFINITION
**
**/

long				SF_Ident = SF_IDENT;

/**************************************************************************/
/*
**
**	CLASS IMPLEMENTATION
**
**/

/*N*/ SmSym::SmSym() :
/*N*/ 	Name(C2S("unknown")),
/*N*/ 	Character('\0'),
/*N*/ 	pHashNext(0),
/*N*/ 	pSymSetManager(0),
/*N*/ 	bPredefined(FALSE),
/*N*/     bDocSymbol(FALSE),
/*N*/ 	aSetName(C2S("unknown"))
/*N*/ {
/*N*/ 	aExportName = Name;
/*N*/ 	Face.SetTransparent(TRUE);
/*N*/     Face.SetAlign(ALIGN_BASELINE);
/*N*/ }


/*N*/ SmSym::SmSym(const SmSym& rSymbol)
/*N*/ {
/*N*/ 	pSymSetManager = 0;
/*N*/     *this = rSymbol;
/*N*/ }


/*N*/ SmSym::SmSym(const String& rName, const Font& rFont, sal_Unicode aChar,
/*N*/ 			 const String& rSet, BOOL bIsPredefined)
/*N*/ {
/*N*/ 	Name		= aExportName	= rName;
/*N*/ 
/*N*/     Face        = rFont;
/*N*/ 	Face.SetTransparent(TRUE);
/*N*/     Face.SetAlign(ALIGN_BASELINE);
/*N*/ 
/*N*/     Character   = aChar;
/*N*/     if (RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet())
/*?*/         Character |= 0xF000;
/*N*/ 	aSetName	= rSet;
/*N*/ 	bPredefined	= bIsPredefined;
/*N*/     bDocSymbol  = FALSE;
/*N*/ 
/*N*/ 	pHashNext	   = 0;
/*N*/ 	pSymSetManager = 0;
/*N*/ }


/*N*/ SmSym& SmSym::operator = (const SmSym& rSymbol)
/*N*/ {
/*N*/ 	Name		= rSymbol.Name;
/*N*/ 	Face		= rSymbol.Face;
/*N*/ 	Character	= rSymbol.Character;
/*N*/ 	aSetName	= rSymbol.aSetName;
/*N*/ 	bPredefined	= rSymbol.bPredefined;
/*N*/     bDocSymbol  = rSymbol.bDocSymbol;
/*N*/ 	aExportName	= rSymbol.aExportName;
/*N*/ 
/*N*/ 	pHashNext = 0;
/*N*/ 
/*N*/     if (pSymSetManager)
/*?*/ 		pSymSetManager->SetModified(TRUE);
/*N*/ 
/*N*/ 	return *this;
/*N*/ }




/**************************************************************************/



/*N*/ SmSymSet::SmSymSet(const String& rName)
/*N*/ {
/*N*/ 	Name = rName;
/*N*/ 	SymbolList.Clear();
/*N*/ 
/*N*/ 	pSymSetManager = 0;
/*N*/ }




/*N*/ USHORT SmSymSet::AddSymbol(SmSym* pSymbol)
/*N*/ {
/*N*/ 	DBG_ASSERT(pSymbol, "Kein Symbol");
/*N*/ 
/*N*/     if (pSymbol)
/*N*/         pSymbol->SetSetName( GetName() );
/*N*/ 	SymbolList.Insert(pSymbol, LIST_APPEND);
/*N*/ 	DBG_ASSERT(SymbolList.GetPos(pSymbol) == SymbolList.Count() - 1,
/*N*/ 		"Sm : ... ergibt falschen return Wert");
/*N*/ 
/*N*/ 	if (pSymSetManager)
/*N*/ 		pSymSetManager->SetModified(TRUE);
/*N*/ 
/*N*/ 	return (USHORT) SymbolList.Count() - 1;
/*N*/ }







/**************************************************************************/

/*N*/ SmSymSetManager_Impl::SmSymSetManager_Impl(
/*N*/         SmSymSetManager &rMgr, USHORT HashTableSize ) :
/*N*/ 
/*N*/     rSymSetMgr    (rMgr)
/*N*/ {
/*N*/     NoSymbolSets    = 0;
/*N*/     NoHashEntries   = HashTableSize;
/*N*/     HashEntries     = new SmSym *[NoHashEntries];
/*N*/     memset( HashEntries, 0, sizeof(SmSym *) * NoHashEntries );
/*N*/     Modified        = FALSE;
/*N*/ }





/**************************************************************************/





/*N*/ UINT32 SmSymSetManager::GetHashIndex(const String& rSymbolName)
/*N*/ {
/*N*/     UINT32 x = 1;
/*N*/ 	for (xub_StrLen i = 0; i < rSymbolName.Len(); i++)
/*N*/         x += x * rSymbolName.GetChar(i) + i;
/*N*/ 
/*N*/ 	return x % pImpl->NoHashEntries;
/*N*/ }


/*N*/ void SmSymSetManager::EnterHashTable(SmSym& rSymbol)
/*N*/ {
/*N*/     int j = GetHashIndex( rSymbol.GetName() );
/*N*/     if (pImpl->HashEntries[j] == 0)
/*N*/         pImpl->HashEntries[j] = &rSymbol;
/*N*/     else
/*N*/     {
/*N*/         SmSym *p = pImpl->HashEntries[j];
/*N*/         while (p->pHashNext)
/*N*/             p = p->pHashNext;
/*N*/         p->pHashNext = &rSymbol;
/*N*/     }
/*N*/     rSymbol.pHashNext = 0;
/*N*/ }


/*N*/ void SmSymSetManager::EnterHashTable(SmSymSet& rSymbolSet)
/*N*/ {
/*N*/ 	for (int i = 0; i < rSymbolSet.GetCount(); i++)
/*N*/         EnterHashTable( *rSymbolSet.SymbolList.GetObject(i) );
/*N*/ }

/*N*/ void SmSymSetManager::FillHashTable()
/*N*/ {
/*N*/ 	if (pImpl->HashEntries)
/*N*/ 	{
/*N*/ 		memset( pImpl->HashEntries, 0, pImpl->NoHashEntries * sizeof(SmSym *) );
/*N*/ 
/*N*/ 		for (UINT32 i = 0; i < pImpl->NoSymbolSets; i++)
/*N*/ 			EnterHashTable( *GetSymbolSet( (USHORT) i ) );
/*N*/ 	}
/*N*/ }





/*N*/ SmSymSetManager::SmSymSetManager(USHORT HashTableSize)
/*N*/ {
/*N*/     pImpl = new SmSymSetManager_Impl( *this, HashTableSize );
/*N*/ }






/*N*/ USHORT SmSymSetManager::AddSymbolSet(SmSymSet* pSymbolSet)
/*N*/ {
/*N*/ 	if (pImpl->NoSymbolSets >= pImpl->SymbolSets.GetSize())
/*?*/ 		pImpl->SymbolSets.SetSize(pImpl->NoSymbolSets + 1);
/*N*/ 
/*N*/ 	pImpl->SymbolSets.Put(pImpl->NoSymbolSets++, pSymbolSet);
/*N*/ 
/*N*/ 	pSymbolSet->pSymSetManager = this;
/*N*/ 
/*N*/ 	for (int i = 0; i < pSymbolSet->GetCount(); i++)
/*?*/ 		pSymbolSet->SymbolList.GetObject(i)->pSymSetManager = this;
/*N*/ 
/*N*/ 	FillHashTable();
/*N*/ 	pImpl->Modified = TRUE;
/*N*/ 
/*N*/ 	return (USHORT) (pImpl->NoSymbolSets - 1);
/*N*/ }

/*N*/ void SmSymSetManager::ChangeSymbolSet(SmSymSet* pSymbolSet)
/*N*/ {
/*N*/ 	if (pSymbolSet)
/*N*/ 	{
/*N*/ 		FillHashTable();
/*N*/ 		pImpl->Modified = TRUE;
/*N*/ 	}
/*N*/ }



/*N*/ USHORT SmSymSetManager::GetSymbolSetPos(const String& rSymbolSetName) const
/*N*/ {
/*N*/ 	for (USHORT i = 0; i < pImpl->NoSymbolSets; i++)
/*N*/ 		if (pImpl->SymbolSets.Get(i)->GetName() == rSymbolSetName)
/*N*/ 			return (i);
/*N*/ 
/*N*/ 	return SYMBOLSET_NONE;
/*N*/ }

/*N*/ SmSym *SmSymSetManager::GetSymbolByName(const String& rSymbolName)
/*N*/ {
/*N*/     SmSym *pSym = pImpl->HashEntries[GetHashIndex(rSymbolName)];
/*N*/     while (pSym)
/*N*/     {
/*N*/         if (pSym->Name == rSymbolName)
/*N*/             break;
/*N*/         pSym = pSym->pHashNext;
/*N*/     }
/*N*/ 
/*N*/ 	return pSym;
/*N*/ }


/*N*/ void SmSymSetManager::AddReplaceSymbol( const SmSym &rSymbol )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }


/*N*/ USHORT SmSymSetManager::GetSymbolCount() const
/*N*/ { 
/*N*/     USHORT nRes = 0;
/*N*/     USHORT nSets = GetSymbolSetCount();
/*N*/     for (USHORT i = 0;  i < nSets;  ++i)
/*N*/         nRes += GetSymbolSet(i)->GetCount();
/*N*/     return nRes;
/*N*/ }


/*N*/ const SmSym * SmSymSetManager::GetSymbolByPos( USHORT nPos ) const
/*N*/ {
/*N*/     const SmSym *pRes = 0;
/*N*/ 
/*N*/     INT16 nIdx = 0;
/*N*/     USHORT nSets = GetSymbolSetCount();
/*N*/     USHORT i = 0;
/*N*/     while (i < nSets  &&  !pRes)
/*N*/     {
/*N*/         USHORT nEntries = GetSymbolSet(i)->GetCount();
/*N*/         if (nPos < nIdx + nEntries)
/*N*/             pRes = &GetSymbolSet(i)->GetSymbol( nPos - nIdx );
/*N*/         else
/*N*/             nIdx += nEntries;
/*N*/         ++i;
/*N*/     }
/*N*/ 
/*N*/     return pRes;
/*N*/ }


/*N*/ void SmSymSetManager::Load()
/*N*/ {
/*N*/     SmMathConfig &rCfg = *SM_MOD1()->GetConfig();
/*N*/ 
/*N*/     USHORT nCount = rCfg.GetSymbolCount();
/*N*/     USHORT i;
/*N*/     for (i = 0;  i < nCount;  ++i)
/*N*/     {
/*N*/         const SmSym *pSym = rCfg.GetSymbol(i);
/*N*/         if (pSym)
/*N*/         {
/*N*/             SmSymSet *pSymSet = 0;
/*N*/             const String &rSetName = pSym->GetSetName();
/*N*/             USHORT nSetPos = GetSymbolSetPos( rSetName );
/*N*/             if (SYMBOLSET_NONE != nSetPos)
/*N*/                 pSymSet = GetSymbolSet( nSetPos );
/*N*/             else
/*N*/             {
/*N*/                 pSymSet = new SmSymSet( rSetName );
/*N*/                 AddSymbolSet( pSymSet );
/*N*/             }
/*N*/ 
/*N*/             pSymSet->AddSymbol( new SmSym( *pSym ) );
/*N*/         }
/*N*/     }
/*N*/     // build HashTables
/*N*/     nCount = GetSymbolSetCount();
/*N*/     for (i = 0;  i < nCount;  ++i)
/*N*/         ChangeSymbolSet( GetSymbolSet( i ) );
/*N*/ 
/*N*/     if (0 == nCount)
/*N*/     {
/*N*/         DBG_ERROR( "no symbol set found" );
/*N*/         pImpl->Modified = FALSE;
/*N*/     }
/*N*/ }





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
