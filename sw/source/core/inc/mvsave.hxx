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

namespace sfx2 {
    class MetadatableUndo;
}

class SvNumberFormatter;
class SwDoc;
class SwFmtAnchor;
class SwFrmFmt;
class SwIndex;
class SwNodeIndex;
class SwNodeRange;
class SwPaM;
class SwNode;
struct SwPosition;

namespace sw { namespace mark
{
    class IMark;

    class SaveBookmark
    {
        public:
            SaveBookmark(bool bSavePos,
                bool bSaveOtherPos,
                const ::sw::mark::IMark& rBkmk,
                const SwNodeIndex& rMvPos,
                const SwIndex* pIdx =0);
            void SetInDoc(SwDoc* pDoc,
                const SwNodeIndex&,
                const SwIndex* pIdx =0);
            IDocumentMarkAccess::MarkType GetOriginalBkmType() const
                { return m_eOrigBkmType; }

        private:
            OUString m_aName;
            OUString m_aShortName;
            KeyCode m_aCode;
            bool m_bSavePos;
            bool m_bSaveOtherPos;
            IDocumentMarkAccess::MarkType m_eOrigBkmType;
            sal_uLong m_nNode1;
            sal_uLong m_nNode2;
            sal_Int32 m_nCntnt1;
            sal_Int32 m_nCntnt2;
            ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndo;
    };
}}

#define SAVEFLY 1
#define SAVEFLY_SPLIT 2

void _DelBookmarks(const SwNodeIndex& rStt,
    const SwNodeIndex& rEnd,
    ::std::vector< ::sw::mark::SaveBookmark> * SaveBkmk =0,
    const SwIndex* pSttIdx =0,
    const SwIndex* pEndIdx =0);
void _SaveCntntIdx( SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt,
                    std::vector<sal_uLong>& rSaveArr, sal_uInt8 nSaveFly = 0 );
void _RestoreCntntIdx( SwDoc* pDoc, std::vector<sal_uLong>& rSaveArr,
                        sal_uLong nNode, sal_Int32 nOffset = 0,
                        bool bAuto = false );
void _RestoreCntntIdx( std::vector<sal_uLong>& rSaveArr, const SwNode& rNd,
                        sal_Int32 nLen, sal_Int32 nCorrLen );

/** data structure to temporarily hold fly anchor positions relative to some
 *  location. */
struct _SaveFly
{
    sal_uLong nNdDiff;              /// relative node difference
    SwFrmFmt* pFrmFmt;          /// the fly's frame format
    bool bInsertPosition;   /// if true, anchor _at_ insert position

    _SaveFly( sal_uLong nNodeDiff, SwFrmFmt* pFmt, bool bInsert )
        : nNdDiff( nNodeDiff ), pFrmFmt( pFmt ), bInsertPosition( bInsert )
    { }
};

typedef ::std::deque< _SaveFly > _SaveFlyArr;

void _RestFlyInRange( _SaveFlyArr& rArr, const SwNodeIndex& rSttIdx,
                      const SwNodeIndex* pInsPos );
void _SaveFlyInRange( const SwNodeRange& rRg, _SaveFlyArr& rArr );
void _SaveFlyInRange( const SwPaM& rPam, const SwNodeIndex& rInsPos,
                       _SaveFlyArr& rArr, sal_Bool bMoveAllFlys );

void DelFlyInRange( const SwNodeIndex& rMkNdIdx,
                    const SwNodeIndex& rPtNdIdx );

class SwDataChanged
{
    const SwPaM* pPam;
    const SwPosition* pPos;
    SwDoc* pDoc;
    sal_uLong nNode;
    sal_Int32 nCntnt;

public:
    SwDataChanged( const SwPaM& rPam );
    SwDataChanged( SwDoc* pDoc, const SwPosition& rPos );
    ~SwDataChanged();

    sal_uLong GetNode() const           { return nNode; }
    sal_Int32 GetCntnt() const { return nCntnt; }
};

// Funktions-Deklaration damit auch alles unter der CrsrShell mal die
// Crsr verschieben kann
// die Funktionen rufen nicht die SwDoc::Corr - Methoden!

    // Setzt alle PaMs im Bereich vom Range nach NewPos
void PaMCorrAbs( const SwPaM& rRange,
                 const SwPosition& rNewPos );

    // Setzt alle PaMs in OldNode auf relative Pos
void PaMCorrRel( const SwNodeIndex &rOldNode,
                 const SwPosition &rNewPos,
                 const sal_Int32 nOffset = 0 );

// Hilfsklasse zum kopieren von absatzgebundenen Flys. Durch die Sortierung
// nach der Ordnungsnummer wird versucht die layout seitige Anordnung
// bei zu behalten
class _ZSortFly
{
    const SwFrmFmt* pFmt;
    const SwFmtAnchor* pAnchor;
    sal_uInt32 nOrdNum;

public:
    _ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAnchor,
                sal_uInt32 nArrOrdNum );
    _ZSortFly& operator=( const _ZSortFly& rCpy )
    {
        pFmt = rCpy.pFmt, pAnchor = rCpy.pAnchor, nOrdNum = rCpy.nOrdNum;
        return *this;
    }

    bool operator==( const _ZSortFly& ) const { return false; }
    bool operator<( const _ZSortFly& rCmp ) const
        { return nOrdNum < rCmp.nOrdNum; }

    const SwFrmFmt* GetFmt() const              { return pFmt; }
    const SwFmtAnchor* GetAnchor() const        { return pAnchor; }
};

class SwTblNumFmtMerge
{
    SvNumberFormatter* pNFmt;
public:
    SwTblNumFmtMerge( const SwDoc& rSrc, SwDoc& rDest );
    ~SwTblNumFmtMerge();
};

class _SaveRedlEndPosForRestore
{
    std::vector<SwPosition*>* pSavArr;
    SwNodeIndex* pSavIdx;
    sal_Int32 nSavCntnt;

    void _Restore();
public:
    _SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx, sal_Int32 nCntnt );
    ~_SaveRedlEndPosForRestore();
    void Restore() { if( pSavArr ) _Restore(); }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_MVSAVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
