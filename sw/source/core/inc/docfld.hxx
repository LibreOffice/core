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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCFLD_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCFLD_HXX

#include <calc.hxx>
#include <doc.hxx>
#include <IDocumentTimerAccess.hxx>
#include <o3tl/sorted_vector.hxx>

class SwTextField;
class SwIndex;
class SwNodeIndex;
class SwContentFrame;
class SwSectionNode;
class SwSection;
class SwTextTOXMark;
class SwTableBox;
class SwTextINetFormat;
class SwFlyFrameFormat;
class SwDoc;
class SwNode;
struct SwPosition;

// Update expression fields
class _SetGetExpField
{
    sal_uLong nNode;
    sal_Int32 nContent;
    union {
        const SwTextField* pTextField;
        const SwSection* pSection;
        const SwPosition* pPos;
        const SwTextTOXMark* pTextTOX;
        const SwTableBox* pTBox;
        const SwTextINetFormat* pTextINet;
        const SwFlyFrameFormat* pFlyFormat;
    } CNTNT;
    enum _SetGetExpFieldType
        {
            TEXTFIELD, TEXTTOXMARK, SECTIONNODE, CRSRPOS, TABLEBOX,
            TEXTINET, FLYFRAME
        } eSetGetExpFieldType;

public:
    _SetGetExpField( const SwNodeIndex& rNdIdx, const SwTextField* pField = nullptr,
                    const SwIndex* pIdx = nullptr );

    _SetGetExpField( const SwNodeIndex& rNdIdx, const SwTextINetFormat& rINet,
                    const SwIndex* pIdx = nullptr );

    _SetGetExpField( const SwSectionNode& rSectNode,
                    const SwPosition* pPos = nullptr  );

    _SetGetExpField( const SwTableBox& rTableBox,
                    const SwPosition* pPos = nullptr  );

    _SetGetExpField( const SwNodeIndex& rNdIdx, const SwTextTOXMark& rTOX,
                    const SwIndex* pIdx );

    _SetGetExpField( const SwPosition& rPos );

    _SetGetExpField( const SwFlyFrameFormat& rFlyFormat, const SwPosition* pPos = nullptr );

    bool operator==( const _SetGetExpField& rField ) const;
    bool operator<( const _SetGetExpField& rField ) const;

    const SwTextField* GetTextField() const
        { return TEXTFIELD == eSetGetExpFieldType ? CNTNT.pTextField : nullptr; }
    const SwSection* GetSection() const
        { return SECTIONNODE == eSetGetExpFieldType ? CNTNT.pSection : nullptr; }
    const SwTextINetFormat* GetINetFormat() const
        { return TEXTINET == eSetGetExpFieldType ? CNTNT.pTextINet : nullptr; }
    const SwFlyFrameFormat* GetFlyFormat() const
        { return FLYFRAME == eSetGetExpFieldType ? CNTNT.pFlyFormat : nullptr; }

    sal_uLong GetNode() const { return nNode; }
    sal_Int32 GetContent() const { return nContent; }
    const void* GetPointer() const { return CNTNT.pTextField; }

    void GetPosOfContent( SwPosition& rPos ) const;

    const SwNode* GetNodeFromContent() const;
    sal_Int32 GetCntPosFromContent() const;

    void SetBodyPos( const SwContentFrame& rFrame );
};

class _SetGetExpFields : public o3tl::sorted_vector<_SetGetExpField*, o3tl::less_ptr_to<_SetGetExpField> >
{
public:
    ~_SetGetExpFields() { DeleteAndDestroyAll(); }
};

// struct for saving strings from the SetExp's string fields
struct _HashStr : public SwHash
{
    OUString aSetStr;
    _HashStr( const OUString& rName, const OUString& rText, _HashStr* = nullptr );
};

struct SwCalcFieldType : public SwHash
{
    const SwFieldType* pFieldType;

    SwCalcFieldType( const OUString& rStr, const SwFieldType* pFieldTyp )
        : SwHash( rStr ), pFieldType( pFieldTyp )
    {}
};

// search for the string that was saved under rName in the hash table
OUString LookString( SwHash** ppTable, sal_uInt16 nSize, const OUString& rName,
                     sal_uInt16* pPos = nullptr );

const int GETFLD_ALL        = 3;        // combine flags via OR
const int GETFLD_CALC       = 1;
const int GETFLD_EXPAND     = 2;

class SwDocUpdateField
{
    _SetGetExpFields* pFieldSortLst;    // current field list for calculation
    SwCalcFieldType*  aFieldTypeTable[ TBLSZ ];

    sal_uLong nNodes;               // if the node count is different
    sal_uInt8 nFieldLstGetMode;
    SwDoc* pDocument;

    bool bInUpdateFields : 1;     // currently there is an UpdateFields
    bool bFieldsDirty : 1;        // some fields are invalid

    void _MakeFieldList( SwDoc& pDoc, int eGetMode );
    void GetBodyNode( const SwTextField& , sal_uInt16 nFieldWhich );
    void GetBodyNode( const SwSectionNode&);

public:
    SwDocUpdateField(SwDoc* pDocument);
    ~SwDocUpdateField();

    const _SetGetExpFields* GetSortLst() const { return pFieldSortLst; }

    void MakeFieldList( SwDoc& rDoc, bool bAll, int eGetMode );

    void InsDelFieldInFieldLst( bool bIns, const SwTextField& rField );

    void InsertFieldType( const SwFieldType& rType );
    void RemoveFieldType( const SwFieldType& rType );

    bool IsInUpdateFields() const         { return bInUpdateFields; }
    void SetInUpdateFields( bool b )      { bInUpdateFields = b; }

    bool IsFieldsDirty() const          { return bFieldsDirty; }
    void SetFieldsDirty( bool b )
    {
        bFieldsDirty = b;

        if (b)
        {
            pDocument->getIDocumentTimerAccess().StartBackgroundJobs();
        }
    }

    SwHash* const * GetFieldTypeTable() const { return reinterpret_cast<SwHash* const *>(aFieldTypeTable); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
