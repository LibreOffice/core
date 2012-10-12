/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <hintids.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/resid.hxx>
#include <editeng/lrspitem.hxx>
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <doctxm.hxx>       // pTOXBaseRing
#include <poolfmt.hxx>
#include <UndoCore.hxx>
#include <UndoRedline.hxx>
#include <UndoNumbering.hxx>
#include <swundo.hxx>
#include <SwUndoFmt.hxx>
#include <rolbck.hxx>
#include <paratr.hxx>
#include <docary.hxx>
#include <mvsave.hxx>
#include <txtfrm.hxx>
#include <pamtyp.hxx>
#include <redline.hxx>
#include <comcore.hrc>
#include <editeng/adjitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <frmatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <SwNodeNum.hxx>
#include <list.hxx>
#include <listfunc.hxx>
#include <switerator.hxx>
#include <comphelper/string.hxx>

#include <map>

namespace {
    static void lcl_ResetIndentAttrs(SwDoc *pDoc, const SwPaM &rPam, sal_uInt16 marker )
    {
        std::set<sal_uInt16> aResetAttrsArray;
        aResetAttrsArray.insert( marker );
        // #i114929#
        // On a selection setup a corresponding Point-and-Mark in order to get
        // the indentation attribute reset on all paragraphs touched by the selection
        if ( rPam.HasMark() &&
             rPam.End()->nNode.GetNode().GetTxtNode() )
        {
            SwPaM aPam( rPam.Start()->nNode,
                        rPam.End()->nNode );
            aPam.Start()->nContent = 0;
            aPam.End()->nContent = rPam.End()->nNode.GetNode().GetTxtNode()->Len();
            pDoc->ResetAttrs( aPam, sal_False, aResetAttrsArray );
        }
        else
        {
            pDoc->ResetAttrs( rPam, sal_False, aResetAttrsArray );
        }
    }
}

#include <stdlib.h>


inline sal_uInt8 GetUpperLvlChg( sal_uInt8 nCurLvl, sal_uInt8 nLevel, sal_uInt16 nMask )
{
    if( 1 < nLevel )
    {
        if( nCurLvl + 1 >= nLevel )
            nCurLvl -= nLevel - 1;
        else
            nCurLvl = 0;
    }
    return static_cast<sal_uInt8>((nMask - 1) & ~(( 1 << nCurLvl ) - 1));
}

void SwDoc::SetOutlineNumRule( const SwNumRule& rRule )
{
    if( pOutlineRule )
        (*pOutlineRule) = rRule;
    else
    {
        pOutlineRule = new SwNumRule( rRule );

        AddNumRule(pOutlineRule); // #i36749#
    }

    pOutlineRule->SetRuleType( OUTLINE_RULE );
    pOutlineRule->SetName( rtl::OUString::createFromAscii(
                                        SwNumRule::GetOutlineRuleName() ),
                           *this);

    // assure that the outline numbering rule is an automatic rule
    pOutlineRule->SetAutoRule( sal_True );

    // test whether the optional CharFormats are defined in this Document
    pOutlineRule->CheckCharFmts( this );

    // notify text nodes, which are registered at the outline style, about the
    // changed outline style
    SwNumRule::tTxtNodeList aTxtNodeList;
    pOutlineRule->GetTxtNodeList( aTxtNodeList );
    for ( SwNumRule::tTxtNodeList::iterator aIter = aTxtNodeList.begin();
          aIter != aTxtNodeList.end(); ++aIter )
    {
        SwTxtNode* pTxtNd = *aIter;
        pTxtNd->NumRuleChgd();

        // assure that list level corresponds to outline level
        if ( pTxtNd->GetTxtColl()->IsAssignedToListLevelOfOutlineStyle() &&
             pTxtNd->GetAttrListLevel() != pTxtNd->GetTxtColl()->GetAssignedOutlineStyleLevel() )
        {
            pTxtNd->SetAttrListLevel( pTxtNd->GetTxtColl()->GetAssignedOutlineStyleLevel() );
        }
    }

    PropagateOutlineRule();
    pOutlineRule->SetInvalidRule(sal_True);
    UpdateNumRule();

    // update if we have foot notes && chapter-wise numbering
    if( !GetFtnIdxs().empty() && FTNNUM_CHAPTER == GetFtnInfo().eNum )
        GetFtnIdxs().UpdateAllFtn();

    UpdateExpFlds(NULL, true);

    SetModified();
}

void SwDoc::PropagateOutlineRule()
{
    for (sal_uInt16 n = 0; n < pTxtFmtCollTbl->size(); n++)
    {
        SwTxtFmtColl *pColl = (*pTxtFmtCollTbl)[n];

        if(pColl->IsAssignedToListLevelOfOutlineStyle())//<-end,zhaojianwei
        {
            // Check only the list style, which is set at the paragraph style
            const SwNumRuleItem & rCollRuleItem = pColl->GetNumRule( sal_False );

            // Check on document setting OUTLINE_LEVEL_YIELDS_OUTLINE_RULE no longer needed.
            if ( rCollRuleItem.GetValue().Len() == 0 )
            {
                SwNumRule * pMyOutlineRule = GetOutlineNumRule();

                if (pMyOutlineRule)
                {
                    SwNumRuleItem aNumItem( pMyOutlineRule->GetName() );

                    pColl->SetFmtAttr(aNumItem);
                }
            }
        }
    }
}

// Increase/Decrease
sal_Bool SwDoc::OutlineUpDown( const SwPaM& rPam, short nOffset )
{
    if( GetNodes().GetOutLineNds().empty() || !nOffset )
        return sal_False;

    // calculate the area
    const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
    const SwNodePtr pSttNd = (SwNodePtr)&rPam.Start()->nNode.GetNode();
    const SwNodePtr pEndNd = (SwNodePtr)&rPam.End()->nNode.GetNode();
    sal_uInt16 nSttPos, nEndPos;

    if( !rOutlNds.Seek_Entry( pSttNd, &nSttPos ) &&
        !nSttPos-- )
        // we're not in an "Outline section"
        return sal_False;

    if( rOutlNds.Seek_Entry( pEndNd, &nEndPos ) )
        ++nEndPos;

    // We now have the wanted area in the OutlineNodes array,
    // so check now if we're not invalidating sublevels
    // (stepping over the limits)
    sal_uInt16 n;

    // Here we go:
    // 1. Create the Template array:
    SwTxtFmtColl* aCollArr[ MAXLEVEL ];
    memset( aCollArr, 0, sizeof( SwTxtFmtColl* ) * MAXLEVEL );

    for( n = 0; n < pTxtFmtCollTbl->size(); ++n )
    {
        if((*pTxtFmtCollTbl)[ n ]->IsAssignedToListLevelOfOutlineStyle())
        {
            const int nLevel = (*pTxtFmtCollTbl)[ n ]->GetAssignedOutlineStyleLevel();
            aCollArr[ nLevel ] = (*pTxtFmtCollTbl)[ n ];
        }//<-end,zhaojianwei
    }

    /* Find the last occupied level (backward). */
    for (n = MAXLEVEL - 1; n > 0; n--)
    {
        if (aCollArr[n] != 0)
            break;
    }

    /* If an occupied level is found, choose next level (which IS
       unoccupied) until a valid level is found. If no occupied level
       was found n is 0 and aCollArr[0] is 0. In this case no demoting
       is possible. */
    if (aCollArr[n] != 0)
    {
        while (n < MAXLEVEL - 1)
        {
            n++;

            SwTxtFmtColl *aTmpColl =
                GetTxtCollFromPool(static_cast<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + n));

            if( aTmpColl->IsAssignedToListLevelOfOutlineStyle() &&
                aTmpColl->GetAssignedOutlineStyleLevel() == n )//<-end,zhaojianwei
            {
                aCollArr[n] = aTmpColl;
                break;
            }
        }
    }

    /* Find the first occupied level (forward). */
    for (n = 0; n < MAXLEVEL - 1; n++)
    {
        if (aCollArr[n] != 0)
            break;
    }

    /* If an occupied level is found, choose previous level (which IS
       unoccupied) until a valid level is found. If no occupied level
       was found n is MAXLEVEL - 1 and aCollArr[MAXLEVEL - 1] is 0. In
       this case no demoting is possible. */
    if (aCollArr[n] != 0)
    {
        while (n > 0)
        {
            n--;

            SwTxtFmtColl *aTmpColl =
                GetTxtCollFromPool(static_cast<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + n));

            //if (aTmpColl->GetOutlineLevel() == n)//#outline level,zhaojianwei
            if( aTmpColl->IsAssignedToListLevelOfOutlineStyle() &&
                aTmpColl->GetAssignedOutlineStyleLevel() == n )//<-end,zhaojianwei
            {
                aCollArr[n] = aTmpColl;
                break;
            }
        }
    }

    /* --> #i13747#

        Build a move table that states from which level to which other level
        an outline will be moved.

        the move table:
        aMoveArr[n] = m: replace aCollArr[n] with aCollArr[m]
    */
    int aMoveArr[MAXLEVEL];
    int nStep; // step size for searching in aCollArr: -1 or 1
    int nNum; // amount of steps for stepping in aCollArr

    if (nOffset < 0)
    {
        nStep = -1;
        nNum = -nOffset;
    }
    else
    {
        nStep = 1;
        nNum = nOffset;
    }

    /* traverse aCollArr */
    for (n = 0; n < MAXLEVEL; n++)
    {
        /* If outline level n has an assigned paragraph style step
           nNum steps forwards (nStep == 1) or backwards (nStep ==
           -1).  One step is to go to the next non-null entry in
           aCollArr in the selected direction. If nNum steps were
           possible write the index of the entry found to aCollArr[n],
           i.e. outline level n will be replaced by outline level
           aCollArr[n].

           If outline level n has no assigned paragraph style
           aMoveArr[n] is set to -1.
        */
        if (aCollArr[n] != NULL)
        {
            sal_uInt16 m = n;
            int nCount = nNum;

            while (nCount > 0 && m + nStep >= 0 && m + nStep < MAXLEVEL)
            {
                m = static_cast<sal_uInt16>(m + nStep);

                if (aCollArr[m] != NULL)
                    nCount--;
            }

            if (nCount == 0)
                aMoveArr[n] = m;
            else
                aMoveArr[n] = -1;

        }
        else
            aMoveArr[n] = -1;
    }

    /* If moving of the outline levels is applicable, i.e. for all
       outline levels occuring in the document there has to be a valid
       target outline level implied by aMoveArr. */
    bool bMoveApplicable = true;
    for (n = nSttPos; n < nEndPos; n++)
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        SwTxtFmtColl* pColl = pTxtNd->GetTxtColl();

        if( pColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            const int nLevel = pColl->GetAssignedOutlineStyleLevel();
            if (aMoveArr[nLevel] == -1)
                bMoveApplicable = false;
        }//<-end,zhaojianwei

        // Check on outline level attribute of text node, if text node is
        // not an outline via a to outline style assigned paragraph style.
        else
        {
            const int nNewOutlineLevel = pTxtNd->GetAttrOutlineLevel() + nOffset;
            if ( nNewOutlineLevel < 1 || nNewOutlineLevel > MAXLEVEL )
            {
                bMoveApplicable = false;
            }
        }
    }

    if (! bMoveApplicable )
        return sal_False;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().StartUndo(UNDO_OUTLINE_LR, NULL);
        SwUndo *const pUndoOLR( new SwUndoOutlineLeftRight( rPam, nOffset ) );
        GetIDocumentUndoRedo().AppendUndo(pUndoOLR);
    }

    // 2. Apply the new Template to all Nodes

    n = nSttPos;
    while( n < nEndPos)
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        SwTxtFmtColl* pColl = pTxtNd->GetTxtColl();

        if( pColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            const int nLevel = pColl->GetAssignedOutlineStyleLevel();//#outline level,add by zhaojianwei

            OSL_ENSURE(aMoveArr[nLevel] >= 0,
                "move table: current TxtColl not found when building table!");


            if (nLevel < MAXLEVEL && aMoveArr[nLevel] >= 0)
            {
                pColl = aCollArr[ aMoveArr[nLevel] ];

                if (pColl != NULL)
                    pColl = (SwTxtFmtColl*)pTxtNd->ChgFmtColl( pColl );
            }

        }
        else if( pTxtNd->GetAttrOutlineLevel() > 0) //#outline level,add by zhaojianwei
        {
            int nLevel = pTxtNd->GetAttrOutlineLevel() + nOffset;
            if( 0 <= nLevel && nLevel <= MAXLEVEL)
                pTxtNd->SetAttrOutlineLevel( nLevel );

        }//<-end,zhaojianwei

        n++;
        // Undo ???
    }
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().EndUndo(UNDO_OUTLINE_LR, NULL);
    }

    ChkCondColls();
    SetModified();

    return sal_True;
}

// Move up/down
sal_Bool SwDoc::MoveOutlinePara( const SwPaM& rPam, short nOffset )
{
    // Do not move to special areas
    const SwPosition& rStt = *rPam.Start(),
                    & rEnd = &rStt == rPam.GetPoint() ? *rPam.GetMark()
                                                      : *rPam.GetPoint();
    if( GetNodes().GetOutLineNds().empty() || !nOffset ||
        (rStt.nNode.GetIndex() < GetNodes().GetEndOfExtras().GetIndex()) ||
        (rEnd.nNode.GetIndex() < GetNodes().GetEndOfExtras().GetIndex()))
    {
        return sal_False;
    }

    sal_uInt16 nAktPos = 0;
    SwNodeIndex aSttRg( rStt.nNode ), aEndRg( rEnd.nNode );

    int nOutLineLevel = MAXLEVEL;           //<-end,zhaojianwei
    SwNode* pSrch = &aSttRg.GetNode();

   if( pSrch->IsTxtNode())
        nOutLineLevel = static_cast<sal_uInt8>(((SwTxtNode*)pSrch)->GetAttrOutlineLevel()-1);//<-end,zhaojianwei
    SwNode* pEndSrch = &aEndRg.GetNode();
    if( !GetNodes().GetOutLineNds().Seek_Entry( pSrch, &nAktPos ) )
    {
        if( !nAktPos )
            return sal_False; // Promoting or demoting before the first outline => no.
        if( --nAktPos )
            aSttRg = *GetNodes().GetOutLineNds()[ nAktPos ];
        else if( 0 > nOffset )
            return sal_False; // Promoting at the top of document?!
        else
            aSttRg = *GetNodes().GetEndOfContent().StartOfSectionNode();
    }
    sal_uInt16 nTmpPos = 0;
    // If the given range ends at an outlined text node we have to decide if it has to be a part of
    // the moving range or not. Normally it will be a sub outline of our chapter
    // and has to be moved, too. But if the chapter ends with a table(or a section end),
    // the next text node will be choosen and this could be the next outline of the same level.
    // The criteria has to be the outline level: sub level => incorporate, same/higher level => no.
    if( GetNodes().GetOutLineNds().Seek_Entry( pEndSrch, &nTmpPos ) )
    {
        if( !pEndSrch->IsTxtNode() || pEndSrch == pSrch ||
            nOutLineLevel < ((SwTxtNode*)pEndSrch)->GetAttrOutlineLevel()-1 )//<-end,zhaojianwei
            ++nTmpPos; // For sub outlines only!
    }

    aEndRg = nTmpPos < GetNodes().GetOutLineNds().size()
                    ? *GetNodes().GetOutLineNds()[ nTmpPos ]
                    : GetNodes().GetEndOfContent();
    if( nOffset >= 0 )
        nAktPos = nTmpPos;
    if( aEndRg == aSttRg )
    {
        OSL_FAIL( "Moving outlines: Surprising selection" );
        aEndRg++;
    }

    const SwNode* pNd;
    // The following code corrects the range to handle sections (start/end nodes)
    // The range will be extended if the least node before the range is a start node
    // which ends inside the range => The complete section will be moved.
    // The range will be shrinked if the last position is a start node.
    // The range will be shrinked if the last node is an end node which starts before the range.
    aSttRg--;
    while( aSttRg.GetNode().IsStartNode() )
    {
        pNd = aSttRg.GetNode().EndOfSectionNode();
        if( pNd->GetIndex() >= aEndRg.GetIndex() )
            break;
        aSttRg--;
    }
    aSttRg++;

    aEndRg--;
    while( aEndRg.GetNode().IsStartNode() )
        aEndRg--;
    while( aEndRg.GetNode().IsEndNode() )
    {
        pNd = aEndRg.GetNode().StartOfSectionNode();
        if( pNd->GetIndex() >= aSttRg.GetIndex() )
            break;
        aEndRg--;
    }
    aEndRg++;

    // calculation of the new position
    if( nOffset < 0 && nAktPos < sal_uInt16(-nOffset) )
        pNd = GetNodes().GetEndOfContent().StartOfSectionNode();
    else if( nAktPos + nOffset >= (sal_uInt16)GetNodes().GetOutLineNds().size() )
        pNd = &GetNodes().GetEndOfContent();
    else
        pNd = GetNodes().GetOutLineNds()[ nAktPos + nOffset ];

    sal_uLong nNewPos = pNd->GetIndex();

    // And now a correction of the insert position if necessary...
    SwNodeIndex aInsertPos( *pNd, -1 );
    while( aInsertPos.GetNode().IsStartNode() )
    {
        // Just before the insert position starts a section:
        // when I'm moving forward I do not want to enter the section,
        // when I'm moving backward I want to stay in the section if I'm already a part of,
        // I want to stay outside if I was outside before.
        if( nOffset < 0 )
        {
            pNd = aInsertPos.GetNode().EndOfSectionNode();
            if( pNd->GetIndex() >= aEndRg.GetIndex() )
                break;
        }
        aInsertPos--;
        --nNewPos;
    }
    if( nOffset >= 0 )
    {
        // When just before the insert position a section ends, it is okay when I'm moving backward
        // because I want to stay outside the section.
        // When moving forward I've to check if I started inside or outside the section
        // because I don't want to enter of leave such a section
        while( aInsertPos.GetNode().IsEndNode() )
        {
            pNd = aInsertPos.GetNode().StartOfSectionNode();
            if( pNd->GetIndex() >= aSttRg.GetIndex() )
                break;
            aInsertPos--;
            --nNewPos;
        }
    }
    // We do not want to move into tables (at the moment)
    aInsertPos++;
    pNd = &aInsertPos.GetNode();
    if( pNd->IsTableNode() )
        pNd = pNd->StartOfSectionNode();
    if( pNd->FindTableNode() )
        return sal_False;

    OSL_ENSURE( aSttRg.GetIndex() > nNewPos || nNewPos >= aEndRg.GetIndex(),
                "Position lies within MoveArea" );

    // If a Position was calculated for the special area, we set it
    // to document start instead.
    // Areas or Tables at the document start will be moved to the
    // back.
    nNewPos = Max( nNewPos, GetNodes().GetEndOfExtras().GetIndex() + 2 );

    long nOffs = nNewPos - ( 0 < nOffset ? aEndRg.GetIndex() : aSttRg.GetIndex());
    SwPaM aPam( aSttRg, aEndRg, 0, -1 );
    return MoveParagraph( aPam, nOffs, sal_True );
}

static sal_uInt16 lcl_FindOutlineName( const SwNodes& rNds, const String& rName,
                            sal_Bool bExact )
{
    sal_uInt16 nSavePos = USHRT_MAX;
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    for( sal_uInt16 n = 0; n < rOutlNds.size(); ++n )
    {
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        String sTxt( pTxtNd->GetExpandTxt() );
        if( sTxt.Equals( rName ) )
        {
            // Found "exact", set Pos to the Node
            nSavePos = n;
            break;
        }
        else if( !bExact && USHRT_MAX == nSavePos &&
                    COMPARE_EQUAL == sTxt.CompareTo( rName, rName.Len()) )
        {
            // maybe we just found the text's first part
            nSavePos = n;
        }
    }

    return nSavePos;
}

static sal_uInt16 lcl_FindOutlineNum( const SwNodes& rNds, String& rName )
{
    // Valid numbers are (always just offsets!):
    //  ([Number]+\.)+  (as a regular expression!)
    //  (Number follwed by a period, with 5 repetitions)
    //  i.e.: "1.1.", "1.", "1.1.1."
    xub_StrLen nPos = 0;
    String sNum = rName.GetToken( 0, '.', nPos );
    if( STRING_NOTFOUND == nPos )
        return USHRT_MAX;           // invalid number!

    sal_uInt16 nLevelVal[ MAXLEVEL ];       // numbers of all levels
    memset( nLevelVal, 0, MAXLEVEL * sizeof( nLevelVal[0] ));
    sal_uInt8 nLevel = 0;
    String sName( rName );

    while( STRING_NOTFOUND != nPos )
    {
        sal_uInt16 nVal = 0;
        sal_Unicode c;
        for( sal_uInt16 n = 0; n < sNum.Len(); ++n )
            if( '0' <= ( c = sNum.GetChar( n )) && c <= '9' )
            {
                nVal *= 10;  nVal += c - '0';
            }
            else if( nLevel )
                break;                      // "almost" valid number
            else
                return USHRT_MAX;           // invalid number!

        if( MAXLEVEL > nLevel )
            nLevelVal[ nLevel++ ] = nVal;

        sName.Erase( 0, nPos );
        nPos = 0;
        sNum = sName.GetToken( 0, '.', nPos );
        // #i4533# without this check all parts delimited by a dot are treated as outline numbers
        if(!comphelper::string::isdigitAsciiString(sNum))
            nPos = STRING_NOTFOUND;
    }
    rName = sName;      // that's the follow-up text

    // read all levels, so search the document for this outline
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    // Without OutlineNodes searching doesn't pay off
    // and we save a crash
    if( rOutlNds.empty() )
        return USHRT_MAX;
    SwTxtNode* pNd;
    nPos = 0;
    // search in the existing outline nodes for the required outline num array
    for( ; nPos < rOutlNds.size(); ++nPos )
    {
        pNd = rOutlNds[ nPos ]->GetTxtNode();
        const int nLvl = pNd->GetAttrOutlineLevel()-1;   //<-end,zhaojianwei
        if( nLvl == nLevel - 1)
        {
            // #i51089#, #i68289#
            // Assure, that text node has the correct numbering level. Otherwise,
            // its number vector will not fit to the searched level.
            if ( pNd->GetNum() &&
                 pNd->GetActualListLevel() == ( nLevel - 1 ) )
            {
                const SwNodeNum & rNdNum = *(pNd->GetNum());
                SwNumberTree::tNumberVector aLevelVal = rNdNum.GetNumberVector();
                // now compare with the one searched for
                bool bEqual = true;
                for( sal_uInt8 n = 0; (n < nLevel) && bEqual; ++n )
                {
                    bEqual = aLevelVal[n] == nLevelVal[n];
                }
                if(bEqual)
                {
                    break;
                }
            }
            else
            {
                // A text node, which has an outline paragraph style applied and
                // has as hard attribute 'no numbering' set, has an outline level,
                // but no numbering tree node. Thus, consider this situation in
                // the assertion condition.
                OSL_ENSURE( !pNd->GetNumRule(),
                        "<lcl_FindOutlineNum(..)> - text node with outline level and numbering rule, but without numbering tree node. This is a serious defect -> inform OD" );
            }
        }
    }
    if( nPos >= rOutlNds.size() )
        nPos = USHRT_MAX;
    return nPos;
}

// Ad this bullet point:

// A Name can contain a Number and/or the Text.
//
// First, we try to find the correct Entry via the Number.
// If it exists, we compare the Text, to see if it's the right one.
// If that's not the case, we search again via the Text. If it is
// found, we got the right entry. Or else we use the one found by
// searching for the Number.
// If we don't have a Number, we search via the Text only.
sal_Bool SwDoc::GotoOutline( SwPosition& rPos, const String& rName ) const
{
    if( rName.Len() )
    {
        const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();

        // 1. step: via the Number:
        String sName( rName );
        sal_uInt16 nFndPos = ::lcl_FindOutlineNum( GetNodes(), sName );
        if( USHRT_MAX != nFndPos )
        {
            SwTxtNode* pNd = rOutlNds[ nFndPos ]->GetTxtNode();
            String sExpandedText = pNd->GetExpandTxt();
            //#i4533# leading numbers followed by a dot have been remove while
            //searching for the outline position
            //to compensate this they must be removed from the paragraphs text content, too
            sal_uInt16 nPos = 0;
            String sTempNum;
            while(sExpandedText.Len() && (sTempNum = sExpandedText.GetToken(0, '.', nPos)).Len() &&
                    STRING_NOTFOUND != nPos &&
                    comphelper::string::isdigitAsciiString(sTempNum))
            {
                sExpandedText.Erase(0, nPos);
                nPos = 0;
            }

            if( !sExpandedText.Equals( sName ) )
            {
                sal_uInt16 nTmp = ::lcl_FindOutlineName( GetNodes(), sName, sal_True );
                if( USHRT_MAX != nTmp )             // found via the Name
                {
                    nFndPos = nTmp;
                    pNd = rOutlNds[ nFndPos ]->GetTxtNode();
                }
            }
            rPos.nNode = *pNd;
            rPos.nContent.Assign( pNd, 0 );
            return sal_True;
        }

        nFndPos = ::lcl_FindOutlineName( GetNodes(), rName, sal_False );
        if( USHRT_MAX != nFndPos )
        {
            SwTxtNode* pNd = rOutlNds[ nFndPos ]->GetTxtNode();
            rPos.nNode = *pNd;
            rPos.nContent.Assign( pNd, 0 );
            return sal_True;
        }

        // #i68289# additional search on hyperlink URL without its outline numbering part
        if ( !sName.Equals( rName ) )
        {
            nFndPos = ::lcl_FindOutlineName( GetNodes(), sName, sal_False );
            if( USHRT_MAX != nFndPos )
            {
                SwTxtNode* pNd = rOutlNds[ nFndPos ]->GetTxtNode();
                rPos.nNode = *pNd;
                rPos.nContent.Assign( pNd, 0 );
                return sal_True;
            }
        }
    }
    return sal_False;
}

static void lcl_ChgNumRule( SwDoc& rDoc, const SwNumRule& rRule )
{
    SwNumRule* pOld = rDoc.FindNumRulePtr( rRule.GetName() );
    OSL_ENSURE( pOld, "we cannot proceed without the old NumRule" );

    sal_uInt16 nChgFmtLevel = 0, nMask = 1;
    sal_uInt8 n;

    for( n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
    {
        const SwNumFmt& rOldFmt = pOld->Get( n ),
                      & rNewFmt = rRule.Get( n );

        if( rOldFmt != rNewFmt )
        {
            nChgFmtLevel |= nMask;
        }
        else if( SVX_NUM_NUMBER_NONE > rNewFmt.GetNumberingType() && 1 < rNewFmt.GetIncludeUpperLevels() &&
                0 != (nChgFmtLevel & GetUpperLvlChg( n, rNewFmt.GetIncludeUpperLevels(),nMask )) )
            nChgFmtLevel |= nMask;
    }

    if( !nChgFmtLevel )         // Nothing has been changed?
    {
        const bool bInvalidateNumRule( pOld->IsContinusNum() != rRule.IsContinusNum() );
        pOld->CheckCharFmts( &rDoc );
        pOld->SetContinusNum( rRule.IsContinusNum() );

        if ( bInvalidateNumRule )
        {
            pOld->SetInvalidRule(sal_True);
        }

        return ;
    }

    SwNumRule::tTxtNodeList aTxtNodeList;
    pOld->GetTxtNodeList( aTxtNodeList );
    sal_uInt8 nLvl( 0 );
    for ( SwNumRule::tTxtNodeList::iterator aIter = aTxtNodeList.begin();
          aIter != aTxtNodeList.end(); ++aIter )
    {
        SwTxtNode* pTxtNd = *aIter;
        nLvl = static_cast<sal_uInt8>(pTxtNd->GetActualListLevel());

        if( nLvl < MAXLEVEL )
        {
            if( nChgFmtLevel & ( 1 << nLvl ))
            {
                pTxtNd->NumRuleChgd();
            }
        }
    }

    for( n = 0; n < MAXLEVEL; ++n )
        if( nChgFmtLevel & ( 1 << n ))
            pOld->Set( n, rRule.GetNumFmt( n ));

    pOld->CheckCharFmts( &rDoc );
    pOld->SetInvalidRule(sal_True);
    pOld->SetContinusNum( rRule.IsContinusNum() );

    rDoc.UpdateNumRule();
}

void SwDoc::SetNumRule( const SwPaM& rPam,
                        const SwNumRule& rRule,
                        const bool bCreateNewList,
                        const String sContinuedListId,
                        sal_Bool bSetItem,
                        const bool bResetIndentAttrs )
{
    SwUndoInsNum * pUndo = NULL;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // Start/End for attributes!
        GetIDocumentUndoRedo().StartUndo( UNDO_INSNUM, NULL );
        pUndo = new SwUndoInsNum( rPam, rRule );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    SwNumRule * pNew = FindNumRulePtr( rRule.GetName() );
    bool bUpdateRule = false;

    if( !pNew )
    {
        pNew = (*pNumRuleTbl)[ MakeNumRule( rRule.GetName(), &rRule ) ];
    }
    else if (rRule != *pNew)
    {
        bUpdateRule = true;
    }

    if (bUpdateRule)
    {
        if( pUndo )
        {
            pUndo->SaveOldNumRule( *pNew );
            ::lcl_ChgNumRule( *this, rRule );
            pUndo->SetLRSpaceEndPos();
        }
        else
        {
            ::lcl_ChgNumRule( *this, rRule );
        }
    }

    if ( bSetItem )
    {
        if ( bCreateNewList )
        {
            String sListId;
            if ( !bUpdateRule )
            {
                // apply list id of list, which has been created for the new list style
                sListId = pNew->GetDefaultListId();
            }
            else
            {
                // create new list and apply its list id
                SwList* pNewList = createList( String(), pNew->GetName() );
                OSL_ENSURE( pNewList,
                        "<SwDoc::SetNumRule(..)> - could not create new list. Serious defect -> please inform OD." );
                sListId = pNewList->GetListId();
            }
            InsertPoolItem( rPam,
                SfxStringItem( RES_PARATR_LIST_ID, sListId ), 0 );
        }
        else if ( sContinuedListId.Len() > 0 )
        {
            // apply given list id
            InsertPoolItem( rPam,
                SfxStringItem( RES_PARATR_LIST_ID, sContinuedListId ), 0 );
        }
    }

    if ( ! rPam.HasMark())
    {
        SwTxtNode * pTxtNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
        // consider case that the PaM doesn't denote a text node - e.g. it denotes a graphic node
        if ( pTxtNd )
        {
            SwNumRule * pRule = pTxtNd->GetNumRule();

            if (pRule && pRule->GetName() == pNew->GetName())
            {
                bSetItem = sal_False;

                if ( !pTxtNd->IsInList() )
                {
                    pTxtNd->AddToList();
                }
            }
            // Only clear numbering attribute at text node, if at paragraph
            // style the new numbering rule is found.
            else if ( !pRule )
            {
                SwTxtFmtColl* pColl = pTxtNd->GetTxtColl();
                if ( pColl )
                {
                    SwNumRule* pCollRule = FindNumRulePtr(pColl->GetNumRule().GetValue());
                    if ( pCollRule && pCollRule->GetName() == pNew->GetName() )
                    {
                        pTxtNd->ResetAttr( RES_PARATR_NUMRULE );
                        bSetItem = sal_False;
                    }
                }
            }
        }
    }

    if ( bSetItem )
    {
        InsertPoolItem( rPam, SwNumRuleItem( pNew->GetName() ), 0 );
    }

    if ( bResetIndentAttrs &&
         pNew && pNew->Get( 0 ).GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        ::lcl_ResetIndentAttrs(this, rPam, RES_LR_SPACE);
    }

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().EndUndo( UNDO_INSNUM, NULL );
    }

    SetModified();
}

void SwDoc::SetCounted(const SwPaM & rPam, bool bCounted)
{
    if ( bCounted )
    {
        ::lcl_ResetIndentAttrs(this, rPam, RES_PARATR_LIST_ISCOUNTED);
    }
    else
    {
        InsertPoolItem( rPam,
            SfxBoolItem( RES_PARATR_LIST_ISCOUNTED, sal_False ), 0 );
    }
}

void SwDoc::SetNumRuleStart( const SwPosition& rPos, sal_Bool bFlag )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        const SwNumRule* pRule = pTxtNd->GetNumRule();
        if( pRule && !bFlag != !pTxtNd->IsListRestart())
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoNumRuleStart(rPos, bFlag) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            pTxtNd->SetListRestart(bFlag ? true : false);

            SetModified();
        }
    }
}

void SwDoc::SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        if ( !pTxtNd->HasAttrListRestartValue() ||
             pTxtNd->GetAttrListRestartValue() != nStt )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoNumRuleStart(rPos, nStt) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }
            pTxtNd->SetAttrListRestartValue( nStt );

            SetModified();
        }
    }
}

// We can only delete if the Rule is unused!
sal_Bool SwDoc::DelNumRule( const String& rName, sal_Bool bBroadcast )
{
    sal_uInt16 nPos = FindNumRule( rName );

    if ( (*pNumRuleTbl)[ nPos ] == GetOutlineNumRule() )
    {
        OSL_FAIL( "<SwDoc::DelNumRule(..)> - No deletion of outline list style. This is serious defect - please inform OD" );
        return sal_False;
    }

    if( USHRT_MAX != nPos && !IsUsed( *(*pNumRuleTbl)[ nPos ] ))
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo * pUndo =
                new SwUndoNumruleDelete(*(*pNumRuleTbl)[nPos], this);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        if (bBroadcast)
            BroadcastStyleOperation(rName, SFX_STYLE_FAMILY_PSEUDO,
                                    SFX_STYLESHEET_ERASED);

        deleteListForListStyle( rName );
        {
            // delete further list, which have the deleted list style as default list style
            std::vector< SwList* > aListsForDeletion;
            tHashMapForLists::iterator aListIter = maLists.begin();
            while ( aListIter != maLists.end() )
            {
                SwList* pList = (*aListIter).second;
                if ( pList->GetDefaultListStyleName() == rName )
                {
                    aListsForDeletion.push_back( pList );
                }

                ++aListIter;
            }
            while ( !aListsForDeletion.empty() )
            {
                SwList* pList = aListsForDeletion.back();
                aListsForDeletion.pop_back();
                deleteList( pList->GetListId() );
            }
        }
        // #i34097# DeleteAndDestroy deletes rName if
        // rName is directly taken from the numrule.
        const String aTmpName( rName );
        delete (*pNumRuleTbl)[ nPos ];
        pNumRuleTbl->erase( pNumRuleTbl->begin() + nPos );
        maNumRuleMap.erase(aTmpName);

        SetModified();
        return sal_True;
    }
    return sal_False;
}

void SwDoc::ChgNumRuleFmts( const SwNumRule& rRule, const String * pName )
{
    SwNumRule* pRule = FindNumRulePtr( pName ? *pName : rRule.GetName() );
    if( pRule )
    {
        SwUndoInsNum* pUndo = 0;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo = new SwUndoInsNum( *pRule, rRule );
            pUndo->GetHistory();
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        ::lcl_ChgNumRule( *this, rRule );

        if( pUndo )
            pUndo->SetLRSpaceEndPos();

        SetModified();
    }
}

sal_Bool SwDoc::RenameNumRule(const String & rOldName, const String & rNewName,
                              sal_Bool bBroadcast)
{
    sal_Bool bResult = sal_False;
    SwNumRule * pNumRule = FindNumRulePtr(rOldName);

    if (pNumRule)
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo * pUndo = new SwUndoNumruleRename(rOldName, rNewName, this);
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwNumRule::tTxtNodeList aTxtNodeList;
        pNumRule->GetTxtNodeList( aTxtNodeList );

        pNumRule->SetName( rNewName, *this );

        SwNumRuleItem aItem(rNewName);

        for ( SwNumRule::tTxtNodeList::iterator aIter = aTxtNodeList.begin();
              aIter != aTxtNodeList.end(); ++aIter )
        {
            SwTxtNode * pTxtNd = *aIter;
            pTxtNd->SetAttr(aItem);
        }

        bResult = sal_True;

        if (bBroadcast)
            BroadcastStyleOperation(rOldName, SFX_STYLE_FAMILY_PSEUDO,
                                    SFX_STYLESHEET_MODIFIED);
    }

    return bResult;
}

void SwDoc::StopNumRuleAnimations( OutputDevice* pOut )
{
    for( sal_uInt16 n = GetNumRuleTbl().size(); n; )
    {
        SwNumRule::tTxtNodeList aTxtNodeList;
        GetNumRuleTbl()[ --n ]->GetTxtNodeList( aTxtNodeList );
        for ( SwNumRule::tTxtNodeList::iterator aTxtNodeIter = aTxtNodeList.begin();
              aTxtNodeIter != aTxtNodeList.end(); ++aTxtNodeIter )
        {
            SwTxtNode* pTNd = *aTxtNodeIter;
            SwIterator<SwTxtFrm,SwTxtNode> aIter(*pTNd);
            for(SwTxtFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
                if( pFrm->HasAnimation() )
                    pFrm->StopAnimation( pOut );
        }
    }
}

sal_Bool SwDoc::ReplaceNumRule( const SwPosition& rPos,
                            const String& rOldRule, const String& rNewRule )
{
    sal_Bool bRet = sal_False;
    SwNumRule *pOldRule = FindNumRulePtr( rOldRule ),
              *pNewRule = FindNumRulePtr( rNewRule );
    if( pOldRule && pNewRule && pOldRule != pNewRule )
    {
        SwUndoInsNum* pUndo = 0;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            // Start/End for attributes!
            GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
            pUndo = new SwUndoInsNum( rPos, *pNewRule, rOldRule );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwNumRule::tTxtNodeList aTxtNodeList;
        pOldRule->GetTxtNodeList( aTxtNodeList );
        if ( aTxtNodeList.size() > 0 )
        {

            SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : 0 );
            sal_uInt16 nChgFmtLevel = 0;
            for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
            {
                const SwNumFmt& rOldFmt = pOldRule->Get( n ),
                    & rNewFmt = pNewRule->Get( n );

                if( rOldFmt.GetAbsLSpace() != rNewFmt.GetAbsLSpace() ||
                    rOldFmt.GetFirstLineOffset() != rNewFmt.GetFirstLineOffset() )
                    nChgFmtLevel |= ( 1 << n );
            }

            const SwTxtNode* pGivenTxtNode = rPos.nNode.GetNode().GetTxtNode();
            SwNumRuleItem aRule( rNewRule );

            for ( SwNumRule::tTxtNodeList::iterator aIter = aTxtNodeList.begin();
                  aIter != aTxtNodeList.end(); ++aIter )
            {
                SwTxtNode* pTxtNd = *aIter;

                if ( pGivenTxtNode &&
                     pGivenTxtNode->GetListId() == pTxtNd->GetListId() )
                {
                    aRegH.RegisterInModify( pTxtNd, *pTxtNd );

                    pTxtNd->SetAttr( aRule );
                    pTxtNd->NumRuleChgd();
                }
            }
            GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
            SetModified();

            bRet = sal_True;
        }
    }

    return bRet;
}

namespace
{
    struct ListStyleData
    {
        SwNumRule* pReplaceNumRule;
        bool bCreateNewList;
        String sListId;

        ListStyleData()
            : pReplaceNumRule( 0 ),
              bCreateNewList( false ),
              sListId()
        {}
    };
}

void SwDoc::MakeUniqueNumRules(const SwPaM & rPaM)
{
    OSL_ENSURE( rPaM.GetDoc() == this, "need same doc" );

    ::std::map<SwNumRule *, ListStyleData> aMyNumRuleMap;

     sal_uLong nStt = rPaM.Start()->nNode.GetIndex();
    sal_uLong nEnd = rPaM.End()->nNode.GetIndex();

    bool bFirst = true;

    for (sal_uLong n = nStt; n <= nEnd; n++)
    {
        SwTxtNode * pCNd = GetNodes()[n]->GetTxtNode();

        if (pCNd)
        {
            SwNumRule * pRule = pCNd->GetNumRule();

            if (pRule && pRule->IsAutoRule() && ! pRule->IsOutlineRule())
            {
                ListStyleData aListStyleData = aMyNumRuleMap[pRule];

                if ( aListStyleData.pReplaceNumRule == 0 )
                {
                    if (bFirst)
                    {
                        SwPosition aPos(*pCNd);
                        aListStyleData.pReplaceNumRule =
                            const_cast<SwNumRule *>
                            (SearchNumRule( aPos, false, pCNd->HasNumber(),
                                            false, 0,
                                            aListStyleData.sListId, true ));
                    }

                    if ( aListStyleData.pReplaceNumRule == 0 )
                    {
                        aListStyleData.pReplaceNumRule = new SwNumRule(*pRule);

                        aListStyleData.pReplaceNumRule->SetName(
                                                GetUniqueNumRuleName(), *this );

                        aListStyleData.bCreateNewList = true;
                    }

                    aMyNumRuleMap[pRule] = aListStyleData;
                }

                SwPaM aPam(*pCNd);

                SetNumRule( aPam, *aListStyleData.pReplaceNumRule,
                            aListStyleData.bCreateNewList,
                            aListStyleData.sListId );
                if ( aListStyleData.bCreateNewList )
                {
                    aListStyleData.bCreateNewList = false;
                    aListStyleData.sListId = pCNd->GetListId();
                    aMyNumRuleMap[pRule] = aListStyleData;
                }

                bFirst = false;
            }
        }
    }
}

sal_Bool SwDoc::NoNum( const SwPaM& rPam )
{

    sal_Bool bRet = SplitNode( *rPam.GetPoint(), false );
    // Do we actually use Numbering at all?
    if( bRet )
    {
        // Set NoNum and Upate
        const SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        SwTxtNode* pNd = rIdx.GetNode().GetTxtNode();
        const SwNumRule* pRule = pNd->GetNumRule();
        if( pRule )
        {
            pNd->SetCountedInList(false);

            SetModified();
        }
        else
            bRet = sal_False;   // no Numbering or just always sal_True?
    }
    return bRet;
}

void SwDoc::DelNumRules( const SwPaM& rPam )
{
    sal_uLong nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        sal_uLong nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    SwUndoDelNum* pUndo;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoDelNum( rPam );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
        pUndo = 0;

    SwRegHistory aRegH( pUndo ? pUndo->GetHistory() : 0 );

    SwNumRuleItem aEmptyRule( aEmptyStr );
    const SwNode* pOutlNd = 0;
    for( ; nStt <= nEnd; ++nStt )
    {
        SwTxtNode* pTNd = GetNodes()[ nStt ]->GetTxtNode();
        SwNumRule* pNumRuleOfTxtNode = pTNd ? pTNd->GetNumRule() : 0;
        if ( pTNd && pNumRuleOfTxtNode )
        {
            // recognize changes of attribute for undo
            aRegH.RegisterInModify( pTNd, *pTNd );

            if( pUndo )
                pUndo->AddNode( *pTNd, sal_False );

            // directly set list style attribute is reset, otherwise empty
            // list style is applied
            const SfxItemSet* pAttrSet = pTNd->GetpSwAttrSet();
            if ( pAttrSet &&
                 pAttrSet->GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_SET )
                pTNd->ResetAttr( RES_PARATR_NUMRULE );
            else
                pTNd->SetAttr( aEmptyRule );

            pTNd->ResetAttr( RES_PARATR_LIST_ID );
            pTNd->ResetAttr( RES_PARATR_LIST_LEVEL );
            pTNd->ResetAttr( RES_PARATR_LIST_ISRESTART );
            pTNd->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
            pTNd->ResetAttr( RES_PARATR_LIST_ISCOUNTED );

            if( RES_CONDTXTFMTCOLL == pTNd->GetFmtColl()->Which() )
                pTNd->ChkCondColl();

            else if( !pOutlNd &&
                ((SwTxtFmtColl*)pTNd->GetFmtColl())->IsAssignedToListLevelOfOutlineStyle() )//<-end,zhaojianwei
                pOutlNd = pTNd;
        }
    }

    // Finally, update all
    UpdateNumRule();

    if( pOutlNd )
        GetNodes().UpdtOutlineIdx( *pOutlNd );
}

void SwDoc::InvalidateNumRules()
{
    for (sal_uInt16 n = 0; n < pNumRuleTbl->size(); ++n)
        (*pNumRuleTbl)[n]->SetInvalidRule(sal_True);
}

// To the next/preceding Bullet at the same Level
static sal_Bool lcl_IsNumOk( sal_uInt8 nSrchNum, sal_uInt8& rLower, sal_uInt8& rUpper,
                    sal_Bool bOverUpper, sal_uInt8 nNumber )
{
    OSL_ENSURE( nNumber < MAXLEVEL,
            "<lcl_IsNumOk(..)> - misusage of method" );

    sal_Bool bRet = sal_False;
    {
        if( bOverUpper ? nSrchNum == nNumber : nSrchNum >= nNumber )
            bRet = sal_True;
        else if( nNumber > rLower )
            rLower = nNumber;
        else if( nNumber < rUpper )
            rUpper = nNumber;
    }
    return bRet;
}

static sal_Bool lcl_IsValidPrevNextNumNode( const SwNodeIndex& rIdx )
{
    sal_Bool bRet = sal_False;
    const SwNode& rNd = rIdx.GetNode();
    switch( rNd.GetNodeType() )
    {
    case ND_ENDNODE:
        bRet = SwTableBoxStartNode == rNd.StartOfSectionNode()->GetStartNodeType() ||
                rNd.StartOfSectionNode()->IsSectionNode();
        break;

    case ND_STARTNODE:
        bRet = SwTableBoxStartNode == ((SwStartNode&)rNd).GetStartNodeType();
        break;

    case ND_SECTIONNODE:            // that one's valid, so proceed
        bRet = sal_True;
        break;
    }
    return bRet;
}

static sal_Bool lcl_GotoNextPrevNum( SwPosition& rPos, sal_Bool bNext,
                            sal_Bool bOverUpper, sal_uInt8* pUpper, sal_uInt8* pLower )
{
    const SwTxtNode* pNd = rPos.nNode.GetNode().GetTxtNode();
    const SwNumRule* pRule;
    if( !pNd || 0 == ( pRule = pNd->GetNumRule()))
        return sal_False;

    sal_uInt8 nSrchNum = static_cast<sal_uInt8>(pNd->GetActualListLevel());

    SwNodeIndex aIdx( rPos.nNode );
    if( ! pNd->IsCountedInList() )
    {
        // If NO_NUMLEVEL is switched on, we search the preceding Node with Numbering
        sal_Bool bError = sal_False;
        do {
            aIdx--;
            if( aIdx.GetNode().IsTxtNode() )
            {
                pNd = aIdx.GetNode().GetTxtNode();
                pRule = pNd->GetNumRule();

                sal_uInt8 nTmpNum;

                if( pRule  )
                {
                    nTmpNum = static_cast<sal_uInt8>(pNd->GetActualListLevel());
                    if( !( ! pNd->IsCountedInList() &&
                         (nTmpNum >= nSrchNum )) )
                        break;      // found it!
                }
                else
                    bError = sal_True;
            }
            else
                bError = !lcl_IsValidPrevNextNumNode( aIdx );

        } while( !bError );
        if( bError )
            return sal_False;
    }

    sal_uInt8 nLower = nSrchNum, nUpper = nSrchNum;
    sal_Bool bRet = sal_False;

    const SwTxtNode* pLast;
    if( bNext )
        aIdx++, pLast = pNd;
    else
        aIdx--, pLast = 0;

    while( bNext ? ( aIdx.GetIndex() < aIdx.GetNodes().Count() - 1 )
                 : aIdx.GetIndex() )
    {
        if( aIdx.GetNode().IsTxtNode() )
        {
            pNd = aIdx.GetNode().GetTxtNode();
            pRule = pNd->GetNumRule();
            if( pRule )
            {
                if( ::lcl_IsNumOk( nSrchNum, nLower, nUpper, bOverUpper,
                                    static_cast<sal_uInt8>(pNd->GetActualListLevel()) ))
                {
                    rPos.nNode = aIdx;
                    rPos.nContent.Assign( (SwTxtNode*)pNd, 0 );
                    bRet = sal_True;
                    break;
                }
                else
                    pLast = pNd;
            }
            else
                break;
        }
        else if( !lcl_IsValidPrevNextNumNode( aIdx ))
            break;

        if( bNext )
            aIdx++;
        else
            aIdx--;
    }

    if( !bRet && !bOverUpper && pLast )     // do not iterate over higher numbers, but still to the end
    {
        if( bNext )
        {
            rPos.nNode = aIdx;
            if( aIdx.GetNode().IsCntntNode() )
                rPos.nContent.Assign( aIdx.GetNode().GetCntntNode(), 0 );
        }
        else
        {
            rPos.nNode.Assign( *pLast );
            rPos.nContent.Assign( (SwTxtNode*)pLast, 0 );
        }
        bRet = sal_True;
    }

    if( bRet )
    {
        if( pUpper )
            *pUpper = nUpper;
        if( pLower )
            *pLower = nLower;
    }
    return bRet;
}

sal_Bool SwDoc::GotoNextNum( SwPosition& rPos, sal_Bool bOverUpper,
                            sal_uInt8* pUpper, sal_uInt8* pLower  )
{
   return ::lcl_GotoNextPrevNum( rPos, sal_True, bOverUpper, pUpper, pLower );
}

const SwNumRule *  SwDoc::SearchNumRule(const SwPosition & rPos,
                                        const bool bForward,
                                        const bool bNum,
                                        const bool bOutline,
                                        int nNonEmptyAllowed,
                                        String& sListId,
                                        const bool bInvestigateStartNode)
{
    const SwNumRule * pResult = NULL;
    SwTxtNode * pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    SwNode * pStartFromNode = pTxtNd;

    if (pTxtNd)
    {
        SwNodeIndex aIdx(rPos.nNode);

        // - the start node has also been investigated, if requested.
        const SwNode * pNode = NULL;
        do
        {
            if ( !bInvestigateStartNode )
            {
                if (bForward)
                    aIdx++;
                else
                    aIdx--;
            }

            if (aIdx.GetNode().IsTxtNode())
            {
                pTxtNd = aIdx.GetNode().GetTxtNode();

                const SwNumRule * pNumRule = pTxtNd->GetNumRule();
                if (pNumRule)
                {
                    if ( ( pNumRule->IsOutlineRule() == ( bOutline ? sal_True : sal_False ) ) &&
                         ( ( bNum && pNumRule->Get(0).IsEnumeration()) ||
                           ( !bNum && pNumRule->Get(0).IsItemize() ) ) ) // #i22362#, #i29560#
                    {
                        pResult = pTxtNd->GetNumRule();
                        // provide also the list id, to which the text node belongs.
                        sListId = pTxtNd->GetListId();
                    }

                    break;
                }
                else if (pTxtNd->Len() > 0 || NULL != pTxtNd->GetNumRule())
                {
                    if (nNonEmptyAllowed == 0)
                        break;

                    nNonEmptyAllowed--;

                    if (nNonEmptyAllowed < 0)
                        nNonEmptyAllowed = -1;
                }
            }

            if ( bInvestigateStartNode )
            {
                if (bForward)
                    aIdx++;
                else
                    aIdx--;
            }

            pNode = &aIdx.GetNode();
        }
        while (!(pNode == GetNodes().DocumentSectionStartNode(pStartFromNode) ||
                 pNode == GetNodes().DocumentSectionEndNode(pStartFromNode)));
    }

    return pResult;
}


sal_Bool SwDoc::GotoPrevNum( SwPosition& rPos, sal_Bool bOverUpper,
                            sal_uInt8* pUpper, sal_uInt8* pLower  )
{
   return ::lcl_GotoNextPrevNum( rPos, sal_False, bOverUpper, pUpper, pLower );
}

sal_Bool SwDoc::NumUpDown( const SwPaM& rPam, sal_Bool bDown )
{
    sal_uLong nStt = rPam.GetPoint()->nNode.GetIndex(),
            nEnd = rPam.GetMark()->nNode.GetIndex();
    if( nStt > nEnd )
    {
        sal_uLong nTmp = nStt; nStt = nEnd; nEnd = nTmp;
    }

    // -> outline nodes are promoted or demoted differently
    bool bOnlyOutline = true;
    bool bOnlyNonOutline = true;
    for (sal_uLong n = nStt; n <= nEnd; n++)
    {
        SwTxtNode * pTxtNd = GetNodes()[n]->GetTxtNode();

        if (pTxtNd)
        {
            SwNumRule * pRule = pTxtNd->GetNumRule();

            if (pRule)
            {
                if (pRule->IsOutlineRule())
                    bOnlyNonOutline = false;
                else
                    bOnlyOutline = false;
            }
        }
    }

    sal_Bool bRet = sal_True;
    sal_Int8 nDiff = bDown ? 1 : -1;

    if (bOnlyOutline)
        bRet = OutlineUpDown(rPam, nDiff);
    else if (bOnlyNonOutline)
    {
        /* #i24560#

        Only promote or demote if all selected paragraphs are
        promotable resp. demotable.

        */
        for (sal_uLong nTmp = nStt; nTmp <= nEnd; ++nTmp)
        {
            SwTxtNode* pTNd = GetNodes()[ nTmp ]->GetTxtNode();

            // Make code robust: consider case that the node doesn't denote a
            // text node.
            if ( pTNd )
            {
                SwNumRule * pRule = pTNd->GetNumRule();

                if (pRule)
                {
                    sal_uInt8 nLevel = static_cast<sal_uInt8>(pTNd->GetActualListLevel());
                    if( (-1 == nDiff && 0 >= nLevel) ||
                        (1 == nDiff && MAXLEVEL - 1 <= nLevel))
                        bRet = sal_False;
                }
            }
        }

        if( bRet )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoNumUpDown(rPam, nDiff) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            String sNumRule;

            for(sal_uLong nTmp = nStt; nTmp <= nEnd; ++nTmp )
            {
                SwTxtNode* pTNd = GetNodes()[ nTmp ]->GetTxtNode();

                if( pTNd)
                {
                    SwNumRule * pRule = pTNd->GetNumRule();

                    if (pRule)
                    {
                        sal_uInt8 nLevel = static_cast<sal_uInt8>(pTNd->GetActualListLevel());
                        nLevel = nLevel + nDiff;

                        pTNd->SetAttrListLevel(nLevel);
                    }
                }
            }

            ChkCondColls();
            SetModified();
        }
    }

    return bRet;
}

sal_Bool SwDoc::MoveParagraph( const SwPaM& rPam, long nOffset, sal_Bool bIsOutlMv )
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    sal_uLong nStIdx = pStt->nNode.GetIndex();
    sal_uLong nEndIdx = pEnd->nNode.GetIndex();

    // Here are some sophisticated checks whether the wished PaM will be moved or not.
    // For moving outlines (bIsOutlMv) I've already done some checks, so here are two different
    // checks...
    SwNode *pTmp1;
    SwNode *pTmp2;
    if( bIsOutlMv )
    {
        // For moving chapters (outline) the following reason will deny the move:
        // if a start node is inside the moved area and its end node outside or vice versa.
        // If a start node is the first moved paragraph, its end node has to be within the moved
        // area, too (e.g. as last node).
        // If an end node is the last node of the moved area, its start node has to be a part of
        // the moved section, too.
        pTmp1 = GetNodes()[ nStIdx ];
        if( pTmp1->IsStartNode() )
        {   // First is a start node
            pTmp2 = pTmp1->EndOfSectionNode();
            if( pTmp2->GetIndex() > nEndIdx )
                return sal_False; // Its end node is behind the moved range
        }
        pTmp1 = pTmp1->StartOfSectionNode()->EndOfSectionNode();
        if( pTmp1->GetIndex() <= nEndIdx )
            return sal_False; // End node inside but start node before moved range => no.
        pTmp1 = GetNodes()[ nEndIdx ];
        if( pTmp1->IsEndNode() )
        {   // The last one is an end node
            pTmp1 = pTmp1->StartOfSectionNode();
            if( pTmp1->GetIndex() < nStIdx )
                return sal_False; // Its start node is before the moved range.
        }
        pTmp1 = pTmp1->StartOfSectionNode();
        if( pTmp1->GetIndex() >= nStIdx )
            return sal_False; // A start node which ends behind the moved area => no.
    }

    sal_uLong nInStIdx, nInEndIdx;
    long nOffs = nOffset;
    if( nOffset > 0 )
    {
        nInEndIdx = nEndIdx;
        nEndIdx += nOffset;
        ++nOffs;
    }
    else
    {
        // Impossible to move to negative index
        if( sal_uLong(abs( nOffset )) > nStIdx)
            return sal_False;

        nInEndIdx = nStIdx - 1;
        nStIdx += nOffset;
    }
    nInStIdx = nInEndIdx + 1;
    // The following paragraphs shall be swapped:
    // Swap [ nStIdx, nInEndIdx ] with [ nInStIdx, nEndIdx ]

    if( nEndIdx >= GetNodes().GetEndOfContent().GetIndex() )
        return sal_False;

    if( !bIsOutlMv )
    {   // And here the restrictions for moving paragraphs other than chapters (outlines)
        // The plan is to exchange [nStIdx,nInEndIdx] and [nStartIdx,nEndIdx]
        // It will checked if the both "start" nodes as well as the both "end" notes belongs to
        // the same start-end-section. This is more restrictive than the conditions checked above.
        // E.g. a paragraph will not escape from a section or be inserted to another section.
        pTmp1 = GetNodes()[ nStIdx ]->StartOfSectionNode();
        pTmp2 = GetNodes()[ nInStIdx ]->StartOfSectionNode();
        if( pTmp1 != pTmp2 )
            return sal_False; // "start" nodes in different sections
        pTmp1 = GetNodes()[ nEndIdx ];
        bool bIsEndNode = pTmp1->IsEndNode();
        if( !pTmp1->IsStartNode() )
        {
            pTmp1 = pTmp1->StartOfSectionNode();
            if( bIsEndNode ) // For end nodes the first start node is of course inside the range,
                pTmp1 = pTmp1->StartOfSectionNode(); // I've to check the start node of the start node.
        }
        pTmp1 = pTmp1->EndOfSectionNode();
        pTmp2 = GetNodes()[ nInEndIdx ];
        if( !pTmp2->IsStartNode() )
        {
            bIsEndNode = pTmp2->IsEndNode();
            pTmp2 = pTmp2->StartOfSectionNode();
            if( bIsEndNode )
                pTmp2 = pTmp2->StartOfSectionNode();
        }
        pTmp2 = pTmp2->EndOfSectionNode();
        if( pTmp1 != pTmp2 )
            return sal_False; // The "end" notes are in different sections
    }

    // Test for Redlining - Can the Selection be moved at all, actually?
    if( !IsIgnoreRedline() )
    {
        sal_uInt16 nRedlPos = GetRedlinePos( pStt->nNode.GetNode(), nsRedlineType_t::REDLINE_DELETE );
        if( USHRT_MAX != nRedlPos )
        {
            SwPosition aStPos( *pStt ), aEndPos( *pEnd );
            aStPos.nContent = 0;
            SwCntntNode* pCNd = pEnd->nNode.GetNode().GetCntntNode();
            aEndPos.nContent = pCNd ? pCNd->Len() : 1;
            sal_Bool bCheckDel = sal_True;

            // There is a some Redline Delete Object for the Area
            for( ; nRedlPos < GetRedlineTbl().size(); ++nRedlPos )
            {
                const SwRedline* pTmp = GetRedlineTbl()[ nRedlPos ];
                if( !bCheckDel || nsRedlineType_t::REDLINE_DELETE == pTmp->GetType() )
                {
                    const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                    switch( ComparePosition( *pRStt, *pREnd, aStPos, aEndPos ))
                    {
                    case POS_COLLIDE_START:
                    case POS_BEHIND:            // Pos1 comes after Pos2
                        nRedlPos = GetRedlineTbl().size();
                        break;

                    case POS_COLLIDE_END:
                    case POS_BEFORE:            // Pos1 comes before Pos2
                        break;
                    case POS_INSIDE:            // Pos1 is completely inside Pos2
                        // that's valid, but check all following for overlapping
                        bCheckDel = sal_False;
                        break;

                    case POS_OUTSIDE:           // Pos2 is completely inside Pos1
                    case POS_EQUAL:             // Pos1 is as big as Pos2
                    case POS_OVERLAP_BEFORE:    // Pos1 overlaps Pos2 in the beginning
                    case POS_OVERLAP_BEHIND:    // Pos1 overlaps Pos2 at the end
                        return sal_False;
                    }
                }
            }
        }
    }

    {
        // Send DataChanged before moving. We then can detect
        // which objects are still in the Area.
        // After the move they could come before/after the
        // Position.
        SwDataChanged aTmp( rPam );
    }

    SwNodeIndex aIdx( nOffset > 0 ? pEnd->nNode : pStt->nNode, nOffs );
    SwNodeRange aMvRg( pStt->nNode, 0, pEnd->nNode, +1 );

    SwRedline* pOwnRedl = 0;
    if( IsRedlineOn() )
    {
        // If the Area is completely in the own Redline, we can move it!
        sal_uInt16 nRedlPos = GetRedlinePos( pStt->nNode.GetNode(), nsRedlineType_t::REDLINE_INSERT );
        if( USHRT_MAX != nRedlPos )
        {
            SwRedline* pTmp = GetRedlineTbl()[ nRedlPos ];
            const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
            SwRedline aTmpRedl( nsRedlineType_t::REDLINE_INSERT, rPam );
            const SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();
            // Is completely in the Area and is the own Redline too?
            if( aTmpRedl.IsOwnRedline( *pTmp ) &&
                (pRStt->nNode < pStt->nNode ||
                (pRStt->nNode == pStt->nNode && !pRStt->nContent.GetIndex()) ) &&
                (pEnd->nNode < pREnd->nNode ||
                (pEnd->nNode == pREnd->nNode &&
                 pCEndNd ? pREnd->nContent.GetIndex() == pCEndNd->Len()
                         : !pREnd->nContent.GetIndex() )) )
            {
                pOwnRedl = pTmp;
                if( nRedlPos + 1 < (sal_uInt16)GetRedlineTbl().size() )
                {
                    pTmp = GetRedlineTbl()[ nRedlPos+1 ];
                    if( *pTmp->Start() == *pREnd )
                        // then don't!
                        pOwnRedl = 0;
                }

                if( pOwnRedl &&
                    !( pRStt->nNode <= aIdx && aIdx <= pREnd->nNode ))
                {
                    // it's not in itself, so don't move it
                    pOwnRedl = 0;
                }
            }
        }

        if( !pOwnRedl )
        {
            GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

            // First the Insert, then the Delete
            SwPosition aInsPos( aIdx );
            aInsPos.nContent.Assign( aIdx.GetNode().GetCntntNode(), 0 );

            SwPaM aPam( pStt->nNode, aMvRg.aEnd );

            SwPaM& rOrigPam = (SwPaM&)rPam;
            rOrigPam.DeleteMark();
            rOrigPam.GetPoint()->nNode = aIdx.GetIndex() - 1;

            sal_Bool bDelLastPara = !aInsPos.nNode.GetNode().IsCntntNode();

            /* When copying to a non-content node Copy will
               insert a paragraph before that node and insert before
               that inserted node. Copy creates an SwUndoInserts that
               does not cover the extra paragraph. Thus we insert the
               extra paragraph ourselves, _with_ correct undo
               information. */
            if (bDelLastPara)
            {
                /* aInsPos points to the non-content node. Move it to
                   the previous content node. */
                SwPaM aInsPam(aInsPos);
                sal_Bool bMoved = aInsPam.Move(fnMoveBackward);
                OSL_ENSURE(bMoved, "No content node found!");

                if (bMoved)
                {
                    /* Append the new node after the content node
                       found. The new position to insert the moved
                       paragraph at is before the inserted
                       paragraph. */
                    AppendTxtNode(*aInsPam.GetPoint());
                    aInsPos = *aInsPam.GetPoint();
                }
            }

            CopyRange( aPam, aInsPos, false );
            if( bDelLastPara )
            {
                // We need to remove the last empty Node again
                aIdx = aInsPos.nNode;
                SwCntntNode* pCNd = GetNodes().GoPrevious( &aInsPos.nNode );
                xub_StrLen nCLen = 0; if( pCNd ) nCLen = pCNd->Len();
                aInsPos.nContent.Assign( pCNd, nCLen );

                // All, that are in the to-be-deleted Node, need to be
                // moved to the next Node
                SwPosition* pPos;
                for( sal_uInt16 n = 0; n < GetRedlineTbl().size(); ++n )
                {
                    SwRedline* pTmp = GetRedlineTbl()[ n ];
                    if( ( pPos = &pTmp->GetBound(sal_True))->nNode == aIdx )
                    {
                        pPos->nNode++;
                        pPos->nContent.Assign( pPos->nNode.GetNode().GetCntntNode(),0);
                    }
                    if( ( pPos = &pTmp->GetBound(sal_False))->nNode == aIdx )
                    {
                        pPos->nNode++;
                        pPos->nContent.Assign( pPos->nNode.GetNode().GetCntntNode(),0);
                    }
                }
                CorrRel( aIdx, aInsPos, 0, sal_False );

                pCNd->JoinNext();
            }

            rOrigPam.GetPoint()->nNode++;
            rOrigPam.GetPoint()->nContent.Assign( rOrigPam.GetCntntNode(), 0 );

            RedlineMode_t eOld = GetRedlineMode();
            checkRedlining(eOld);
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                // Still NEEDS to be optimized (even after 14 years)
                SetRedlineMode(
                   (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE));
                SwUndo *const pUndo(new SwUndoRedlineDelete(aPam, UNDO_DELETE));
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            SwRedline* pNewRedline = new SwRedline( nsRedlineType_t::REDLINE_DELETE, aPam );

            // prevent assertion from aPam's target being deleted
            // (Alternatively, one could just let aPam go out of scope, but
            // that requires touching a lot of code.)
            aPam.GetBound(sal_True).nContent.Assign( NULL, 0 );
            aPam.GetBound(sal_False).nContent.Assign( NULL, 0 );

            AppendRedline( pNewRedline, true );

            // Still NEEDS to be optimized!
            SetRedlineMode( eOld );
            GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
            SetModified();

            return sal_True;
        }
    }

    if( !pOwnRedl && !IsIgnoreRedline() && !GetRedlineTbl().empty() )
    {
        SwPaM aTemp(aIdx);
        SplitRedline(aTemp);
    }

    sal_uLong nRedlSttNd(0), nRedlEndNd(0);
    if( pOwnRedl )
    {
        const SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        nRedlSttNd = pRStt->nNode.GetIndex();
        nRedlEndNd = pREnd->nNode.GetIndex();
    }

    SwUndoMoveNum* pUndo = 0;
    sal_uLong nMoved = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMoveNum( rPam, nOffset, bIsOutlMv );
        nMoved = rPam.End()->nNode.GetIndex() - rPam.Start()->nNode.GetIndex() + 1;
    }


    MoveNodeRange( aMvRg, aIdx, DOC_MOVEREDLINES );

    if( pUndo )
    {
        // i57907: Under circumstances (sections at the end of a chapter)
        // the rPam.Start() is not moved to the new position.
        // But aIdx should be at the new end position and as long as the
        // number of moved paragraphs is nMoved, I know, where the new
        // position is.
        pUndo->SetStartNode( aIdx.GetIndex() - nMoved );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if( pOwnRedl )
    {
        SwPosition *pRStt = pOwnRedl->Start(), *pREnd = pOwnRedl->End();
        if( pRStt->nNode.GetIndex() != nRedlSttNd )
        {
            pRStt->nNode = nRedlSttNd;
            pRStt->nContent.Assign( pRStt->nNode.GetNode().GetCntntNode(),0);
        }
        if( pREnd->nNode.GetIndex() != nRedlEndNd )
        {
            pREnd->nNode = nRedlEndNd;
            SwCntntNode* pCNd = pREnd->nNode.GetNode().GetCntntNode();
            xub_StrLen nL = 0; if( pCNd ) nL = pCNd->Len();
            pREnd->nContent.Assign( pCNd, nL );
        }
    }

    SetModified();
    return sal_True;
}

sal_Bool SwDoc::NumOrNoNum( const SwNodeIndex& rIdx, sal_Bool bDel )
{
    sal_Bool bResult = sal_False;
    SwTxtNode * pTxtNd = rIdx.GetNode().GetTxtNode();

    if (pTxtNd && pTxtNd->GetNumRule() != NULL &&
        (pTxtNd->HasNumber() || pTxtNd->HasBullet()))
    {
        if ( !pTxtNd->IsCountedInList() == !bDel)
        {
            sal_Bool bOldNum = bDel; // == pTxtNd->IsCounted();
            sal_Bool bNewNum = bDel ? sal_False : sal_True;
            pTxtNd->SetCountedInList(bNewNum ? true : false);

            SetModified();

            bResult = sal_True;

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndoNumOrNoNum * pUndo =
                    new SwUndoNumOrNoNum(rIdx, bOldNum, bNewNum);

                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }
        }
        else if (bDel && pTxtNd->GetNumRule(sal_False) &&
                 pTxtNd->GetActualListLevel() >= 0 &&
                 pTxtNd->GetActualListLevel() < MAXLEVEL)
        {
            SwPaM aPam(*pTxtNd);

            DelNumRules(aPam);

            bResult = sal_True;
        }
    }

    return bResult;
}

SwNumRule* SwDoc::GetCurrNumRule( const SwPosition& rPos ) const
{
    SwNumRule* pRet = 0;
    SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();

    if( pTNd )
    {
        pRet = pTNd->GetNumRule();
    }

    return pRet;
}

sal_uInt16 SwDoc::FindNumRule( const String& rName ) const
{
    for( sal_uInt16 n = pNumRuleTbl->size(); n; )
        if( (*pNumRuleTbl)[ --n ]->GetName() == rName )
            return n;

    return USHRT_MAX;
}

SwNumRule* SwDoc::FindNumRulePtr( const String& rName ) const
{
    SwNumRule * pResult = 0;

    pResult = maNumRuleMap[rName];

    if ( !pResult )
    {
        for (sal_uInt16 n = 0; n < pNumRuleTbl->size(); ++n)
        {
            if ((*pNumRuleTbl)[n]->GetName() == rName)
            {
                pResult = (*pNumRuleTbl)[n];

                break;
            }
        }
    }

    return pResult;
}

void SwDoc::AddNumRule(SwNumRule * pRule)
{
    if ((SAL_MAX_UINT16 - 1) <= pNumRuleTbl->size())
    {
        OSL_ENSURE(false, "SwDoc::AddNumRule: table full.");
        abort(); // this should never happen on real documents
    }
    pNumRuleTbl->push_back(pRule);
    maNumRuleMap[pRule->GetName()] = pRule;
    pRule->SetNumRuleMap(&maNumRuleMap);

    createListForListStyle( pRule->GetName() );
}

sal_uInt16 SwDoc::MakeNumRule( const String &rName,
            const SwNumRule* pCpy,
            sal_Bool bBroadcast,
            const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode )
{
    SwNumRule* pNew;
    if( pCpy )
    {
        pNew = new SwNumRule( *pCpy );

        pNew->SetName( GetUniqueNumRuleName( &rName ), *this );

        if( pNew->GetName() != rName )
        {
            pNew->SetPoolFmtId( USHRT_MAX );
            pNew->SetPoolHelpId( USHRT_MAX );
            pNew->SetPoolHlpFileId( UCHAR_MAX );
            pNew->SetDefaultListId( String() );
        }
        pNew->CheckCharFmts( this );
    }
    else
    {
        pNew = new SwNumRule( GetUniqueNumRuleName( &rName ),
                              eDefaultNumberFormatPositionAndSpaceMode );
    }

    sal_uInt16 nRet = pNumRuleTbl->size();

    AddNumRule(pNew);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoNumruleCreate(pNew, this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(pNew->GetName(), SFX_STYLE_FAMILY_PSEUDO,
                                SFX_STYLESHEET_CREATED);

    return nRet;
}

String SwDoc::GetUniqueNumRuleName( const String* pChkStr, sal_Bool bAutoNum ) const
{
    String aName;
    if( bAutoNum )
    {
        long n = Time( Time::SYSTEM ).GetTime();
        n += Date( Date::SYSTEM ).GetDate();
        aName = String::CreateFromInt32( n );
        if( pChkStr && !pChkStr->Len() )
            pChkStr = 0;
    }
    else if( pChkStr && pChkStr->Len() )
        aName = *pChkStr;
    else
    {
        pChkStr = 0;
        aName = SW_RESSTR( STR_NUMRULE_DEFNAME );
    }

    sal_uInt16 nNum(0), nTmp, nFlagSize = ( pNumRuleTbl->size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    xub_StrLen nNmLen = aName.Len();
    if( !bAutoNum && pChkStr )
    {
        while( nNmLen-- && '0' <= aName.GetChar( nNmLen ) &&
                           '9' >= aName.GetChar( nNmLen ) )
            ; //nop

        if( ++nNmLen < aName.Len() )
        {
            aName.Erase( nNmLen );
            pChkStr = 0;
        }
    }

    const SwNumRule* pNumRule;
    sal_uInt16 n;

    for( n = 0; n < pNumRuleTbl->size(); ++n )
        if( 0 != ( pNumRule = (*pNumRuleTbl)[ n ] ) )
        {
            const String& rNm = pNumRule->GetName();
            if( rNm.Match( aName ) == nNmLen )
            {
                // Determine Number and set the Flag
                nNum = (sal_uInt16)rNm.Copy( nNmLen ).ToInt32();
                if( nNum-- && nNum < pNumRuleTbl->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && pChkStr->Equals( rNm ) )
                pChkStr = 0;
        }

    if( !pChkStr )
    {
        // All Numbers have been flagged accordingly, so identify the right Number
        nNum = pNumRuleTbl->size();
        for( n = 0; n < nFlagSize; ++n )
            if( 0xff != ( nTmp = pSetFlags[ n ] ))
            {
                // identify the Number
                nNum = n * 8;
                while( nTmp & 1 )
                    ++nNum, nTmp >>= 1;
                break;
            }

    }
    delete [] pSetFlags;
    if( pChkStr && pChkStr->Len() )
        return *pChkStr;
    return aName += String::CreateFromInt32( ++nNum );
}

void SwDoc::UpdateNumRule()
{
    const SwNumRuleTbl& rNmTbl = GetNumRuleTbl();
    for( sal_uInt16 n = 0; n < rNmTbl.size(); ++n )
        if( rNmTbl[ n ]->IsInvalidRule() )
            rNmTbl[ n ]->Validate();
}

void SwDoc::MarkListLevel( const String& sListId,
                           const int nListLevel,
                           const sal_Bool bValue )
{
    SwList* pList = getListByName( sListId );

    if ( pList )
    {
        MarkListLevel( *pList, nListLevel, bValue );
    }
}

void SwDoc::MarkListLevel( SwList& rList,
                           const int nListLevel,
                           const sal_Bool bValue )
{
    // Set new marked list level and notify all affected nodes of the changed mark.
    rList.MarkListLevel( nListLevel, bValue );
}

sal_Bool SwDoc::IsFirstOfNumRule(SwPosition & rPos)
{
    sal_Bool bResult = sal_False;
    SwTxtNode * pTxtNode = rPos.nNode.GetNode().GetTxtNode();

    if (pTxtNode)
    {
        SwNumRule * pNumRule = pTxtNode->GetNumRule();

        if (pNumRule)
            bResult = pTxtNode->IsFirstOfNumRule();
    }

    return bResult;
}

// implementation for interface <IDocumentListItems>
bool SwDoc::lessThanNodeNum::operator()( const SwNodeNum* pNodeNumOne,
                                         const SwNodeNum* pNodeNumTwo ) const
{
    return pNodeNumOne->LessThan( *pNodeNumTwo );
}

void SwDoc::addListItem( const SwNodeNum& rNodeNum )
{
    if ( mpListItemsList == 0 )
    {
        return;
    }

    const bool bAlreadyInserted(
            mpListItemsList->find( &rNodeNum ) != mpListItemsList->end() );
    OSL_ENSURE( !bAlreadyInserted,
            "<SwDoc::InsertListItem(..)> - <SwNodeNum> instance already registered as numbered item!" );
    if ( !bAlreadyInserted )
    {
        mpListItemsList->insert( &rNodeNum );
    }
}

void SwDoc::removeListItem( const SwNodeNum& rNodeNum )
{
    if ( mpListItemsList == 0 )
    {
        return;
    }

    const tImplSortedNodeNumList::size_type nDeleted = mpListItemsList->erase( &rNodeNum );
    if ( nDeleted > 1 )
    {
        OSL_FAIL( "<SwDoc::RemoveListItem(..)> - <SwNodeNum> was registered more than once as numbered item!" );
    }
}

String SwDoc::getListItemText( const SwNodeNum& rNodeNum,
                               const bool bWithNumber,
                               const bool bWithSpacesForLevel ) const
{
    return rNodeNum.GetTxtNode()
           ? rNodeNum.GetTxtNode()->GetExpandTxt( 0, STRING_LEN, bWithNumber,
                                                  bWithNumber, bWithSpacesForLevel )
           : String();
}

void SwDoc::getListItems( tSortedNodeNumList& orNodeNumList ) const
{
    orNodeNumList.clear();
    orNodeNumList.reserve( mpListItemsList->size() );

    tImplSortedNodeNumList::iterator aIter;
    tImplSortedNodeNumList::iterator aEndIter = mpListItemsList->end();
    for ( aIter = mpListItemsList->begin(); aIter != aEndIter; ++aIter )
    {
        orNodeNumList.push_back( (*aIter) );
    }
}

void SwDoc::getNumItems( tSortedNodeNumList& orNodeNumList ) const
{
    orNodeNumList.clear();
    orNodeNumList.reserve( mpListItemsList->size() );

    tImplSortedNodeNumList::iterator aIter;
    tImplSortedNodeNumList::iterator aEndIter = mpListItemsList->end();
    for ( aIter = mpListItemsList->begin(); aIter != aEndIter; ++aIter )
    {
        const SwNodeNum* pNodeNum = (*aIter);
        if ( pNodeNum->IsCounted() &&
             pNodeNum->GetTxtNode() && pNodeNum->GetTxtNode()->HasNumber() )
        {
            orNodeNumList.push_back( pNodeNum );
        }
    }
}

// implementation for interface <IDocumentOutlineNodes>
sal_Int32 SwDoc::getOutlineNodesCount() const
{
    return GetNodes().GetOutLineNds().size();
}

int SwDoc::getOutlineLevel( const sal_Int32 nIdx ) const
{
    return GetNodes().GetOutLineNds()[ static_cast<sal_uInt16>(nIdx) ]->
                                           // GetTxtNode()->GetOutlineLevel();              //#outline level,zhaojianwei
                                GetTxtNode()->GetAttrOutlineLevel()-1;  //<-end,zhaojianwei
}

String SwDoc::getOutlineText( const sal_Int32 nIdx,
                              const bool bWithNumber,
                              const bool bWithSpacesForLevel ) const
{
    return GetNodes().GetOutLineNds()[ static_cast<sal_uInt16>(nIdx) ]->
                GetTxtNode()->GetExpandTxt( 0, STRING_LEN, bWithNumber,
                                            bWithNumber, bWithSpacesForLevel );
}

SwTxtNode* SwDoc::getOutlineNode( const sal_Int32 nIdx ) const
{
    return GetNodes().GetOutLineNds()[ static_cast<sal_uInt16>(nIdx) ]->GetTxtNode();
}

void SwDoc::getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const
{
    orOutlineNodeList.clear();
    orOutlineNodeList.reserve( getOutlineNodesCount() );

    const sal_uInt16 nOutlCount( static_cast<sal_uInt16>(getOutlineNodesCount()) );
    for ( sal_uInt16 i = 0; i < nOutlCount; ++i )
    {
        orOutlineNodeList.push_back(
            GetNodes().GetOutLineNds()[i]->GetTxtNode() );
    }
}

// implementation of interface IDocumentListsAccess
SwList* SwDoc::createList( String sListId,
                           const String sDefaultListStyleName )
{
    if ( sListId.Len() == 0 )
    {
        sListId = listfunc::CreateUniqueListId( *this );
    }

    if ( getListByName( sListId ) )
    {
        OSL_FAIL( "<SwDoc::createList(..)> - provided list id already used. Serious defect -> please inform OD." );
        return 0;
    }

    SwNumRule* pDefaultNumRuleForNewList = FindNumRulePtr( sDefaultListStyleName );
    if ( !pDefaultNumRuleForNewList )
    {
        OSL_FAIL( "<SwDoc::createList(..)> - for provided default list style name no list style is found. Serious defect -> please inform OD." );
        return 0;
    }

    SwList* pNewList = new SwList( sListId, *pDefaultNumRuleForNewList, GetNodes() );
    maLists[sListId] = pNewList;

    return pNewList;
}

void SwDoc::deleteList( const String sListId )
{
    SwList* pList = getListByName( sListId );
    if ( pList )
    {
        maLists.erase( sListId );
        delete pList;
    }
}

SwList* SwDoc::getListByName( const String sListId ) const
{
    SwList* pList = 0;

    boost::unordered_map< String, SwList*, StringHash >::const_iterator
                                            aListIter = maLists.find( sListId );
    if ( aListIter != maLists.end() )
    {
        pList = (*aListIter).second;
    }

    return pList;
}

SwList* SwDoc::createListForListStyle( const String sListStyleName )
{
    if ( sListStyleName.Len() == 0 )
    {
        OSL_FAIL( "<SwDoc::createListForListStyle(..)> - no list style name provided. Serious defect -> please inform OD." );
        return 0;
    }

    if ( getListForListStyle( sListStyleName ) )
    {
        OSL_FAIL( "<SwDoc::createListForListStyle(..)> - a list for the provided list style name already exists. Serious defect -> please inform OD." );
        return 0;
    }

    SwNumRule* pNumRule = FindNumRulePtr( sListStyleName );
    if ( !pNumRule )
    {
        OSL_FAIL( "<SwDoc::createListForListStyle(..)> - for provided list style name no list style is found. Serious defect -> please inform OD." );
        return 0;
    }

    String sListId( pNumRule->GetDefaultListId() ); // can be empty String
    if ( getListByName( sListId ) )
    {
        sListId = String();
    }
    SwList* pNewList = createList( sListId, sListStyleName );
    maListStyleLists[sListStyleName] = pNewList;
    pNumRule->SetDefaultListId( pNewList->GetListId() );

    return pNewList;
}

SwList* SwDoc::getListForListStyle( const String sListStyleName ) const
{
    SwList* pList = 0;

    boost::unordered_map< String, SwList*, StringHash >::const_iterator
                            aListIter = maListStyleLists.find( sListStyleName );
    if ( aListIter != maListStyleLists.end() )
    {
        pList = (*aListIter).second;
    }

    return pList;
}

void SwDoc::deleteListForListStyle( const String sListStyleName )
{
    String sListId;
    {
        SwList* pList = getListForListStyle( sListStyleName );
        OSL_ENSURE( pList,
                "<SwDoc::deleteListForListStyle(..)> - misusage of method: no list found for given list style name" );
        if ( pList )
        {
            sListId = pList->GetListId();
        }
    }
    if ( sListId.Len() > 0 )
    {
        maListStyleLists.erase( sListStyleName );
        deleteList( sListId );
    }
}

void SwDoc::trackChangeOfListStyleName( const String sListStyleName,
                                        const String sNewListStyleName )
{
    SwList* pList = getListForListStyle( sListStyleName );
    OSL_ENSURE( pList,
            "<SwDoc::changeOfListStyleName(..)> - misusage of method: no list found for given list style name" );

    if ( pList != 0 )
    {
        maListStyleLists.erase( sListStyleName );
        maListStyleLists[sNewListStyleName] = pList;
    }
}

namespace listfunc
{
    const String MakeListIdUnique( const SwDoc& rDoc,
                                   const String aSuggestedUniqueListId )
    {
        long nHitCount = 0;
        String aTmpStr = aSuggestedUniqueListId;
        while ( rDoc.getListByName( aTmpStr ) )
        {
            ++nHitCount;
            aTmpStr = aSuggestedUniqueListId;
            aTmpStr += String::CreateFromInt32( nHitCount );
        }

        return aTmpStr;
    }
    const String CreateUniqueListId( const SwDoc& rDoc )
    {
        // #i92478#
        String aNewListId = rtl::OUString("list");
        sal_Int64 n = Time( Time::SYSTEM ).GetTime();
        n += Date( Date::SYSTEM ).GetDate();
        n += rand();
        // #i92478#
        aNewListId += rtl::OUString::valueOf(n);

        return MakeListIdUnique( rDoc, aNewListId );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
