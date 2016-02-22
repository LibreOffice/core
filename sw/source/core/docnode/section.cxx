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
#include <DocumentLinksAdministrationManager.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <calbck.hxx>
#include <svl/smplhint.hxx>
#include <algorithm>
#include <ndsect.hxx>

using namespace ::com::sun::star;

class SwIntrnlSectRefLink : public SwBaseLink
{
    SwSectionFormat& rSectFormat;
public:
    SwIntrnlSectRefLink( SwSectionFormat& rFormat, SfxLinkUpdateMode nUpdateType, SotClipboardFormatId nFormat )
        : SwBaseLink( nUpdateType, nFormat ),
        rSectFormat( rFormat )
    {}

    virtual void Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    virtual const SwNode* GetAnchor() const override;
    virtual bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, sal_Int32 nStt = 0,
                            sal_Int32 nEnd = -1 ) const override;

    inline SwSectionNode* GetSectNode()
    {
        const SwNode* pSectNd( GetAnchor() );
        return const_cast<SwSectionNode*>( dynamic_cast<const SwSectionNode*>( pSectNd ) );
    }
};


SwSectionData::SwSectionData(SectionType const eType, OUString const& rName)
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

OUString SwSectionData::CollapseWhiteSpaces(const OUString& sName)
{
    const sal_Int32 nLen = sName.getLength();
    const sal_Unicode cRef = ' ';
    OUStringBuffer aBuf(nLen+1);
    for (sal_Int32 i = 0; i<nLen; )
    {
        const sal_Unicode cCur = sName[i++];
        aBuf.append(cCur);
        if (cCur!=cRef)
            continue;
        while (i<nLen && sName[i]==cRef)
            ++i;
    }
    return aBuf.makeStringAndClear();
}

SwSection::SwSection(
        SectionType const eType, OUString const& rName, SwSectionFormat & rFormat)
    : SwClient(& rFormat)
    , m_Data(eType, rName)
{
    SwSection *const pParentSect = GetParent();
    if( pParentSect )
    {
        if( pParentSect->IsHiddenFlag() )
        {
            SetHidden();
        }

        m_Data.SetProtectFlag( pParentSect->IsProtectFlag() );
        // edit in readonly sections
        m_Data.SetEditInReadonlyFlag( pParentSect->IsEditInReadonlyFlag() );
    }

    if (!m_Data.IsProtectFlag())
    {
        m_Data.SetProtectFlag( rFormat.GetProtect().IsContentProtected() );
    }

    if (!m_Data.IsEditInReadonlyFlag()) // edit in readonly sections
    {
        m_Data.SetEditInReadonlyFlag( rFormat.GetEditInReadonly().GetValue() );
    }
}

SwSection::~SwSection()
{
    SwSectionFormat* pFormat = GetFormat();
    if( !pFormat )
        return;

    SwDoc* pDoc = pFormat->GetDoc();
    if( pDoc->IsInDtor() )
    {
        // We reattach our Format to the default FrameFormat
        // to not get any dependencies
        if( pFormat->DerivedFrom() != pDoc->GetDfltFrameFormat() )
            pFormat->RegisterToFormat( *pDoc->GetDfltFrameFormat() );
    }
    else
    {
        pFormat->Remove( this ); // remove

        if (CONTENT_SECTION != m_Data.GetType())
        {
            pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( m_RefLink );
        }

        if (m_RefObj.Is())
        {
            pDoc->getIDocumentLinksAdministration().GetLinkManager().RemoveServer( &m_RefObj );
        }

        // If the Section is the last Client in the Format we can delete it
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFormat );
        pFormat->ModifyNotification( &aMsgHint, &aMsgHint );
        if( !pFormat->HasWriterListeners() )
        {
            // Do not add to the Undo. This should've happened earlier.
            ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
            pDoc->DelSectionFormat( pFormat );
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
    // The next two may actually overwrite the m_Data.m_bProtect or EditInReadonly Flag
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
    m_Data.SetProtectFlag(IsProtect());
    m_Data.SetEditInReadonlyFlag(IsEditInReadonly());
    bool const bResult( m_Data == rCmp );
    m_Data.SetProtectFlag(bProtect);
    m_Data.SetEditInReadonlyFlag(bEditInReadonly);
    return bResult;
}

void SwSection::ImplSetHiddenFlag(bool const bTmpHidden, bool const bCondition)
{
    SwSectionFormat* pFormat = GetFormat();
    OSL_ENSURE(pFormat, "ImplSetHiddenFlag: no format?");
    if( pFormat )
    {
        const bool bHide = bTmpHidden && bCondition;

        if (bHide) // should be hidden
        {
            if (!m_Data.IsHiddenFlag()) // is not hidden
            {
                // Is the Parent hidden?
                // This should be shown by the bHiddenFlag.

                // Tell all Children that they are hidden
                SwMsgPoolItem aMsgItem( RES_SECTION_HIDDEN );
                pFormat->ModifyNotification( &aMsgItem, &aMsgItem );

                // Delete all Frames
                pFormat->DelFrames();
            }
        }
        else if (m_Data.IsHiddenFlag()) // show Nodes again
        {
            // Show all Frames (Child Sections are accounted for by MakeFrames)
            // Only if the Parent Section is not restricting us!
            SwSection* pParentSect = pFormat->GetParentSection();
            if( !pParentSect || !pParentSect->IsHiddenFlag() )
            {
                // Tell all Children that the Parent is not hidden anymore
                SwMsgPoolItem aMsgItem( RES_SECTION_NOT_HIDDEN );
                pFormat->ModifyNotification( &aMsgItem, &aMsgItem );

                pFormat->MakeFrames();
            }
        }
    }
}

bool SwSection::CalcHiddenFlag() const
{
    const SwSection* pSect = this;
    do {
        if( pSect->IsHidden() && pSect->IsCondHidden() )
            return true;
    } while( nullptr != ( pSect = pSect->GetParent()) );

    return false;
}

bool SwSection::IsProtect() const
{
    SwSectionFormat const *const pFormat( GetFormat() );
    OSL_ENSURE(pFormat, "SwSection::IsProtect: no format?");
    return (pFormat)
        ?   pFormat->GetProtect().IsContentProtected()
        :   IsProtectFlag();
}

// edit in readonly sections
bool SwSection::IsEditInReadonly() const
{
    SwSectionFormat const *const pFormat( GetFormat() );
    OSL_ENSURE(pFormat, "SwSection::IsEditInReadonly: no format?");
    return (pFormat)
        ?   pFormat->GetEditInReadonly().GetValue()
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
    SwSectionFormat *const pFormat( GetFormat() );
    OSL_ENSURE(pFormat, "SwSection::SetProtect: no format?");
    if (pFormat)
    {
        SvxProtectItem aItem( RES_PROTECT );
        aItem.SetContentProtect( bFlag );
        pFormat->SetFormatAttr( aItem );
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
    SwSectionFormat *const pFormat( GetFormat() );
    OSL_ENSURE(pFormat, "SwSection::SetEditInReadonly: no format?");
    if (pFormat)
    {
        SwFormatEditInReadonly aItem;
        aItem.SetValue( bFlag );
        pFormat->SetFormatAttr( aItem );
        // note: this will call m_Data.SetEditInReadonlyFlag via Modify!
    }
    else
    {
        m_Data.SetEditInReadonlyFlag(bFlag);
    }
}

void SwSection::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    bool bUpdateFootnote = false;
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
        if (pNew && pOld)
        {
            SfxItemSet* pNewSet = const_cast<SwAttrSetChg*>(static_cast<const SwAttrSetChg*>(pNew))->GetChgSet();
            SfxItemSet* pOldSet = const_cast<SwAttrSetChg*>(static_cast<const SwAttrSetChg*>(pOld))->GetChgSet();
            const SfxPoolItem* pItem;

            if( SfxItemState::SET == pNewSet->GetItemState(
                        RES_PROTECT, false, &pItem ) )
            {
                m_Data.SetProtectFlag( static_cast<SvxProtectItem const*>(pItem)
                        ->IsContentProtected() );
                pNewSet->ClearItem( RES_PROTECT );
                pOldSet->ClearItem( RES_PROTECT );
            }

            // --> edit in readonly sections
            if( SfxItemState::SET == pNewSet->GetItemState(
                        RES_EDIT_IN_READONLY, false, &pItem ) )
            {
                m_Data.SetEditInReadonlyFlag(
                    static_cast<SwFormatEditInReadonly const*>(pItem)->GetValue());
                pNewSet->ClearItem( RES_EDIT_IN_READONLY );
                pOldSet->ClearItem( RES_EDIT_IN_READONLY );
            }

            if( SfxItemState::SET == pNewSet->GetItemState(
                        RES_FTN_AT_TXTEND, false, &pItem ) ||
                SfxItemState::SET == pNewSet->GetItemState(
                        RES_END_AT_TXTEND, false, &pItem ))
            {
                    bUpdateFootnote = true;
            }

            if( !pNewSet->Count() )
                return;
        }
        break;

    case RES_PROTECT:
        if( pNew )
        {
            bool bNewFlag =
                static_cast<const SvxProtectItem*>(pNew)->IsContentProtected();
            if( !bNewFlag )
            {
                // Switching off: See if there is protection transferred
                // by the Parents
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
                static_cast<const SwFormatEditInReadonly*>(pNew)->GetValue();
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
        // Is handled by the Layout, if appropriate
        break;

    case RES_FTN_AT_TXTEND:
        if( pNew && pOld )
        {
            bUpdateFootnote = true;
        }
        break;

    case RES_END_AT_TXTEND:
        if( pNew && pOld )
        {
            bUpdateFootnote = true;
        }
        break;

    default:
        CheckRegistration( pOld, pNew );
        break;
    }

    if( bUpdateFootnote )
    {
        SwSectionNode* pSectNd = GetFormat()->GetSectionNode();
        if( pSectNd )
            pSectNd->GetDoc()->GetFootnoteIdxs().UpdateFootnote(SwNodeIndex( *pSectNd ));
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

// Set/remove the linked FileName
OUString SwSection::GetLinkFileName() const
{
    if (m_RefLink.Is())
    {
        OUString sTmp;
        switch (m_Data.GetType())
        {
        case DDE_LINK_SECTION:
            sTmp = m_RefLink->GetLinkSourceName();
            break;

        case FILE_LINK_SECTION:
            {
                OUString sRange;
                OUString sFilter;
                if (m_RefLink->GetLinkManager() &&
                    sfx2::LinkManager::GetDisplayNames(
                        m_RefLink, nullptr, &sTmp, &sRange, &sFilter ))
                {
                    sTmp += OUString(sfx2::cTokenSeparator) + sFilter
                         +  OUString(sfx2::cTokenSeparator) + sRange;
                }
                else if( GetFormat() && !GetFormat()->GetSectionNode() )
                {
                    // If the Section is in the UndoNodesArray, the LinkManager
                    // does not contain the Link, thus it cannot be queried for it.
                    // Thus return the current Name.
                    return m_Data.GetLinkFileName();
                }
            }
            break;
        default: break;
        }
        m_Data.SetLinkFileName(sTmp);
    }
    return m_Data.GetLinkFileName();
}

void SwSection::SetLinkFileName(const OUString& rNew, OUString const*const pPassWd)
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

// If it was a Linked Section, we need to make all Child Links visible
void SwSection::MakeChildLinksVisible( const SwSectionNode& rSectNd )
{
    const SwNode* pNd;
    const ::sfx2::SvBaseLinks& rLnks = rSectNd.GetDoc()->getIDocumentLinksAdministration().GetLinkManager().GetLinks();
    for( auto n = rLnks.size(); n; )
    {
        ::sfx2::SvBaseLink* pBLnk = &(*rLnks[ --n ]);
        if( pBLnk && !pBLnk->IsVisible() &&
            dynamic_cast< const SwBaseLink *>( pBLnk ) !=  nullptr &&
            nullptr != ( pNd = static_cast<SwBaseLink*>(pBLnk)->GetAnchor() ) )
        {
            pNd = pNd->StartOfSectionNode(); // If it's a SectionNode
            const SwSectionNode* pParent;
            while( nullptr != ( pParent = pNd->FindSectionNode() ) &&
                    ( CONTENT_SECTION == pParent->GetSection().GetType()
                        || pNd == &rSectNd ))
                    pNd = pParent->StartOfSectionNode();

            // It's within a normal Section, so show again
            if( !pParent )
                pBLnk->SetVisible( true );
        }
    }
}

const SwTOXBase* SwSection::GetTOXBase() const
{
    const SwTOXBase* pRet = nullptr;
    if( TOX_CONTENT_SECTION == GetType() )
        pRet = dynamic_cast<const SwTOXBaseSection*>(this);
    return pRet;
}

SwSectionFormat::SwSectionFormat( SwFrameFormat* pDrvdFrame, SwDoc *pDoc )
    : SwFrameFormat( pDoc->GetAttrPool(), OUString(), pDrvdFrame )
{
    LockModify();
    SetFormatAttr( *GetDfltAttr( RES_COL ) );
    UnlockModify();
}

SwSectionFormat::~SwSectionFormat()
{
    if( !GetDoc()->IsInDtor() )
    {
        SwSectionNode* pSectNd;
        const SwNodeIndex* pIdx = GetContent( false ).GetContentIdx();
        if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
            nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            SwSection& rSect = pSectNd->GetSection();
            // If it was a linked Section, we need to make all Child Links
            // visible again
            if( rSect.IsConnected() )
                SwSection::MakeChildLinksVisible( *pSectNd );

            // Check whether we need to be visible, before deleting the Nodes
            if( rSect.IsHiddenFlag() )
            {
                SwSection* pParentSect = rSect.GetParent();
                if( !pParentSect || !pParentSect->IsHiddenFlag() )
                {
                    // Make Nodes visible again
                    rSect.SetHidden(false);
                }
            }
            // mba: test iteration; objects are removed while iterating
            // use hint which allows to specify, if the content shall be saved or not
            CallSwClientNotify( SwSectionFrameMoveAndDeleteHint( true ) );

            // Raise the Section up
            SwNodeRange aRg( *pSectNd, 0, *pSectNd->EndOfSectionNode() );
            GetDoc()->GetNodes().SectionUp( &aRg );
        }
        LockModify();
        ResetFormatAttr( RES_CNTNT );
        UnlockModify();
    }
}

SwSection * SwSectionFormat::GetSection() const
{
    return SwIterator<SwSection,SwSectionFormat>( *this ).First();
}

// Do not destroy all Frames in aDepend (Frames are recognized with a dynamic_cast).
void SwSectionFormat::DelFrames()
{
    SwSectionNode* pSectNd;
    const SwNodeIndex* pIdx = GetContent(false).GetContentIdx();
    if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
        nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
    {
        // First delete the <SwSectionFrame> of the <SwSectionFormat> instance
        // mba: test iteration as objects are removed in iteration
        // use hint which allows to specify, if the content shall be saved or not
        CallSwClientNotify( SwSectionFrameMoveAndDeleteHint( false ) );

        // Then delete frames of the nested <SwSectionFormat> instances
        SwIterator<SwSectionFormat,SwSectionFormat> aIter( *this );
        SwSectionFormat *pLast = aIter.First();
        while ( pLast )
        {
            pLast->DelFrames();
            pLast = aIter.Next();
        }

        sal_uLong nEnde = pSectNd->EndOfSectionIndex();
        sal_uLong nStart = pSectNd->GetIndex()+1;
        sw_DeleteFootnote( pSectNd, nStart, nEnde );
    }
    if( pIdx )
    {
        // Send Hint for PageDesc. Actually the Layout contained in the
        // Paste of the Frame itself would need to do this. But that leads
        // to subsequent errors, which we'd need to solve at run-time.
        SwNodeIndex aNextNd( *pIdx );
        SwContentNode* pCNd = GetDoc()->GetNodes().GoNextSection( &aNextNd, true, false );
        if( pCNd )
        {
            const SfxPoolItem& rItem = pCNd->GetSwAttrSet().Get( RES_PAGEDESC );
            pCNd->ModifyNotification( &rItem, &rItem );
        }
    }
}

// Create the Views
void SwSectionFormat::MakeFrames()
{
    SwSectionNode* pSectNd;
    const SwNodeIndex* pIdx = GetContent(false).GetContentIdx();

    if( pIdx && &GetDoc()->GetNodes() == &pIdx->GetNodes() &&
        nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
    {
        SwNodeIndex aIdx( *pIdx );
        pSectNd->MakeFrames( &aIdx );
    }
}

void SwSectionFormat::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    bool bClients = false;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
    case RES_ATTRSET_CHG:
        if (HasWriterListeners() && pOld && pNew)
        {
            SfxItemSet* pNewSet = const_cast<SwAttrSetChg*>(static_cast<const SwAttrSetChg*>(pNew))->GetChgSet();
            SfxItemSet* pOldSet = const_cast<SwAttrSetChg*>(static_cast<const SwAttrSetChg*>(pOld))->GetChgSet();
            const SfxPoolItem *pItem;
            if( SfxItemState::SET == pNewSet->GetItemState(
                                        RES_PROTECT, false, &pItem ))
            {
                ModifyBroadcast( pItem, pItem );
                pNewSet->ClearItem( RES_PROTECT );
                pOldSet->ClearItem( RES_PROTECT );
            }

            // --> edit in readonly sections
            if( SfxItemState::SET == pNewSet->GetItemState(
                        RES_EDIT_IN_READONLY, false, &pItem ) )
            {
                ModifyBroadcast( pItem, pItem );
                pNewSet->ClearItem( RES_EDIT_IN_READONLY );
                pOldSet->ClearItem( RES_EDIT_IN_READONLY );
            }

            if( SfxItemState::SET == pNewSet->GetItemState(
                                    RES_FTN_AT_TXTEND, false, &pItem ))
            {
                ModifyBroadcast( &pOldSet->Get( RES_FTN_AT_TXTEND ), pItem );
                pNewSet->ClearItem( RES_FTN_AT_TXTEND );
                pOldSet->ClearItem( RES_FTN_AT_TXTEND );
            }
            if( SfxItemState::SET == pNewSet->GetItemState(
                                    RES_END_AT_TXTEND, false, &pItem ))
            {
                ModifyBroadcast( &pOldSet->Get( RES_END_AT_TXTEND ), pItem );
                pNewSet->ClearItem( RES_END_AT_TXTEND );
                pOldSet->ClearItem( RES_END_AT_TXTEND );
            }
            if( !static_cast<const SwAttrSetChg*>(pOld)->GetChgSet()->Count() )
                return;
        }
        break;

    case RES_SECTION_RESETHIDDENFLAG:
    case RES_FTN_AT_TXTEND:
    case RES_END_AT_TXTEND : bClients = true;
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
        // Pass through these Messages until the End of the tree!
        if( HasWriterListeners() )
        {
            ModifyBroadcast( pOld, pNew );
        }
        return; // That's it!

    case RES_OBJECTDYING:
        if( !GetDoc()->IsInDtor() && pOld &&
            static_cast<const SwPtrMsgPoolItem *>(pOld)->pObject == static_cast<void*>(GetRegisteredIn()) )
        {
            // My Parents will be destroyed, so get the Parent's Parent
            // and update
            SwFrameFormat::Modify( pOld, pNew ); // Rewire first!
            UpdateParent();
            return;
        }
        break;

    case RES_FMT_CHG:
        if( !GetDoc()->IsInDtor() &&
            static_cast<const SwFormatChg*>(pNew)->pChangedFormat == static_cast<void*>(GetRegisteredIn()) &&
            dynamic_cast<const SwSectionFormat*>(static_cast<const SwFormatChg*>(pNew)->pChangedFormat) != nullptr )
        {
            // My Parent will be changed, thus I need to update
            SwFrameFormat::Modify( pOld, pNew ); // Rewire first!
            UpdateParent();
            return;
        }
        break;
    }
    SwFrameFormat::Modify( pOld, pNew );

    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXTextSection(uno::Reference<text::XTextSection>(nullptr));
    }
}

// Get info from the Format
bool SwSectionFormat::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_FINDNEARESTNODE:
        if( static_cast<const SwFormatPageDesc&>(GetFormatAttr( RES_PAGEDESC )).GetPageDesc() )
        {
            const SwSectionNode* pNd = GetSectionNode();
            if( pNd )
                static_cast<SwFindNearestNode&>(rInfo).CheckNode( *pNd );
        }
        return true;

    case RES_CONTENT_VISIBLE:
        {
            SwFrame* pFrame = SwIterator<SwFrame,SwFormat>(*this).First();
            // if the current section has no own frame search for the children
            if(!pFrame)
            {
                SwIterator<SwSectionFormat,SwSectionFormat> aFormatIter(*this);
                SwSectionFormat* pChild = aFormatIter.First();
                while(pChild && !pFrame)
                {
                    pFrame = SwIterator<SwFrame,SwFormat>(*pChild).First();
                    pChild = aFormatIter.Next();
                }
            }
            static_cast<SwPtrMsgPoolItem&>(rInfo).pObject = pFrame;
        }
        return false;
    }
    return SwModify::GetInfo( rInfo );
}

static bool lcl_SectionCmpPos( const SwSection *pFirst, const SwSection *pSecond)
{
    const SwSectionFormat* pFSectFormat = pFirst->GetFormat();
    const SwSectionFormat* pSSectFormat = pSecond->GetFormat();
    OSL_ENSURE( pFSectFormat && pSSectFormat &&
            pFSectFormat->GetContent(false).GetContentIdx() &&
            pSSectFormat->GetContent(false).GetContentIdx(),
                "ungueltige Sections" );
    return pFSectFormat->GetContent(false).GetContentIdx()->GetIndex() <
                  pSSectFormat->GetContent(false).GetContentIdx()->GetIndex();
}

static bool lcl_SectionCmpNm( const SwSection *pFSect, const SwSection *pSSect)
{
    OSL_ENSURE( pFSect && pSSect, "Invalid Sections" );
    return pFSect->GetSectionName() < pSSect->GetSectionName();
}

// get all Sections that have been derived from this one
void SwSectionFormat::GetChildSections( SwSections& rArr,
                                        SectionSort eSort,
                                        bool bAllSections ) const
{
    rArr.clear();

    if( HasWriterListeners() )
    {
        SwIterator<SwSectionFormat,SwSectionFormat> aIter(*this);
        const SwNodeIndex* pIdx;
        for( SwSectionFormat* pLast = aIter.First(); pLast; pLast = aIter.Next() )
            if( bAllSections ||
                ( nullptr != ( pIdx = pLast->GetContent(false).
                GetContentIdx()) && &pIdx->GetNodes() == &GetDoc()->GetNodes() ))
            {
                SwSection* pDummy = pLast->GetSection();
                rArr.push_back( pDummy );
            }

        // Do we need any sorting?
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
}

// See whether the Section is within the Nodes or the UndoNodes array
bool SwSectionFormat::IsInNodesArr() const
{
    const SwNodeIndex* pIdx = GetContent(false).GetContentIdx();
    return pIdx && &pIdx->GetNodes() == &GetDoc()->GetNodes();
}

// Parent was changed
void SwSectionFormat::UpdateParent()
{
    if( !HasWriterListeners() )
        return;

    SwSection* pSection = nullptr;
    const SvxProtectItem* pProtect(nullptr);
    // edit in readonly sections
    const SwFormatEditInReadonly* pEditInReadonly = nullptr;
    bool bIsHidden = false;

    SwIterator<SwClient,SwSectionFormat> aIter(*this);
    for(SwClient* pLast = aIter.First(); pLast; pLast = aIter.Next())
    {
        if( dynamic_cast<const SwSectionFormat*>(pLast) !=  nullptr )
        {
            if( !pSection )
            {
                pSection = GetSection();
                if( GetRegisteredIn() )
                {
                    const SwSection* pPS = GetParentSection();
                    pProtect = &pPS->GetFormat()->GetProtect();
                    // edit in readonly sections
                    pEditInReadonly = &pPS->GetFormat()->GetEditInReadonly();
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
            if (!pProtect->IsContentProtected() !=
                !pSection->IsProtectFlag())
            {
                pLast->ModifyNotification( static_cast<SfxPoolItem const *>(pProtect),
                                static_cast<SfxPoolItem const *>(pProtect) );
            }

            // edit in readonly sections
            if (!pEditInReadonly->GetValue() !=
                !pSection->IsEditInReadonlyFlag())
            {
                pLast->ModifyNotification( static_cast<SfxPoolItem const *>(pEditInReadonly),
                                static_cast<SfxPoolItem const *>(pEditInReadonly) );
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
                dynamic_cast<const SwSection*>(pLast) !=  nullptr )
        {
            pSection = static_cast<SwSection*>(pLast);
            if( GetRegisteredIn() )
            {
                const SwSection* pPS = GetParentSection();
                pProtect = &pPS->GetFormat()->GetProtect();
                // edit in readonly sections
                pEditInReadonly = &pPS->GetFormat()->GetEditInReadonly();
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
    }
}

SwSectionNode* SwSectionFormat::GetSectionNode(bool const bAlways)
{
    const SwNodeIndex* pIdx = GetContent(false).GetContentIdx();
    if( pIdx && ( bAlways || &pIdx->GetNodes() == &GetDoc()->GetNodes() ))
        return pIdx->GetNode().GetSectionNode();
    return nullptr;
}

// Is this Section valid for the GlobalDocument?
const SwSection* SwSectionFormat::GetGlobalDocSection() const
{
    const SwSectionNode* pNd = GetSectionNode();
    if( pNd &&
        ( FILE_LINK_SECTION == pNd->GetSection().GetType() ||
          TOX_CONTENT_SECTION == pNd->GetSection().GetType() ) &&
        pNd->GetIndex() > pNd->GetNodes().GetEndOfExtras().GetIndex() &&
        !pNd->StartOfSectionNode()->IsSectionNode() &&
        !pNd->StartOfSectionNode()->FindSectionNode() )
        return &pNd->GetSection();
    return nullptr;
}

// sw::Metadatable
::sfx2::IXmlIdRegistry& SwSectionFormat::GetRegistry()
{
    return GetDoc()->GetXmlIdRegistry();
}

bool SwSectionFormat::IsInClipboard() const
{
    return GetDoc()->IsClipBoard();
}

bool SwSectionFormat::IsInUndo() const
{
    return !IsInNodesArr();
}

bool SwSectionFormat::IsInContent() const
{
    SwNodeIndex const*const pIdx = GetContent(false).GetContentIdx();
    OSL_ENSURE(pIdx, "SwSectionFormat::IsInContent: no index?");
    return pIdx == nullptr || !GetDoc()->IsInHeaderFooter(*pIdx);
}

// n.b.: if the section format represents an index, then there is both a
// SwXDocumentIndex and a SwXTextSection instance for this single core object.
// these two can both implement XMetadatable and forward to the same core
// section format.  but here only one UNO object can be returned,
// so always return the text section.
uno::Reference< rdf::XMetadatable >
SwSectionFormat::MakeUnoObject()
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

void SwSectionFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swSectionFormat"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    GetAttrSet().dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

void SwSectionFormats::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swSectionFormats"));
    for (size_t i = 0; i < size(); ++i)
        GetFormat(i)->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
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
    const ::sfx2::SvBaseLinks& rLnks = rSectNd.GetDoc()->getIDocumentLinksAdministration().GetLinkManager().GetLinks();
    for ( auto n = rLnks.size(); n > 0; )
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

    const OUString sName( pDShell->GetMedium()->GetName() );
    const OUString sMimeType( SotExchange::GetFormatMimeType( SotClipboardFormatId::SIMPLE_FILE ));
    uno::Any aValue;
    aValue <<= sName; // Arbitrary name

    const ::sfx2::SvBaseLinks& rLnks = pDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks();
    for( auto n = rLnks.size(); n; )
    {
        SwBaseLink* pBLink;

        ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
        if( pLnk && pLnk != &rUpdLnk &&
            OBJECT_CLIENT_FILE == pLnk->GetObjType() &&
            dynamic_cast< const SwBaseLink *>( pLnk ) !=  nullptr &&
            ( pBLink = static_cast<SwBaseLink*>(pLnk) )->IsInRange( rSectNd.GetIndex(),
                                                rSectNd.EndOfSectionIndex() ) )
        {
            // It's in the Section, so update. But only if it's not in the same File!
            OUString sFName;
            sfx2::LinkManager::GetDisplayNames( pBLink, nullptr, &sFName );
            if( sFName != sName )
            {
                pBLink->DataChanged( sMimeType, aValue );

                // If needed find the Link pointer to avoid skipping one or calling one twice
                if( n >= rLnks.size() && 0 != ( n = rLnks.size() ))
                    --n;

                if( n && pLnk != &(*rLnks[ n ]) )
                {
                    // Find - it can only precede it!
                    while( n )
                        if( pLnk == &(*rLnks[ --n ] ) )
                            break;
                }
            }
        }
    }
}

::sfx2::SvBaseLink::UpdateResult SwIntrnlSectRefLink::DataChanged(
    const OUString& rMimeType, const uno::Any & rValue )
{
    SwSectionNode* pSectNd = rSectFormat.GetSectionNode();
    SwDoc* pDoc = rSectFormat.GetDoc();

    SotClipboardFormatId nDataFormat = SotExchange::GetFormatIdFromMimeType( rMimeType );

    if( !pSectNd || !pDoc || pDoc->IsInDtor() || ChkNoDataFlag() ||
        sfx2::LinkManager::RegisterStatusInfoId() == nDataFormat )
    {
        // Should we be in the Undo already?
        return SUCCESS;
    }

    //  #i38810# - Due to possible existing signatures, the
    // document has to be modified after updating a link.
    pDoc->getIDocumentState().SetModified();
    // set additional flag that links have been updated, in order to check this
    // during load.
    pDoc->getIDocumentLinksAdministration().SetLinksUpdated( true );

    // Always switch off Undo
    bool const bWasUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
    pDoc->GetIDocumentUndoRedo().DoUndo(false);
    bool bWasVisibleLinks = pDoc->getIDocumentLinksAdministration().IsVisibleLinks();
    pDoc->getIDocumentLinksAdministration().SetVisibleLinks( false );

    SwPaM* pPam;
    SwViewShell* pVSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    SwEditShell* pESh = pDoc->GetEditShell();
    pDoc->getIDocumentFieldsAccess().LockExpFields();
    {
        // Insert an empty TextNode at the Section's start
        SwNodeIndex aIdx( *pSectNd, +1 );
        SwNodeIndex aEndIdx( *pSectNd->EndOfSectionNode() );
        SwTextNode* pNewNd = pDoc->GetNodes().MakeTextNode( aIdx,
                        pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ) );

        if( pESh )
            pESh->StartAllAction();
        else if( pVSh )
            pVSh->StartAction();

        SwPosition aPos( aIdx, SwIndex( pNewNd, 0 ));
        --aPos.nNode;
        SwDoc::CorrAbs( aIdx, aEndIdx, aPos, true );

        pPam = new SwPaM( aPos );

        // Delete everything succeeding it
        --aIdx;
        DelFlyInRange( aIdx, aEndIdx );
        _DelBookmarks(aIdx, aEndIdx);
        ++aIdx;

        pDoc->GetNodes().Delete( aIdx, aEndIdx.GetIndex() - aIdx.GetIndex() );
    }

    SwSection& rSection = pSectNd->GetSection();
    rSection.SetConnectFlag(false);

    Reader* pRead = nullptr;
    switch( nDataFormat )
    {
    case SotClipboardFormatId::STRING:
        pRead = ReadAscii;
        break;

    case SotClipboardFormatId::RTF:
        pRead = SwReaderWriter::GetReader( READER_WRITER_RTF );
        break;

    case SotClipboardFormatId::SIMPLE_FILE:
        if ( rValue.hasValue() )
        {
            OUString sFileName;
            if ( !(rValue >>= sFileName) )
                break;
            OUString sFilter;
            OUString sRange;
            sfx2::LinkManager::GetDisplayNames( this, nullptr, &sFileName,
                                                    &sRange, &sFilter );

            RedlineMode_t eOldRedlineMode = nsRedlineMode_t::REDLINE_NONE;
            SfxObjectShellRef xDocSh;
            SfxObjectShellLock xLockRef;
            int nRet;
            if( sFileName.isEmpty() )
            {
                xDocSh = pDoc->GetDocShell();
                nRet = 1;
            }
            else
            {
                nRet = SwFindDocShell( xDocSh, xLockRef, sFileName,
                                    rSection.GetLinkFilePassword(),
                                    sFilter, 0, pDoc->GetDocShell() );
                if( nRet )
                {
                    SwDoc* pSrcDoc = static_cast<SwDocShell*>(&xDocSh)->GetDoc();
                    eOldRedlineMode = pSrcDoc->getIDocumentRedlineAccess().GetRedlineMode();
                    pSrcDoc->getIDocumentRedlineAccess().SetRedlineMode( nsRedlineMode_t::REDLINE_SHOW_INSERT );
                }
            }

            if( nRet )
            {
                rSection.SetConnectFlag();

                SwNodeIndex aSave( pPam->GetPoint()->nNode, -1 );
                SwNodeRange* pCpyRg = nullptr;

                if( xDocSh->GetMedium() &&
                    rSection.GetLinkFilePassword().isEmpty() )
                {
                    const SfxPoolItem* pItem;
                    if( SfxItemState::SET == xDocSh->GetMedium()->GetItemSet()->
                        GetItemState( SID_PASSWORD, false, &pItem ) )
                        rSection.SetLinkFilePassword(
                                static_cast<const SfxStringItem*>(pItem)->GetValue() );
                }

                SwDoc* pSrcDoc = static_cast<SwDocShell*>(&xDocSh)->GetDoc();

                if( !sRange.isEmpty() )
                {
                    // Catch recursion
                    bool bRecursion = false;
                    if( pSrcDoc == pDoc )
                    {
                        tools::SvRef<SwServerObject> refObj( static_cast<SwServerObject*>(
                                        pDoc->getIDocumentLinksAdministration().CreateLinkSource( sRange )));
                        if( refObj.Is() )
                        {
                            bRecursion = refObj->IsLinkInServer( this ) ||
                                        ChkNoDataFlag();
                        }
                    }

                    SwNodeIndex& rInsPos = pPam->GetPoint()->nNode;

                    SwPaM* pCpyPam = nullptr;
                    if( !bRecursion &&
                        pSrcDoc->GetDocumentLinksAdministrationManager().SelectServerObj( sRange, pCpyPam, pCpyRg )
                        && pCpyPam )
                    {
                        if( pSrcDoc != pDoc ||
                            pCpyPam->Start()->nNode > rInsPos ||
                            rInsPos >= pCpyPam->End()->nNode )
                        {
                            pSrcDoc->getIDocumentContentOperations().CopyRange( *pCpyPam, *pPam->GetPoint(), /*bCopyAll=*/false, /*bCheckPos=*/true );
                        }
                        delete pCpyPam;
                    }
                    if( pCpyRg && pSrcDoc == pDoc &&
                        pCpyRg->aStart < rInsPos && rInsPos < pCpyRg->aEnd )
                    {
                        delete pCpyRg;
                        pCpyRg = nullptr;
                    }
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
                    pSrcDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks( false );
                }

                if( pCpyRg )
                {
                    SwNodeIndex& rInsPos = pPam->GetPoint()->nNode;
                    bool bCreateFrame = rInsPos.GetIndex() <=
                                pDoc->GetNodes().GetEndOfExtras().GetIndex() ||
                                rInsPos.GetNode().FindTableNode();

                    SwTableNumFormatMerge aTNFM( *pSrcDoc, *pDoc );

                    pSrcDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly( *pCpyRg, 0, rInsPos, nullptr, bCreateFrame );
                    ++aSave;

                    if( !bCreateFrame )
                        ::MakeFrames( pDoc, aSave, rInsPos );

                    // Delete last Node, only if it was copied successfully
                    // (the Section contains more than one Node)
                    if( 2 < pSectNd->EndOfSectionIndex() - pSectNd->GetIndex() )
                    {
                        aSave = rInsPos;
                        pPam->Move( fnMoveBackward, fnGoNode );
                        pPam->SetMark(); // Rewire both SwPositions

                        pDoc->CorrAbs( aSave, *pPam->GetPoint(), 0, true );
                        pDoc->GetNodes().Delete( aSave );
                    }
                    delete pCpyRg;
                }

                lcl_BreakSectionLinksInSect( *pSectNd );

                // Update all Links in this Section
                lcl_UpdateLinksInSect( *this, *pSectNd );
            }
            if( xDocSh.Is() )
            {
                if( 2 == nRet )
                    xDocSh->DoClose();
                else if( static_cast<SwDocShell*>(&xDocSh)->GetDoc() )
                    static_cast<SwDocShell*>(&xDocSh)->GetDoc()->getIDocumentRedlineAccess().SetRedlineMode(
                                eOldRedlineMode );
            }
        }
        break;
    default: break;
    }

    // Only create DDE if Shell is available!
    uno::Sequence< sal_Int8 > aSeq;
    if( pRead && rValue.hasValue() && ( rValue >>= aSeq ) )
    {
        if( pESh )
        {
            pESh->Push();
            SwPaM* pCursor = pESh->GetCursor();
            *pCursor->GetPoint() = *pPam->GetPoint();
            delete pPam;
            pPam = pCursor;
        }

        SvMemoryStream aStrm( const_cast<sal_Int8 *>(aSeq.getConstArray()), aSeq.getLength(),
                                StreamMode::READ );
        aStrm.Seek( 0 );

        // TODO/MBA: it's impossible to set a BaseURL here!
        SwReader aTmpReader( aStrm, OUString(), pDoc->GetDocShell()->GetMedium()->GetBaseURL(), *pPam );

        if( !IsError( aTmpReader.Read( *pRead ) ))
        {
            rSection.SetConnectFlag();
        }

        if( pESh )
        {
            pESh->Pop( false );
            pPam = nullptr; // pam was deleted earlier
        }
    }

    // remove all undo actions and turn undo on again
    pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
    pDoc->GetIDocumentUndoRedo().DoUndo(bWasUndo);
    pDoc->getIDocumentLinksAdministration().SetVisibleLinks( bWasVisibleLinks );

    pDoc->getIDocumentFieldsAccess().UnlockExpFields();
    if( !pDoc->getIDocumentFieldsAccess().IsExpFieldsLocked() )
        pDoc->getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

    if( pESh )
        pESh->EndAllAction();
    else if( pVSh )
        pVSh->EndAction();
    delete pPam; // Was created at the start

    return SUCCESS;
}

void SwIntrnlSectRefLink::Closed()
{
    SwDoc* pDoc = rSectFormat.GetDoc();
    if( pDoc && !pDoc->IsInDtor() )
    {
        // Advise says goodbye: mark the Section as not protected
        // and change the Flag
        const SwSectionFormats& rFormats = pDoc->GetSections();
        for( auto n = rFormats.size(); n; )
            if( rFormats[ --n ] == &rSectFormat )
            {
                SwViewShell* pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
                SwEditShell* pESh = pDoc->GetEditShell();

                if( pESh )
                    pESh->StartAllAction();
                else
                    pSh->StartAction();

                SwSectionData aSectionData(*rSectFormat.GetSection());
                aSectionData.SetType( CONTENT_SECTION );
                aSectionData.SetLinkFileName( OUString() );
                aSectionData.SetHidden( false );
                aSectionData.SetProtectFlag( false );
                // edit in readonly sections
                aSectionData.SetEditInReadonlyFlag( false );

                aSectionData.SetConnectFlag( false );

                pDoc->UpdateSection( n, aSectionData );

                // Make all Links within the Section visible again
                SwSectionNode* pSectNd = rSectFormat.GetSectionNode();
                if( pSectNd )
                    SwSection::MakeChildLinksVisible( *pSectNd );

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
    SwSectionFormat* pFormat = GetFormat();
    OSL_ENSURE(pFormat, "SwSection::CreateLink: no format?");
    if (!pFormat || (CONTENT_SECTION == m_Data.GetType()))
        return ;

    SfxLinkUpdateMode nUpdateType = SfxLinkUpdateMode::ALWAYS;

    if (!m_RefLink.Is())
    {
        // create BaseLink
        m_RefLink = new SwIntrnlSectRefLink( *pFormat, nUpdateType, SotClipboardFormatId::RTF );
    }
    else
    {
        pFormat->GetDoc()->getIDocumentLinksAdministration().GetLinkManager().Remove( m_RefLink );
    }

    SwIntrnlSectRefLink *const pLnk =
        static_cast<SwIntrnlSectRefLink*>(& m_RefLink);

    const OUString sCmd(SwSectionData::CollapseWhiteSpaces(m_Data.GetLinkFileName()));
    pLnk->SetUpdateMode( nUpdateType );
    pLnk->SetVisible( pFormat->GetDoc()->getIDocumentLinksAdministration().IsVisibleLinks() );

    switch (m_Data.GetType())
    {
    case DDE_LINK_SECTION:
        pLnk->SetLinkSourceName( sCmd );
        pFormat->GetDoc()->getIDocumentLinksAdministration().GetLinkManager().InsertDDELink( pLnk );
        break;
    case FILE_LINK_SECTION:
        {
            pLnk->SetContentType( SotClipboardFormatId::SIMPLE_FILE );
            sal_Int32 nIndex = 0;
            const OUString sFile(sCmd.getToken( 0, sfx2::cTokenSeparator, nIndex ));
            const OUString sFltr(sCmd.getToken( 0, sfx2::cTokenSeparator, nIndex ));
            const OUString sRange(sCmd.getToken( 0, sfx2::cTokenSeparator, nIndex ));
            pFormat->GetDoc()->getIDocumentLinksAdministration().GetLinkManager().InsertFileLink( *pLnk,
                                static_cast<sal_uInt16>(m_Data.GetType()),
                                sFile,
                                ( !sFltr.isEmpty() ? &sFltr : nullptr ),
                                ( !sRange.isEmpty() ? &sRange : nullptr ) );
        }
        break;
    default:
        OSL_ENSURE( false, "What kind of Link is this?" );
    }

    switch( eCreateType )
    {
    case CREATE_CONNECT: // Connect Link right away
        pLnk->Connect();
        break;

    case CREATE_UPDATE: // Connect Link and update
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

    // Release link, if it exists
    if (m_RefLink.Is())
    {
        SwSectionFormat *const pFormat( GetFormat() );
        OSL_ENSURE(pFormat, "SwSection::BreakLink: no format?");
        if (pFormat)
        {
            pFormat->GetDoc()->getIDocumentLinksAdministration().GetLinkManager().Remove( m_RefLink );
        }
        m_RefLink.Clear();
    }
    // change type
    SetType( CONTENT_SECTION );
    // reset linked file data
    SetLinkFileName( OUString() );
    SetLinkFilePassword( OUString() );
}

const SwNode* SwIntrnlSectRefLink::GetAnchor() const
{
    return rSectFormat.GetSectionNode();
}

bool SwIntrnlSectRefLink::IsInRange( sal_uLong nSttNd, sal_uLong nEndNd,
                                     sal_Int32 , sal_Int32 ) const
{
    SwStartNode* pSttNd = rSectFormat.GetSectionNode();
    return pSttNd &&
            nSttNd < pSttNd->GetIndex() &&
            pSttNd->EndOfSectionIndex() < nEndNd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
