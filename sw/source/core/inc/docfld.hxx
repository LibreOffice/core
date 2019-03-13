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
#include <memory>

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
class SwNode;
struct SwPosition;
enum class SwFieldIds : sal_uInt16;

// Update expression fields
class SetGetExpField
{
    sal_uLong m_nNode;
    sal_Int32 m_nContent;
    union {
        const SwTextField* pTextField;
        const SwSection* pSection;
        const SwPosition* pPos;
        const SwTextTOXMark* pTextTOX;
        const SwTableBox* pTBox;
        const SwTextINetFormat* pTextINet;
        const SwFlyFrameFormat* pFlyFormat;
    } m_CNTNT;
    enum SetGetExpFieldType
        {
            TEXTFIELD, TEXTTOXMARK, SECTIONNODE, CRSRPOS, TABLEBOX,
            TEXTINET, FLYFRAME
        } m_eSetGetExpFieldType;

public:
    SetGetExpField( const SwNodeIndex& rNdIdx, const SwTextField* pField = nullptr,
                    const SwIndex* pIdx = nullptr );

    SetGetExpField( const SwNodeIndex& rNdIdx, const SwTextINetFormat& rINet );

    SetGetExpField( const SwSectionNode& rSectNode,
                    const SwPosition* pPos = nullptr  );

    SetGetExpField( const SwTableBox& rTableBox  );

    SetGetExpField( const SwNodeIndex& rNdIdx, const SwTextTOXMark& rTOX );

    SetGetExpField( const SwPosition& rPos );

    SetGetExpField( const SwFlyFrameFormat& rFlyFormat, const SwPosition* pPos );

    bool operator==( const SetGetExpField& rField ) const;
    bool operator<( const SetGetExpField& rField ) const;

    const SwTextField* GetTextField() const
        { return TEXTFIELD == m_eSetGetExpFieldType ? m_CNTNT.pTextField : nullptr; }
    const SwSection* GetSection() const
        { return SECTIONNODE == m_eSetGetExpFieldType ? m_CNTNT.pSection : nullptr; }
    const SwTextINetFormat* GetINetFormat() const
        { return TEXTINET == m_eSetGetExpFieldType ? m_CNTNT.pTextINet : nullptr; }
    const SwFlyFrameFormat* GetFlyFormat() const
        { return FLYFRAME == m_eSetGetExpFieldType ? m_CNTNT.pFlyFormat : nullptr; }

    sal_uLong GetNode() const { return m_nNode; }
    sal_Int32 GetContent() const { return m_nContent; }
    const void* GetPointer() const { return m_CNTNT.pTextField; }

    void GetPosOfContent( SwPosition& rPos ) const;

    const SwNode* GetNodeFromContent() const;
    sal_Int32 GetCntPosFromContent() const;

    void SetBodyPos( const SwContentFrame& rFrame );
};

class SetGetExpFields : public o3tl::sorted_vector<std::unique_ptr<SetGetExpField>, o3tl::less_uniqueptr_to<SetGetExpField> >
{
};

// struct for saving strings from the SetExp's string fields
struct HashStr : public SwHash
{
    OUString aSetStr;
    HashStr( const OUString& rName, const OUString& rText, HashStr* );
};

struct SwCalcFieldType : public SwHash
{
    const SwFieldType* pFieldType;

    SwCalcFieldType( const OUString& rStr, const SwFieldType* pFieldTyp )
        : SwHash( rStr ), pFieldType( pFieldTyp )
    {}
};

// search for the string that was saved under rName in the hash table
OUString LookString( SwHashTable<HashStr> const & rTable, const OUString& rName );

const int GETFLD_ALL        = 3;        // combine flags via OR
const int GETFLD_CALC       = 1;
const int GETFLD_EXPAND     = 2;

class SwDocUpdateField
{
    std::unique_ptr<SetGetExpFields> m_pFieldSortList; ///< current field list for calculation
    SwHashTable<SwCalcFieldType> m_FieldTypeTable;

    sal_uLong m_nNodes; ///< to check if the node count changed
    int m_nFieldListGetMode;
    SwDoc& m_rDoc;

    bool m_bInUpdateFields : 1; ///< currently in an UpdateFields call
    bool m_bFieldsDirty : 1;    ///< some fields are invalid

    void MakeFieldList_( SwDoc& pDoc, int eGetMode );
    void GetBodyNode( const SwTextField& , SwFieldIds nFieldWhich );
    void GetBodyNode( const SwSectionNode&);

public:
    SwDocUpdateField(SwDoc& rDocument);
    ~SwDocUpdateField();

    const SetGetExpFields* GetSortList() const { return m_pFieldSortList.get(); }

    void MakeFieldList( SwDoc& rDoc, bool bAll, int eGetMode );

    void InsDelFieldInFieldLst( bool bIns, const SwTextField& rField );

    void InsertFieldType( const SwFieldType& rType );
    void RemoveFieldType( const SwFieldType& rType );

    bool IsInUpdateFields() const         { return m_bInUpdateFields; }
    void SetInUpdateFields( bool b )      { m_bInUpdateFields = b; }

    bool IsFieldsDirty() const          { return m_bFieldsDirty; }
    void SetFieldsDirty( bool b )
    {
        m_bFieldsDirty = b;

        if (b)
        {
            m_rDoc.getIDocumentTimerAccess().StartIdling();
        }
    }

    SwHashTable<SwCalcFieldType> const& GetFieldTypeTable() const { return m_FieldTypeTable; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
