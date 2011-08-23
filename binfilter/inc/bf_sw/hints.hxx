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
#ifndef _HINTS_HXX
#define _HINTS_HXX

#include <tools/table.hxx>
#include <swatrset.hxx>
class OutputDevice; 
namespace binfilter {

class SwFmt;

class SwTable;
class SwNode;
class SwNodes;
class SwCntntNode;
class SwPageFrm;
class SwFrm;
class SwTxtNode;
class SwHistory;

// Basis-Klasse fuer alle Message-Hints:
//	"Overhead" vom SfxPoolItem wird hier behandelt
class SwMsgPoolItem : public SfxPoolItem
{
public:
    SwMsgPoolItem( USHORT nWhich );

    // "Overhead" vom SfxPoolItem
     virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
};


// ---------------------------------------
// SwPtrMsgPoolItem (altes SwObjectDying!)
// ---------------------------------------

class SwPtrMsgPoolItem : public SwMsgPoolItem
{
public:
    void * pObject;

    SwPtrMsgPoolItem( USHORT nId, void * pObj )
        : SwMsgPoolItem( nId ), pObject( pObj )
    {}
};



/*
 * SwFmtChg wird verschickt, wenn ein Format gegen ein anderes
 * Format ausgewechselt worden ist. Es werden immer 2. Hints verschickt,
 * das alte und neue Format.
 */
class SwFmtChg: public SwMsgPoolItem
{
public:
    SwFmt *pChangedFmt;
    SwFmtChg( SwFmt *pFmt );
};


class SwInsChr: public SwMsgPoolItem
{
public:
    xub_StrLen nPos;

    SwInsChr( USHORT nP );
};

class SwInsTxt: public SwMsgPoolItem
{
public:
    xub_StrLen nPos;
    xub_StrLen nLen;

    SwInsTxt( xub_StrLen nP, xub_StrLen nL );
};

class SwDelChr: public SwMsgPoolItem
{
public:
    xub_StrLen nPos;

    SwDelChr( xub_StrLen nP );
};

class SwDelTxt: public SwMsgPoolItem
{
public:
    xub_StrLen nStart;
    xub_StrLen nLen;

    SwDelTxt( xub_StrLen nS, xub_StrLen nL );
};

class SwUpdateAttr: public SwMsgPoolItem
{
public:
    xub_StrLen nStart;
    xub_StrLen nEnd;
    USHORT nWhichAttr;
    SwUpdateAttr( xub_StrLen nS, xub_StrLen nE, USHORT nW );
};


// SwRefMarkFldUpdate wird verschickt, wenn sich die ReferenzMarkierungen
// Updaten sollen. Um Seiten-/KapitelNummer feststellen zu koennen, muss
// der akt. Frame befragt werden. Dafuer wird das akt. OutputDevice benoetigt.

// SwDocPosUpdate wird verschickt, um zu signalisieren, dass nur die
// Frames ab oder bis zu einer bestimmten dokument-globalen Position
// geupdated werden brauchen. Zur Zeit wird dies nur beim Updaten
// von Seitennummernfeldern benoetigt.

class SwDocPosUpdate : public SwMsgPoolItem
{
public:
    const long nDocPos;
    SwDocPosUpdate( const long nDocPos );
};

// SwTableFmlUpdate wird verschickt, wenn sich die Tabelle neu berechnen soll
// JP 16.02.99: oder wenn die Tabelle selbst gemergt oder gesplittet wird
enum TableFmlUpdtFlags { TBL_CALC = 0,
                         TBL_BOXNAME,
                         TBL_BOXPTR,
                         TBL_RELBOXNAME,
                         TBL_MERGETBL,
                         TBL_SPLITTBL
                       };
class SwTableFmlUpdate : public SwMsgPoolItem
{
public:
    const SwTable* pTbl;		// Pointer auf die zu aktuelle Tabelle
    union {
        const SwTable* pDelTbl;		// Merge: Ptr auf die zu loeschende Tabelle
        const String* pNewTblNm;	// Split: der Name der neuen Tabelle
    } DATA;
    SwHistory* pHistory;
    USHORT nSplitLine;			// Split: ab dieser BaseLine wird gespl.
    TableFmlUpdtFlags eFlags;
    BOOL bModified : 1;
    BOOL bBehindSplitLine : 1;

    SwTableFmlUpdate( const SwTable* );
};


class SwAutoFmtGetDocNode: public SwMsgPoolItem
{
public:
    const SwCntntNode* pCntntNode;
    const SwNodes* pNodes;

    SwAutoFmtGetDocNode( const SwNodes* pNds );
};

/*
 * SwAttrSetChg wird verschicht, wenn sich in dem SwAttrSet rTheChgdSet
 * etwas veraendert hat. Es werden immer 2. Hints
 * verschickt, die alten und neuen Items in dem rTheChgdSet.
 */
class SwAttrSetChg: public SwMsgPoolItem
{
    BOOL bDelSet;
    SwAttrSet* pChgSet;				// was sich veraendert hat
    const SwAttrSet* pTheChgdSet;	// wird nur zum Vergleichen gebraucht !!
public:
    SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet );
    SwAttrSetChg( const SwAttrSetChg& );
    ~SwAttrSetChg();

    // was sich veraendert hat
    const SwAttrSet* GetChgSet() const	{ return pChgSet; }
          SwAttrSet* GetChgSet() 		{ return pChgSet; }

    // wo es sich geaendert hat
    const SwAttrSet* GetTheChgdSet() const	{ return pTheChgdSet; }

    SfxItemState GetItemState( USHORT nWhichIn, BOOL bSrchInParent = TRUE,
                               const SfxPoolItem **ppItem = 0 ) const
    {	return pChgSet->GetItemState( nWhichIn, bSrchInParent, ppItem ); }

    USHORT Count() const { return pChgSet->Count(); }
    void ClearItem( USHORT nWhichIn = 0 )
    {
#ifndef DBG_UTIL
        pChgSet->ClearItem( nWhichIn );
#else
        ;
#endif
    }
};

class SwCondCollCondChg: public SwMsgPoolItem
{
public:
    SwFmt *pChangedFmt;
    SwCondCollCondChg( SwFmt *pFmt );
};

class SwVirtPageNumInfo: public SwMsgPoolItem
{
    const SwPageFrm *pPage;
    const SwPageFrm *pOrigPage;
    const SwFrm 	*pFrm;		//An einem Absatz/Tabelle koennen mehrere
                                //Attribute sitzen. Der Frame muss dann
                                //muss dann letztlich bei bestimmen
                                //welches Attribut gilt und um welche physikalische
                                //Seite es sich handelt.
public:
    SwVirtPageNumInfo( const SwPageFrm *pPg );

    const SwPageFrm *GetPage()			{ return pPage;    }
    const SwPageFrm *GetOrigPage()		{ return pOrigPage;}
    const SwFrm *GetFrm()				{ return pFrm; }
    void  SetInfo( const SwPageFrm *pPg,
                   const SwFrm *pF	) 	{ pFrm = pF, pPage = pPg; }
};


DECLARE_TABLE( SwTxtNodeTable, SwTxtNode* )

class SwNumRuleInfo : public SwMsgPoolItem
{
    SwTxtNodeTable aList;
    const String& rName;
public:
    SwNumRuleInfo( const String& rRuleName );

    const String& GetName() const { return rName; }
    void AddNode( SwTxtNode& rNd );

    // erzeuge die Liste aller Nodes der NumRule in dem angegebenem Doc
    // Der Code steht im docnum.cxx
    void MakeList( SwDoc& rDoc );
    SwTxtNodeTable& GetList() const { return (SwTxtNodeTable&)aList; }
};



class SwStringMsgPoolItem : public SwMsgPoolItem
{
    String sStr;
public:
    
    const String& GetString() const { return sStr; }

    SwStringMsgPoolItem( USHORT nId, const String& rStr )
        : SwMsgPoolItem( nId ), sStr( rStr )
    {}
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
