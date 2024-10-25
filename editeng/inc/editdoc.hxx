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

#include "ParagraphPortionList.hxx"
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

#if defined(YRS)
class IYrsTransactionSupplier;
typedef struct TransactionInner YTransaction;
typedef struct YTextEvent YTextEvent;
#endif


#define CHARPOSGROW     16
#define DEFTAB          720

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent = true, SvtScriptType nScriptType = SvtScriptType::NONE );
sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, SvtScriptType nScriptType );
bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType );

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE );

class EditDoc;

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


/** Edit document
 *
 * Holder, responsible for the content nodes and the manipulation of those.
 */
class EditDoc
{
private:
    mutable sal_Int32 mnLastCache;
    std::vector<std::unique_ptr<ContentNode>> maContents;

    rtl::Reference<SfxItemPool> mpItemPool;
    Link<LinkParamNone*,void> maModifyHdl;

    SvxFont         maDefFont;           //faster than ever from the pool!!
    sal_uInt16      mnDefTab;
    bool            mbIsVertical:1;
    TextRotation    mnRotation;
    bool            mbIsFixedCellHeight:1;

    bool            mbModified:1;
    bool            mbDisableAttributeExpanding:1;

#if defined(YRS)
    OString m_CommentId;
    IYrsTransactionSupplier * m_pYrsSupplier{nullptr};
public:
//    void SetYrsTransactionSupplier(IYrsTransactionSupplier *);
    void SetYrsCommentId(IYrsTransactionSupplier *, OString const& rId);
    void YrsWriteEEState();
    void YrsReadEEState(YTransaction *, ImpEditEngine & rIEE);
    void YrsApplyEEDelta(YTransaction *, YTextEvent const* pEvent, ImpEditEngine & rIEE);
    void YrsSetStyle(sal_Int32 nPara, ::std::u16string_view rStyle);
    void YrsSetParaAttr(sal_Int32 nPara, SfxPoolItem const& rItem);
    OString GetCommentId() const;
#endif

public:
                    EditDoc( SfxItemPool* pItemPool );
                    ~EditDoc();

    void            dumpAsXml(xmlTextWriterPtr pWriter) const;
    void            ClearSpellErrors();

    bool            IsModified() const      { return mbModified; }
    void            SetModified( bool b );

    void            DisableAttributeExpanding() { mbDisableAttributeExpanding = true; }

    void            SetModifyHdl(const Link<LinkParamNone*,void>& rLink)
    {
        maModifyHdl = rLink;
    }

    void            CreateDefFont( bool bUseStyles );
    const SvxFont&  GetDefFont() const { return maDefFont; }

    void            SetDefTab(sal_uInt16 nTab);

    sal_uInt16      GetDefTab() const
    {
        return mnDefTab;
    }

    void            SetVertical(bool bVertical);

    bool            IsEffectivelyVertical() const;
    bool            IsTopToBottom() const;
    bool            GetVertical() const;
    void            SetRotation(TextRotation nRotation);
    TextRotation    GetRotation() const                     { return mnRotation; }

    void            SetFixedCellHeight( bool bUseFixedCellHeight )
    {
        mbIsFixedCellHeight = bUseFixedCellHeight;
    }
    bool            IsFixedCellHeight() const
    {
        return mbIsFixedCellHeight;
    }

    EditPaM         Clear();
    EditPaM         RemoveText();
    void            RemoveChars( EditPaM aPaM, sal_Int32 nChars );
    EditPaM         InsertText( EditPaM aPaM, const OUString& rStr );
    EditPaM         InsertParaBreak( EditPaM aPaM, bool bKeepEndingAttribs );
    EditPaM         InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem );
    EditPaM         ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight );

    OUString        GetText( LineEnd eEnd ) const;
    sal_Int32       GetTextLen() const;

    OUString       GetParaAsString( sal_Int32 nNode ) const;
    static OUString  GetParaAsString(const ContentNode* pNode, sal_Int32 nStartPos = 0, sal_Int32 nEndPos = -1);

    EditPaM GetStartPaM() const;
    EditPaM GetEndPaM() const;

    SfxItemPool&        GetItemPool()
    {
        return *mpItemPool;
    }
    const SfxItemPool&  GetItemPool() const
    {
        return *mpItemPool;
    }

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
    void Insert(sal_Int32 nPos, std::unique_ptr<ContentNode> p);
    /// deletes
    void Remove(sal_Int32 nPos);
    /// does not delete
    std::unique_ptr<ContentNode> Release(sal_Int32 nPos);

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
public:
    EditEngineItemPool();
private:
    virtual ~EditEngineItemPool() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
