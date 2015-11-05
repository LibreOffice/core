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
#include <svl/itemiter.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svtools/htmltokn.h>
#include <editeng/boxitem.hxx>

#include "doc.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "paratr.hxx"
#include "htmlnum.hxx"
#include "css1kywd.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

using namespace ::com::sun::star;

class _HTMLAttrContext_SaveDoc
{
    SwHTMLNumRuleInfo aNumRuleInfo; // In Umgebung gueltige Numerierung
    SwPosition  *pPos;              // hierhin beim verlassen den
                                    // Kontexts zurueckgesprungen
    _HTMLAttrTable *pAttrTab;       // In Umgebung gueltige Attribute,
                                    // wenn Attributierung nicht
                                    // beibehalten werden soll.

    size_t nContextStMin;           // In Umgebung gueltige Stack-
                                    // Untergrenze, wenn der Stack
                                    // geschuetzt werden soll.
    size_t nContextStAttrMin;       // In Umgebung gueltige Stack-
                                    // Untergrenze, wenn die Attribute
                                    // nicht beibehalten werden sollen.

    bool bStripTrailingPara : 1;    // letzen Absatz entfernen?
    bool bKeepNumRules : 1;         // Numerierung beibehalten?
    bool bFixHeaderDist : 1;
    bool bFixFooterDist : 1;

public:

    _HTMLAttrContext_SaveDoc() :
        pPos( 0 ), pAttrTab( 0 ),
        nContextStMin( SIZE_MAX ), nContextStAttrMin( SIZE_MAX ),
        bStripTrailingPara( false ), bKeepNumRules( false ),
        bFixHeaderDist( false ), bFixFooterDist( false )
    {}

    ~_HTMLAttrContext_SaveDoc() { delete pPos; delete pAttrTab; }

    // Die Position gehoert uns, muss also angelegt und zerstoert werden
    void SetPos( const SwPosition& rPos ) { pPos = new SwPosition(rPos); }
    const SwPosition *GetPos() const { return pPos; }

    // Der Index gehoert uns nicht. Kein Anlgen und Zerstoeren.
    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; }

    _HTMLAttrTable *GetAttrTab( bool bCreate= false );

    void SetContextStMin( size_t nMin ) { nContextStMin = nMin; }
    size_t GetContextStMin() const { return nContextStMin; }

    void SetContextStAttrMin( size_t nMin ) { nContextStAttrMin = nMin; }
    size_t GetContextStAttrMin() const { return nContextStAttrMin; }

    void SetStripTrailingPara( bool bSet ) { bStripTrailingPara = bSet; }
    bool GetStripTrailingPara() const { return bStripTrailingPara; }

    void SetKeepNumRules( bool bSet ) { bKeepNumRules = bSet; }
    bool GetKeepNumRules() const { return bKeepNumRules; }

    void SetFixHeaderDist( bool bSet ) { bFixHeaderDist = bSet; }
    bool GetFixHeaderDist() const { return bFixHeaderDist; }

    void SetFixFooterDist( bool bSet ) { bFixFooterDist = bSet; }
    bool GetFixFooterDist() const { return bFixFooterDist; }
};

_HTMLAttrTable *_HTMLAttrContext_SaveDoc::GetAttrTab( bool bCreate )
{
    if( !pAttrTab && bCreate )
    {
        pAttrTab = new _HTMLAttrTable;
        memset( pAttrTab, 0, sizeof( _HTMLAttrTable ));
    }
    return pAttrTab;
}

_HTMLAttrContext_SaveDoc *_HTMLAttrContext::GetSaveDocContext( bool bCreate )
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

void SwHTMLParser::SplitAttrTab( const SwPosition& rNewPos )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(m_aParaAttrs.empty(),
        "Danger: there are non-final paragraph attributes");
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

    const SwNodeIndex* pOldEndPara = &m_pPam->GetPoint()->nNode;
    sal_Int32 nOldEndCnt = m_pPam->GetPoint()->nContent.GetIndex();

    const SwNodeIndex& rNewSttPara = rNewPos.nNode;
    sal_Int32 nNewSttCnt = rNewPos.nContent.GetIndex();

    bool bMoveBack = false;

    // alle noch offenen Attribute beenden und hinter der Tabelle
    // neu aufspannen
    _HTMLAttr** pHTMLAttributes = reinterpret_cast<_HTMLAttr**>(&m_aAttrTab);
    for (auto nCnt = sizeof(_HTMLAttrTable) / sizeof(_HTMLAttr*); nCnt--; ++pHTMLAttributes)
    {
        _HTMLAttr *pAttr = *pHTMLAttributes;
        while( pAttr )
        {
            _HTMLAttr *pNext = pAttr->GetNext();
            _HTMLAttr *pPrev = pAttr->GetPrev();

            sal_uInt16 nWhich = pAttr->pItem->Which();
            if( !nOldEndCnt && RES_PARATR_BEGIN <= nWhich &&
                pAttr->GetSttParaIdx() < pOldEndPara->GetIndex() )
            {
                // Das Attribut muss eine Content-Position weiter vorne
                // beendet werden
                if( !bMoveBack )
                {
                    bMoveBack = m_pPam->Move( fnMoveBackward );
                    nOldEndCnt = m_pPam->GetPoint()->nContent.GetIndex();
                }
            }
            else if( bMoveBack )
            {
                m_pPam->Move( fnMoveForward );
                nOldEndCnt = m_pPam->GetPoint()->nContent.GetIndex();
            }

            if( (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
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
                    if (pSetAttr->bInsAtStart)
                        m_aSetAttrTab.push_front( pSetAttr );
                    else
                        m_aSetAttrTab.push_back( pSetAttr );
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
                    if (pPrev->bInsAtStart)
                        m_aSetAttrTab.push_front( pPrev );
                    else
                        m_aSetAttrTab.push_back( pPrev );
                }
            }

            // den Start des Attributs neu setzen
            pAttr->nSttPara = rNewSttPara;
            pAttr->nEndPara = rNewSttPara;
            pAttr->nSttContent = nNewSttCnt;
            pAttr->nEndContent = nNewSttCnt;
            pAttr->pPrev = 0;

            pAttr = pNext;
        }
    }

    if( bMoveBack )
        m_pPam->Move( fnMoveForward );

}

void SwHTMLParser::SaveDocContext( _HTMLAttrContext *pCntxt,
                                   sal_uInt16 nFlags,
                                   const SwPosition *pNewPos )
{
    _HTMLAttrContext_SaveDoc *pSave = pCntxt->GetSaveDocContext( true );
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

        if( (HTML_CNTXT_KEEP_ATTRS & nFlags) != 0 )
        {
            // Attribute an aktueller Position beenden und an neuer neu anfangen
            SplitAttrTab( *pNewPos );
        }
        else
        {
            _HTMLAttrTable *pSaveAttrTab = pSave->GetAttrTab( true );
            SaveAttrTab( *pSaveAttrTab );
        }

        pSave->SetPos( *m_pPam->GetPoint() );
        *m_pPam->GetPoint() = *pNewPos;
    }

    // Mit dem Setzen von nContextStMin koennen automatisch auch
    // keine gerade offenen Listen (DL/OL/UL) mehr beendet werden.
    if( (HTML_CNTXT_PROTECT_STACK & nFlags) != 0  )
    {
        pSave->SetContextStMin( m_nContextStMin );
        m_nContextStMin = m_aContexts.size();

        if( (HTML_CNTXT_KEEP_ATTRS & nFlags) == 0 )
        {
            pSave->SetContextStAttrMin( m_nContextStAttrMin );
            m_nContextStAttrMin = m_aContexts.size();
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

        *m_pPam->GetPoint() = *pSave->GetPos();

        // Die bisherigen Attribute koennen wir schonmal setzen.
        SetAttr();
    }

    if( SIZE_MAX != pSave->GetContextStMin() )
    {
        m_nContextStMin = pSave->GetContextStMin();
        if( SIZE_MAX != pSave->GetContextStAttrMin() )
            m_nContextStAttrMin = pSave->GetContextStAttrMin();
    }

    if( !pSave->GetKeepNumRules() )
    {
        // Die bisherige gemerkte Numerierung wieder setzen
        GetNumInfo().Set( pSave->GetNumInfo() );
    }

    pCntxt->ClearSaveDocContext();
}

void SwHTMLParser::EndContext( _HTMLAttrContext *pContext )
{
    if( pContext->GetPopStack() )
    {
        // Alle noch offenen Kontexte beenden. Der eigene
        // Kontext muss bereits geloscht sein!
        while( m_aContexts.size() > m_nContextStMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            OSL_ENSURE( pCntxt != pContext,
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
        m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( pContext->GetAppendMode() );

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
    for( auto pAttr : rAttrs )
    {
        // einfaches Loeschen reicht hier nicht, weil das
        // Attribut auch aus seiner Liste ausgetragen werden
        // muss. Theoretisch koennt man natuerlich auch die Liste
        // und die Attribute getrennt loeschen, aber wenn man
        // dann was falsch gemacht hat, sieht es uebel aus.
        DeleteAttr( pAttr );
    }

    OSL_ENSURE( !pContext->GetSpansSection(),
            "Bereich kann nicht mehr verlassen werden" );

    OSL_ENSURE( !pContext->HasSaveDocContext(),
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

bool SwHTMLParser::DoPositioning( SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo &rPropInfo,
                                  _HTMLAttrContext *pContext )
{
    bool bRet = false;

    // Unter folgenden Umstaenden wird jetzt ein Rahmen aufgemacht:
    // - das Tag wird absolut positioniert und left/top sind beide
    //   gegeben und enthalten auch keine %-Angabe, oder
    // - das Tag soll fliessen, und
    // - es wurde eine Breite angegeben (in beiden Faellen noetig)
    if( SwCSS1Parser::MayBePositioned( rPropInfo ) )
    {
        SfxItemSet aFrmItemSet( m_pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs(aFrmItemSet );

        // Ausrichtung setzen
        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE, rItemSet, rPropInfo,
                                aFrmItemSet );

        // Groesse setzen
        SetVarSize( rItemSet, rPropInfo, aFrmItemSet );

        // Abstaende setzen
        SetSpace( Size(0,0), rItemSet, rPropInfo, aFrmItemSet );

        // Sonstige CSS1-Attribute Setzen
        SetFrameFormatAttrs( rItemSet, rPropInfo,
                        HTML_FF_BOX|HTML_FF_PADDING|HTML_FF_BACKGROUND|HTML_FF_DIRECTION,
                        aFrmItemSet );

        InsertFlyFrame( aFrmItemSet, pContext, rPropInfo.aId,
                        CONTEXT_FLAGS_ABSPOS );
        pContext->SetPopStack( true );
        rPropInfo.aId.clear();
        bRet = true;
    }

    return bRet;
}

bool SwHTMLParser::CreateContainer( const OUString& rClass,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo &rPropInfo,
                                    _HTMLAttrContext *pContext )
{
    bool bRet = false;
    if( rClass.equalsIgnoreAsciiCase( "sd-abs-pos" ) &&
        SwCSS1Parser::MayBePositioned( rPropInfo ) )
    {
        // Container-Klasse
        SfxItemSet *pFrmItemSet = pContext->GetFrmItemSet( m_pDoc );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs( *pFrmItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE,
                                rItemSet, rPropInfo, *pFrmItemSet );
        Size aDummy(0,0);
        SetFixSize( aDummy, aDummy, false, false, rItemSet, rPropInfo,
                    *pFrmItemSet );
        SetSpace( aDummy, rItemSet, rPropInfo, *pFrmItemSet );
        SetFrameFormatAttrs( rItemSet, rPropInfo, HTML_FF_BOX|HTML_FF_BACKGROUND|HTML_FF_DIRECTION,
                        *pFrmItemSet );

        bRet = true;
    }

    return bRet;
}

void SwHTMLParser::InsertAttrs( SfxItemSet &rItemSet,
                                SvxCSS1PropertyInfo &rPropInfo,
                                _HTMLAttrContext *pContext,
                                bool bCharLvl )
{
    // Ein DropCap-Attribut basteln, wenn auf Zeichen-Ebene vor dem
    // ersten Zeichen ein float: left vorkommt
    if( bCharLvl && !m_pPam->GetPoint()->nContent.GetIndex() &&
        SVX_ADJUST_LEFT == rPropInfo.eFloat )
    {
        SwFormatDrop aDrop;
        aDrop.GetChars() = 1;

        m_pCSS1Parser->FillDropCap( aDrop, rItemSet );

        // Nur wenn das Initial auch ueber mehrere Zeilen geht, wird das
        // DropCap-Attribut gesetzt. Sonst setzten wir die Attribute hart.
        if( aDrop.GetLines() > 1 )
        {
            NewAttr( &m_aAttrTab.pDropCap, aDrop );

            _HTMLAttrs &rAttrs = pContext->GetAttrs();
            rAttrs.push_back( m_aAttrTab.pDropCap );

            return;
        }
    }

    if( !bCharLvl )
        m_pCSS1Parser->SetFormatBreak( rItemSet, rPropInfo );

    OSL_ENSURE(m_aContexts.size() <= m_nContextStAttrMin ||
            m_aContexts.back() != pContext,
            "SwHTMLParser::InsertAttrs: Context already on the Stack");

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
                // fuer alle folgenden in SetTextCollAttrs)

                const SvxLRSpaceItem *pLRItem =
                    static_cast<const SvxLRSpaceItem *>(pItem);

                // die bisherigen Absatz-Abstaende holen (ohne die vom
                // obersten Kontext, denn den veraendern wir ja gerade) ...
                sal_uInt16 nOldLeft = 0, nOldRight = 0;
                short nOldIndent = 0;
                bool bIgnoreTop = m_aContexts.size() > m_nContextStMin &&
                                  m_aContexts.back() == pContext;
                GetMarginsFromContext( nOldLeft, nOldRight, nOldIndent,
                                       bIgnoreTop  );

                // und noch die aktuell gueltigen
                sal_uInt16 nLeft = nOldLeft, nRight = nOldRight;
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
                    OSL_ENSURE( rPropInfo.nLeftMargin < 0 ||
                            rPropInfo.nLeftMargin == pLRItem->GetTextLeft(),
                            "linker Abstand stimmt nicht mit Item ueberein" );
                    if( rPropInfo.nLeftMargin < 0 &&
                        -rPropInfo.nLeftMargin > nOldLeft )
                        nLeft = 0;
                    else
                        nLeft = nOldLeft + static_cast< sal_uInt16 >(rPropInfo.nLeftMargin);
                }
                if( rPropInfo.bRightMargin )
                {
                    OSL_ENSURE( rPropInfo.nRightMargin < 0 ||
                            rPropInfo.nRightMargin == pLRItem->GetRight(),
                            "rechter Abstand stimmt nicht mit Item ueberein" );
                    if( rPropInfo.nRightMargin < 0 &&
                        -rPropInfo.nRightMargin > nOldRight )
                        nRight = 0;
                    else
                        nRight = nOldRight + static_cast< sal_uInt16 >(rPropInfo.nRightMargin);
                }
                if( rPropInfo.bTextIndent )
                    nIndent = pLRItem->GetTextFirstLineOfst();

                // und die Werte fuer nachfolgende Absaetze merken
                pContext->SetMargins( nLeft, nRight, nIndent );

                // das Attribut noch am aktuellen Absatz setzen
                SvxLRSpaceItem aLRItem( *pLRItem );
                aLRItem.SetTextFirstLineOfst( nIndent );
                aLRItem.SetTextLeft( nLeft );
                aLRItem.SetRight( nRight );
                NewAttr( &m_aAttrTab.pLRSpace, aLRItem );
                EndAttr( m_aAttrTab.pLRSpace, 0, false );
            }
            break;

        case RES_UL_SPACE:
            if( !rPropInfo.bTopMargin || !rPropInfo.bBottomMargin )
            {
                sal_uInt16 nUpper = 0, nLower = 0;
                GetULSpaceFromContext( nUpper, nLower );
                SvxULSpaceItem aULSpace( *static_cast<const SvxULSpaceItem *>(pItem) );
                if( !rPropInfo.bTopMargin )
                    aULSpace.SetUpper( nUpper );
                if( !rPropInfo.bBottomMargin )
                    aULSpace.SetLower( nLower );

                NewAttr( &m_aAttrTab.pULSpace, aULSpace );

                // ... und noch die Kontext-Information speichern
                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( m_aAttrTab.pULSpace );

                pContext->SetULSpace( aULSpace.GetUpper(), aULSpace.GetLower() );
            }
            else
            {
                ppAttr = &m_aAttrTab.pULSpace;
            }
            break;
        case RES_CHRATR_FONTSIZE:
            // es werden keine Attribute mit %-Angaben gesetzt
            if( static_cast<const SvxFontHeightItem *>(pItem)->GetProp() == 100 )
                ppAttr = &m_aAttrTab.pFontHeight;
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            // es werden keine Attribute mit %-Angaben gesetzt
            if( static_cast<const SvxFontHeightItem *>(pItem)->GetProp() == 100 )
                ppAttr = &m_aAttrTab.pFontHeightCJK;
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            // es werden keine Attribute mit %-Angaben gesetzt
            if( static_cast<const SvxFontHeightItem *>(pItem)->GetProp() == 100 )
                ppAttr = &m_aAttrTab.pFontHeightCTL;
            break;

        case RES_BACKGROUND:
            if( bCharLvl )
            {
                // das Frame-Attr ggf. in ein Char-Attr umwandeln
                SvxBrushItem aBrushItem( *static_cast<const SvxBrushItem *>(pItem) );
                aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );

                // Das Attribut setzen ...
                NewAttr( &m_aAttrTab.pCharBrush, aBrushItem );

                // ... und noch die Kontext-Information speichern
                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( m_aAttrTab.pCharBrush );
            }
            else if( pContext->GetToken() != HTML_TABLEHEADER_ON &&
                     pContext->GetToken() != HTML_TABLEDATA_ON )
            {
                ppAttr = &m_aAttrTab.pBrush;
            }
            break;

        case RES_BOX:
            if( bCharLvl )
            {
                SvxBoxItem aBoxItem( *static_cast<const SvxBoxItem *>(pItem) );
                aBoxItem.SetWhich( RES_CHRATR_BOX );

                NewAttr( &m_aAttrTab.pCharBox, aBoxItem );

                _HTMLAttrs &rAttrs = pContext->GetAttrs();
                rAttrs.push_back( m_aAttrTab.pCharBox );
            }
            else
            {
                ppAttr = &m_aAttrTab.pBox;
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
            rAttrs.push_back( *ppAttr );
        }

        // auf zum naechsten Item
        pItem = aIter.NextItem();
    }

    if( !rPropInfo.aId.isEmpty() )
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
    rAttrs.push_back( *ppAttr );
}

void SwHTMLParser::SplitPREListingXMP( _HTMLAttrContext *pCntxt )
{
    // PRE/Listing/XMP soll beim beenden des Kontexts beendet werden.
    pCntxt->SetFinishPREListingXMP( true );

    // Und die jetzt gueltigen Flags sollen wieder gesetzt werden.
    if( IsReadPRE() )
        pCntxt->SetRestartPRE( true );
    if( IsReadXMP() )
        pCntxt->SetRestartXMP( true );
    if( IsReadListing() )
        pCntxt->SetRestartListing( true );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
