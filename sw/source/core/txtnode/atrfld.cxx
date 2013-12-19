/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "fldbas.hxx"          // fuer FieldType
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <txtannotationfld.hxx>
#include <docufld.hxx>
#include <doc.hxx>

#include "reffld.hxx"
#include "ddefld.hxx"
#include "usrfld.hxx"
#include "expfld.hxx"
#include "swfont.hxx"       // fuer GetFldsColor
#include "ndtxt.hxx"        // SwTxtNode
#include "calc.hxx"         // Update fuer UserFields
#include "hints.hxx"
#include <IDocumentFieldsAccess.hxx>
#include <fieldhint.hxx>
#include <svl/smplhint.hxx>

TYPEINIT3( SwFmtFld, SfxPoolItem, SwClient,SfxBroadcaster)
TYPEINIT1(SwFmtFldHint, SfxHint);

/****************************************************************************
 *
 *  class SwFmtFld
 *
 ****************************************************************************/

// constructor for default item in attribute-pool
SwFmtFld::SwFmtFld( sal_uInt16 nWhich )
    : SfxPoolItem( nWhich )
    , SwClient()
    , SfxBroadcaster()
    , mpField( NULL )
    , mpTxtFld( NULL )
{
}

SwFmtFld::SwFmtFld( const SwField &rFld )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwClient( rFld.GetTyp() )
    , SfxBroadcaster()
    , mpField( rFld.CopyField() )
    , mpTxtFld( NULL )
{
    if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
    {
        // input field in-place editing
        SetWhich( RES_TXTATR_INPUTFIELD );
        dynamic_cast<SwInputField*>(GetField())->SetFmtFld( *this );
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
    , SwClient()
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
            dynamic_cast<SwInputField*>(GetField())->SetFmtFld( *this );
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

    Broadcast( SwFmtFldHint( this, SWFMTFLD_REMOVED ) );
    delete mpField;

    // bei einige FeldTypen muessen wir den FeldTypen noch loeschen
    if( pType && pType->IsLastDepend() )
    {
        sal_Bool bDel = sal_False;
        switch( pType->Which() )
        {
        case RES_USERFLD:
            bDel = ((SwUserFieldType*)pType)->IsDeleted();
            break;

        case RES_SETEXPFLD:
            bDel = ((SwSetExpFieldType*)pType)->IsDeleted();
            break;

        case RES_DDEFLD:
            bDel = ((SwDDEFieldType*)pType)->IsDeleted();
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
    if (NULL != mpField)
        delete mpField;

    mpField = _pField;
    if ( GetField()->GetTyp()->Which() == RES_INPUTFLD )
    {
        dynamic_cast<SwInputField* >(GetField())->SetFmtFld( *this );
    }
    Broadcast( SwFmtFldHint( this, SWFMTFLD_CHANGED ) );
}

void SwFmtFld::SetTxtFld( SwTxtFld& rTxtFld )
{
    mpTxtFld = &rTxtFld;
}

void SwFmtFld::ClearTxtFld()
{
    mpTxtFld = NULL;
}

int SwFmtFld::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( ( mpField && ((SwFmtFld&)rAttr).GetField()
               && mpField->GetTyp() == ((SwFmtFld&)rAttr).GetField()->GetTyp()
               && mpField->GetFormat() == ((SwFmtFld&)rAttr).GetField()->GetFormat() ) )
             || ( !mpField && !((SwFmtFld&)rAttr).GetField() );
}

SfxPoolItem* SwFmtFld::Clone( SfxItemPool* ) const
{
    return new SwFmtFld( *this );
}

void SwFmtFld::SwClientNotify( const SwModify&, const SfxHint& rHint )
{
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
        pPaM->GetPoint()->nContent.Assign( (SwTxtNode*)&rTxtNode, *mpTxtFld->GetStart() );

        String const aEntry( GetField()->ExpandField( pDoc->IsClipBoard() ) );
        pPaM->SetMark();
        pPaM->Move( fnMoveForward );
        pDoc->DeleteRange( *pPaM );
        pDoc->InsertString( *pPaM, aEntry );
    }
}

void SwFmtFld::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !mpTxtFld )
        return;

    // don't do anything, especially not expand!
    if( pNew && pNew->Which() == RES_OBJECTDYING )
        return;

    SwTxtNode* pTxtNd = (SwTxtNode*)&mpTxtFld->GetTxtNode();
    ASSERT( pTxtNd, "wo ist denn mein Node?" );
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
                    // --> OD 2007-09-06 #i81002#
//                    ((SwGetRefField*)GetFld())->UpdateField();
                    dynamic_cast<SwGetRefField*>(GetField())->UpdateField( mpTxtFld );
                    // <--
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
        SwUserFieldType* pType = (SwUserFieldType*)GetField()->GetTyp();
        if(!pType->IsValid())
        {
            SwCalc aCalc( *pTxtNd->GetDoc() );
            pType->GetValue( aCalc );
        }
    }
    mpTxtFld->ExpandTxtFld();
}

sal_Bool SwFmtFld::GetInfo( SfxPoolItem& rInfo ) const
{
    const SwTxtNode* pTxtNd;
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() ||
        !mpTxtFld || 0 == ( pTxtNd = mpTxtFld->GetpTxtNode() ) ||
        &pTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        return sal_True;

    ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pTxtNd;
    return sal_False;
}


bool SwFmtFld::IsFldInDoc() const
{
    return mpTxtFld != NULL
           && mpTxtFld->IsFldInDoc();
}

sal_Bool SwFmtFld::IsProtect() const
{
    return mpTxtFld != NULL
           && mpTxtFld->GetpTxtNode() != NULL
           && mpTxtFld->GetpTxtNode()->IsProtect();
}




SwTxtFld::SwTxtFld(
    SwFmtFld & rAttr,
    xub_StrLen const nStartPos )
    : SwTxtAttr( rAttr, nStartPos )
    , m_aExpand( rAttr.GetField()->ExpandField(true) )
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

void SwTxtFld::ExpandTxtFld() const
{
    ASSERT( m_pTxtNode, "SwTxtFld: where is my TxtNode?" );

    const SwField* pFld = GetFmtFld().GetField();
    const XubString aNewExpand( pFld->ExpandField(m_pTxtNode->GetDoc()->IsClipBoard()) );

    if( aNewExpand == m_aExpand )
    {
        // Bei Seitennummernfeldern
        const sal_uInt16 nWhich = pFld->GetTyp()->Which();
        if ( RES_CHAPTERFLD != nWhich
             && RES_PAGENUMBERFLD != nWhich
             && RES_REFPAGEGETFLD != nWhich
             // Page count fields to not use aExpand during formatting,
             // therefore an invalidation of the text frame has to be triggered even if aNewExpand == aExpand:
             && ( RES_DOCSTATFLD != nWhich || DS_PAGE != static_cast<const SwDocStatField*>(pFld)->GetSubType() )
             && ( RES_GETEXPFLD != nWhich || ((SwGetExpField*)pFld)->IsInBodyTxt() ) )
        {
            if( m_pTxtNode->CalcHiddenParaField() )
            {
                m_pTxtNode->ModifyNotification( 0, 0 );
            }
            return;
        }
    }

    m_aExpand = aNewExpand;

    const_cast<SwTxtFld*>(this)->NotifyContentChange( const_cast<SwFmtFld&>(GetFmtFld()) );
}


void SwTxtFld::CopyTxtFld( SwTxtFld *pDest ) const
{
    ASSERT( m_pTxtNode, "SwTxtFld: where is my TxtNode?" );
    ASSERT( pDest->m_pTxtNode, "SwTxtFld: where is pDest's TxtNode?" );

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
                ((SwDDEFieldType*)rDestFmtFld.GetField()->GetTyp())->DecRefCnt();
            }
            ((SwDDEFieldType*)pFldType)->IncRefCnt();
        }

        ASSERT( pFldType, "unbekannter FieldType" );
        pFldType->Add( &rDestFmtFld );          // ummelden
        rDestFmtFld.GetField()->ChgTyp( pFldType );
    }

    // Expressionfelder Updaten
    if( nFldWhich == RES_SETEXPFLD
        || nFldWhich == RES_GETEXPFLD
        || nFldWhich == RES_HIDDENTXTFLD )
    {
        SwTxtFld* pFld = (SwTxtFld*)this;
        pDestIDFA->UpdateExpFlds( pFld, true );
    }
    // Tabellenfelder auf externe Darstellung
    else if( RES_TABLEFLD == nFldWhich
             && ((SwTblField*)rDestFmtFld.GetField())->IsIntrnlName() )
    {
        // erzeuge aus der internen (fuer CORE) die externe (fuer UI) Formel
        const SwTableNode* pTblNd = m_pTxtNode->FindTableNode();
        if( pTblNd )        // steht in einer Tabelle
            ((SwTblField*)rDestFmtFld.GetField())->PtrToBoxNm( &pTblNd->GetTable() );
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


// input field in-place editing
SwTxtInputFld::SwTxtInputFld(
    SwFmtFld & rAttr,
    xub_StrLen const nStart,
    xub_StrLen const nEnd )

    : SwTxtFld( rAttr, nStart )
    , m_nEnd( nEnd )
{
    SetHasDummyChar( false );
    SetHasContent( true );

    SetDontExpand( true );
    SetLockExpandFlag( true );
    SetDontExpandStartAttr( true );

    SetNesting( true );
}

SwTxtInputFld::~SwTxtInputFld()
{
}

xub_StrLen* SwTxtInputFld::GetEnd()
{
    return &m_nEnd;
}

void SwTxtInputFld::NotifyContentChange( SwFmtFld& rFmtFld )
{
    SwTxtFld::NotifyContentChange( rFmtFld );

    UpdateTextNodeContent( GetFieldContent() );
}

const String SwTxtInputFld::GetFieldContent() const
{
    return GetFmtFld().GetField()->ExpandField(false);
}

void SwTxtInputFld::UpdateFieldContent()
{
    if ( IsFldInDoc()
         && (*GetStart()) != (*End()) )
    {
        ASSERT( (*End()) - (*GetStart()) >= 2,
                "<SwTxtInputFld::UpdateFieldContent()> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
        // skip CH_TXT_ATR_INPUTFIELDSTART character
        const xub_StrLen nIdx = (*GetStart()) + 1;
        // skip CH_TXT_ATR_INPUTFIELDEND character
        const xub_StrLen nLen = static_cast<xub_StrLen>(std::max( 0, ( (*End()) - 1 - nIdx ) ));
        const String aNewFieldContent = GetTxtNode().GetExpandTxt( nIdx, nLen );

        const SwInputField* pInputFld = dynamic_cast<const SwInputField*>(GetFmtFld().GetField());
        ASSERT( pInputFld != NULL,
                "<SwTxtInputFld::GetContent()> - Missing <SwInputFld> instance!" );
        if ( pInputFld != NULL )
        {
            const_cast<SwInputField*>(pInputFld)->applyFieldContent( aNewFieldContent );
        }
    }
}

void SwTxtInputFld::UpdateTextNodeContent( const String& rNewContent )
{
    if ( !IsFldInDoc() )
    {
        ASSERT( false, "<SwTxtInputFld::UpdateTextNodeContent(..)> - misusage as Input Field is not in document content." );
        return;
    }

    ASSERT( (*End()) - (*GetStart()) >= 2,
            "<SwTxtInputFld::UpdateTextNodeContent(..)> - Are CH_TXT_ATR_INPUTFIELDSTART and/or CH_TXT_ATR_INPUTFIELDEND missing?" );
    // skip CH_TXT_ATR_INPUTFIELDSTART character
    const xub_StrLen nIdx = (*GetStart()) + 1;
    // skip CH_TXT_ATR_INPUTFIELDEND character
    const xub_StrLen nDelLen = static_cast<xub_StrLen>(std::max( 0, ( (*End()) - 1 - nIdx ) ));
    SwIndex aIdx( &GetTxtNode(), nIdx );
    GetTxtNode().ReplaceText( aIdx, nDelLen, rNewContent );
}




// text annotation field
SwTxtAnnotationFld::SwTxtAnnotationFld(
    SwFmtFld & rAttr,
    xub_StrLen const nStart )
    : SwTxtFld( rAttr, nStart )
{
}

SwTxtAnnotationFld::~SwTxtAnnotationFld()
{
}


::sw::mark::IMark* SwTxtAnnotationFld::GetAnnotationMark(
    SwDoc* pDoc ) const
{
    const SwPostItField* pPostItField = dynamic_cast<const SwPostItField*>(GetFmtFld().GetField());
    ASSERT( pPostItField != NULL, "<SwTxtAnnotationFld::GetAnnotationMark()> - field missing" );
    if ( pPostItField == NULL )
    {
        return NULL;
    }

    if ( pDoc == NULL )
    {
        pDoc = static_cast<const SwPostItFieldType*>(pPostItField->GetTyp())->GetDoc();
    }
    ASSERT( pDoc != NULL, "<SwTxtAnnotationFld::GetAnnotationMark()> - missing document" );
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

