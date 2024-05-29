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
#ifndef INCLUDED_SW_INC_HINTS_HXX
#define INCLUDED_SW_INC_HINTS_HXX

#include "swatrset.hxx"
#include "swtypes.hxx"
#include <utility>
#include <vcl/vclptr.hxx>

class SwFormat;
class OutputDevice;
class SwTable;
class SwNode;
class SwNodes;
class SwPageFrame;
class SwFrame;
class SwHistory;
class SwTextNode;
class SwTextFormatColl;
class SwFrameFormat;
class SwTableBox;
class SwTableBoxFormat;
class SwTableLine;
class SwTableLineFormat;
class SwTableBox;

// Base class for all Message-Hints:
// "Overhead" of SfxPoolItem is handled here
class SwMsgPoolItem : public SfxPoolItem
{
public:
    SwMsgPoolItem( sal_uInt16 nWhich );

    // "Overhead" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwMsgPoolItem*  Clone( SfxItemPool* pPool = nullptr ) const override;
};

// SwPtrMsgPoolItem (old SwObjectDying!)

class SwPtrMsgPoolItem final : public SwMsgPoolItem
{
public:
    void * pObject;

    SwPtrMsgPoolItem( sal_uInt16 nId, void * pObj )
        : SwMsgPoolItem( nId ), pObject( pObj )
    {}
};

/**
 * SwFormatChg is sent when a format has changed to another format. 2 Hints are always sent
 * the old and the new format
 *
 * This is typically owned by an sw::LegacyModifyHint, which knows if this pool item is the old or
 * the new format.
 */
class SwFormatChg final : public SwMsgPoolItem
{
public:
    SwFormat *pChangedFormat;
    SwFormatChg( SwFormat *pFormat );
};



namespace sw {

/// text is moved into pDestNode
class MoveText final : public SfxHint
{
public:
    SwTextNode * pDestNode;
    sal_Int32 nDestStart;
    sal_Int32 nSourceStart;
    sal_Int32 nLen;

    MoveText(SwTextNode *pD, sal_Int32 nD, sal_Int32 nS, sal_Int32 nL)
        : SfxHint(SfxHintId::SwMoveText), pDestNode(pD), nDestStart(nD), nSourceStart(nS), nLen(nL) {}
};

class InsertText final : public SfxHint
{
public:
    const sal_Int32 nPos;
    const sal_Int32 nLen;
    const bool isInsideFieldmarkCommand;
    const bool isInsideFieldmarkResult;

    InsertText(sal_Int32 nP, sal_Int32 nL, bool isInFMCommand, bool isInFMResult);
};

class DeleteText final : public SfxHint
{
public:
    const sal_Int32 nStart;
    const sal_Int32 nLen;

    DeleteText( sal_Int32 nS, sal_Int32 nL );
};

class DeleteChar final : public SfxHint
{
public:
    const sal_Int32 m_nPos;

    DeleteChar( const sal_Int32 nPos );
};

/// new delete redline is created
class RedlineDelText final : public SfxHint
{
public:
    sal_Int32 nStart;
    sal_Int32 nLen;

    RedlineDelText(sal_Int32 nS, sal_Int32 nL) : SfxHint(SfxHintId::SwRedlineDelText), nStart(nS), nLen(nL) {}
};

/// delete redline is removed
class RedlineUnDelText final : public SfxHint
{
public:
    sal_Int32 nStart;
    sal_Int32 nLen;

    RedlineUnDelText(sal_Int32 nS, sal_Int32 nL) : SfxHint(SfxHintId::SwRedlineUnDelText), nStart(nS), nLen(nL) {}
};

/** DocPosUpdate is sent to signal that only the frames from or to a specified document-global position
   have to be updated. At the moment this is only needed when updating pagenumber fields. */
class DocPosUpdate final : public SfxHint
{
public:
    const SwTwips m_nDocPos;
    DocPosUpdate(const SwTwips nDocPos)
        : SfxHint(SfxHintId::SwDocPosUpdate)
        , m_nDocPos(nDocPos)
    {};
};
class DocPosUpdateAtIndex final : public SfxHint
{
public:
    const SwTwips m_nDocPos;
    const SwTextNode& m_rNode;
    const sal_uInt32 m_nIndex;
    DocPosUpdateAtIndex(const SwTwips nDocPos, const SwTextNode& rNode, sal_uInt32 nIndex)
        : SfxHint(SfxHintId::SwDocPosUpdateAtIndex)
        , m_nDocPos(nDocPos)
        , m_rNode(rNode)
        , m_nIndex(nIndex)
    {};
};
class CondCollCondChg final : public SfxHint
{
public:
    const SwTextFormatColl& m_rColl;
    CondCollCondChg(const SwTextFormatColl& rColl) : m_rColl(rColl) {};
};

class GrfRereadAndInCacheHint final : public SfxHint
{
};

class PreGraphicArrivedHint final : public SfxHint
{
public:
    PreGraphicArrivedHint() : SfxHint(SfxHintId::SwPreGraphicArrived) {}
};

class PostGraphicArrivedHint final : public SfxHint
{
public:
    PostGraphicArrivedHint() : SfxHint(SfxHintId::SwPostGraphicArrived) {}
};

class GraphicPieceArrivedHint final : public SfxHint
{
public:
    GraphicPieceArrivedHint() : SfxHint(SfxHintId::SwGraphicPieceArrived) {}
};

class LinkedGraphicStreamArrivedHint final : public SfxHint
{
public:
    LinkedGraphicStreamArrivedHint() : SfxHint(SfxHintId::SwLinkedGraphicStreamArrived) {}
};

class MoveTableLineHint final : public SfxHint
{
public:
    const SwFrameFormat& m_rNewFormat;
    const SwTableLine& m_rTableLine;
    MoveTableLineHint(const SwFrameFormat& rNewFormat, const SwTableLine& rTableLine): m_rNewFormat(rNewFormat), m_rTableLine(rTableLine) {};
};

class MoveTableBoxHint final : public SfxHint
{
public:
    const SwFrameFormat& m_rNewFormat;
    const SwTableBox& m_rTableBox;
    MoveTableBoxHint(const SwFrameFormat& rNewFormat, const SwTableBox& rTableBox): m_rNewFormat(rNewFormat), m_rTableBox(rTableBox) {};
};

class DocumentDyingHint final : public SfxHint
{
public:
    DocumentDyingHint() : SfxHint(SfxHintId::SwDocumentDying) {}
};

class TableLineFormatChanged final : public SfxHint
{
public:
    const SwTableLineFormat& m_rNewFormat;
    const SwTableLine& m_rTabLine;
    TableLineFormatChanged(const SwTableLineFormat& rNewFormat, const SwTableLine& rTabLine) : m_rNewFormat(rNewFormat), m_rTabLine(rTabLine) {};
};
class TableBoxFormatChanged final : public SfxHint
{
public:
    const SwTableBoxFormat& m_rNewFormat;
    const SwTableBox& m_rTableBox;
    TableBoxFormatChanged(const SwTableBoxFormat& rNewFormat, const SwTableBox& rTableBox) : m_rNewFormat(rNewFormat), m_rTableBox(rTableBox) {};
};
class NameChanged final : public SfxHint
{
public:
    const OUString m_sOld;
    const OUString m_sNew;
    NameChanged(const OUString& rOld, const OUString& rNew) : SfxHint(SfxHintId::SwNameChanged), m_sOld(rOld), m_sNew(rNew) {};
};
class TitleChanged final : public SfxHint
{
public:
    const OUString m_sOld;
    const OUString m_sNew;
    TitleChanged(const OUString& rOld, const OUString& rNew) : SfxHint(SfxHintId::SwTitleChanged), m_sOld(rOld), m_sNew(rNew) {};
};
class DescriptionChanged final : public SfxHint
{
public:
    DescriptionChanged() : SfxHint(SfxHintId::SwDescriptionChanged) {}
};
class SectionHidden final: public SfxHint
{
public:
    const bool m_isHidden;
    SectionHidden(const bool isHidden = true) : SfxHint(SfxHintId::SwSectionHidden), m_isHidden(isHidden) {};
};
class TableHeadingChange final: public SfxHint
{
public:
    TableHeadingChange() : SfxHint(SfxHintId::SwTableHeadingChange) {};
};
class VirtPageNumHint final: public SfxHint
{
    const SwPageFrame* m_pPage;
    const SwPageFrame* m_pOrigPage;
    const SwFrame* m_pFrame;
    bool m_bFound;
    /** Multiple attributes can be attached to a single paragraph / table
     The frame, in the end, has to decide which attribute takes effect and which physical page it involves */
public:
    VirtPageNumHint(const SwPageFrame* pPg);
    const SwPageFrame* GetPage() const
        { return m_pPage; }
    const SwPageFrame* GetOrigPage() const
        { return m_pOrigPage; }
    const SwFrame* GetFrame() const
        { return m_pFrame; }
    void SetInfo(const SwPageFrame* pPg, const SwFrame *pF)
        { m_pFrame = pF; m_pPage = pPg; }
    void SetFound()
    {
        assert(!m_bFound);
        m_bFound = true;
    }
    bool IsFound()
        { return m_bFound; }
};
class AutoFormatUsedHint final : public SfxHint
{
    bool& m_isUsed;
    const SwNodes& m_rNodes;
public:
    AutoFormatUsedHint(bool& isUsed, const SwNodes& rNodes) : SfxHint(SfxHintId::SwAutoFormatUsedHint), m_isUsed(isUsed), m_rNodes(rNodes) {}
    void SetUsed() const { m_isUsed = true; }
    void CheckNode(const SwNode*) const;
};
}

class SwUpdateAttr final : public SwMsgPoolItem
{
private:
    sal_Int32 m_nStart;
    sal_Int32 m_nEnd;
    sal_uInt16 m_nWhichAttr;
    std::vector<sal_uInt16> m_aWhichFmtAttrs; // attributes changed inside RES_TXTATR_AUTOFMT

public:
    SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW );
    SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW, std::vector<sal_uInt16> aW );

    sal_Int32 getStart() const
    {
        return m_nStart;
    }

    sal_Int32 getEnd() const
    {
        return m_nEnd;
    }

    sal_uInt16 getWhichAttr() const
    {
        return m_nWhichAttr;
    }

    const std::vector<sal_uInt16>& getFmtAttrs() const
    {
        return m_aWhichFmtAttrs;
    }
};

/// SwTableFormulaUpdate is sent when the table has to be newly calculated or when a table itself is merged or split
enum TableFormulaUpdateFlags { TBL_CALC = 0,
                         TBL_BOXNAME,
                         TBL_BOXPTR,
                         TBL_RELBOXNAME,
                         TBL_MERGETBL,
                         TBL_SPLITTBL
                       };
class SwTableFormulaUpdate final
{
public:
    const SwTable* m_pTable;         ///< Pointer to the current table
    union {
        const SwTable* pDelTable;  ///< Merge: Pointer to the table to be removed
        const OUString* pNewTableNm; ///< Split: the name of the new table
    } m_aData;
    sal_uInt16 m_nSplitLine;       ///< Split: from this BaseLine on will be split
    TableFormulaUpdateFlags m_eFlags;
    bool m_bModified : 1;
    bool m_bBehindSplitLine : 1;

    /** Is sent if a table should be recalculated */
    SwTableFormulaUpdate( const SwTable* );
};

/*
 * SwAttrSetChg is sent when something has changed in the SwAttrSet rTheChgdSet.
 * 2 Hints are always sent, the old and the new items in the rTheChgdSet.
 */
class SwAttrSetChg final : public SwMsgPoolItem
{
    bool m_bDelSet;
    SwAttrSet* m_pChgSet;           ///< what has changed
    const SwAttrSet* m_pTheChgdSet; ///< is only used to compare
public:
    SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet );
    SwAttrSetChg( const SwAttrSetChg& );
    virtual ~SwAttrSetChg() override;

    /// What has changed
    const SwAttrSet* GetChgSet() const     { return m_pChgSet; }
          SwAttrSet* GetChgSet()           { return m_pChgSet; }

    /// Where it has changed
    const SwAttrSet* GetTheChgdSet() const { return m_pTheChgdSet; }

    sal_uInt16 Count() const { return m_pChgSet->Count(); }
    void ClearItem( sal_uInt16 nWhichL )
#ifdef DBG_UTIL
        ;
#else
    { m_pChgSet->ClearItem( nWhichL ); }
#endif
};


class SwFindNearestNode final : public SwMsgPoolItem
{
    const SwNode *m_pNode, *m_pFound;
public:
    SwFindNearestNode( const SwNode& rNd );
    void CheckNode( const SwNode& rNd );

    const SwNode* GetFoundNode() const { return m_pFound; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
