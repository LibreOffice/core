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

#include <hintids.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/sfx.hrc>
#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <svl/whiter.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtpdsc.hxx>
#include <fmtflcnt.hxx>
#include <fmtinfmt.hxx>
#include <txatbase.hxx>
#include <frmatr.hxx>
#include <charfmt.hxx>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include "fldbas.hxx"
#include <breakit.hxx>
#include <htmlnum.hxx>
#include <wrthtml.hxx>
#include <htmlfly.hxx>
#include <numrule.hxx>
#include <rtl/strbuf.hxx>

#include <svtools/HtmlWriter.hxx>

#include <memory>
#include <algorithm>

using namespace css;

HTMLOutEvent aAnchorEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonclick,      OOO_STRING_SVTOOLS_HTML_O_onclick,      SFX_EVENT_MOUSECLICK_OBJECT },
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseover,  OOO_STRING_SVTOOLS_HTML_O_onmouseover,  SFX_EVENT_MOUSEOVER_OBJECT  },
    { OOO_STRING_SVTOOLS_HTML_O_SDonmouseout,       OOO_STRING_SVTOOLS_HTML_O_onmouseout,       SFX_EVENT_MOUSEOUT_OBJECT   },
    { 0,                        0,                      0                           }
};

static Writer& OutHTML_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt );

sal_uInt16 SwHTMLWriter::GetDefListLvl( const OUString& rNm, sal_uInt16 nPoolId )
{
    if( nPoolId == RES_POOLCOLL_HTML_DD )
    {
        return 1 | HTML_DLCOLL_DD;
    }
    else if( nPoolId == RES_POOLCOLL_HTML_DT )
    {
        return 1 | HTML_DLCOLL_DT;
    }

    OUString sDTDD( OOO_STRING_SVTOOLS_HTML_dt );
    sDTDD += " ";
    if( rNm.startsWith(sDTDD) )
        // DefinitionList - term
        return (sal_uInt16)rNm.copy( sDTDD.getLength() ).toInt32() | HTML_DLCOLL_DT;

    sDTDD = OOO_STRING_SVTOOLS_HTML_dd " ";
    if( rNm.startsWith(sDTDD) )
        // DefinitionList - definition
        return (sal_uInt16)rNm.copy( sDTDD.getLength() ).toInt32() | HTML_DLCOLL_DD;

    return 0;
}

void SwHTMLWriter::OutAndSetDefList( sal_uInt16 nNewLvl )
{
    // eventuell muss erst mal eine Liste aufgemacht werden
    if( nDefListLvl < nNewLvl )
    {
        // output </pre> for the previous(!) pararagraph, if required.
        // Preferable, the <pre> is exported by OutHTML_SwFormatOff for the
           // previous  paragraph already, but that's not possible, because a very
        // deep look at the next paragraph (this one) is required to figure
        // out that a def list starts here.

        ChangeParaToken( 0 );

        // entsprechend dem Level-Unterschied schreiben!
        for( sal_uInt16 i=nDefListLvl; i<nNewLvl; i++ )
        {
            if( bLFPossible )
                OutNewLine();
            HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_deflist );
            IncIndentLevel();
            bLFPossible = true;
        }
    }
    else if( nDefListLvl > nNewLvl )
    {
        for( sal_uInt16 i=nNewLvl ; i < nDefListLvl; i++ )
        {
            DecIndentLevel();
            if( bLFPossible )
                OutNewLine();
            HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_deflist, false );
            bLFPossible = true;
        }
    }

    nDefListLvl = nNewLvl;
}

void SwHTMLWriter::ChangeParaToken( sal_uInt16 nNew )
{
    if( nNew != nLastParaToken && HTML_PREFORMTXT_ON == nLastParaToken )
    {
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_preformtxt, false );
        bLFPossible = true;
    }
    nLastParaToken = nNew;
}

sal_uInt16 SwHTMLWriter::GetCSS1ScriptForScriptType( sal_uInt16 nScriptType )
{
    sal_uInt16 nRet = CSS1_OUTMODE_ANY_SCRIPT;

    switch( nScriptType )
    {
    case i18n::ScriptType::LATIN:
        nRet = CSS1_OUTMODE_WESTERN;
        break;
    case i18n::ScriptType::ASIAN:
        nRet = CSS1_OUTMODE_CJK;
        break;
    case i18n::ScriptType::COMPLEX:
        nRet = CSS1_OUTMODE_CTL;
        break;
    }

    return nRet;
}

// fuer die Formate muesste eine einzige Ausgabe-Funktion genuegen !
/*
 * Formate wie folgt ausgeben:
 * - fuer Formate, fuer die es entsprechende HTML-Tags gibt wird das
 *   Tag ausgegeben
 * - fuer alle anderen wird ein Absatz-Tag <P> ausgegeben und bUserFormat
 *   gesetzt
 * - Wenn eine Absatz-Ausrichtung am uebergebenen Item-Set des Nodes
 *   oder im Item-Set des Format gesetzt ist, wird ein ALIGN=xxx ausgegeben,
 *   sofern HTML es zulaesst
 * - in jedem Fall wird harte Attributierung als STYLE-Option geschrieben.
 *   Wenn bUserFormat nicht gesetzt ist, wird nur der uebergebene Item-Set
 *   betrachtet. Sonst werden auch Attribute des Formats ausgegeben.
 */

struct SwHTMLTextCollOutputInfo
{
    OString aToken;        // auszugendens End-Token
    std::unique_ptr<SfxItemSet> pItemSet;       // harte Attributierung

    bool bInNumBulList;         // in einer Aufzaehlungs-Liste;
    bool bParaPossible;         // ein </P> darf zusaetzlich ausgegeben werden
    bool bOutPara;              // ein </P> soll ausgegeben werden
    bool bOutDiv;               // write a </DIV>

    SwHTMLTextCollOutputInfo() :
        bInNumBulList( false ),
        bParaPossible( false ),
        bOutPara( false ),
        bOutDiv( false )
    {}

    ~SwHTMLTextCollOutputInfo();

    bool HasParaToken() const { return aToken.getLength()==1 && aToken[0]=='P'; }
    bool ShouldOutputToken() const { return bOutPara || !HasParaToken(); }
};

SwHTMLTextCollOutputInfo::~SwHTMLTextCollOutputInfo()
{
}

SwHTMLFormatInfo::SwHTMLFormatInfo( const SwFormat *pF, SwDoc *pDoc, SwDoc *pTemplate,
                              bool bOutStyles,
                              LanguageType eDfltLang,
                              sal_uInt16 nCSS1Script, bool bHardDrop )
    : pFormat(pF)
    , pRefFormat(0)
    , pItemSet(0)
    , nLeftMargin(0)
    , nRightMargin(0)
    , nFirstLineIndent(0)
    , nTopMargin(0)
    , nBottomMargin(0)
    , bScriptDependent( false )
{
    sal_uInt16 nRefPoolId = 0;
    // Den Selektor des Formats holen
    sal_uInt16 nDeep = SwHTMLWriter::GetCSS1Selector( pFormat, aToken, aClass,
                                                  nRefPoolId );
    OSL_ENSURE( nDeep ? !aToken.isEmpty() : aToken.isEmpty(),
            "Hier stimmt doch was mit dem Token nicht!" );
    OSL_ENSURE( nDeep ? nRefPoolId != 0 : nRefPoolId == 0,
            "Hier stimmt doch was mit der Vergleichs-Vorlage nicht!" );

    bool bTextColl = pFormat->Which() == RES_TXTFMTCOLL ||
                    pFormat->Which() == RES_CONDTXTFMTCOLL;

    const SwFormat *pReferenceFormat = 0; // Vergleichs-Format
    if( nDeep != 0 )
    {
        // Es ist eine HTML-Tag-Vorlage oder die Vorlage ist von einer
        // solchen abgeleitet
        if( !bOutStyles )
        {
            // wenn keine Styles exportiert werden, muss evtl. zusaetlich
            // harte Attributierung geschrieben werden
            switch( nDeep )
            {
            case CSS1_FMT_ISTAG:
            case CSS1_FMT_CMPREF:
                // fuer HTML-Tag-Vorlagen die Unterscheide zum Original
                // (sofern verfuegbar)
                pReferenceFormat = SwHTMLWriter::GetTemplateFormat( nRefPoolId,
                                                        &pTemplate->getIDocumentStylePoolAccess() );
                break;

            default:
                // sonst die zur HTML-Tag-Vorlage des Originals oder des
                // aktuellen Doks, wenn die nicht verfuegbar ist
                if( pTemplate )
                    pReferenceFormat = SwHTMLWriter::GetTemplateFormat( nRefPoolId,
                                                            &pTemplate->getIDocumentStylePoolAccess() );
                else
                    pReferenceFormat = SwHTMLWriter::GetParentFormat( *pFormat, nDeep );
                break;
            }
        }
    }
    else if( bTextColl )
    {
        // Nicht von einer HTML-Tag-Vorlage abgeleitete Absatz-Vorlagen
        // muessen als harte Attributierung relativ zur Textkoerper-Volage
        // exportiert werden. Fuer Nicht-Styles-Export sollte die der
        // HTML-Vorlage als Referenz dienen
        if( !bOutStyles && pTemplate )
            pReferenceFormat = pTemplate->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT, false );
        else
            pReferenceFormat = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT, false );
    }

    if( pReferenceFormat || nDeep==0 )
    {
        pItemSet = new SfxItemSet( *pFormat->GetAttrSet().GetPool(),
                                       pFormat->GetAttrSet().GetRanges() );
        // wenn Unterschiede zu einer anderen Vorlage geschrieben werden
        // sollen ist harte Attributierung noetig. Fuer Vorlagen, die
        // nicht von HTML-Tag-Vorlagen abgeleitet sind, gilt das immer

        pItemSet->Set( pFormat->GetAttrSet() );

        bool bSetDefaults = true, bClearSame = true;
        if( pReferenceFormat )
            SwHTMLWriter::SubtractItemSet( *pItemSet, pReferenceFormat->GetAttrSet(),
                                           bSetDefaults, bClearSame );

        // einen leeren Item-Set gleich loeschen, das spart speater
        // Arbeit
        if( !pItemSet->Count() )
        {
            delete pItemSet;
            pItemSet = 0;
        }
    }

    if( bTextColl )
    {
        if( bOutStyles )
        {
            // We have to add hard attributes for any script dependent
            // item that is not accessed by the style
            static const sal_uInt16 aWhichIds[3][4] =
            {
                { RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
                    RES_CHRATR_POSTURE, RES_CHRATR_WEIGHT },
                { RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
                    RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT },
                { RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE,
                    RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT }
            };

            sal_uInt16 nRef = 0;
            sal_uInt16 aSets[2] = {0,0};
            switch( nCSS1Script )
            {
            case CSS1_OUTMODE_WESTERN:
                nRef = 0;
                aSets[0] = 1;
                aSets[1] = 2;
                break;
            case CSS1_OUTMODE_CJK:
                nRef = 1;
                aSets[0] = 0;
                aSets[1] = 2;
                break;
            case CSS1_OUTMODE_CTL:
                nRef = 2;
                aSets[0] = 0;
                aSets[1] = 1;
                break;
            }
            for( int i=0; i<4; ++i )
            {
                const SfxPoolItem& rRef = pFormat->GetFormatAttr( aWhichIds[nRef][i] );
                for( size_t j=0; j<SAL_N_ELEMENTS(aSets); ++j )
                {
                    const SfxPoolItem& rSet = pFormat->GetFormatAttr( aWhichIds[aSets[j]][i] );
                    if( rSet != rRef )
                    {
                        if( !pItemSet )
                            pItemSet = new SfxItemSet( *pFormat->GetAttrSet().GetPool(),
                                                       pFormat->GetAttrSet().GetRanges() );
                        pItemSet->Put( rSet );
                    }
                }
            }
        }

        // Ggf. noch ein DropCap-Attribut uebernehmen
        if( bOutStyles && bHardDrop && nDeep != 0 )
        {
            const SfxPoolItem *pItem;
            if( SfxItemState::SET==pFormat->GetAttrSet().GetItemState(
                                    RES_PARATR_DROP, true, &pItem ) )
            {
                bool bPut = true;
                if( pTemplate )
                {
                    pReferenceFormat = SwHTMLWriter::GetTemplateFormat( nRefPoolId, &pTemplate->getIDocumentStylePoolAccess() );
                    const SfxPoolItem *pRefItem;
                    bool bRefItemSet =
                        SfxItemState::SET==pReferenceFormat->GetAttrSet().GetItemState(
                                        RES_PARATR_DROP, true, &pRefItem );
                    bPut = !bRefItemSet || *pItem!=*pRefItem;
                }
                if( bPut )
                {
                    if( !pItemSet )
                        pItemSet = new SfxItemSet( *pFormat->GetAttrSet().GetPool(),
                                                   pFormat->GetAttrSet().GetRanges() );
                    pItemSet->Put( *pItem );
                }
            }
        }

        // Die diversen default-Abstaende aus der Vorlage oder der
        // Vergleischs-Vorlage merken
        const SvxLRSpaceItem &rLRSpace =
            (pReferenceFormat ? pReferenceFormat : pFormat)->GetLRSpace();
        nLeftMargin = rLRSpace.GetTextLeft();
        nRightMargin = rLRSpace.GetRight();
        nFirstLineIndent = rLRSpace.GetTextFirstLineOfst();

        const SvxULSpaceItem &rULSpace =
            (pReferenceFormat ? pReferenceFormat : pFormat)->GetULSpace();
        nTopMargin = rULSpace.GetUpper();
        nBottomMargin = rULSpace.GetLower();

        // export language if it differs from the default language
        sal_uInt16 nWhichId =
            SwHTMLWriter::GetLangWhichIdFromScript( nCSS1Script );
        const SvxLanguageItem& rLang =
            static_cast<const SvxLanguageItem&>(pFormat->GetFormatAttr( nWhichId ));
        LanguageType eLang = rLang.GetLanguage();
        if( eLang != eDfltLang )
        {
            if( !pItemSet )
                pItemSet = new SfxItemSet( *pFormat->GetAttrSet().GetPool(),
                                           pFormat->GetAttrSet().GetRanges() );
            pItemSet->Put( rLang );
        }

        static const sal_uInt16 aWhichIds[3] =
            { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
                RES_CHRATR_CTL_LANGUAGE };
        for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
        {
            if( aWhichIds[i] != nWhichId )
            {
                const SvxLanguageItem& rTmpLang =
                    static_cast<const SvxLanguageItem&>(pFormat->GetFormatAttr(aWhichIds[i]));
                if( rTmpLang.GetLanguage() != eLang )
                {
                    if( !pItemSet )
                        pItemSet = new SfxItemSet( *pFormat->GetAttrSet().GetPool(),
                                                   pFormat->GetAttrSet().GetRanges() );
                    pItemSet->Put( rTmpLang );
                }
            }
        }
    }
}

SwHTMLFormatInfo::~SwHTMLFormatInfo()
{
    delete pItemSet;
}

void OutHTML_SwFormat( Writer& rWrt, const SwFormat& rFormat,
                    const SfxItemSet *pNodeItemSet,
                    SwHTMLTextCollOutputInfo& rInfo )
{
    OSL_ENSURE( RES_CONDTXTFMTCOLL==rFormat.Which() || RES_TXTFMTCOLL==rFormat.Which(),
            "keine Absatz-Vorlage" );

    SwHTMLWriter & rHWrt = static_cast<SwHTMLWriter&>(rWrt);

    // Erstmal ein par Flags ...
    sal_uInt16 nNewDefListLvl = 0;
    sal_uInt16 nNumStart = USHRT_MAX;
    bool bForceDL = false;
    bool bDT = false;
    rInfo.bInNumBulList = false;    // Wir sind in einer Liste?
    bool bNumbered = false;         // Der aktuelle Absatz ist numeriert
    bool bPara = false;             // das aktuelle Token ist <P>
    rInfo.bParaPossible = false;    // ein <P> darf zusaetzlich ausgegeben werden
    bool bNoEndTag = false;         // kein End-Tag ausgeben

    rHWrt.bNoAlign = false;         // kein ALIGN=... moeglich
    bool bNoStyle = false;          // kein STYLE=... moeglich
    sal_uInt8 nBulletGrfLvl = 255;      // Die auszugebende Bullet-Grafik

    // Sind wir in einer Aufzaehlungs- oder Numerierungliste?
    const SwTextNode* pTextNd = rWrt.pCurPam->GetNode().GetTextNode();

    SwHTMLNumRuleInfo aNumInfo;
    if( rHWrt.GetNextNumInfo() )
    {
        aNumInfo = *rHWrt.GetNextNumInfo();
        rHWrt.ClearNextNumInfo();
    }
    else
    {
        aNumInfo.Set( *pTextNd );
    }

    if( aNumInfo.GetNumRule() )
    {
        rInfo.bInNumBulList = true;
        nNewDefListLvl = 0;

        // ist der aktuelle Absatz numeriert?
        bNumbered = aNumInfo.IsNumbered();
        sal_uInt8 nLvl = aNumInfo.GetLevel();

        OSL_ENSURE( pTextNd->GetActualListLevel() == nLvl,
                "Gemerkter Num-Level ist falsch" );
        OSL_ENSURE( bNumbered == pTextNd->IsCountedInList(),
                "Gemerkter Numerierungs-Zustand ist falsch" );

        if( bNumbered )
        {
            nBulletGrfLvl = nLvl; // nur veruebergehend!!!
            // #i57919#
            // correction of re-factoring done by cws swnumtree:
            // - <nNumStart> has to contain the restart value, if the
            //   numbering is restarted at this text node. Value <USHRT_MAX>
            //   indicates, that no additional restart value has to be written.
            if ( pTextNd->IsListRestart() )
            {
                nNumStart = static_cast< sal_uInt16 >(pTextNd->GetActualListStartValue());
            }
            OSL_ENSURE( rHWrt.nLastParaToken == 0,
                "<PRE> wurde nicht vor <LI> beendet." );
        }
    }

    // Jetzt holen wir das Token und ggf. die Klasse
    std::unique_ptr<SwHTMLFormatInfo> pTmpInfo(new SwHTMLFormatInfo(&rFormat));
    SwHTMLFormatInfo *pFormatInfo;
    SwHTMLFormatInfos::iterator it = rHWrt.m_TextCollInfos.find( pTmpInfo );
    if (it != rHWrt.m_TextCollInfos.end())
    {
        pFormatInfo = it->get();
    }
    else
    {
        pFormatInfo = new SwHTMLFormatInfo( &rFormat, rWrt.pDoc, rHWrt.pTemplate,
                                      rHWrt.bCfgOutStyles, rHWrt.eLang,
                                      rHWrt.nCSS1Script,
                                      false );
        rHWrt.m_TextCollInfos.insert(std::unique_ptr<SwHTMLFormatInfo>(pFormatInfo));
        if( rHWrt.aScriptParaStyles.count( rFormat.GetName() ) )
            pFormatInfo->bScriptDependent = true;
    }

    // Jetzt wird festgelegt, was aufgrund des Tokens so moeglich ist
    sal_uInt16 nToken = 0;          // Token fuer Tag-Wechsel
    bool bOutNewLine = false;   // nur ein LF ausgeben?
    if( !pFormatInfo->aToken.isEmpty() )
    {
        // Es ist eine HTML-Tag-Vorlage oder die Vorlage ist von einer
        // solchen abgeleitet
        rInfo.aToken = pFormatInfo->aToken;

        if (rInfo.aToken == OOO_STRING_SVTOOLS_HTML_address)
        {
            rInfo.bParaPossible = true;
            rHWrt.bNoAlign = true;
        }
        else if (rInfo.aToken == OOO_STRING_SVTOOLS_HTML_blockquote)
        {
            rInfo.bParaPossible = true;
            rHWrt.bNoAlign = true;
        }
        else if (rInfo.aToken == OOO_STRING_SVTOOLS_HTML_parabreak)
        {
            bPara = true;
        }
        else if (rInfo.aToken == OOO_STRING_SVTOOLS_HTML_preformtxt)
        {
            if (HTML_PREFORMTXT_ON == rHWrt.nLastParaToken)
            {
                bOutNewLine = true;
            }
            else
            {
                nToken = HTML_PREFORMTXT_ON;
                rHWrt.bNoAlign = true;
                bNoEndTag = true;
            }
        }
        else if (rInfo.aToken == OOO_STRING_SVTOOLS_HTML_dt || rInfo.aToken == OOO_STRING_SVTOOLS_HTML_dd)
        {
            bDT = rInfo.aToken == OOO_STRING_SVTOOLS_HTML_dt;
            rInfo.bParaPossible = !bDT;
            rHWrt.bNoAlign = true;
            bForceDL = true;
        }
    }
    else
    {
        // alle Vorlagen, die nicht einem HTML-Tag entsprechen oder von
        // diesem abgeleitet sind, werden als <P> exportiert

        rInfo.aToken = OOO_STRING_SVTOOLS_HTML_parabreak;
        bPara = true;
    }

    // Falls noetig, die harte Attributierung der Vorlage uebernehmen
    if( pFormatInfo->pItemSet )
    {
        OSL_ENSURE( !rInfo.pItemSet.get(), "Wo kommt der Item-Set her?" );
        rInfo.pItemSet.reset(new SfxItemSet( *pFormatInfo->pItemSet ));
    }

    // und noch die harte Attributierung des Absatzes dazunehmen
    if( pNodeItemSet )
    {
        if( rInfo.pItemSet.get() )
            rInfo.pItemSet->Put( *pNodeItemSet );
        else
            rInfo.pItemSet.reset(new SfxItemSet( *pNodeItemSet ));
    }

    // den unteren Absatz-Abstand brauchen wir noch
    const SvxULSpaceItem& rULSpace =
        pNodeItemSet ? static_cast<const SvxULSpaceItem &>(pNodeItemSet->Get(RES_UL_SPACE))
                     : rFormat.GetULSpace();

    if( (rHWrt.bOutHeader &&
         rWrt.pCurPam->GetPoint()->nNode.GetIndex() ==
            rWrt.pCurPam->GetMark()->nNode.GetIndex()) ||
         rHWrt.bOutFooter )
    {
        if( rHWrt.bCfgOutStyles )
        {
            SvxULSpaceItem aULSpaceItem( rULSpace );
            if( rHWrt.bOutHeader )
                aULSpaceItem.SetLower( rHWrt.nHeaderFooterSpace );
            else
                aULSpaceItem.SetUpper( rHWrt.nHeaderFooterSpace );

            if (!rInfo.pItemSet.get())
            {
                rInfo.pItemSet.reset(new SfxItemSet(*rFormat.GetAttrSet().GetPool(), RES_UL_SPACE, RES_UL_SPACE));
            }
            rInfo.pItemSet->Put( aULSpaceItem );
        }
        rHWrt.bOutHeader = false;
        rHWrt.bOutFooter = false;
    }

    if( bOutNewLine )
    {
        // nur einen Zeilen-Umbruch (ohne Einrueckung) am Absatz-Anfang
        // ausgeben
        rInfo.aToken.clear();   // kein End-Tag ausgeben
        rWrt.Strm().WriteCharPtr( SAL_NEWLINE_STRING );

        return;
    }

    // soll ein ALIGN=... geschrieben werden?
    const SfxPoolItem* pAdjItem = 0;
    const SfxPoolItem* pItem;

    if( rInfo.pItemSet &&
        SfxItemState::SET == rInfo.pItemSet->GetItemState( RES_PARATR_ADJUST,
                                                      false, &pItem ) )
    {
        pAdjItem = pItem;
    }

    // Unteren Absatz-Abstand beachten ? (nie im letzen Absatz von
    // Tabellen)
    bool bUseParSpace = !rHWrt.bOutTable ||
                        (rWrt.pCurPam->GetPoint()->nNode.GetIndex() !=
                         rWrt.pCurPam->GetMark()->nNode.GetIndex());
    // Wenn Styles exportiert werden, wird aus eingerueckten Absaetzen
    // eine Definitions-Liste
    const SvxLRSpaceItem& rLRSpace =
        pNodeItemSet ? static_cast<const SvxLRSpaceItem &>(pNodeItemSet->Get(RES_LR_SPACE))
                     : rFormat.GetLRSpace();
    if( (!rHWrt.bCfgOutStyles || bForceDL) && !rInfo.bInNumBulList )
    {
        sal_Int32 nLeftMargin;
        if( bForceDL )
            nLeftMargin = rLRSpace.GetTextLeft();
        else
            nLeftMargin = rLRSpace.GetTextLeft() > pFormatInfo->nLeftMargin
                ? rLRSpace.GetTextLeft() - pFormatInfo->nLeftMargin
                : 0;

        if( nLeftMargin > 0 && rHWrt.nDefListMargin > 0 )
        {
            nNewDefListLvl = static_cast< sal_uInt16 >((nLeftMargin + (rHWrt.nDefListMargin/2)) /
                                                    rHWrt.nDefListMargin);
            if( nNewDefListLvl == 0 && bForceDL && !bDT )
                nNewDefListLvl = 1;
        }
        else
        {
            // If the left margin is 0 or negative, emulating indent
            // with <dd> does not work. We then set a def list only if
            // the dd style is used.
            nNewDefListLvl = (bForceDL&& !bDT) ? 1 : 0;
        }

        bool bIsNextTextNode =
            rWrt.pDoc->GetNodes()[rWrt.pCurPam->GetPoint()->nNode.GetIndex()+1]
                     ->IsTextNode();

        if( bForceDL && bDT )
        {
            // Statt eines DD muessen wir hier auch ein DT der Ebene
            // darueber nehmen
            nNewDefListLvl++;
        }
        else if( !nNewDefListLvl && !rHWrt.bCfgOutStyles && bPara &&
                 rULSpace.GetLower()==0 &&
                 ((bUseParSpace && bIsNextTextNode) || rHWrt.nDefListLvl==1) &&
                 (!pAdjItem || SVX_ADJUST_LEFT==
                    static_cast<const SvxAdjustItem *>(pAdjItem)->GetAdjust()) )
        {
            // Absaetze ohne unteren Abstand als DT exportieren
            nNewDefListLvl = 1;
            bDT = true;
            rInfo.bParaPossible = false;
            rHWrt.bNoAlign = true;
        }
    }

    if( nNewDefListLvl != rHWrt.nDefListLvl )
        rHWrt.OutAndSetDefList( nNewDefListLvl );

    // ggf. eine Aufzaehlung- oder Numerierungsliste beginnen
    if( rInfo.bInNumBulList )
    {
        OSL_ENSURE( !rHWrt.nDefListLvl, "DL in OL geht nicht!" );
        OutHTML_NumBulListStart( rHWrt, aNumInfo );

        if( bNumbered )
        {
            if( !rHWrt.aBulletGrfs[nBulletGrfLvl].isEmpty()  )
                bNumbered = false;
            else
                nBulletGrfLvl = 255;
        }
    }

    // Die Defaults aus der Vorlage merken, denn sie muessen nicht
    // exportiert werden
    rHWrt.nDfltLeftMargin = pFormatInfo->nLeftMargin;
    rHWrt.nDfltRightMargin = pFormatInfo->nRightMargin;
    rHWrt.nDfltFirstLineIndent = pFormatInfo->nFirstLineIndent;

    if( rInfo.bInNumBulList )
    {
        if( !rHWrt.IsHTMLMode( HTMLMODE_LSPACE_IN_NUMBUL ) )
            rHWrt.nDfltLeftMargin = rLRSpace.GetTextLeft();

        // In Numerierungs-Listen keinen Ertzeilen-Einzug ausgeben.
        rHWrt.nFirstLineIndent = rLRSpace.GetTextFirstLineOfst();
    }

    if( rInfo.bInNumBulList && bNumbered && bPara && !rHWrt.bCfgOutStyles )
    {
        // ein einzelnes LI hat keinen Abstand
        rHWrt.nDfltTopMargin = 0;
        rHWrt.nDfltBottomMargin = 0;
    }
    else if( rHWrt.nDefListLvl && bPara )
    {
        // ein einzelnes DD hat auch keinen Abstand
        rHWrt.nDfltTopMargin = 0;
        rHWrt.nDfltBottomMargin = 0;
    }
    else
    {
        rHWrt.nDfltTopMargin = pFormatInfo->nTopMargin;
        // Wenn im letzten Absatz einer Tabelle der
        // untere Absatz-Abstand veraendert wird, vertut sich
        // Netscape total. Deshalb exportieren wir hier erstmal
        // nichts, indem wir den Abstand aus dem Absatz als Default
        // setzen.
        if( rHWrt.bCfgNetscape4 && !bUseParSpace )
            rHWrt.nDfltBottomMargin = rULSpace.GetLower();
        else
            rHWrt.nDfltBottomMargin = pFormatInfo->nBottomMargin;
    }

    if( rHWrt.nDefListLvl )
    {
        rHWrt.nLeftMargin =
            (rHWrt.nDefListLvl-1) * rHWrt.nDefListMargin;
    }

    if( rHWrt.bLFPossible )
        rHWrt.OutNewLine(); // Absatz-Tag in neue Zeile
    rInfo.bOutPara = false;

    // das ist jetzt unser neues Token
    rHWrt.ChangeParaToken( nToken );

    bool bHasParSpace = bUseParSpace && rULSpace.GetLower() > 0;

    // ggf ein List-Item aufmachen
    if( rInfo.bInNumBulList && bNumbered )
    {
        HtmlWriter html(rWrt.Strm());
        html.start(OOO_STRING_SVTOOLS_HTML_li);
        if( USHRT_MAX != nNumStart )
            html.attribute(OOO_STRING_SVTOOLS_HTML_O_value, OString::number(nNumStart));
        html.endAttribute();
    }

    if( rHWrt.nDefListLvl > 0 && !bForceDL )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), bDT ? OOO_STRING_SVTOOLS_HTML_dt : OOO_STRING_SVTOOLS_HTML_dd );
    }

    if( pAdjItem &&
        rHWrt.IsHTMLMode( HTMLMODE_NO_CONTROL_CENTERING ) &&
        rHWrt.HasControls() )
    {
        // The align=... attribute does behave strange in netscape
        // if there are controls in a paragraph, because the control and
        // all text behind the control does not recognize this attribute.
        OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_division);
        rWrt.Strm().WriteOString( sOut );

        rHWrt.bTextAttr = false;
        rHWrt.bOutOpts = true;
        OutHTML_SvxAdjust( rWrt, *pAdjItem );
        rWrt.Strm().WriteChar( '>' );
        pAdjItem = 0;
        rHWrt.bNoAlign = false;
        rInfo.bOutDiv = true;
        rHWrt.IncIndentLevel();
        rHWrt.bLFPossible = true;
            rHWrt.OutNewLine();
    }

    // fuer BLOCKQUOTE, ADDRESS und DD wird noch ein Absatz-Token
    // ausgegeben, wenn,
    // - keine Styles geschrieben werden, und
    // - ein untere Abstand oder eine Absatz-Ausrichtung existiert
    OString aToken = rInfo.aToken;
    if( !rHWrt.bCfgOutStyles && rInfo.bParaPossible && !bPara &&
        (bHasParSpace || pAdjItem) )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rInfo.aToken.getStr() );
        aToken = OOO_STRING_SVTOOLS_HTML_parabreak;
        bPara = true;
        rHWrt.bNoAlign = false;
        bNoStyle = false;
    }

    LanguageType eLang;
    if (rInfo.pItemSet)
        eLang = static_cast<const SvxLanguageItem&>(rInfo.pItemSet->Get(SwHTMLWriter::GetLangWhichIdFromScript(rHWrt.nCSS1Script))).GetLanguage();
    else
        eLang = rHWrt.eLang;

    if( rInfo.pItemSet )
    {
        static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE };

        for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
        {
            // export language if it differs from the default language only.
            const SfxPoolItem *pTmpItem;
            if( SfxItemState::SET == rInfo.pItemSet->GetItemState( aWhichIds[i],
                        true, &pTmpItem ) &&
                static_cast<const SvxLanguageItem *>(pTmpItem)->GetLanguage() == eLang )
                rInfo.pItemSet->ClearItem( aWhichIds[i] );
        }
    }

    // and the text direction
    sal_uInt16 nDir = rHWrt.GetHTMLDirection(
            (pNodeItemSet ? static_cast < const SvxFrameDirectionItem& >(
                                    pNodeItemSet->Get( RES_FRAMEDIR ) )
                          : rFormat.GetFrmDir() ).GetValue() );

    // Ein <P> wird nur geschrieben, wenn
    // - wir in keiner OL/UL/DL sind, oder
    // - der Absatz einer OL/UL nicht numeriert ist, oder
    // - keine Styles exportiert werden und
    //      - ein unterer Abstand oder
    //      - eine Absatz-Ausrichtung existiert, ode
    // - Styles exportiert werden und,
    //      - die Textkoerper-Vorlage geaendert wurde, oder
    //      - ein Benutzer-Format exportiert wird, oder
    //      - Absatz-Attribute existieren
    if( !bPara ||
        (!rInfo.bInNumBulList && !rHWrt.nDefListLvl) ||
        (rInfo.bInNumBulList && !bNumbered) ||
        (!rHWrt.bCfgOutStyles &&
         (bHasParSpace || pAdjItem ||
          (eLang != LANGUAGE_DONTKNOW && eLang != rHWrt.eLang))) ||
        nDir != rHWrt.nDirection ||
        rHWrt.bCfgOutStyles )
    {
        // jetzt werden Optionen ausgegeben
        rHWrt.bTextAttr = false;
        rHWrt.bOutOpts = true;

        OString sOut = "<" + aToken;

        if( eLang != LANGUAGE_DONTKNOW && eLang != rHWrt.eLang )
        {
            rWrt.Strm().WriteOString( sOut );
            sOut = "";
            rHWrt.OutLanguage( eLang );
        }

        if( nDir != rHWrt.nDirection )
        {
            if( !sOut.isEmpty() )
            {
                rWrt.Strm().WriteOString( sOut );
                sOut = "";
            }
            rHWrt.OutDirection( nDir );
        }

        if( rHWrt.bCfgOutStyles &&
            (!pFormatInfo->aClass.isEmpty() || pFormatInfo->bScriptDependent) )
        {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_class) + "=\"";
            rWrt.Strm().WriteOString( sOut );
            sOut = "";
            OUString aClass( pFormatInfo->aClass );
            if( pFormatInfo->bScriptDependent )
            {
                if( !aClass.isEmpty() )
                   aClass += "-";
                switch( rHWrt.nCSS1Script )
                {
                case CSS1_OUTMODE_WESTERN:
                    aClass += "western";
                    break;
                case CSS1_OUTMODE_CJK:
                    aClass += "cjk";
                    break;
                case CSS1_OUTMODE_CTL:
                    aClass += "ctl";
                    break;
                }
            }
            HTMLOutFuncs::Out_String( rWrt.Strm(), aClass,
                                      rHWrt.eDestEnc, &rHWrt.aNonConvertableCharacters );
            sOut += "\"";
        }
        rWrt.Strm().WriteOString( sOut );
        sOut = "";

        // ggf. Ausrichtung ausgeben.
        if( !rHWrt.bNoAlign && pAdjItem )
            OutHTML_SvxAdjust( rWrt, *pAdjItem );

        rHWrt.bParaDotLeaders = bPara && rHWrt.bCfgPrintLayout && rHWrt.indexOfDotLeaders(
                pTextNd->GetAnyFormatColl().GetPoolFormatId(), pTextNd->GetText()) > -1;

        // und nun ggf. noch die STYLE-Option
        if( rHWrt.bCfgOutStyles && rInfo.pItemSet && !bNoStyle)
        {
            OutCSS1_ParaTagStyleOpt( rWrt, *rInfo.pItemSet );
        }

        if (rHWrt.bParaDotLeaders) {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_class) + "=\"" +
                OString(sCSS2_P_CLASS_leaders) + "\"><" +
                OString(OOO_STRING_SVTOOLS_HTML_O_span);
            rWrt.Strm().WriteOString( sOut );
            sOut = "";
        }

        rWrt.Strm().WriteChar( '>' );

        // Soll ein </P> geschrieben wenrden
        rInfo.bOutPara =
            bPara &&
            ( rHWrt.bCfgOutStyles || bHasParSpace );

        // wenn kein End-Tag geschrieben werden soll, es loeschen
        if( bNoEndTag )
            rInfo.aToken.clear();
    }

    if( nBulletGrfLvl != 255 )
    {
        OSL_ENSURE( aNumInfo.GetNumRule(), "Wo ist die Numerierung geblieben???" );
        OSL_ENSURE( nBulletGrfLvl < MAXLEVEL, "So viele Ebenen gibt's nicht" );
        const SwNumFormat& rNumFormat = aNumInfo.GetNumRule()->Get(nBulletGrfLvl);
        OutHTML_BulletImage( rWrt, OOO_STRING_SVTOOLS_HTML_image, rNumFormat.GetBrush() );
    }

    rHWrt.GetNumInfo() = aNumInfo;

    // die Defaults zuruecksetzen
    rHWrt.nDfltLeftMargin = 0;
    rHWrt.nDfltRightMargin = 0;
    rHWrt.nDfltFirstLineIndent = 0;
    rHWrt.nDfltTopMargin = 0;
    rHWrt.nDfltBottomMargin = 0;
    rHWrt.nLeftMargin = 0;
    rHWrt.nFirstLineIndent = 0;
}

void OutHTML_SwFormatOff( Writer& rWrt, const SwHTMLTextCollOutputInfo& rInfo )
{
    SwHTMLWriter & rHWrt = static_cast<SwHTMLWriter&>(rWrt);

    // wenn es kein Token gibt haben wir auch nichts auszugeben
    if( rInfo.aToken.isEmpty() )
    {
        rHWrt.FillNextNumInfo();
        const SwHTMLNumRuleInfo& rNextInfo = *rHWrt.GetNextNumInfo();
        // Auch in PRE muss eine Bullet-Liste beendet werden
        if( rInfo.bInNumBulList )
        {

            const SwHTMLNumRuleInfo& rNRInfo = rHWrt.GetNumInfo();
            if( rNextInfo.GetNumRule() != rNRInfo.GetNumRule() ||
                rNextInfo.GetDepth() != rNRInfo.GetDepth() ||
                rNextInfo.IsNumbered() || rNextInfo.IsRestart() )
                rHWrt.ChangeParaToken( 0 );
            OutHTML_NumBulListEnd( rHWrt, rNextInfo );
        }
        else if( rNextInfo.GetNumRule() != 0 )
            rHWrt.ChangeParaToken( 0 );

        return;
    }

    if( rInfo.ShouldOutputToken() )
    {
        if( rHWrt.bLFPossible )
            rHWrt.OutNewLine( true );

        // fuer BLOCKQUOTE, ADDRESS und DD wird ggf noch ein
        // Absatz-Token ausgegeben, wenn
        // - keine Styles geschrieben werden, und
        // - ein untere Abstand existiert
        if( rInfo.bParaPossible && rInfo.bOutPara )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_parabreak, false );

        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rInfo.aToken.getStr(), false );
        rHWrt.bLFPossible =
            rInfo.aToken != OOO_STRING_SVTOOLS_HTML_dt &&
            rInfo.aToken != OOO_STRING_SVTOOLS_HTML_dd &&
            rInfo.aToken != OOO_STRING_SVTOOLS_HTML_li;
    }
    if( rInfo.bOutDiv )
    {
        rHWrt.DecIndentLevel();
        if( rHWrt.bLFPossible )
            rHWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_division, false );
        rHWrt.bLFPossible = true;
    }

    // ggf. eine Aufzaehlung- oder Numerierungsliste beenden
    if( rInfo.bInNumBulList )
    {
        rHWrt.FillNextNumInfo();
        OutHTML_NumBulListEnd( rHWrt, *rHWrt.GetNextNumInfo() );
    }
}

class HTMLStartEndPos
{
    sal_Int32 nStart;
    sal_Int32 nEnd;
    SfxPoolItem* pItem;

public:

    HTMLStartEndPos( const SfxPoolItem& rItem, sal_Int32 nStt, sal_Int32 nE );
    ~HTMLStartEndPos();

    const SfxPoolItem *GetItem() const { return pItem; }

    void SetStart( sal_Int32 nStt ) { nStart = nStt; }
    sal_Int32 GetStart() const { return nStart; }

    sal_Int32 GetEnd() const { return nEnd; }
    void SetEnd( sal_Int32 nE ) { nEnd = nE; }
};

HTMLStartEndPos::HTMLStartEndPos( const SfxPoolItem& rItem, sal_Int32 nStt,
                                                        sal_Int32 nE ) :
    nStart( nStt ),
    nEnd( nE ),
    pItem( rItem.Clone() )
{}

HTMLStartEndPos::~HTMLStartEndPos()
{
    delete pItem;
}

typedef std::vector<HTMLStartEndPos *> HTMLStartEndPositions;

enum HTMLOnOffState { HTML_NOT_SUPPORTED,   // unsupported Attribute
                      HTML_REAL_VALUE,      // Attribute with value
                      HTML_ON_VALUE,        // Attribute is On-Tag
                      HTML_OFF_VALUE,       // Attribute is Off-Tag
                      HTML_CHRFMT_VALUE,    // Attribute for character format
                      HTML_COLOR_VALUE,     // Attribute for foreground color
                      HTML_STYLE_VALUE,     // Attribute must be exported as style
                      HTML_DROPCAP_VALUE,   // DropCap-Attribute
                      HTML_AUTOFMT_VALUE }; // Attribute for automatic character styles

class HTMLEndPosLst
{
    HTMLStartEndPositions aStartLst;  // nach Anfangs-Psoitionen sortierte Liste
    HTMLStartEndPositions aEndLst;    // nach End-Psotionen sortierte Liste
    std::deque<sal_Int32> aScriptChgLst;    // positions where script changes
                                    // 0 is not contained in this list,
                                    // but the text length
    // the script that is valid up to the position
    // contained in aScriptChgList at the same index
    ::std::vector<sal_uInt16> aScriptLst;

    SwDoc *pDoc;            // das aktuelle Dokument
    SwDoc* pTemplate;       // die HTML-Vorlage (oder 0)
    const Color* pDfltColor;// die Default-Vordergrund-Farbe
    std::set<OUString>& rScriptTextStyles;

    sal_uLong nHTMLMode;
    bool bOutStyles : 1;    // werden Styles exportiert

    // Eine SttEndPos in die Start- und Ende-Listen eintragen bzw. aus
    // ihnen loeschen, wobei die Ende-Position bekannt ist
    void _InsertItem( HTMLStartEndPos *pPos, HTMLStartEndPositions::size_type nEndPos );
    void _RemoveItem( HTMLStartEndPositions::size_type nEndPos );

    // die "Art" es Attributs ermitteln
    HTMLOnOffState GetHTMLItemState( const SfxPoolItem& rItem );

    // Existiert ein bestimmtes On-Tag-Item
    bool ExistsOnTagItem( sal_uInt16 nWhich, sal_Int32 nPos );

    // Existiert ein Item zum ausschalten eines Attributs, das genauso
    // exportiert wird wie das uebergebene Item im gleichen Bereich?
    bool ExistsOffTagItem( sal_uInt16 nWhich, sal_Int32 nStartPos,
                                          sal_Int32 nEndPos );

    // das Ende eines gesplitteten Items anpassen
    void FixSplittedItem( HTMLStartEndPos *pPos, sal_Int32 nNewEnd,
                            HTMLStartEndPositions::size_type nStartPos );

    // Ein Attribut in die Listen eintragen und ggf. aufteilen
    void InsertItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                               sal_Int32 nEnd );

    // Ein bereits vorhandenes Attribut aufteilen
    void SplitItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                              sal_Int32 nEnd );

    // Insert without taking care of script
    void InsertNoScript( const SfxPoolItem& rItem, sal_Int32 nStart,
                          sal_Int32 nEnd, SwHTMLFormatInfos& rFormatInfos,
                         bool bParaAttrs=false );

    const SwHTMLFormatInfo *GetFormatInfo( const SwFormat& rFormat,
                                     SwHTMLFormatInfos& rFormatInfos );

public:

    HTMLEndPosLst( SwDoc *pDoc, SwDoc* pTemplate, const Color* pDfltColor,
                   bool bOutStyles, sal_uLong nHTMLMode,
                   const OUString& rText, std::set<OUString>& rStyles );
    ~HTMLEndPosLst();

    // Ein Attribut einfuegen
    void Insert( const SfxPoolItem& rItem, sal_Int32 nStart,  sal_Int32 nEnd,
                 SwHTMLFormatInfos& rFormatInfos, bool bParaAttrs=false );
    void Insert( const SfxItemSet& rItemSet, sal_Int32 nStart, sal_Int32 nEnd,
                 SwHTMLFormatInfos& rFormatInfos, bool bDeep,
                 bool bParaAttrs=false );
    void Insert( const SwDrawFrameFormat& rFormat, sal_Int32 nPos,
                 SwHTMLFormatInfos& rFormatInfos );

    sal_uInt16 GetScriptAtPos( sal_Int32 nPos,
                               sal_uInt16 nWeak=CSS1_OUTMODE_ANY_SCRIPT );

    void OutStartAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                        HTMLOutContext *pContext = 0 );
    void OutEndAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                      HTMLOutContext *pContext = 0 );

    bool IsHTMLMode( sal_uLong nMode ) const { return (nHTMLMode & nMode) != 0; }
};

void HTMLEndPosLst::_InsertItem( HTMLStartEndPos *pPos, HTMLStartEndPositions::size_type nEndPos )
{
    // In der Start-Liste das Attribut hinter allen vorher und an
    // der gleichen Position gestarteten Attributen einfuegen
    sal_Int32 nStart = pPos->GetStart();
    HTMLStartEndPositions::size_type i {0};

    while( i < aStartLst.size() && aStartLst[i]->GetStart() <= nStart )
        ++i;
    aStartLst.insert( aStartLst.begin() + i, pPos );

    // die Position in der Ende-Liste wurde uebergeben
    aEndLst.insert( aEndLst.begin() + nEndPos, pPos );
}

void HTMLEndPosLst::_RemoveItem( HTMLStartEndPositions::size_type nEndPos )
{
    HTMLStartEndPos *pPos = aEndLst[nEndPos];

    // jetzt Suchen wir es in der Start-Liste
    HTMLStartEndPositions::iterator it =
        std::find(aStartLst.begin(), aStartLst.end(), pPos );
    OSL_ENSURE(it != aStartLst.end(), "Item not found in Start List!");
    if( it != aStartLst.end() )
        aStartLst.erase( it );

    aEndLst.erase( aEndLst.begin() + nEndPos );

    delete pPos;
}

HTMLOnOffState HTMLEndPosLst::GetHTMLItemState( const SfxPoolItem& rItem )
{
    HTMLOnOffState eState = HTML_NOT_SUPPORTED;
    switch( rItem.Which() )
    {
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CTL_POSTURE:
        switch( static_cast<const SvxPostureItem&>(rItem).GetPosture() )
        {
        case ITALIC_NORMAL:
            eState = HTML_ON_VALUE;
            break;
        case ITALIC_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_CROSSEDOUT:
        switch( static_cast<const SvxCrossedOutItem&>(rItem).GetStrikeout() )
        {
        case STRIKEOUT_SINGLE:
        case STRIKEOUT_DOUBLE:
            eState = HTML_ON_VALUE;
            break;
        case STRIKEOUT_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            ;
        }
        break;

    case RES_CHRATR_ESCAPEMENT:
        switch( (const SvxEscapement)
                        static_cast<const SvxEscapementItem&>(rItem).GetEnumValue() )
        {
        case SVX_ESCAPEMENT_SUPERSCRIPT:
        case SVX_ESCAPEMENT_SUBSCRIPT:
            eState = HTML_ON_VALUE;
            break;
        case SVX_ESCAPEMENT_OFF:
            eState = HTML_OFF_VALUE;
            break;
        default:
            ;
        }
        break;

    case RES_CHRATR_UNDERLINE:
        switch( static_cast<const SvxUnderlineItem&>(rItem).GetLineStyle() )
        {
        case UNDERLINE_SINGLE:
            eState = HTML_ON_VALUE;
            break;
        case UNDERLINE_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_OVERLINE:
    case RES_CHRATR_HIDDEN:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_WEIGHT:
    case RES_CHRATR_CJK_WEIGHT:
    case RES_CHRATR_CTL_WEIGHT:
        switch( static_cast<const SvxWeightItem&>(rItem).GetWeight() )
        {
        case WEIGHT_BOLD:
            eState = HTML_ON_VALUE;
            break;
        case WEIGHT_NORMAL:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_BLINK:
        eState = static_cast<const SvxBlinkItem&>(rItem).GetValue() ? HTML_ON_VALUE
                                                         : HTML_OFF_VALUE;
        break;

    case RES_CHRATR_COLOR:
        eState = HTML_COLOR_VALUE;
        break;

    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_TXTATR_INETFMT:
        eState = HTML_REAL_VALUE;
        break;

    case RES_TXTATR_CHARFMT:
        eState = HTML_CHRFMT_VALUE;
        break;

    case RES_TXTATR_AUTOFMT:
        eState = HTML_AUTOFMT_VALUE;
        break;

    case RES_CHRATR_CASEMAP:
        eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_KERNING:
        eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_BACKGROUND:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_PARATR_DROP:
        eState = HTML_DROPCAP_VALUE;
        break;

    case RES_CHRATR_BOX:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;
    }

    return eState;
}

bool HTMLEndPosLst::ExistsOnTagItem( sal_uInt16 nWhich, sal_Int32 nPos )
{
    for( auto pTest : aStartLst )
    {
        if( pTest->GetStart() > nPos )
        {
            // dieses uns alle folgenden Attribute beginnen spaeter
            break;
        }
        else if( pTest->GetEnd() > nPos )
        {
            // das Attribut beginnt vor oder an der aktuellen Position
            // und endet hinter ihr
            const SfxPoolItem *pItem = pTest->GetItem();
            if( pItem->Which() == nWhich &&
                HTML_ON_VALUE == GetHTMLItemState(*pItem) )
            {
                // ein On-Tag-Attibut wurde gefunden
                return true;
            }
        }
    }

    return false;
}

bool HTMLEndPosLst::ExistsOffTagItem( sal_uInt16 nWhich, sal_Int32 nStartPos,
                                      sal_Int32 nEndPos )
{
    if( nWhich != RES_CHRATR_CROSSEDOUT &&
        nWhich != RES_CHRATR_UNDERLINE &&
        nWhich != RES_CHRATR_BLINK )
    {
        return false;
    }

    for( auto pTest : aStartLst )
    {
        if( pTest->GetStart() > nStartPos )
        {
            // dieses uns alle folgenden Attribute beginnen spaeter
            break;
        }
        else if( pTest->GetStart()==nStartPos &&
                 pTest->GetEnd()==nEndPos )
        {
            // das Attribut beginnt vor oder an der aktuellen Position
            // und endet hinter ihr
            const SfxPoolItem *pItem = pTest->GetItem();
            sal_uInt16 nTstWhich = pItem->Which();
            if( (nTstWhich == RES_CHRATR_CROSSEDOUT ||
                 nTstWhich == RES_CHRATR_UNDERLINE ||
                 nTstWhich == RES_CHRATR_BLINK) &&
                HTML_OFF_VALUE == GetHTMLItemState(*pItem) )
            {
                // Ein Off-Tag-Attibut wurde gefunden, das genauso
                // exportiert wird, wie das aktuelle Item
                return true;
            }
        }
    }

    return false;
}

void HTMLEndPosLst::FixSplittedItem( HTMLStartEndPos *pPos, sal_Int32 nNewEnd,
                                        HTMLStartEndPositions::size_type nStartPos )
{
    // die End-Position entsprechend fixen
    pPos->SetEnd( nNewEnd );

    // das Item aus der End-Liste entfernen
    HTMLStartEndPositions::iterator it =
        std::find(aEndLst.begin(), aEndLst.end(), pPos );
    OSL_ENSURE(it != aEndLst.end(), "Item not found in End List!" );
    if( it != aEndLst.end() )
        aEndLst.erase( it );

    // es wird von nun an als letztes an der entsprechenden Position
    // beendet
    HTMLStartEndPositions::size_type nEndPos {0};
    while( nEndPos < aEndLst.size() && aEndLst[nEndPos]->GetEnd() <= nNewEnd )
        ++nEndPos;
    aEndLst.insert( aEndLst.begin() + nEndPos, pPos );

    // jetzt noch die spaeter gestarteten Attribute anpassen
    for( HTMLStartEndPositions::size_type i = nStartPos+1; i<aStartLst.size(); ++i )
    {
        HTMLStartEndPos *pTest = aStartLst[i];
        sal_Int32 nTestEnd = pTest->GetEnd();
        if( pTest->GetStart() >= nNewEnd )
        {
            // das Test-Attribut und alle folgenden beginnen, nachdem das
            // gesplittete Attribut endet
            break;
        }
        else if( nTestEnd > nNewEnd )
        {
            // das Test-Attribut beginnt, bevor das gesplittete Attribut
            // endet und endet danach, muss also auch gesplittet werden

            // das neue Ende setzen
            pTest->SetEnd( nNewEnd );

            // das Attribut aus der End-Liste entfernen
            it = std::find(aEndLst.begin(), aEndLst.end(), pTest );
            OSL_ENSURE(it != aEndLst.end(), "Item not found in End List!" );
            if( it != aEndLst.end() )
                aEndLst.erase( it );

            // es endet jetzt als erstes Attribut an der entsprechenden
            // Position. Diese Position in der Ende-Liste kennen wir schon.
            aEndLst.insert( aEndLst.begin() + nEndPos, pTest );

            // den "Rest" des Attributs neu einfuegen
            InsertItem( *pTest->GetItem(), nNewEnd, nTestEnd );
        }
    }
}

void HTMLEndPosLst::InsertItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                                          sal_Int32 nEnd )
{
    HTMLStartEndPositions::size_type i;
    for( i = 0; i < aEndLst.size(); i++ )
    {
        HTMLStartEndPos *pTest = aEndLst[i];
        sal_Int32 nTestEnd = pTest->GetEnd();
        if( nTestEnd <= nStart )
        {
            // das Test-Attribut endet, bevor das neue beginnt
            continue;
        }
        else if( nTestEnd < nEnd )
        {
            if( pTest->GetStart() < nStart )
            {
                // das Test-Attribut endet, bevor das neue endet. Das
                // neue Attribut muss deshalb aufgesplittet werden
                _InsertItem( new HTMLStartEndPos( rItem, nStart, nTestEnd ), i );
                nStart = nTestEnd;
            }
        }
        else
        {
            // das Test-Attribut (und alle folgenden) endet, bevor das neue
            // endet
            break;
        }
    }

    // ein Attribut muss noch eingefuegt werden
    _InsertItem( new HTMLStartEndPos( rItem, nStart, nEnd ), i );
}

void HTMLEndPosLst::SplitItem( const SfxPoolItem& rItem, sal_Int32 nStart,
                                                           sal_Int32 nEnd )
{
    sal_uInt16 nWhich = rItem.Which();

    // erstmal muessen wir die alten Items anhand der Startliste suchen
    // und die neuen Item-Bereiche festlegen

    for( HTMLStartEndPositions::size_type i=0; i<aStartLst.size(); ++i )
    {
        HTMLStartEndPos *pTest = aStartLst[i];
        sal_Int32 nTestStart = pTest->GetStart();
        sal_Int32 nTestEnd = pTest->GetEnd();

        if( nTestStart >= nEnd )
        {
            // dieses und alle nachfolgenden Attribute beginnen spaeter
            break;
        }
        else if( nTestEnd > nStart )
        {
            // das Test Attribut endet im zu loeschenenden Bereich
            const SfxPoolItem *pItem = pTest->GetItem();

            // nur entsprechende On-Tag Attribute muessen beruecksichtigt
            // werden
            if( pItem->Which() == nWhich &&
                HTML_ON_VALUE == GetHTMLItemState( *pItem ) )
            {
                bool bDelete = true;

                if( nTestStart < nStart )
                {
                    // der Start des neuen Attribut entspricht
                    // dem neuen Ende des Attribts
                    FixSplittedItem( pTest, nStart, i );
                    bDelete = false;
                }
                else
                {
                    // das Test-Item beginnt erst hinter dem neuen
                    // Ende des Attribts und kann deshalb komplett
                    // geloescht werden
                    aStartLst.erase( aStartLst.begin() + i );
                    i--;

                    HTMLStartEndPositions::iterator it =
                        std::find(aEndLst.begin(), aEndLst.end(), pTest );
                    OSL_ENSURE(it != aEndLst.end(), "Item not found in End List!" );
                    if( it != aEndLst.end() )
                        aEndLst.erase( it );
                }

                // ggf den zweiten Teil des gesplitteten Attribts einfuegen
                if( nTestEnd > nEnd )
                {
                    InsertItem( *pTest->GetItem(), nEnd, nTestEnd );
                }

                if( bDelete )
                    delete pTest;
            }
        }
    }
}

const SwHTMLFormatInfo *HTMLEndPosLst::GetFormatInfo( const SwFormat& rFormat,
                                                SwHTMLFormatInfos& rFormatInfos )
{
    SwHTMLFormatInfo *pFormatInfo;
    std::unique_ptr<SwHTMLFormatInfo> pTmpInfo(new SwHTMLFormatInfo(&rFormat));
    SwHTMLFormatInfos::iterator it = rFormatInfos.find( pTmpInfo );
    if (it != rFormatInfos.end())
    {
        pFormatInfo = it->get();
    }
    else
    {
        pFormatInfo = new SwHTMLFormatInfo( &rFormat, pDoc, pTemplate,
                                      bOutStyles );
        rFormatInfos.insert(std::unique_ptr<SwHTMLFormatInfo>(pFormatInfo));
        if ( rScriptTextStyles.count( rFormat.GetName() ) )
            pFormatInfo->bScriptDependent = true;
    }

    return pFormatInfo;
}

HTMLEndPosLst::HTMLEndPosLst( SwDoc *pD, SwDoc* pTempl,
                              const Color* pDfltCol, bool bStyles,
                              sal_uLong nMode, const OUString& rText,
                              std::set<OUString>& rStyles ):
    pDoc( pD ),
    pTemplate( pTempl ),
    pDfltColor( pDfltCol ),
    rScriptTextStyles( rStyles ),
    nHTMLMode( nMode ),
    bOutStyles( bStyles )
{
    sal_Int32 nEndPos = rText.getLength();
    sal_Int32 nPos = 0;
    while( nPos < nEndPos )
    {
        sal_uInt16 nScript = g_pBreakIt->GetBreakIter()->getScriptType( rText, nPos );
        nPos = g_pBreakIt->GetBreakIter()->endOfScript( rText, nPos, nScript );
        aScriptChgLst.push_back( nPos );
        aScriptLst.push_back( nScript );
    }
}

HTMLEndPosLst::~HTMLEndPosLst()
{
    OSL_ENSURE(aStartLst.empty(), "Start List not empty in destructor");
    OSL_ENSURE(aEndLst.empty(), "End List not empty in destructor");
}

void HTMLEndPosLst::InsertNoScript( const SfxPoolItem& rItem,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwHTMLFormatInfos& rFormatInfos, bool bParaAttrs )
{
    // kein Bereich ?? dann nicht aufnehmen, wird nie wirksam !!
    if( nStart != nEnd )
    {
        bool bSet = false, bSplit = false;
        switch( GetHTMLItemState(rItem) )
        {
        case HTML_ON_VALUE:
            // das Attribut wird ausgegeben, wenn es nicht sowieso
            // schon an ist
            if( !ExistsOnTagItem( rItem.Which(), nStart ) )
                bSet = true;
            break;

        case HTML_OFF_VALUE:
            // wenn das entsprechne Attribut an ist, wird es gesplittet,
            // Zusaetlich wird es aber als Style ausgegeben, wenn es nicht
            // am ganzen Absatz gesetzt ist, weil es dann ja schon mit dem
            // ABsatz-Tag ausgegeben wurde.
            if( ExistsOnTagItem( rItem.Which(), nStart ) )
                bSplit = true;
            bSet = bOutStyles && !bParaAttrs &&
                   !ExistsOffTagItem( rItem.Which(), nStart, nEnd );
            break;

        case HTML_REAL_VALUE:
            // das Attribut kann immer ausgegeben werden
            bSet = true;
            break;

        case HTML_STYLE_VALUE:
            // Das Attribut kann nur als CSS1 ausgegeben werden. Wenn
            // es am Absatz gesetzt ist, wurde es schon mit dem
            // Absatz-Tag ausgegeben. Einzige Ausnahme ist das
            // Zeichen-Hintergrund-Attribut. Es muss immer wie ein
            // Hint behandelt werden.
            bSet = bOutStyles &&
                   (!bParaAttrs
                  || rItem.Which()==RES_CHRATR_BACKGROUND
                  || rItem.Which()==RES_CHRATR_BOX
                  || rItem.Which()==RES_CHRATR_OVERLINE);
            break;

        case HTML_CHRFMT_VALUE:
            {
                OSL_ENSURE( RES_TXTATR_CHARFMT == rItem.Which(),
                        "Doch keine Zeichen-Vorlage" );
                const SwFormatCharFormat& rChrFormat = static_cast<const SwFormatCharFormat&>(rItem);
                const SwCharFormat* pFormat = rChrFormat.GetCharFormat();

                const SwHTMLFormatInfo *pFormatInfo = GetFormatInfo( *pFormat, rFormatInfos );
                if( !pFormatInfo->aToken.isEmpty() )
                {
                    // das Zeichenvorlagen-Tag muss vor den harten
                    // Attributen ausgegeben werden
                    InsertItem( rItem, nStart, nEnd );
                }
                if( pFormatInfo->pItemSet )
                {
                    Insert( *pFormatInfo->pItemSet, nStart, nEnd,
                            rFormatInfos, true, bParaAttrs );
                }
            }
            break;

        case HTML_AUTOFMT_VALUE:
            {
                const SwFormatAutoFormat& rAutoFormat = static_cast<const SwFormatAutoFormat&>(rItem);
                const std::shared_ptr<SfxItemSet> pSet = rAutoFormat.GetStyleHandle();
                if( pSet.get() )
                    Insert( *pSet.get(), nStart, nEnd, rFormatInfos, true, bParaAttrs );
            }
            break;

        case HTML_COLOR_VALUE:
            // Eine Vordergrund-Farbe als Absatz-Attribut wird nur
            // exportiert, wenn sie nicht der Default-Farbe entspricht.
            {
                OSL_ENSURE( RES_CHRATR_COLOR == rItem.Which(),
                        "Doch keine Vordergrund-Farbe" );
                Color aColor( static_cast<const SvxColorItem&>(rItem).GetValue() );
                if( COL_AUTO == aColor.GetColor() )
                    aColor.SetColor( COL_BLACK );
                bSet = !bParaAttrs || !pDfltColor ||
                       !pDfltColor->IsRGBEqual( aColor );
            }
            break;

        case HTML_DROPCAP_VALUE:
            {
                OSL_ENSURE( RES_PARATR_DROP == rItem.Which(),
                        "Doch kein Drop-Cap" );
                const SwFormatDrop& rDrop = static_cast<const SwFormatDrop&>(rItem);
                nEnd = nStart + rDrop.GetChars();
                if( !bOutStyles )
                {
                    // Zumindest die Attribute der Zeichenvorlage uebernehmen
                    const SwCharFormat *pCharFormat = rDrop.GetCharFormat();
                    if( pCharFormat )
                    {
                        Insert( pCharFormat->GetAttrSet(), nStart, nEnd,
                                rFormatInfos, true, bParaAttrs );
                    }
                }
                else
                {
                    bSet = true;
                }
            }
            break;
        default:
            ;
        }

        if( bSet )
            InsertItem( rItem, nStart, nEnd );
        if( bSplit )
            SplitItem( rItem, nStart, nEnd );
    }
}

void HTMLEndPosLst::Insert( const SfxPoolItem& rItem,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwHTMLFormatInfos& rFormatInfos, bool bParaAttrs )
{
    bool bDependsOnScript = false, bDependsOnAnyScript = false;
    sal_uInt16 nScript = i18n::ScriptType::LATIN;
    switch( rItem.Which() )
    {
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        bDependsOnScript = true;
        nScript = i18n::ScriptType::LATIN;
        break;

    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        bDependsOnScript = true;
        nScript = i18n::ScriptType::ASIAN;
        break;

    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        bDependsOnScript = true;
        nScript = i18n::ScriptType::COMPLEX;
        break;
    case RES_TXTATR_CHARFMT:
        {
            const SwFormatCharFormat& rChrFormat = static_cast<const SwFormatCharFormat&>(rItem);
            const SwCharFormat* pFormat = rChrFormat.GetCharFormat();
            const SwHTMLFormatInfo *pFormatInfo = GetFormatInfo( *pFormat, rFormatInfos );
            if( pFormatInfo->bScriptDependent )
            {
                bDependsOnScript = true;
                bDependsOnAnyScript = true;
            }
        }
        break;
    case RES_TXTATR_INETFMT:
        {
            if( GetFormatInfo( *pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                     RES_POOLCHR_INET_NORMAL), rFormatInfos )->bScriptDependent ||
                GetFormatInfo( *pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                     RES_POOLCHR_INET_VISIT), rFormatInfos )->bScriptDependent )
            {
                bDependsOnScript = true;
                bDependsOnAnyScript = true;
            }
        }
        break;
    }

    if( bDependsOnScript )
    {
        sal_Int32 nPos = nStart;
        for( size_t i=0; i < aScriptChgLst.size(); i++ )
        {
            sal_Int32 nChgPos = aScriptChgLst[i];
            if( nPos >= nChgPos )
            {
                // the hint starts behind or at the next script change,
                // so we may continue with this position.
                continue;
            }
            if( nEnd <= nChgPos )
            {
                // the (rest of) the hint ends before or at the next script
                // change, so we can insert it, but only if it belongs
                // to the current script.
                if( bDependsOnAnyScript || nScript == aScriptLst[i] )
                    InsertNoScript( rItem, nPos, nEnd, rFormatInfos,
                                    bParaAttrs );
                break;
            }

            // the hint starts before the next script change and ends behind
            // it, so we can insert a hint up to the next script change and
            // continue with the rest of the hint.
            if( bDependsOnAnyScript || nScript == aScriptLst[i] )
                InsertNoScript( rItem, nPos, nChgPos, rFormatInfos, bParaAttrs );
            nPos = nChgPos;
        }
    }
    else
    {
        InsertNoScript( rItem, nStart, nEnd, rFormatInfos, bParaAttrs );
    }
}

void HTMLEndPosLst::Insert( const SfxItemSet& rItemSet,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwHTMLFormatInfos& rFormatInfos,
                            bool bDeep, bool bParaAttrs )
{
    SfxWhichIter aIter( rItemSet );

    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        const SfxPoolItem *pItem;
        if( SfxItemState::SET == rItemSet.GetItemState( nWhich, bDeep, &pItem ) )
        {
            Insert( *pItem, nStart, nEnd, rFormatInfos, bParaAttrs );
        }

        nWhich = aIter.NextWhich();
    }
}

void HTMLEndPosLst::Insert( const SwDrawFrameFormat& rFormat, sal_Int32 nPos,
                            SwHTMLFormatInfos& rFormatInfos )
{
    const SdrObject* pTextObj = SwHTMLWriter::GetMarqueeTextObj( rFormat );

    if( pTextObj )
    {
        // die Edit-Engine-Attribute des Objekts als SW-Attribute holen
        // und als Hints einsortieren. Wegen der Menge der Hints werden
        // Styles hierbei nicht beruecksichtigt!
        const SfxItemSet& rFormatItemSet = rFormat.GetAttrSet();
        SfxItemSet aItemSet( *rFormatItemSet.GetPool(), RES_CHRATR_BEGIN,
                                                     RES_CHRATR_END );
        SwHTMLWriter::GetEEAttrsFromDrwObj( aItemSet, pTextObj, true );
        bool bOutStylesOld = bOutStyles;
        bOutStyles = false;
        Insert( aItemSet, nPos, nPos+1, rFormatInfos, false );
        bOutStyles = bOutStylesOld;
    }
}

sal_uInt16 HTMLEndPosLst::GetScriptAtPos( sal_Int32 nPos, sal_uInt16 nWeak )
{
    sal_uInt16 nRet = CSS1_OUTMODE_ANY_SCRIPT;

    size_t nScriptChgs = aScriptChgLst.size();
    size_t i=0;
    while( i < nScriptChgs && nPos >= aScriptChgLst[i] )
        i++;
    OSL_ENSURE( i < nScriptChgs, "script list is to short" );
    if( i < nScriptChgs )
    {
        if( i18n::ScriptType::WEAK == aScriptLst[i] )
            nRet = nWeak;
        else
            nRet = SwHTMLWriter::GetCSS1ScriptForScriptType( aScriptLst[i] );
    }

    return nRet;
}

void HTMLEndPosLst::OutStartAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                                      HTMLOutContext *pContext  )
{
    rHWrt.bTagOn = true;

    // Character border attribute must be the first which is written out
    // because of border merge.
    HTMLStartEndPositions::size_type nCharBoxIndex = 0;
    while( nCharBoxIndex < aStartLst.size() &&
           aStartLst[nCharBoxIndex]->GetItem()->Which() != RES_CHRATR_BOX )
    {
        ++nCharBoxIndex;
    }

    // die Attribute in der Start-Liste sind aufsteigend sortiert
    for( HTMLStartEndPositions::size_type i=0; i< aStartLst.size(); ++i )
    {
        HTMLStartEndPos *pPos = 0;
        if( nCharBoxIndex < aStartLst.size() )
        {
            if( i == 0 )
                pPos = aStartLst[nCharBoxIndex];
            else if( i == nCharBoxIndex )
                pPos = aStartLst[0];
            else
                pPos = aStartLst[i];
        }
        else
            pPos = aStartLst[i];

        sal_Int32 nStart = pPos->GetStart();
        if( nStart > nPos )
        {
            // dieses und alle folgenden Attribute werden erst noch geoeffnet
            break;
        }
        else if( nStart == nPos )
        {
            // das Attribut ausgeben
            sal_uInt16 nCSS1Script = rHWrt.nCSS1Script;
            sal_uInt16 nWhich = pPos->GetItem()->Which();
            if( RES_TXTATR_CHARFMT == nWhich ||
                RES_TXTATR_INETFMT == nWhich ||
                 RES_PARATR_DROP == nWhich )
            {
                rHWrt.nCSS1Script = GetScriptAtPos( nPos, nCSS1Script );
            }
            if( pContext )
            {
                HTMLOutFuncs::FlushToAscii( rHWrt.Strm(), *pContext );
                pContext = 0; // one time ony
            }
            Out( aHTMLAttrFnTab, *pPos->GetItem(), rHWrt );
            rHWrt.nCSS1Script = nCSS1Script;
        }
    }
}

void HTMLEndPosLst::OutEndAttrs( SwHTMLWriter& rHWrt, sal_Int32 nPos,
                                     HTMLOutContext *pContext )
{
    rHWrt.bTagOn = false;

    // die Attribute in der End-Liste sind aufsteigend sortiert
    HTMLStartEndPositions::size_type i {0};
    while( i < aEndLst.size() )
    {
        HTMLStartEndPos *pPos = aEndLst[i];
        sal_Int32 nEnd = pPos->GetEnd();

        if( SAL_MAX_INT32 == nPos || nEnd == nPos )
        {
            if( pContext )
            {
                HTMLOutFuncs::FlushToAscii( rHWrt.Strm(), *pContext );
                pContext = 0; // one time ony
            }
            // Skip closing span if next character span has the same border (border merge)
            bool bSkipOut = false;
            if( pPos->GetItem()->Which() == RES_CHRATR_BOX )
            {
                HTMLStartEndPositions::iterator it =
                    std::find(aStartLst.begin(), aStartLst.end(), pPos );
                OSL_ENSURE(it != aStartLst.end(), "Item not found in Start List!" );
                if (it != aStartLst.end())
                    ++it;
                while(it != aStartLst.end() )
                {
                    HTMLStartEndPos *pEndPos = *it;
                    if( pEndPos->GetItem()->Which() == RES_CHRATR_BOX &&
                        *static_cast<const SvxBoxItem*>(pEndPos->GetItem()) ==
                        *static_cast<const SvxBoxItem*>(pPos->GetItem()) )
                    {
                        pEndPos->SetStart(pPos->GetStart());
                        bSkipOut = true;
                        break;
                    }
                    ++it;
                }
            }
            if( !bSkipOut )
            {
                Out( aHTMLAttrFnTab, *pPos->GetItem(), rHWrt );
            }
            _RemoveItem( i );
        }
        else if( nEnd > nPos )
        {
            // dieses und alle folgenden Attribute werden erst spaeter beendet
            break;
        }
        else
        {
            // Das Attribut wird vor der aktuellen Position beendet. Das
            // darf nicht sein, aber wie koennen trotzdem damit umgehen
            OSL_ENSURE( nEnd >= nPos,
                    "Das Attribut sollte schon laengst beendet sein" );
            i++;
        }
    }
}

/* Ausgabe der Nodes */
Writer& OutHTML_SwTextNode( Writer& rWrt, const SwContentNode& rNode )
{
    const SwTextNode * pNd = &static_cast<const SwTextNode&>(rNode);
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    const OUString& rStr = pNd->GetText();
    sal_Int32 nEnd = rStr.getLength();

    // Besonderheit: leere Node und HR-Vorlage (horizontaler Strich)
    //              nur ein <HR> ausgeben
    sal_uInt16 nPoolId = pNd->GetAnyFormatColl().GetPoolFormatId();

    // Handle horizontal rule <hr>
    if (!nEnd &&
        (RES_POOLCOLL_HTML_HR==nPoolId || pNd->GetAnyFormatColl().GetName() == OOO_STRING_SVTOOLS_HTML_horzrule))
    {
        // dann die absatz-gebundenen Grafiken/OLE-Objekte im Absatz
        // MIB 8.7.97: Ein <PRE> spannen wir um die Linie auf. Dann stimmen
        // zwar die Abstaende nicht, aber sonst bekommen wir einen leeren
        // Absatz hinter dem <HR> und das ist noch unschoener.
        rHTMLWrt.ChangeParaToken( 0 );

        // Alle an dem Node verankerten Rahmen ausgeben
        rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_ANY );

        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine(); // Absatz-Tag in eine neue Zeile

        rHTMLWrt.bLFPossible = true;

        HtmlWriter aHtml(rWrt.Strm());
        aHtml.start(OOO_STRING_SVTOOLS_HTML_horzrule);

        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();
        if( !pItemSet )
        {
            aHtml.endAttribute();
            return rHTMLWrt;
        }
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pItemSet->GetItemState( RES_LR_SPACE, false, &pItem ))
        {
            sal_Int32 nLeft = static_cast<const SvxLRSpaceItem*>(pItem)->GetLeft();
            sal_Int32 nRight = static_cast<const SvxLRSpaceItem*>(pItem)->GetRight();
            if( nLeft || nRight )
            {
                const SwFrameFormat& rPgFormat =
                    rHTMLWrt.pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool
                    ( RES_POOLPAGE_HTML, false )->GetMaster();
                const SwFormatFrmSize& rSz   = rPgFormat.GetFrmSize();
                const SvxLRSpaceItem& rLR = rPgFormat.GetLRSpace();
                const SwFormatCol& rCol = rPgFormat.GetCol();

                long nPageWidth = rSz.GetWidth() - rLR.GetLeft() - rLR.GetRight();

                if( 1 < rCol.GetNumCols() )
                    nPageWidth /= rCol.GetNumCols();

                const SwTableNode* pTableNd = pNd->FindTableNode();
                if( pTableNd )
                {
                    const SwTableBox* pBox = pTableNd->GetTable().GetTableBox(
                                    pNd->StartOfSectionIndex() );
                    if( pBox )
                        nPageWidth = pBox->GetFrameFormat()->GetFrmSize().GetWidth();
                }

                OString sWidth = OString::number(SwHTMLWriter::ToPixel(nPageWidth - nLeft - nRight, false));
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_width, sWidth);

                if( !nLeft )
                    aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_align, OOO_STRING_SVTOOLS_HTML_AL_left);
                else if( !nRight )
                    aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_align, OOO_STRING_SVTOOLS_HTML_AL_right);
                else
                    aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_align, OOO_STRING_SVTOOLS_HTML_AL_center);
            }
        }

        if( SfxItemState::SET == pItemSet->GetItemState( RES_BOX, false, &pItem ))
        {
            const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
            const editeng::SvxBorderLine* pBorderLine = pBoxItem->GetBottom();
            if( pBorderLine )
            {
                sal_uInt16 nWidth = pBorderLine->GetScaledWidth();
                OString sWidth = OString::number(SwHTMLWriter::ToPixel(nWidth, false));
                aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_size, sWidth);

                const Color& rBorderColor = pBorderLine->GetColor();
                if( !rBorderColor.IsRGBEqual( Color(COL_GRAY) ) )
                {
                    HtmlWriterHelper::applyColor(aHtml, OOO_STRING_SVTOOLS_HTML_O_color, rBorderColor);
                }

                if( !pBorderLine->GetInWidth() )
                {
                    aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_noshade, OOO_STRING_SVTOOLS_HTML_O_noshade);
                }
            }
        }
        aHtml.end();
        return rHTMLWrt;
    }

    // Die leeren Nodes mit 2pt Font-Hoehe und der Stand-Vorlage, die
    // vor Tabellen und Bereichen eingefuegt werden, nicht exportieren,
    // Bookmarks oder absatzgebundene Grafiken aber schon.
    // MIB 21.7.97: Ausserdem auch keine leeren Tabellen-Zellen exportieren.
    if( !nEnd && (nPoolId == RES_POOLCOLL_STANDARD ||
                  nPoolId == RES_POOLCOLL_TABLE ||
                  nPoolId == RES_POOLCOLL_TABLE_HDLN) )
    {
        // Der aktuelle Node ist leer und enthaelt Standard-Vorlage ...
        const SfxPoolItem* pItem;
        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();
        if( pItemSet && pItemSet->Count() &&
            SfxItemState::SET == pItemSet->GetItemState( RES_CHRATR_FONTSIZE, false, &pItem ) &&
            40 == static_cast<const SvxFontHeightItem *>(pItem)->GetHeight() )
        {
            // ... ausserdem ist die 2pt Schrift eingestellt ...
            sal_uLong nNdPos = rWrt.pCurPam->GetPoint()->nNode.GetIndex();
            const SwNode *pNextNd = rWrt.pDoc->GetNodes()[nNdPos+1];
            const SwNode *pPrevNd = rWrt.pDoc->GetNodes()[nNdPos-1];
            bool bStdColl = nPoolId == RES_POOLCOLL_STANDARD;
            if( ( bStdColl && (pNextNd->IsTableNode() || pNextNd->IsSectionNode()) ) ||
                ( !bStdColl &&
                   pNextNd->IsEndNode() &&
                   pPrevNd->IsStartNode() &&
                   SwTableBoxStartNode == pPrevNd->GetStartNode()->GetStartNodeType() ) )
            {
                // ... und er steht vor einer Tabelle ohne einem Bereich
                rHTMLWrt.OutBookmarks();
                rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken;

                // Alle an dem Node verankerten Rahmen ausgeben
                rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_ANY );
                rHTMLWrt.bLFPossible = false;

                return rWrt;
            }
        }
    }

    // PagePreaks uns PagDescs abfangen
    bool bPageBreakBehind = false;
    if( rHTMLWrt.bCfgFormFeed &&
        !(rHTMLWrt.bOutTable || rHTMLWrt.bOutFlyFrame) &&
        rHTMLWrt.pStartNdIdx->GetIndex() != rHTMLWrt.pCurPam->GetPoint()->nNode.GetIndex() )
    {
        bool bPageBreakBefore = false;
        const SfxPoolItem* pItem;
        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();

        if( pItemSet )
        {
            if( SfxItemState::SET == pItemSet->GetItemState( RES_PAGEDESC, true, &pItem ) &&
                static_cast<const SwFormatPageDesc *>(pItem)->GetPageDesc() )
            {
                bPageBreakBefore = true;
            }
            else if( SfxItemState::SET == pItemSet->GetItemState( RES_BREAK, true, &pItem ) )
            {
                switch( static_cast<const SvxFormatBreakItem *>(pItem)->GetBreak() )
                {
                case SVX_BREAK_PAGE_BEFORE:
                    bPageBreakBefore = true;
                    break;
                case SVX_BREAK_PAGE_AFTER:
                    bPageBreakBehind = true;
                    break;
                case SVX_BREAK_PAGE_BOTH:
                    bPageBreakBefore = true;
                    bPageBreakBehind = true;
                    break;
                default:
                    break;
                }
            }
        }

        if( bPageBreakBefore )
            rWrt.Strm().WriteChar( '\f' );
    }

    // eventuell eine Form oeffnen
    rHTMLWrt.OutForm();

    // An dem Node "verankerte" Seitenegebunde Rahmen ausgeben
    bool bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_PREFIX );

    // An dem Node verankerte Rahmen ausgeben, die vor dem
    // Absatz-Tag geschrieben werden sollen.
    if( bFlysLeft )
    {
        bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_BEFORE );
    }

    if( rHTMLWrt.pCurPam->GetPoint()->nNode == rHTMLWrt.pCurPam->GetMark()->nNode )
    {
        nEnd = rHTMLWrt.pCurPam->GetMark()->nContent.GetIndex();
    }

    // gibt es harte Attribute, die als Optionen geschrieben werden muessen?
    rHTMLWrt.bTagOn = true;

    // jetzt das Tag des Absatzes ausgeben
    const SwFormat& rFormat = pNd->GetAnyFormatColl();
    SwHTMLTextCollOutputInfo aFormatInfo;
    bool bOldLFPossible = rHTMLWrt.bLFPossible;
    OutHTML_SwFormat( rWrt, rFormat, pNd->GetpSwAttrSet(), aFormatInfo );

    // Wenn vor dem Absatz-Tag keine neue Zeile aufgemacht wurde, dann
    // tun wir das jetzt
    rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken;
    if( !bOldLFPossible && rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    // dann die Bookmarks (inkl. End-Tag)
    rHTMLWrt.bOutOpts = false;
    rHTMLWrt.OutBookmarks();

    // jetzt ist noch mal eine gute Gelegenheit fuer ein LF, sofern es noch
    // erlaubt ist
    if( rHTMLWrt.bLFPossible &&
        rHTMLWrt.GetLineLen() >= rHTMLWrt.nWhishLineLen )
    {
        rHTMLWrt.OutNewLine();
    }
    rHTMLWrt.bLFPossible = false;

    // Text, der aus einer Outline-Numerierung kommt ermitteln
    sal_Int32 nOffset = 0;
    OUString aOutlineText;
    OUString aFullText;

    // export numbering string as plain text only for the outline numbering,
    // because the outline numbering isn't exported as a numbering - see <SwHTMLNumRuleInfo::Set(..)>
    if ( pNd->IsOutline() &&
         pNd->GetNumRule() == pNd->GetDoc()->GetOutlineNumRule() )
    {
        aOutlineText = pNd->GetNumString();
        nOffset = nOffset + aOutlineText.getLength();
        aFullText = aOutlineText;
    }
    OUString aFootEndNoteSym;
    if( rHTMLWrt.pFormatFootnote )
    {
        aFootEndNoteSym = rHTMLWrt.GetFootEndNoteSym( *rHTMLWrt.pFormatFootnote );
        nOffset = nOffset + aFootEndNoteSym.getLength();
        aFullText += aFootEndNoteSym;
    }

    // Table of Contents or other paragraph with dot leaders?
    sal_Int32 nIndexTab = rHTMLWrt.indexOfDotLeaders( nPoolId, rStr );
    if (nIndexTab > -1)
        // skip part after the tabulator (page number)
        nEnd = nIndexTab;

    // gibt es harte Attribute, die als Tags geschrieben werden muessen?
    aFullText += rStr;
    HTMLEndPosLst aEndPosLst( rWrt.pDoc, rHTMLWrt.pTemplate,
                              rHTMLWrt.pDfltColor, rHTMLWrt.bCfgOutStyles,
                              rHTMLWrt.GetHTMLMode(), aFullText,
                                 rHTMLWrt.aScriptTextStyles );
    if( aFormatInfo.pItemSet )
    {
        aEndPosLst.Insert( *aFormatInfo.pItemSet, 0, nEnd + nOffset,
                           rHTMLWrt.m_CharFormatInfos, false, true );
    }

    if( !aOutlineText.isEmpty() || rHTMLWrt.pFormatFootnote )
    {
        // Absatz-Attribute ausgeben, damit der Text die Attribute des
        // Absatzes bekommt.
        aEndPosLst.OutStartAttrs( rHTMLWrt, 0 );

        // Theoretisch muesste man hier die Zeichen-Vorlage der Numerierung
        // beachten. Da man die ueber die UI nicht setzen kann, ignorieren
        // wir sie erstmal.

        if( !aOutlineText.isEmpty() )
            HTMLOutFuncs::Out_String( rWrt.Strm(), aOutlineText,
                                         rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters);

        if( rHTMLWrt.pFormatFootnote )
        {
            rHTMLWrt.OutFootEndNoteSym( *rHTMLWrt.pFormatFootnote, aFootEndNoteSym,
                                        aEndPosLst.GetScriptAtPos( aOutlineText.getLength(), rHTMLWrt.nCSS1Script ) );
            rHTMLWrt.pFormatFootnote = 0;
        }
    }

    // erstmal den Start berichtigen. D.h. wird nur ein Teil vom Satz
    // ausgegeben, so muessen auch da die Attribute stimmen!!
    rHTMLWrt.bTextAttr = true;

    size_t nAttrPos = 0;
    sal_Int32 nStrPos = rHTMLWrt.pCurPam->GetPoint()->nContent.GetIndex();
    const SwTextAttr * pHt = 0;
    const size_t nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > ( pHt = pNd->GetSwpHints().Get(0) )->GetStart() )
    {
        // Ok, es gibt vorher Attribute, die ausgegeben werden muessen
        do {
            aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );

            nAttrPos++;
            if( pHt->Which() == RES_TXTATR_FIELD
                || pHt->Which() == RES_TXTATR_ANNOTATION )
                continue;

            if ( pHt->End() && !pHt->HasDummyChar() )
            {
                const sal_Int32 nHtEnd = *pHt->End(),
                       nHtStt = pHt->GetStart();
                if( !rHTMLWrt.bWriteAll && nHtEnd <= nStrPos )
                    continue;

                // leere Hints am Anfang nicht beachten, oder ??
                if( nHtEnd == nHtStt )
                    continue;

                // Attribut in die Liste aufnehemen
                if( rHTMLWrt.bWriteAll )
                    aEndPosLst.Insert( pHt->GetAttr(), nHtStt + nOffset,
                                       nHtEnd + nOffset,
                                       rHTMLWrt.m_CharFormatInfos );
                else
                {
                    sal_Int32 nTmpStt = nHtStt < nStrPos ? nStrPos : nHtStt;
                    sal_Int32 nTmpEnd = nHtEnd < nEnd ? nHtEnd : nEnd;
                    aEndPosLst.Insert( pHt->GetAttr(), nTmpStt + nOffset,
                                       nTmpEnd + nOffset,
                                       rHTMLWrt.m_CharFormatInfos );
                }
                continue;
                // aber nicht ausgeben, das erfolgt spaeter !!
            }

        } while( nAttrPos < nCntAttr && nStrPos >
            ( pHt = pNd->GetSwpHints().Get( nAttrPos ) )->GetStart() );

        // dann gebe mal alle gesammelten Attribute von der String-Pos aus
        aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );
        aEndPosLst.OutStartAttrs( rHTMLWrt, nStrPos + nOffset );
    }

    bool bWriteBreak = (HTML_PREFORMTXT_ON != rHTMLWrt.nLastParaToken);
    if( bWriteBreak && pNd->GetNumRule()  )
        bWriteBreak = false;

    {
        HTMLOutContext aContext( rHTMLWrt.eDestEnc );

        sal_Int32 nPreSplitPos = 0;
        for( ; nStrPos < nEnd; nStrPos++ )
        {
            // Die an der aktuellen Position verankerten Rahmen ausgeben
            if( bFlysLeft )
            {
                aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset, &aContext );
                bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                                nStrPos, HTML_POS_INSIDE,
                                                &aContext );
            }

            bool bOutChar = true;
            const SwTextAttr * pTextHt = 0;
            if( nAttrPos < nCntAttr && pHt->GetStart() == nStrPos
                && nStrPos != nEnd )
            {
                do {
                    if ( pHt->End() && !pHt->HasDummyChar() )
                    {
                        if( *pHt->End() != nStrPos )
                        {
                            // Hints mit Ende einsortieren, wenn sie keinen
                            // leeren Bereich aufspannen (Hints, die keinen
                            // Bereich aufspannen werden ignoriert
                            aEndPosLst.Insert( pHt->GetAttr(), nStrPos + nOffset,
                                               *pHt->End() + nOffset,
                                               rHTMLWrt.m_CharFormatInfos );
                        }
                    }
                    else
                    {
                        // Hints ohne-Ende werden als letztes ausgebeben
                        OSL_ENSURE( !pTextHt, "Wieso gibt es da schon ein Attribut ohne Ende?" );
                        if( rHTMLWrt.nTextAttrsToIgnore>0 )
                        {
                            rHTMLWrt.nTextAttrsToIgnore--;
                        }
                        else
                        {
                            pTextHt = pHt;
                            sal_uInt16 nFieldWhich;
                            if( RES_TXTATR_FIELD != pHt->Which()
                                || ( RES_POSTITFLD != (nFieldWhich = static_cast<const SwFormatField&>(pHt->GetAttr()).GetField()->Which())
                                     && RES_SCRIPTFLD != nFieldWhich ) )
                            {
                                bWriteBreak = false;
                            }
                        }
                        bOutChar = false;       // keine 255 ausgeben
                    }
                } while( ++nAttrPos < nCntAttr && nStrPos ==
                    ( pHt = pNd->GetSwpHints().Get( nAttrPos ) )->GetStart() );
            }

            // Manche Draw-Formate koennen auch noch Attribute mitbringen
            if( pTextHt && RES_TXTATR_FLYCNT == pTextHt->Which() )
            {
                const SwFrameFormat* pFrameFormat =
                    static_cast<const SwFormatFlyCnt &>(pTextHt->GetAttr()).GetFrameFormat();

                if( RES_DRAWFRMFMT == pFrameFormat->Which() )
                    aEndPosLst.Insert( *static_cast<const SwDrawFrameFormat *>(pFrameFormat),
                                        nStrPos + nOffset,
                                        rHTMLWrt.m_CharFormatInfos );
            }

            aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset, &aContext );
            aEndPosLst.OutStartAttrs( rHTMLWrt, nStrPos + nOffset, &aContext );

            if( pTextHt )
            {
                rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken && nStrPos > 0 &&
                                       rStr[nStrPos-1] == ' ';
                sal_uInt16 nCSS1Script = rHTMLWrt.nCSS1Script;
                rHTMLWrt.nCSS1Script = aEndPosLst.GetScriptAtPos(
                                                nStrPos + nOffset, nCSS1Script );
                HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
                Out( aHTMLAttrFnTab, pTextHt->GetAttr(), rHTMLWrt );
                rHTMLWrt.nCSS1Script = nCSS1Script;
                rHTMLWrt.bLFPossible = false;
            }

            if( bOutChar )
            {
                // #i120442#: get the UTF-32 codepoint by converting an eventual UTF-16 unicode surrogate pair
                sal_uInt64 c = rStr[nStrPos];
                if( nStrPos < nEnd - 1 )
                {
                    const sal_Unicode d = rStr[nStrPos + 1];
                    if( (c >= 0xd800 && c <= 0xdbff) && (d >= 0xdc00 && d <= 0xdfff) )
                    {
                        sal_uInt64 templow = d&0x03ff;
                        sal_uInt64 temphi = ((c&0x03ff) + 0x0040)<<10;
                        c = temphi|templow;
                        nStrPos++;
                    }
                }

                // try to split a line after about 255 characters
                // at a space character unless in a PRE-context
                if( ' '==c && !rHTMLWrt.nLastParaToken  )
                {
                    sal_Int32 nLineLen;
                    if( rHTMLWrt.nLastParaToken )
                        nLineLen = nStrPos - nPreSplitPos;
                    else
                        nLineLen = rHTMLWrt.GetLineLen();

                    sal_Int32 nWordLen = rStr.indexOf( ' ', nStrPos+1 );
                    if( nWordLen == -1 )
                        nWordLen = nEnd;
                    nWordLen -= nStrPos;

                    if( nLineLen >= rHTMLWrt.nWhishLineLen ||
                        (nLineLen+nWordLen) >= rHTMLWrt.nWhishLineLen )
                    {
                        HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
                        rHTMLWrt.OutNewLine();
                        bOutChar = false;
                        if( rHTMLWrt.nLastParaToken )
                            nPreSplitPos = nStrPos+1;
                    }
                }

                if( bOutChar )
                {
                    if( 0x0a == c )
                    {
                        HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
                        HtmlWriter aHtml(rWrt.Strm());
                        aHtml.single(OOO_STRING_SVTOOLS_HTML_linebreak);
                    }
                    // #i120442#: if c is outside the unicode base plane output it as "&#******;"
                    else if( c > 0xffff)
                    {
                        OString sOut("&#");
                        sOut += OString::number( (sal_uInt64)c );
                        sOut += ";";
                        rWrt.Strm().WriteCharPtr( sOut.getStr() );
                    }
                    else if (c == CH_TXT_ATR_FORMELEMENT)
                    {
                        // Placeholder for a single-point fieldmark.

                        SwPosition aMarkPos = *rWrt.pCurPam->GetPoint();
                        aMarkPos.nContent += nStrPos - aMarkPos.nContent.GetIndex();
                        rHTMLWrt.OutPointFieldmarks(aMarkPos);
                    }
                    else
                        HTMLOutFuncs::Out_Char( rWrt.Strm(), (sal_Unicode)c, aContext, &rHTMLWrt.aNonConvertableCharacters );

                    // if a paragraph's last character is a hard line break
                    // then we need to add an extra <br>
                    // because browsers like Mozilla wouldn't add a line for the next paragraph
                    bWriteBreak = (0x0a == c) &&
                                  (HTML_PREFORMTXT_ON != rHTMLWrt.nLastParaToken);
                }
            }
        }
        HTMLOutFuncs::FlushToAscii( rWrt.Strm(), aContext );
    }

    aEndPosLst.OutEndAttrs( rHTMLWrt, SAL_MAX_INT32 );

    // Die an der letzten Position verankerten Rahmen ausgeben
    if( bFlysLeft )
        bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                       nEnd, HTML_POS_INSIDE );
    OSL_ENSURE( !bFlysLeft, "Es wurden nicht alle Rahmen gespeichert!" );

    rHTMLWrt.bTextAttr = false;

    if( bWriteBreak )
    {
        bool bEndOfCell = rHTMLWrt.bOutTable &&
                         rWrt.pCurPam->GetPoint()->nNode.GetIndex() ==
                         rWrt.pCurPam->GetMark()->nNode.GetIndex();

        if( bEndOfCell && !nEnd &&
            rHTMLWrt.IsHTMLMode(HTMLMODE_NBSP_IN_TABLES) )
        {
            // Wenn der letzte Absatz einer Tabellezelle leer ist und
            // wir fuer den MS-IE exportieren, schreiben wir statt eines
            // <BR> ein &nbsp;
            rWrt.Strm().WriteChar( '&' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_S_nbsp ).WriteChar( ';' );
        }
        else
        {
            HtmlWriter aHtml(rHTMLWrt.Strm());
            aHtml.single(OOO_STRING_SVTOOLS_HTML_linebreak);
            const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>( pNd->GetSwAttrSet().Get(RES_UL_SPACE) );
            if (rULSpace.GetLower() > 0 &&
                !bEndOfCell &&
                !rHTMLWrt.IsHTMLMode(HTMLMODE_NO_BR_AT_PAREND) )
            {
                aHtml.single(OOO_STRING_SVTOOLS_HTML_linebreak);
            }
            rHTMLWrt.bLFPossible = true;
        }
    }

    if( rHTMLWrt.bClearLeft || rHTMLWrt.bClearRight )
    {
        const sal_Char* pString;
        if( rHTMLWrt.bClearLeft )
        {
            if( rHTMLWrt.bClearRight )
                pString = OOO_STRING_SVTOOLS_HTML_AL_all;
            else
                pString = OOO_STRING_SVTOOLS_HTML_AL_left;
        }
        else
        {
            pString = OOO_STRING_SVTOOLS_HTML_AL_right;
        }

        HtmlWriter aHtml(rHTMLWrt.Strm());
        aHtml.start(OOO_STRING_SVTOOLS_HTML_linebreak);
        aHtml.attribute(OOO_STRING_SVTOOLS_HTML_O_clear, pString);
        aHtml.end();

        rHTMLWrt.bClearLeft = false;
        rHTMLWrt.bClearRight = false;

        rHTMLWrt.bLFPossible = true;
    }

    // wenn ein LF nicht schon erlaubt ist wird es erlaubt, wenn der
    // Absatz mit einem ' ' endet
    if( !rHTMLWrt.bLFPossible && !rHTMLWrt.nLastParaToken &&
        nEnd > 0 && ' ' == rStr[nEnd-1] )
        rHTMLWrt.bLFPossible = true;

    // dot leaders: print the skipped page number in a different span element
    if (nIndexTab > -1) {
        OString sOut = OUStringToOString(rStr.copy(nIndexTab + 1), RTL_TEXTENCODING_ASCII_US);
        rWrt.Strm().WriteOString( "</span><span>" + sOut + "</span>" );
    }

    rHTMLWrt.bTagOn = false;
    OutHTML_SwFormatOff( rWrt, aFormatInfo );

    // eventuell eine Form schliessen
    rHTMLWrt.OutForm( false );

    if( bPageBreakBehind )
        rWrt.Strm().WriteChar( '\f' );

    return rHTMLWrt;
}

sal_uInt32 SwHTMLWriter::ToPixel( sal_uInt32 nVal, const bool bVert )
{
    if( Application::GetDefaultDevice() && nVal )
    {
        Size aSz( bVert ? 0 : nVal, bVert ? nVal : 0 );
        aSz = Application::GetDefaultDevice()->LogicToPixel(aSz, MapMode( MAP_TWIP ));
        nVal = bVert ? aSz.Height() : aSz.Width();
        if( !nVal )     // wo ein Twip ist sollte auch ein Pixel sein
            nVal = 1;
    }
    return nVal;
}

static Writer& OutHTML_CSS1Attr( Writer& rWrt, const SfxPoolItem& rHt )
{
    // wenn gerade Hints geschrieben werden versuchen wir den Hint als
    // CSS1-Attribut zu schreiben

    if( static_cast<SwHTMLWriter&>(rWrt).bCfgOutStyles && static_cast<SwHTMLWriter&>(rWrt).bTextAttr )
        OutCSS1_HintSpanTag( rWrt, rHt );

    return rWrt;
}

/* File CHRATR.HXX: */

static Writer& OutHTML_SvxColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( !rHTMLWrt.bTextAttr && rHTMLWrt.bCfgOutStyles && rHTMLWrt.bCfgPreferStyles )
    {
        // Font-Farbe nicht als Tag schreiben, wenn Styles normalen Tags
        // vorgezogen werden
        return rWrt;
    }

    if( rHTMLWrt.bTagOn )
    {
        Color aColor( static_cast<const SvxColorItem&>(rHt).GetValue() );
        if( COL_AUTO == aColor.GetColor() )
            aColor.SetColor( COL_BLACK );

        OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_font) + " " +
            OString(OOO_STRING_SVTOOLS_HTML_O_color) + "=";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_Color( rWrt.Strm(), aColor, rHTMLWrt.eDestEnc ).WriteChar( '>' );
    }
    else
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_font, false );

    return rWrt;
}

static Writer& OutHTML_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontItalic nPosture = static_cast<const SvxPostureItem&>(rHt).GetPosture();
    if( ITALIC_NORMAL == nPosture )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_italic, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SvxFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        OUString aNames;
        SwHTMLWriter::PrepareFontList( static_cast<const SvxFontItem&>(rHt), aNames, 0,
                           rHTMLWrt.IsHTMLMode(HTMLMODE_FONT_GENERIC) );
        OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_font) + " " +
            OString(OOO_STRING_SVTOOLS_HTML_O_face) + "=\"";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( rWrt.Strm(), aNames, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters )
           .WriteCharPtr( "\">" );
    }
    else
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_font, false );

    return rWrt;
}

static Writer& OutHTML_SvxFontHeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_font);

        sal_uInt32 nHeight = static_cast<const SvxFontHeightItem&>(rHt).GetHeight();
        sal_uInt16 nSize = rHTMLWrt.GetHTMLFontSize( nHeight );
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_size) + "=\"" +
            OString::number(static_cast<sal_Int32>(nSize)) + "\"";
        rWrt.Strm().WriteOString( sOut );

        if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
        {
            // always export font size as CSS option, too
            OutCSS1_HintStyleOpt( rWrt, rHt );
        }
        rWrt.Strm().WriteChar( '>' );
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_font, false );
    }

    return rWrt;
}

static Writer& OutHTML_SvxLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    LanguageType eLang = static_cast<const SvxLanguageItem &>(rHt).GetLanguage();
    if( LANGUAGE_DONTKNOW == eLang )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_span);
        rWrt.Strm().WriteOString( sOut );
        rHTMLWrt.OutLanguage( static_cast<const SvxLanguageItem &>(rHt).GetLanguage() );
        rWrt.Strm().WriteChar( '>' );
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_span, false );
    }

    return rWrt;
}
static Writer& OutHTML_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontWeight nBold = static_cast<const SvxWeightItem&>(rHt).GetWeight();
    if( WEIGHT_BOLD == nBold )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_bold, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    // Wegen Netscape schrieben wir hier STRIKE und nicht S raus!
    const FontStrikeout nStrike = static_cast<const SvxCrossedOutItem&>(rHt).GetStrikeout();
    if( STRIKEOUT_NONE != nStrike )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_strike, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SvxEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SvxEscapement eEscape =
        (const SvxEscapement)static_cast<const SvxEscapementItem&>(rHt).GetEnumValue();
    const sal_Char *pStr = 0;
    switch( eEscape )
    {
    case SVX_ESCAPEMENT_SUPERSCRIPT: pStr = OOO_STRING_SVTOOLS_HTML_superscript; break;
    case SVX_ESCAPEMENT_SUBSCRIPT: pStr = OOO_STRING_SVTOOLS_HTML_subscript; break;
    default:
        ;
    }

    if( pStr )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontUnderline eUnder = static_cast<const SvxUnderlineItem&>(rHt).GetLineStyle();
    if( UNDERLINE_NONE != eUnder )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_underline, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SwFlyCnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    const SwFormatFlyCnt& rFlyCnt = static_cast<const SwFormatFlyCnt&>(rHt);

    const SwFrameFormat& rFormat = *rFlyCnt.GetFrameFormat();
    const SdrObject *pSdrObj = 0;

    SwHTMLFrmType eType =
        (SwHTMLFrmType)rHTMLWrt.GuessFrmType( rFormat, pSdrObj );
    sal_uInt8 nMode = aHTMLOutFrmAsCharTable[eType][rHTMLWrt.nExportMode];
    rHTMLWrt.OutFrameFormat( nMode, rFormat, pSdrObj );
    return rWrt;
}

// Das ist jetzt unser Blink-Item. Blinkend wird eingeschaltet, indem man
// das Item auf true setzt!
static Writer& OutHTML_SwBlink( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( static_cast<const SvxBlinkItem&>(rHt).GetValue() )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_blink, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTextAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

Writer& OutHTML_INetFormat( Writer& rWrt, const SwFormatINetFormat& rINetFormat, bool bOn )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    OUString aURL( rINetFormat.GetValue() );
    const SvxMacroTableDtor *pMacTable = rINetFormat.GetMacroTable();
    bool bEvents = pMacTable != 0 && !pMacTable->empty();

    // Anything to output at all?
    if( aURL.isEmpty() && !bEvents && rINetFormat.GetName().isEmpty() )
        return rWrt;

    // bOn controls if we are writing the opening or closing tag
    if( !bOn )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_anchor, false );
        return rWrt;
    }

    OString sOut = "<" + OString(OOO_STRING_SVTOOLS_HTML_anchor);

    bool bScriptDependent = false;
    {
        const SwCharFormat* pFormat = rWrt.pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                 RES_POOLCHR_INET_NORMAL );
        std::unique_ptr<SwHTMLFormatInfo> pFormatInfo(new SwHTMLFormatInfo(pFormat));
        auto const it = rHTMLWrt.m_CharFormatInfos.find( pFormatInfo );
        if (it != rHTMLWrt.m_CharFormatInfos.end())
        {
            bScriptDependent = (*it)->bScriptDependent;
        }
    }
    if( !bScriptDependent )
    {
        const SwCharFormat* pFormat = rWrt.pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool(
                 RES_POOLCHR_INET_VISIT );
        std::unique_ptr<SwHTMLFormatInfo> pFormatInfo(new SwHTMLFormatInfo(pFormat));
        auto const it = rHTMLWrt.m_CharFormatInfos.find( pFormatInfo );
        if (it != rHTMLWrt.m_CharFormatInfos.end())
        {
            bScriptDependent = (*it)->bScriptDependent;
        }
    }

    if( bScriptDependent )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_class) + "=\"";
        const sal_Char* pStr = 0;
        switch( rHTMLWrt.nCSS1Script )
        {
        case CSS1_OUTMODE_WESTERN:
            pStr = "western";
            break;
        case CSS1_OUTMODE_CJK:
            pStr = "cjk";
            break;
        case CSS1_OUTMODE_CTL:
            pStr = "ctl";
            break;
        }
        sOut += OString(pStr) + "\"";
    }

    rWrt.Strm().WriteOString( sOut );
    sOut = "";

    OUString sRel;

    if( !aURL.isEmpty() || bEvents )
    {
        OUString sTmp( aURL.toAsciiUpperCase() );
        sal_Int32 nPos = sTmp.indexOf( "\" REL=" );
        if( nPos >= 0 )
        {
            sRel = aURL.copy( nPos+1 );
            aURL = aURL.copy( 0, nPos);
        }
        aURL = comphelper::string::strip(aURL, ' ');

        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_href) + "=\"";
        rWrt.Strm().WriteOString( sOut );
        rHTMLWrt.OutHyperlinkHRefValue( aURL );
        sOut = "\"";
    }

    if( !rINetFormat.GetName().isEmpty() )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_name) + "=\"";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( rWrt.Strm(), rINetFormat.GetName(),
                                  rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut = "\"";
    }

    const OUString& rTarget = rINetFormat.GetTargetFrame();
    if( !rTarget.isEmpty() )
    {
        sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_target) + "=\"";
        rWrt.Strm().WriteOString( sOut );
        HTMLOutFuncs::Out_String( rWrt.Strm(), rTarget, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut = "\"";
    }

    if( !sRel.isEmpty() )
        sOut += OUStringToOString(sRel, RTL_TEXTENCODING_ASCII_US);

    if( !sOut.isEmpty() )
        rWrt.Strm().WriteOString( sOut );

    if( bEvents )
        HTMLOutFuncs::Out_Events( rWrt.Strm(), *pMacTable, aAnchorEventTable,
                                  rHTMLWrt.bCfgStarBasic, rHTMLWrt.eDestEnc,
                                     &rHTMLWrt.aNonConvertableCharacters    );
    rWrt.Strm().WriteCharPtr( ">" );

    return rWrt;
}

static Writer& OutHTML_SwFormatINetFormat( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SwFormatINetFormat& rINetFormat = static_cast<const SwFormatINetFormat&>(rHt);

    if( rHTMLWrt.bTagOn )
    {
        // ggf. ein noch offenes Attribut voruebergehend beenden
        if( rHTMLWrt.aINetFormats.size() )
        {
            SwFormatINetFormat *pINetFormat =
                rHTMLWrt.aINetFormats.back();
            OutHTML_INetFormat( rWrt, *pINetFormat, false );
        }

        // jetzt das neue aufmachen
        OutHTML_INetFormat( rWrt, rINetFormat, true );

        // und merken
        SwFormatINetFormat *pINetFormat = new SwFormatINetFormat( rINetFormat );
        rHTMLWrt.aINetFormats.push_back( pINetFormat );
    }
    else
    {
        // das
        OutHTML_INetFormat( rWrt, rINetFormat, false );

        OSL_ENSURE( rHTMLWrt.aINetFormats.size(), "da fehlt doch ein URL-Attribut" );
        if( rHTMLWrt.aINetFormats.size() )
        {
            // das eigene Attribut vom Stack holen
            SwFormatINetFormat *pINetFormat = rHTMLWrt.aINetFormats.back();
            rHTMLWrt.aINetFormats.pop_back();
            delete pINetFormat;
        }

        if( !rHTMLWrt.aINetFormats.empty() )
        {
            // es ist noch ein Attribut auf dem Stack, das wieder geoeffnet
            // werden muss
            SwFormatINetFormat *pINetFormat = rHTMLWrt.aINetFormats.back();
            OutHTML_INetFormat( rWrt, *pINetFormat, true );
        }
    }

    return rWrt;
}

static Writer& OutHTML_SwTextCharFormat( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SwFormatCharFormat& rChrFormat = static_cast<const SwFormatCharFormat&>(rHt);
    const SwCharFormat* pFormat = rChrFormat.GetCharFormat();

    if( !pFormat )
    {
        return rWrt;
    }

    std::unique_ptr<SwHTMLFormatInfo> pTmpInfo(new SwHTMLFormatInfo(pFormat));
    SwHTMLFormatInfos::const_iterator it = rHTMLWrt.m_CharFormatInfos.find(pTmpInfo);
    if (it == rHTMLWrt.m_CharFormatInfos.end())
        return rWrt;

    const SwHTMLFormatInfo *pFormatInfo = it->get();
    OSL_ENSURE( pFormatInfo, "Wieso gint es keine Infos ueber die Zeichenvorlage?" );

    if( rHTMLWrt.bTagOn )
    {
        OString sOut = "<";
        if( !pFormatInfo->aToken.isEmpty() )
            sOut += pFormatInfo->aToken;
        else
            sOut += OString(OOO_STRING_SVTOOLS_HTML_span);

        if( rHTMLWrt.bCfgOutStyles &&
            (!pFormatInfo->aClass.isEmpty() || pFormatInfo->bScriptDependent) )
        {
            sOut += " " + OString(OOO_STRING_SVTOOLS_HTML_O_class) + "=\"";
            rWrt.Strm().WriteOString( sOut );
            OUString aClass( pFormatInfo->aClass );
            if( pFormatInfo->bScriptDependent )
            {
                if( !aClass.isEmpty() )
                   aClass += "-";
                switch( rHTMLWrt.nCSS1Script )
                {
                case CSS1_OUTMODE_WESTERN:
                    aClass += "western";
                    break;
                case CSS1_OUTMODE_CJK:
                    aClass += "cjk";
                    break;
                case CSS1_OUTMODE_CTL:
                    aClass += "ctl";
                    break;
                }
            }
            HTMLOutFuncs::Out_String( rWrt.Strm(), aClass,
                                          rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
            sOut = "\"";
        }
        sOut += ">";
        rWrt.Strm().WriteOString( sOut );
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                !pFormatInfo->aToken.isEmpty() ? pFormatInfo->aToken.getStr()
                                       : OOO_STRING_SVTOOLS_HTML_span,
                false );
    }

    return rWrt;
}

static Writer& OutHTML_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    if( !rHTMLWrt.bOutOpts || !rHTMLWrt.bTagOn )
        return  rWrt;

    const SvxAdjustItem& rAdjust = static_cast<const SvxAdjustItem&>(rHt);
    const sal_Char* pStr = 0;
    switch( rAdjust.GetAdjust() )
    {
    case SVX_ADJUST_CENTER: pStr = OOO_STRING_SVTOOLS_HTML_AL_center; break;
    case SVX_ADJUST_LEFT: pStr = OOO_STRING_SVTOOLS_HTML_AL_left; break;
    case SVX_ADJUST_RIGHT: pStr = OOO_STRING_SVTOOLS_HTML_AL_right; break;
    case SVX_ADJUST_BLOCK: pStr = OOO_STRING_SVTOOLS_HTML_AL_justify; break;
    default:
        ;
    }
    if( pStr )
    {
        OString sOut = " " + OString(OOO_STRING_SVTOOLS_HTML_O_align) + "=\"" +
            OString(pStr) + "\"";
        rWrt.Strm().WriteOString( sOut );
    }

    return rWrt;
}

/*
 * lege hier die Tabellen fuer die HTML-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der HTML-DLL
 * bekannt sein muessen.
 */

SwAttrFnTab aHTMLAttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutHTML_CSS1Attr,
/* RES_CHRATR_CHARSETCOLOR  */      0,
/* RES_CHRATR_COLOR */              OutHTML_SvxColor,
/* RES_CHRATR_CONTOUR   */          0,
/* RES_CHRATR_CROSSEDOUT    */      OutHTML_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutHTML_SvxEscapement,
/* RES_CHRATR_FONT  */              OutHTML_SvxFont,
/* RES_CHRATR_FONTSIZE  */          OutHTML_SvxFontHeight,
/* RES_CHRATR_KERNING   */          OutHTML_CSS1Attr,
/* RES_CHRATR_LANGUAGE  */          OutHTML_SvxLanguage,
/* RES_CHRATR_POSTURE   */          OutHTML_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          0,
/* RES_CHRATR_UNDERLINE */          OutHTML_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutHTML_SwWeight,
/* RES_CHRATR_WORDLINEMODE  */      0,
/* RES_CHRATR_AUTOKERN  */          0,
/* RES_CHRATR_BLINK */              OutHTML_SwBlink,
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutHTML_CSS1Attr, // Neu: Zeichenhintergrund
/* RES_CHRATR_CJK_FONT */           OutHTML_SvxFont,
/* RES_CHRATR_CJK_FONTSIZE */       OutHTML_SvxFontHeight,
/* RES_CHRATR_CJK_LANGUAGE */       OutHTML_SvxLanguage,
/* RES_CHRATR_CJK_POSTURE */        OutHTML_SwPosture,
/* RES_CHRATR_CJK_WEIGHT */         OutHTML_SwWeight,
/* RES_CHRATR_CTL_FONT */           OutHTML_SvxFont,
/* RES_CHRATR_CTL_FONTSIZE */       OutHTML_SvxFontHeight,
/* RES_CHRATR_CTL_LANGUAGE */       OutHTML_SvxLanguage,
/* RES_CHRATR_CTL_POSTURE */        OutHTML_SwPosture,
/* RES_CHRATR_CTL_WEIGHT */         OutHTML_SwWeight,
/* RES_CHRATR_ROTATE */             0,
/* RES_CHRATR_EMPHASIS_MARK */      0,
/* RES_CHRATR_TWO_LINES */          0,
/* RES_CHRATR_SCALEW */             0,
/* RES_CHRATR_RELIEF */             0,
/* RES_CHRATR_HIDDEN */             OutHTML_CSS1Attr,
/* RES_CHRATR_OVERLINE */           OutHTML_CSS1Attr,
/* RES_CHRATR_RSID */               0,
/* RES_CHRATR_BOX */                OutHTML_CSS1Attr,
/* RES_CHRATR_SHADOW */             0,
/* RES_CHRATR_HIGHLIGHT */          0,
/* RES_CHRATR_GRABBAG */            0,
/* RES_CHRATR_BIDIRTL */            0,
/* RES_CHRATR_IDCTHINT */           0,

/* RES_TXTATR_REFMARK */            0,
/* RES_TXTATR_TOXMARK */            0,
/* RES_TXTATR_META */               0,
/* RES_TXTATR_METAFIELD */          0,
/* RES_TXTATR_AUTOFMT */            0,
/* RES_TXTATR_INETFMT */            OutHTML_SwFormatINetFormat,
/* RES_TXTATR_CHARFMT */            OutHTML_SwTextCharFormat,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_INPUTFIELD */         OutHTML_SwFormatField,

/* RES_TXTATR_FIELD */              OutHTML_SwFormatField,
/* RES_TXTATR_FLYCNT */             OutHTML_SwFlyCnt,
/* RES_TXTATR_FTN */                OutHTML_SwFormatFootnote,
/* RES_TXTATR_ANNOTATION */         OutHTML_SwFormatField,
/* RES_TXTATR_DUMMY3 */             0,
/* RES_TXTATR_DUMMY1 */             0, // Dummy:
/* RES_TXTATR_DUMMY2 */             0, // Dummy:

/* RES_PARATR_LINESPACING   */      0,
/* RES_PARATR_ADJUST    */          OutHTML_SvxAdjust,
/* RES_PARATR_SPLIT */              0,
/* RES_PARATR_WIDOWS    */          0,
/* RES_PARATR_ORPHANS   */          0,
/* RES_PARATR_TABSTOP   */          0,
/* RES_PARATR_HYPHENZONE*/          0,
/* RES_PARATR_DROP */               OutHTML_CSS1Attr,
/* RES_PARATR_REGISTER */           0, // neu:  Registerhaltigkeit
/* RES_PARATR_NUMRULE */            0, // Dummy:
/* RES_PARATR_SCRIPTSPACE */        0, // Dummy:
/* RES_PARATR_HANGINGPUNCTUATION */ 0, // Dummy:
/* RES_PARATR_FORBIDDEN_RULES */    0, // new
/* RES_PARATR_VERTALIGN */          0, // new
/* RES_PARATR_SNAPTOGRID*/          0, // new
/* RES_PARATR_CONNECT_TO_BORDER */  0, // new
/* RES_PARATR_OUTLINELEVEL */       0,
/* RES_PARATR_RSID */               0,
/* RES_PARATR_GRABBAG */            0,

/* RES_PARATR_LIST_ID */            0, // new
/* RES_PARATR_LIST_LEVEL */         0, // new
/* RES_PARATR_LIST_ISRESTART */     0, // new
/* RES_PARATR_LIST_RESTARTVALUE */  0, // new
/* RES_PARATR_LIST_ISCOUNTED */     0, // new

/* RES_FILL_ORDER   */              0,
/* RES_FRM_SIZE */                  0,
/* RES_PAPER_BIN    */              0,
/* RES_LR_SPACE */                  0,
/* RES_UL_SPACE */                  0,
/* RES_PAGEDESC */                  0,
/* RES_BREAK */                     0,
/* RES_CNTNT */                     0,
/* RES_HEADER */                    0,
/* RES_FOOTER */                    0,
/* RES_PRINT */                     0,
/* RES_OPAQUE */                    0,
/* RES_PROTECT */                   0,
/* RES_SURROUND */                  0,
/* RES_VERT_ORIENT */               0,
/* RES_HORI_ORIENT */               0,
/* RES_ANCHOR */                    0,
/* RES_BACKGROUND */                0,
/* RES_BOX  */                      0,
/* RES_SHADOW */                    0,
/* RES_FRMMACRO */                  0,
/* RES_COL */                       0,
/* RES_KEEP */                      0,
/* RES_URL */                       0,
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_CHAIN */                     0,
/* RES_TEXTGRID */                  0,
/* RES_LINENUMBER */                0,
/* RES_FTN_AT_TXTEND */             0,
/* RES_END_AT_TXTEND */             0,
/* RES_COLUMNBALANCE */             0,
/* RES_FRAMEDIR */                  0,
/* RES_HEADER_FOOTER_EAT_SPACING */ 0,
/* RES_ROW_SPLIT */                 0,
/* RES_FOLLOW_TEXT_FLOW */          0,
/* RES_COLLAPSING_BORDERS */        0,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  0,
/* RES_AUTO_STYLE */                0,
/* RES_FRMATR_STYLE_NAME */         0,
/* RES_FRMATR_CONDITIONAL_STYLE_NAME */ 0,
/* RES_FRMATR_GRABBAG */            0,
/* RES_TEXT_VERT_ADJUST */          0,

/* RES_GRFATR_MIRRORGRF */          0,
/* RES_GRFATR_CROPGRF   */          0,
/* RES_GRFATR_ROTATION */           0,
/* RES_GRFATR_LUMINANCE */          0,
/* RES_GRFATR_CONTRAST */           0,
/* RES_GRFATR_CHANNELR */           0,
/* RES_GRFATR_CHANNELG */           0,
/* RES_GRFATR_CHANNELB */           0,
/* RES_GRFATR_GAMMA */              0,
/* RES_GRFATR_INVERT */             0,
/* RES_GRFATR_TRANSPARENCY */       0,
/* RES_GRFATR_DRWAMODE */           0,
/* RES_GRFATR_DUMMY1 */             0,
/* RES_GRFATR_DUMMY2 */             0,
/* RES_GRFATR_DUMMY3 */             0,
/* RES_GRFATR_DUMMY4 */             0,
/* RES_GRFATR_DUMMY5 */             0,

/* RES_BOXATR_FORMAT */             0,
/* RES_BOXATR_FORMULA */            0,
/* RES_BOXATR_VALUE */              0
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
