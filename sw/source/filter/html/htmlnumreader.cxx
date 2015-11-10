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

#include <com/sun/star/style/NumberingType.hpp>
#include <hintids.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmlout.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <numrule.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>

#include "htmlnum.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

#include <SwNodeNum.hxx>

using namespace css;

// <UL TYPE=...>
static HTMLOptionEnum aHTMLULTypeTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_ULTYPE_disc,      HTML_BULLETCHAR_DISC   },
    { OOO_STRING_SVTOOLS_HTML_ULTYPE_circle,    HTML_BULLETCHAR_CIRCLE },
    { OOO_STRING_SVTOOLS_HTML_ULTYPE_square,    HTML_BULLETCHAR_SQUARE },
    { nullptr,                                        0                      }
};


void SwHTMLParser::NewNumBulList( int nToken )
{
    SwHTMLNumRuleInfo& rInfo = GetNumInfo();

    // Erstmal einen neuen Absatz aufmachen
    bool bSpace = (rInfo.GetDepth() + m_nDefListDeep) == 0;
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( bSpace ? AM_SPACE : AM_NOSPACE, false );
    else if( bSpace )
        AddParSpace();

    // Die Numerierung-Ebene erhoehen
    rInfo.IncDepth();
    sal_uInt8 nLevel = (sal_uInt8)( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                        : MAXLEVEL) - 1 );

    // ggf. ein Regelwerk anlegen
    if( !rInfo.GetNumRule() )
    {
        sal_uInt16 nPos = m_pDoc->MakeNumRule( m_pDoc->GetUniqueNumRuleName() );
        rInfo.SetNumRule( m_pDoc->GetNumRuleTable()[nPos] );
    }

    // das Format anpassen, falls es fuer den Level noch nicht
    // geschehen ist!
    bool bNewNumFormat = rInfo.GetNumRule()->GetNumFormat( nLevel ) == nullptr;
    bool bChangeNumFormat = false;

    // das default Numerierungsformat erstellen
    SwNumFormat aNumFormat( rInfo.GetNumRule()->Get(nLevel) );
    rInfo.SetNodeStartValue( nLevel );
    if( bNewNumFormat )
    {
        sal_uInt16 nChrFormatPoolId = 0;
        if( HTML_ORDERLIST_ON == nToken )
        {
            aNumFormat.SetNumberingType(SVX_NUM_ARABIC);
            nChrFormatPoolId = RES_POOLCHR_NUM_LEVEL;
        }
        else
        {
            // Wir setzen hier eine Zeichenvorlage, weil die UI das auch
            // so macht. Dadurch wurd immer auch eine 9pt-Schrift
            // eingestellt, was in Netscape nicht der Fall ist. Bisher hat
            // das noch niemanden gestoert.
            // #i63395# - Only apply user defined default bullet font
            if ( numfunc::IsDefBulletFontUserDefined() )
            {
                aNumFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
            }
            aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aNumFormat.SetBulletChar( cBulletChar );       // das Bulletzeichen !!
            nChrFormatPoolId = RES_POOLCHR_BUL_LEVEL;
        }

        sal_uInt16 nAbsLSpace = HTML_NUMBUL_MARGINLEFT;

        short nFirstLineIndent  = HTML_NUMBUL_INDENT;
        if( nLevel > 0 )
        {
            const SwNumFormat& rPrevNumFormat = rInfo.GetNumRule()->Get( nLevel-1 );
            nAbsLSpace = nAbsLSpace + rPrevNumFormat.GetAbsLSpace();
            nFirstLineIndent = rPrevNumFormat.GetFirstLineOffset();
        }
        aNumFormat.SetAbsLSpace( nAbsLSpace );
        aNumFormat.SetFirstLineOffset( nFirstLineIndent );
        aNumFormat.SetCharFormat( m_pCSS1Parser->GetCharFormatFromPool(nChrFormatPoolId) );

        bChangeNumFormat = true;
    }
    else if( 1 != aNumFormat.GetStart() )
    {
        // Wenn die Ebene schon mal benutzt wurde, muss der Start-Wert
        // ggf. hart am Absatz gesetzt werden.
        rInfo.SetNodeStartValue( nLevel, 1 );
    }

    // und es ggf. durch die Optionen veraendern
    OUString aId, aStyle, aClass, aLang, aDir;
    OUString aBulletSrc;
    sal_Int16 eVertOri = text::VertOrientation::NONE;
    sal_uInt16 nWidth=USHRT_MAX, nHeight=USHRT_MAX;
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_TYPE:
            if( bNewNumFormat && !rOption.GetString().isEmpty() )
            {
                switch( nToken )
                {
                case HTML_ORDERLIST_ON:
                    bChangeNumFormat = true;
                    switch( rOption.GetString()[0] )
                    {
                    case 'A':   aNumFormat.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER); break;
                    case 'a':   aNumFormat.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER); break;
                    case 'I':   aNumFormat.SetNumberingType(SVX_NUM_ROMAN_UPPER);        break;
                    case 'i':   aNumFormat.SetNumberingType(SVX_NUM_ROMAN_LOWER);        break;
                    default:    bChangeNumFormat = false;
                    }
                    break;

                case HTML_UNORDERLIST_ON:
                    aNumFormat.SetBulletChar( (sal_Unicode)rOption.GetEnum(
                                    aHTMLULTypeTable,aNumFormat.GetBulletChar() ) );
                    bChangeNumFormat = true;
                    break;
                }
            }
            break;
        case HTML_O_START:
            {
                sal_uInt16 nStart = (sal_uInt16)rOption.GetNumber();
                if( bNewNumFormat )
                {
                    aNumFormat.SetStart( nStart );
                    bChangeNumFormat = true;
                }
                else
                {
                    rInfo.SetNodeStartValue( nLevel, nStart );
                }
            }
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_LANG:
            aLang = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        case HTML_O_SRC:
            if( bNewNumFormat )
            {
                aBulletSrc = rOption.GetString();
                if( !InternalImgToPrivateURL(aBulletSrc) )
                    aBulletSrc = URIHelper::SmartRel2Abs( INetURLObject( m_sBaseURL ), aBulletSrc, Link<OUString *, bool>(), false );
            }
            break;
        case HTML_O_WIDTH:
            nWidth = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            nHeight = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_ALIGN:
            eVertOri =
                (sal_Int16)rOption.GetEnum( aHTMLImgVAlignTable,
                                                static_cast< sal_uInt16 >(eVertOri) );
            break;
        }
    }

    if( !aBulletSrc.isEmpty() )
    {
        // Eine Bullet-Liste mit Grafiken
        aNumFormat.SetNumberingType(SVX_NUM_BITMAP);

        // Die Grafik als Brush anlegen
        SvxBrushItem aBrushItem( RES_BACKGROUND );
        aBrushItem.SetGraphicLink( aBulletSrc );
        aBrushItem.SetGraphicPos( GPOS_AREA );

        // Die Groesse nur beachten, wenn Breite und Hoehe vorhanden sind
        Size aTwipSz( nWidth, nHeight), *pTwipSz=nullptr;
        if( nWidth!=USHRT_MAX && nHeight!=USHRT_MAX )
        {
            aTwipSz =
                Application::GetDefaultDevice()->PixelToLogic( aTwipSz,
                                                    MapMode(MAP_TWIP) );
            pTwipSz = &aTwipSz;
        }

        // Die Ausrichtung auch nur beachten, wenn eine Ausrichtung
        // angegeben wurde
        aNumFormat.SetGraphicBrush( &aBrushItem, pTwipSz,
                            text::VertOrientation::NONE!=eVertOri ? &eVertOri : nullptr);

        // Und noch die Grafik merken, um sie in den Absaetzen nicht
        // einzufuegen
        m_aBulletGrfs[nLevel] = aBulletSrc;
        bChangeNumFormat = true;
    }
    else
        m_aBulletGrfs[nLevel].clear();

    // den aktuellen Absatz erst einmal nicht numerieren
    {
        sal_uInt8 nLvl = nLevel;
        SetNodeNum( nLvl, false );
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            if( bNewNumFormat )
            {
                if( aPropInfo.bLeftMargin )
                {
                    // Der Der Default-Einzug wurde schon eingefuegt.
                    sal_uInt16 nAbsLSpace =
                        aNumFormat.GetAbsLSpace() - HTML_NUMBUL_MARGINLEFT;
                    if( aPropInfo.nLeftMargin < 0 &&
                        nAbsLSpace < -aPropInfo.nLeftMargin )
                        nAbsLSpace = 0U;
                    else if( aPropInfo.nLeftMargin > USHRT_MAX ||
                             (long)nAbsLSpace +
                                            aPropInfo.nLeftMargin > USHRT_MAX )
                        nAbsLSpace = USHRT_MAX;
                    else
                        nAbsLSpace = nAbsLSpace + (sal_uInt16)aPropInfo.nLeftMargin;

                    aNumFormat.SetAbsLSpace( nAbsLSpace );
                    bChangeNumFormat = true;
                }
                if( aPropInfo.bTextIndent )
                {
                    short nTextIndent =
                        static_cast<const SvxLRSpaceItem &>(aItemSet.Get( RES_LR_SPACE ))
                                                        .GetTextFirstLineOfst();
                    aNumFormat.SetFirstLineOffset( nTextIndent );
                    bChangeNumFormat = true;
                }
            }
            aPropInfo.bLeftMargin = aPropInfo.bTextIndent = false;
            if( !aPropInfo.bRightMargin )
                aItemSet.ClearItem( RES_LR_SPACE );

            // #i89812# - Perform change to list style before calling <DoPositioning(..)>,
            // because <DoPositioning(..)> may open a new context and thus may
            // clear the <SwHTMLNumRuleInfo> instance hold by local variable <rInfo>.
            if( bChangeNumFormat )
            {
                rInfo.GetNumRule()->Set( nLevel, aNumFormat );
                m_pDoc->ChgNumRuleFormats( *rInfo.GetNumRule() );
                bChangeNumFormat = false;
            }

            DoPositioning( aItemSet, aPropInfo, pCntxt );

            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( bChangeNumFormat )
    {
        rInfo.GetNumRule()->Set( nLevel, aNumFormat );
        m_pDoc->ChgNumRuleFormats( *rInfo.GetNumRule() );
    }

    PushContext( pCntxt );

    // die Attribute der neuen Vorlage setzen
    SetTextCollAttrs( pCntxt );
}

void SwHTMLParser::EndNumBulList( int nToken )
{
    SwHTMLNumRuleInfo& rInfo = GetNumInfo();

    // Ein neuer Absatz muss aufgemacht werden, wenn
    // - der aktuelle nicht leer ist, also Text oder absatzgebundene Objekte
    //   enthaelt.
    // - der aktuelle Absatz numeriert ist.
    bool bAppend = m_pPam->GetPoint()->nContent.GetIndex() > 0;
    if( !bAppend )
    {
        SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();

        bAppend = (pTextNode && ! pTextNode->IsOutline() && pTextNode->IsCountedInList()) ||

            HasCurrentParaFlys();
    }

    bool bSpace = (rInfo.GetDepth() + m_nDefListDeep) == 1;
    if( bAppend )
        AppendTextNode( bSpace ? AM_SPACE : AM_NOSPACE, false );
    else if( bSpace )
        AddParSpace();

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pCntxt = nToken!=0 ? PopContext( static_cast< sal_uInt16 >(nToken & ~1) ) : nullptr;

    // Keine Liste aufgrund eines Tokens beenden, wenn der Kontext
    // nie angelgt wurde oder nicht beendet werden darf.
    if( rInfo.GetDepth()>0 && (!nToken || pCntxt) )
    {
        rInfo.DecDepth();
        if( !rInfo.GetDepth() )     // wars der letze Level ?
        {
            // Die noch nicht angepassten Formate werden jetzt noch
            // angepasst, damit es sich besser Editieren laesst.
            const SwNumFormat *pRefNumFormat = nullptr;
            bool bChanged = false;
            for( sal_uInt16 i=0; i<MAXLEVEL; i++ )
            {
                const SwNumFormat *pNumFormat = rInfo.GetNumRule()->GetNumFormat(i);
                if( pNumFormat )
                {
                    pRefNumFormat = pNumFormat;
                }
                else if( pRefNumFormat )
                {
                    SwNumFormat aNumFormat( rInfo.GetNumRule()->Get(i) );
                    aNumFormat.SetNumberingType(pRefNumFormat->GetNumberingType() != SVX_NUM_BITMAP
                                        ? pRefNumFormat->GetNumberingType() : style::NumberingType::CHAR_SPECIAL);
                    if( SVX_NUM_CHAR_SPECIAL == aNumFormat.GetNumberingType() )
                    {
                        // #i63395# - Only apply user defined default bullet font
                        if ( numfunc::IsDefBulletFontUserDefined() )
                        {
                            aNumFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
                        }
                        aNumFormat.SetBulletChar( cBulletChar );
                    }
                    aNumFormat.SetAbsLSpace( (i+1) * HTML_NUMBUL_MARGINLEFT );
                    aNumFormat.SetFirstLineOffset( HTML_NUMBUL_INDENT );
                    aNumFormat.SetCharFormat( pRefNumFormat->GetCharFormat() );
                    rInfo.GetNumRule()->Set( i, aNumFormat );
                    bChanged = true;
                }
            }
            if( bChanged )
                m_pDoc->ChgNumRuleFormats( *rInfo.GetNumRule() );

            // Beim letzen Append wurde das NumRule-Item und das
            // NodeNum-Objekt mit kopiert. Beides muessen wir noch
            // loeschen. Das ResetAttr loescht das NodeNum-Objekt mit!
            m_pPam->GetNode().GetTextNode()->ResetAttr( RES_PARATR_NUMRULE );

            rInfo.Clear();
        }
        else
        {
            // the next paragraph not numbered first
            SetNodeNum( rInfo.GetLevel(), false );
        }
    }

    // und noch Attribute beenden
    bool bSetAttrs = false;
    if( pCntxt )
    {
        EndContext( pCntxt );
        delete pCntxt;
        bSetAttrs = true;
    }

    if( nToken )
        SetTextCollAttrs();

    if( bSetAttrs )
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen

}

void SwHTMLParser::NewNumBulListItem( int nToken )
{
    sal_uInt8 nLevel = GetNumInfo().GetLevel();
    OUString aId, aStyle, aClass, aLang, aDir;
    sal_uInt16 nStart = HTML_LISTHEADER_ON != nToken
                        ? GetNumInfo().GetNodeStartValue( nLevel )
                        : USHRT_MAX;
    if( USHRT_MAX != nStart )
        GetNumInfo().SetNodeStartValue( nLevel );

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_VALUE:
                nStart = (sal_uInt16)rOption.GetNumber();
                break;
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_NOSPACE, false );
    m_bNoParSpace = false;    // In <LI> wird kein Abstand eingefuegt!

    const bool bCountedInList = nToken != HTML_LISTHEADER_ON;

    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    OUString aNumRuleName;
    if( GetNumInfo().GetNumRule() )
    {
        aNumRuleName = GetNumInfo().GetNumRule()->GetName();
    }
    else
    {
        aNumRuleName = m_pDoc->GetUniqueNumRuleName();
        SwNumRule aNumRule( aNumRuleName,
                            SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
        SwNumFormat aNumFormat( aNumRule.Get( 0 ) );
        // #i63395# - Only apply user defined default bullet font
        if ( numfunc::IsDefBulletFontUserDefined() )
        {
            aNumFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
        }
        aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
        aNumFormat.SetBulletChar( cBulletChar );   // das Bulletzeichen !!
        aNumFormat.SetCharFormat( m_pCSS1Parser->GetCharFormatFromPool(RES_POOLCHR_BUL_LEVEL) );
        aNumFormat.SetFirstLineOffset( HTML_NUMBUL_INDENT );
        aNumRule.Set( 0, aNumFormat );

        m_pDoc->MakeNumRule( aNumRuleName, &aNumRule );

        OSL_ENSURE( !m_nOpenParaToken,
                "Jetzt geht ein offenes Absatz-Element verloren" );
        // Wir tun so, als ob wir in einem Absatz sind. Dann wird
        // beim naechsten Absatz wenigstens die Numerierung
        // weggeschmissen, die nach dem naechsten AppendTextNode uebernommen
        // wird.
        m_nOpenParaToken = static_cast< sal_uInt16 >(nToken);
    }

    SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();
    static_cast<SwContentNode *>(pTextNode)->SetAttr( SwNumRuleItem(aNumRuleName) );
    pTextNode->SetAttrListLevel(nLevel);
    // #i57656# - <IsCounted()> state of text node has to be adjusted accordingly.
    if ( nLevel < MAXLEVEL )
    {
        pTextNode->SetCountedInList( bCountedInList );
    }
    // #i57919#
    // correction of refactoring done by cws swnumtree
    // - <nStart> contains the start value, if the numbering has to be restarted
    //   at this text node. Value <USHRT_MAX> indicates, that numbering isn't
    //   restarted at this text node
    if ( nStart != USHRT_MAX )
    {
        pTextNode->SetListRestart( true );
        pTextNode->SetAttrListRestartValue( nStart );
    }

    if( GetNumInfo().GetNumRule() )
        GetNumInfo().GetNumRule()->SetInvalidRule( true );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    // die neue Vorlage setzen
    SetTextCollAttrs( pCntxt );

    // Laufbalkenanzeige aktualisieren
    ShowStatline();
}

void SwHTMLParser::EndNumBulListItem( int nToken, bool bSetColl,
                                      bool /*bLastPara*/ )
{
    // einen neuen Absatz aufmachen
    if( !nToken && m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_NOSPACE );

    // Kontext zu dem Token suchen und vom Stack holen
    _HTMLAttrContext *pCntxt = nullptr;
    auto nPos = m_aContexts.size();
    nToken &= ~1;
    while( !pCntxt && nPos>m_nContextStMin )
    {
        sal_uInt16 nCntxtToken = m_aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_LI_ON:
        case HTML_LISTHEADER_ON:
            if( !nToken || nToken == nCntxtToken  )
            {
                pCntxt = m_aContexts[nPos];
                m_aContexts.erase( m_aContexts.begin() + nPos );
            }
            break;
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_DIRLIST_ON:
            // keine LI/LH ausserhalb der aktuellen Liste betrachten
            nPos = m_nContextStMin;
            break;
        }
    }

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und die bisherige Vorlage setzen
    if( bSetColl )
        SetTextCollAttrs();
}

void SwHTMLParser::SetNodeNum( sal_uInt8 nLevel, bool bCountedInList )
{
    SwTextNode* pTextNode = m_pPam->GetNode().GetTextNode();
    OSL_ENSURE( pTextNode, "Kein Text-Node an PaM-Position" );

    OSL_ENSURE( GetNumInfo().GetNumRule(), "Kein Numerierungs-Regel" );
    const OUString& rName = GetNumInfo().GetNumRule()->GetName();
    static_cast<SwContentNode *>(pTextNode)->SetAttr( SwNumRuleItem(rName) );

    pTextNode->SetAttrListLevel( nLevel );
    pTextNode->SetCountedInList( bCountedInList );

    // NumRule invalidieren, weil sie durch ein EndAction bereits
    // auf valid geschaltet worden sein kann.
    GetNumInfo().GetNumRule()->SetInvalidRule( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
