/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _DOCFLD_HXX
#define _DOCFLD_HXX

#include <calc.hxx>
#include <doc.hxx>
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
OUString LookString( SwHash** ppTbl, sal_uInt16 nSize, const OUString& rName,
                     sal_uInt16* pPos = 0 );

const int GETFLD_ALL        = 3;        // combine flags via OR
const int GETFLD_CALC       = 1;
const int GETFLD_EXPAND     = 2;

class SwDocUpdtFld
{
    _SetGetExpFlds* pFldSortLst;    // current field list for calculation
    SwCalcFldType*  aFldTypeTable[ TBLSZ ];

    sal_uLong nNodes;               // if the node count is different
    sal_uInt8 nFldLstGetMode;
    SwDoc* pDocument;

    bool bInUpdateFlds : 1;     // currently there is an UpdateFlds
    bool bFldsDirty : 1;        // some fields are invalid

    void _MakeFldList( SwDoc& pDoc, int eGetMode );
    void GetBodyNode( const SwTxtFld& , sal_uInt16 nFldWhich );
    void GetBodyNode( const SwSectionNode&);

public:
    SwDocUpdtFld(SwDoc* pDocument);
    ~SwDocUpdtFld();

    const _SetGetExpFlds* GetSortLst() const { return pFldSortLst; }

    void MakeFldList( SwDoc& rDoc, int bAll, int eGetMode );

    void InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld );

    void InsertFldType( const SwFieldType& rType );
    void RemoveFldType( const SwFieldType& rType );

    bool IsInUpdateFlds() const         { return bInUpdateFlds; }
    void SetInUpdateFlds( bool b )      { bInUpdateFlds = b; }

    bool IsFieldsDirty() const          { return bFldsDirty; }
    void SetFieldsDirty( bool b )
    {
        bFldsDirty = b;

        if (b)
        {
            pDocument->StartBackgroundJobs();
        }
    }

    SwHash** GetFldTypeTable() const { return (SwHash**)aFldTypeTable; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
