/*************************************************************************
 *
 *  $RCSfile: mvsave.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MVSAVE_HXX
#define _MVSAVE_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _KEYCOD_HXX //autogen
#include <vcl/keycod.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

class SvNumberFormatter;
class SvULongs;
class SwBookmark;
class SwDoc;
class SwCrsrShell;
class SwFmtAnchor;
class SwFrmFmt;
class SwIndex;
class SwNodeIndex;
class SwNodeRange;
class SwPaM;
class SwRedline;
struct SwPosition;

enum SaveBookmarkType { BKMK_POS_NONE   = 0x00,
                        BKMK_POS        = 0x01,
                        BKMK_POS_OTHER  = 0x02
                         };

class SaveBookmark
{
    String  aName, aShortName;
    ULONG nNode1, nNode2;
    xub_StrLen nCntnt1, nCntnt2;
    KeyCode aCode;
    SaveBookmarkType eBkmkType;

public:
    SaveBookmark( int, const SwBookmark&, const SwNodeIndex&,
                                    const SwIndex* pIdx = 0 );
    void SetInDoc( SwDoc* pDoc, const SwNodeIndex&, const SwIndex* pIdx = 0);
};

SV_DECL_PTRARR_DEL( SaveBookmarks, SaveBookmark*, 0, 10 )

void _DelBookmarks( const SwNodeIndex& rStt,
                    const SwNodeIndex& rEnd,
                    SaveBookmarks* pSaveBkmk = 0,
                    const SwIndex* pSttIdx = 0,
                    const SwIndex* pEndIdx = 0 );


#define SAVEFLY 1
#define SAVEFLY_SPLIT 2

void _SaveCntntIdx( SwDoc* pDoc, ULONG nNode, xub_StrLen nCntnt,
                    SvULongs& rSaveArr, BYTE nSaveFly = 0 );
void _RestoreCntntIdx( SwDoc* pDoc, SvULongs& rSaveArr,
                        ULONG nNode, xub_StrLen nOffset = 0,
                        BOOL bAuto = FALSE );
void _RestoreCntntIdx( SvULongs& rSaveArr, const SwNode& rNd,
                        xub_StrLen nLen, xub_StrLen nCorrLen );


struct _SaveFly
{
    ULONG nNdDiff;
    SwFrmFmt* pFrmFmt;
    _SaveFly( ULONG nNodeDiff, SwFrmFmt* pFmt )
        : nNdDiff( nNodeDiff ), pFrmFmt( pFmt ) {}
};

SV_DECL_VARARR( _SaveFlyArr, _SaveFly, 0, 10 )

void _RestFlyInRange( _SaveFlyArr& rArr, const SwNodeIndex& rSttIdx );
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
    ULONG nNode;
    xub_StrLen nCntnt;
    USHORT nType;       // Insert/Move/Delete/... (UndoIds)

public:
    SwDataChanged( const SwPaM& rPam, USHORT nType );
    SwDataChanged( SwDoc* pDoc, const SwPosition& rPos, USHORT nType );
    ~SwDataChanged();

    ULONG GetNode() const           { return nNode; }
    xub_StrLen GetCntnt() const     { return nCntnt; }
};


// Funktions-Deklaration damit auch alles unter der CrsrShell mal die
// Crsr verschieben kann
// die Funktionen rufen nicht die SwDoc::Corr - Methoden!

    // Setzt alle PaMs an OldPos auf NewPos + Offset
void PaMCorrAbs( const SwPosition &rOldPos,
                const SwPosition &rNewPos,
                const xub_StrLen nOffset = 0 );

    // Setzt alle PaMs in OldNode auf NewPos + Offset
void PaMCorrAbs( const SwNodeIndex &rOldNode,
                const SwPosition &rNewPos,
                const xub_StrLen nOffset = 0 );

    // Setzt alle PaMs im Bereich vom Range nach NewPos
void PaMCorrAbs( const SwPaM& rRange,
                 const SwPosition& rNewPos );

    // Setzt alle PaMs im Bereich von [StartNode, EndNode] nach NewPos
void PaMCorrAbs( SwCrsrShell *pShell,
                 const SwNodeIndex &rStartNode,
                 const SwNodeIndex &rEndNode,
                 const SwPosition &rNewPos );

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
    UINT32 nOrdNum;

public:
    _ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAnchor,
                UINT32 nArrOrdNum );
    _ZSortFly& operator=( const _ZSortFly& rCpy )
    {
        pFmt = rCpy.pFmt, pAnchor = rCpy.pAnchor, nOrdNum = rCpy.nOrdNum;
        return *this;
    }

    int operator==( const _ZSortFly& ) const { return FALSE; }
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

    void _Restore();
public:
    _SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx );
    ~_SaveRedlEndPosForRestore();
    void Restore() { if( pSavArr ) _Restore(); }
};


#endif  // _MVSAVE_HXX

