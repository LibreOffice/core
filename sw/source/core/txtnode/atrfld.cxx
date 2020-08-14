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

#include <fmtfld.hxx>

#include <libxml/xmlwriter.h>

#include <fldbas.hxx>
#include <txtfld.hxx>
#include <txtannotationfld.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <doc.hxx>

#include <pam.hxx>
#include <reffld.hxx>
#include <ddefld.hxx>
#include <usrfld.hxx>
#include <expfld.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <hints.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <fieldhint.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>


// constructor for default item in attribute-pool
SwFormatField::SwFormatField( sal_uInt16 nWhich )
    : SfxPoolItem( nWhich )
    , SfxBroadcaster()
    , mpTextField( nullptr )
{
}

SwFormatField::SwFormatField( const SwField &rField )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SfxBroadcaster()
    , mpField( rField.CopyField() )
    , mpTextField( nullptr )
{
    rField.GetTyp()->Add(this);
    if ( mpField->GetTyp()->Which() == SwFieldIds::Input )
    {
        // input field in-place editing
        SetWhich( RES_TXTATR_INPUTFIELD );
        static_cast<SwInputField*>(mpField.get())->SetFormatField( *this );
    }
    else if (mpField->GetTyp()->Which() == SwFieldIds::SetExp)
    {
        // see SwWrtShell::StartInputFieldDlg
        SwSetExpField *const pSetField(static_cast<SwSetExpField *>(mpField.get()));
        if (pSetField->GetInputFlag()
            // only for string fields for now - inline editing of number fields
            // tends to produce error messages...
            && (static_cast<SwSetExpFieldType*>(pSetField->GetTyp())->GetType()
                & nsSwGetSetExpType::GSE_STRING))
        {
            SetWhich( RES_TXTATR_INPUTFIELD );
        }
        pSetField->SetFormatField(*this);
    }
    else if ( mpField->GetTyp()->Which() == SwFieldIds::Postit )
    {
        // text annotation field
        SetWhich( RES_TXTATR_ANNOTATION );
    }
}

// #i24434#
// Since Items are used in ItemPool and in default constructed ItemSets with
// full pool range, all items need to be clonable. Thus, this one needed to be
// corrected
SwFormatField::SwFormatField( const SwFormatField& rAttr )
    : SfxPoolItem( rAttr )
    , SfxBroadcaster()
    , mpTextField( nullptr )
{
    if ( !rAttr.mpField )
        return;

    rAttr.mpField->GetTyp()->Add(this);
    mpField = rAttr.mpField->CopyField();
    if ( mpField->GetTyp()->Which() == SwFieldIds::Input )
    {
        // input field in-place editing
        SetWhich( RES_TXTATR_INPUTFIELD );
        SwInputField *pField = dynamic_cast<SwInputField*>(mpField.get());
        assert(pField);
        if (pField)
            pField->SetFormatField( *this );
    }
    else if (mpField->GetTyp()->Which() == SwFieldIds::SetExp)
    {
        SwSetExpField *const pSetField(static_cast<SwSetExpField *>(mpField.get()));
        if (pSetField->GetInputFlag()
            && (static_cast<SwSetExpFieldType*>(pSetField->GetTyp())->GetType()
                & nsSwGetSetExpType::GSE_STRING))
        {
            SetWhich( RES_TXTATR_INPUTFIELD );
        }
        // see SwWrtShell::StartInputFieldDlg
        pSetField->SetFormatField(*this);
    }
    else if ( mpField->GetTyp()->Which() == SwFieldIds::Postit )
    {
        // text annotation field
        SetWhich( RES_TXTATR_ANNOTATION );
    }
}

SwFormatField::~SwFormatField()
{
    SwFieldType* pType = mpField ? mpField->GetTyp() : nullptr;

    if (pType && pType->Which() == SwFieldIds::Database)
        pType = nullptr;  // DB field types destroy themselves

    Broadcast( SwFormatFieldHint( this, SwFormatFieldHintWhich::REMOVED ) );
    mpField.reset();

    // some fields need to delete their field type
    if( !(pType && pType->HasOnlyOneListener()) )
        return;

    bool bDel = false;
    switch( pType->Which() )
    {
    case SwFieldIds::User:
        bDel = static_cast<SwUserFieldType*>(pType)->IsDeleted();
        break;

    case SwFieldIds::SetExp:
        bDel = static_cast<SwSetExpFieldType*>(pType)->IsDeleted();
        break;

    case SwFieldIds::Dde:
        bDel = static_cast<SwDDEFieldType*>(pType)->IsDeleted();
        break;
    default: break;
    }

    if( bDel )
    {
        // unregister before deleting
        pType->Remove( this );
        delete pType;
    }
}

void SwFormatField::RegisterToFieldType( SwFieldType& rType )
{
    rType.Add(this);
}

void SwFormatField::SetField(std::unique_ptr<SwField> _pField)
{
    mpField = std::move(_pField);
    if ( mpField->GetTyp()->Which() == SwFieldIds::Input )
    {
        static_cast<SwInputField* >(mpField.get())->SetFormatField( *this );
    }
    else if (mpField->GetTyp()->Which() == SwFieldIds::SetExp)
    {
        // see SwWrtShell::StartInputFieldDlg
        static_cast<SwSetExpField *>(mpField.get())->SetFormatField(*this);
    }
    Broadcast( SwFormatFieldHint( this, SwFormatFieldHintWhich::CHANGED ) );
}

void SwFormatField::SetTextField( SwTextField& rTextField )
{
    mpTextField = &rTextField;
}

void SwFormatField::ClearTextField()
{
    mpTextField = nullptr;
}

bool SwFormatField::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( mpField
             && static_cast<const SwFormatField&>(rAttr).mpField
             && mpField->GetTyp() == static_cast<const SwFormatField&>(rAttr).mpField->GetTyp()
             && mpField->GetFormat() == static_cast<const SwFormatField&>(rAttr).mpField->GetFormat() )
           ||
           ( !mpField && !static_cast<const SwFormatField&>(rAttr).mpField );
}

SwFormatField* SwFormatField::Clone( SfxItemPool* ) const
{
    return new SwFormatField( *this );
}

void SwFormatField::InvalidateField()
{
    SwPtrMsgPoolItem const item(RES_REMOVE_UNO_OBJECT,
            &static_cast<SwModify&>(*this)); // cast to base class (void*)
    CallSwClientNotify(sw::LegacyModifyHint{ &item, &item });
}

void SwFormatField::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rModify, rHint);
    if (const auto pFieldHint = dynamic_cast<const SwFieldHint*>( &rHint ))
    {
        if( !mpTextField )
            return;

        // replace field content by text
        SwPaM* pPaM = pFieldHint->m_pPaM;
        SwDoc* pDoc = pPaM->GetDoc();
        const SwTextNode& rTextNode = mpTextField->GetTextNode();
        pPaM->GetPoint()->nNode = rTextNode;
        pPaM->GetPoint()->nContent.Assign( const_cast<SwTextNode*>(&rTextNode), mpTextField->GetStart() );

        OUString const aEntry(mpField->ExpandField(pDoc->IsClipBoard(), pFieldHint->m_pLayout));
        pPaM->SetMark();
        pPaM->Move( fnMoveForward );
        pDoc->getIDocumentContentOperations().DeleteRange( *pPaM );
        pDoc->getIDocumentContentOperations().InsertString( *pPaM, aEntry );
    } else if (const auto pLegacyHint = dynamic_cast<const sw::LegacyModifyHint*>( &rHint ))
    {
        if( !mpTextField )
            return;
        UpdateTextNode(pLegacyHint->m_pOld, pLegacyHint->m_pNew);
    } else if (const auto pFindForFieldHint = dynamic_cast<const sw::FindFormatForFieldHint*>( &rHint ))
    {
        if(pFindForFieldHint->m_rpFormat == nullptr && pFindForFieldHint->m_pField == GetField())
            pFindForFieldHint->m_rpFormat = this;
    } else if (const auto pFindForPostItIdHint = dynamic_cast<const sw::FindFormatForPostItIdHint*>( &rHint ))
    {
        auto pPostItField = dynamic_cast<SwPostItField*>(mpField.get());
        if(pPostItField && pFindForPostItIdHint->m_rpFormat == nullptr && pFindForPostItIdHint->m_nPostItId == pPostItField->GetPostItId())
            pFindForPostItIdHint->m_rpFormat = this;
    } else if (const auto pCollectPostItsHint = dynamic_cast<const sw::CollectPostItsHint*>( &rHint ))
    {
        if(GetTextField() && IsFieldInDoc() && (!pCollectPostItsHint->m_bHideRedlines || !sw::IsFieldDeletedInModel(pCollectPostItsHint->m_rIDRA, *GetTextField())))
            pCollectPostItsHint->m_rvFormatFields.push_back(this);
    } else if (const auto pHasHiddenInfoHint = dynamic_cast<const sw::HasHiddenInformationNotesHint*>( &rHint ))
    {
        if(!pHasHiddenInfoHint->m_rbHasHiddenInformationNotes && GetTextField() && IsFieldInDoc())
            pHasHiddenInfoHint->m_rbHasHiddenInformationNotes = true;
    } else if (const auto pGatherNodeIndexHint = dynamic_cast<const sw::GatherNodeIndexHint*>( &rHint ))
    {
        if(auto pTextField = GetTextField())
            pGatherNodeIndexHint->m_rvNodeIndex.push_back(pTextField->GetTextNode().GetIndex());
    } else if (const auto pGatherRefFieldsHint = dynamic_cast<const sw::GatherRefFieldsHint*>( &rHint ))
    {
        if(!GetTextField() || pGatherRefFieldsHint->m_nType != GetField()->GetSubType())
            return;
        SwTextNode* pNd = GetTextField()->GetpTextNode();
        if(pNd && pNd->GetNodes().IsDocNodes())
            pGatherRefFieldsHint->m_rvRFields.push_back(static_cast<SwGetRefField*>(GetField()));
    } else if (const auto pGatherFieldsHint = dynamic_cast<const sw::GatherFieldsHint*>( &rHint ))
    {
        if(pGatherFieldsHint->m_bCollectOnlyInDocNodes)
        {
            if(!GetTextField())
                return;
            SwTextNode* pNd = GetTextField()->GetpTextNode();
            if(!pNd || !pNd->GetNodes().IsDocNodes())
                return;
        }
        pGatherFieldsHint->m_rvFields.push_back(this);
    }
}

void SwFormatField::UpdateTextNode(const SfxPoolItem* pOld, const SfxPoolItem* pNew)
{
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached UNO object
        m_wXTextField = nullptr;
        // ??? why does this Modify method not already do this?
        NotifyClients(pOld, pNew);
        return;
    }

    if( !mpTextField )
        return;

    // don't do anything, especially not expand!
    if( pNew && pNew->Which() == RES_OBJECTDYING )
        return;

    SwTextNode* pTextNd = &mpTextField->GetTextNode();
    OSL_ENSURE( pTextNd, "Where is my Node?" );

    bool bTriggerNode = false;
    bool bExpand = false;
    const SfxPoolItem* pNodeOld = nullptr;
    const SfxPoolItem* pNodeNew = nullptr;
    if(pNew)
    {
        switch(pNew->Which())
        {
        case RES_REFMARKFLD_UPDATE:
            // update GetRef fields
            if( SwFieldIds::GetRef == mpField->GetTyp()->Which() )
            {
                // #i81002#
                static_cast<SwGetRefField*>(mpField.get())->UpdateField( mpTextField );
            }
            break;
        case RES_DOCPOS_UPDATE:
            // handled in SwTextFrame::Modify()
            bTriggerNode = true;
            pNodeOld = pNew;
            pNodeNew = this;
            break;
        case RES_ATTRSET_CHG:
        case RES_FMT_CHG:
            bTriggerNode = true;
            pNodeOld = pOld;
            pNodeNew = pNew;
            break;
        default:
            break;
        }
    }
    if(!bTriggerNode)
    {
        switch (mpField->GetTyp()->Which())
        {
            case SwFieldIds::HiddenPara:
                if( !pOld || pOld->Which() != RES_HIDDENPARA_PRINT ) {
                    bExpand =true;
                    break;
                }
                [[fallthrough]];
            case SwFieldIds::DbSetNumber:
            case SwFieldIds::DbNumSet:
            case SwFieldIds::DbNextSet:
            case SwFieldIds::DatabaseName:
                bTriggerNode = true;
                pNodeNew = pNew;
                break;
            case SwFieldIds::User:
            {
                SwUserFieldType* pType = static_cast<SwUserFieldType*>(mpField->GetTyp());
                if(!pType->IsValid())
                {
                    SwCalc aCalc( *pTextNd->GetDoc() );
                    pType->GetValue( aCalc );
                }
                bExpand = true;
            }
            break;
            default:
                bExpand = true;
                break;
        }
    }
    if(bTriggerNode)
    {
        pTextNd->ModifyNotification(pNodeOld, pNodeNew);
    }
    if(bExpand)
    {
        mpTextField->ExpandTextField( pOld == nullptr && pNew == nullptr );
    }
}

bool SwFormatField::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !mpTextField )
        return true;
    const SwTextNode* pTextNd = mpTextField->GetpTextNode();
    return nullptr == pTextNd ||
        &pTextNd->GetNodes() != static_cast<SwAutoFormatGetDocNode&>(rInfo).pNodes;
}

bool SwFormatField::IsFieldInDoc() const
{
    return mpTextField != nullptr
           && mpTextField->IsFieldInDoc();
}

bool SwFormatField::IsProtect() const
{
    return mpTextField != nullptr
           && mpTextField->GetpTextNode() != nullptr
           && mpTextField->GetpTextNode()->IsProtect();
}

void SwFormatField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatField"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("mpTextField"), "%p", mpTextField);

    SfxPoolItem::dumpAsXml(pWriter);
    mpField->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

// class SwTextField ////////////////////////////////////////////////////

SwTextField::SwTextField(
    SwFormatField & rAttr,
    sal_Int32 const nStartPos,
    bool const bInClipboard)
    : SwTextAttr( rAttr, nStartPos )
// fdo#39694 the ExpandField here may not give the correct result in all cases,
// but is better than nothing
    , m_aExpand( rAttr.GetField()->ExpandField(bInClipboard, nullptr) )
    , m_pTextNode( nullptr )
{
    rAttr.SetTextField( *this );
    SetHasDummyChar(true);
}

SwTextField::~SwTextField( )
{
    SwFormatField & rFormatField( static_cast<SwFormatField &>(GetAttr()) );
    if ( this == rFormatField.GetTextField() )
    {
        rFormatField.ClearTextField();
    }
}

bool SwTextField::IsFieldInDoc() const
{
    return GetpTextNode() != nullptr
           && GetpTextNode()->GetNodes().IsDocNodes();
}

void SwTextField::ExpandTextField(const bool bForceNotify) const
{
    OSL_ENSURE( m_pTextNode, "SwTextField: where is my TextNode?" );

    const SwField* pField = GetFormatField().GetField();
    const OUString aNewExpand( pField->ExpandField(m_pTextNode->GetDoc()->IsClipBoard(),
        // can't do any better than this here...
        m_pTextNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()) );

    const SwFieldIds nWhich = pField->GetTyp()->Which();
    const bool bSameExpandSimpleNotification
        = SwFieldIds::Chapter != nWhich && SwFieldIds::PageNumber != nWhich
          && SwFieldIds::RefPageGet != nWhich
          // Page count fields to not use aExpand during formatting,
          // therefore an invalidation of the text frame has to be triggered even if aNewExpand == aExpand:
          && (SwFieldIds::DocStat != nWhich
              || DS_PAGE != static_cast<const SwDocStatField*>(pField)->GetSubType())
          && (SwFieldIds::GetExp != nWhich
              || static_cast<const SwGetExpField*>(pField)->IsInBodyText());

    bool bHiddenParaChanged = false;
    if (aNewExpand != m_aExpand || bSameExpandSimpleNotification)
        bHiddenParaChanged = m_pTextNode->CalcHiddenParaField();

    if (aNewExpand == m_aExpand)
    {
        if ( bSameExpandSimpleNotification )
        {
            if( bHiddenParaChanged )
            {
                m_pTextNode->ModifyNotification( nullptr, nullptr );
            }
            if ( !bForceNotify )
            {
                // done, if no further notification forced.
                return;
            }
        }
    }
    else
        m_aExpand = aNewExpand;

    const_cast<SwTextField*>(this)->NotifyContentChange( const_cast<SwFormatField&>(GetFormatField()) );
}

void SwTextField::CopyTextField( SwTextField *pDest ) const
{
    OSL_ENSURE( m_pTextNode, "SwTextField: where is my TextNode?" );
    OSL_ENSURE( pDest->m_pTextNode, "SwTextField: where is pDest's TextNode?" );

    IDocumentFieldsAccess* pIDFA = &m_pTextNode->getIDocumentFieldsAccess();
    IDocumentFieldsAccess* pDestIDFA = &pDest->m_pTextNode->getIDocumentFieldsAccess();

    SwFormatField& rDestFormatField = const_cast<SwFormatField&>(pDest->GetFormatField());
    const SwFieldIds nFieldWhich = rDestFormatField.GetField()->GetTyp()->Which();

    if( pIDFA != pDestIDFA )
    {
        // different documents, e.g. clipboard:
        // register field type in target document
        SwFieldType* pFieldType;
        if( nFieldWhich != SwFieldIds::Database
            && nFieldWhich != SwFieldIds::User
            && nFieldWhich != SwFieldIds::SetExp
            && nFieldWhich != SwFieldIds::Dde
            && SwFieldIds::TableOfAuthorities != nFieldWhich )
        {
            pFieldType = pDestIDFA->GetSysFieldType( nFieldWhich );
        }
        else
        {
            pFieldType = pDestIDFA->InsertFieldType( *rDestFormatField.GetField()->GetTyp() );
        }

        // DDE fields need special treatment
        if( SwFieldIds::Dde == nFieldWhich )
        {
            if( rDestFormatField.GetTextField() )
            {
                static_cast<SwDDEFieldType*>(rDestFormatField.GetField()->GetTyp())->DecRefCnt();
            }
            static_cast<SwDDEFieldType*>(pFieldType)->IncRefCnt();
        }

        OSL_ENSURE( pFieldType, "unknown FieldType" );
        pFieldType->Add( &rDestFormatField ); // register at the field type
        rDestFormatField.GetField()->ChgTyp( pFieldType );
    }

    // update expression fields
    if( nFieldWhich == SwFieldIds::SetExp
        || nFieldWhich == SwFieldIds::GetExp
        || nFieldWhich == SwFieldIds::HiddenText )
    {
        SwTextField* pField = const_cast<SwTextField*>(this);
        pDestIDFA->UpdateExpFields( pField, true );
    }
    // table fields: external display
    else if( SwFieldIds::Table == nFieldWhich
             && static_cast<SwTableField*>(rDestFormatField.GetField())->IsIntrnlName() )
    {
        // convert internal (core) to external (UI) formula
        const SwTableNode* pTableNd = m_pTextNode->FindTableNode();
        if( pTableNd )        // in a table?
            static_cast<SwTableField*>(rDestFormatField.GetField())->PtrToBoxNm( &pTableNd->GetTable() );
    }
}

void SwTextField::NotifyContentChange(SwFormatField& rFormatField)
{
    //if not in undo section notify the change
    if (m_pTextNode && m_pTextNode->GetNodes().IsDocNodes())
    {
        m_pTextNode->ModifyNotification(nullptr, &rFormatField);
    }
}

/*static*/
void SwTextField::GetPamForTextField(
    const SwTextField& rTextField,
    std::shared_ptr< SwPaM >& rPamForTextField )
{
    if (rTextField.GetpTextNode() == nullptr)
    {
        SAL_WARN("sw.core", "<SwTextField::GetPamForField> - missing <SwTextNode>");
        return;
    }

    const SwTextNode& rTextNode = rTextField.GetTextNode();

    rPamForTextField = std::make_shared<SwPaM>( rTextNode,
                                    (rTextField.End() != nullptr) ? *(rTextField.End()) : ( rTextField.GetStart() + 1 ),
                                    rTextNode,
                                    rTextField.GetStart() );

}

/*static*/
void SwTextField::DeleteTextField( const SwTextField& rTextField )
{
    if (rTextField.GetpTextNode() != nullptr)
    {
        std::shared_ptr< SwPaM > pPamForTextField;
        GetPamForTextField(rTextField, pPamForTextField);
        if (pPamForTextField != nullptr)
        {
            rTextField.GetTextNode().GetDoc()->getIDocumentContentOperations().DeleteAndJoin(*pPamForTextField);
        }
    }
}

// class SwTextInputField ///////////////////////////////////////////////

// input field in-place editing
SwTextInputField::SwTextInputField(
    SwFormatField & rAttr,
    sal_Int32 const nStart,
    sal_Int32 const nEnd,
    bool const bInClipboard )

    : SwTextAttr( rAttr, nStart )
    , SwTextAttrNesting( rAttr, nStart, nEnd )
    , SwTextField( rAttr, nStart, bInClipboard )
    , m_bLockNotifyContentChange( false )
{
    SetHasDummyChar( false );
    SetHasContent( true );
}

SwTextInputField::~SwTextInputField()
{
}

bool SwTextInputField::LockNotifyContentChange()
{
    if (m_bLockNotifyContentChange)
    {
        return false;
    }
    m_bLockNotifyContentChange = true;
    return true;
}

void SwTextInputField::UnlockNotifyContentChange()
{
    m_bLockNotifyContentChange = false;
}

void SwTextInputField::NotifyContentChange( SwFormatField& rFormatField )
{
    if ( !m_bLockNotifyContentChange )
    {
        LockNotifyContentChange();

        SwTextField::NotifyContentChange( rFormatField );
        UpdateTextNodeContent( GetFieldContent() );

        UnlockNotifyContentChange();
    }
}

OUString SwTextInputField::GetFieldContent() const
{
    return GetFormatField().GetField()->ExpandField(false, nullptr/*ignored anyway*/);
}

void SwTextInputField::UpdateFieldContent()
{
    if ( !(IsFieldInDoc()
         && GetStart() != (*End())) )
        return;

    assert( (*End()) - GetStart() >= 2 &&
            "<SwTextInputField::UpdateFieldContent()> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
    // skip CH_TXT_ATR_INPUTFIELDSTART character
    const sal_Int32 nIdx = GetStart() + 1;
    // skip CH_TXT_ATR_INPUTFIELDEND character
    const sal_Int32 nLen = static_cast<sal_Int32>(std::max<sal_Int32>( 0, ( (*End()) - 1 - nIdx ) ));
    const OUString aNewFieldContent = GetTextNode().GetExpandText(nullptr, nIdx, nLen);

    const SwField* pField = GetFormatField().GetField();
    const SwInputField* pInputField = dynamic_cast<const SwInputField*>(pField);
    if (pInputField)
        const_cast<SwInputField*>(pInputField)->applyFieldContent( aNewFieldContent );

    const SwSetExpField* pExpField = dynamic_cast<const SwSetExpField*>(pField);
    if (pExpField)
    {
        assert(pExpField->GetInputFlag());
        const_cast<SwSetExpField*>(pExpField)->SetPar2(aNewFieldContent);
    }
    assert(pInputField || pExpField);

    // trigger update of fields for scenarios in which the Input Field's content is part of e.g. a table formula
    GetTextNode().GetDoc()->getIDocumentFieldsAccess().GetUpdateFields().SetFieldsDirty(true);
}

void SwTextInputField::UpdateTextNodeContent( const OUString& rNewContent )
{
    assert(IsFieldInDoc() &&
        "<SwTextInputField::UpdateTextNodeContent(..)> - misusage as Input Field is not in document content.");

    assert( (*End()) - GetStart() >= 2 &&
            "<SwTextInputField::UpdateTextNodeContent(..)> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
    // skip CH_TXT_ATR_INPUTFIELDSTART character
    const sal_Int32 nIdx = GetStart() + 1;
    // skip CH_TXT_ATR_INPUTFIELDEND character
    const sal_Int32 nDelLen = std::max<sal_Int32>( 0, ( (*End()) - 1 - nIdx ) );
    SwIndex aIdx( &GetTextNode(), nIdx );
    GetTextNode().ReplaceText( aIdx, nDelLen, rNewContent );
}

// class SwTextAnnotationField //////////////////////////////////////////

// text annotation field
SwTextAnnotationField::SwTextAnnotationField(
    SwFormatField & rAttr,
    sal_Int32 const nStart,
    bool const bInClipboard )
    : SwTextAttr( rAttr, nStart )
    , SwTextField( rAttr, nStart, bInClipboard )
{
}

SwTextAnnotationField::~SwTextAnnotationField()
{
}

::sw::mark::IMark* SwTextAnnotationField::GetAnnotationMark() const
{
    auto pPostItField = dynamic_cast<const SwPostItField*>(GetFormatField().GetField());
    assert(pPostItField);

    SwDoc* pDoc = static_cast<const SwPostItFieldType*>(pPostItField->GetTyp())->GetDoc();
    assert(pDoc != nullptr);

    IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t pMark = pMarksAccess->findAnnotationMark( pPostItField->GetName() );
    return pMark != pMarksAccess->getAnnotationMarksEnd()
           ? *pMark
           : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
