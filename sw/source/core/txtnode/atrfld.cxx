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

#include "fldbas.hxx"
#include <txtfld.hxx>
#include <txtannotationfld.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <doc.hxx>

#include "pam.hxx"
#include "reffld.hxx"
#include "ddefld.hxx"
#include "usrfld.hxx"
#include "expfld.hxx"
#include "swfont.hxx"
#include "ndtxt.hxx"
#include "calc.hxx"
#include "hints.hxx"
#include <IDocumentFieldsAccess.hxx>
#include <fieldhint.hxx>
#include <svl/smplhint.hxx>


// constructor for default item in attribute-pool
SwFormatField::SwFormatField( sal_uInt16 nWhich )
    : SfxPoolItem( nWhich )
    , SwModify(nullptr)
    , SfxBroadcaster()
    , mpField( nullptr )
    , mpTextField( nullptr )
{
}

SwFormatField::SwFormatField( const SwField &rField )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwModify( rField.GetTyp() )
    , SfxBroadcaster()
    , mpField( rField.CopyField() )
    , mpTextField( nullptr )
{
    if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
    {
        // input field in-place editing
        SetWhich( RES_TXTATR_INPUTFIELD );
        static_cast<SwInputField*>(GetField())->SetFormatField( *this );
    }
    else if (GetField()->GetTyp()->Which() == RES_SETEXPFLD)
    {
        // see SwWrtShell::StartInputFieldDlg
        static_cast<SwSetExpField *>(GetField())->SetFormatField(*this);
    }
    else if ( GetField()->GetTyp()->Which() == RES_POSTITFLD )
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
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwModify(nullptr)
    , SfxBroadcaster()
    , mpField( nullptr )
    , mpTextField( nullptr )
{
    if ( rAttr.GetField() )
    {
        rAttr.GetField()->GetTyp()->Add(this);
        mpField = rAttr.GetField()->CopyField();
        if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
        {
            // input field in-place editing
            SetWhich( RES_TXTATR_INPUTFIELD );
            SwInputField *pField = dynamic_cast<SwInputField*>(GetField());
            assert(pField);
            if (pField)
                pField->SetFormatField( *this );
        }
        else if (GetField()->GetTyp()->Which() == RES_SETEXPFLD)
        {
            // see SwWrtShell::StartInputFieldDlg
            static_cast<SwSetExpField *>(GetField())->SetFormatField(*this);
        }
        else if ( GetField()->GetTyp()->Which() == RES_POSTITFLD )
        {
            // text annotation field
            SetWhich( RES_TXTATR_ANNOTATION );
        }
    }
}

SwFormatField::~SwFormatField()
{
    SwFieldType* pType = mpField ? mpField->GetTyp() : nullptr;

    if (pType && pType->Which() == RES_DBFLD)
        pType = nullptr;  // DB-Feldtypen zerstoeren sich selbst

    Broadcast( SwFormatFieldHint( this, SwFormatFieldHintWhich::REMOVED ) );
    delete mpField;

    // bei einige FeldTypen muessen wir den FeldTypen noch loeschen
    if( pType && pType->HasOnlyOneListener() )
    {
        bool bDel = false;
        switch( pType->Which() )
        {
        case RES_USERFLD:
            bDel = static_cast<SwUserFieldType*>(pType)->IsDeleted();
            break;

        case RES_SETEXPFLD:
            bDel = static_cast<SwSetExpFieldType*>(pType)->IsDeleted();
            break;

        case RES_DDEFLD:
            bDel = static_cast<SwDDEFieldType*>(pType)->IsDeleted();
            break;
        }

        if( bDel )
        {
            // vorm loeschen erstmal austragen
            pType->Remove( this );
            delete pType;
        }
    }
}

void SwFormatField::RegisterToFieldType( SwFieldType& rType )
{
    rType.Add(this);
}

void SwFormatField::SetField(SwField * _pField)
{
    delete mpField;

    mpField = _pField;
    if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
    {
        static_cast<SwInputField* >(GetField())->SetFormatField( *this );
    }
    else if (GetField()->GetTyp()->Which() == RES_SETEXPFLD)
    {
        // see SwWrtShell::StartInputFieldDlg
        static_cast<SwSetExpField *>(GetField())->SetFormatField(*this);
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
    return ( ( mpField && static_cast<const SwFormatField&>(rAttr).GetField()
               && mpField->GetTyp() == static_cast<const SwFormatField&>(rAttr).GetField()->GetTyp()
               && mpField->GetFormat() == static_cast<const SwFormatField&>(rAttr).GetField()->GetFormat() ) )
             || ( !mpField && !static_cast<const SwFormatField&>(rAttr).GetField() );
}

SfxPoolItem* SwFormatField::Clone( SfxItemPool* ) const
{
    return new SwFormatField( *this );
}

void SwFormatField::InvalidateField()
{
    SwPtrMsgPoolItem const item(RES_REMOVE_UNO_OBJECT,
            &static_cast<SwModify&>(*this)); // cast to base class (void*)
    NotifyClients(&item, &item);
}

void SwFormatField::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rModify, rHint);
    if( !mpTextField )
        return;

    const SwFieldHint* pHint = dynamic_cast<const SwFieldHint*>( &rHint );
    if ( pHint )
    {
        // replace field content by text
        SwPaM* pPaM = pHint->GetPaM();
        SwDoc* pDoc = pPaM->GetDoc();
        const SwTextNode& rTextNode = mpTextField->GetTextNode();
        pPaM->GetPoint()->nNode = rTextNode;
        pPaM->GetPoint()->nContent.Assign( const_cast<SwTextNode*>(&rTextNode), mpTextField->GetStart() );

        OUString const aEntry( GetField()->ExpandField( pDoc->IsClipBoard() ) );
        pPaM->SetMark();
        pPaM->Move( fnMoveForward );
        pDoc->getIDocumentContentOperations().DeleteRange( *pPaM );
        pDoc->getIDocumentContentOperations().InsertString( *pPaM, aEntry );
    }
}

void SwFormatField::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached UNO object
        SetXTextField(css::uno::Reference<css::text::XTextField>(nullptr));
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
    OSL_ENSURE( pTextNd, "wo ist denn mein Node?" );
    if( pNew )
    {
        switch( pNew->Which() )
        {
        case RES_TXTATR_FLDCHG:
                // "Farbe hat sich geaendert !"
                // this, this fuer "nur Painten"
                pTextNd->ModifyNotification( this, this );
                return;
        case RES_REFMARKFLD_UPDATE:
                // GetReferenz-Felder aktualisieren
                if( RES_GETREFFLD == GetField()->GetTyp()->Which() )
                {
                    // #i81002#
                    static_cast<SwGetRefField*>(GetField())->UpdateField( mpTextField );
                }
                break;
        case RES_DOCPOS_UPDATE:
                // Je nach DocPos aktualisieren (SwTextFrame::Modify())
                pTextNd->ModifyNotification( pNew, this );
                return;

        case RES_ATTRSET_CHG:
        case RES_FMT_CHG:
                pTextNd->ModifyNotification( pOld, pNew );
                return;
        default:
                break;
        }
    }

    switch (GetField()->GetTyp()->Which())
    {
        case RES_HIDDENPARAFLD:
            if( !pOld || RES_HIDDENPARA_PRINT != pOld->Which() )
                break;
            SAL_FALLTHROUGH;
        case RES_DBSETNUMBERFLD:
        case RES_DBNUMSETFLD:
        case RES_DBNEXTSETFLD:
        case RES_DBNAMEFLD:
            pTextNd->ModifyNotification( nullptr, pNew);
            return;
    }

    if( RES_USERFLD == GetField()->GetTyp()->Which() )
    {
        SwUserFieldType* pType = static_cast<SwUserFieldType*>(GetField()->GetTyp());
        if(!pType->IsValid())
        {
            SwCalc aCalc( *pTextNd->GetDoc() );
            pType->GetValue( aCalc );
        }
    }

    const bool bForceNotify = (pOld == nullptr) && (pNew == nullptr);
    mpTextField->ExpandTextField( bForceNotify );
}

bool SwFormatField::GetInfo( SfxPoolItem& rInfo ) const
{
    const SwTextNode* pTextNd;
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() ||
        !mpTextField || nullptr == ( pTextNd = mpTextField->GetpTextNode() ) ||
        &pTextNd->GetNodes() != static_cast<SwAutoFormatGetDocNode&>(rInfo).pNodes )
        return true;

    static_cast<SwAutoFormatGetDocNode&>(rInfo).pContentNode = pTextNd;
    return false;
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFormatField"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("mpTextField"), "%p", mpTextField);

    SfxPoolItem::dumpAsXml(pWriter);
    GetField()->dumpAsXml(pWriter);

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
    , m_aExpand( rAttr.GetField()->ExpandField(bInClipboard) )
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
    const OUString aNewExpand( pField->ExpandField(m_pTextNode->GetDoc()->IsClipBoard()) );

    if (aNewExpand == m_aExpand)
    {
        // Bei Seitennummernfeldern
        const sal_uInt16 nWhich = pField->GetTyp()->Which();
        if ( RES_CHAPTERFLD != nWhich
             && RES_PAGENUMBERFLD != nWhich
             && RES_REFPAGEGETFLD != nWhich
             // Page count fields to not use aExpand during formatting,
             // therefore an invalidation of the text frame has to be triggered even if aNewExpand == aExpand:
             && ( RES_DOCSTATFLD != nWhich || DS_PAGE != static_cast<const SwDocStatField*>(pField)->GetSubType() )
             && ( RES_GETEXPFLD != nWhich || static_cast<const SwGetExpField*>(pField)->IsInBodyText() ) )
        {
            if( m_pTextNode->CalcHiddenParaField() )
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

    m_aExpand = aNewExpand;

    const_cast<SwTextField*>(this)->NotifyContentChange( const_cast<SwFormatField&>(GetFormatField()) );
}

void SwTextField::CopyTextField( SwTextField *pDest ) const
{
    OSL_ENSURE( m_pTextNode, "SwTextField: where is my TextNode?" );
    OSL_ENSURE( pDest->m_pTextNode, "SwTextField: where is pDest's TextNode?" );

    IDocumentFieldsAccess* pIDFA = &m_pTextNode->getIDocumentFieldsAccess();
    IDocumentFieldsAccess* pDestIDFA = &pDest->m_pTextNode->getIDocumentFieldsAccess();

    SwFormatField& rDestFormatField = (SwFormatField&)pDest->GetFormatField();
    const sal_uInt16 nFieldWhich = rDestFormatField.GetField()->GetTyp()->Which();

    if( pIDFA != pDestIDFA )
    {
        // Die Hints stehen in unterschiedlichen Dokumenten,
        // der Feldtyp muss im neuen Dokument angemeldet werden.
        // Z.B: Kopieren ins ClipBoard.
        SwFieldType* pFieldType;
        if( nFieldWhich != RES_DBFLD
            && nFieldWhich != RES_USERFLD
            && nFieldWhich != RES_SETEXPFLD
            && nFieldWhich != RES_DDEFLD
            && RES_AUTHORITY != nFieldWhich )
        {
            pFieldType = pDestIDFA->GetSysFieldType( nFieldWhich );
        }
        else
        {
            pFieldType = pDestIDFA->InsertFieldType( *rDestFormatField.GetField()->GetTyp() );
        }

        // Sonderbehandlung fuer DDE-Felder
        if( RES_DDEFLD == nFieldWhich )
        {
            if( rDestFormatField.GetTextField() )
            {
                static_cast<SwDDEFieldType*>(rDestFormatField.GetField()->GetTyp())->DecRefCnt();
            }
            static_cast<SwDDEFieldType*>(pFieldType)->IncRefCnt();
        }

        OSL_ENSURE( pFieldType, "unbekannter FieldType" );
        pFieldType->Add( &rDestFormatField );          // ummelden
        rDestFormatField.GetField()->ChgTyp( pFieldType );
    }

    // Expressionfelder Updaten
    if( nFieldWhich == RES_SETEXPFLD
        || nFieldWhich == RES_GETEXPFLD
        || nFieldWhich == RES_HIDDENTXTFLD )
    {
        SwTextField* pField = const_cast<SwTextField*>(this);
        pDestIDFA->UpdateExpFields( pField, true );
    }
    // Tabellenfelder auf externe Darstellung
    else if( RES_TABLEFLD == nFieldWhich
             && static_cast<SwTableField*>(rDestFormatField.GetField())->IsIntrnlName() )
    {
        // erzeuge aus der internen (fuer CORE) die externe (fuer UI) Formel
        const SwTableNode* pTableNd = m_pTextNode->FindTableNode();
        if( pTableNd )        // steht in einer Tabelle
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

    rPamForTextField.reset( new SwPaM( rTextNode,
                                    (rTextField.End() != nullptr) ? *(rTextField.End()) : ( rTextField.GetStart() + 1 ),
                                    rTextNode,
                                    rTextField.GetStart() ) );

}

/*static*/
void SwTextField::DeleteTextField( const SwTextField& rTextField )
{
    if (rTextField.GetpTextNode() != nullptr)
    {
        std::shared_ptr< SwPaM > pPamForTextField;
        GetPamForTextField(rTextField, pPamForTextField);
        if (pPamForTextField.get() != nullptr)
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

void SwTextInputField::LockNotifyContentChange()
{
    m_bLockNotifyContentChange = true;
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

const OUString SwTextInputField::GetFieldContent() const
{
    return GetFormatField().GetField()->ExpandField(false);
}

void SwTextInputField::UpdateFieldContent()
{
    if ( IsFieldInDoc()
         && GetStart() != (*End()) )
    {
        OSL_ENSURE( (*End()) - GetStart() >= 2,
                "<SwTextInputField::UpdateFieldContent()> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
        // skip CH_TXT_ATR_INPUTFIELDSTART character
        const sal_Int32 nIdx = GetStart() + 1;
        // skip CH_TXT_ATR_INPUTFIELDEND character
        const sal_Int32 nLen = static_cast<sal_Int32>(std::max<sal_Int32>( 0, ( (*End()) - 1 - nIdx ) ));
        const OUString aNewFieldContent = GetTextNode().GetExpandText( nIdx, nLen );

        const SwInputField* pInputField = dynamic_cast<const SwInputField*>(GetFormatField().GetField());
        OSL_ENSURE( pInputField != nullptr,
                "<SwTextInputField::GetContent()> - Missing <SwInputField> instance!" );
        if ( pInputField != nullptr )
        {
            const_cast<SwInputField*>(pInputField)->applyFieldContent( aNewFieldContent );
            // trigger update of fields for scenarios in which the Input Field's content is part of e.g. a table formula
            GetTextNode().GetDoc()->getIDocumentFieldsAccess().GetUpdateFields().SetFieldsDirty(true);
        }
    }
}

void SwTextInputField::UpdateTextNodeContent( const OUString& rNewContent )
{
    if ( !IsFieldInDoc() )
    {
        OSL_ENSURE( false, "<SwTextInputField::UpdateTextNodeContent(..)> - misusage as Input Field is not in document content." );
        return;
    }

    OSL_ENSURE( (*End()) - GetStart() >= 2,
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

::sw::mark::IMark* SwTextAnnotationField::GetAnnotationMark(
    SwDoc* pDoc ) const
{
    const SwPostItField* pPostItField = dynamic_cast<const SwPostItField*>(GetFormatField().GetField());
    OSL_ENSURE( pPostItField != nullptr, "<SwTextAnnotationField::GetAnnotationMark()> - field missing" );
    if ( pPostItField == nullptr )
    {
        return nullptr;
    }

    if ( pDoc == nullptr )
    {
        pDoc = static_cast<const SwPostItFieldType*>(pPostItField->GetTyp())->GetDoc();
    }
    OSL_ENSURE( pDoc != nullptr, "<SwTextAnnotationField::GetAnnotationMark()> - missing document" );
    if ( pDoc == nullptr )
    {
        return nullptr;
    }

    IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t pMark = pMarksAccess->findAnnotationMark( pPostItField->GetName() );
    return pMark != pMarksAccess->getAnnotationMarksEnd()
           ? pMark->get()
           : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
