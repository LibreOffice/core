/*************************************************************************
 *
 *  $RCSfile: htmlctxt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif

#include "doc.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "paratr.hxx"
#include "htmlnum.hxx"
#include "css1kywd.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

/*  */


class _HTMLAttrContext_SaveDoc
{
    SwHTMLNumRuleInfo aNumRuleInfo; // In Umgebung gueltige Numerierung
    SwPosition  *pPos;              // hierhin beim verlassen den
                                    // Kontexts zurueckgesprungen
    _HTMLAttrTable *pAttrTab;       // In Umgebung gueltige Attribute,
                                    // wenn Attributierung nicht
                                    // beibehalten werden soll.

    USHORT nContextStMin;           // In Umgebung gueltige Stack-
                                    // Untergrenze, wenn der Stack
                                    // geschuetzt werden soll.
    USHORT nContextStAttrMin;       // In Umgebung gueltige Stack-
                                    // Untergrenze, wenn die Attribute
                                    // nicht beibehalten werden sollen.

    BOOL bStripTrailingPara : 1;    // letzen Absatz entfernen?
    BOOL bKeepNumRules : 1;         // Numerierung beibehalten?
    BOOL bPopStack : 1;             // Stack-Elemente oberhalb des
                                    // zu schliessenden entfernen?
    BOOL bFixHeaderDist : 1;
    BOOL bFixFooterDist : 1;

public:

    _HTMLAttrContext_SaveDoc() :
        pPos( 0 ), pAttrTab( 0 ),
        nContextStMin( USHRT_MAX ), nContextStAttrMin( USHRT_MAX ),
        bStripTrailingPara( FALSE ), bKeepNumRules( FALSE ),
        bPopStack( FALSE ),
        bFixHeaderDist( FALSE ), bFixFooterDist( FALSE )
    {}

    ~_HTMLAttrContext_SaveDoc() { delete pPos; delete pAttrTab; }

    // Die Position gehoert uns, muss also angelegt und zerstoert werden
    void SetPos( const SwPosition& rPos ) { pPos = new SwPosition(rPos); }
    const SwPosition *GetPos() const { return pPos; }

    // Der Index gehoert uns nicht. Kein Anlgen und Zerstoeren.
    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; }

    _HTMLAttrTable *GetAttrTab( BOOL bCreate= FALSE );

    void SetContextStMin( USHORT nMin ) { nContextStMin = nMin; }
    USHORT GetContextStMin() const { return nContextStMin; }

    void SetContextStAttrMin( USHORT nMin ) { nContextStAttrMin = nMin; }
    USHORT GetContextStAttrMin() const { return nContextStAttrMin; }

    void SetStripTrailingPara( BOOL bSet ) { bStripTrailingPara = bSet; }
    BOOL GetStripTrailingPara() const { return bStripTrailingPara; }

    void SetKeepNumRules( BOOL bSet ) { bKeepNumRules = bSet; }
    BOOL GetKeepNumRules() const { return bKeepNumRules; }

    void SetFixHeaderDist( BOOL bSet ) { bFixHeaderDist = bSet; }
    BOOL GetFixHeaderDist() const { return bFixHeaderDist; }

    void SetFixFooterDist( BOOL bSet ) { bFixFooterDist = bSet; }
    BOOL GetFixFooterDist() const { return bFixFooterDist; }
};

_HTMLAttrTable *_HTMLAttrContext_SaveDoc::GetAttrTab( BOOL bCreate )
{
    if( !pAttrTab && bCreate )
    {
        pAttrTab = new _HTMLAttrTable;
        memset( pAttrTab, 0, sizeof( _HTMLAttrTable ));
    }
    return pAttrTab;
}

/*  */

_HTMLAttrContext_SaveDoc *_HTMLAttrContext::GetSaveDocContext( BOOL bCreate )
{
    if( !pSaveDocContext && bCreate )
        pSaveDocContext = new _HTMLAttrContext_SaveDoc;

    return pSaveDocContext;
}

void _HTMLAttrContext::ClearSaveDocContext()
{
    delete pSaveDocContext;
    pSaveDocContext = 0;
}

/*  */

void SwHTMLParser::SplitAttrTab( const SwPosition& rNewPos )
{
    // Hier darf es keine vorlauefigen Absatz-Attribute geben, den die
    // koennten jetzt gesetzt werden und dann sind die Zeiger ungueltig!!!
    ASSERT( !aParaAttrs.Count(),
        "Hoechste Gefahr: Es gibt noch nicht-endgueltige Absatz-Attribute" );
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    const SwNodeIndex* pOldEndPara = &pPam->GetPoint()->nNode;
    xub_StrLen nOldEndCnt = pPam->GetPoint()->nContent.GetIndex();

    const SwNodeIndex& rNewSttPara = rNewPos.nNode;
    xub_StrLen nNewSttCnt = rNewPos.nContent.GetIndex();

    BOOL bMoveBack = FALSE;

    // alle noch offenen Attribute beenden und hinter der Tabelle
    // neu aufspannen
    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( USHORT nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
            nCnt--; ++pTbl )
    {
        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            _HTMLAttr *pNext = pAttr->GetNext();
            _HTMLAttr *pPrev = pAttr->GetPrev();

            USHORT nWhich = pAttr->pItem->Which();
            if( !nOldEndCnt && RES_PARATR_BEGIN <= nWhich &&
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() )
            {
                // Das Attribut muss eine Content-Position weiter vorne
                // beendet werden
                if( !bMoveBack )
                {
                    bMoveBack = pPam->Move( fnMoveBackward );
                    nOldEndCnt = pPam->GetPoint()->nContent.GetIndex();
                }
            }
            else if( bMoveBack )
            {
                pPam->Move( fnMoveForward );
                nOldEndCnt = pPam->GetPoint()->nContent.GetIndex();
            }

            if( RES_PARATR_BEGIN <= nWhich && bMoveBack ||
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() ||
                (pAttr->GetSttPara() == *pOldEndPara &&
                 pAttr->GetSttCnt() != nOldEndCnt) )
            {
                // Das Attribut muss gesetzt werden. Da wir
                // das Original noch brauchen, weil Zeiger auf das Attribut
                // noch in den Kontexten existieren, muessen wir es clonen.
                // Die Next-Liste geht dabei verloren, aber die
                // Previous-Liste bleibt erhalten
                _HTMLAttr *pSetAttr = pAttr->Clone( *pOldEndPara, nOldEndCnt );

                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    USHORT nTmp =
                        pSetAttr->bInsAtStart ? 0 : aSetAttrTab.Count();
                    aSetAttrTab.Insert( pSetAttr, nTmp );
                }
            }
            else if( pPrev )
            {
                // Wenn das Attribut nicht gesetzt vor der Tabelle
                // gesetzt werden muss, muessen der Previous-Attribute
                // trotzdem gesetzt werden.
                if( pNext )
                    pNext->InsertPrev( pPrev );
                else
                {
                    USHORT nTmp = pPrev->bInsAtStart ? 0 : aSetAttrTab.Count();
                    aSetAttrTab.Insert( pPrev, nTmp );
                }
            }

            // den Start des Attributs neu setzen
            pAttr->nSttPara = rNewSttPara;
            pAttr->nEndPara = rNewSttPara;
            pAttr->nSttCntnt = nNewSttCnt;
            pAttr->nEndCntnt = nNewSttCnt;
            pAttr->pPrev = 0;

            pAttr = pNext;
        }
    }

    if( bMoveBack )
        pPam->Move( fnMoveForward );

}

void SwHTMLParser::SaveDocContext( _HTMLAttrContext *pCntxt,
                                   USHORT nFlags,
                                   const SwPosition *pNewPos )
{
    _HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext( TRUE );
    pSave->SetStripTrailingPara( (HTML_CNTXT_STRIP_PARA & nFlags) != 0 );
    pSave->SetKeepNumRules( (HTML_CNTXT_KEEP_NUMRULE & nFlags) != 0 );
    pSave->SetFixHeaderDist( (HTML_CNTXT_HEADER_DIST & nFlags) != 0 );
    pSave->SetFixFooterDist( (HTML_CNTXT_FOOTER_DIST & nFlags) != 0 );

    if( pNewPos )
    {
        // Wenn der PaM an eine andere Position gesetzt wird, muss
        // die Numerierung gerettet werden..
        if( !pSave->GetKeepNumRules() )
        {
            // Die Numerierung soll nicht beibehalten werden. Also muss
            // der aktuelle Zustand gerettet und die Numerierung
            // anschliessend ausgeschaltet werden.
            pSave->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
        }
#ifndef NUM_RELSPACE
        else if( GetNumInfo().GetNumRule() && pTable )
        {
            // Die NumRule befindet sich nicht mehr unbedingt in einem
            // zusammenhaengenden Bereich und muss deshalb vollst.
            // aktualisiert werden.
            GetNumInfo().SetUpdateWholeNum( TRUE );
        }
#endif

        if( (HTML_CNTXT_KEEP_ATTRS & nFlags) != 0 )
        {
            // Attribute an aktueller Position beenden und an neuer neu anfangen
            SplitAttrTab( *pNewPos );
        }
        else
        {
            _HTMLAttrTable *pSaveAttrTab = pSave->GetAttrTab( TRUE );
            SaveAttrTab( *pSaveAttrTab );
        }


        pSave->SetPos( *pPam->GetPoint() );
        *pPam->GetPoint() = *pNewPos;
    }

    // Mit dem Setzen von nContextStMin koennen automatisch auch
    // keine gerade offenen Listen (DL/OL/UL) mehr beendet werden.
    if( (HTML_CNTXT_PROTECT_STACK & nFlags) != 0  )
    {
        pSave->SetContextStMin( nContextStMin );
        nContextStMin = aContexts.Count();

        if( (HTML_CNTXT_KEEP_ATTRS & nFlags) == 0 )
        {
            pSave->SetContextStAttrMin( nContextStAttrMin );
            nContextStAttrMin = aContexts.Count();
        }
    }
}

void SwHTMLParser::RestoreDocContext( _HTMLAttrContext *pCntxt )
{
    _HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext();
    if( !pSave )
        return;

    if( pSave->GetStripTrailingPara() )
        StripTrailingPara();

    if( pSave->GetPos() )
    {
        if( pSave->GetFixHeaderDist() || pSave->GetFixFooterDist() )
            FixHeaderFooterDistance( pSave->GetFixHeaderDist(),
                                     pSave->GetPos() );

        _HTMLAttrTable *pSaveAttrTab = pSave->GetAttrTab();
        if( !pSaveAttrTab )
        {
            // Attribute an aktueller Position beenden und an alter neu
            // anfangen.
            SplitAttrTab( *pSave->GetPos() );
        }
        else
        {
            RestoreAttrTab( *pSaveAttrTab );
        }

        *pPam->GetPoint() = *pSave->GetPos();

        // Die bisherigen Attribute koennen wir schonmal setzen.
        SetAttr();
    }

    if( USHRT_MAX != pSave->GetContextStMin() )
    {
        nContextStMin = pSave->GetContextStMin();
        if( USHRT_MAX != pSave->GetContextStAttrMin() )
            nContextStAttrMin = pSave->GetContextStAttrMin();
    }

    if( !pSave->GetKeepNumRules() )
    {
        // Die bisherige gemerkte Numerierung wieder setzen
        GetNumInfo().Set( pSave->GetNumInfo() );
    }
#ifndef NUM_RELSPACE
    else if( GetNumInfo().GetNumRule() && pTable )
    {
        // Die NumRule befindet sich nicht mehr unbedingt in einem
        // zusammenhaengenden Bereich und muss deshalb vollst.
        // aktualisiert werden.
        GetNumInfo().SetUpdateWholeNum( TRUE );
    }
#endif

    pCntxt->ClearSaveDocContext();
}

/*  */

void SwHTMLParser::EndContext( _HTMLAttrContext *pContext )
{
    if( pContext->GetPopStack() )
    {
        // Alle noch offenen Kontexte beenden. Der eigene
        // Kontext muss bereits geloscht sein!
        while( aContexts.Count() > nContextStMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ASSERT( pCntxt != pContext,
                    "Kontext noch im Stack" );
            if( pCntxt == pContext )
                break;

            EndContext( pCntxt );
            delete pCntxt;
        }
    }

    // Alle noch offenen Attribute beenden
    if( pContext->HasAttrs() )
        EndContextAttrs( pContext );

    // Falls ein Bereich geoeffnet wurde, den verlassen. Da Bereiche
    // auch innerhalb von absolut positionierten Objekten angelegt werden,
    // muss das passieren, bever ein alter Dokument-Kontext restauriert wird.
    if( pContext->GetSpansSection() )
        EndSection();

    // Rahmen und sonstige Sonderbereiche verlassen.
    if( pContext->HasSaveDocContext() )
        RestoreDocContext( pContext );

    // Ggf. noch einen Ansatz-Umbruch einfuegen
    if( AM_NONE != pContext->GetAppendMode() &&
        pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( pContext->GetAppendMode() );

    // PRE-/LISTING- und XMP-Umgebungen wieder starten
    if( pContext->IsFinishPREListingXMP() )
        FinishPREListingXMP();

    if( pContext->IsRestartPRE() )
        StartPRE();

    if( pContext->IsRestartXMP() )
        StartXMP();

    if( pContext->IsRestartListing() )
        StartListing();
}

void SwHTMLParser::ClearContext( _HTMLAttrContext *pContext )
{
    _HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( USHORT i=0; i<rAttrs.Count(); i++ )
    {
        // einfaches Loeschen reicht hier nicht, weil das
        // Attribut auch aus seiner Liste ausgetragen werden
        // muss. Theoretisch koennt man natuerlich auch die Liste
        // und die Attribute getrennt loeschen, aber wenn man
        // dann was falsch gemacht hat, sieht es uebel aus.
        DeleteAttr( rAttrs[i] );
    }

    ASSERT( !pContext->GetSpansSection(),
            "Bereich kann nicht mehr verlassen werden" );

    ASSERT( !pContext->HasSaveDocContext(),
            "Rahmen kann nicht mehr verlassen werden" );

    // PRE-/LISTING- und XMP-Umgebungen wieder starten
    if( pContext->IsFinishPREListingXMP() )
        FinishPREListingXMP();

    if( pContext->IsRestartPRE() )
        StartPRE();

    if( pContext->IsRestartXMP() )
        StartXMP();

    if( pContext->IsRestartListing() )
        StartListing();
}

/*  */

BOOL SwHTMLParser::DoPositioning( SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo &rPropInfo,
                                  _HTMLAttrContext *pContext )
{
    BOOL bRet = FALSE;

    // Unter folgenden Umstaenden wird jetzt ein Rahmen aufgemacht:
    // - das Tag wird absolut positioniert und left/top sind beide
    //   gegeben und enthalten auch keine %-Angabe, oder
    // - das Tag soll fliessen, und
    // - es wurde eine Breite angegeben (in beiden Faellen noetig)
    if( SwCSS1Parser::MayBePositioned( rPropInfo ) )
    {
        SfxItemSet aFrmItemSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs(aFrmItemSet );

        // Ausrichtung setzen
        SetAnchorAndAdjustment( VERT_NONE, HORI_NONE, rItemSet, rPropInfo,
                                aFrmItemSet );

        // Groesse setzen
        SetVarSize( rItemSet, rPropInfo, aFrmItemSet );

        // Abstaende setzen
        SetSpace( Size(0,0), rItemSet, rPropInfo, aFrmItemSet );

        // Sonstige CSS1-Attribute Setzen
        SetFrmFmtAttrs( rItemSet, rPropInfo,
                        HTML_FF_BOX|HTML_FF_PADDING|HTML_FF_BACKGROUND,
                        aFrmItemSet );

        InsertFlyFrame( aFrmItemSet, pContext, rPropInfo.aId,
                        CONTEXT_FLAGS_ABSPOS );
        pContext->SetPopStack( TRUE );
        rPropInfo.aId.Erase();
        bRet = TRUE;
    }

    return bRet;
}

BOOL SwHTMLParser::CreateContainer( const String& rClass,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo &rPropInfo,
                                    _HTMLAttrContext *pContext )
{
    BOOL bRet = FALSE;
    if( rClass.EqualsIgnoreCaseAscii(sCSS1_class_abs_pos) &&
        pCSS1Parser->MayBePositioned( rPropInfo ) )
    {
        // Container-Klasse
        SfxItemSet *pFrmItemSet = pContext->GetFrmItemSet( pDoc );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs( *pFrmItemSet );

        SetAnchorAndAdjustment( VERT_NONE, HORI_NONE,
                                rItemSet, rPropInfo, *pFrmItemSet );
        Size aDummy(0,0);
        SetFixSize( aDummy, aDummy, FALSE, FALSE, rItemSet, rPropInfo,
                    *pFrmItemSet );
        SetSpace( aDummy, rItemSet, rPropInfo, *pFrmItemSet );
        SetFrmFmtAttrs( rItemSet, rPropInfo, HTML_FF_BOX|HTML_FF_BACKGROUND,
                        *pFrmItemSet );

        bRet = TRUE;
    }

    return bRet;
}

/*  */

void SwHTMLParser::InsertAttrs( SfxItemSet &rItemSet,
                                SvxCSS1PropertyInfo &rPropInfo,
                                _HTMLAttrContext *pContext,
                                BOOL bCharLvl )
{
    // Ein DropCap-Attribut basteln, wenn auf Zeichen-Ebene vor dem
    // ersten Zeichen ein float: left vorkommt
    if( bCharLvl && !pPam->GetPoint()->nContent.GetIndex() &&
        SVX_ADJUST_LEFT == rPropInfo.eFloat )
    {
        SwFmtDrop aDrop;
        aDrop.GetChars() = 1;

        pCSS1Parser->FillDropCap( aDrop, rItemSet );

        // Nur wenn das Initial auch ueber mehrere Zeilen geht, wird das
        // DropCap-Attribut gesetzt. Sonst setzten wir die Attribute hart.
        if( aDrop.GetLines() > 1 )
        {
            NewAttr( &aAttrTab.pDropCap, aDrop );

            _HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.Insert( aAttrTab.pDropCap, rAttrs.Count() );

            return;
        }
    }

// Feature: PrintExt
    if( !bCharLvl )
        pCSS1Parser->SetFmtBreak( rItemSet, rPropInfo );
// /Feature: PrintExt

    ASSERT( aContexts.Count() <= nContextStAttrMin ||
            aContexts[aContexts.Count()-1] != pContext,
            "SwHTMLParser::InsertAttrs: Kontext doch schon auf dem Stack" );

    SfxItemIter aIter( rItemSet );

    const SfxPoolItem *pItem = aIter.FirstItem();
    while( pItem )
    {
        _HTMLAttr **ppAttr = 0;

        switch( pItem->Which() )
        {
        case RES_LR_SPACE:
            {
                // Absatz-Einzuege muessen addiert werden und werden immer
                // nur absatzweise gesetzt (fuer den ersten Absatz hier,
                // fuer alle folgenden in SetTxtCollAttrs)

                const SvxLRSpaceItem *pLRItem =
                    (const SvxLRSpaceItem *)pItem;

                // die bisherigen Absatz-Abstaende holen (ohne die vom
                // obersten Kontext, denn den veraendern wir ja gerade) ...
                USHORT nOldLeft = 0, nOldRight = 0;
                short nOldIndent = 0;
                BOOL bIgnoreTop = aContexts.Count() > nContextStMin &&
                                  aContexts[aContexts.Count()-1] == pContext;
                GetMarginsFromContext( nOldLeft, nOldRight, nOldIndent,
                                       bIgnoreTop  );


                // und noch die aktuell gueltigen
                USHORT nLeft = nOldLeft, nRight = nOldRight;
                short nIndent = nOldIndent;
                pContext->GetMargins( nLeft, nRight, nIndent );

                // ... und die neuen Abstaende zu den alten addieren
                // Hier werden nicht die aus dem Item genommen, sondern die
                // extra gemerkten, weil die auch negativ sein koennen. Die
                // Abfrage ueber das Item funktioniert aber trotzdem, denn
                // fuer negative Werte wird das Item (mit Wert 0) auch
                // eingefuegt.
                if( rPropInfo.bLeftMargin )
                {
                    ASSERT( rPropInfo.nLeftMargin < 0 ||
                            rPropInfo.nLeftMargin == pLRItem->GetTxtLeft(),
                            "linker Abstand stimmt nicht mit Item ueberein" );
                    if( rPropInfo.nLeftMargin < 0 &&
                        -rPropInfo.nLeftMargin > nOldLeft )
                        nLeft = 0;
                    else
                        nLeft = nOldLeft + rPropInfo.nLeftMargin;
                }
                if( rPropInfo.bRightMargin )
                {
                    ASSERT( rPropInfo.nRightMargin < 0 ||
                            rPropInfo.nRightMargin == pLRItem->GetRight(),
                            "rechter Abstand stimmt nicht mit Item ueberein" );
                    if( rPropInfo.nRightMargin < 0 &&
                        -rPropInfo.nRightMargin > nOldRight )
                        nRight = 0;
                    else
                        nRight = nOldRight + rPropInfo.nRightMargin;
                }
                if( rPropInfo.bTextIndent )
                    nIndent = pLRItem->GetTxtFirstLineOfst();

                // und die Werte fuer nachfolgende Absaetze merken
                pContext->SetMargins( nLeft, nRight, nIndent );

#ifndef NUM_RELSPACE
                // Numerierungen erfordern etwas mehr Aufwand
                if( GetNumInfo().GetNumRule() )
                {
                    SwTxtNode  *pNumTxtNode = pPam->GetNode()->GetTxtNode();

                    if( pNumTxtNode->GetNum() &&
                        !(NO_NUMLEVEL & pNumTxtNode->GetNum()->GetLevel()) )
                        nIndent += HTML_NUMBUL_INDENT;

                    // den Absatz muessen wir jetzt selbst attributieren
                    pNumTxtNode->SetNumLSpace( FALSE );
                }
#endif

                // das Attribut noch am aktuellen Absatz setzen
                SvxLRSpaceItem aLRItem( *pLRItem );
                aLRItem.SetTxtFirstLineOfst( nIndent );
                aLRItem.SetTxtLeft( nLeft );
                aLRItem.SetRight( nRight );
                NewAttr( &aAttrTab.pLRSpace, aLRItem );
                EndAttr( aAttrTab.pLRSpace, 0, FALSE );
            }
            break;

        case RES_UL_SPACE:
            if( !rPropInfo.bTopMargin || !rPropInfo.bBottomMargin )
            {
                USHORT nUpper = 0, nLower = 0;
                GetULSpaceFromContext( nUpper, nLower );
                SvxULSpaceItem aULSpace( *((const SvxULSpaceItem *)pItem) );
                if( !rPropInfo.bTopMargin )
                    aULSpace.SetUpper( nUpper );
                if( !rPropInfo.bBottomMargin )
                    aULSpace.SetLower( nLower );

                NewAttr( &aAttrTab.pULSpace, aULSpace );

                // ... und noch die Kontext-Information speichern
                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.Insert( aAttrTab.pULSpace, rAttrs.Count() );

                pContext->SetULSpace( aULSpace.GetUpper(), aULSpace.GetLower() );
            }
            else
            {
                ppAttr = &aAttrTab.pULSpace;
            }
            break;
        case RES_CHRATR_FONTSIZE:
            // es werden keine Attribute mit %-Angaben gesetzt
            if( ((const SvxFontHeightItem *)pItem)->GetProp() == 100 )
                ppAttr = &aAttrTab.pFontHeight;
            break;

        case RES_BACKGROUND:
            if( bCharLvl )
            {
                // das Frame-Attr ggf. in ein Char-Attr umwandeln
                SvxBrushItem aBrushItem( *(const SvxBrushItem *)pItem );
                aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );

                // Das Attribut setzen ...
                NewAttr( &aAttrTab.pCharBrush, aBrushItem );

                // ... und noch die Kontext-Information speichern
                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.Insert( aAttrTab.pCharBrush, rAttrs.Count() );
            }
            else if( pContext->GetToken() != HTML_TABLEHEADER_ON &&
                     pContext->GetToken() != HTML_TABLEDATA_ON )
            {
                ppAttr = &aAttrTab.pBrush;
            }
            break;

        default:
            // den zu dem Item gehoehrenden Tabellen-Eintrag ermitteln ...
            ppAttr = GetAttrTabEntry( pItem->Which() );
            break;
        }

        if( ppAttr )
        {
            // Das Attribut setzen ...
            NewAttr( ppAttr, *pItem );

            // ... und noch die Kontext-Information speichern
            _HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.Insert( *ppAttr, rAttrs.Count() );
        }

        // auf zum naechsten Item
        pItem = aIter.NextItem();
    }

    if( rPropInfo.aId.Len() )
        InsertBookmark( rPropInfo.aId );
}

void SwHTMLParser::InsertAttr( _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                               _HTMLAttrContext *pCntxt )
{
    if( !ppAttr )
    {
        ppAttr = GetAttrTabEntry( rItem.Which() );
        if( !ppAttr )
            return;
    }

    // das Attribut setzen
    NewAttr( ppAttr, rItem );

    // und im Kontext merken
    _HTMLAttrs &rAttrs = pCntxt->GetAttrs();
    rAttrs.Insert( *ppAttr, rAttrs.Count() );
}

void SwHTMLParser::SplitPREListingXMP( _HTMLAttrContext *pCntxt )
{
    // PRE/Listing/XMP soll beim beenden des Kontexts beendet werden.
    pCntxt->SetFinishPREListingXMP( TRUE );

    // Und die jetzt gueltigen Flags sollen wieder gesetzt werden.
    if( IsReadPRE() )
        pCntxt->SetRestartPRE( TRUE );
    if( IsReadXMP() )
        pCntxt->SetRestartXMP( TRUE );
    if( IsReadListing() )
        pCntxt->SetRestartListing( TRUE );

    // PRE/Listing/XMP wird auuserdem sofort beendet
    FinishPREListingXMP();
}

SfxItemSet *_HTMLAttrContext::GetFrmItemSet( SwDoc *pCreateDoc )
{
    if( !pFrmItemSet && pCreateDoc )
        pFrmItemSet = new SfxItemSet( pCreateDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    return pFrmItemSet;
}
/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlctxt.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.33  2000/09/18 16:04:44  willem.vandorp
      OpenOffice header added.

      Revision 1.32  2000/04/10 12:20:55  mib
      unicode

      Revision 1.31  1999/03/26 10:37:28  MIB
      #63049#: Sofortige Numerierungs-Aktualisierung in Tabellen ist jetzt unnoetig


      Rev 1.30   26 Mar 1999 11:37:28   MIB
   #63049#: Sofortige Numerierungs-Aktualisierung in Tabellen ist jetzt unnoetig

      Rev 1.29   23 Mar 1999 15:28:14   MIB
   #63049#: Relative Einzuege in Numerierungen

      Rev 1.28   17 Mar 1999 16:46:20   MIB
   #63049#: Numerierungen mit relativen Abstaenden

      Rev 1.27   06 Nov 1998 15:33:36   MIB
   #59043#: negative Absatz-Einzuege

      Rev 1.26   21 Apr 1998 13:46:10   MIB
   fix: Keine Bookmark fuer abs-pos Objekte mit ID einfuegen

      Rev 1.25   15 Apr 1998 14:47:42   MIB
   Zwei-seitige Printing-Extensions

      Rev 1.24   03 Apr 1998 12:21:46   MIB
   Export des Rahmen-Namens als ID

      Rev 1.23   02 Apr 1998 19:43:24   MIB
   Positionierung von spaltigen Rahmen

      Rev 1.22   27 Mar 1998 10:04:10   MIB
   direkte Positionierung von Grafiken etc.

      Rev 1.21   25 Mar 1998 12:08:18   MIB
   abs.-pos. Container angefangen

      Rev 1.20   19 Mar 1998 09:03:28   MIB
   Abs.-Pos.: Attributierung nicht in Rahmen uebernehmen, Hoehe nun Mindest-Hoehe

      Rev 1.19   15 Mar 1998 16:51:14   MIB
   abs.-pos.: Abstaende bei Positionierung beachten, Hoehe nur noch Mindest-Hoehe

      Rev 1.18   13 Mar 1998 09:25:52   MIB
   Abs-Pos: margins als Rahmen-Abstaende uebernehmen

      Rev 1.17   11 Mar 1998 18:25:38   MIB
   abs. Pos.: Verankerung angepassr

      Rev 1.16   20 Feb 1998 19:04:30   MA
   header

      Rev 1.15   19 Jan 1998 16:25:56   MIB
   Numerierungs-Umbau

      Rev 1.14   26 Nov 1997 19:09:22   MA
   includes

      Rev 1.13   17 Nov 1997 10:16:48   JP
   Umstellung Numerierung

      Rev 1.12   20 Oct 1997 09:51:56   MIB
   Nur fuer absolute positioning benoetigten Code groesstenteils auskommentiert

      Rev 1.11   17 Oct 1997 13:20:32   MIB
   page-break-xxx auch uber STYLE/CLASS und ID-Optionen

      Rev 1.10   09 Oct 1997 14:36:10   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.9   16 Sep 1997 17:25:10   MIB
   abs. Positioning fuer Absatz-Tags

      Rev 1.8   16 Sep 1997 11:15:00   MIB
   Kopf-/Fusszeilen ohne Moven von Nodes, autom. Beenden von Bereichen/Rahmen

      Rev 1.7   12 Sep 1997 11:56:16   MIB
   fix #41253#: Script in PRE (keine Schleifen), MULTICOL ohne Move von Nodes

      Rev 1.6   08 Sep 1997 17:49:12   MIB
   Verankerung von AUTO_CNTNT-Rahmen nicht mehr verschieben

      Rev 1.5   08 Sep 1997 10:33:44   MIB
   Keine Schleifen fuer PRE mehr (auch fix #41253#) (nicht freigeschaltet)

      Rev 1.4   15 Aug 1997 12:47:48   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.3   12 Aug 1997 13:44:18   OS
   Header-Umstellung

      Rev 1.2   07 Aug 1997 15:07:50   OM
   Headerfile-Umstellung

      Rev 1.1   04 Aug 1997 13:54:08   MIB
   aboslute psoitioning (fuer fast alle Zeichen-Attribute/-Vorlagen)

      Rev 1.0   04 Aug 1997 13:02:32   MIB
   Initial revision.

*************************************************************************/

