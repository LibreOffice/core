/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: section.cxx,v $
 * $Revision: 1.31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <stdlib.h>
#include <hintids.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/protitem.hxx>
#include <svx/linkmgr.hxx>
#include <tools/urlobj.hxx>

#include <sfx2/sfxsids.hrc>
#include <sfx2/fcontnr.hxx>
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <errhdl.hxx>
#include <doc.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <frmtool.hxx>
#include <editsh.hxx>
#include <hints.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <ndtxt.hxx>
#include <section.hxx>
#include <swserv.hxx>
#include <shellio.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>
#include <swbaslnk.hxx>
#include <mvsave.hxx>
#include <sectfrm.hxx>
#include <fmtftntx.hxx>
#include <ftnidx.hxx>
#include <doctxm.hxx>
// --> FME 2004-06-22 #114856# edit in readonly sections
#include <fmteiro.hxx>
// <--
#include <swerror.h>

using namespace ::com::sun::star;


SV_IMPL_REF( SwServerObject )

//static const char __FAR_DATA sSectionFmtNm[] = "Section";
#define sSectionFmtNm aEmptyStr

class SwIntrnlSectRefLink : public SwBaseLink
{
    SwSectionFmt& rSectFmt;
public:
    SwIntrnlSectRefLink( SwSectionFmt& rFmt, USHORT nUpdateType, USHORT nFmt )
        : SwBaseLink( nUpdateType, nFmt ),
        rSectFmt( rFmt )
    {}

    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                                const uno::Any & rValue );

    virtual const SwNode* GetAnchor() const;
    virtual BOOL IsInRange( ULONG nSttNd, ULONG nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;

    // --> OD 2007-02-14 #b6521322#
    inline SwSectionNode* GetSectNode()
    {
        const SwNode* pSectNd( const_cast<SwIntrnlSectRefLink*>(this)->GetAnchor() );
        return const_cast<SwSectionNode*>( dynamic_cast<const SwSectionNode*>( pSectNd ) );
    }
    // <--
};


TYPEINIT1(SwSectionFmt,SwFrmFmt );
TYPEINIT1(SwSection,SwClient );

typedef SwSection* SwSectionPtr;

SV_IMPL_PTRARR( SwSections, SwSection*)
SV_IMPL_PTRARR(SwSectionFmts,SwSectionFmt*)



SwSection::SwSection( SectionType eTyp, const String& rName,
                    SwSectionFmt* pFmt )
    : SwClient( pFmt ), sSectionNm( rName ),
    eType( eTyp )
{
    bHidden = FALSE;
    bHiddenFlag = FALSE;
    bProtectFlag = FALSE;
    // --> FME 2004-06-22 #114856# edit in readonly sections
    bEditInReadonlyFlag = FALSE;
    // <--
    bCondHiddenFlag = TRUE;
    bConnectFlag = TRUE;

    SwSectionPtr pParentSect = GetParent();
    if( pParentSect )
    {
        if( pParentSect->IsHiddenFlag() )
            SetHidden( TRUE );

        _SetProtectFlag( pParentSect->IsProtectFlag() );
        // --> FME 2004-06-22 #114856# edit in readonly sections
        _SetEditInReadonlyFlag( pParentSect->IsEditInReadonlyFlag() );
        // <--
    }

    if( pFmt && !bProtectFlag )
        _SetProtectFlag( pFmt->GetProtect().IsCntntProtected() );

    // --> FME 2004-06-22 #114856# edit in readonly sections
    if ( pFmt && !bEditInReadonlyFlag )
        _SetEditInReadonlyFlag( pFmt->GetEditInReadonly().GetValue() );
    // <--
}


SwSection::~SwSection()
{
    SwSectionFmt* pFmt = GetFmt();
    if( !pFmt )
        return;

    SwDoc* pDoc = pFmt->GetDoc();
    if( pDoc->IsInDtor() )
    {
        // dann melden wir noch schnell unser Format um ans dflt FrameFmt,
        // damit es keine Abhaengigkeiten gibt
        if( pFmt->DerivedFrom() != pDoc->GetDfltFrmFmt() )
            pDoc->GetDfltFrmFmt()->Add( pFmt );
    }
    else
    {
        pFmt->Remove( this );               // austragen,

        if( CONTENT_SECTION != eType )      // den Link austragen
            pDoc->GetLinkManager().Remove( refLink );

        if( refObj.Is() )                   // als Server austragen
            pDoc->GetLinkManager().RemoveServer( &refObj );

        // ist die Section der letzte Client im Format, kann dieses
        // geloescht werden
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
        pFmt->Modify( &aMsgHint, &aMsgHint );
        if( !pFmt->GetDepends() )
        {
            // Bug: 28191 - nicht ins Undo aufnehmen, sollte schon vorher
            //          geschehen sein!!
            BOOL bUndo = pDoc->DoesUndo();
            pDoc->DoUndo( FALSE );
            pDoc->DelSectionFmt( pFmt );    // und loeschen
            pDoc->DoUndo( bUndo );
        }
    }
    if( refObj.Is() )
        refObj->Closed();
}


SwSection& SwSection::operator=( const SwSection& rCpy )
{
    sSectionNm = rCpy.sSectionNm;
    sCondition = rCpy.sCondition;
    sLinkFileName = rCpy.GetLinkFileName();
    SetLinkFilePassWd( rCpy.GetLinkFilePassWd() );
    SetConnectFlag( rCpy.IsConnectFlag() );
    SetPasswd( rCpy.GetPasswd() );

    eType = rCpy.eType;

    if( !GetFmt() )
    {
        SetProtect( rCpy.IsProtect() );
        // --> FME 2004-06-22 #114856# edit in readonly sections
        SetEditInReadonly( rCpy.IsEditInReadonly() );
        // <--
    }
    else if( rCpy.GetFmt() )
    {
        _SetProtectFlag( rCpy.bProtectFlag );
        // --> FME 2004-06-22 #114856# edit in readonly sections
        _SetEditInReadonlyFlag( rCpy.bEditInReadonlyFlag );
        // <--
    }
    else
    {
        SetProtect( rCpy.bProtectFlag );
        // --> FME 2004-06-22 #114856# edit in readonly sections
        SetEditInReadonly( rCpy.bEditInReadonlyFlag );
        // <--
    }

    bCondHiddenFlag = TRUE;     // sollte immer defaultet werden
    SetHidden( rCpy.bHidden );

    return *this;
}


BOOL SwSection::operator==( const SwSection& rCmp ) const
{
    return  sSectionNm == rCmp.sSectionNm &&
            sCondition == rCmp.sCondition &&
            eType == rCmp.eType &&
            bHidden == rCmp.bHidden &&
            IsProtect() == rCmp.IsProtect() &&
            // --> FME 2004-06-22 #114856# edit in readonly sections
            IsEditInReadonly() == rCmp.IsEditInReadonly() &&
            // <--
            GetLinkFileName() == rCmp.GetLinkFileName() &&
            GetLinkFilePassWd() == rCmp.GetLinkFilePassWd() &&
            GetPasswd() == rCmp.GetPasswd() &&
            ( !GetFmt() || !rCmp.GetFmt() || GetFmt() == rCmp.GetFmt());
}


void SwSection::_SetHiddenFlag( BOOL bTmpHidden, BOOL bCondition )
{
    SwSectionFmt* pFmt = GetFmt();
    if( pFmt )
    {
        BOOL bHide = bTmpHidden && bCondition;

        if( bHide )                         // die Nodes also "verstecken"
        {
            if( !bHiddenFlag )              // ist nicht versteckt
            {
                // wie sieht es mit dem Parent aus, ist der versteckt ?
                // (eigentlich muesste das vom bHiddenFlag angezeigt werden!)

                // erstmal allen Childs sagen, das sie versteckt sind
                SwMsgPoolItem aMsgItem( RES_SECTION_HIDDEN );
                pFmt->Modify( &aMsgItem, &aMsgItem );

                // alle Frames loeschen
                pFmt->DelFrms();
            }
        }
        else if( bHiddenFlag )              // die Nodes wieder anzeigen
        {
            // alle Frames sichtbar machen ( Childs Sections werden vom
            // MakeFrms beruecksichtigt). Aber nur wenn die ParentSection
            // nichts dagegen hat !
            SwSection* pParentSect = pFmt->GetParentSection();
            if( !pParentSect || !pParentSect->IsHiddenFlag() )
            {
                // erstmal allen Childs sagen, das der Parent nicht mehr
                // versteckt ist
                SwMsgPoolItem aMsgItem( RES_SECTION_NOT_HIDDEN );
                pFmt->Modify( &aMsgItem, &aMsgItem );

                pFmt->MakeFrms();
            }
        }
    }
}

BOOL SwSection::CalcHiddenFlag() const
{
    const SwSection* pSect = this;
    do {
        if( pSect->IsHidden() && pSect->IsCondHidden() )
            return TRUE;
    } while( 0 != ( pSect = pSect->GetParent()) );

    return FALSE;
}

BOOL SwSection::_IsProtect() const
{
    return GetFmt()->GetProtect().IsCntntProtected();
}

// --> FME 2004-06-22 #114856# edit in readonly sections
BOOL SwSection::_IsEditInReadonly() const
{
    return GetFmt()->GetEditInReadonly().GetValue();
}
// <--

void SwSection::SetHidden( BOOL bFlag )
{
    if( !bHidden == !bFlag )
        return;

    bHidden = bFlag;
    _SetHiddenFlag( bHidden, bCondHiddenFlag );
}


void SwSection::SetProtect( BOOL bFlag )
{
    if( GetFmt() )
    {
        SvxProtectItem aItem( RES_PROTECT );
        aItem.SetCntntProtect( (BOOL)bFlag );
        GetFmt()->SetFmtAttr( aItem );
    }
    else
        bProtectFlag = bFlag;
}

// --> FME 2004-06-22 #114856# edit in readonly sections
void SwSection::SetEditInReadonly( BOOL bFlag )
{
    if( GetFmt() )
    {
        SwFmtEditInReadonly aItem;
        aItem.SetValue( (BOOL)bFlag );
        GetFmt()->SetFmtAttr( aItem );
    }
    else
        bEditInReadonlyFlag = bFlag;
}
// <--

void SwSection::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    BOOL bRemake = FALSE, bUpdateFtn = FALSE;
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
        {
            SfxItemSet* pNewSet = ((SwAttrSetChg*)pNew)->GetChgSet();
            SfxItemSet* pOldSet = ((SwAttrSetChg*)pOld)->GetChgSet();
            const SfxPoolItem* pItem;

            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_PROTECT, FALSE, &pItem ) )
            {
                _SetProtectFlag( ((SvxProtectItem*)pItem)->IsCntntProtected() );
                pNewSet->ClearItem( RES_PROTECT );
                pOldSet->ClearItem( RES_PROTECT );
            }

            // --> FME 2004-06-22 #114856# edit in readonly sections
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_EDIT_IN_READONLY, FALSE, &pItem ) )
            {
                _SetEditInReadonlyFlag( ((SwFmtEditInReadonly*)pItem)->GetValue() );
                pNewSet->ClearItem( RES_EDIT_IN_READONLY );
                pOldSet->ClearItem( RES_EDIT_IN_READONLY );
            }
            // <--

            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_FTN_AT_TXTEND, FALSE, &pItem ) ||
                SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_END_AT_TXTEND, FALSE, &pItem ))
                    bUpdateFtn = TRUE;

            if( !pNewSet->Count() )
                return;
        }
        break;

    case RES_PROTECT:
        if( pNew )
        {
            BOOL bNewFlag = ((SvxProtectItem*)pNew)->IsCntntProtected();
            if( !bNewFlag )
            {
                // Abschalten: teste ob nicht vielleich ueber die Parents
                //              doch ein Schutzt besteht!
                const SwSection* pSect = this;
                do {
                    if( pSect->IsProtect() )
                    {
                        bNewFlag = TRUE;
                        break;
                    }
                } while( 0 != ( pSect = pSect->GetParent()) );
            }

            _SetProtectFlag( bNewFlag );
        }
        return;
    // --> FME 2004-06-22 #114856# edit in readonly sections
    case RES_EDIT_IN_READONLY:
        if( pNew )
        {
            BOOL bNewFlag = ((SwFmtEditInReadonly*)pNew)->GetValue();
            _SetEditInReadonlyFlag( bNewFlag );
        }
        return;
    // <--

    case RES_SECTION_HIDDEN:
        bHiddenFlag = TRUE;
        return;

    case RES_SECTION_NOT_HIDDEN:
    case RES_SECTION_RESETHIDDENFLAG:
        bHiddenFlag = bHidden && bCondHiddenFlag;
        return;

    case RES_COL:
        /* wird ggf. vom Layout erledigt */
        break;

    case RES_FTN_AT_TXTEND:
        if( pNew && pOld )
            bUpdateFtn = TRUE;
        break;

    case RES_END_AT_TXTEND:
        if( pNew && pOld )
            bUpdateFtn = TRUE;
        break;
    }

    if( bRemake )
    {
        GetFmt()->DelFrms();
        GetFmt()->MakeFrms();
    }

    if( bUpdateFtn )
    {
        SwSectionNode* pSectNd = GetFmt()->GetSectionNode( FALSE );
        if( pSectNd )
            pSectNd->GetDoc()->GetFtnIdxs().UpdateFtn(SwNodeIndex( *pSectNd ));
    }
    SwClient::Modify( pOld, pNew );
}

void SwSection::SetRefObject( SwServerObject* pObj )
{
    refObj = pObj;
}


void SwSection::SetCondHidden( BOOL bFlag )
{
    if( !bCondHiddenFlag == !bFlag )
        return;

    bCondHiddenFlag = bFlag;
    _SetHiddenFlag( bHidden, bCondHiddenFlag );
}


// setze/erfrage den gelinkten FileNamen
const String& SwSection::GetLinkFileName() const
{
    if( refLink.Is() )
    {
        String sTmp;
        switch( eType )
        {
        case DDE_LINK_SECTION:
            sTmp = refLink->GetLinkSourceName();
            break;

        case FILE_LINK_SECTION:
            {
                String sRange, sFilter;
                if( refLink->GetLinkManager() &&
                    refLink->GetLinkManager()->GetDisplayNames(
                        refLink, 0, &sTmp, &sRange, &sFilter ) )
                {
                    ( sTmp += sfx2::cTokenSeperator ) += sFilter;
                    ( sTmp += sfx2::cTokenSeperator ) += sRange;
                }
                else if( GetFmt() && !GetFmt()->GetSectionNode() )
                {
                    // ist die Section im UndoNodesArray, dann steht
                    // der Link nicht im LinkManager, kann also auch nicht
                    // erfragt werden. Dann returne den akt. Namen
                    return sLinkFileName;
                }
            }
            break;
        default: break;
        }
        ((SwSection*)this)->sLinkFileName = sTmp;
    }
    return sLinkFileName;
}


void SwSection::SetLinkFileName( const String& rNew, const String* pPassWd )
{
    if( refLink.Is() )
        refLink->SetLinkSourceName( rNew );
    else
        sLinkFileName = rNew;
    if( pPassWd )
        SetLinkFilePassWd( *pPassWd );
}

// falls es ein gelinkter Bereich war, dann muessen alle
// Child-Verknuepfungen sichtbar bemacht werden.
void SwSection::MakeChildLinksVisible( const SwSectionNode& rSectNd )
{
    const SwNode* pNd;
    const ::sfx2::SvBaseLinks& rLnks = rSectNd.GetDoc()->GetLinkManager().GetLinks();
    for( USHORT n = rLnks.Count(); n; )
    {
        ::sfx2::SvBaseLink* pBLnk = &(*rLnks[ --n ]);
        if( pBLnk && !pBLnk->IsVisible() &&
            pBLnk->ISA( SwBaseLink ) &&
            0 != ( pNd = ((SwBaseLink*)pBLnk)->GetAnchor() ) )
        {
            pNd = pNd->StartOfSectionNode();    // falls SectionNode ist!
            const SwSectionNode* pParent;
            while( 0 != ( pParent = pNd->FindSectionNode() ) &&
                    ( CONTENT_SECTION == pParent->GetSection().GetType()
                        || pNd == &rSectNd ))
                    pNd = pParent->StartOfSectionNode();

            // steht nur noch in einer normalen Section, also
            // wieder anzeigen
            if( !pParent )
                pBLnk->SetVisible( TRUE );
        }
    }
}

const SwTOXBase* SwSection::GetTOXBase() const
{
    const SwTOXBase* pRet = 0;
    if( TOX_CONTENT_SECTION == GetType() )
        pRet = PTR_CAST( SwTOXBaseSection, this );
    return pRet;
}

SwSectionFmt::SwSectionFmt( SwSectionFmt* pDrvdFrm, SwDoc *pDoc )
    : SwFrmFmt( pDoc->GetAttrPool(), sSectionFmtNm, pDrvdFrm )
{
    LockModify();
    SetFmtAttr( *GetDfltAttr( RES_COL ) );
    UnlockModify();
}

SwSectionFmt::~SwSectionFmt()
{
    if( !GetDoc()->IsInDtor() )
    {
        SwSectionNode* pSectNd;
        const SwNodeIndex* pIdx = GetCntnt( FALSE ).GetCntntIdx();
        if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
            0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            SwSection& rSect = pSectNd->GetSection();
            // falls es ein gelinkter Bereich war, dann muessen alle
            // Child-Verknuepfungen sichtbar bemacht werden.
            if( rSect.IsConnected() )
                rSect.MakeChildLinksVisible( *pSectNd );

            // vorm loeschen der Nodes pruefe, ob wir uns nicht
            // noch anzeigen muessen!
            if( rSect.IsHiddenFlag() )
            {
                SwSectionPtr pParentSect = rSect.GetParent();
                if( !pParentSect || !pParentSect->IsHiddenFlag() )
                {
                    // Nodes wieder anzeigen
                    rSect.SetHidden( FALSE );
                }
            }
            SwClientIter aIter( *this );
            SwClient *pLast = aIter.GoStart();
            while ( pLast )
            {
                if ( pLast->IsA( TYPE(SwFrm) ) )
                {
                    SwSectionFrm *pFrm = (SwSectionFrm*)pLast;
                    SwSectionFrm::MoveCntntAndDelete( pFrm, TRUE );
                    pLast = aIter.GoStart();
                }
                else
                    pLast = aIter++;
            }
            // hebe die Section doch mal auf
            SwNodeRange aRg( *pSectNd, 0, *pSectNd->EndOfSectionNode() );
            GetDoc()->GetNodes().SectionUp( &aRg );
        }
        LockModify();
        ResetFmtAttr( RES_CNTNT );
        UnlockModify();
    }
}


SwSectionPtr SwSectionFmt::_GetSection() const
{
    if( GetDepends() )
    {
        SwClientIter aIter( *(SwSectionFmt*)this );
        return (SwSectionPtr)aIter.First( TYPE(SwSection) );
    }

    ASSERT( FALSE, "keine Section als Client." )
    return 0;
}

extern void lcl_DeleteFtn( SwSectionNode *pNd, ULONG nStt, ULONG nEnd );

//Vernichtet alle Frms in aDepend (Frms werden per PTR_CAST erkannt).
void SwSectionFmt::DelFrms()
{
    SwSectionNode* pSectNd;
    const SwNodeIndex* pIdx = GetCntnt(FALSE).GetCntntIdx();
    if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
        0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
    {
        SwClientIter aIter( *this );
        SwClient *pLast = aIter.GoStart();
        // --> OD 2007-08-14 #147431#
        // First delete the <SwSectionFrm> of the <SwSectionFmt> instance
        while ( pLast )
        {
            if ( pLast->IsA( TYPE(SwFrm) ) )
            {
                SwSectionFrm *pFrm = (SwSectionFrm*)pLast;
                SwSectionFrm::MoveCntntAndDelete( pFrm, FALSE );
                pLast = aIter.GoStart();
            }
            else
            {
                pLast = aIter++;
            }
        }
        // Then delete frames of the nested <SwSectionFmt> instances
        pLast = aIter.GoStart();
        while ( pLast )
        {
            if ( pLast->IsA( TYPE(SwSectionFmt) ) )
            {
                ((SwSectionFmt*)pLast)->DelFrms();
            }
            pLast = aIter++;
        }
        // <--
        ULONG nEnde = pSectNd->EndOfSectionIndex();
        ULONG nStart = pSectNd->GetIndex()+1;
        lcl_DeleteFtn( pSectNd, nStart, nEnde );
    }
    if( pIdx )
    {
        //JP 22.09.98:
        //Hint fuer Pagedesc versenden. Das mueste eigntlich das Layout im
        //Paste der Frames selbst erledigen, aber das fuehrt dann wiederum
        //zu weiteren Folgefehlern, die mit Laufzeitkosten geloest werden
        //muesten. #56977# #55001# #56135#
        SwNodeIndex aNextNd( *pIdx );
        SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection( &aNextNd, TRUE, FALSE );
        if( pCNd )
        {
            const SfxPoolItem& rItem = pCNd->GetSwAttrSet().Get( RES_PAGEDESC );
            pCNd->Modify( (SfxPoolItem*)&rItem, (SfxPoolItem*)&rItem );
        }
    }
}


//Erzeugt die Ansichten
void SwSectionFmt::MakeFrms()
{
    SwSectionNode* pSectNd;
    const SwNodeIndex* pIdx = GetCntnt(FALSE).GetCntntIdx();

    if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
        0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
    {
        SwNodeIndex aIdx( *pIdx );
        pSectNd->MakeFrms( &aIdx );
    }
}

void lcl_ClientIter( SwSectionFmt* pFmt, const SfxPoolItem* pOld,
                                        const SfxPoolItem* pNew )
{
    SwClientIter aIter( *pFmt );
    SwClient * pLast = aIter.GoStart();
    if( pLast )
        do {
            pLast->Modify( (SfxPoolItem*)pOld, (SfxPoolItem*)pNew );
        } while( 0 != ( pLast = aIter++ ));
}

void SwSectionFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    BOOL bClients = FALSE;
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
    case RES_ATTRSET_CHG:
        if( GetDepends() )
        {
            SfxItemSet* pNewSet = ((SwAttrSetChg*)pNew)->GetChgSet();
            SfxItemSet* pOldSet = ((SwAttrSetChg*)pOld)->GetChgSet();
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                                        RES_PROTECT, FALSE, &pItem ))
            {
                lcl_ClientIter( this, pItem, pItem );
                pNewSet->ClearItem( RES_PROTECT );
                pOldSet->ClearItem( RES_PROTECT );
            }

            // --> FME 2004-06-22 #114856# edit in readonly sections
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_EDIT_IN_READONLY, FALSE, &pItem ) )
            {
                lcl_ClientIter( this, pItem, pItem );
                pNewSet->ClearItem( RES_EDIT_IN_READONLY );
                pOldSet->ClearItem( RES_EDIT_IN_READONLY );
            }
            // <--

            if( SFX_ITEM_SET == pNewSet->GetItemState(
                                    RES_FTN_AT_TXTEND, FALSE, &pItem ))
            {
                lcl_ClientIter( this, &pOldSet->Get( RES_FTN_AT_TXTEND ),
                                        pItem );
                pNewSet->ClearItem( RES_FTN_AT_TXTEND );
                pOldSet->ClearItem( RES_FTN_AT_TXTEND );
            }
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                                    RES_END_AT_TXTEND, FALSE, &pItem ))
            {
                lcl_ClientIter( this, &pOldSet->Get( RES_END_AT_TXTEND ),
                                        pItem );
                pNewSet->ClearItem( RES_END_AT_TXTEND );
                pOldSet->ClearItem( RES_END_AT_TXTEND );
            }
            if( !((SwAttrSetChg*)pOld)->GetChgSet()->Count() )
                return;
        }
        break;

    case RES_SECTION_RESETHIDDENFLAG:
    case RES_FTN_AT_TXTEND:
    case RES_END_AT_TXTEND : bClients = TRUE;
                            // no break !!
    case RES_SECTION_HIDDEN:
    case RES_SECTION_NOT_HIDDEN:
        {
            SwSection* pSect = GetSection();
            if( pSect && ( bClients || ( RES_SECTION_HIDDEN == nWhich ?
                            !pSect->IsHiddenFlag() : pSect->IsHiddenFlag() ) ) )
            {
                // selbst ueber die Clients iterieren, sollte schneller sein!
                SwClientIter aIter( *this );
                SwClient * pLast = aIter.GoStart();
                do {
                    pLast->Modify( pOld, pNew );
                } while( 0 != ( pLast = aIter++ ));
            }
        }
        return ;


    case RES_PROTECT:
    // --> FME 2004-06-22 #114856# edit in readonly sections
    case RES_EDIT_IN_READONLY:
    // <--
        // diese Messages bis zum Ende des Baums durchreichen !
        if( GetDepends() )
        {
            SwClientIter aIter( *this );
            SwClient * pLast = aIter.GoStart();
            if( pLast )     // konnte zum Anfang gesprungen werden ??
                do {
                    pLast->Modify( pOld, pNew );
                } while( 0 != ( pLast = aIter++ ));
        }
        return;     // das wars

    case RES_OBJECTDYING:
        if( !GetDoc()->IsInDtor() &&
            ((SwPtrMsgPoolItem *)pOld)->pObject == (void*)GetRegisteredIn() )
        {
            // mein Parent wird vernichtet, dann an den Parent vom Parent
            // umhaengen und wieder aktualisieren
            SwFrmFmt::Modify( pOld, pNew );     //  erst umhaengen !!!
            UpdateParent();
            return;
        }
        break;

    case RES_FMT_CHG:
        if( !GetDoc()->IsInDtor() &&
            ((SwFmtChg*)pNew)->pChangedFmt == (void*)GetRegisteredIn() &&
            ((SwFmtChg*)pNew)->pChangedFmt->IsA( TYPE( SwSectionFmt )) )
        {
            // mein Parent wird veraendert, muss mich aktualisieren
            SwFrmFmt::Modify( pOld, pNew );     //  erst umhaengen !!!
            UpdateParent();
            return;
        }
        break;
    }
    SwFrmFmt::Modify( pOld, pNew );
}

        // erfrage vom Format Informationen
BOOL SwSectionFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_FINDNEARESTNODE:
        if( ((SwFmtPageDesc&)GetFmtAttr( RES_PAGEDESC )).GetPageDesc() )
        {
            const SwSectionNode* pNd = GetSectionNode();
            if( pNd )
                ((SwFindNearestNode&)rInfo).CheckNode( *pNd );
        }
        return TRUE;

    case RES_CONTENT_VISIBLE:
        {
            SwFrm* pFrm = (SwFrm*)SwClientIter( *(SwSectionFmt*)this ).First( TYPE(SwFrm) );
            // if the current section has no own frame search for the children
            if(!pFrm)
            {
                SwClientIter aFormatIter( *(SwSectionFmt*)this );
                SwSectionFmt* pChild = (SwSectionFmt*)aFormatIter.
                                                First( TYPE(SwSectionFmt) );
                while(pChild && !pFrm)
                {
                    pFrm = (SwFrm*)SwClientIter( *pChild ).First( TYPE(SwFrm) );
                    pChild = (SwSectionFmt*)aFormatIter.Next();
                }
            }
            ((SwPtrMsgPoolItem&)rInfo).pObject = pFrm;
        }
        return FALSE;
    }
    return SwModify::GetInfo( rInfo );
}

extern "C" {

    int
#if defined( WNT )
     __cdecl
#endif
#if defined( ICC )
     _Optlink
#endif
        lcl_SectionCmpPos( const void *pFirst, const void *pSecond)
    {
        const SwSectionFmt* pFSectFmt = (*(SwSectionPtr*)pFirst)->GetFmt();
        const SwSectionFmt* pSSectFmt = (*(SwSectionPtr*)pSecond)->GetFmt();
        ASSERT( pFSectFmt && pSSectFmt &&
                pFSectFmt->GetCntnt(FALSE).GetCntntIdx() &&
                pSSectFmt->GetCntnt(FALSE).GetCntntIdx(),
                    "ungueltige Sections" );
        return (int)((long)pFSectFmt->GetCntnt(FALSE).GetCntntIdx()->GetIndex()) -
                      pSSectFmt->GetCntnt(FALSE).GetCntntIdx()->GetIndex();
    }

    int
#if defined( WNT )
     __cdecl
#endif
#if defined( ICC )
     _Optlink
#endif
        lcl_SectionCmpNm( const void *pFirst, const void *pSecond)
    {
        const SwSectionPtr pFSect = *(SwSectionPtr*)pFirst;
        const SwSectionPtr pSSect = *(SwSectionPtr*)pSecond;
        ASSERT( pFSect && pSSect, "ungueltige Sections" );
        StringCompare eCmp = pFSect->GetName().CompareTo( pSSect->GetName() );
        return eCmp == COMPARE_EQUAL ? 0
                            : eCmp == COMPARE_LESS ? 1 : -1;
    }
}

    // alle Sections, die von dieser abgeleitet sind
USHORT SwSectionFmt::GetChildSections( SwSections& rArr,
                                        SectionSort eSort,
                                        BOOL bAllSections ) const
{
    rArr.Remove( 0, rArr.Count() );

    if( GetDepends() )
    {
        SwClientIter aIter( *(SwSectionFmt*)this );
        SwClient * pLast;
        const SwNodeIndex* pIdx;
        for( pLast = aIter.First(TYPE(SwSectionFmt)); pLast; pLast = aIter.Next() )
            if( bAllSections ||
                ( 0 != ( pIdx = ((SwSectionFmt*)pLast)->GetCntnt(FALSE).
                GetCntntIdx()) && &pIdx->GetNodes() == &GetDoc()->GetNodes() ))
            {
                const SwSection* Dummy=((SwSectionFmt*)pLast)->GetSection();
                rArr.C40_INSERT( SwSection,
                    Dummy,
                    rArr.Count() );
            }

        // noch eine Sortierung erwuenscht ?
        if( 1 < rArr.Count() )
            switch( eSort )
            {
            case SORTSECT_NAME:
                qsort( (void*)rArr.GetData(),
                        rArr.Count(),
                        sizeof( SwSectionPtr ),
                        lcl_SectionCmpNm );
                break;

            case SORTSECT_POS:
                qsort( (void*)rArr.GetData(),
                        rArr.Count(),
                        sizeof( SwSectionPtr ),
                        lcl_SectionCmpPos );
                break;
            case SORTSECT_NOT: break;
            }
    }
    return rArr.Count();
}

    // erfrage, ob sich die Section im Nodes-Array oder UndoNodes-Array
    // befindet.
BOOL SwSectionFmt::IsInNodesArr() const
{
    const SwNodeIndex* pIdx = GetCntnt(FALSE).GetCntntIdx();
    return pIdx && &pIdx->GetNodes() == &GetDoc()->GetNodes();
}


void SwSectionFmt::UpdateParent()       // Parent wurde veraendert
{
    if( !GetDepends() )
        return;

    SwSectionPtr pSection = 0;
    const SvxProtectItem* pProtect(0);
    // --> FME 2004-06-22 #114856# edit in readonly sections
    const SwFmtEditInReadonly* pEditInReadonly = 0;
    // <--
    int bIsHidden = FALSE;

    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            if( pLast->IsA( TYPE(SwSectionFmt) ) )
            {
                if( !pSection )
                {
                    pSection = GetSection();
                    if( GetRegisteredIn() )
                    {
                        const SwSectionPtr pPS = GetParentSection();
                        pProtect = &pPS->GetFmt()->GetProtect();
                        // --> FME 2004-06-22 #114856# edit in readonly sections
                        pEditInReadonly = &pPS->GetFmt()->GetEditInReadonly();
                        // <--
                        bIsHidden = pPS->IsHiddenFlag();
                    }
                    else
                    {
                        pProtect = &GetProtect();
                        // --> FME 2004-06-22 #114856# edit in readonly sections
                        pEditInReadonly = &GetEditInReadonly();
                        // <--
                        bIsHidden = pSection->IsHidden();
                    }
                }
                if( pProtect->IsCntntProtected() !=
                    pSection->IsProtectFlag() )
                    pLast->Modify( (SfxPoolItem*)pProtect,
                                    (SfxPoolItem*)pProtect );

                // --> FME 2004-06-22 #114856# edit in readonly sections
                if ( pEditInReadonly->GetValue() !=
                     pSection->IsEditInReadonlyFlag() )
                    pLast->Modify( (SfxPoolItem*)pEditInReadonly,
                                    (SfxPoolItem*)pEditInReadonly );
                // <--

                if( bIsHidden == pSection->IsHiddenFlag() )
                {
                    SwMsgPoolItem aMsgItem( static_cast<USHORT>(bIsHidden
                                ? RES_SECTION_HIDDEN
                                : RES_SECTION_NOT_HIDDEN ) );
                    pLast->Modify( &aMsgItem, &aMsgItem );
                }
            }
            else if( !pSection &&
                    pLast->IsA( TYPE(SwSection) ) )
            {
                pSection = (SwSectionPtr)pLast;
                if( GetRegisteredIn() )
                {
                    const SwSectionPtr pPS = GetParentSection();
                    pProtect = &pPS->GetFmt()->GetProtect();
                    // --> FME 2004-06-22 #114856# edit in readonly sections
                    pEditInReadonly = &pPS->GetFmt()->GetEditInReadonly();
                    // <--
                    bIsHidden = pPS->IsHiddenFlag();
                }
                else
                {
                    pProtect = &GetProtect();
                    // --> FME 2004-06-22 #114856# edit in readonly sections
                    pEditInReadonly = &GetEditInReadonly();
                    // <--
                    bIsHidden = pSection->IsHidden();
                }
            }
        } while( 0 != ( pLast = aIter++ ));
}


SwSectionNode* SwSectionFmt::GetSectionNode( BOOL bAlways )
{
    const SwNodeIndex* pIdx = GetCntnt(FALSE).GetCntntIdx();
    if( pIdx && ( bAlways || &pIdx->GetNodes() == &GetDoc()->GetNodes() ))
        return pIdx->GetNode().GetSectionNode();
    return 0;
}

    // ist die Section eine gueltige fuers GlobalDocument?
const SwSection* SwSectionFmt::GetGlobalDocSection() const
{
    const SwSectionNode* pNd = GetSectionNode();
    if( pNd &&
        ( FILE_LINK_SECTION == pNd->GetSection().GetType() ||
          TOX_CONTENT_SECTION == pNd->GetSection().GetType() ) &&
        pNd->GetIndex() > pNd->GetNodes().GetEndOfExtras().GetIndex() &&
        !pNd->StartOfSectionNode()->IsSectionNode() &&
        !pNd->StartOfSectionNode()->FindSectionNode() )
        return &pNd->GetSection();
    return 0;
}

// --> OD 2007-02-14 #b6521322#
// Method to break section links inside a linked section
void lcl_BreakSectionLinksInSect( const SwSectionNode& rSectNd )
{
    if ( !rSectNd.GetDoc() )
    {
        ASSERT( false,
                "method <lcl_RemoveSectionLinksInSect(..)> - no Doc at SectionNode" );
        return;
    }

    if ( !rSectNd.GetSection().IsConnected() )
    {
        ASSERT( false,
                "method <lcl_RemoveSectionLinksInSect(..)> - no Link at Section of SectionNode" );
        return;
    }
    const ::sfx2::SvBaseLink* pOwnLink( &(rSectNd.GetSection().GetBaseLink() ) );
    const ::sfx2::SvBaseLinks& rLnks = rSectNd.GetDoc()->GetLinkManager().GetLinks();
    for ( USHORT n = rLnks.Count(); n > 0; )
    {
        SwIntrnlSectRefLink* pSectLnk = dynamic_cast<SwIntrnlSectRefLink*>(&(*rLnks[ --n ]));
        if ( pSectLnk && pSectLnk != pOwnLink &&
             pSectLnk->IsInRange( rSectNd.GetIndex(), rSectNd.EndOfSectionIndex() ) )
        {
            // break the link of the corresponding section.
            // the link is also removed from the link manager
            pSectLnk->GetSectNode()->GetSection().BreakLink();

            // for robustness, because link is removed from the link manager
            if ( n > rLnks.Count() )
            {
                n = rLnks.Count();
            }
        }
    }
}
// <--

void lcl_UpdateLinksInSect( SwBaseLink& rUpdLnk, SwSectionNode& rSectNd )
{
    SwDoc* pDoc = rSectNd.GetDoc();
    SwDocShell* pDShell = pDoc->GetDocShell();
    if( !pDShell || !pDShell->GetMedium() )
        return ;

    String sName( pDShell->GetMedium()->GetName() );
    SwBaseLink* pBLink;
    String sMimeType( SotExchange::GetFormatMimeType( FORMAT_FILE ));
    uno::Any aValue;
    aValue <<= ::rtl::OUString( sName );                        // beliebiger Name

    const ::sfx2::SvBaseLinks& rLnks = pDoc->GetLinkManager().GetLinks();
    for( USHORT n = rLnks.Count(); n; )
    {
        ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
        if( pLnk && pLnk != &rUpdLnk &&
            OBJECT_CLIENT_FILE == pLnk->GetObjType() &&
            pLnk->ISA( SwBaseLink ) &&
            ( pBLink = (SwBaseLink*)pLnk )->IsInRange( rSectNd.GetIndex(),
                                                rSectNd.EndOfSectionIndex() ) )
        {
            // liegt in dem Bereich: also updaten. Aber nur wenns nicht
            // im gleichen File liegt
            String sFName;
            pDoc->GetLinkManager().GetDisplayNames( pBLink, 0, &sFName, 0, 0 );
            if( sFName != sName )
            {
                pBLink->DataChanged( sMimeType, aValue );

                // ggfs. neu den Link-Pointer wieder suchen, damit nicht einer
                // ausgelassen oder doppelt gerufen wird.
                if( n >= rLnks.Count() && 0 != ( n = rLnks.Count() ))
                    --n;

                if( n && pLnk != &(*rLnks[ n ]) )
                {
                    // suchen - kann nur davor liegen!!
                    while( n )
                        if( pLnk == &(*rLnks[ --n ] ) )
                            break;
                }
            }
        }
    }
}


// sucht sich die richtige DocShell raus oder erzeugt eine neue:
// Der Return-Wert gibt an, was mit der Shell zu geschehen hat:
//  0 - Fehler, konnte DocShell nicht finden
//  1 - DocShell ist ein existieren Document
//  2 - DocShell wurde neu angelegt, muss also wieder geschlossen werden

int lcl_FindDocShell( SfxObjectShellRef& xDocSh,
                        const String& rFileName,
                        const String& rPasswd,
                        String& rFilter,
                        INT16 nVersion,
                        SwDocShell* pDestSh )
{
    if( !rFileName.Len() )
        return 0;

    // 1. existiert die Datei schon in der Liste aller Dokumente?
    INetURLObject aTmpObj( rFileName );
    aTmpObj.SetMark( aEmptyStr );

    // erstmal nur ueber die DocumentShells laufen und die mit dem
    // Namen heraussuchen:
    TypeId aType( TYPE(SwDocShell) );

    SfxObjectShell* pShell = pDestSh;
    BOOL bFirst = 0 != pShell;

    if( !bFirst )
        // keine DocShell uebergeben, also beginne mit der ersten aus der
        // DocShell Liste
        pShell = SfxObjectShell::GetFirst( &aType );

    while( pShell )
    {
        // die wollen wir haben
        SfxMedium* pMed = pShell->GetMedium();
        if( pMed && pMed->GetURLObject() == aTmpObj )
        {
            const SfxPoolItem* pItem;
            if( ( SFX_ITEM_SET == pMed->GetItemSet()->GetItemState(
                                            SID_VERSION, FALSE, &pItem ) )
                    ? (nVersion == ((SfxInt16Item*)pItem)->GetValue())
                    : !nVersion )
            {
                // gefunden also returnen
                xDocSh = pShell;
                return 1;
            }
        }

        if( bFirst )
        {
            bFirst = FALSE;
            pShell = SfxObjectShell::GetFirst( &aType );
        }
        else
            pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }

    // 2. selbst die Date oeffnen
    SfxMedium* pMed = new SfxMedium( aTmpObj.GetMainURL(
                             INetURLObject::NO_DECODE ), STREAM_READ, TRUE );
    if( INET_PROT_FILE == aTmpObj.GetProtocol() )
        pMed->DownLoad();     // nur mal das Medium anfassen (DownLoaden)

    const SfxFilter* pSfxFlt = 0;
    if( !pMed->GetError() )
    {
        String sFactory(String::CreateFromAscii(SwDocShell::Factory().GetShortName()));
        SfxFilterMatcher aMatcher( sFactory );

        // kein Filter, dann suche ihn. Ansonsten teste, ob der angegebene
        // ein gueltiger ist
        if( rFilter.Len() )
        {
            pSfxFlt = aMatcher.GetFilter4FilterName( rFilter );
        }

        if( nVersion )
            pMed->GetItemSet()->Put( SfxInt16Item( SID_VERSION, nVersion ));

        if( rPasswd.Len() )
            pMed->GetItemSet()->Put( SfxStringItem( SID_PASSWORD, rPasswd ));

        if( !pSfxFlt )
            aMatcher.DetectFilter( *pMed, &pSfxFlt, FALSE, FALSE );

        if( pSfxFlt )
        {
            // ohne Filter geht gar nichts
            pMed->SetFilter( pSfxFlt );

            xDocSh = new SwDocShell( SFX_CREATE_MODE_INTERNAL );
            if( xDocSh->DoLoad( pMed ) )
                return 2;
        }
    }

    if( !xDocSh.Is() )      // Medium muss noch geloescht werden
        delete pMed;

    return 0;   // das war wohl nichts
}


void SwIntrnlSectRefLink::DataChanged( const String& rMimeType,
                                const uno::Any & rValue )
{
    SwSectionNode* pSectNd = rSectFmt.GetSectionNode( FALSE );
    SwDoc* pDoc = rSectFmt.GetDoc();

    ULONG nDataFormat = SotExchange::GetFormatIdFromMimeType( rMimeType );

    if( !pSectNd || !pDoc || pDoc->IsInDtor() || ChkNoDataFlag() ||
        SvxLinkManager::RegisterStatusInfoId() == nDataFormat )
    {
        // sollten wir schon wieder im Undo stehen?
        return ;
    }

    // --> OD 2005-02-11 #i38810# - Due to possible existing signatures, the
    // document has to be modified after updating a link.
    pDoc->SetModified();
    // set additional flag that links have been updated, in order to check this
    // during load.
    pDoc->SetLinksUpdated( sal_True );
    // <--

    // Undo immer abschalten
    BOOL bWasUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );
    BOOL bWasVisibleLinks = pDoc->IsVisibleLinks();
    pDoc->SetVisibleLinks( FALSE );

    SwPaM* pPam;
    ViewShell* pVSh = 0;
    SwEditShell* pESh = pDoc->GetEditShell( &pVSh );
    pDoc->LockExpFlds();
    {
        // am Anfang des Bereichs einen leeren TextNode einfuegen
        SwNodeIndex aIdx( *pSectNd, +1 );
        SwNodeIndex aEndIdx( *pSectNd->EndOfSectionNode() );
        SwTxtNode* pNewNd = pDoc->GetNodes().MakeTxtNode( aIdx,
                        pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );

        if( pESh )
            pESh->StartAllAction();
        else if( pVSh )
            pVSh->StartAction();

        SwPosition aPos( aIdx, SwIndex( pNewNd, 0 ));
        aPos.nNode--;
        pDoc->CorrAbs( aIdx, aEndIdx, aPos, TRUE );

        pPam = new SwPaM( aPos );

        //und alles dahinter liegende loeschen
        aIdx--;
        DelFlyInRange( aIdx, aEndIdx );
        _DelBookmarks(aIdx, aEndIdx);
        aIdx++;

        pDoc->GetNodes().Delete( aIdx, aEndIdx.GetIndex() - aIdx.GetIndex() );
    }

    SwSection& rSection = pSectNd->GetSection();
    rSection.SetConnectFlag( FALSE );

    ::rtl::OUString sNewFileName;
    Reader* pRead = 0;
    switch( nDataFormat )
    {
    case FORMAT_STRING:
        pRead = ReadAscii;
        break;

    case FORMAT_RTF:
        pRead = SwReaderWriter::GetReader( READER_WRITER_RTF );
        break;

    case FORMAT_FILE:
        if( rValue.hasValue() && ( rValue >>= sNewFileName ) )
        {
            String sFilter, sRange, sFileName( sNewFileName );
            pDoc->GetLinkManager().GetDisplayNames( this, 0, &sFileName,
                                                    &sRange, &sFilter );

            RedlineMode_t eOldRedlineMode = nsRedlineMode_t::REDLINE_NONE;
            SfxObjectShellRef xDocSh;
            int nRet;
            if( !sFileName.Len() )
            {
                xDocSh = pDoc->GetDocShell();
                nRet = 1;
            }
            else
            {
                nRet = lcl_FindDocShell( xDocSh, sFileName,
                                    rSection.GetLinkFilePassWd(),
                                    sFilter, 0, pDoc->GetDocShell() );
                if( nRet )
                {
                    SwDoc* pSrcDoc = ((SwDocShell*)&xDocSh)->GetDoc();
                    eOldRedlineMode = pSrcDoc->GetRedlineMode();
                    pSrcDoc->SetRedlineMode( nsRedlineMode_t::REDLINE_SHOW_INSERT );
                }
            }

            if( nRet )
            {
                rSection.SetConnectFlag( TRUE );

                SwNodeIndex aSave( pPam->GetPoint()->nNode, -1 );
                SwNodeRange* pCpyRg = 0;

                if( xDocSh->GetMedium() &&
                    !rSection.GetLinkFilePassWd().Len() )
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == xDocSh->GetMedium()->GetItemSet()->
                        GetItemState( SID_PASSWORD, FALSE, &pItem ) )
                        rSection.SetLinkFilePassWd(
                                ((SfxStringItem*)pItem)->GetValue() );
                }

                SwDoc* pSrcDoc = ((SwDocShell*)&xDocSh)->GetDoc();

                if( sRange.Len() )
                {
                    // Rekursionen abfangen
                    BOOL bRecursion = FALSE;
                    if( pSrcDoc == pDoc )
                    {
                        SwServerObjectRef refObj( (SwServerObject*)
                                        pDoc->CreateLinkSource( sRange ));
                        if( refObj.Is() )
                        {
                            bRecursion = refObj->IsLinkInServer( this ) ||
                                        ChkNoDataFlag();
                        }
                    }

                    SwNodeIndex& rInsPos = pPam->GetPoint()->nNode;

                    SwPaM* pCpyPam = 0;
                    if( !bRecursion &&
                        pSrcDoc->SelectServerObj( sRange, pCpyPam, pCpyRg )
                        && pCpyPam )
                    {
                        if( pSrcDoc != pDoc ||
                            pCpyPam->Start()->nNode > rInsPos ||
                            rInsPos >= pCpyPam->End()->nNode )
                        {
                            pSrcDoc->CopyRange( *pCpyPam, *pPam->GetPoint(),
                                    false );
                        }
                        delete pCpyPam;
                    }
                    if( pCpyRg && pSrcDoc == pDoc &&
                        pCpyRg->aStart < rInsPos && rInsPos < pCpyRg->aEnd )
                        delete pCpyRg, pCpyRg = 0;
                }
                else if( pSrcDoc != pDoc )
                    pCpyRg = new SwNodeRange( pSrcDoc->GetNodes().GetEndOfExtras(), 2,
                                          pSrcDoc->GetNodes().GetEndOfContent() );

                // --> OD 2007-11-30 #i81653#
                // Update links of extern linked document or extern linked
                // document section, if section is protected.
                if ( pSrcDoc != pDoc &&
                     rSection.IsProtectFlag() )
                {
                    pSrcDoc->GetLinkManager().UpdateAllLinks( FALSE, TRUE, FALSE, 0 );
                }
                // <--
                if( pCpyRg )
                {
                    SwNodeIndex& rInsPos = pPam->GetPoint()->nNode;
                    BOOL bCreateFrm = rInsPos.GetIndex() <=
                                pDoc->GetNodes().GetEndOfExtras().GetIndex() ||
                                rInsPos.GetNode().FindTableNode();

                    SwTblNumFmtMerge aTNFM( *pSrcDoc, *pDoc );

                    pSrcDoc->CopyWithFlyInFly( *pCpyRg, 0, rInsPos, bCreateFrm );
                    aSave++;

                    if( !bCreateFrm )
                        ::MakeFrms( pDoc, aSave, rInsPos );

                    // den letzten Node noch loeschen, aber nur wenn
                    // erfolgreich kopiert werden konnte, also der Bereich
                    // mehr als 1 Node enthaelt
                    if( 2 < pSectNd->EndOfSectionIndex() - pSectNd->GetIndex() )
                    {
                        aSave = rInsPos;
                        pPam->Move( fnMoveBackward, fnGoNode );
                        pPam->SetMark();    // beide SwPositions ummelden!

                        pDoc->CorrAbs( aSave, *pPam->GetPoint(), 0, TRUE );
                        pDoc->GetNodes().Delete( aSave, 1 );
                    }
                    delete pCpyRg;
                }

                // --> OD 2007-02-14 #b6521322#
                lcl_BreakSectionLinksInSect( *pSectNd );
                // <--

                // update alle Links in diesem Bereich
                lcl_UpdateLinksInSect( *this, *pSectNd );
            }
            if( xDocSh.Is() )
            {
                if( 2 == nRet )
                    xDocSh->DoClose();
                else if( ((SwDocShell*)&xDocSh)->GetDoc() )
                    ((SwDocShell*)&xDocSh)->GetDoc()->SetRedlineMode(
                                eOldRedlineMode );
            }
        }
        break;
    }

    // !!!! DDE nur updaten wenn Shell vorhanden ist??
    uno::Sequence< sal_Int8 > aSeq;
    if( pRead && rValue.hasValue() && ( rValue >>= aSeq ) )
    {
        if( pESh )
        {
            pESh->Push();
            SwPaM* pCrsr = pESh->GetCrsr();
            *pCrsr->GetPoint() = *pPam->GetPoint();
            delete pPam;
            pPam = pCrsr;
        }

        SvMemoryStream aStrm( (void*)aSeq.getConstArray(), aSeq.getLength(),
                                STREAM_READ );
        aStrm.Seek( 0 );

#if OSL_DEBUG_LEVEL > 1
        {
            SvFileStream aDeb( String::CreateFromAscii(
                    "file:///d|/temp/update.txt" ), STREAM_WRITE );
            aDeb << aStrm;
        }
        aStrm.Seek( 0 );
#endif

        // TODO/MBA: it's impossible to set a BaseURL here!
        SwReader aTmpReader( aStrm, aEmptyStr, pDoc->GetDocShell()->GetMedium()->GetBaseURL(), *pPam );

        if( !IsError( aTmpReader.Read( *pRead ) ))
            rSection.SetConnectFlag( TRUE );

        if( pESh )
        {
            pESh->Pop( FALSE );
            pPam = 0;                   // pam is deleted before
        }
    }


    // Alle UndoActions entfernen und Undo wieder einschalten
    pDoc->DelAllUndoObj();
    pDoc->DoUndo( bWasUndo );
    pDoc->SetVisibleLinks( bWasVisibleLinks );

    pDoc->UnlockExpFlds();
    if( !pDoc->IsExpFldsLocked() )
        pDoc->UpdateExpFlds(NULL, true);

    if( pESh )
        pESh->EndAllAction();
    else if( pVSh )
        pVSh->EndAction();
    delete pPam;            // wurde am Anfang angelegt
}


void SwIntrnlSectRefLink::Closed()
{
    SwDoc* pDoc = rSectFmt.GetDoc();
    if( pDoc && !pDoc->IsInDtor() )
    {
        // Advise verabschiedet sich, den Bereich als nicht geschuetzt
        // kennzeichnen und das Flag umsetzen

        const SwSectionFmts& rFmts = pDoc->GetSections();
        for( USHORT n = rFmts.Count(); n; )
            if( rFmts[ --n ] == &rSectFmt )
            {
                ViewShell* pSh;
                SwEditShell* pESh = pDoc->GetEditShell( &pSh );

                if( pESh )
                    pESh->StartAllAction();
                else
                    pSh->StartAction();

                SwSection aSect( CONTENT_SECTION, aEmptyStr );
                aSect = *rSectFmt.GetSection();
                aSect.SetType( CONTENT_SECTION );
                aSect.SetLinkFileName( aEmptyStr );
                aSect.SetHidden( FALSE );
                aSect.SetProtect( FALSE );
                // --> FME 2004-06-22 #114856# edit in readonly sections
                aSect.SetEditInReadonly( FALSE );
                // <--

                aSect.SetConnectFlag( FALSE );

                pDoc->ChgSection( n, aSect );

                // alle in der Section liegenden Links werden sichtbar
                SwSectionNode* pSectNd = rSectFmt.GetSectionNode( FALSE );
                if( pSectNd )
                    pSectNd->GetSection().MakeChildLinksVisible( *pSectNd );

                if( pESh )
                    pESh->EndAllAction();
                else
                    pSh->EndAction();
                break;
            }
    }
    SvBaseLink::Closed();
}


void SwSection::CreateLink( LinkCreateType eCreateType )
{
    SwSectionFmt* pFmt = GetFmt();
    if( !pFmt || CONTENT_SECTION == eType )
        return ;

    USHORT nUpdateType = sfx2::LINKUPDATE_ALWAYS;

    if( !refLink.Is() )
        // dann mal den BaseLink aufbauen
        refLink = new SwIntrnlSectRefLink( *pFmt, nUpdateType, FORMAT_RTF );
    else
        // sonst aus dem Linkmanager entfernen
        pFmt->GetDoc()->GetLinkManager().Remove( refLink );

    SwIntrnlSectRefLink* pLnk = (SwIntrnlSectRefLink*)&refLink;

    String sCmd( sLinkFileName );
    xub_StrLen nPos;
    while( STRING_NOTFOUND != (nPos = sCmd.SearchAscii( "  " )) )
        sCmd.Erase( nPos, 1 );

    pLnk->SetUpdateMode( nUpdateType );
    pLnk->SetVisible( pFmt->GetDoc()->IsVisibleLinks() );

    switch( eType )
    {
    case DDE_LINK_SECTION:
        pLnk->SetLinkSourceName( sCmd );
        pFmt->GetDoc()->GetLinkManager().InsertDDELink( pLnk );
        break;
    case FILE_LINK_SECTION:
        {
            pLnk->SetContentType( FORMAT_FILE );
            String sFltr( sCmd.GetToken( 1, sfx2::cTokenSeperator ) );
            String sRange( sCmd.GetToken( 2, sfx2::cTokenSeperator ) );
            pFmt->GetDoc()->GetLinkManager().InsertFileLink( *pLnk,
                                static_cast<USHORT>(eType),
                                sCmd.GetToken( 0, sfx2::cTokenSeperator ),
                                ( sFltr.Len() ? &sFltr : 0 ),
                                ( sRange.Len() ? &sRange : 0 ) );
        }
        break;
    default:
        ASSERT( !this, "Was ist das fuer ein Link?" )
    }

    switch( eCreateType )
    {
    case CREATE_CONNECT:            // Link gleich connecten
        pLnk->Connect();
        break;

    case CREATE_UPDATE:         // Link connecten und updaten
        pLnk->Update();
        break;
    case CREATE_NONE: break;
    }
}

// --> OD 2007-02-14 #b6521322#
void SwSection::BreakLink()
{
    const SectionType eCurrentType( GetType() );
    if ( eCurrentType == CONTENT_SECTION ||
         eCurrentType == TOX_HEADER_SECTION ||
         eCurrentType == TOX_CONTENT_SECTION )
    {
        // nothing to do
        return;
    }

    // release link, if it exists
    if ( refLink.Is() )
    {
        if ( GetFmt() )
        {
            GetFmt()->GetDoc()->GetLinkManager().Remove( refLink );
        }
        refLink.Clear();
    }
    // change type
    SetType( CONTENT_SECTION );
    // reset linked file data
    SetLinkFileName( aEmptyStr );
    SetLinkFilePassWd( aEmptyStr );
}
// <--

const SwNode* SwIntrnlSectRefLink::GetAnchor() const
{
    return rSectFmt.GetSectionNode( FALSE );
}


BOOL SwIntrnlSectRefLink::IsInRange( ULONG nSttNd, ULONG nEndNd,
                                     xub_StrLen , xub_StrLen ) const
{
    SwStartNode* pSttNd = rSectFmt.GetSectionNode( FALSE );
    return pSttNd &&
            nSttNd < pSttNd->GetIndex() &&
            pSttNd->EndOfSectionIndex() < nEndNd;
}



