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


#include <stdlib.h>
#include <hintids.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <editeng/protitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/fcontnr.hxx>
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <frmtool.hxx>
#include <editsh.hxx>
#include <hints.hxx>
#include <docsh.hxx>
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
#include <fmteiro.hxx>
#include <swerror.h>
#include <unosection.hxx>
#include <switerator.hxx>
#include <svl/smplhint.hxx>
#include <algorithm>

using namespace ::com::sun::star;


SV_IMPL_REF( SwServerObject )

#define sSectionFmtNm aEmptyStr

class SwIntrnlSectRefLink : public SwBaseLink
{
    SwSectionFmt& rSectFmt;
public:
    SwIntrnlSectRefLink( SwSectionFmt& rFmt, sal_uInt16 nUpdateType, sal_uInt16 nFmt )
        : SwBaseLink( nUpdateType, nFmt ),
        rSectFmt( rFmt )
    {}

    virtual void Closed();
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );

    virtual const SwNode* GetAnchor() const;
    virtual sal_Bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;

    inline SwSectionNode* GetSectNode()
    {
        const SwNode* pSectNd( const_cast<SwIntrnlSectRefLink*>(this)->GetAnchor() );
        return const_cast<SwSectionNode*>( dynamic_cast<const SwSectionNode*>( pSectNd ) );
    }
};


TYPEINIT1(SwSectionFmt,SwFrmFmt );
TYPEINIT1(SwSection,SwClient );


SwSectionData::SwSectionData(SectionType const eType, String const& rName)
    : m_eType(eType)
    , m_sSectionName(rName)
    , m_bHiddenFlag(false)
    , m_bProtectFlag(false)
    , m_bEditInReadonlyFlag(false) // edit in readonly sections
    , m_bHidden(false)
    , m_bCondHiddenFlag(true)
    , m_bConnectFlag(true)
{
}

// this must have the same semantics as operator=()
SwSectionData::SwSectionData(SwSection const& rSection)
    : m_eType(rSection.GetType())
    , m_sSectionName(rSection.GetSectionName())
    , m_sCondition(rSection.GetCondition())
    , m_sLinkFileName(rSection.GetLinkFileName())
    , m_sLinkFilePassword(rSection.GetLinkFilePassword())
    , m_Password(rSection.GetPassword())
    , m_bHiddenFlag(rSection.IsHiddenFlag())
    , m_bProtectFlag(rSection.IsProtect())
    // edit in readonly sections
    , m_bEditInReadonlyFlag(rSection.IsEditInReadonly())
    , m_bHidden(rSection.IsHidden())
    , m_bCondHiddenFlag(true)
    , m_bConnectFlag(rSection.IsConnectFlag())
{
}

// this must have the same semantics as operator=()
SwSectionData::SwSectionData(SwSectionData const& rOther)
    : m_eType(rOther.m_eType)
    , m_sSectionName(rOther.m_sSectionName)
    , m_sCondition(rOther.m_sCondition)
    , m_sLinkFileName(rOther.m_sLinkFileName)
    , m_sLinkFilePassword(rOther.m_sLinkFilePassword)
    , m_Password(rOther.m_Password)
    , m_bHiddenFlag(rOther.m_bHiddenFlag)
    , m_bProtectFlag(rOther.m_bProtectFlag)
    // edit in readonly sections
    , m_bEditInReadonlyFlag(rOther.m_bEditInReadonlyFlag)
    , m_bHidden(rOther.m_bHidden)
    , m_bCondHiddenFlag(true)
    , m_bConnectFlag(rOther.m_bConnectFlag)
{
}

// the semantics here are weird for reasons of backward compatibility
SwSectionData & SwSectionData::operator= (SwSectionData const& rOther)
{
    m_eType = rOther.m_eType;
    m_sSectionName = rOther.m_sSectionName;
    m_sCondition = rOther.m_sCondition;
    m_sLinkFileName = rOther.m_sLinkFileName;
    m_sLinkFilePassword = rOther.m_sLinkFilePassword;
    m_bConnectFlag = rOther.m_bConnectFlag;
    m_Password = rOther.m_Password;

    m_bEditInReadonlyFlag = rOther.m_bEditInReadonlyFlag;
    m_bProtectFlag = rOther.m_bProtectFlag;

    m_bHidden = rOther.m_bHidden;
    // FIXME: old code did not assign m_bHiddenFlag ?
    // FIXME: why should m_bCondHiddenFlag always default to true?
    m_bCondHiddenFlag = true;

    return *this;
}

// the semantics here are weird for reasons of backward compatibility
bool SwSectionData::operator==(SwSectionData const& rOther) const
{
    return (m_eType == rOther.m_eType)
        && (m_sSectionName == rOther.m_sSectionName)
        && (m_sCondition == rOther.m_sCondition)
        && (m_bHidden == rOther.m_bHidden)
        && (m_bProtectFlag == rOther.m_bProtectFlag)
        && (m_bEditInReadonlyFlag == rOther.m_bEditInReadonlyFlag)
        && (m_sLinkFileName == rOther.m_sLinkFileName)
        && (m_sLinkFilePassword == rOther.m_sLinkFilePassword)
        && (m_Password == rOther.m_Password);
    // FIXME: old code ignored m_bCondHiddenFlag m_bHiddenFlag m_bConnectFlag
}

// SwSection ===========================================================

SwSection::SwSection(
        SectionType const eType, String const& rName, SwSectionFmt & rFormat)
    : SwClient(& rFormat)
    , m_Data(eType, rName)
{
    SwSection *const pParentSect = GetParent();
    if( pParentSect )
    {
        if( pParentSect->IsHiddenFlag() )
        {
            SetHidden( true );
        }

        m_Data.SetProtectFlag( pParentSect->IsProtectFlag() );
        // edit in readonly sections
        m_Data.SetEditInReadonlyFlag( pParentSect->IsEditInReadonlyFlag() );
    }

    if (!m_Data.IsProtectFlag())
    {
        m_Data.SetProtectFlag( rFormat.GetProtect().IsCntntProtected() );
    }

    if (!m_Data.IsEditInReadonlyFlag()) // edit in readonly sections
    {
        m_Data.SetEditInReadonlyFlag( rFormat.GetEditInReadonly().GetValue() );
    }
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
            pFmt->RegisterToFormat( *pDoc->GetDfltFrmFmt() );
    }
    else
    {
        pFmt->Remove( this );               // austragen,

        if (CONTENT_SECTION != m_Data.GetType())
        {
            pDoc->GetLinkManager().Remove( m_RefLink );
        }

        if (m_RefObj.Is())
        {
            pDoc->GetLinkManager().RemoveServer( &m_RefObj );
        }

        // ist die Section der letzte Client im Format, kann dieses
        // geloescht werden
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
        pFmt->ModifyNotification( &aMsgHint, &aMsgHint );
        if( !pFmt->GetDepends() )
        {
            // Bug: 28191 - nicht ins Undo aufnehmen, sollte schon vorher
            //          geschehen sein!!
            ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
            pDoc->DelSectionFmt( pFmt );    // und loeschen
        }
    }
    if (m_RefObj.Is())
    {
        m_RefObj->Closed();
    }
}

void SwSection::SetSectionData(SwSectionData const& rData)
{
    bool const bOldHidden( m_Data.IsHidden() );
    m_Data = rData;
    // next 2 may actually overwrite m_Data.m_b{Protect,EditInReadonly}Flag
    // in Modify, which should result in same flag value as the old code!
    SetProtect(m_Data.IsProtectFlag());
    SetEditInReadonly(m_Data.IsEditInReadonlyFlag());
    if (bOldHidden != m_Data.IsHidden()) // check if changed...
    {
        ImplSetHiddenFlag(m_Data.IsHidden(), m_Data.IsCondHidden());
    }
}

bool SwSection::DataEquals(SwSectionData const& rCmp) const
{
    // note that the old code compared the flags of the parameter with the
    // format attributes of this; the following mess should do the same...
    (void) GetLinkFileName(); // updates m_sLinkFileName
    bool const bProtect(m_Data.IsProtectFlag());
    bool const bEditInReadonly(m_Data.IsEditInReadonlyFlag());
    const_cast<SwSection*>(this)->m_Data.SetProtectFlag(IsProtect());
    const_cast<SwSection*>(this)->m_Data
        .SetEditInReadonlyFlag(IsEditInReadonly());
    bool const bResult( m_Data == rCmp );
    const_cast<SwSection*>(this)->m_Data.SetProtectFlag(bProtect);
    const_cast<SwSection*>(this)->m_Data.SetEditInReadonlyFlag(bEditInReadonly);
    return bResult;
}


void SwSection::ImplSetHiddenFlag(bool const bTmpHidden, bool const bCondition)
{
    SwSectionFmt* pFmt = GetFmt();
    OSL_ENSURE(pFmt, "ImplSetHiddenFlag: no format?");
    if( pFmt )
    {
        const bool bHide = bTmpHidden && bCondition;

        if (bHide) // should be hidden
        {
            if (!m_Data.IsHiddenFlag()) // is not hidden
            {
                // wie sieht es mit dem Parent aus, ist der versteckt ?
                // (eigentlich muesste das vom bHiddenFlag angezeigt werden!)

                // erstmal allen Children sagen, das sie versteckt sind
                SwMsgPoolItem aMsgItem( RES_SECTION_HIDDEN );
                pFmt->ModifyNotification( &aMsgItem, &aMsgItem );

                // alle Frames loeschen
                pFmt->DelFrms();
            }
        }
        else if (m_Data.IsHiddenFlag()) // show Nodes again
        {
            // alle Frames sichtbar machen ( Children Sections werden vom
            // MakeFrms beruecksichtigt). Aber nur wenn die ParentSection
            // nichts dagegen hat !
            SwSection* pParentSect = pFmt->GetParentSection();
            if( !pParentSect || !pParentSect->IsHiddenFlag() )
            {
                // erstmal allen Children sagen, das der Parent nicht mehr
                // versteckt ist
                SwMsgPoolItem aMsgItem( RES_SECTION_NOT_HIDDEN );
                pFmt->ModifyNotification( &aMsgItem, &aMsgItem );

                pFmt->MakeFrms();
            }
        }
    }
}

sal_Bool SwSection::CalcHiddenFlag() const
{
    const SwSection* pSect = this;
    do {
        if( pSect->IsHidden() && pSect->IsCondHidden() )
            return sal_True;
    } while( 0 != ( pSect = pSect->GetParent()) );

    return sal_False;
}

bool SwSection::IsProtect() const
{
    SwSectionFmt *const pFmt( GetFmt() );
    OSL_ENSURE(pFmt, "SwSection::IsProtect: no format?");
    return (pFmt)
        ?   pFmt->GetProtect().IsCntntProtected()
        :   IsProtectFlag();
}

// edit in readonly sections
bool SwSection::IsEditInReadonly() const
{
    SwSectionFmt *const pFmt( GetFmt() );
    OSL_ENSURE(pFmt, "SwSection::IsEditInReadonly: no format?");
    return (pFmt)
        ?   pFmt->GetEditInReadonly().GetValue()
        :   IsEditInReadonlyFlag();
}

void SwSection::SetHidden(bool const bFlag)
{
    if (!m_Data.IsHidden() == !bFlag)
        return;

    m_Data.SetHidden(bFlag);
    ImplSetHiddenFlag(bFlag, m_Data.IsCondHidden());
}


void SwSection::SetProtect(bool const bFlag)
{
    SwSectionFmt *const pFormat( GetFmt() );
    OSL_ENSURE(pFormat, "SwSection::SetProtect: no format?");
    if (pFormat)
    {
        SvxProtectItem aItem( RES_PROTECT );
        aItem.SetCntntProtect( (sal_Bool)bFlag );
        pFormat->SetFmtAttr( aItem );
        // note: this will call m_Data.SetProtectFlag via Modify!
    }
    else
    {
        m_Data.SetProtectFlag(bFlag);
    }
}

// edit in readonly sections
void SwSection::SetEditInReadonly(bool const bFlag)
{
    SwSectionFmt *const pFormat( GetFmt() );
    OSL_ENSURE(pFormat, "SwSection::SetEditInReadonly: no format?");
    if (pFormat)
    {
        SwFmtEditInReadonly aItem;
        aItem.SetValue( (sal_Bool)bFlag );
        pFormat->SetFmtAttr( aItem );
        // note: this will call m_Data.SetEditInReadonlyFlag via Modify!
    }
    else
    {
        m_Data.SetEditInReadonlyFlag(bFlag);
    }
}

void SwSection::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    bool bRemake = false;
    bool bUpdateFtn = false;
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
        {
            SfxItemSet* pNewSet = ((SwAttrSetChg*)pNew)->GetChgSet();
            SfxItemSet* pOldSet = ((SwAttrSetChg*)pOld)->GetChgSet();
            const SfxPoolItem* pItem;

            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_PROTECT, sal_False, &pItem ) )
            {
                m_Data.SetProtectFlag( static_cast<SvxProtectItem const*>(pItem)
                        ->IsCntntProtected() );
                pNewSet->ClearItem( RES_PROTECT );
                pOldSet->ClearItem( RES_PROTECT );
            }

            // --> edit in readonly sections
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_EDIT_IN_READONLY, sal_False, &pItem ) )
            {
                m_Data.SetEditInReadonlyFlag(
                    static_cast<SwFmtEditInReadonly const*>(pItem)->GetValue());
                pNewSet->ClearItem( RES_EDIT_IN_READONLY );
                pOldSet->ClearItem( RES_EDIT_IN_READONLY );
            }

            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_FTN_AT_TXTEND, sal_False, &pItem ) ||
                SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_END_AT_TXTEND, sal_False, &pItem ))
            {
                    bUpdateFtn = true;
            }

            if( !pNewSet->Count() )
                return;
        }
        break;

    case RES_PROTECT:
        if( pNew )
        {
            bool bNewFlag =
                static_cast<const SvxProtectItem*>(pNew)->IsCntntProtected();
            if( !bNewFlag )
            {
                // Abschalten: teste ob nicht vielleich ueber die Parents
                //              doch ein Schutzt besteht!
                const SwSection* pSect = this;
                do {
                    if( pSect->IsProtect() )
                    {
                        bNewFlag = true;
                        break;
                    }
                    pSect = pSect->GetParent();
                } while (pSect);
            }

            m_Data.SetProtectFlag( bNewFlag );
        }
        return;
    // edit in readonly sections
    case RES_EDIT_IN_READONLY:
        if( pNew )
        {
            const bool bNewFlag =
                static_cast<const SwFmtEditInReadonly*>(pNew)->GetValue();
            m_Data.SetEditInReadonlyFlag( bNewFlag );
        }
        return;

    case RES_SECTION_HIDDEN:
        m_Data.SetHiddenFlag(true);
        return;

    case RES_SECTION_NOT_HIDDEN:
    case RES_SECTION_RESETHIDDENFLAG:
        m_Data.SetHiddenFlag( m_Data.IsHidden() && m_Data.IsCondHidden() );
        return;

    case RES_COL:
        /* wird ggf. vom Layout erledigt */
        break;

    case RES_FTN_AT_TXTEND:
        if( pNew && pOld )
        {
            bUpdateFtn = true;
        }
        break;

    case RES_END_AT_TXTEND:
        if( pNew && pOld )
        {
            bUpdateFtn = true;
        }
        break;

    default:
        CheckRegistration( pOld, pNew );
        break;
    }

    if( bRemake )
    {
        GetFmt()->DelFrms();
        GetFmt()->MakeFrms();
    }

    if( bUpdateFtn )
    {
        SwSectionNode* pSectNd = GetFmt()->GetSectionNode( sal_False );
        if( pSectNd )
            pSectNd->GetDoc()->GetFtnIdxs().UpdateFtn(SwNodeIndex( *pSectNd ));
    }
}

void SwSection::SetRefObject( SwServerObject* pObj )
{
    m_RefObj = pObj;
}


void SwSection::SetCondHidden(bool const bFlag)
{
    if (!m_Data.IsCondHidden() == !bFlag)
        return;

    m_Data.SetCondHidden(bFlag);
    ImplSetHiddenFlag(m_Data.IsHidden(), bFlag);
}


// setze/erfrage den gelinkten FileNamen
const String& SwSection::GetLinkFileName() const
{
    if (m_RefLink.Is())
    {
        String sTmp;
        switch (m_Data.GetType())
        {
        case DDE_LINK_SECTION:
            sTmp = m_RefLink->GetLinkSourceName();
            break;

        case FILE_LINK_SECTION:
            {
                String sRange, sFilter;
                if (m_RefLink->GetLinkManager() &&
                    m_RefLink->GetLinkManager()->GetDisplayNames(
                        m_RefLink, 0, &sTmp, &sRange, &sFilter ))
                {
                    ( sTmp += sfx2::cTokenSeperator ) += sFilter;
                    ( sTmp += sfx2::cTokenSeperator ) += sRange;
                }
                else if( GetFmt() && !GetFmt()->GetSectionNode() )
                {
                    // ist die Section im UndoNodesArray, dann steht
                    // der Link nicht im LinkManager, kann also auch nicht
                    // erfragt werden. Dann returne den akt. Namen
                    return m_Data.GetLinkFileName();
                }
            }
            break;
        default: break;
        }
        const_cast<SwSection*>(this)->m_Data.SetLinkFileName(sTmp);
    }
    return m_Data.GetLinkFileName();
}


void SwSection::SetLinkFileName(const String& rNew, String const*const pPassWd)
{
    if (m_RefLink.Is())
    {
        m_RefLink->SetLinkSourceName( rNew );
    }
    m_Data.SetLinkFileName(rNew);
    if( pPassWd )
    {
        SetLinkFilePassword( *pPassWd );
    }
}

// falls es ein gelinkter Bereich war, dann muessen alle
// Child-Verknuepfungen sichtbar bemacht werden.
void SwSection::MakeChildLinksVisible( const SwSectionNode& rSectNd )
{
    const SwNode* pNd;
    const ::sfx2::SvBaseLinks& rLnks = rSectNd.GetDoc()->GetLinkManager().GetLinks();
    for( sal_uInt16 n = rLnks.size(); n; )
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
                pBLnk->SetVisible( sal_True );
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

// SwSectionFmt ========================================================

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
        const SwNodeIndex* pIdx = GetCntnt( sal_False ).GetCntntIdx();
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
                SwSection* pParentSect = rSect.GetParent();
                if( !pParentSect || !pParentSect->IsHiddenFlag() )
                {
                    // Nodes wieder anzeigen
                    rSect.SetHidden(false);
                }
            }
            // mba: test iteration; objects are removed while iterating
            // use hint which allows to specify, if the content shall be saved or not
            CallSwClientNotify( SwSectionFrmMoveAndDeleteHint( sal_True ) );

            // hebe die Section doch mal auf
            SwNodeRange aRg( *pSectNd, 0, *pSectNd->EndOfSectionNode() );
            GetDoc()->GetNodes().SectionUp( &aRg );
        }
        LockModify();
        ResetFmtAttr( RES_CNTNT );
        UnlockModify();
    }
}


SwSection * SwSectionFmt::GetSection() const
{
    return SwIterator<SwSection,SwSectionFmt>::FirstElement( *this );
}

extern void sw_DeleteFtn( SwSectionNode *pNd, sal_uLong nStt, sal_uLong nEnd );

//Vernichtet alle Frms in aDepend (Frms werden per PTR_CAST erkannt).
void SwSectionFmt::DelFrms()
{
    SwSectionNode* pSectNd;
    const SwNodeIndex* pIdx = GetCntnt(sal_False).GetCntntIdx();
    if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
        0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
    {
        // First delete the <SwSectionFrm> of the <SwSectionFmt> instance
        // mba: test iteration as objects are removed in iteration
        // use hint which allows to specify, if the content shall be saved or not
        CallSwClientNotify( SwSectionFrmMoveAndDeleteHint( sal_False ) );

        // Then delete frames of the nested <SwSectionFmt> instances
        SwIterator<SwSectionFmt,SwSectionFmt> aIter( *this );
        SwSectionFmt *pLast = aIter.First();
        while ( pLast )
        {
            pLast->DelFrms();
            pLast = aIter.Next();
        }

        sal_uLong nEnde = pSectNd->EndOfSectionIndex();
        sal_uLong nStart = pSectNd->GetIndex()+1;
        sw_DeleteFtn( pSectNd, nStart, nEnde );
    }
    if( pIdx )
    {
        //Hint fuer Pagedesc versenden. Das mueste eigntlich das Layout im
        //Paste der Frames selbst erledigen, aber das fuehrt dann wiederum
        //zu weiteren Folgefehlern, die mit Laufzeitkosten geloest werden
        //muesten.
        SwNodeIndex aNextNd( *pIdx );
        SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection( &aNextNd, sal_True, sal_False );
        if( pCNd )
        {
            const SfxPoolItem& rItem = pCNd->GetSwAttrSet().Get( RES_PAGEDESC );
            pCNd->ModifyNotification( (SfxPoolItem*)&rItem, (SfxPoolItem*)&rItem );
        }
    }
}


//Erzeugt die Ansichten
void SwSectionFmt::MakeFrms()
{
    SwSectionNode* pSectNd;
    const SwNodeIndex* pIdx = GetCntnt(sal_False).GetCntntIdx();

    if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
        0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
    {
        SwNodeIndex aIdx( *pIdx );
        pSectNd->MakeFrms( &aIdx );
    }
}

void SwSectionFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    sal_Bool bClients = sal_False;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
    case RES_ATTRSET_CHG:
        if( GetDepends() )
        {
            SfxItemSet* pNewSet = ((SwAttrSetChg*)pNew)->GetChgSet();
            SfxItemSet* pOldSet = ((SwAttrSetChg*)pOld)->GetChgSet();
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                                        RES_PROTECT, sal_False, &pItem ))
            {
                ModifyBroadcast( (SfxPoolItem*)pItem, (SfxPoolItem*)pItem );
                pNewSet->ClearItem( RES_PROTECT );
                pOldSet->ClearItem( RES_PROTECT );
            }

            // --> edit in readonly sections
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                        RES_EDIT_IN_READONLY, sal_False, &pItem ) )
            {
                ModifyBroadcast( (SfxPoolItem*)pItem, (SfxPoolItem*)pItem );
                pNewSet->ClearItem( RES_EDIT_IN_READONLY );
                pOldSet->ClearItem( RES_EDIT_IN_READONLY );
            }

            if( SFX_ITEM_SET == pNewSet->GetItemState(
                                    RES_FTN_AT_TXTEND, sal_False, &pItem ))
            {
                ModifyBroadcast( (SfxPoolItem*)&pOldSet->Get( RES_FTN_AT_TXTEND ), (SfxPoolItem*)pItem );
                pNewSet->ClearItem( RES_FTN_AT_TXTEND );
                pOldSet->ClearItem( RES_FTN_AT_TXTEND );
            }
            if( SFX_ITEM_SET == pNewSet->GetItemState(
                                    RES_END_AT_TXTEND, sal_False, &pItem ))
            {
                ModifyBroadcast( (SfxPoolItem*)&pOldSet->Get( RES_END_AT_TXTEND ), (SfxPoolItem*)pItem );
                pNewSet->ClearItem( RES_END_AT_TXTEND );
                pOldSet->ClearItem( RES_END_AT_TXTEND );
            }
            if( !((SwAttrSetChg*)pOld)->GetChgSet()->Count() )
                return;
        }
        break;

    case RES_SECTION_RESETHIDDENFLAG:
    case RES_FTN_AT_TXTEND:
    case RES_END_AT_TXTEND : bClients = sal_True;
                            // no break !!
    case RES_SECTION_HIDDEN:
    case RES_SECTION_NOT_HIDDEN:
        {
            SwSection* pSect = GetSection();
            if( pSect && ( bClients || ( RES_SECTION_HIDDEN == nWhich ?
                            !pSect->IsHiddenFlag() : pSect->IsHiddenFlag() ) ) )
            {
                ModifyBroadcast( pOld, pNew );
            }
        }
        return ;


    case RES_PROTECT:
    case RES_EDIT_IN_READONLY: // edit in readonly sections
        // diese Messages bis zum Ende des Baums durchreichen !
        if( GetDepends() )
        {
            ModifyBroadcast( pOld, pNew );
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

    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXTextSection(uno::Reference<text::XTextSection>(0));
    }
}

        // erfrage vom Format Informationen
sal_Bool SwSectionFmt::GetInfo( SfxPoolItem& rInfo ) const
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
        return sal_True;

    case RES_CONTENT_VISIBLE:
        {
            SwFrm* pFrm = SwIterator<SwFrm,SwFmt>::FirstElement(*this);
            // if the current section has no own frame search for the children
            if(!pFrm)
            {
                SwIterator<SwSectionFmt,SwSectionFmt> aFormatIter(*this);
                SwSectionFmt* pChild = aFormatIter.First();
                while(pChild && !pFrm)
                {
                    pFrm = SwIterator<SwFrm,SwFmt>::FirstElement(*pChild);
                    pChild = aFormatIter.Next();
                }
            }
            ((SwPtrMsgPoolItem&)rInfo).pObject = pFrm;
        }
        return sal_False;
    }
    return SwModify::GetInfo( rInfo );
}

static bool lcl_SectionCmpPos( const SwSection *pFirst, const SwSection *pSecond)
{
    const SwSectionFmt* pFSectFmt = pFirst->GetFmt();
    const SwSectionFmt* pSSectFmt = pSecond->GetFmt();
    OSL_ENSURE( pFSectFmt && pSSectFmt &&
            pFSectFmt->GetCntnt(sal_False).GetCntntIdx() &&
            pSSectFmt->GetCntnt(sal_False).GetCntntIdx(),
                "ungueltige Sections" );
    return pFSectFmt->GetCntnt(sal_False).GetCntntIdx()->GetIndex() <
                  pSSectFmt->GetCntnt(sal_False).GetCntntIdx()->GetIndex();
}

static bool lcl_SectionCmpNm( const SwSection *pFSect, const SwSection *pSSect)
{
    OSL_ENSURE( pFSect && pSSect, "ungueltige Sections" );
    StringCompare const eCmp =
        pFSect->GetSectionName().CompareTo( pSSect->GetSectionName() );
    return eCmp == COMPARE_LESS;
}

    // alle Sections, die von dieser abgeleitet sind
sal_uInt16 SwSectionFmt::GetChildSections( SwSections& rArr,
                                        SectionSort eSort,
                                        sal_Bool bAllSections ) const
{
    rArr.clear();

    if( GetDepends() )
    {
        SwIterator<SwSectionFmt,SwSectionFmt> aIter(*this);
        const SwNodeIndex* pIdx;
        for( SwSectionFmt* pLast = aIter.First(); pLast; pLast = aIter.Next() )
            if( bAllSections ||
                ( 0 != ( pIdx = pLast->GetCntnt(sal_False).
                GetCntntIdx()) && &pIdx->GetNodes() == &GetDoc()->GetNodes() ))
            {
                SwSection* pDummy = pLast->GetSection();
                rArr.push_back( pDummy );
            }

        // noch eine Sortierung erwuenscht ?
        if( 1 < rArr.size() )
            switch( eSort )
            {
            case SORTSECT_NAME:
                std::sort( rArr.begin(), rArr.end(), lcl_SectionCmpNm );
                break;

            case SORTSECT_POS:
                std::sort( rArr.begin(), rArr.end(), lcl_SectionCmpPos );
                break;
            case SORTSECT_NOT: break;
            }
    }
    return rArr.size();
}

    // erfrage, ob sich die Section im Nodes-Array oder UndoNodes-Array
    // befindet.
sal_Bool SwSectionFmt::IsInNodesArr() const
{
    const SwNodeIndex* pIdx = GetCntnt(sal_False).GetCntntIdx();
    return pIdx && &pIdx->GetNodes() == &GetDoc()->GetNodes();
}


void SwSectionFmt::UpdateParent()       // Parent wurde veraendert
{
    if( !GetDepends() )
        return;

    SwSection* pSection = 0;
    const SvxProtectItem* pProtect(0);
    // edit in readonly sections
    const SwFmtEditInReadonly* pEditInReadonly = 0;
    bool bIsHidden = false;

    SwClientIter aIter( *this );    // TODO
    ::SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            if( pLast->IsA( TYPE(SwSectionFmt) ) )
            {
                if( !pSection )
                {
                    pSection = GetSection();
                    if( GetRegisteredIn() )
                    {
                        const SwSection* pPS = GetParentSection();
                        pProtect = &pPS->GetFmt()->GetProtect();
                        // edit in readonly sections
                        pEditInReadonly = &pPS->GetFmt()->GetEditInReadonly();
                        bIsHidden = pPS->IsHiddenFlag();
                    }
                    else
                    {
                        pProtect = &GetProtect();
                        // edit in readonly sections
                        pEditInReadonly = &GetEditInReadonly();
                        bIsHidden = pSection->IsHidden();
                    }
                }
                if (!pProtect->IsCntntProtected() !=
                    !pSection->IsProtectFlag())
                {
                    pLast->ModifyNotification( (SfxPoolItem*)pProtect,
                                    (SfxPoolItem*)pProtect );
                }

                // edit in readonly sections
                if (!pEditInReadonly->GetValue() !=
                    !pSection->IsEditInReadonlyFlag())
                {
                    pLast->ModifyNotification( (SfxPoolItem*)pEditInReadonly,
                                    (SfxPoolItem*)pEditInReadonly );
                }

                if( bIsHidden == pSection->IsHiddenFlag() )
                {
                    SwMsgPoolItem aMsgItem( static_cast<sal_uInt16>(bIsHidden
                                ? RES_SECTION_HIDDEN
                                : RES_SECTION_NOT_HIDDEN ) );
                    pLast->ModifyNotification( &aMsgItem, &aMsgItem );
                }
            }
            else if( !pSection &&
                    pLast->IsA( TYPE(SwSection) ) )
            {
                pSection = (SwSection*)pLast;
                if( GetRegisteredIn() )
                {
                    const SwSection* pPS = GetParentSection();
                    pProtect = &pPS->GetFmt()->GetProtect();
                    // edit in readonly sections
                    pEditInReadonly = &pPS->GetFmt()->GetEditInReadonly();
                    bIsHidden = pPS->IsHiddenFlag();
                }
                else
                {
                    pProtect = &GetProtect();
                    // edit in readonly sections
                    pEditInReadonly = &GetEditInReadonly();
                    bIsHidden = pSection->IsHidden();
                }
            }
        } while( 0 != ( pLast = aIter++ ));
}


SwSectionNode* SwSectionFmt::GetSectionNode(bool const bAlways)
{
    const SwNodeIndex* pIdx = GetCntnt(sal_False).GetCntntIdx();
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

// sw::Metadatable
::sfx2::IXmlIdRegistry& SwSectionFmt::GetRegistry()
{
    return GetDoc()->GetXmlIdRegistry();
}

bool SwSectionFmt::IsInClipboard() const
{
    return GetDoc()->IsClipBoard();
}

bool SwSectionFmt::IsInUndo() const
{
    return !IsInNodesArr();
}

bool SwSectionFmt::IsInContent() const
{
    SwNodeIndex const*const pIdx = GetCntnt(sal_False).GetCntntIdx();
    OSL_ENSURE(pIdx, "SwSectionFmt::IsInContent: no index?");
    return (pIdx) ? !GetDoc()->IsInHeaderFooter(*pIdx) : true;
}

// n.b.: if the section format represents an index, then there is both a
// SwXDocumentIndex and a SwXTextSection instance for this single core object.
// these two can both implement XMetadatable and forward to the same core
// section format.  but here only one UNO object can be returned,
// so always return the text section.
uno::Reference< rdf::XMetadatable >
SwSectionFmt::MakeUnoObject()
{
    uno::Reference<rdf::XMetadatable> xMeta;
    SwSection *const pSection( GetSection() );
    if (pSection)
    {
        xMeta.set(  SwXTextSection::CreateXTextSection(this,
                        TOX_HEADER_SECTION == pSection->GetType()),
                    uno::UNO_QUERY );
    }
    return xMeta;
}


// Method to break section links inside a linked section
static void lcl_BreakSectionLinksInSect( const SwSectionNode& rSectNd )
{
    if ( !rSectNd.GetDoc() )
    {
        OSL_FAIL( "method <lcl_RemoveSectionLinksInSect(..)> - no Doc at SectionNode" );
        return;
    }

    if ( !rSectNd.GetSection().IsConnected() )
    {
        OSL_FAIL( "method <lcl_RemoveSectionLinksInSect(..)> - no Link at Section of SectionNode" );
        return;
    }
    const ::sfx2::SvBaseLink* pOwnLink( &(rSectNd.GetSection().GetBaseLink() ) );
    const ::sfx2::SvBaseLinks& rLnks = rSectNd.GetDoc()->GetLinkManager().GetLinks();
    for ( sal_uInt16 n = rLnks.size(); n > 0; )
    {
        SwIntrnlSectRefLink* pSectLnk = dynamic_cast<SwIntrnlSectRefLink*>(&(*rLnks[ --n ]));
        if ( pSectLnk && pSectLnk != pOwnLink &&
             pSectLnk->IsInRange( rSectNd.GetIndex(), rSectNd.EndOfSectionIndex() ) )
        {
            // break the link of the corresponding section.
            // the link is also removed from the link manager
            pSectLnk->GetSectNode()->GetSection().BreakLink();

            // for robustness, because link is removed from the link manager
            if ( n > rLnks.size() )
            {
                n = rLnks.size();
            }
        }
    }
}

static void lcl_UpdateLinksInSect( SwBaseLink& rUpdLnk, SwSectionNode& rSectNd )
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
    for( sal_uInt16 n = rLnks.size(); n; )
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
                if( n >= rLnks.size() && 0 != ( n = rLnks.size() ))
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
//  2 - DocShell wurde neu angelegt, muss also wieder geschlossen werden ( will be assigned to xLockRef additionaly )

int sw_FindDocShell( SfxObjectShellRef& xDocSh,
                        SfxObjectShellLock& xLockRef,
                        const String& rFileName,
                        const String& rPasswd,
                        String& rFilter,
                        sal_Int16 nVersion,
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
    sal_Bool bFirst = 0 != pShell;

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
                                            SID_VERSION, sal_False, &pItem ) )
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
            bFirst = sal_False;
            pShell = SfxObjectShell::GetFirst( &aType );
        }
        else
            pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }

    // 2. selbst die Date oeffnen
    SfxMedium* pMed = new SfxMedium( aTmpObj.GetMainURL(
                             INetURLObject::NO_DECODE ), STREAM_READ );
    if( INET_PROT_FILE == aTmpObj.GetProtocol() )
        pMed->DownLoad();     // nur mal das Medium anfassen (DownLoaden)

    const SfxFilter* pSfxFlt = 0;
    if( !pMed->GetError() )
    {
        String sFactory(rtl::OUString::createFromAscii(SwDocShell::Factory().GetShortName()));
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
            aMatcher.DetectFilter( *pMed, &pSfxFlt, sal_False, sal_False );

        if( pSfxFlt )
        {
            // ohne Filter geht gar nichts
            pMed->SetFilter( pSfxFlt );

            // if the new shell is created, SfxObjectShellLock should be used to let it be closed later for sure
            xLockRef = new SwDocShell( SFX_CREATE_MODE_INTERNAL );
            xDocSh = (SfxObjectShell*)xLockRef;
            if( xDocSh->DoLoad( pMed ) )
                return 2;
        }
    }

    if( !xDocSh.Is() )      // Medium muss noch geloescht werden
        delete pMed;

    return 0;   // das war wohl nichts
}



::sfx2::SvBaseLink::UpdateResult SwIntrnlSectRefLink::DataChanged(
    const String& rMimeType, const uno::Any & rValue )
{
    SwSectionNode* pSectNd = rSectFmt.GetSectionNode( sal_False );
    SwDoc* pDoc = rSectFmt.GetDoc();

    sal_uLong nDataFormat = SotExchange::GetFormatIdFromMimeType( rMimeType );

    if( !pSectNd || !pDoc || pDoc->IsInDtor() || ChkNoDataFlag() ||
        sfx2::LinkManager::RegisterStatusInfoId() == nDataFormat )
    {
        // sollten wir schon wieder im Undo stehen?
        return SUCCESS;
    }

    //  #i38810# - Due to possible existing signatures, the
    // document has to be modified after updating a link.
    pDoc->SetModified();
    // set additional flag that links have been updated, in order to check this
    // during load.
    pDoc->SetLinksUpdated( sal_True );

    // Undo immer abschalten
    bool const bWasUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
    pDoc->GetIDocumentUndoRedo().DoUndo(false);
    sal_Bool bWasVisibleLinks = pDoc->IsVisibleLinks();
    pDoc->SetVisibleLinks( sal_False );

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
        pDoc->CorrAbs( aIdx, aEndIdx, aPos, sal_True );

        pPam = new SwPaM( aPos );

        //und alles dahinter liegende loeschen
        aIdx--;
        DelFlyInRange( aIdx, aEndIdx );
        _DelBookmarks(aIdx, aEndIdx);
        ++aIdx;

        pDoc->GetNodes().Delete( aIdx, aEndIdx.GetIndex() - aIdx.GetIndex() );
    }

    SwSection& rSection = pSectNd->GetSection();
    rSection.SetConnectFlag(false);

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
            SfxObjectShellLock xLockRef;
            int nRet;
            if( !sFileName.Len() )
            {
                xDocSh = pDoc->GetDocShell();
                nRet = 1;
            }
            else
            {
                nRet = sw_FindDocShell( xDocSh, xLockRef, sFileName,
                                    rSection.GetLinkFilePassword(),
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
                rSection.SetConnectFlag(true);

                SwNodeIndex aSave( pPam->GetPoint()->nNode, -1 );
                SwNodeRange* pCpyRg = 0;

                if( xDocSh->GetMedium() &&
                    !rSection.GetLinkFilePassword().Len() )
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == xDocSh->GetMedium()->GetItemSet()->
                        GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                        rSection.SetLinkFilePassword(
                                ((SfxStringItem*)pItem)->GetValue() );
                }

                SwDoc* pSrcDoc = ((SwDocShell*)&xDocSh)->GetDoc();

                if( sRange.Len() )
                {
                    // Rekursionen abfangen
                    sal_Bool bRecursion = sal_False;
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

                // #i81653#
                // Update links of extern linked document or extern linked
                // document section, if section is protected.
                if ( pSrcDoc != pDoc &&
                     rSection.IsProtectFlag() )
                {
                    pSrcDoc->GetLinkManager().UpdateAllLinks( sal_False, sal_True, sal_False, 0 );
                }

                if( pCpyRg )
                {
                    SwNodeIndex& rInsPos = pPam->GetPoint()->nNode;
                    sal_Bool bCreateFrm = rInsPos.GetIndex() <=
                                pDoc->GetNodes().GetEndOfExtras().GetIndex() ||
                                rInsPos.GetNode().FindTableNode();

                    SwTblNumFmtMerge aTNFM( *pSrcDoc, *pDoc );

                    pSrcDoc->CopyWithFlyInFly( *pCpyRg, 0, rInsPos, bCreateFrm );
                    ++aSave;

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

                        pDoc->CorrAbs( aSave, *pPam->GetPoint(), 0, sal_True );
                        pDoc->GetNodes().Delete( aSave, 1 );
                    }
                    delete pCpyRg;
                }

                lcl_BreakSectionLinksInSect( *pSectNd );

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

        // TODO/MBA: it's impossible to set a BaseURL here!
        SwReader aTmpReader( aStrm, aEmptyStr, pDoc->GetDocShell()->GetMedium()->GetBaseURL(), *pPam );

        if( !IsError( aTmpReader.Read( *pRead ) ))
        {
            rSection.SetConnectFlag(true);
        }

        if( pESh )
        {
            pESh->Pop( sal_False );
            pPam = 0;                   // pam is deleted before
        }
    }


    // remove all undo actions and turn undo on again
    pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
    pDoc->GetIDocumentUndoRedo().DoUndo(bWasUndo);
    pDoc->SetVisibleLinks( bWasVisibleLinks );

    pDoc->UnlockExpFlds();
    if( !pDoc->IsExpFldsLocked() )
        pDoc->UpdateExpFlds(NULL, true);

    if( pESh )
        pESh->EndAllAction();
    else if( pVSh )
        pVSh->EndAction();
    delete pPam;            // wurde am Anfang angelegt

    return SUCCESS;
}


void SwIntrnlSectRefLink::Closed()
{
    SwDoc* pDoc = rSectFmt.GetDoc();
    if( pDoc && !pDoc->IsInDtor() )
    {
        // Advise verabschiedet sich, den Bereich als nicht geschuetzt
        // kennzeichnen und das Flag umsetzen

        const SwSectionFmts& rFmts = pDoc->GetSections();
        for( sal_uInt16 n = rFmts.size(); n; )
            if( rFmts[ --n ] == &rSectFmt )
            {
                ViewShell* pSh;
                SwEditShell* pESh = pDoc->GetEditShell( &pSh );

                if( pESh )
                    pESh->StartAllAction();
                else
                    pSh->StartAction();

                SwSectionData aSectionData(*rSectFmt.GetSection());
                aSectionData.SetType( CONTENT_SECTION );
                aSectionData.SetLinkFileName( aEmptyStr );
                aSectionData.SetHidden( false );
                aSectionData.SetProtectFlag( false );
                // edit in readonly sections
                aSectionData.SetEditInReadonlyFlag( false );

                aSectionData.SetConnectFlag( false );

                pDoc->UpdateSection( n, aSectionData );

                // alle in der Section liegenden Links werden sichtbar
                SwSectionNode* pSectNd = rSectFmt.GetSectionNode( sal_False );
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
    OSL_ENSURE(pFmt, "SwSection::CreateLink: no format?");
    if (!pFmt || (CONTENT_SECTION == m_Data.GetType()))
        return ;

    sal_uInt16 nUpdateType = sfx2::LINKUPDATE_ALWAYS;

    if (!m_RefLink.Is())
    {
        // create BaseLink
        m_RefLink = new SwIntrnlSectRefLink( *pFmt, nUpdateType, FORMAT_RTF );
    }
    else
    {
        pFmt->GetDoc()->GetLinkManager().Remove( m_RefLink );
    }

    SwIntrnlSectRefLink *const pLnk =
        static_cast<SwIntrnlSectRefLink*>(& m_RefLink);

    String sCmd( m_Data.GetLinkFileName() );
    xub_StrLen nPos;
    while( STRING_NOTFOUND != (nPos = sCmd.SearchAscii( "  " )) )
        sCmd.Erase( nPos, 1 );

    pLnk->SetUpdateMode( nUpdateType );
    pLnk->SetVisible( pFmt->GetDoc()->IsVisibleLinks() );

    switch (m_Data.GetType())
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
                                static_cast<sal_uInt16>(m_Data.GetType()),
                                sCmd.GetToken( 0, sfx2::cTokenSeperator ),
                                ( sFltr.Len() ? &sFltr : 0 ),
                                ( sRange.Len() ? &sRange : 0 ) );
        }
        break;
    default:
        OSL_ENSURE( !this, "Was ist das fuer ein Link?" );
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
    if (m_RefLink.Is())
    {
        SwSectionFmt *const pFormat( GetFmt() );
        OSL_ENSURE(pFormat, "SwSection::BreakLink: no format?");
        if (pFormat)
        {
            pFormat->GetDoc()->GetLinkManager().Remove( m_RefLink );
        }
        m_RefLink.Clear();
    }
    // change type
    SetType( CONTENT_SECTION );
    // reset linked file data
    SetLinkFileName( aEmptyStr );
    SetLinkFilePassword( aEmptyStr );
}

const SwNode* SwIntrnlSectRefLink::GetAnchor() const
{
    return rSectFmt.GetSectionNode( sal_False );
}


sal_Bool SwIntrnlSectRefLink::IsInRange( sal_uLong nSttNd, sal_uLong nEndNd,
                                     xub_StrLen , xub_StrLen ) const
{
    SwStartNode* pSttNd = rSectFmt.GetSectionNode( sal_False );
    return pSttNd &&
            nSttNd < pSttNd->GetIndex() &&
            pSttNd->EndOfSectionIndex() < nEndNd;
}

sal_uInt16 SwSectionFmts::GetPos(const SwSectionFmt* p) const
{
    const_iterator it = std::find(begin(), end(), p);
    return it == end() ? USHRT_MAX : it - begin();
}

bool SwSectionFmts::Contains(const SwSectionFmt* p) const
{
    return std::find(begin(), end(), p) != end();
}

SwSectionFmts::~SwSectionFmts()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
