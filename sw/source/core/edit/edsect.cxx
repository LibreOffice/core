/*************************************************************************
 *
 *  $RCSfile: edsect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-02 14:35:59 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif

#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>      // SwSectionFrm
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>       // SwCntntFrm
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>       // SwTabFrm
#endif


    // SS fuer Bereiche
const SwSection* SwEditShell::InsertSection( const SwSection& rNew,
                                             const SfxItemSet* pAttr )
{
    const SwSection* pRet = 0;
    if( !IsTableMode() )
    {
        StartAllAction();
        GetDoc()->StartUndo( UNDO_INSSECTION );

        FOREACHPAM_START(this)
            const SwSection* pNew = GetDoc()->Insert( *PCURCRSR,
                                                        rNew, pAttr );
            if( !pRet )
                pRet = pNew;
        FOREACHPAM_END()

        // Undo-Klammerung hier beenden
        GetDoc()->EndUndo( UNDO_INSSECTION );
        EndAllAction();
    }
    return pRet;
}


BOOL SwEditShell::IsInsRegionAvailable() const
{
    SwPaM* pCrsr;
    if( IsTableMode() || ( pCrsr = GetCrsr() )->GetNext() != pCrsr )
        return FALSE;
    if( pCrsr->HasMark() )
        return 0 != GetDoc()->IsInsRegionAvailable( *pCrsr );

    return TRUE;
}


const SwSection* SwEditShell::GetCurrSection() const
{
    if( IsTableMode() )
        return 0;

    return GetDoc()->GetCurrSection( *GetCrsr()->GetPoint() );
}

/*-----------------17.03.99 11:53-------------------
 * SwEditShell::GetAnySection liefert den fuer Spalten
 * zustaendigen Bereich, bei Fussnoten kann es nicht der
 * Bereich innerhalb der Fussnote sein.
 * --------------------------------------------------*/

const SwSection* SwEditShell::GetAnySection( BOOL bOutOfTab ) const
{
    SwFrm* pFrm = GetCurrFrm();
    if( bOutOfTab && pFrm )
        pFrm = pFrm->FindTabFrm();
    if( pFrm && pFrm->IsInSct() )
    {
        SwSectionFrm* pSect = pFrm->FindSctFrm();
        ASSERT( pSect, "GetAnySection: Where's my Sect?" );
        if( pSect->IsInFtn() && pSect->GetUpper()->IsInSct() )
        {
            pSect = pSect->GetUpper()->FindSctFrm();
            ASSERT( pSect, "GetAnySection: Where's my SectFrm?" );
        }
        return pSect->GetSection();
    }
    return NULL;
}

USHORT SwEditShell::GetSectionFmtCount() const
{
    return GetDoc()->GetSections().Count();
}


BOOL SwEditShell::IsAnySectionInDoc( BOOL bChkReadOnly, BOOL bChkHidden, BOOL bChkTOX ) const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    USHORT nCnt = rFmts.Count();
    for( USHORT n = 0; n < nCnt; ++n )
    {
        SectionType eTmpType;
        const SwSectionFmt* pFmt = rFmts[ n ];
        if( pFmt->IsInNodesArr() &&
            (bChkTOX  ||
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType ))
        {
            const SwSection& rSect = *rFmts[ n ]->GetSection();
            if( (!bChkReadOnly && !bChkHidden ) ||
                (bChkReadOnly && rSect.IsProtectFlag() ) ||
                (bChkHidden && rSect.IsHiddenFlag() ) )
                break;
        }
    }
    return n != nCnt;
}

USHORT SwEditShell::GetSectionFmtPos( const SwSectionFmt& rFmt ) const
{
    SwSectionFmt* pFmt = (SwSectionFmt*)&rFmt;
    return GetDoc()->GetSections().GetPos( pFmt );
}

const SwSectionFmt& SwEditShell::GetSectionFmt( USHORT nFmt ) const
{
    return *GetDoc()->GetSections()[ nFmt ];
}


void SwEditShell::DelSectionFmt( USHORT nFmt )
{
    StartAllAction();
    GetDoc()->DelSectionFmt( GetDoc()->GetSections()[ nFmt ] );
    // rufe das AttrChangeNotify auf der UI-Seite.
    CallChgLnk();
    EndAllAction();
}


void SwEditShell::ChgSection( USHORT nSect, const SwSection& rSect,
                                const SfxItemSet* pAttr )
{
    StartAllAction();
    GetDoc()->ChgSection( nSect, rSect, pAttr );
    // rufe das AttrChangeNotify auf der UI-Seite.
    CallChgLnk();
    EndAllAction();
}

const ::com::sun::star::uno::Sequence <sal_Int8>&
    SwEditShell::GetSectionPasswd() const
{
    return GetDoc()->GetSectionPasswd();
}


void SwEditShell::ChgSectionPasswd(
        const ::com::sun::star::uno::Sequence <sal_Int8>& rNew,
        const SwSection* pSection )
{
    GetDoc()->ChgSectionPasswd( rNew, pSection );
}


String SwEditShell::GetUniqueSectionName( const String* pChkStr ) const
{
    return GetDoc()->GetUniqueSectionName( pChkStr );
}

void SwEditShell::SetSectionAttr( const SfxItemSet& rSet,
                                    SwSectionFmt* pSectFmt )
{
    if( pSectFmt )
        _SetSectionAttr( *pSectFmt, rSet );
    else
    {
        // for all section in the selection

        FOREACHPAM_START(this)

            const SwPosition* pStt = PCURCRSR->Start(),
                            * pEnd = PCURCRSR->End();

            const SwSectionNode* pSttSectNd = pStt->nNode.GetNode().FindSectionNode(),
                               * pEndSectNd = pEnd->nNode.GetNode().FindSectionNode();

            if( pSttSectNd || pEndSectNd )
            {
                if( pSttSectNd )
                    _SetSectionAttr( *pSttSectNd->GetSection().GetFmt(),
                                    rSet );
                if( pEndSectNd && pSttSectNd != pEndSectNd )
                    _SetSectionAttr( *pEndSectNd->GetSection().GetFmt(),
                                    rSet );

                if( pSttSectNd && pEndSectNd )
                {
                    SwNodeIndex aSIdx( pStt->nNode );
                    SwNodeIndex aEIdx( pEnd->nNode );
                    if( pSttSectNd->EndOfSectionIndex() <
                        pEndSectNd->GetIndex() )
                    {
                        aSIdx = pSttSectNd->EndOfSectionIndex() + 1;
                        aEIdx = *pEndSectNd;
                    }

                    while( aSIdx < aEIdx )
                    {
                        if( 0 != (pSttSectNd = aSIdx.GetNode().GetSectionNode())
                            || ( aSIdx.GetNode().IsEndNode() &&
                                0 != ( pSttSectNd = aSIdx.GetNode().
                                    FindStartNode()->GetSectionNode())) )
                            _SetSectionAttr( *pSttSectNd->GetSection().GetFmt(),
                                            rSet );
                        aSIdx++;
                    }
                }
            }

        FOREACHPAM_END()
    }
}

void SwEditShell::_SetSectionAttr( SwSectionFmt& rSectFmt,
                                    const SfxItemSet& rSet )
{
    StartAllAction();
    if(SFX_ITEM_SET == rSet.GetItemState(RES_CNTNT, FALSE))
    {
        SfxItemSet aSet(rSet);
        aSet.ClearItem(RES_CNTNT);
        GetDoc()->SetAttr( aSet, rSectFmt );
    }
    else
        GetDoc()->SetAttr( rSet, rSectFmt );

    // rufe das AttrChangeNotify auf der UI-Seite.
    CallChgLnk();
    EndAllAction();
}

// search inside the cursor selection for full selected sections.
// if any part of section in the selection return 0.
// if more than one in the selection return the count
USHORT SwEditShell::GetFullSelectedSectionCount() const
{
    USHORT nRet = 0;
    FOREACHPAM_START(this)

        const SwPosition* pStt = PCURCRSR->Start(),
                        * pEnd = PCURCRSR->End();
        const SwCntntNode* pCNd;
        // check the selection, if Start at Node begin and End at Node end
        if( pStt->nContent.GetIndex() ||
            ( 0 == ( pCNd = pEnd->nNode.GetNode().GetCntntNode() )) ||
            pCNd->Len() != pEnd->nContent.GetIndex() )
        {
            nRet = 0;
            break;
        }

// !!!!!!!!!!!!!!!!!!!!!!!!!!
// what about table at start or end ?
//      There is no selection possible!
// What about only a table inside the section ?
//      There is only a table selection possible!

        SwNodeIndex aSIdx( pStt->nNode, -1 ), aEIdx( pEnd->nNode, +1 );
        if( !aSIdx.GetNode().IsSectionNode() ||
            !aEIdx.GetNode().IsEndNode() ||
            !aEIdx.GetNode().FindStartNode()->IsSectionNode() )
        {
            nRet = 0;
            break;
        }

        ++nRet;
        if( &aSIdx.GetNode() != aEIdx.GetNode().FindStartNode() )
            ++nRet;

    FOREACHPAM_END()
    return nRet;
}

// is the cursor at the last contentposition of any section and is the
// insertposition not readonly?
// The return have 3 values:
//  0 - not at start or end of Section
//  1 - at start of Section
//  2 - at End of Section
int SwEditShell::CanInsertNodeAtEndOfSection() const
{
    int nRet = 0;
    if( !IsTableMode() )
    {
        const SwPosition& rPos = *GetCrsr()->GetPoint();
        const SwSectionNode* pSectNd = rPos.nNode.GetNode().FindSectionNode();
        if( pSectNd )
        {
            BOOL bEnd = FALSE, bStart = FALSE;
            const SwCntntNode* pCntNd = rPos.nNode.GetNode().GetCntntNode();

            SwNodeIndex aEnd( rPos.nNode, 1 );
            while( aEnd.GetNode().IsEndNode() &&
                    (const SwNode*)&aEnd.GetNode() !=
                    pSectNd->EndOfSectionNode() )
                aEnd++;

            if( aEnd.GetNode().IsEndNode() &&
                ( !pCntNd || pCntNd->Len() == rPos.nContent.GetIndex() ))
                bEnd = TRUE;
            else
            {
                aEnd = rPos.nNode;
                aEnd--;
                while( aEnd.GetNode().IsStartNode() &&
                        (const SwNode*)&aEnd.GetNode() != pSectNd )
                    aEnd--;

                if( (const SwNode*)&aEnd.GetNode() == pSectNd &&
                    ( !pCntNd || !rPos.nContent.GetIndex() ))
                    bStart = TRUE;
            }

            if( bEnd || bStart )
            {
                // is the insertposition readonly?
                if( bEnd )
                {
                    aEnd = *pSectNd->EndOfSectionNode();
                    aEnd++;
                }
                else
                {
                    aEnd = *pSectNd;
                    aEnd--;
                    // the IsProtect-Method of SwNode test by sectionnode
                    // his parent!
                    if( aEnd.GetNode().IsSectionNode() )
                        aEnd = *pSectNd;
                }

                do {
                    if( !aEnd.GetNode().IsProtect() )
                    {
                        nRet = bStart ? 1 : 2;
                        break;
                    }
                    else
                    {
                        // skip protected sections without any content at
                        // start or end
                        if( bStart )
                        {
                            if( !aEnd.GetNode().IsSectionNode() ||
                                !aEnd.GetNode().StartOfSectionIndex() )
                                break;
                            aEnd--;
                        }
                        else
                        {
                            if( !aEnd.GetNode().IsEndNode() ||
                                !aEnd.GetNode().StartOfSectionNode()->
                                            IsSectionNode() )
                                break;
                               aEnd++;
                        }
                    }
                } while( TRUE );
            }
        }
    }
    return nRet;
}

BOOL SwEditShell::AppendNodeInSection()
{
    int nRet = CanInsertNodeAtEndOfSection();
    if( nRet )
    {
        StartAllAction();

        SwPosition& rPos = *GetCrsr()->GetPoint();
        const SwSectionNode* pSectNd = rPos.nNode.GetNode().FindSectionNode();
        SwPosition aPos( *pSectNd );
        if( 1 == nRet )
        {
            do {
                const SwNode* pPrvNd = &aPos.nNode.GetNode();
                aPos.nNode--;
                if( !aPos.nNode.GetNode().IsSectionNode() ||
                    !pPrvNd->IsProtect() )
                    break;
            } while( TRUE );
        }
        else
        {
            SwNodeIndex aIdx( *pSectNd->EndOfSectionNode(), 1 );
            do {
                if( !aIdx.GetNode().IsEndNode() ||
                    !aIdx.GetNode().FindStartNode()->IsSectionNode() ||
                    !aIdx.GetNode().IsProtect() )
                    break;
                aIdx++;
            } while( TRUE );
            aIdx--;
            aPos.nNode = aIdx;
        }

        GetDoc()->AppendTxtNode( aPos );
        rPos = aPos;

        // rufe das AttrChangeNotify auf der UI-Seite.
        CallChgLnk();
        EndAllAction();
    }
    return 0 != nRet;
}



