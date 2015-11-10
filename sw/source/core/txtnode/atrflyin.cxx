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

#include "hintids.hxx"
#include "cntfrm.hxx"
#include "doc.hxx"
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include "pam.hxx"
#include "flyfrm.hxx"
#include "ndtxt.hxx"
#include "frmfmt.hxx"
#include <fmtflcnt.hxx>
#include <txtflcnt.hxx>
#include <fmtanchr.hxx>
#include "swfont.hxx"
#include "txtfrm.hxx"
#include "flyfrms.hxx"
#include <objectformatter.hxx>
#include <calbck.hxx>

SwFormatFlyCnt::SwFormatFlyCnt( SwFrameFormat *pFrameFormat )
    : SfxPoolItem( RES_TXTATR_FLYCNT ),
    pTextAttr( nullptr ),
    pFormat( pFrameFormat )
{
}

bool SwFormatFlyCnt::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return( pTextAttr && static_cast<const SwFormatFlyCnt&>(rAttr).pTextAttr &&
            pTextAttr->GetStart() == static_cast<const SwFormatFlyCnt&>(rAttr).pTextAttr->GetStart() &&
            pFormat == static_cast<const SwFormatFlyCnt&>(rAttr).GetFrameFormat() );
}

SfxPoolItem* SwFormatFlyCnt::Clone( SfxItemPool* ) const
{
    return new SwFormatFlyCnt( pFormat );
}

SwTextFlyCnt::SwTextFlyCnt( SwFormatFlyCnt& rAttr, sal_Int32 nStartPos )
    : SwTextAttr( rAttr, nStartPos )
{
    rAttr.pTextAttr = this;
    SetHasDummyChar(true);
}

/*
 * An dieser Stelle soll einmal der Gesamtzusammenhang bei der Erzeugung
 * eines neuen SwTextFlyCnt erlaeutert werden.
 * Das MakeTextHint() wird z.B. im SwTextNode::Copy() gerufen.
 * Fuer die komplette Verdopplung sind folgende Schritte notwendig:
 * 1) Duplizieren des pFormat incl. Inhalt, Attributen etc.
 * 2) Setzen des Ankers
 * 3) Benachrichtigung
 * Da fuer die Bewaeltigung der Aufgaben nicht immer alle Informationen
 * bereitstehen und darueber hinaus bestimmte Methoden erst zu einem
 * spaeteren Zeitpunkt gerufen werden duerfen (weil nocht nicht alle
 * Nodeinformationen vorliegen), verteilt sich der Ablauf.
 * ad 1) MakeTextHint() wird durch den Aufruf von SwDoc::CopyLayout()
 * der das neue FlyFrameFormat erzeugt und mit dem duplizierten Inhalt des
 * FlyFrm verbunden.
 * ad 2) SetAnchor() wird von SwTextNode::Insert() gerufen und sorgt fuer das
 * setzen des Ankers (die SwPosition des Dummy-Zeichens wird dem FlyFrameFormat
 * per SetAttr bekannt gegeben). Dies kann nicht im MakeTextHint erledigt
 * werden, da der Zielnode unbestimmt ist.
 * ad 3) _GetFlyFrm() wird im Formatierungsprozess vom LineIter gerufen
 * und sucht den FlyFrm zum Dummyzeichen des aktuellen ContentFrm. Wird keiner
 * gefunden, so wird ein neuer FlyFrm angelegt.
 * Kritisch an diesem Vorgehen ist, dass das pContent->AppendFly() eine
 * sofortige Neuformatierung von pContent anstoesst. Die Rekursion kommt
 * allerdings durch den Lockmechanismus in SwTextFrm::Format() nicht
 * zu stande.
 * Attraktiv ist der Umstand, dass niemand ueber die vom Node abhaengigen
 * ContentFrms iterieren braucht, um die FlyInCntFrm anzulegen. Dies geschieht
 * bei der Arbeit.
 */

void SwTextFlyCnt::CopyFlyFormat( SwDoc* pDoc )
{
    SwFrameFormat* pFormat = GetFlyCnt().GetFrameFormat();
    OSL_ENSURE( pFormat, "von welchem Format soll ich eine Kopie erzeugen?" );
    // Das FlyFrameFormat muss dupliziert werden.
    // In CopyLayoutFormat (siehe doclay.cxx) wird das FlyFrameFormat erzeugt
    // und der Inhalt dupliziert.

    // disable undo while copying attribute
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    SwFormatAnchor aAnchor( pFormat->GetAnchor() );
    if ((FLY_AT_PAGE != aAnchor.GetAnchorId()) &&
        (pDoc != pFormat->GetDoc()))   // different documents?
    {
        // JP 03.06.96: dann sorge dafuer, das der koperierte Anker auf
        //              gueltigen Content zeigt! Die Umsetzung auf die
        //              richtige Position erfolgt spaeter.
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), +2 );
        SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
        if( !pCNd )
            pCNd = pDoc->GetNodes().GoNext( &aIdx );

        SwPosition pos = *aAnchor.GetContentAnchor();
        pos.nNode = aIdx;
        if (FLY_AS_CHAR == aAnchor.GetAnchorId())
        {
            pos.nContent.Assign( pCNd, 0 );
        }
        else
        {
            pos.nContent.Assign( nullptr, 0 );
            OSL_ENSURE( false, "CopyFlyFormat: Was fuer ein Anker?" );
        }
        aAnchor.SetAnchor( &pos );
    }

    SwFrameFormat* pNew = pDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *pFormat, aAnchor, false, false );
    ((SwFormatFlyCnt&)GetFlyCnt()).SetFlyFormat( pNew );
}

// SetAnchor() wird von SwTextNode::Insert() gerufen und sorgt fuer das
// setzen des Ankers (die SwPosition des Dummy-Zeichens wird dem FlyFrameFormat
// per SetAttr bekannt gegeben). Dies kann nicht im MakeTextHint erledigt
// werden, da der Zielnode unbestimmt ist.
// (siehe Kommentar in SwTextFlyCnt::MakeTextHint)
void SwTextFlyCnt::SetAnchor( const SwTextNode *pNode )
{
    // fuers Undo muss der neue Anker schon bekannt sein !

    // Wir ermitteln den Index im Nodesarray zum Node

    SwDoc* pDoc = const_cast<SwDoc*>(pNode->GetDoc());

    SwIndex aIdx( const_cast<SwTextNode*>(pNode), GetStart() );
    SwPosition aPos( *pNode->StartOfSectionNode(), aIdx );
    SwFrameFormat* pFormat = GetFlyCnt().GetFrameFormat();
    SwFormatAnchor aAnchor( pFormat->GetAnchor() );
    SwNode *const pOldNode(aAnchor.GetContentAnchor()
            ? &aAnchor.GetContentAnchor()->nNode.GetNode()
            : nullptr);

    if (!pOldNode || !pOldNode->GetNodes().IsDocNodes() ||
        pOldNode != static_cast<SwNode const *>(pNode))
    {
        aPos.nNode = *pNode;
    }
    else
    {
        aPos.nNode = *pOldNode;
    }

    aAnchor.SetType( FLY_AS_CHAR );        // default!
    aAnchor.SetAnchor( &aPos );

    // beim Ankerwechsel werden immer alle FlyFrms vom Attribut geloescht
    // JP 25.04.95: wird innerhalb des SplitNodes die Frames verschoben
    //              koennen die Frames erhalten bleiben.
    if( ( !pNode->GetpSwpHints() || !pNode->GetpSwpHints()->IsInSplitNode() )
        && RES_DRAWFRMFMT != pFormat->Which() )
        pFormat->DelFrms();

    // stehen wir noch im falschen Dokument ?
    if( pDoc != pFormat->GetDoc() )
    {
        // disable undo while copying attribute
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
        SwFrameFormat* pNew = pDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *pFormat, aAnchor, false, false );

        ::sw::UndoGuard const undoGuardFormat(
            pFormat->GetDoc()->GetIDocumentUndoRedo());
        pFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat( pFormat );
        ((SwFormatFlyCnt&)GetFlyCnt()).SetFlyFormat( pNew );
    }
    else if( pNode->GetpSwpHints() &&
            pNode->GetpSwpHints()->IsInSplitNode() &&
            RES_DRAWFRMFMT != pFormat->Which() )
    {
        pFormat->LockModify();
        pFormat->SetFormatAttr( aAnchor );        // nur den Anker neu setzen
        // tdf#91228 must notify the anchor nodes despite LockModify
        assert(pOldNode);
        pOldNode->RemoveAnchoredFly(pFormat);
        aPos.nNode.GetNode().AddAnchoredFly(pFormat);
        pFormat->UnlockModify();
    }
    else
    {
        assert(!pFormat->IsModifyLocked()); // need to notify anchor node
        pFormat->SetFormatAttr( aAnchor );        // nur den Anker neu setzen
    }

    // Am Node haengen u.a. abhaengige CntFrms.
    // Fuer jeden CntFrm wird ein SwFlyInCntFrm angelegt.
}

// _GetFlyFrm() wird im Formatierungsprozess vom LineIter gerufen
// und sucht den FlyFrm zum Dummyzeichen des aktuellen ContentFrm. Wird keiner
// gefunden, so wird ein neuer FlyFrm angelegt.
// (siehe Kommentar ind SwTextFlyCnt::MakeTextHint)
SwFlyInCntFrm *SwTextFlyCnt::_GetFlyFrm( const SwFrm *pCurrFrm )
{
    SwFrameFormat* pFrameFormat = GetFlyCnt().GetFrameFormat();
    if( RES_DRAWFRMFMT == pFrameFormat->Which() )
    {
        OSL_ENSURE(  false, "SwTextFlyCnt::_GetFlyFrm: DrawInCnt-Baustelle!" );
        return nullptr;
    }

    SwIterator<SwFlyFrm,SwFormat> aIter( *GetFlyCnt().pFormat );
    OSL_ENSURE( pCurrFrm->IsTextFrm(), "SwTextFlyCnt::_GetFlyFrm for TextFrms only." );
    SwFrm* pFrm = aIter.First();
    if ( pFrm )
    {
        SwTextFrm *pFirst = const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pCurrFrm));
        while ( pFirst->IsFollow() )
            pFirst = pFirst->FindMaster();
        do
            {
                SwTextFrm *pTmp = pFirst;
                do
                {   if( static_cast<SwFlyFrm*>(pFrm)->GetAnchorFrm() == static_cast<SwFrm*>(pTmp) )
                    {
                        if ( pTmp != pCurrFrm )
                        {
                            pTmp->RemoveFly( static_cast<SwFlyFrm*>(pFrm) );
                            const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pCurrFrm))->AppendFly( static_cast<SwFlyFrm*>(pFrm) );
                        }
                        return static_cast<SwFlyInCntFrm*>(pFrm);
                    }
                    pTmp = pTmp->GetFollow();
                } while ( pTmp );

                pFrm = aIter.Next();

        } while( pFrm );
    }

    // Wir haben keinen passenden FlyFrm gefunden, deswegen wird ein
    // neuer angelegt.
    // Dabei wird eine sofortige Neuformatierung von pCurrFrm angestossen.
    // Die Rekursion wird durch den Lockmechanismus in SwTextFrm::Format()
    // abgewuergt.
    SwFrm* pCurrFrame = const_cast< SwFrm* >(pCurrFrm);
    SwFlyInCntFrm *pFly = new SwFlyInCntFrm( static_cast<SwFlyFrameFormat*>(pFrameFormat), pCurrFrame, pCurrFrame );
    pCurrFrame->AppendFly( pFly );
    pFly->RegistFlys();

    // 7922: Wir muessen dafuer sorgen, dass der Inhalt des FlyInCnt
    // nach seiner Konstruktion stramm durchformatiert wird.
    // #i26945# - Use new object formatter to format Writer
    // fly frame and its content.
    SwObjectFormatter::FormatObj( *pFly, const_cast<SwFrm*>(pCurrFrm),
                                  pCurrFrm->FindPageFrm() );

    return pFly;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
