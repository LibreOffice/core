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

#ifndef _DOCFLD_HXX
#define _DOCFLD_HXX

#include <calc.hxx>			// fuer SwHash
#include <doc.hxx>
namespace binfilter {

class SwTxtFld;
class SwIndex;
class SwNodeIndex;
class SwCntntNode;
class SwCntntFrm;
class SwSectionNode;
class SwSection;
class SwTxtTOXMark;
class SwTableBox;
class SwTxtINetFmt;
class SwFlyFrmFmt;

// Update an den Expression Feldern
class _SetGetExpFld
{
    ULONG nNode;
    xub_StrLen nCntnt;
    union {
        const SwTxtFld* pTxtFld;
        const SwSection* pSection;
        const SwPosition* pPos;
        const SwTxtTOXMark* pTxtTOX;
        const SwTableBox* pTBox;
        const SwTxtINetFmt* pTxtINet;
        const SwFlyFrmFmt* pFlyFmt;
    } CNTNT;
    enum _SetGetExpFldType
        {
            TEXTFIELD, TEXTTOXMARK, SECTIONNODE, CRSRPOS, TABLEBOX,
            TEXTINET, FLYFRAME
        } eSetGetExpFldType;

public:
    _SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld = 0,
                    const SwIndex* pIdx = 0 );

    _SetGetExpFld( const SwSectionNode& rSectNode,
                    const SwPosition* pPos = 0  );

    BOOL operator==( const _SetGetExpFld& rFld ) const
    {	return nNode == rFld.nNode && nCntnt == rFld.nCntnt &&
                ( !CNTNT.pTxtFld || !rFld.CNTNT.pTxtFld ||
                    CNTNT.pTxtFld == rFld.CNTNT.pTxtFld ); }
    BOOL operator<( const _SetGetExpFld& rFld ) const;

     const SwTxtFld* GetFld() const
        { return TEXTFIELD == eSetGetExpFldType ? CNTNT.pTxtFld : 0; }
     const SwSection* GetSection() const
         { return SECTIONNODE == eSetGetExpFldType ? CNTNT.pSection : 0; }
     ULONG GetNode() const { return nNode; }
    const void* GetPointer() const { return CNTNT.pTxtFld; }
    const SwNode* GetNodeFromCntnt() const;
    xub_StrLen GetCntPosFromCntnt() const;
};

typedef _SetGetExpFld* _SetGetExpFldPtr;
SV_DECL_PTRARR_SORT_DEL( _SetGetExpFlds, _SetGetExpFldPtr, 0, 10 )


// Struktur zum Speichern der Strings aus SetExp-String-Feldern
struct _HashStr : public SwHash
{
    String aSetStr;
    _HashStr( const String& rName, const String& rText, _HashStr* = 0 );
};

struct SwCalcFldType : public SwHash
{
    const SwFieldType* pFldType;

    SwCalcFldType( const String& rStr, const SwFieldType* pFldTyp )
        : SwHash( rStr ), pFldType( pFldTyp )
    {}
};

// Suche nach dem String, der unter dem Namen in der HashTabelle abgelegt
// wurde
void LookString( SwHash** ppTbl, USHORT nSize, const String& rName,
                    String& rRet, USHORT* pPos = 0 );


// --------

const int GETFLD_ALL		= 3;		// veroderte Flags !!
const int GETFLD_CALC		= 1;
const int GETFLD_EXPAND		= 2;

class SwDocUpdtFld
{
    _SetGetExpFlds* pFldSortLst;	// akt. Field-Liste zum Calculieren
    SwCalcFldType*  aFldTypeTable[ TBLSZ ];

// noch eine weitere Optimierung - wird z.Z. nicht angesprochen!
    long nFldUpdtPos;				// ab dieser Position mit Update starten
    SwCntntNode* pCNode;			// der TxtNode zur UpdatePos.

    ULONG nNodes;					// sollte die NodesAnzahl unterschiedlich sein
    BYTE nFldLstGetMode;

    BOOL bInUpdateFlds : 1;			// zur Zeit laeuft ein UpdateFlds,
    BOOL bFldsDirty : 1;			// irgendwelche Felder sind ungueltig

    void _MakeFldList( SwDoc& pDoc, int eGetMode );
    void GetBodyNode( const SwTxtFld& , USHORT nFldWhich );
    void GetBodyNode( const SwSectionNode&);
public:
    SwDocUpdtFld();
    ~SwDocUpdtFld();

    const _SetGetExpFlds* GetSortLst() const { return pFldSortLst; }

    void MakeFldList( SwDoc& rDoc, int bAll, int eGetMode )
    {
        if( !pFldSortLst || bAll || !( eGetMode & nFldLstGetMode ) ||
            rDoc.GetNodes().Count() != nNodes )
            _MakeFldList( rDoc, eGetMode );
    }

    void InsDelFldInFldLst( BOOL bIns, const SwTxtFld& rFld );

    void InsertFldType( const SwFieldType& rType );

    BOOL IsInUpdateFlds() const			{ return bInUpdateFlds; }
    void SetInUpdateFlds( BOOL b ) 		{ bInUpdateFlds = b; }

    BOOL IsFieldsDirty() const			{ return bFldsDirty; }
    void SetFieldsDirty( BOOL b ) 		{ bFldsDirty = b; }

    SwHash**	GetFldTypeTable() const { return (SwHash**)aFldTypeTable; }
};


} //namespace binfilter
#endif	// _DOCFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
