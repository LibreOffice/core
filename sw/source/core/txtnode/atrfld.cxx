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

TYPEINIT3( SwFmtFld, SfxPoolItem, SwClient,SfxBroadcaster)
TYPEINIT1(SwFmtFldHint, SfxHint);

/****************************************************************************
 *
 *  class SwFmtFld
 *
 ****************************************************************************/

    // Konstruktor fuers Default vom Attribut-Pool
SwFmtFld::SwFmtFld()
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwClient( 0 )
    , SfxBroadcaster()
    , pField( 0 )
    , pTxtAttr( 0 )
{
}

SwFmtFld::SwFmtFld( const SwField &rFld )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwClient( rFld.GetTyp() )
    , SfxBroadcaster()
    , pField( 0 )
    , pTxtAttr( 0 )
{
    pField = rFld.CopyField();
}

// #i24434#
// Since Items are used in ItemPool and in default constructed ItemSets with
// full pool range, all items need to be clonable. Thus, this one needed to be
// corrected
SwFmtFld::SwFmtFld( const SwFmtFld& rAttr )
    : SfxPoolItem( RES_TXTATR_FIELD )
    , SwClient()
    , SfxBroadcaster()
    , pField( 0 )
    , pTxtAttr( 0 )
{
    if(rAttr.GetField())
    {
        rAttr.GetField()->GetTyp()->Add(this);
        pField = rAttr.GetField()->CopyField();
    }
}

SwFmtFld::~SwFmtFld()
{
    SwFieldType* pType = pField ? pField->GetTyp() : 0;

    if (pType && pType->Which() == RES_DBFLD)
        pType = 0;  // DB-Feldtypen zerstoeren sich selbst

    Broadcast( SwFmtFldHint( this, SWFMTFLD_REMOVED ) );
    delete pField;

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
void SwFmtFld::SetFld(SwField * _pField)
{
    delete pField;

    pField = _pField;
    Broadcast( SwFmtFldHint( this, SWFMTFLD_CHANGED ) );
}

int SwFmtFld::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( ( pField && ((SwFmtFld&)rAttr).GetField()
               && pField->GetTyp() == ((SwFmtFld&)rAttr).GetField()->GetTyp()
               && pField->GetFormat() == ((SwFmtFld&)rAttr).GetField()->GetFormat() ) )
             || ( !pField && !((SwFmtFld&)rAttr).GetField() );
}

SfxPoolItem* SwFmtFld::Clone( SfxItemPool* ) const
{
    return new SwFmtFld( *this );
}

void SwFmtFld::SwClientNotify( const SwModify&, const SfxHint& rHint )
{
    if( !pTxtAttr )
        return;

    const SwFieldHint* pHint = dynamic_cast<const SwFieldHint*>( &rHint );
    if ( pHint )
    {
        // replace field content by text
        SwPaM* pPaM = pHint->GetPaM();
        SwDoc* pDoc = pPaM->GetDoc();
        const SwTxtNode& rTxtNode = pTxtAttr->GetTxtNode();
        pPaM->GetPoint()->nNode = rTxtNode;
        pPaM->GetPoint()->nContent.Assign( (SwTxtNode*)&rTxtNode, *pTxtAttr->GetStart() );

        OUString const aEntry( GetField()->ExpandField( pDoc->IsClipBoard() ) );
        pPaM->SetMark();
        pPaM->Move( fnMoveForward );
        pDoc->DeleteRange( *pPaM );
        pDoc->InsertString( *pPaM, aEntry );
    }
}

void SwFmtFld::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !pTxtAttr )
        return;

    // don't do anything, especially not expand!
    if( pNew && pNew->Which() == RES_OBJECTDYING )
        return;

    SwTxtNode* pTxtNd = (SwTxtNode*)&pTxtAttr->GetTxtNode();
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
//                    ((SwGetRefField*)GetFld())->UpdateField();
                    dynamic_cast<SwGetRefField*>(GetField())->UpdateField( pTxtAttr );
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
    pTxtAttr->Expand();
}

bool SwFmtFld::GetInfo( SfxPoolItem& rInfo ) const
{
    const SwTxtNode* pTxtNd;
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() ||
        !pTxtAttr || 0 == ( pTxtNd = pTxtAttr->GetpTxtNode() ) ||
        &pTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        return true;

    ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pTxtNd;
    return false;
}


sal_Bool SwFmtFld::IsFldInDoc() const
{
    const SwTxtNode* pTxtNd;
    return pTxtAttr && 0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->GetNodes().IsDocNodes();
}

sal_Bool SwFmtFld::IsProtect() const
{
    const SwTxtNode* pTxtNd;
    return pTxtAttr && 0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            pTxtNd->IsProtect();
}

/*************************************************************************
|*
|*                SwTxtFld::SwTxtFld()
|*
|*    Beschreibung      Attribut fuer automatischen Text, Ctor
|*
*************************************************************************/

SwTxtFld::SwTxtFld(SwFmtFld & rAttr, sal_Int32 const nStartPos,
        bool const bInClipboard)
    : SwTxtAttr( rAttr, nStartPos )
// fdo#39694 the ExpandField here may not give the correct result in all cases,
// but is better than nothing
    , m_aExpand( rAttr.GetField()->ExpandField(bInClipboard) )
    , m_pTxtNode( 0 )
{
    rAttr.pTxtAttr = this;
    SetHasDummyChar(true);
}

SwTxtFld::~SwTxtFld( )
{
    SwFmtFld & rFmtFld( static_cast<SwFmtFld &>(GetAttr()) );
    if (this == rFmtFld.pTxtAttr)
    {
        rFmtFld.pTxtAttr = 0; // #i110140# invalidate!
    }
}

/*************************************************************************
|*
|*                SwTxtFld::Expand()
|*
|*    Beschreibung      exandiert das Feld und tauscht den Text im Node
|*
*************************************************************************/

void SwTxtFld::Expand() const
{
    // Wenn das expandierte Feld sich nicht veraendert hat, wird returnt
    OSL_ENSURE( m_pTxtNode, "SwTxtFld: where is my TxtNode?" );

    const SwField* pFld = GetFmtFld().GetField();
    OUString aNewExpand(
        pFld->ExpandField(m_pTxtNode->GetDoc()->IsClipBoard()) );

    if( aNewExpand == m_aExpand )
    {
        // Bei Seitennummernfeldern
        const sal_uInt16 nWhich = pFld->GetTyp()->Which();
        if( RES_CHAPTERFLD != nWhich && RES_PAGENUMBERFLD != nWhich &&
            RES_REFPAGEGETFLD != nWhich &&
            // #122919# Page count fields to not use aExpand
            // during formatting, therefore an invalidation of the text frame
            // has to be triggered even if aNewExpand == aExpand:
            ( RES_DOCSTATFLD != nWhich || DS_PAGE != static_cast<const SwDocStatField*>(pFld)->GetSubType() ) &&
            ( RES_GETEXPFLD != nWhich || ((SwGetExpField*)pFld)->IsInBodyTxt() ) )
        {
            // BP: das muesste man noch optimieren!
            //JP 12.06.97: stimmt, man sollte auf jedenfall eine Status-
            //              aenderung an die Frames posten
            if( m_pTxtNode->CalcHiddenParaField() )
            {
                m_pTxtNode->ModifyNotification( 0, 0 );
            }
            return;
        }
    }

    m_aExpand = aNewExpand;

    // 0, this for formatting
    m_pTxtNode->ModifyNotification( 0, const_cast<SwFmtFld*>( &GetFmtFld() ) );
}

/*************************************************************************
 *                      SwTxtFld::CopyFld()
 *************************************************************************/

void SwTxtFld::CopyFld( SwTxtFld *pDest ) const
{
    OSL_ENSURE( m_pTxtNode, "SwTxtFld: where is my TxtNode?" );
    OSL_ENSURE( pDest->m_pTxtNode, "SwTxtFld: where is pDest's TxtNode?" );

    IDocumentFieldsAccess* pIDFA = m_pTxtNode->getIDocumentFieldsAccess();
    IDocumentFieldsAccess* pDestIDFA = pDest->m_pTxtNode->getIDocumentFieldsAccess();

    SwFmtFld& rFmtFld = (SwFmtFld&)pDest->GetFmtFld();
    const sal_uInt16 nFldWhich = rFmtFld.GetField()->GetTyp()->Which();

    if( pIDFA != pDestIDFA )
    {
        // Die Hints stehen in unterschiedlichen Dokumenten,
        // der Feldtyp muss im neuen Dokument angemeldet werden.
        // Z.B: Kopieren ins ClipBoard.
        SwFieldType* pFldType;
        if( nFldWhich != RES_DBFLD && nFldWhich != RES_USERFLD &&
            nFldWhich != RES_SETEXPFLD && nFldWhich != RES_DDEFLD &&
            RES_AUTHORITY != nFldWhich )
            pFldType = pDestIDFA->GetSysFldType( nFldWhich );
        else
            pFldType = pDestIDFA->InsertFldType( *rFmtFld.GetField()->GetTyp() );

        // Sonderbehandlung fuer DDE-Felder
        if( RES_DDEFLD == nFldWhich )
        {
            if( rFmtFld.GetTxtFld() )
                ((SwDDEFieldType*)rFmtFld.GetField()->GetTyp())->DecRefCnt();
            ((SwDDEFieldType*)pFldType)->IncRefCnt();
        }

        OSL_ENSURE( pFldType, "unbekannter FieldType" );
        pFldType->Add( &rFmtFld );          // ummelden
        rFmtFld.GetField()->ChgTyp( pFldType );
    }

    // Expressionfelder Updaten
    if( nFldWhich == RES_SETEXPFLD || nFldWhich == RES_GETEXPFLD ||
        nFldWhich == RES_HIDDENTXTFLD )
    {
        SwTxtFld* pFld = (SwTxtFld*)this;
        pDestIDFA->UpdateExpFlds( pFld, true );
    }
    // Tabellenfelder auf externe Darstellung
    else if( RES_TABLEFLD == nFldWhich &&
        ((SwTblField*)rFmtFld.GetField())->IsIntrnlName() )
    {
        // erzeuge aus der internen (fuer CORE) die externe (fuer UI) Formel
        const SwTableNode* pTblNd = m_pTxtNode->FindTableNode();
        if( pTblNd )        // steht in einer Tabelle
            ((SwTblField*)rFmtFld.GetField())->PtrToBoxNm( &pTblNd->GetTable() );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
