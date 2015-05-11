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

#include "fldbas.hxx"
#include <fmtfld.hxx>
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

TYPEINIT3(SwFmtFld, SfxPoolItem, SwModify, SfxBroadcaster)

// constructor for default item in attribute-pool
SwFmtFld::SwFmtFld( sal_uInt16 nWhich )
    : SfxPoolItem( nWhich )
    , SwModify(0)
    , SfxBroadcaster()
    , mpField( NULL )
    , mpTxtFld( NULL )
{
}

SwFmtFld::SwFmtFld( const SwField &rFld )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwModify( rFld.GetTyp() )
    , SfxBroadcaster()
    , mpField( rFld.CopyField() )
    , mpTxtFld( NULL )
{
    if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
    {
        // input field in-place editing
        SetWhich( RES_TXTATR_INPUTFIELD );
        static_cast<SwInputField*>(GetField())->SetFmtFld( *this );
    }
    else if (GetField()->GetTyp()->Which() == RES_SETEXPFLD)
    {
        // see SwWrtShell::StartInputFldDlg
        static_cast<SwSetExpField *>(GetField())->SetFmtFld(*this);
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
SwFmtFld::SwFmtFld( const SwFmtFld& rAttr )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwModify(0)
    , SfxBroadcaster()
    , mpField( NULL )
    , mpTxtFld( NULL )
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
                pField->SetFmtFld( *this );
        }
        else if (GetField()->GetTyp()->Which() == RES_SETEXPFLD)
        {
            // see SwWrtShell::StartInputFldDlg
            static_cast<SwSetExpField *>(GetField())->SetFmtFld(*this);
        }
        else if ( GetField()->GetTyp()->Which() == RES_POSTITFLD )
        {
            // text annotation field
            SetWhich( RES_TXTATR_ANNOTATION );
        }
    }
}

SwFmtFld::~SwFmtFld()
{
    SwFieldType* pType = mpField ? mpField->GetTyp() : 0;

    if (pType && pType->Which() == RES_DBFLD)
        pType = 0;  // DB-Feldtypen zerstoeren sich selbst

    Broadcast( SwFmtFldHint( this, SwFmtFldHintWhich::REMOVED ) );
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

void SwFmtFld::RegisterToFieldType( SwFieldType& rType )
{
    rType.Add(this);
}

// #111840#
void SwFmtFld::SetField(SwField * _pField)
{
    delete mpField;

    mpField = _pField;
    if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
    {
        static_cast<SwInputField* >(GetField())->SetFmtFld( *this );
    }
    else if (GetField()->GetTyp()->Which() == RES_SETEXPFLD)
    {
        // see SwWrtShell::StartInputFldDlg
        static_cast<SwSetExpField *>(GetField())->SetFmtFld(*this);
    }
    Broadcast( SwFmtFldHint( this, SwFmtFldHintWhich::CHANGED ) );
}

void SwFmtFld::SetTxtFld( SwTxtFld& rTxtFld )
{
    mpTxtFld = &rTxtFld;
}

void SwFmtFld::ClearTxtFld()
{
    mpTxtFld = NULL;
}

bool SwFmtFld::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( ( mpField && static_cast<const SwFmtFld&>(rAttr).GetField()
               && mpField->GetTyp() == static_cast<const SwFmtFld&>(rAttr).GetField()->GetTyp()
               && mpField->GetFormat() == static_cast<const SwFmtFld&>(rAttr).GetField()->GetFormat() ) )
             || ( !mpField && !static_cast<const SwFmtFld&>(rAttr).GetField() );
}

SfxPoolItem* SwFmtFld::Clone( SfxItemPool* ) const
{
    return new SwFmtFld( *this );
}

void SwFmtFld::InvalidateField()
{
    SwPtrMsgPoolItem const item(RES_REMOVE_UNO_OBJECT,
            &static_cast<SwModify&>(*this)); // cast to base class (void*)
    NotifyClients(&item, &item);
}

void SwFmtFld::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    SwClient::SwClientNotify(rModify, rHint);
    if( !mpTxtFld )
        return;

    const SwFieldHint* pHint = dynamic_cast<const SwFieldHint*>( &rHint );
    if ( pHint )
    {
        // replace field content by text
        SwPaM* pPaM = pHint->GetPaM();
        SwDoc* pDoc = pPaM->GetDoc();
        const SwTxtNode& rTxtNode = mpTxtFld->GetTxtNode();
        pPaM->GetPoint()->nNode = rTxtNode;
        pPaM->GetPoint()->nContent.Assign( const_cast<SwTxtNode*>(&rTxtNode), mpTxtFld->GetStart() );

        OUString const aEntry( GetField()->ExpandField( pDoc->IsClipBoard() ) );
        pPaM->SetMark();
        pPaM->Move( fnMoveForward );
        pDoc->getIDocumentContentOperations().DeleteRange( *pPaM );
        pDoc->getIDocumentContentOperations().InsertString( *pPaM, aEntry );
    }
}

void SwFmtFld::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached UNO object
        SetXTextField(css::uno::Reference<css::text::XTextField>(0));
        // ??? why does this Modify method not already do this?
        NotifyClients(pOld, pNew);
        return;
    }

    if( !mpTxtFld )
        return;

    // don't do anything, especially not expand!
    if( pNew && pNew->Which() == RES_OBJECTDYING )
        return;

    SwTxtNode* pTxtNd = &mpTxtFld->GetTxtNode();
    OSL_ENSURE( pTxtNd, "wo ist denn mein Node?" );
    if( pNew )
    {
        switch( pNew->Which() )
        {
        case RES_TXTATR_FLDCHG:
                // "Farbe hat sich geaendert !"
                // this, this fuer "nur Painten"
                pTxtNd->ModifyNotification( this, this );
                return;
        case RES_REFMARKFLD_UPDATE:
                // GetReferenz-Felder aktualisieren
                if( RES_GETREFFLD == GetField()->GetTyp()->Which() )
                {
                    // #i81002#
                    static_cast<SwGetRefField*>(GetField())->UpdateField( mpTxtFld );
                }
                break;
        case RES_DOCPOS_UPDATE:
                // Je nach DocPos aktualisieren (SwTxtFrm::Modify())
                pTxtNd->ModifyNotification( pNew, this );
                return;

        case RES_ATTRSET_CHG:
        case RES_FMT_CHG:
                pTxtNd->ModifyNotification( pOld, pNew );
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
        case RES_DBSETNUMBERFLD:
        case RES_DBNUMSETFLD:
        case RES_DBNEXTSETFLD:
        case RES_DBNAMEFLD:
            pTxtNd->ModifyNotification( 0, pNew);
            return;
    }

    if( RES_USERFLD == GetField()->GetTyp()->Which() )
    {
        SwUserFieldType* pType = static_cast<SwUserFieldType*>(GetField()->GetTyp());
        if(!pType->IsValid())
        {
            SwCalc aCalc( *pTxtNd->GetDoc() );
            pType->GetValue( aCalc );
        }
    }

    const bool bForceNotify = (pOld == NULL) && (pNew == NULL);
    mpTxtFld->ExpandTxtFld( bForceNotify );
}

bool SwFmtFld::GetInfo( SfxPoolItem& rInfo ) const
{
    const SwTxtNode* pTxtNd;
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() ||
        !mpTxtFld || 0 == ( pTxtNd = mpTxtFld->GetpTxtNode() ) ||
        &pTxtNd->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
        return true;

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = pTxtNd;
    return false;
}

bool SwFmtFld::IsFldInDoc() const
{
    return mpTxtFld != NULL
           && mpTxtFld->IsFldInDoc();
}

bool SwFmtFld::IsProtect() const
{
    return mpTxtFld != NULL
           && mpTxtFld->GetpTxtNode() != NULL
           && mpTxtFld->GetpTxtNode()->IsProtect();
}

// class SwTxtFld ////////////////////////////////////////////////////

SwTxtFld::SwTxtFld(
    SwFmtFld & rAttr,
    sal_Int32 const nStartPos,
    bool const bInClipboard)
    : SwTxtAttr( rAttr, nStartPos )
// fdo#39694 the ExpandField here may not give the correct result in all cases,
// but is better than nothing
    , m_aExpand( rAttr.GetField()->ExpandField(bInClipboard) )
    , m_pTxtNode( NULL )
{
    rAttr.SetTxtFld( *this );
    SetHasDummyChar(true);
}

SwTxtFld::~SwTxtFld( )
{
    SwFmtFld & rFmtFld( static_cast<SwFmtFld &>(GetAttr()) );
    if ( this == rFmtFld.GetTxtFld() )
    {
        rFmtFld.ClearTxtFld();
    }
}

bool SwTxtFld::IsFldInDoc() const
{
    return GetpTxtNode() != NULL
           && GetpTxtNode()->GetNodes().IsDocNodes();
}

void SwTxtFld::ExpandTxtFld(const bool bForceNotify) const
{
    OSL_ENSURE( m_pTxtNode, "SwTxtFld: where is my TxtNode?" );

    const SwField* pFld = GetFmtFld().GetField();
    const OUString aNewExpand( pFld->ExpandField(m_pTxtNode->GetDoc()->IsClipBoard()) );

    if (aNewExpand == m_aExpand)
    {
        // Bei Seitennummernfeldern
        const sal_uInt16 nWhich = pFld->GetTyp()->Which();
        if ( RES_CHAPTERFLD != nWhich
             && RES_PAGENUMBERFLD != nWhich
             && RES_REFPAGEGETFLD != nWhich
             // Page count fields to not use aExpand during formatting,
             // therefore an invalidation of the text frame has to be triggered even if aNewExpand == aExpand:
             && ( RES_DOCSTATFLD != nWhich || DS_PAGE != static_cast<const SwDocStatField*>(pFld)->GetSubType() )
             && ( RES_GETEXPFLD != nWhich || static_cast<const SwGetExpField*>(pFld)->IsInBodyTxt() ) )
        {
            if( m_pTxtNode->CalcHiddenParaField() )
            {
                m_pTxtNode->ModifyNotification( 0, 0 );
            }
            if ( !bForceNotify )
            {
                // done, if no further notification forced.
                return;
            }
        }
    }

    m_aExpand = aNewExpand;

    const_cast<SwTxtFld*>(this)->NotifyContentChange( const_cast<SwFmtFld&>(GetFmtFld()) );
}

void SwTxtFld::CopyTxtFld( SwTxtFld *pDest ) const
{
    OSL_ENSURE( m_pTxtNode, "SwTxtFld: where is my TxtNode?" );
    OSL_ENSURE( pDest->m_pTxtNode, "SwTxtFld: where is pDest's TxtNode?" );

    IDocumentFieldsAccess* pIDFA = m_pTxtNode->getIDocumentFieldsAccess();
    IDocumentFieldsAccess* pDestIDFA = pDest->m_pTxtNode->getIDocumentFieldsAccess();

    SwFmtFld& rDestFmtFld = (SwFmtFld&)pDest->GetFmtFld();
    const sal_uInt16 nFldWhich = rDestFmtFld.GetField()->GetTyp()->Which();

    if( pIDFA != pDestIDFA )
    {
        // Die Hints stehen in unterschiedlichen Dokumenten,
        // der Feldtyp muss im neuen Dokument angemeldet werden.
        // Z.B: Kopieren ins ClipBoard.
        SwFieldType* pFldType;
        if( nFldWhich != RES_DBFLD
            && nFldWhich != RES_USERFLD
            && nFldWhich != RES_SETEXPFLD
            && nFldWhich != RES_DDEFLD
            && RES_AUTHORITY != nFldWhich )
        {
            pFldType = pDestIDFA->GetSysFldType( nFldWhich );
        }
        else
        {
            pFldType = pDestIDFA->InsertFldType( *rDestFmtFld.GetField()->GetTyp() );
        }

        // Sonderbehandlung fuer DDE-Felder
        if( RES_DDEFLD == nFldWhich )
        {
            if( rDestFmtFld.GetTxtFld() )
            {
                static_cast<SwDDEFieldType*>(rDestFmtFld.GetField()->GetTyp())->DecRefCnt();
            }
            static_cast<SwDDEFieldType*>(pFldType)->IncRefCnt();
        }

        OSL_ENSURE( pFldType, "unbekannter FieldType" );
        pFldType->Add( &rDestFmtFld );          // ummelden
        rDestFmtFld.GetField()->ChgTyp( pFldType );
    }

    // Expressionfelder Updaten
    if( nFldWhich == RES_SETEXPFLD
        || nFldWhich == RES_GETEXPFLD
        || nFldWhich == RES_HIDDENTXTFLD )
    {
        SwTxtFld* pFld = const_cast<SwTxtFld*>(this);
        pDestIDFA->UpdateExpFlds( pFld, true );
    }
    // Tabellenfelder auf externe Darstellung
    else if( RES_TABLEFLD == nFldWhich
             && static_cast<SwTblField*>(rDestFmtFld.GetField())->IsIntrnlName() )
    {
        // erzeuge aus der internen (fuer CORE) die externe (fuer UI) Formel
        const SwTableNode* pTblNd = m_pTxtNode->FindTableNode();
        if( pTblNd )        // steht in einer Tabelle
            static_cast<SwTblField*>(rDestFmtFld.GetField())->PtrToBoxNm( &pTblNd->GetTable() );
    }
}

void SwTxtFld::NotifyContentChange(SwFmtFld& rFmtFld)
{
    //if not in undo section notify the change
    if (m_pTxtNode && m_pTxtNode->GetNodes().IsDocNodes())
    {
        m_pTxtNode->ModifyNotification(0, &rFmtFld);
    }
}

/*static*/
void SwTxtFld::GetPamForTxtFld(
    const SwTxtFld& rTxtFld,
    boost::shared_ptr< SwPaM >& rPamForTxtFld )
{
    if (rTxtFld.GetpTxtNode() == NULL)
    {
        SAL_WARN("sw.core", "<SwTxtFld::GetPamForField> - missing <SwTxtNode>");
        return;
    }

    const SwTxtNode& rTxtNode = rTxtFld.GetTxtNode();

    rPamForTxtFld.reset( new SwPaM( rTxtNode,
                                    (rTxtFld.End() != NULL) ? *(rTxtFld.End()) : ( rTxtFld.GetStart() + 1 ),
                                    rTxtNode,
                                    rTxtFld.GetStart() ) );

}

/*static*/
void SwTxtFld::DeleteTxtFld( const SwTxtFld& rTxtFld )
{
    if (rTxtFld.GetpTxtNode() != NULL)
    {
        boost::shared_ptr< SwPaM > pPamForTxtFld;
        GetPamForTxtFld(rTxtFld, pPamForTxtFld);
        if (pPamForTxtFld.get() != NULL)
        {
            rTxtFld.GetTxtNode().GetDoc()->getIDocumentContentOperations().DeleteAndJoin(*pPamForTxtFld);
        }
    }
}

// class SwTxtInputFld ///////////////////////////////////////////////

// input field in-place editing
SwTxtInputFld::SwTxtInputFld(
    SwFmtFld & rAttr,
    sal_Int32 const nStart,
    sal_Int32 const nEnd,
    bool const bInClipboard )

    : SwTxtAttr( rAttr, nStart )
    , SwTxtAttrNesting( rAttr, nStart, nEnd )
    , SwTxtFld( rAttr, nStart, bInClipboard )
    , m_bLockNotifyContentChange( false )
{
    SetHasDummyChar( false );
    SetHasContent( true );
}

SwTxtInputFld::~SwTxtInputFld()
{
}

void SwTxtInputFld::LockNotifyContentChange()
{
    m_bLockNotifyContentChange = true;
}

void SwTxtInputFld::UnlockNotifyContentChange()
{
    m_bLockNotifyContentChange = false;
}

void SwTxtInputFld::NotifyContentChange( SwFmtFld& rFmtFld )
{
    if ( !m_bLockNotifyContentChange )
    {
        LockNotifyContentChange();

        SwTxtFld::NotifyContentChange( rFmtFld );
        UpdateTextNodeContent( GetFieldContent() );

        UnlockNotifyContentChange();
    }
}

const OUString SwTxtInputFld::GetFieldContent() const
{
    return GetFmtFld().GetField()->ExpandField(false);
}

void SwTxtInputFld::UpdateFieldContent()
{
    if ( IsFldInDoc()
         && GetStart() != (*End()) )
    {
        OSL_ENSURE( (*End()) - GetStart() >= 2,
                "<SwTxtInputFld::UpdateFieldContent()> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
        // skip CH_TXT_ATR_INPUTFIELDSTART character
        const sal_Int32 nIdx = GetStart() + 1;
        // skip CH_TXT_ATR_INPUTFIELDEND character
        const sal_Int32 nLen = static_cast<sal_Int32>(std::max<sal_Int32>( 0, ( (*End()) - 1 - nIdx ) ));
        const OUString aNewFieldContent = GetTxtNode().GetExpandTxt( nIdx, nLen );

        const SwInputField* pInputFld = dynamic_cast<const SwInputField*>(GetFmtFld().GetField());
        OSL_ENSURE( pInputFld != NULL,
                "<SwTxtInputFld::GetContent()> - Missing <SwInputFld> instance!" );
        if ( pInputFld != NULL )
        {
            const_cast<SwInputField*>(pInputFld)->applyFieldContent( aNewFieldContent );
            // trigger update of fields for scenarios in which the Input Field's content is part of e.g. a table formula
            GetTxtNode().GetDoc()->getIDocumentFieldsAccess().GetUpdtFlds().SetFieldsDirty(true);
        }
    }
}

void SwTxtInputFld::UpdateTextNodeContent( const OUString& rNewContent )
{
    if ( !IsFldInDoc() )
    {
        OSL_ENSURE( false, "<SwTxtInputFld::UpdateTextNodeContent(..)> - misusage as Input Field is not in document content." );
        return;
    }

    OSL_ENSURE( (*End()) - GetStart() >= 2,
            "<SwTxtInputFld::UpdateTextNodeContent(..)> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
    // skip CH_TXT_ATR_INPUTFIELDSTART character
    const sal_Int32 nIdx = GetStart() + 1;
    // skip CH_TXT_ATR_INPUTFIELDEND character
    const sal_Int32 nDelLen = std::max<sal_Int32>( 0, ( (*End()) - 1 - nIdx ) );
    SwIndex aIdx( &GetTxtNode(), nIdx );
    GetTxtNode().ReplaceText( aIdx, nDelLen, rNewContent );
}

// class SwTxtAnnotationFld //////////////////////////////////////////

// text annotation field
SwTxtAnnotationFld::SwTxtAnnotationFld(
    SwFmtFld & rAttr,
    sal_Int32 const nStart,
    bool const bInClipboard )
    : SwTxtAttr( rAttr, nStart )
    , SwTxtFld( rAttr, nStart, bInClipboard )
{
}

SwTxtAnnotationFld::~SwTxtAnnotationFld()
{
}

::sw::mark::IMark* SwTxtAnnotationFld::GetAnnotationMark(
    SwDoc* pDoc ) const
{
    const SwPostItField* pPostItField = dynamic_cast<const SwPostItField*>(GetFmtFld().GetField());
    OSL_ENSURE( pPostItField != NULL, "<SwTxtAnnotationFld::GetAnnotationMark()> - field missing" );
    if ( pPostItField == NULL )
    {
        return NULL;
    }

    if ( pDoc == NULL )
    {
        pDoc = static_cast<const SwPostItFieldType*>(pPostItField->GetTyp())->GetDoc();
    }
    OSL_ENSURE( pDoc != NULL, "<SwTxtAnnotationFld::GetAnnotationMark()> - missing document" );
    if ( pDoc == NULL )
    {
        return NULL;
    }

    IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t pMark = pMarksAccess->findAnnotationMark( pPostItField->GetName() );
    return pMark != pMarksAccess->getAnnotationMarksEnd()
           ? pMark->get()
           : NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
