/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SW_MVSAVE_HXX
#define SW_MVSAVE_HXX

#include <tools/string.hxx>
#include <vcl/keycod.hxx>
#include <svl/svarray.hxx>
#include <IDocumentMarkAccess.hxx>
#include <vector>

namespace sfx2 {
    class MetadatableUndo;
}

class SvNumberFormatter;
class SvULongs;
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
            ::rtl::OUString m_aName;
            ::rtl::OUString m_aShortName;
            KeyCode m_aCode;
            bool m_bSavePos;
            bool m_bSaveOtherPos;
            IDocumentMarkAccess::MarkType m_eOrigBkmType;
            sal_uLong m_nNode1;
            sal_uLong m_nNode2;
            xub_StrLen m_nCntnt1;
            xub_StrLen m_nCntnt2;
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
void _SaveCntntIdx( SwDoc* pDoc, sal_uLong nNode, xub_StrLen nCntnt,
                    SvULongs& rSaveArr, sal_uInt8 nSaveFly = 0 );
void _RestoreCntntIdx( SwDoc* pDoc, SvULongs& rSaveArr,
                        sal_uLong nNode, xub_StrLen nOffset = 0,
                        sal_Bool bAuto = sal_False );
void _RestoreCntntIdx( SvULongs& rSaveArr, const SwNode& rNd,
                        xub_StrLen nLen, xub_StrLen nCorrLen );


/** data structure to temporarily hold fly anchor positions relative to some
 *  location. */
struct _SaveFly
{
    sal_uLong nNdDiff;              /// relative node difference
    SwFrmFmt* pFrmFmt;          /// the fly's frame format
    sal_Bool bInsertPosition;   /// if true, anchor _at_ insert position

    _SaveFly( sal_uLong nNodeDiff, SwFrmFmt* pFmt, sal_Bool bInsert )
        : nNdDiff( nNodeDiff ), pFrmFmt( pFmt ), bInsertPosition( bInsert )
    { }
};

SV_DECL_VARARR( _SaveFlyArr, _SaveFly, 0, 10 )

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
    xub_StrLen nCntnt;
    sal_uInt16 nType;       // Insert/Move/Delete/... (UndoIds)

public:
    SwDataChanged( const SwPaM& rPam, sal_uInt16 nType );
    SwDataChanged( SwDoc* pDoc, const SwPosition& rPos, sal_uInt16 nType );
    ~SwDataChanged();

    sal_uLong GetNode() const           { return nNode; }
    xub_StrLen GetCntnt() const     { return nCntnt; }
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
                 const xub_StrLen nOffset = 0 );


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

    int operator==( const _ZSortFly& ) const { return sal_False; }
    int operator<( const _ZSortFly& rCmp ) const
        { return nOrdNum < rCmp.nOrdNum; }

    const SwFrmFmt* GetFmt() const              { return pFmt; }
    const SwFmtAnchor* GetAnchor() const        { return pAnchor; }
};

SV_DECL_VARARR_SORT( _ZSortFlys, _ZSortFly, 0, 10 )


class SwTblNumFmtMerge
{
    SvNumberFormatter* pNFmt;
public:
    SwTblNumFmtMerge( const SwDoc& rSrc, SwDoc& rDest );
    ~SwTblNumFmtMerge();
};


class _SaveRedlEndPosForRestore
{
    SvPtrarr* pSavArr;
    SwNodeIndex* pSavIdx;
    xub_StrLen nSavCntnt;

    void _Restore();
public:
    _SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx, xub_StrLen nCntnt );
    ~_SaveRedlEndPosForRestore();
    void Restore() { if( pSavArr ) _Restore(); }
};


#endif  // SW_MVSAVE_HXX

