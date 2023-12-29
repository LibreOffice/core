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

#pragma once

#include "editattr.hxx"
#include "edtspell.hxx"
#include "eerdll2.hxx"
#include <editeng/svxfont.hxx>
#include <editeng/EPaM.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <svl/languageoptions.hxx>
#include <tools/lineend.hxx>
#include <o3tl/typed_flags_set.hxx>
#include "TextPortion.hxx"
#include "TextPortionList.hxx"
#include "ItemList.hxx"
#include "ContentNode.hxx"
#include "EditLineList.hxx"
#include "EditPaM.hxx"
#include "EditSelection.hxx"

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

class ImpEditEngine;
enum class TextRotation;


#define CHARPOSGROW     16
#define DEFTAB          720

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent = true, SvtScriptType nScriptType = SvtScriptType::NONE );
sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, SvtScriptType nScriptType );
bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType );

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE );

class EditDoc;

struct ScriptTypePosInfo
{
    short   nScriptType;
    sal_Int32  nStartPos;
    sal_Int32  nEndPos;

    ScriptTypePosInfo( short Type, sal_Int32 Start, sal_Int32 End )
    : nScriptType(Type)
    , nStartPos(Start)
    , nEndPos(End)
    {
    }
};

typedef std::vector<ScriptTypePosInfo> ScriptTypePosInfos;

struct WritingDirectionInfo
{
    sal_uInt8   nType;
    sal_Int32  nStartPos;
    sal_Int32  nEndPos;

    WritingDirectionInfo( sal_uInt8 Type, sal_Int32 Start, sal_Int32 End )
    : nType(Type)
    , nStartPos(Start)
    , nEndPos(End)
    {
    }
};


typedef std::vector<WritingDirectionInfo> WritingDirectionInfos;

class ContentAttribsInfo
{
private:
    typedef std::vector<std::unique_ptr<EditCharAttrib> > CharAttribsType;

    SfxItemSet          aPrevParaAttribs;
    CharAttribsType     aPrevCharAttribs;

public:
                        ContentAttribsInfo( SfxItemSet aParaAttribs );

    const SfxItemSet&       GetPrevParaAttribs() const  { return aPrevParaAttribs; }
    const CharAttribsType&  GetPrevCharAttribs() const  { return aPrevCharAttribs; }

    void AppendCharAttrib(EditCharAttrib* pNew);
};

typedef std::vector<Color> SvxColorList;

enum class DeleteMode {
    Simple, RestOfWord, RestOfContent
};

class ParaPortion
{
    friend class ImpEditEngine; // to adjust the height
private:
    EditLineList        aLineList;
    TextPortionList     aTextPortionList;
    ContentNode*        pNode;
    tools::Long                nHeight;

    ScriptTypePosInfos      aScriptInfos;
    WritingDirectionInfos   aWritingDirectionInfos;

    sal_Int32              nInvalidPosStart;
    sal_Int32              nFirstLineOffset;   // For Writer-LineSpacing-Interpretation
    sal_Int32             nBulletX;
    sal_Int32              nInvalidDiff;

    bool                bInvalid            : 1;
    bool                bSimple             : 1;    // only linear Tap
    bool                bVisible            : 1;    // Belongs to the node!
    bool                bForceRepaint       : 1;

                        ParaPortion( const ParaPortion& ) = delete;

public:
                        ParaPortion( ContentNode* pNode );
                        ~ParaPortion();

    sal_Int32 GetLineNumber( sal_Int32 nIndex ) const;

    EditLineList&       GetLines()                  { return aLineList; }
    const EditLineList& GetLines() const { return aLineList; }

    bool                IsInvalid() const           { return bInvalid; }
    bool                IsSimpleInvalid()   const   { return bSimple; }
    void                SetValid()                  { bInvalid = false; bSimple = true;}

    bool                MustRepaint() const         { return bForceRepaint; }
    void                SetMustRepaint( bool bRP )  { bForceRepaint = bRP; }

    sal_Int32           GetBulletX() const          { return nBulletX; }
    void                SetBulletX( sal_Int32 n )   { nBulletX = n; }

    void                MarkInvalid( sal_Int32 nStart, sal_Int32 nDiff);
    void                MarkSelectionInvalid( sal_Int32 nStart );

    void                SetVisible( bool bVisible );
    bool                IsVisible() const { return bVisible; }

    bool                IsEmpty() { return GetTextPortions().Count() == 1 && GetTextPortions()[0].GetLen() == 0; }

    tools::Long                GetHeight() const           { return ( bVisible ? nHeight : 0 ); }
    sal_Int32           GetFirstLineOffset() const  { return ( bVisible ? nFirstLineOffset : 0 ); }
    void                ResetHeight()   { nHeight = 0; nFirstLineOffset = 0; }

    ContentNode*        GetNode() const             { return pNode; }
    TextPortionList&    GetTextPortions()           { return aTextPortionList; }
    const TextPortionList& GetTextPortions() const { return aTextPortionList; }

    sal_Int32           GetInvalidPosStart() const  { return nInvalidPosStart; }
    short               GetInvalidDiff() const      { return nInvalidDiff; }

    void                CorrectValuesBehindLastFormattedLine( sal_Int32 nLastFormattedLine );
#if OSL_DEBUG_LEVEL > 0
    static bool DbgCheckTextPortions(ParaPortion const&);
#endif
};



class ParaPortionList
{
    mutable sal_Int32 nLastCache;
    std::vector<std::unique_ptr<ParaPortion>> maPortions;
public:
                    ParaPortionList();
                    ~ParaPortionList();

    void            Reset();
    tools::Long GetYOffset(const ParaPortion* pPPortion) const;
    sal_Int32 FindParagraph(tools::Long nYOffset) const;

    const ParaPortion* SafeGetObject(sal_Int32 nPos) const;
    ParaPortion* SafeGetObject(sal_Int32 nPos);

    sal_Int32 GetPos(const ParaPortion* p) const;
    ParaPortion* operator[](sal_Int32 nPos);
    const ParaPortion* operator[](sal_Int32 nPos) const;

    std::unique_ptr<ParaPortion> Release(sal_Int32 nPos);
    void Remove(sal_Int32 nPos);
    void Insert(sal_Int32 nPos, std::unique_ptr<ParaPortion> p);
    void Append(std::unique_ptr<ParaPortion> p);
    sal_Int32 Count() const;

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    // temporary:
    static void DbgCheck(ParaPortionList const&, EditDoc const& rDoc);
#endif
};

class DeletedNodeInfo
{
private:
    ContentNode*    mpInvalidNode;
    sal_Int32       nInvalidParagraph;

public:
            DeletedNodeInfo( ContentNode* pNode, sal_Int32 nPos )
            : mpInvalidNode(pNode)
            , nInvalidParagraph(nPos)
            {
            }

    ContentNode* GetNode() const { return mpInvalidNode; }
    sal_Int32    GetPosition() const { return nInvalidParagraph; }
};



class EditDoc
{
private:
    mutable sal_Int32 nLastCache;
    std::vector<std::unique_ptr<ContentNode> > maContents;

    rtl::Reference<SfxItemPool> pItemPool;
    Link<LinkParamNone*,void>      aModifyHdl;

    SvxFont         maDefFont;           //faster than ever from the pool!!
    sal_uInt16      nDefTab;
    bool            bIsVertical:1;
    TextRotation    mnRotation;
    bool            bIsFixedCellHeight:1;

    bool            bModified:1;
    bool            bDisableAttributeExpanding:1;

public:
                    EditDoc( SfxItemPool* pItemPool );
                    ~EditDoc();

    void            dumpAsXml(xmlTextWriterPtr pWriter) const;
    void            ClearSpellErrors();

    bool            IsModified() const      { return bModified; }
    void            SetModified( bool b );

    void            DisableAttributeExpanding() { bDisableAttributeExpanding = true; }

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink ) { aModifyHdl = rLink; }

    void            CreateDefFont( bool bUseStyles );
    const SvxFont&  GetDefFont() const { return maDefFont; }

    void            SetDefTab( sal_uInt16 nTab )    { nDefTab = nTab ? nTab : DEFTAB; }
    sal_uInt16      GetDefTab() const           { return nDefTab; }

    void            SetVertical( bool bVertical )   { bIsVertical = bVertical; }
    bool            IsEffectivelyVertical() const;
    bool            IsTopToBottom() const;
    bool            GetVertical() const;
    void            SetRotation( TextRotation nRotation )   { mnRotation = nRotation; }
    TextRotation    GetRotation() const                     { return mnRotation; }

    void            SetFixedCellHeight( bool bUseFixedCellHeight )  { bIsFixedCellHeight = bUseFixedCellHeight; }
    bool            IsFixedCellHeight() const               { return bIsFixedCellHeight; }

    EditPaM         Clear();
    EditPaM         RemoveText();
    void            RemoveChars( EditPaM aPaM, sal_Int32 nChars );
    EditPaM         InsertText( EditPaM aPaM, std::u16string_view rStr );
    EditPaM         InsertParaBreak( EditPaM aPaM, bool bKeepEndingAttribs );
    EditPaM         InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem );
    EditPaM         ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight );

    OUString        GetText( LineEnd eEnd ) const;
    sal_Int32       GetTextLen() const;

    OUString       GetParaAsString( sal_Int32 nNode ) const;
    static OUString  GetParaAsString(const ContentNode* pNode, sal_Int32 nStartPos = 0, sal_Int32 nEndPos = -1);

    EditPaM GetStartPaM() const;
    EditPaM GetEndPaM() const;

    SfxItemPool&        GetItemPool()                   { return *pItemPool; }
    const SfxItemPool&  GetItemPool() const             { return *pItemPool; }

    void            InsertAttrib( const SfxPoolItem& rItem, ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd );
    void            InsertAttrib( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem );
    void            InsertAttribInSelection( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem );
    bool            RemoveAttribs( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt16 nWhich );
    bool            RemoveAttribs( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich );
    static void     FindAttribs( ContentNode* pNode, sal_Int32 nStartPos, sal_Int32 nEndPos, SfxItemSet& rCurSet );

    sal_Int32 GetPos(const ContentNode* pNode) const;
    const ContentNode* GetObject(sal_Int32 nPos) const;
    ContentNode* GetObject(sal_Int32 nPos);
    sal_Int32 Count() const;
    const ContentNode* operator[](sal_Int32 nPos) const;
    ContentNode* operator[](sal_Int32 nPos);
    void Insert(sal_Int32 nPos, ContentNode* p);
    /// deletes
    void Remove(sal_Int32 nPos);
    /// does not delete
    void Release(sal_Int32 nPos);

    static OUString GetSepStr( LineEnd eEnd );
};

inline EditCharAttrib* GetAttrib(CharAttribList::AttribsType& rAttribs, std::size_t nAttr)
{
    return (nAttr < rAttribs.size()) ? rAttribs[nAttr].get() : nullptr;
}

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
void CheckOrderedList(const CharAttribList::AttribsType& rAttribs);
#endif

class EditEngineItemPool final : public SfxItemPool
{
private:
    std::shared_ptr<DefItems> m_xDefItems;
public:
    EditEngineItemPool();
private:
    virtual ~EditEngineItemPool() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
