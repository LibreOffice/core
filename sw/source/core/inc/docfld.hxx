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

#include <calc.hxx>
#include <o3tl/sorted_vector.hxx>

class SwTxtFld;
class SwIndex;
class SwNodeIndex;
class SwCntntFrm;
class SwSectionNode;
class SwSection;
class SwTxtTOXMark;
class SwTableBox;
class SwTxtINetFmt;
class SwFlyFrmFmt;
class SwDoc;
class SwNode;
struct SwPosition;

// Update expression fields
class _SetGetExpFld
{
    sal_uLong nNode;
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

    _SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtINetFmt& rINet,
                    const SwIndex* pIdx = 0 );

    _SetGetExpFld( const SwSectionNode& rSectNode,
                    const SwPosition* pPos = 0  );

    _SetGetExpFld( const SwTableBox& rTableBox,
                    const SwPosition* pPos = 0  );

    _SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtTOXMark& rTOX,
                    const SwIndex* pIdx );

    _SetGetExpFld( const SwPosition& rPos );

    _SetGetExpFld( const SwFlyFrmFmt& rFlyFmt, const SwPosition* pPos = 0 );

    bool operator==( const _SetGetExpFld& rFld ) const
        {   return nNode == rFld.nNode && nCntnt == rFld.nCntnt &&
                ( !CNTNT.pTxtFld || !rFld.CNTNT.pTxtFld ||
                    CNTNT.pTxtFld == rFld.CNTNT.pTxtFld ); }
    bool operator<( const _SetGetExpFld& rFld ) const;

    const SwTxtFld* GetFld() const
        { return TEXTFIELD == eSetGetExpFldType ? CNTNT.pTxtFld : 0; }
    const SwTxtTOXMark* GetTOX() const
        { return TEXTTOXMARK == eSetGetExpFldType ? CNTNT.pTxtTOX : 0; }
    const SwSection* GetSection() const
        { return SECTIONNODE == eSetGetExpFldType ? CNTNT.pSection : 0; }
    const SwTableBox* GetTableBox() const
        { return TABLEBOX == eSetGetExpFldType ? CNTNT.pTBox : 0; }
    const SwTxtINetFmt* GetINetFmt() const
        { return TEXTINET == eSetGetExpFldType ? CNTNT.pTxtINet : 0; }
    const SwFlyFrmFmt* GetFlyFmt() const
        { return FLYFRAME == eSetGetExpFldType ? CNTNT.pFlyFmt : 0; }

    sal_uLong GetNode() const { return nNode; }
    xub_StrLen GetCntnt() const { return nCntnt; }
    const void* GetPointer() const { return CNTNT.pTxtFld; }

    void GetPos( SwPosition& rPos ) const;
    void GetPosOfContent( SwPosition& rPos ) const;

    const SwNode* GetNodeFromCntnt() const;
    xub_StrLen GetCntPosFromCntnt() const;

    void SetBodyPos( const SwCntntFrm& rFrm );
};

class _SetGetExpFlds : public o3tl::sorted_vector<_SetGetExpFld*, o3tl::less_ptr_to<_SetGetExpFld> >
{
public:
    ~_SetGetExpFlds() { DeleteAndDestroyAll(); }
};

// struct for saving strings from the SetExp's string fields
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

// search for the string that was saved under rName in the hash table
void LookString( SwHash** ppTbl, sal_uInt16 nSize, const String& rName,
                    String& rRet, sal_uInt16* pPos = 0 );

const int GETFLD_ALL        = 3;        // combine flags via OR
const int GETFLD_CALC       = 1;
const int GETFLD_EXPAND     = 2;

class SwDocUpdtFld
{
    _SetGetExpFlds* pFldSortLst;    // current field list for calculation
    SwCalcFldType*  aFldTypeTable[ TBLSZ ];

    sal_uLong nNodes;               // if the node count is different
    sal_uInt8 nFldLstGetMode;

    bool bInUpdateFlds : 1;     // currently there is an UpdateFlds
    bool bFldsDirty : 1;        // some fields are invalid

    void _MakeFldList( SwDoc& pDoc, int eGetMode );
    void GetBodyNode( const SwTxtFld& , sal_uInt16 nFldWhich );
    void GetBodyNode( const SwSectionNode&);

public:
    SwDocUpdtFld();
    ~SwDocUpdtFld();

    const _SetGetExpFlds* GetSortLst() const { return pFldSortLst; }

    void MakeFldList( SwDoc& rDoc, int bAll, int eGetMode );

    void InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld );

    void InsertFldType( const SwFieldType& rType );
    void RemoveFldType( const SwFieldType& rType );

    bool IsInUpdateFlds() const         { return bInUpdateFlds; }
    void SetInUpdateFlds( bool b )      { bInUpdateFlds = b; }

    bool IsFieldsDirty() const          { return bFldsDirty; }
    void SetFieldsDirty( bool b )       { bFldsDirty = b; }

    SwHash** GetFldTypeTable() const { return (SwHash**)aFldTypeTable; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
