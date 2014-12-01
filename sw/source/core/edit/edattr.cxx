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

#include <hintids.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <txatbase.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <doc.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <ftnidx.hxx>
#include <expfld.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <breakit.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <crsskip.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>
#include <svl/ctloptions.hxx>
#include <svl/itemiter.hxx>
#include <charfmt.hxx>
#include <numrule.hxx>

#include <algorithm>
#include <charatr.hxx>

/*
 * hard Formatting (Attributes)
 */

// if selection is bigger as max nodes or more than max selections
// => no attributes
const sal_uInt16& getMaxLookup()
{
    static const sal_uInt16 nMaxLookup = 1000;
    return nMaxLookup;
}

bool SwEditShell::GetPaMAttr( SwPaM* pPaM, SfxItemSet& rSet,
                              const bool bMergeIndentValuesOfNumRule ) const
{
    // ??? pPaM can be different from the Cursor ???
    if( GetCrsrCnt() > getMaxLookup() )
    {
        rSet.InvalidateAllItems();
        return false;
    }

    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;

    SwPaM* pStartPaM = pPaM;
    do {
        // #i27615# if the cursor is in front of the numbering label
        // the attributes to get are those from the numbering format.
        if (pPaM->IsInFrontOfLabel())
        {
            SwTxtNode * pTxtNd = pPaM->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNd)
            {
                SwNumRule * pNumRule = pTxtNd->GetNumRule();

                if (pNumRule)
                {
                    int nListLevel = pTxtNd->GetActualListLevel();

                    if (nListLevel < 0)
                        nListLevel = 0;

                    if (nListLevel >= MAXLEVEL)
                        nListLevel = MAXLEVEL - 1;

                    const OUString & aCharFmtName =
                        pNumRule->Get(static_cast<sal_uInt16>(nListLevel)).GetCharFmtName();
                    SwCharFmt * pCharFmt =
                        GetDoc()->FindCharFmtByName(aCharFmtName);

                    if (pCharFmt)
                        rSet.Put(pCharFmt->GetAttrSet());
                }
            }

            continue;
        }

        sal_uLong nSttNd = pPaM->GetMark()->nNode.GetIndex(),
              nEndNd = pPaM->GetPoint()->nNode.GetIndex();
        sal_Int32 nSttCnt = pPaM->GetMark()->nContent.GetIndex();
        sal_Int32 nEndCnt = pPaM->GetPoint()->nContent.GetIndex();

        if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
        {
            std::swap(nSttNd, nEndNd);
            std::swap(nSttCnt, nEndCnt);
        }

        if( nEndNd - nSttNd >= getMaxLookup() )
        {
            rSet.ClearItem();
            rSet.InvalidateAllItems();
            return false;
        }

        // at first node the node enter his values into the GetSet (Initial)
        // all additional nodes are additional merged to GetSet
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            SwNode* pNd = GetDoc()->GetNodes()[ n ];
            switch( pNd->GetNodeType() )
            {
            case ND_TEXTNODE:
                {
                    const sal_Int32 nStt = (n == nSttNd) ? nSttCnt : 0;
                    const sal_Int32 nEnd = (n == nEndNd)
                        ? nEndCnt
                        : pNd->GetTxtNode()->GetTxt().getLength();

                    static_cast<SwTxtNode*>(pNd)->GetAttr( *pSet, nStt, nEnd,
                                                false, true,
                                                bMergeIndentValuesOfNumRule );
                }
                break;
            case ND_GRFNODE:
            case ND_OLENODE:
                static_cast<SwCntntNode*>(pNd)->GetAttr( *pSet );
                break;

            default:
                pNd = 0;
            }

            if( pNd )
            {
                if( pSet != &rSet )
                    rSet.MergeValues( aSet );

                if( aSet.Count() )
                    aSet.ClearItem();
            }
            pSet = &aSet;
        }

    } while ( ( pPaM = static_cast<SwPaM*>(pPaM->GetNext()) ) != pStartPaM );

    return true;
}

bool SwEditShell::GetCurAttr( SfxItemSet& rSet,
                              const bool bMergeIndentValuesOfNumRule ) const
{
    return GetPaMAttr( GetCrsr(), rSet, bMergeIndentValuesOfNumRule );
}

bool SwEditShell::GetCurParAttr( SfxItemSet& rSet) const
{
    return GetPaMParAttr( GetCrsr(), rSet );
}

bool SwEditShell::GetPaMParAttr( SwPaM* pPaM, SfxItemSet& rSet ) const
{
    // number of nodes the function has explored so far
    sal_uInt16 numberOfLookup = 0;

    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet* pSet = &rSet;

    SwPaM* pStartPaM = pPaM;
    do { // for all the point and mark (selections)

        // get the start and the end node of the current selection
        sal_uLong nSttNd = pPaM->GetMark()->nNode.GetIndex(),
              nEndNd = pPaM->GetPoint()->nNode.GetIndex();

        // reverse start and end if there number aren't sorted correctly
        if( nSttNd > nEndNd )
            std::swap(nSttNd, nEndNd);

        // for all the nodes in the current selection
        // get the node (paragraph) attributes
        // and merge them in rSet
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            // get the node
            SwNode* pNd = GetDoc()->GetNodes()[ n ];

            if( pNd->IsTxtNode() )
            {
                // get the node (paragraph) attributes
                static_cast<SwCntntNode*>(pNd)->GetAttr(*pSet);

                if( pSet != &rSet && aSet.Count() )
                {
                    rSet.MergeValues( aSet );
                    aSet.ClearItem();
                }

                pSet = &aSet;
            }

            ++numberOfLookup;

            // if the maximum number of node that can be inspected has been reached
            if (numberOfLookup >= getMaxLookup())
                return false;
        }
    } while ( ( pPaM = static_cast<SwPaM*>(pPaM->GetNext()) ) != pStartPaM );

    return true;
}

SwTxtFmtColl* SwEditShell::GetCurTxtFmtColl( ) const
{
    return GetPaMTxtFmtColl( GetCrsr() );
}

SwTxtFmtColl* SwEditShell::GetPaMTxtFmtColl( SwPaM* pPaM ) const
{
    // number of nodes the function have explored so far
    sal_uInt16 numberOfLookup = 0;

    SwPaM* pStartPaM = pPaM;
    do { // for all the point and mark (selections)

        // get the start and the end node of the current selection
        sal_uLong nSttNd = pPaM->GetMark()->nNode.GetIndex(),
              nEndNd = pPaM->GetPoint()->nNode.GetIndex();

        // reverse start and end if they aren't sorted correctly
        if( nSttNd > nEndNd )
            std::swap(nSttNd, nEndNd);

        // for all the nodes in the current Point and Mark
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            // get the node
            SwNode* pNd = GetDoc()->GetNodes()[ n ];

            ++numberOfLookup;

            // if the maximum number of node that can be inspected has been reached
            if (numberOfLookup >= getMaxLookup())
                return NULL;

            if( pNd->IsTxtNode() )
            {
                // if it's a text node get its named paragraph format
                SwTxtFmtColl* pFmt = pNd->GetTxtNode()->GetTxtColl();

                // if the paragraph format exist stop here and return it
                if( pFmt != NULL )
                    return pFmt;
            }
        }
    } while ( ( pPaM = static_cast<SwPaM*>(pPaM->GetNext()) ) != pStartPaM );

    // if none of the selected node contain a named paragraph format
    return NULL;
}

std::vector<std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM> >> SwEditShell::GetItemWithPaM( sal_uInt16 nWhich )
{
    std::vector<std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM> >> vItem;
    SwPaM* pPaM = GetCrsr();
    SwPaM* pStartPaM = pPaM;
    do { // for all the point and mark (selections)

        // get the start and the end node of the current selection
        sal_uLong nSttNd = pPaM->Start()->nNode.GetIndex(),
              nEndNd = pPaM->End()->nNode.GetIndex();
        sal_Int32 nSttCnt = pPaM->Start()->nContent.GetIndex();
        sal_Int32 nEndCnt = pPaM->End()->nContent.GetIndex();

        SwPaM* pNewPaM = 0;
        const SfxPoolItem* pItem = 0;

        // for all the nodes in the current selection
        for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
        {
            SwNode* pNd = GetDoc()->GetNodes()[ n ];
            if( pNd->IsTxtNode() )
            {
                SwTxtNode* pTxtNd = static_cast< SwTxtNode* >( pNd );
                const sal_Int32 nStt = (n == nSttNd) ? nSttCnt : 0;
                const sal_Int32 nEnd = (n == nEndNd)
                    ? nEndCnt : pTxtNd->GetTxt().getLength();
                const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTxtNd );
                sal_uInt8 nScript = pScriptInfo ? pScriptInfo->ScriptType( nStt ) : css::i18n::ScriptType::WEAK;
                nWhich = GetWhichOfScript( nWhich, nScript );

                // item from attribute set
                if( pTxtNd->HasSwAttrSet() )
                {
                    pNewPaM = new SwPaM(*pNd, nStt, *pNd, nEnd);
                    pItem = pTxtNd->GetSwAttrSet().GetItem( nWhich );
                    vItem.push_back( std::make_pair( pItem, std::unique_ptr<SwPaM>(pNewPaM) ) );
                }

                if( !pTxtNd->HasHints() )
                    continue;

                // items with limited range
                const size_t nSize = pTxtNd->GetpSwpHints()->Count();
                for( size_t m = 0; m < nSize; m++ )
                {
                    const SwTxtAttr* pHt = (*pTxtNd->GetpSwpHints())[m];
                    if( pHt->Which() == RES_TXTATR_AUTOFMT ||
                        pHt->Which() == RES_TXTATR_CHARFMT ||
                        pHt->Which() == RES_TXTATR_INETFMT )
                    {
                        const sal_Int32 nAttrStart = pHt->GetStart();
                        const sal_Int32* pAttrEnd = pHt->End();

                        // Ignore items not in selection
                        if( nAttrStart > nEnd )
                            break;
                        if( *pAttrEnd <= nStt )
                            continue;

                        nScript = pScriptInfo ? pScriptInfo->ScriptType( nStt ) : css::i18n::ScriptType::WEAK;
                        nWhich = GetWhichOfScript( nWhich, nScript );
                        const SfxItemSet* pAutoSet = CharFmt::GetItemSet( pHt->GetAttr() );
                        if( pAutoSet )
                        {
                            SfxItemIter aItemIter( *pAutoSet );
                            pItem = aItemIter.GetCurItem();
                            while( pItem )
                            {
                                if( pItem->Which() == nWhich )
                                {
                                    sal_Int32 nStart = 0, nStop = 0;
                                    if( nAttrStart < nStt ) //Attribut starts before selection
                                        nStart = nStt;
                                    else
                                        nStart = nAttrStart;
                                    if( *pAttrEnd > nEnd ) //Attribut ends after selection
                                        nStop = nEnd;
                                    else
                                        nStop = *pAttrEnd;
                                    pNewPaM = new SwPaM(*pNd, nStart, *pNd, nStop);
                                    vItem.push_back( std::make_pair( pItem, std::unique_ptr<SwPaM>(pNewPaM) ) );
                                    break;
                                }
                                pItem = aItemIter.NextItem();
                            }
                            // default item
                            if( !pItem && !pTxtNd->HasSwAttrSet() )
                            {
                                pNewPaM = new SwPaM(*pNd, nStt, *pNd, nEnd);
                                pItem = pAutoSet->GetPool()->GetPoolDefaultItem( nWhich );
                                vItem.push_back( std::make_pair( pItem,  std::unique_ptr<SwPaM>(pNewPaM)) );
                            }
                        }
                    }
                }
            }
        }
    } while ( ( pPaM = static_cast<SwPaM*>(pPaM->GetNext()) ) != pStartPaM );
    return vItem;
}

bool SwEditShell::GetCurFtn( SwFmtFtn* pFillFtn )
{
    // The cursor must be positioned on the current footnotes anchor:
    SwPaM* pCrsr = GetCrsr();
    SwTxtNode* pTxtNd = pCrsr->GetNode().GetTxtNode();
    if( !pTxtNd )
        return false;

    SwTxtAttr *const pFtn = pTxtNd->GetTxtAttrForCharAt(
        pCrsr->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    if( pFtn && pFillFtn )
    {
        // Transfer data from the attribute
        const SwFmtFtn &rFtn = static_cast<SwTxtFtn*>(pFtn)->GetFtn();
        pFillFtn->SetNumber( rFtn );
        pFillFtn->SetEndNote( rFtn.IsEndNote() );
    }
    return 0 != pFtn;
}

bool SwEditShell::SetCurFtn( const SwFmtFtn& rFillFtn )
{
    bool bChgd = false;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr(), *pFirst = pCrsr;
    do {
        bChgd |=
            mpDoc->SetCurFtn( *pCrsr, rFillFtn.GetNumStr(), rFillFtn.GetNumber(), rFillFtn.IsEndNote() );

    } while( pFirst != ( pCrsr = static_cast<SwPaM*>(pCrsr->GetNext()) ));

    EndAllAction();
    return bChgd;
}

bool SwEditShell::HasFtns( bool bEndNotes ) const
{
    const SwFtnIdxs &rIdxs = mpDoc->GetFtnIdxs();
    for ( sal_uInt16 i = 0; i < rIdxs.size(); ++i )
    {
        const SwFmtFtn &rFtn = rIdxs[i]->GetFtn();
        if ( bEndNotes == rFtn.IsEndNote() )
            return true;
    }
    return false;
}

/// Give a List of all footnotes and their beginning texts
size_t SwEditShell::GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes )
{
    rList.Clear();

    const size_t nFtnCnt = mpDoc->GetFtnIdxs().size();
    SwTxtFtn* pTxtFtn;
    for( size_t n = 0; n < nFtnCnt; ++n )
    {
        pTxtFtn = mpDoc->GetFtnIdxs()[ n ];
        const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
        if ( rFtn.IsEndNote() != bEndNotes )
            continue;

        SwNodeIndex* pIdx = pTxtFtn->GetStartNode();
        if( pIdx )
        {
            SwNodeIndex aIdx( *pIdx, 1 );
            SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
            if( !pTxtNd )
                pTxtNd = static_cast<SwTxtNode*>(mpDoc->GetNodes().GoNext( &aIdx ));

            if( pTxtNd )
            {
                OUString sTxt( rFtn.GetViewNumStr( *mpDoc ));
                if( !sTxt.isEmpty() )
                    sTxt += " ";
                sTxt += pTxtNd->GetExpandTxt( 0, -1 );

                _SeqFldLstElem* pNew = new _SeqFldLstElem( sTxt,
                                            pTxtFtn->GetSeqRefNo() );
                while( rList.InsertSort( pNew ) )
                    pNew->sDlgEntry += " ";
            }
        }
    }

    return rList.Count();
}

/// Adjust left margin via object bar (similar to adjustment of numerations).
bool SwEditShell::IsMoveLeftMargin( bool bRight, bool bModulus ) const
{
    bool bRet = true;

    const SvxTabStopItem& rTabItem = static_cast<const SvxTabStopItem&>(GetDoc()->
                                GetDefault( RES_PARATR_TABSTOP ));
    sal_uInt16 nDefDist = static_cast<sal_uInt16>(rTabItem.Count() ? rTabItem[0].GetTabPos() : 1134);
    if( !nDefDist )
        return false;

    FOREACHPAM_START(GetCrsr())

        sal_uLong nSttNd = PCURCRSR->GetMark()->nNode.GetIndex(),
              nEndNd = PCURCRSR->GetPoint()->nNode.GetIndex();

        if( nSttNd > nEndNd )
            std::swap(nSttNd, nEndNd);

        SwCntntNode* pCNd;
        for( sal_uLong n = nSttNd; bRet && n <= nEndNd; ++n )
            if( 0 != ( pCNd = GetDoc()->GetNodes()[ n ]->GetTxtNode() ))
            {
                const SvxLRSpaceItem& rLS = static_cast<const SvxLRSpaceItem&>(
                                            pCNd->GetAttr( RES_LR_SPACE ));
                if( bRight )
                {
                    long nNext = rLS.GetTxtLeft() + nDefDist;
                    if( bModulus )
                        nNext = ( nNext / nDefDist ) * nDefDist;
                    SwFrm* pFrm = pCNd->getLayoutFrm( GetLayout() );
                    if ( pFrm )
                    {
                        const sal_uInt16 nFrmWidth = static_cast<sal_uInt16>( pFrm->IsVertical() ?
                                                 pFrm->Frm().Height() :
                                                 pFrm->Frm().Width() );
                        bRet = nFrmWidth > ( nNext + MM50 );
                    }
                    else
                        bRet = false;
                }
            }

        if( !bRet )
            break;

    FOREACHPAM_END()
    return bRet;
}

void SwEditShell::MoveLeftMargin( bool bRight, bool bModulus )
{
    StartAllAction();
    StartUndo( UNDO_START );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Multiple selection ?
    {
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->MoveLeftMargin( aRangeArr.SetPam( n, aPam ),
                                        bRight, bModulus );
    }
    else
        GetDoc()->MoveLeftMargin( *pCrsr, bRight, bModulus );

    EndUndo( UNDO_END );
    EndAllAction();
}

static inline sal_uInt16 lcl_SetScriptFlags( sal_uInt16 nType )
{
    sal_uInt16 nRet;
       switch( nType )
    {
    case ::com::sun::star::i18n::ScriptType::LATIN:     nRet = SCRIPTTYPE_LATIN;    break;
    case ::com::sun::star::i18n::ScriptType::ASIAN:     nRet = SCRIPTTYPE_ASIAN;    break;
    case ::com::sun::star::i18n::ScriptType::COMPLEX:   nRet = SCRIPTTYPE_COMPLEX;  break;
    default: nRet = 0;
    }
    return nRet;
}

static bool lcl_IsNoEndTxtAttrAtPos( const SwTxtNode& rTNd, sal_Int32 nPos,
                            sal_uInt16 &rScrpt, bool bInSelection, bool bNum )
{
    bool bRet = false;
    OUString sExp;

    // consider numbering
    if ( bNum )
    {
        bRet = false;

        if ( rTNd.IsInList() )
        {
            OSL_ENSURE( rTNd.GetNumRule(),
                    "<lcl_IsNoEndTxtAttrAtPos(..)> - no list style found at text node. Serious defect -> please inform OD." );
            const SwNumRule* pNumRule = rTNd.GetNumRule();
            if(pNumRule)
            {
                int nListLevel = rTNd.GetActualListLevel();

                if (nListLevel < 0)
                    nListLevel = 0;

                if (nListLevel >= MAXLEVEL)
                    nListLevel = MAXLEVEL - 1;

                const SwNumFmt &rNumFmt = pNumRule->Get( static_cast<sal_uInt16>(nListLevel) );
                if( SVX_NUM_BITMAP != rNumFmt.GetNumberingType() )
                {
                    if ( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
                        sExp = OUString(rNumFmt.GetBulletChar());
                    else
                        sExp = rTNd.GetNumString();
                }
            }
        }
    }

    // and fields
    if (nPos < rTNd.GetTxt().getLength() && CH_TXTATR_BREAKWORD == rTNd.GetTxt()[nPos])
    {
        const SwTxtAttr* const pAttr = rTNd.GetTxtAttrForCharAt( nPos );
        if (pAttr)
        {
            bRet = true; // all other than fields can be
                         // defined as weak-script ?
            if ( RES_TXTATR_FIELD == pAttr->Which() )
            {
                const SwField* const pFld = pAttr->GetFmtFld().GetField();
                if (pFld)
                {
                    sExp += pFld->ExpandField(true);
                }
            }
        }
    }

    const sal_Int32 nEnd = sExp.getLength();
    if ( nEnd )
    {
        if( bInSelection )
        {
            sal_uInt16 nScript;
            for( sal_Int32 n = 0; n < nEnd;
                 n = g_pBreakIt->GetBreakIter()->endOfScript( sExp, n, nScript ))
            {
                nScript = g_pBreakIt->GetBreakIter()->getScriptType( sExp, n );
                rScrpt |= lcl_SetScriptFlags( nScript );
            }
        }
        else
            rScrpt |= lcl_SetScriptFlags( g_pBreakIt->GetBreakIter()->
                                        getScriptType( sExp, nEnd-1 ));
    }

    return bRet;
}

/// returns the script type of the selection
sal_uInt16 SwEditShell::GetScriptType() const
{
    sal_uInt16 nRet = 0;

    {
        FOREACHPAM_START(GetCrsr())

            const SwPosition *pStt = PCURCRSR->Start(),
                             *pEnd = pStt == PCURCRSR->GetMark()
                                    ? PCURCRSR->GetPoint()
                                    : PCURCRSR->GetMark();
            if( pStt == pEnd || *pStt == *pEnd )
            {
                const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
                if( pTNd )
                {
                    // try to get SwScriptInfo
                    const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTNd );

                    sal_Int32 nPos = pStt->nContent.GetIndex();
                    //Task 90448: we need the scripttype of the previous
                    //              position, if no selection exist!
                    if( nPos )
                    {
                        SwIndex aIdx( pStt->nContent );
                        if( pTNd->GoPrevious( &aIdx, CRSR_SKIP_CHARS ) )
                            nPos = aIdx.GetIndex();
                    }

                    sal_uInt16 nScript;

                    if (!pTNd->GetTxt().isEmpty())
                    {
                        nScript = pScriptInfo ?
                                  pScriptInfo->ScriptType( nPos ) :
                                  g_pBreakIt->GetBreakIter()->getScriptType( pTNd->GetTxt(), nPos );
                    }
                    else
                        nScript = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );

                    if( !lcl_IsNoEndTxtAttrAtPos( *pTNd, nPos, nRet, false, false ))
                        nRet |= lcl_SetScriptFlags( nScript );
                }
            }
            else if ( g_pBreakIt->GetBreakIter().is() )
            {
                sal_uLong nEndIdx = pEnd->nNode.GetIndex();
                SwNodeIndex aIdx( pStt->nNode );
                for( ; aIdx.GetIndex() <= nEndIdx; ++aIdx )
                    if( aIdx.GetNode().IsTxtNode() )
                    {
                        const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
                        const OUString& rTxt = pTNd->GetTxt();

                        // try to get SwScriptInfo
                        const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTNd );

                        sal_Int32 nChg = aIdx == pStt->nNode
                                                ? pStt->nContent.GetIndex()
                                                : 0;
                        sal_Int32 nEndPos = aIdx == nEndIdx
                                                ? pEnd->nContent.GetIndex()
                                                : rTxt.getLength();

                        OSL_ENSURE( nEndPos <= rTxt.getLength(),
                                "Index outside the range - endless loop!" );
                        if (nEndPos > rTxt.getLength())
                            nEndPos = rTxt.getLength();

                        sal_uInt16 nScript;
                        while( nChg < nEndPos )
                        {
                            nScript = pScriptInfo ?
                                      pScriptInfo->ScriptType( nChg ) :
                                      g_pBreakIt->GetBreakIter()->getScriptType(
                                                                rTxt, nChg );

                            if( !lcl_IsNoEndTxtAttrAtPos( *pTNd, nChg, nRet, true,
                                      0 == nChg && rTxt.getLength() == nEndPos))
                                nRet |= lcl_SetScriptFlags( nScript );

                            if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                                break;

                            sal_Int32 nFldPos = nChg+1;

                            nChg = pScriptInfo ?
                                   pScriptInfo->NextScriptChg( nChg ) :
                                   g_pBreakIt->GetBreakIter()->endOfScript(
                                                    rTxt, nChg, nScript );

                            nFldPos = rTxt.indexOf(
                                            CH_TXTATR_BREAKWORD, nFldPos);
                            if ((-1 != nFldPos) && (nFldPos < nChg))
                                nChg = nFldPos;
                        }
                        if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                            break;
                    }
            }
            if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                break;

        FOREACHPAM_END()
    }
    if( !nRet )
        nRet = SvtLanguageOptions::GetScriptTypeOfLanguage( LANGUAGE_SYSTEM );
    return nRet;
}

sal_uInt16 SwEditShell::GetCurLang() const
{
    const SwPaM* pCrsr = GetCrsr();
    const SwPosition& rPos = *pCrsr->GetPoint();
    const SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
    sal_uInt16 nLang;
    if( pTNd )
    {
        //JP 24.9.2001: if exist no selection, then get the language before
        //              the current character!
        sal_Int32 nPos = rPos.nContent.GetIndex();
        if( nPos && !pCrsr->HasMark() )
            --nPos;
        nLang = pTNd->GetLang( nPos );
    }
    else
        nLang = LANGUAGE_DONTKNOW;
    return nLang;
}

sal_uInt16 SwEditShell::GetScalingOfSelectedText() const
{
    const SwPaM* pCrsr = GetCrsr();
    const SwPosition* pStt = pCrsr->Start();
    const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
    OSL_ENSURE( pTNd, "no textnode available" );

    sal_uInt16 nScaleWidth;
    if( pTNd )
    {
        const SwPosition* pEnd = pStt == pCrsr->GetPoint()
                                        ? pCrsr->GetMark()
                                        : pCrsr->GetPoint();
        const sal_Int32 nStt = pStt->nContent.GetIndex();
        const sal_Int32 nEnd = pStt->nNode == pEnd->nNode
            ? pEnd->nContent.GetIndex()
            : pTNd->GetTxt().getLength();
        nScaleWidth = pTNd->GetScalingOfSelectedText( nStt, nEnd );
    }
    else
        nScaleWidth = 100;              // default are no scaling -> 100%
    return nScaleWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
