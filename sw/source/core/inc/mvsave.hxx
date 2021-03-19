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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_MVSAVE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_MVSAVE_HXX

#include <vcl/keycod.hxx>
#include <IDocumentMarkAccess.hxx>
#include <vector>
#include <deque>
#include <o3tl/typed_flags_set.hxx>

namespace sfx2 {
    class MetadatableUndo;
}

class SvNumberFormatter;
class SwDoc;
class SwFormatAnchor;
class SwFrameFormat;
class SwIndex;
class SwNodeIndex;
class SwNodeRange;
class SwPaM;
class SwNode;
struct SwPosition;

namespace sw::mark
{
    class IMark;

    class SaveBookmark
    {
    public:
            SaveBookmark(
                const ::sw::mark::IMark& rBkmk,
                const SwNodeIndex& rMvPos,
                const SwIndex* pIdx);
            void SetInDoc(SwDoc* pDoc,
                const SwNodeIndex&,
                const SwIndex* pIdx =nullptr);

    private:
            OUString m_aName;
            OUString m_aShortName;
            bool m_bHidden;
            OUString m_aHideCondition;
            vcl::KeyCode m_aCode;
            IDocumentMarkAccess::MarkType m_eOrigBkmType;
            sal_uLong m_nNode1;
            sal_uLong m_nNode2;
            sal_Int32 m_nContent1;
            sal_Int32 m_nContent2;
            std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndo;
    };

    enum class RestoreMode { Flys = 1, NonFlys = 2, All = 3 };

    /// Takes care of storing relevant attributes of an SwTextNode before split, then restore them on the new node.
    class ContentIdxStore
    {
    public:

            virtual void Clear() =0;
            virtual bool Empty() =0;
            virtual void Save(SwDoc& rDoc, sal_uLong nNode, sal_Int32 nContent, bool bSaveFlySplit=false) =0;
            virtual void Restore(SwDoc& rDoc, sal_uLong nNode, sal_Int32 nOffset=0, bool bAuto = false, bool bAtStart = false, RestoreMode = RestoreMode::All) =0;
            virtual void Restore(SwNode& rNd, sal_Int32 nLen, sal_Int32 nCorrLen, RestoreMode = RestoreMode::All) =0;
            virtual ~ContentIdxStore() {};
            static std::shared_ptr<ContentIdxStore> Create();
    };
}

namespace o3tl {
    template<> struct typed_flags<sw::mark::RestoreMode> : is_typed_flags<sw::mark::RestoreMode, 3> {};
}

void DelBookmarks(const SwNodeIndex& rStt,
    const SwNodeIndex& rEnd,
    std::vector< ::sw::mark::SaveBookmark> * SaveBkmk =nullptr,
    const SwIndex* pSttIdx =nullptr,
    const SwIndex* pEndIdx =nullptr);

/** data structure to temporarily hold fly anchor positions relative to some
 *  location. */
struct SaveFly
{
    SwFrameFormat* pFrameFormat;      /// the fly's frame format
    sal_uLong nNdDiff;      /// relative node difference
    sal_Int32 nContentIndex; ///< index in node
    bool isAtInsertNode;   ///< if true, anchor _at_ insert node index

    SaveFly( sal_uLong nNodeDiff, sal_Int32 const nCntntIdx, SwFrameFormat* pFormat, bool bInsert )
        : pFrameFormat(pFormat)
        , nNdDiff(nNodeDiff)
        , nContentIndex(nCntntIdx)
        , isAtInsertNode(bInsert)
    { }
};

typedef std::deque< SaveFly > SaveFlyArr;

void RestFlyInRange( SaveFlyArr& rArr, const SwPosition& rSttIdx,
                      const SwNodeIndex* pInsPos );
void SaveFlyInRange( const SwNodeRange& rRg, SaveFlyArr& rArr );
void SaveFlyInRange( const SwPaM& rPam, const SwPosition& rInsPos,
                       SaveFlyArr& rArr, bool bMoveAllFlys );

void DelFlyInRange( const SwNodeIndex& rMkNdIdx,
                    const SwNodeIndex& rPtNdIdx,
                    SwIndex const* pMkIdx = nullptr,
                    SwIndex const* pPtIdx = nullptr);

class SwDataChanged
{
    const SwPaM* m_pPam;
    const SwPosition* m_pPos;
    SwDoc& m_rDoc;
    sal_Int32 m_nContent;

public:
    SwDataChanged( const SwPaM& rPam );
    SwDataChanged( SwDoc& rDoc, const SwPosition& rPos );
    ~SwDataChanged();

    sal_Int32 GetContent() const { return m_nContent; }
};

/**
 * Function declarations so that everything below the CursorShell can
 * move the Cursor once in a while.
 * These functions do not call the SwDoc::Corr methods!
 */
void PaMCorrAbs( const SwPaM& rRange,
                 const SwPosition& rNewPos );

/// Sets all PaMs in OldNode to relative Pos
void PaMCorrRel( const SwNodeIndex &rOldNode,
                 const SwPosition &rNewPos,
                 const sal_Int32 nOffset = 0 );

/**
 * Helper to copy paragraph-bound Flys.
 * By sorting by order number, we try to retain the layout.
 */
class ZSortFly
{
    const SwFrameFormat* m_pFormat;
    const SwFormatAnchor* m_pAnchor;
    sal_uInt32 m_nOrdNum;

public:
    ZSortFly( const SwFrameFormat* pFrameFormat, const SwFormatAnchor* pFlyAnchor,
                sal_uInt32 nArrOrdNum );

    bool operator==( const ZSortFly& ) const { return false; }
    bool operator<( const ZSortFly& rCmp ) const
        { return m_nOrdNum < rCmp.m_nOrdNum; }

    const SwFrameFormat* GetFormat() const              { return m_pFormat; }
    const SwFormatAnchor* GetAnchor() const        { return m_pAnchor; }
};

class SwTableNumFormatMerge
{
    SvNumberFormatter* pNFormat;
public:
    SwTableNumFormatMerge( const SwDoc& rSrc, SwDoc& rDest );
    ~SwTableNumFormatMerge();
};

class SaveRedlEndPosForRestore
{
    std::vector<SwPosition*> mvSavArr;
    std::unique_ptr<SwNodeIndex> mpSaveIndex;
    sal_Int32 mnSaveContent;

public:
    SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx, sal_Int32 nContent );
    ~SaveRedlEndPosForRestore();
    void Restore();
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_MVSAVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
