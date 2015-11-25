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

#include <stdlib.h>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/urihelper.hxx>
#include <rtl/tencinfo.h>
#include <vcl/wrkwin.hxx>
#include <sfx2/linkmgr.hxx>

#include <svtools/htmlcfg.hxx>
#include <vcl/svapp.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/frmdiritem.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#include <com/sun/star/form/XFormController.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <fmthdft.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <txatbase.hxx>
#include <frmatr.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>
#include <fltini.hxx>
#include <viewopt.hxx>
#include <IMark.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <section.hxx>
#include <swtable.hxx>
#include <fldbas.hxx>
#include <fmtclds.hxx>
#include <docsh.hxx>
#include <wrthtml.hxx>
#include <htmlnum.hxx>
#include <htmlfly.hxx>
#include <swmodule.hxx>
#include <statstr.hrc>
#include <swerror.h>
#include <rtl/strbuf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <xmloff/odffields.hxx>
#include <tools/urlobj.hxx>

#define MAX_INDENT_LEVEL 20

using namespace css;

static sal_Char sIndentTabs[MAX_INDENT_LEVEL+2] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

SwHTMLWriter::SwHTMLWriter( const OUString& rBaseURL )
    : m_pHTMLPosFlyFrames(nullptr)
    , m_pNumRuleInfo(new SwHTMLNumRuleInfo)
    , m_pNextNumRuleInfo(nullptr)
    , m_nHTMLMode(0)
    , m_eCSS1Unit(FUNIT_NONE)
    , m_pFootEndNotes(nullptr)
    , mxFormComps()
    , m_pTemplate(nullptr)
    , m_pDfltColor(nullptr)
    , m_pStartNdIdx(nullptr)
    , m_pCurrPageDesc(nullptr)
    , m_pFormatFootnote(nullptr)
    , m_nWarn(0)
    , m_nLastLFPos(0)
    , m_nLastParaToken(0)
    , m_nBkmkTabPos(-1)
    , m_nImgMapCnt(1)
    , m_nFormCntrlCnt(0)
    , m_nEndNote(0)
    , m_nFootNote(0)
    , m_nLeftMargin(0)
    , m_nDfltLeftMargin(0)
    , m_nDfltRightMargin(0)
    , m_nFirstLineIndent(0)
    , m_nDfltFirstLineIndent(0)
    , m_nDfltTopMargin(0)
    , m_nDfltBottomMargin(0)
    , m_nIndentLvl(0)
    , m_nWhishLineLen(0)
    , m_nDefListLvl(0)
    , m_nDefListMargin(0)
    , m_nHeaderFooterSpace(0)
    , m_nTextAttrsToIgnore(0)
    , m_nExportMode(0)
    , m_nCSS1OutMode(0)
    , m_nCSS1Script(CSS1_OUTMODE_WESTERN)
    , m_nDirection(FRMDIR_HORI_LEFT_TOP)
    , m_eDestEnc(RTL_TEXTENCODING_MS_1252)
    , m_eLang(LANGUAGE_DONTKNOW)
    , m_bCfgOutStyles( false )
    , m_bCfgPreferStyles( false )
    , m_bCfgFormFeed( false )
    , m_bCfgStarBasic( false )
    , m_bCfgCpyLinkedGrfs( false )
    , m_bFirstLine(true)
    , m_bTagOn( false )
    , m_bTextAttr( false )
    , m_bOutOpts( false )
    , m_bOutTable( false )
    , m_bOutHeader( false )
    , m_bOutFooter( false )
    , m_bOutFlyFrame( false )
    , m_bFirstCSS1Rule( false )
    , m_bFirstCSS1Property( false )
    , m_bPoolCollTextModified( false )
    , m_bCSS1IgnoreFirstPageDesc( false )
    , m_bNoAlign( false )
    , m_bClearLeft( false )
    , m_bClearRight( false )
    , m_bLFPossible( false )
    , m_bPreserveForm( false )
    , m_bCfgNetscape4( false )
    , mbSkipImages(false)
    , mbSkipHeaderFooter(false)
    , m_bCfgPrintLayout( false )
    , m_bParaDotLeaders( false )
{
    SetBaseURL(rBaseURL);
}

SwHTMLWriter::~SwHTMLWriter()
{
    delete m_pNumRuleInfo;
}

void SwHTMLWriter::SetupFilterOptions(SfxMedium& rMedium)
{
    const SfxItemSet* pSet = rMedium.GetItemSet();
    if (pSet == nullptr)
        return;

    const SfxPoolItem* pItem;
    if (pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) != SfxItemState::SET)
        return;


    OUString sFilterOptions = static_cast<const SfxStringItem*>(pItem)->GetValue();
    if (sFilterOptions == "SkipImages")
    {
        mbSkipImages = true;
    }
    else if (sFilterOptions == "SkipHeaderFooter")
    {
        mbSkipHeaderFooter = true;
    }
}

sal_uLong SwHTMLWriter::WriteStream()
{
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();

    // font heights 1-7
    m_aFontHeights[0] = rHtmlOptions.GetFontSize( 0 ) * 20;
    m_aFontHeights[1] = rHtmlOptions.GetFontSize( 1 ) * 20;
    m_aFontHeights[2] = rHtmlOptions.GetFontSize( 2 ) * 20;
    m_aFontHeights[3] = rHtmlOptions.GetFontSize( 3 ) * 20;
    m_aFontHeights[4] = rHtmlOptions.GetFontSize( 4 ) * 20;
    m_aFontHeights[5] = rHtmlOptions.GetFontSize( 5 ) * 20;
    m_aFontHeights[6] = rHtmlOptions.GetFontSize( 6 ) * 20;

    // ueberhaupt Styles ausgeben
    // (dann auch obere und untere Absatz-Abstaende)
    m_nExportMode = rHtmlOptions.GetExportMode();
    m_nHTMLMode = GetHtmlMode(nullptr);

    if( HTML_CFG_WRITER == m_nExportMode || HTML_CFG_NS40 == m_nExportMode )
        m_nHTMLMode |= HTMLMODE_BLOCK_SPACER;

    if( HTML_CFG_WRITER == m_nExportMode || HTML_CFG_MSIE == m_nExportMode )
        m_nHTMLMode |= (HTMLMODE_FLOAT_FRAME | HTMLMODE_LSPACE_IN_NUMBUL);

    if( HTML_CFG_MSIE == m_nExportMode )
        m_nHTMLMode |= HTMLMODE_NBSP_IN_TABLES;

    if( HTML_CFG_WRITER == m_nExportMode || HTML_CFG_NS40 == m_nExportMode || HTML_CFG_MSIE == m_nExportMode )
        m_nHTMLMode |= HTMLMODE_ABS_POS_FLY | HTMLMODE_ABS_POS_DRAW;

    if( HTML_CFG_WRITER == m_nExportMode )
        m_nHTMLMode |= HTMLMODE_FLY_MARGINS;

    if( HTML_CFG_NS40 == m_nExportMode )
        m_nHTMLMode |= HTMLMODE_BORDER_NONE;

    m_nHTMLMode |= HTMLMODE_FONT_GENERIC;

    if( HTML_CFG_NS40==m_nExportMode )
        m_nHTMLMode |= HTMLMODE_NO_CONTROL_CENTERING;

    m_bCfgOutStyles = IsHTMLMode(HTMLMODE_SOME_STYLES | HTMLMODE_FULL_STYLES);
    m_bCfgNetscape4 = (HTML_CFG_NS40 == m_nExportMode);

    if( IsHTMLMode(HTMLMODE_SOME_STYLES | HTMLMODE_FULL_STYLES) )
        m_nHTMLMode |= HTMLMODE_PRINT_EXT;

    const sal_Char *pHelpHack = getenv( "HelpEx" );
    if( pHelpHack )
    {
        OString aTmp(pHelpHack);
        if (aTmp.equalsIgnoreAsciiCase("Hilfe"))
            m_nHTMLMode |= HTMLMODE_NO_BR_AT_PAREND;
    }

    m_eCSS1Unit = (FieldUnit)SW_MOD()->GetMetric( pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) );

    bool bWriteUTF8 = bWriteClipboardDoc;
    m_eDestEnc = bWriteUTF8 ? RTL_TEXTENCODING_UTF8 : rHtmlOptions.GetTextEncoding();
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( m_eDestEnc );
    m_eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    // Nur noch fuer den MS-IE ziehen wir den Export von Styles vor.
    m_bCfgPreferStyles = HTML_CFG_MSIE == m_nExportMode;

    m_bCfgStarBasic = rHtmlOptions.IsStarBasic();

    m_bCfgFormFeed = !IsHTMLMode(HTMLMODE_PRINT_EXT);
    m_bCfgCpyLinkedGrfs = rHtmlOptions.IsSaveGraphicsLocal();

    m_bCfgPrintLayout = rHtmlOptions.IsPrintLayoutExtension();

    // die HTML-Vorlage holen
    bool bOldHTMLMode = false;
    SwTextFormatColls::size_type nOldTextFormatCollCnt = 0;
    SwCharFormats::size_type nOldCharFormatCnt = 0;

    OSL_ENSURE( !m_pTemplate, "Wo kommt denn die HTML-Vorlage hier her?" );
    m_pTemplate = static_cast<HTMLReader*>(ReadHTML)->GetTemplateDoc();
    if( m_pTemplate )
    {
        m_pTemplate->acquire();
        bOldHTMLMode = m_pTemplate->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE);
        m_pTemplate->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, true);

        nOldTextFormatCollCnt = m_pTemplate->GetTextFormatColls()->size();
        nOldCharFormatCnt = m_pTemplate->GetCharFormats()->size();
    }

    if( bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, pDoc->GetNodes().Count(),
                         pDoc->GetDocShell());

    m_pDfltColor = nullptr;
    m_pFootEndNotes = nullptr;
    m_pFormatFootnote = nullptr;
    m_bOutTable = m_bOutHeader = m_bOutFooter = m_bOutFlyFrame = false;
    mxFormComps.clear();
    m_nFormCntrlCnt = 0;
    m_bPreserveForm = false;
    m_bClearLeft = m_bClearRight = false;
    m_bLFPossible = false;

    m_nLeftMargin = m_nDfltLeftMargin = m_nDfltRightMargin = 0;
    m_nDfltTopMargin = m_nDfltBottomMargin = 0;
    m_nFirstLineIndent = m_nDfltFirstLineIndent = 0;
    m_bPoolCollTextModified = false;
    m_bFirstCSS1Property = m_bFirstCSS1Rule = false;
    m_bCSS1IgnoreFirstPageDesc = false;
    m_nIndentLvl = 0;
    m_nWhishLineLen = 70;
    m_nLastLFPos = 0;
    m_nDefListLvl = 0;
    m_nDefListMargin = ((m_pTemplate && !m_bCfgOutStyles) ? m_pTemplate : pDoc)
        ->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_HTML_DD, false )
        ->GetLRSpace().GetTextLeft();
    m_nHeaderFooterSpace = 0;
    m_nTextAttrsToIgnore = 0;
    m_nCSS1OutMode = 0;
    SvtScriptType nScript = SvtLanguageOptions::GetScriptTypeOfLanguage( GetAppLanguage() );
    switch( nScript )
    {
    case SvtScriptType::ASIAN:
        m_nCSS1Script = CSS1_OUTMODE_CJK;
        break;
    case SvtScriptType::COMPLEX:
        m_nCSS1Script = CSS1_OUTMODE_CTL;
        break;
    default:
        m_nCSS1Script = CSS1_OUTMODE_WESTERN;
        break;
    }
    m_eLang = static_cast<const SvxLanguageItem&>(pDoc
            ->GetDefault(GetLangWhichIdFromScript(m_nCSS1Script))).GetLanguage();

    m_nFootNote = m_nEndNote = 0;

    m_nWarn = 0;
    GetNumInfo().Clear();
    m_pNextNumRuleInfo = nullptr;

    OString aStartTags;

    // Tabellen und Bereiche am Doc.-Anfang beachten
    {
        SwTableNode * pTNd = pCurPam->GetNode().FindTableNode();
        if( pTNd && bWriteAll )
        {
            // mit dem Tabellen-Node anfangen !!
            pCurPam->GetPoint()->nNode = *pTNd;

            if( bWriteOnlyFirstTable )
                pCurPam->GetMark()->nNode = *pTNd->EndOfSectionNode();
        }

        // erster Node (der einen Seitenumbruch enthalten darf)
        m_pStartNdIdx = new SwNodeIndex( pCurPam->GetPoint()->nNode );

        SwSectionNode * pSNd = pCurPam->GetNode().FindSectionNode();
        while( pSNd )
        {
            if( bWriteAll )
            {
                // mit dem Section-Node anfangen !!
                pCurPam->GetPoint()->nNode = *pSNd;
            }
            else
            {
                OSL_ENSURE( FILE_LINK_SECTION != pSNd->GetSection().GetType(),
                        "Export gelinkter Bereiche am Dok-Anfang ist nicht implemntiert" );

                // nur das Tag fuer die Section merken
                OString aName = HTMLOutFuncs::ConvertStringToHTML(
                    pSNd->GetSection().GetSectionName(), m_eDestEnc,
                    &m_aNonConvertableCharacters );

                OStringBuffer sOut;
                sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_division)
                    .append(' ').append(OOO_STRING_SVTOOLS_HTML_O_id)
                    .append("=\"").append(aName).append('\"').append('>')
                    .append(aStartTags);
                aStartTags = sOut.makeStringAndClear();
            }
            // FindSectionNode() an einem SectionNode liefert den selben!
            pSNd = pSNd->StartOfSectionNode()->FindSectionNode();
        }
    }

    // Tabelle fuer die freifliegenden Rahmen erzeugen, aber nur wenn
    // das gesamte Dokument geschrieben wird
    m_pHTMLPosFlyFrames = nullptr;
    CollectFlyFrames();
    m_nLastParaToken = 0;
    GetControls();
    CollectLinkTargets();

    sal_uInt16 nHeaderAttrs = 0;
    m_pCurrPageDesc = MakeHeader( nHeaderAttrs );

    m_bLFPossible = true;

    // Formulare, die nur HiddenControls enthalten ausgeben.
    OutHiddenForms();

    if( !aStartTags.isEmpty() )
        Strm().WriteCharPtr( aStartTags.getStr() );

    const SfxPoolItem *pItem;
    const SfxItemSet& rPageItemSet = m_pCurrPageDesc->GetMaster().GetAttrSet();
    if( !bWriteClipboardDoc && pDoc->GetDocShell() &&
         (!pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) &&
          !pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE)) &&
        SfxItemState::SET == rPageItemSet.GetItemState( RES_HEADER, true, &pItem) )
    {
        const SwFrameFormat *pHeaderFormat =
            static_cast<const SwFormatHeader *>(pItem)->GetHeaderFormat();
        if( pHeaderFormat )
            OutHTML_HeaderFooter( *this, *pHeaderFormat, true );
    }

    m_nTextAttrsToIgnore = nHeaderAttrs;
    Out_SwDoc( pOrigPam );
    m_nTextAttrsToIgnore = 0;

    if( mxFormComps.is() )
        OutForm( false, mxFormComps );

    if( m_pFootEndNotes )
        OutFootEndNotes();

    if( !bWriteClipboardDoc && pDoc->GetDocShell() &&
        (!pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) && !pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE))  &&
        SfxItemState::SET == rPageItemSet.GetItemState( RES_FOOTER, true, &pItem) )
    {
        const SwFrameFormat *pFooterFormat =
            static_cast<const SwFormatFooter *>(pItem)->GetFooterFormat();
        if( pFooterFormat )
            OutHTML_HeaderFooter( *this, *pFooterFormat, false );
    }

    if( m_bLFPossible )
        OutNewLine();
    if (!mbSkipHeaderFooter)
    {
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_body, false );
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_html, false );
    }

    // loesche die Tabelle mit den freifliegenden Rahmen
    OSL_ENSURE( !m_pHTMLPosFlyFrames, "Wurden nicht alle Rahmen ausgegeben" );
    if( m_pHTMLPosFlyFrames )
    {
        m_pHTMLPosFlyFrames->DeleteAndDestroyAll();
        delete m_pHTMLPosFlyFrames;
        m_pHTMLPosFlyFrames = nullptr;
    }

    m_aHTMLControls.DeleteAndDestroyAll();

    if (!m_CharFormatInfos.empty())
        m_CharFormatInfos.clear();

    if (!m_TextCollInfos.empty())
        m_TextCollInfos.clear();

    if(!m_aImgMapNames.empty())
        m_aImgMapNames.clear();

    m_aImplicitMarks.clear();

    m_aOutlineMarks.clear();

    m_aOutlineMarkPoss.clear();

    m_aNumRuleNames.clear();

    m_aScriptParaStyles.clear();
    m_aScriptTextStyles.clear();

    delete m_pDfltColor;
    m_pDfltColor = nullptr;

    delete m_pStartNdIdx;
    m_pStartNdIdx = nullptr;

    mxFormComps.clear();

    OSL_ENSURE( !m_pFootEndNotes,
            "SwHTMLWriter::Write: Footnotes nicht durch OutFootEndNotes geloescht" );

    m_pCurrPageDesc = nullptr;

    ClearNextNumInfo();

    for( int i=0; i<MAXLEVEL; ++i )
        m_aBulletGrfs[i].clear();

    m_aNonConvertableCharacters.clear();

    if( bShowProgress )
        ::EndProgress( pDoc->GetDocShell() );

    if( m_pTemplate )
    {
        // Waehrend des Exports angelegte Zeichen- und Abastzvorlagen
        // loeschen
        auto nTextFormatCollCnt = m_pTemplate->GetTextFormatColls()->size();
        while( nTextFormatCollCnt > nOldTextFormatCollCnt )
            m_pTemplate->DelTextFormatColl( --nTextFormatCollCnt );
        OSL_ENSURE( m_pTemplate->GetTextFormatColls()->size() == nOldTextFormatCollCnt,
                "falsche Anzahl TextFormatColls geloescht" );

        auto nCharFormatCnt = m_pTemplate->GetCharFormats()->size();
        while( nCharFormatCnt > nOldCharFormatCnt )
            m_pTemplate->DelCharFormat( --nCharFormatCnt );
        OSL_ENSURE( m_pTemplate->GetCharFormats()->size() == nOldCharFormatCnt,
                "falsche Anzahl CharFormats geloescht" );

        // HTML-Modus wieder restaurieren
        m_pTemplate->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, bOldHTMLMode);

        if( 0 == m_pTemplate->release() )
            delete m_pTemplate;

        m_pTemplate = nullptr;
    }

    return m_nWarn;
}

static const SwFormatCol *lcl_html_GetFormatCol( const SwSection& rSection,
                                       const SwSectionFormat& rFormat )
{
    const SwFormatCol *pCol = nullptr;

    const SfxPoolItem* pItem;
    if( FILE_LINK_SECTION != rSection.GetType() &&
        SfxItemState::SET == rFormat.GetAttrSet().GetItemState(RES_COL,false,&pItem) &&
        static_cast<const SwFormatCol *>(pItem)->GetNumCols() > 1 )
    {
        pCol = static_cast<const SwFormatCol *>(pItem);
    }

    return pCol;
}

static bool lcl_html_IsMultiColStart( const SwHTMLWriter& rHTMLWrt, sal_uLong nIndex )
{
    bool bRet = false;
    const SwSectionNode *pSectNd =
        rHTMLWrt.pDoc->GetNodes()[nIndex]->GetSectionNode();
    if( pSectNd )
    {
        const SwSection& rSection = pSectNd->GetSection();
        const SwSectionFormat *pFormat = rSection.GetFormat();
        if( pFormat && lcl_html_GetFormatCol( rSection, *pFormat ) )
            bRet = true;
    }

    return bRet;
}

static bool lcl_html_IsMultiColEnd( const SwHTMLWriter& rHTMLWrt, sal_uLong nIndex )
{
    bool bRet = false;
    const SwEndNode *pEndNd = rHTMLWrt.pDoc->GetNodes()[nIndex]->GetEndNode();
    if( pEndNd )
        bRet = lcl_html_IsMultiColStart( rHTMLWrt,
                                         pEndNd->StartOfSectionIndex() );

    return bRet;
}

static void lcl_html_OutSectionStartTag( SwHTMLWriter& rHTMLWrt,
                                     const SwSection& rSection,
                                     const SwSectionFormat& rFormat,
                                  const SwFormatCol *pCol,
                                  bool bContinued=false )
{
    OSL_ENSURE( pCol || !bContinued, "Continuation of DIV" );

    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_division);

    const OUString& rName = rSection.GetSectionName();
    if( !rName.isEmpty() && !bContinued )
    {
        sOut.append(" " + OString(OOO_STRING_SVTOOLS_HTML_O_id) + "=\"");
        rHTMLWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), rName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut.append('\"');
    }

    sal_uInt16 nDir = rHTMLWrt.GetHTMLDirection( rFormat.GetAttrSet() );
    rHTMLWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    rHTMLWrt.OutDirection( nDir );

    if( FILE_LINK_SECTION == rSection.GetType() )
    {
        sOut.append(" " + OString(OOO_STRING_SVTOOLS_HTML_O_href) + "=\"");
        rHTMLWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        const OUString& aFName = rSection.GetLinkFileName();
        OUString aURL( aFName.getToken(0,sfx2::cTokenSeparator) );
        OUString aFilter( aFName.getToken(1,sfx2::cTokenSeparator) );
        OUString aSection( aFName.getToken(2,sfx2::cTokenSeparator) );

        OUString aEncURL( URIHelper::simpleNormalizedMakeRelative(rHTMLWrt.GetBaseURL(), aURL ) );
        sal_Unicode cDelim = 255U;
        bool bURLContainsDelim = (-1 != aEncURL.indexOf( cDelim ) );

        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aEncURL,
                                  rHTMLWrt.m_eDestEnc,
                                  &rHTMLWrt.m_aNonConvertableCharacters );
        const sal_Char* pDelim = "&#255;";
        if( !aFilter.isEmpty() || !aSection.isEmpty() || bURLContainsDelim )
            rHTMLWrt.Strm().WriteCharPtr( pDelim );
        if( !aFilter.isEmpty() )
            HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aFilter, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        if( !aSection.isEmpty() || bURLContainsDelim  )
                rHTMLWrt.Strm().WriteCharPtr( pDelim );
        if( !aSection.isEmpty() )
        {
            sal_Int32 nPos = aSection.indexOf( '%' );
            while( nPos != -1 )
            {
                aSection = aSection.replaceAt(nPos, 1, "%25");
                nPos = aSection.indexOf( '%', nPos+3 );
            }
            nPos = aSection.indexOf( cDelim );
            while( nPos != -1 )
            {
                aSection = aSection.replaceAt(nPos, 1, "%FF" );
                nPos = aSection.indexOf( cDelim, nPos+3 );
            }
            HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aSection,
                                      rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        }
        sOut.append('\"');
    }
    else if( pCol )
    {
        // minimum gutter width
        sal_uInt16 nGutter = pCol->GetGutterWidth( true );
        if( nGutter!=USHRT_MAX )
        {
            if( nGutter && Application::GetDefaultDevice() )
            {
                nGutter = (sal_uInt16)Application::GetDefaultDevice()
                                ->LogicToPixel( Size(nGutter, 0), MapMode(MAP_TWIP) ).Width();
            }
            sOut.append(" " + OString(OOO_STRING_SVTOOLS_HTML_O_gutter) + "=\"" + OString::number(nGutter) + "\"");
        }
    }

    rHTMLWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    if( rHTMLWrt.IsHTMLMode( rHTMLWrt.m_bCfgOutStyles ? HTMLMODE_ON : 0 ) )
        rHTMLWrt.OutCSS1_SectionFormatOptions( rFormat, pCol );

    rHTMLWrt.Strm().WriteChar( '>' );

    rHTMLWrt.m_bLFPossible = true;
    if( !rName.isEmpty() && !bContinued )
        rHTMLWrt.OutImplicitMark( rName, "region" );

    rHTMLWrt.IncIndentLevel();
}

static void lcl_html_OutSectionEndTag( SwHTMLWriter& rHTMLWrt )
{
    rHTMLWrt.DecIndentLevel();
    if( rHTMLWrt.m_bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), OOO_STRING_SVTOOLS_HTML_division, false );
    rHTMLWrt.m_bLFPossible = true;
}

static Writer& OutHTML_Section( Writer& rWrt, const SwSectionNode& rSectNd )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // End <PRE> and any <DL>, because a definition list's level may
    // change inside the section.
    rHTMLWrt.ChangeParaToken( 0 );
    rHTMLWrt.OutAndSetDefList( 0 );

    const SwSection& rSection = rSectNd.GetSection();
    const SwSectionFormat *pFormat = rSection.GetFormat();
    OSL_ENSURE( pFormat, "Section without a format?" );

    bool bStartTag = true;
    bool bEndTag = true;
    const SwSectionFormat *pSurrFormat = nullptr;
    const SwSectionNode *pSurrSectNd = nullptr;
    const SwSection *pSurrSection = nullptr;
    const SwFormatCol *pSurrCol = nullptr;

    sal_uInt32 nSectSttIdx = rSectNd.GetIndex();
    sal_uInt32 nSectEndIdx = rSectNd.EndOfSectionIndex();
    const SwFormatCol *pCol = lcl_html_GetFormatCol( rSection, *pFormat );
    if( pCol )
    {
        // If the next node is a columned section node, too, don't export
        // an empty section.
        if( lcl_html_IsMultiColStart( rHTMLWrt, nSectSttIdx+1 ) )
            bStartTag = false;

        // The same applies if the section end with another columned section.
        if( lcl_html_IsMultiColEnd( rHTMLWrt, nSectEndIdx-1 ) )
            bEndTag = false;

        //.is there a columned section around this one?
        const SwStartNode *pSttNd = rSectNd.StartOfSectionNode();
        if( pSttNd )
        {
            pSurrSectNd = pSttNd->FindSectionNode();
            if( pSurrSectNd )
            {
                const SwStartNode *pBoxSttNd = pSttNd->FindTableBoxStartNode();
                if( !pBoxSttNd ||
                    pBoxSttNd->GetIndex() < pSurrSectNd->GetIndex() )
                {
                    pSurrSection = &pSurrSectNd->GetSection();
                    pSurrFormat = pSurrSection->GetFormat();
                    if( pSurrFormat )
                        pSurrCol = lcl_html_GetFormatCol( *pSurrSection,
                                                       *pSurrFormat );
                }
            }
        }
    }

    // The surrounding section must be closed before the current one is
    // opended, except that it start immediately before the current one or
    // another end immediately before the current one
    if( pSurrCol && nSectSttIdx - pSurrSectNd->GetIndex() > 1 &&
        !lcl_html_IsMultiColEnd( rHTMLWrt, nSectSttIdx-1 ) )
        lcl_html_OutSectionEndTag( rHTMLWrt );

    if( bStartTag )
        lcl_html_OutSectionStartTag( rHTMLWrt, rSection, *pFormat, pCol );

    {
        HTMLSaveData aSaveData( rHTMLWrt,
            rHTMLWrt.pCurPam->GetPoint()->nNode.GetIndex()+1,
            rSectNd.EndOfSectionIndex(),
            false, pFormat );
        rHTMLWrt.Out_SwDoc( rHTMLWrt.pCurPam );
    }

    rHTMLWrt.pCurPam->GetPoint()->nNode = *rSectNd.EndOfSectionNode();

    if( bEndTag )
        lcl_html_OutSectionEndTag( rHTMLWrt );

    // The surrounding section must be started again, except that it ends
    // immeditaly behind the current one.
    if( pSurrCol &&
        pSurrSectNd->EndOfSectionIndex() - nSectEndIdx > 1 &&
        !lcl_html_IsMultiColStart( rHTMLWrt, nSectEndIdx+1 ) )
        lcl_html_OutSectionStartTag( rHTMLWrt, *pSurrSection, *pSurrFormat,
                                     pSurrCol, true );

    return rWrt;
}

void SwHTMLWriter::Out_SwDoc( SwPaM* pPam )
{
    bool bSaveWriteAll = bWriteAll;     // sichern

    // suche die naechste text::Bookmark-Position aus der text::Bookmark-Tabelle
    m_nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;

    // gebe alle Bereiche des Pams in das HTML-File aus.
    do {
        bWriteAll = bSaveWriteAll;
        m_bFirstLine = true;

        // suche den ersten am Pam-auszugebenen FlyFrame
        // fehlt noch:

        while( pCurPam->GetPoint()->nNode.GetIndex() < pCurPam->GetMark()->nNode.GetIndex() ||
              (pCurPam->GetPoint()->nNode.GetIndex() == pCurPam->GetMark()->nNode.GetIndex() &&
               pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
        {
            SwNode&  rNd = pCurPam->GetNode();

            OSL_ENSURE( !(rNd.IsGrfNode() || rNd.IsOLENode()),
                    "Grf- oder OLE-Node hier unerwartet" );
            if( rNd.IsTextNode() )
            {
                SwTextNode* pTextNd = rNd.GetTextNode();

                if( !m_bFirstLine )
                    pCurPam->GetPoint()->nContent.Assign( pTextNd, 0 );

                OutHTML_SwTextNode( *this, *pTextNd );
            }
            else if( rNd.IsTableNode() )
            {
                OutHTML_SwTableNode( *this, *rNd.GetTableNode(), nullptr );
                m_nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;
            }
            else if( rNd.IsSectionNode() )
            {
                OutHTML_Section( *this, *rNd.GetSectionNode() );
                m_nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;
            }
            else if( &rNd == &pDoc->GetNodes().GetEndOfContent() )
                break;

            ++pCurPam->GetPoint()->nNode;   // Bewegen
            sal_uInt32 nPos = pCurPam->GetPoint()->nNode.GetIndex();

            if( bShowProgress )
                ::SetProgressState( nPos, pDoc->GetDocShell() );   // Wie weit ?

            /* sollen nur die Selectierten Bereiche gesichert werden, so
             * duerfen nur die vollstaendigen Nodes gespeichert werde,
             * d.H. der 1. und n. Node teilweise, der 2. bis n-1. Node
             * vollstaendig. (vollstaendig heisst mit allen Formaten! )
             */
            bWriteAll = bSaveWriteAll ||
                        nPos != pCurPam->GetMark()->nNode.GetIndex();
            m_bFirstLine = false;
            m_bOutFooter = false; // Nach einem Node keine Fusszeile mehr
        }

        ChangeParaToken( 0 ); // MIB 8.7.97: Machen wir jetzt hier und nicht
                              // beim Aufrufer
        OutAndSetDefList( 0 );

    } while( CopyNextPam( &pPam ) );        // bis alle PaM's bearbeitet

    bWriteAll = bSaveWriteAll;          // wieder auf alten Wert zurueck
}

// schreibe die StyleTabelle, algemeine Angaben,Header/Footer/Footnotes
static void OutBodyColor( const sal_Char* pTag, const SwFormat *pFormat,
                          SwHTMLWriter& rHWrt )
{
    const SwFormat *pRefFormat = nullptr;

    if( rHWrt.m_pTemplate )
        pRefFormat = SwHTMLWriter::GetTemplateFormat( pFormat->GetPoolFormatId(),
                                                &rHWrt.m_pTemplate->getIDocumentStylePoolAccess() );

    const SvxColorItem *pColorItem = nullptr;

    const SfxItemSet& rItemSet = pFormat->GetAttrSet();
    const SfxPoolItem *pRefItem = nullptr, *pItem = nullptr;
    bool bItemSet = SfxItemState::SET == rItemSet.GetItemState( RES_CHRATR_COLOR,
                                                           true, &pItem);
    bool bRefItemSet = pRefFormat &&
        SfxItemState::SET == pRefFormat->GetAttrSet().GetItemState( RES_CHRATR_COLOR,
                                                            true, &pRefItem);
    if( bItemSet )
    {
        // wenn das Item nur in der Vorlage des aktuellen Doks gesetzt
        // ist oder einen anderen Wert hat, als in der HTML-Vorlage,
        // wird es gesetzt
        const SvxColorItem *pCItem = static_cast<const SvxColorItem*>(pItem);

        if( !bRefItemSet )
        {
            pColorItem = pCItem;
        }
        else
        {
            Color aColor( pCItem->GetValue() );
            if( COL_AUTO == aColor.GetColor() )
                aColor.SetColor( COL_BLACK );

            Color aRefColor( static_cast<const SvxColorItem*>(pRefItem)->GetValue() );
            if( COL_AUTO == aRefColor.GetColor() )
                aRefColor.SetColor( COL_BLACK );

            if( !aColor.IsRGBEqual( aRefColor ) )
                pColorItem = pCItem;
        }
    }
    else if( bRefItemSet )
    {
        // Das Item war in der HTML-Vorlage noch gesetzt, also geben wir
        // das Default aus
        pColorItem = static_cast<const SvxColorItem*>(&rItemSet.GetPool()
                                        ->GetDefaultItem( RES_CHRATR_COLOR ));
    }

    if( pColorItem )
    {
        OStringBuffer sOut;
        sOut.append(" " + OString(pTag) + "=");
        rHWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        Color aColor( pColorItem->GetValue() );
        if( COL_AUTO == aColor.GetColor() )
            aColor.SetColor( COL_BLACK );
        HTMLOutFuncs::Out_Color( rHWrt.Strm(), aColor, rHWrt.m_eDestEnc );
        if( RES_POOLCOLL_STANDARD==pFormat->GetPoolFormatId() )
            rHWrt.m_pDfltColor = new Color( aColor );
    }
}

sal_uInt16 SwHTMLWriter::OutHeaderAttrs()
{
    sal_uLong nIdx = pCurPam->GetPoint()->nNode.GetIndex();
    sal_uLong nEndIdx = pCurPam->GetMark()->nNode.GetIndex();

    SwTextNode *pTextNd = nullptr;
    while( nIdx<=nEndIdx &&
        nullptr==(pTextNd=pDoc->GetNodes()[nIdx]->GetTextNode()) )
        nIdx++;

    OSL_ENSURE( pTextNd, "Kein Text-Node gefunden" );
    if( !pTextNd || !pTextNd->HasHints() )
        return 0;

    sal_uInt16 nAttrs = 0;
    const size_t nCntAttr = pTextNd->GetSwpHints().Count();
    sal_Int32 nOldPos = 0;
    for( size_t i=0; i<nCntAttr; ++i )
    {
        const SwTextAttr *pHt = pTextNd->GetSwpHints().Get(i);
        if( !pHt->End() )
        {
            sal_Int32 nPos = pHt->GetStart();
            if( nPos-nOldPos > 1
                || ( pHt->Which() != RES_TXTATR_FIELD
                     && pHt->Which() != RES_TXTATR_ANNOTATION ) )
                break;

            const sal_uInt16 nFieldWhich =
                static_cast<const SwFormatField&>(pHt->GetAttr()).GetField()->GetTyp()->Which();
            if( RES_POSTITFLD!=nFieldWhich &&
                RES_SCRIPTFLD!=nFieldWhich )
                break;

            OutNewLine();
            OutHTML_SwFormatField( *this, pHt->GetAttr() );
            nOldPos = nPos;
            OSL_ENSURE( nAttrs<SAL_MAX_UINT16, "Too many attributes" );
            nAttrs++;
        }
    }

    return nAttrs;
}

const SwPageDesc *SwHTMLWriter::MakeHeader( sal_uInt16 &rHeaderAttrs )
{
    OStringBuffer sOut;
    if (!mbSkipHeaderFooter)
    {
        sOut.append(OString(OOO_STRING_SVTOOLS_HTML_doctype) + " " + OString(OOO_STRING_SVTOOLS_HTML_doctype40));
        HTMLOutFuncs::Out_AsciiTag( Strm(), sOut.makeStringAndClear().getStr() );

        // baue den Vorspann
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_html );

        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_head );

        IncIndentLevel();   // Inhalt von <HEAD> einruecken

        // DokumentInfo
        OString sIndent = GetIndentString();

        uno::Reference<document::XDocumentProperties> xDocProps;
        SwDocShell *pDocShell(pDoc->GetDocShell());
        if (pDocShell)
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                                                                       pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            xDocProps.set(xDPS->getDocumentProperties());
        }

        // xDocProps may be null here (when copying)
        SfxFrameHTMLWriter::Out_DocInfo( Strm(), GetBaseURL(), xDocProps,
                                         sIndent.getStr(), m_eDestEnc,
                                         &m_aNonConvertableCharacters );

        // Kommentare und Meta-Tags des ersten Absatzes
        rHeaderAttrs = OutHeaderAttrs();

        OutFootEndNoteInfo();
    }

    const SwPageDesc *pPageDesc = nullptr;

    // In Nicht-HTML-Dokumenten wird die erste gesetzte Seitenvorlage
    // exportiert und wenn keine gesetzt ist die Standard-Vorlage
    sal_uLong nNodeIdx = pCurPam->GetPoint()->nNode.GetIndex();

    while( nNodeIdx < pDoc->GetNodes().Count() )
    {
        SwNode *pNd = pDoc->GetNodes()[ nNodeIdx ];
        if( pNd->IsContentNode() )
        {
            pPageDesc = static_cast<const SwFormatPageDesc &>(pNd->GetContentNode()
                ->GetAttr(RES_PAGEDESC)).GetPageDesc();
            break;
        }
        else if( pNd->IsTableNode() )
        {
            pPageDesc = pNd->GetTableNode()->GetTable().GetFrameFormat()
                           ->GetPageDesc().GetPageDesc();
            break;
        }

        nNodeIdx++;
    }

    if( !pPageDesc )
        pPageDesc = &pDoc->GetPageDesc( 0 );

    if (!mbSkipHeaderFooter)
    {
        // und nun ... das Style-Sheet!!!
        if( m_bCfgOutStyles )
        {
            OutStyleSheet( *pPageDesc );
        }

        // und nun ... das BASIC und JavaScript!
        if( pDoc->GetDocShell() )   // nur mit DocShell ist Basic moeglich
            OutBasic();

        DecIndentLevel();   // Inhalt von <HEAD> einruecken
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_head, false );

        // der Body wird nicht eingerueckt, weil sonst alles eingerueckt waere!
        OutNewLine();
        sOut.append("<" + OString(OOO_STRING_SVTOOLS_HTML_body));
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

        // language
        OutLanguage( m_eLang );

        // Textfarbe ausgeben, wenn sie an der Standard-Vorlage gesetzt ist
        // und sich geaendert hat.
        OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_text,
                      pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD, false ),
                      *this );

        // Farben fuer (un)besuchte Links
        OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_link,
                      pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( RES_POOLCHR_INET_NORMAL ),
                      *this );
        OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_vlink,
                      pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( RES_POOLCHR_INET_VISIT ),
                      *this );

        const SfxItemSet& rItemSet = pPageDesc->GetMaster().GetAttrSet();

        // fdo#86857 page styles now contain the XATTR_*, not RES_BACKGROUND
        SvxBrushItem const aBrushItem(
                getSvxBrushItemFromSourceSet(rItemSet, RES_BACKGROUND));
        OutBackground(&aBrushItem, true);

        m_nDirection = GetHTMLDirection( rItemSet );
        OutDirection( m_nDirection );

        if( m_bCfgOutStyles )
            OutCSS1_BodyTagStyleOpt( *this, rItemSet );

        // Events anhaengen
        if( pDoc->GetDocShell() )   // nur mit DocShell ist Basic moeglich
            OutBasicBodyEvents();

        Strm().WriteChar( '>' );
    }

    return pPageDesc;
}

void SwHTMLWriter::OutAnchor( const OUString& rName )
{
    OStringBuffer sOut;
    sOut.append("<" + OString(OOO_STRING_SVTOOLS_HTML_anchor) + " " + OString(OOO_STRING_SVTOOLS_HTML_O_name) + "=\"");
    Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    HTMLOutFuncs::Out_String( Strm(), rName, m_eDestEnc, &m_aNonConvertableCharacters ).WriteCharPtr( "\">" );
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_anchor, false );
}

void SwHTMLWriter::OutBookmarks()
{
    // hole das aktuelle Bookmark
    const ::sw::mark::IMark* pBookmark = nullptr;
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    if(m_nBkmkTabPos != -1)
        pBookmark = (pMarkAccess->getAllMarksBegin() + m_nBkmkTabPos)->get();
    // Ausgabe aller Bookmarks in diesem Absatz. Die Content-Position
    // wird vorerst nicht beruecksichtigt!
    sal_uInt32 nNode = pCurPam->GetPoint()->nNode.GetIndex();
    while( m_nBkmkTabPos != -1
           && pBookmark->GetMarkPos().nNode.GetIndex() == nNode )
    {
        // Der Bereich derBookmark wird erstam ignoriert, da er von uns
        // auch nicht eingelesen wird.

        // erst die SWG spezifischen Daten:
        if ( dynamic_cast< const ::sw::mark::IBookmark* >(pBookmark) && !pBookmark->GetName().isEmpty() )
        {
            OutAnchor( pBookmark->GetName() );
        }

        if( ++m_nBkmkTabPos >= pMarkAccess->getAllMarksCount() )
            m_nBkmkTabPos = -1;
        else
            pBookmark = (pMarkAccess->getAllMarksBegin() + m_nBkmkTabPos)->get();
    }

    sal_uInt32 nPos;
    for( nPos = 0; nPos < m_aOutlineMarkPoss.size() &&
                   m_aOutlineMarkPoss[nPos] < nNode; nPos++ )
        ;

    while( nPos < m_aOutlineMarkPoss.size() && m_aOutlineMarkPoss[nPos] == nNode )
    {
        OUString sMark( m_aOutlineMarks[nPos] );
        OutAnchor( sMark.replace('?', '_') ); // '?' causes problems in IE/Netscape 5
        m_aOutlineMarkPoss.erase( m_aOutlineMarkPoss.begin()+nPos );
        m_aOutlineMarks.erase( m_aOutlineMarks.begin() + nPos );
    }
}

void SwHTMLWriter::OutPointFieldmarks( const SwPosition& rPos )
{
    // "point" fieldmarks that occupy single character space, as opposed to
    // range fieldmarks that are associated with start and end points.

    const IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return;

    const sw::mark::IFieldmark* pMark = pMarkAccess->getFieldmarkFor(rPos);
    if (!pMark)
        return;

    if (pMark->GetFieldname() == ODF_FORMCHECKBOX)
    {
        const sw::mark::ICheckboxFieldmark* pCheckBox =
            dynamic_cast<const sw::mark::ICheckboxFieldmark*>(pMark);

        if (pCheckBox)
        {
            OString aOut("<");
            aOut += OOO_STRING_SVTOOLS_HTML_input;
            aOut += " ";
            aOut += OOO_STRING_SVTOOLS_HTML_O_type;
            aOut += "=\"";
            aOut += OOO_STRING_SVTOOLS_HTML_IT_checkbox;
            aOut += "\"";

            if (pCheckBox->IsChecked())
            {
                aOut += " ";
                aOut += OOO_STRING_SVTOOLS_HTML_O_checked;
                aOut += "=\"";
                aOut += OOO_STRING_SVTOOLS_HTML_O_checked;
                aOut += "\"";
            }

            aOut += "/>";
            Strm().WriteCharPtr(aOut.getStr());
        }
    }

    // TODO : Handle other single-point fieldmark types here (if any).
}

void SwHTMLWriter::OutImplicitMark( const OUString& rMark,
                                    const sal_Char *pMarkType )
{
    if( !rMark.isEmpty() && !m_aImplicitMarks.empty() )
    {
        OUString sMark(rMark + OUStringLiteral1<cMarkSeparator>() + OUString::createFromAscii(pMarkType));
        if( 0 != m_aImplicitMarks.erase( sMark ) )
        {
            OutAnchor(sMark.replace('?', '_')); // '?' causes problems in IE/Netscape 5
        }
    }
}

OUString SwHTMLWriter::convertHyperlinkHRefValue(const OUString& rURL)
{
    OUString sURL(rURL);
    sal_Int32 nPos = sURL.lastIndexOf(cMarkSeparator);
    if (nPos != -1)
    {
        OUString sCompare(comphelper::string::remove(sURL.copy(nPos + 1), ' '));
        if (!sCompare.isEmpty())
        {
            sCompare = sCompare.toAsciiLowerCase();
            if( sCompare == "region"  || sCompare == "frame"   ||
                sCompare == "graphic" || sCompare == "ole"     ||
                sCompare == "table"   || sCompare == "outline" ||
                sCompare == "text" )
            {
                sURL = sURL.replace( '?', '_' );   // '?' causes problems in IE/Netscape 5
            }
        }
    }
    else if (!sURL.isEmpty() && sURL[0] != '#')
    {
        // Link is not started from "#", so looks like external link. Encode this URL.
        INetURLObject aURL(sURL);
        sURL = aURL.GetMainURL(INetURLObject::NO_DECODE);
    }
    return URIHelper::simpleNormalizedMakeRelative( GetBaseURL(), sURL );
}

void SwHTMLWriter::OutHyperlinkHRefValue( const OUString& rURL )
{
    OUString sURL = convertHyperlinkHRefValue(rURL);
    HTMLOutFuncs::Out_String( Strm(), sURL, m_eDestEnc, &m_aNonConvertableCharacters );
}

void SwHTMLWriter::OutBackground( const SvxBrushItem *pBrushItem, bool bGraphic )
{
    const Color &rBackColor = pBrushItem->GetColor();
    /// check, if background color is not "no fill"/"auto fill", instead of
    /// only checking, if transparency is not set.
    if( rBackColor.GetColor() != COL_TRANSPARENT )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_bgcolor).append('=');
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_Color( Strm(), rBackColor, m_eDestEnc);
    }

    if( !bGraphic )
        return;

    OUString aGraphicInBase64;
    const Graphic* pGrf = pBrushItem->GetGraphic();
    if( pGrf )
    {
        if( !XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64) )
        {
            m_nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
        }
        Strm().WriteCharPtr( " " OOO_STRING_SVTOOLS_HTML_O_background "=\"" );
        Strm().WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_data ":" );
        HTMLOutFuncs::Out_String( Strm(), aGraphicInBase64, m_eDestEnc, &m_aNonConvertableCharacters ).WriteChar( '\"' );
    }
}

void SwHTMLWriter::OutBackground( const SfxItemSet& rItemSet, bool bGraphic )
{
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false,
                                               &pItem ))
    {
        OutBackground( static_cast<const SvxBrushItem*>(pItem), bGraphic );
    }
}

sal_uInt16 SwHTMLWriter::GetLangWhichIdFromScript( sal_uInt16 nScript )
{
    sal_uInt16 nWhichId;
    switch( nScript )
    {
    case CSS1_OUTMODE_CJK:
        nWhichId = RES_CHRATR_CJK_LANGUAGE;
        break;
    case CSS1_OUTMODE_CTL:
        nWhichId = RES_CHRATR_CJK_LANGUAGE;
        break;
    default:
        nWhichId = RES_CHRATR_LANGUAGE;
        break;
    }
    return nWhichId;
}

void SwHTMLWriter::OutLanguage( LanguageType nLang )
{
    if( LANGUAGE_DONTKNOW != nLang )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_lang)
            .append("=\"");
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( Strm(), LanguageTag::convertToBcp47(nLang),
                                  m_eDestEnc, &m_aNonConvertableCharacters ).WriteChar( '"' );
    }
}

sal_uInt16 SwHTMLWriter::GetHTMLDirection( const SfxItemSet& rItemSet ) const
{
    return GetHTMLDirection(
        static_cast < const SvxFrameDirectionItem& >( rItemSet.Get( RES_FRAMEDIR ) )
            .GetValue() );
}

sal_uInt16 SwHTMLWriter::GetHTMLDirection( sal_uInt16 nDir ) const
{
    switch( nDir )
    {
    case FRMDIR_VERT_TOP_LEFT:
        nDir = FRMDIR_HORI_LEFT_TOP;
        break;
    case FRMDIR_VERT_TOP_RIGHT:
        nDir = FRMDIR_HORI_RIGHT_TOP;
        break;
    case FRMDIR_ENVIRONMENT:
        nDir = m_nDirection;
    }

    return nDir;
}

void SwHTMLWriter::OutDirection( sal_uInt16 nDir )
{
    OString sConverted = convertDirection(nDir);
    if (!sConverted.isEmpty())
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_dir)
            .append("=\"").append(sConverted).append('\"');
        Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    }
}

OString SwHTMLWriter::convertDirection(sal_uInt16 nDir)
{
    OString sConverted;
    switch (nDir)
    {
    case FRMDIR_HORI_LEFT_TOP:
    case FRMDIR_VERT_TOP_LEFT:
        sConverted = "ltr";
        break;
    case FRMDIR_HORI_RIGHT_TOP:
    case FRMDIR_VERT_TOP_RIGHT:
        sConverted = "rtl";
        break;
    }
    return sConverted;
}

OString SwHTMLWriter::GetIndentString(sal_uInt16 nIncLvl)
{
    OString sRet;

    // etwas umstaendlich, aber wir haben nur einen Indent-String!
    sal_uInt16 nLevel = m_nIndentLvl + nIncLvl;

    if( nLevel && nLevel <= MAX_INDENT_LEVEL)
    {
        sIndentTabs[nLevel] = 0;
        sRet = sIndentTabs;
        sIndentTabs[nLevel] = '\t';
    }

    return sRet;
}

void SwHTMLWriter::OutNewLine( bool bCheck )
{
    if( !bCheck || (Strm().Tell()-m_nLastLFPos) > m_nIndentLvl )
    {
        Strm().WriteCharPtr( SAL_NEWLINE_STRING );
        m_nLastLFPos = Strm().Tell();
    }

    if( m_nIndentLvl && m_nIndentLvl <= MAX_INDENT_LEVEL)
    {
        sIndentTabs[m_nIndentLvl] = 0;
        Strm().WriteCharPtr( sIndentTabs );
        sIndentTabs[m_nIndentLvl] = '\t';
    }
}

sal_uInt16 SwHTMLWriter::GetHTMLFontSize( sal_uInt32 nHeight ) const
{
    sal_uInt16 nSize = 1;
    for( sal_uInt16 i=6; i>0; i-- )
    {
        if( nHeight > (m_aFontHeights[i] + m_aFontHeights[i-1])/2 )
        {
            nSize = i+1;
            break;
        }
    }

    return nSize;
}

// Paragraphs with Table of Contents and other index styles will be typeset with
// dot leaders at the position of the last tabulator in PrintLayout (CSS2) mode
sal_Int32 SwHTMLWriter::indexOfDotLeaders( sal_uInt16 nPoolId, const OUString& rStr )
{
    if (m_bCfgPrintLayout && ((nPoolId >= RES_POOLCOLL_TOX_CNTNT1 && nPoolId <= RES_POOLCOLL_TOX_CNTNT5) ||
        (nPoolId >= RES_POOLCOLL_TOX_IDX1 && nPoolId <= RES_POOLCOLL_TOX_IDX3) ||
        (nPoolId >= RES_POOLCOLL_TOX_USER1 && nPoolId <= RES_POOLCOLL_TOX_CNTNT10) ||
        nPoolId == RES_POOLCOLL_TOX_ILLUS1 || nPoolId == RES_POOLCOLL_TOX_TABLES1 ||
        nPoolId == RES_POOLCOLL_TOX_OBJECT1 ||
        (nPoolId >= RES_POOLCOLL_TOX_AUTHORITIES1 && nPoolId <= RES_POOLCOLL_TOX_USER10))) {
             sal_Int32 i = rStr.lastIndexOf('\t');
             // there are only ASCII (Latin-1) characters after the tabulator
             if (i > -1 && OUStringToOString(rStr.copy(i + 1), RTL_TEXTENCODING_ASCII_US).indexOf('?') == -1)
                 return i;
    }
    return -1;
}

// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
HTMLSaveData::HTMLSaveData(SwHTMLWriter& rWriter, sal_uLong nStt,
                           sal_uLong nEnd, bool bSaveNum,
                           const SwFrameFormat *pFrameFormat)
    : rWrt(rWriter)
    , pOldPam(rWrt.pCurPam)
    , pOldEnd(rWrt.GetEndPaM())
    , pOldNumRuleInfo(nullptr)
    , pOldNextNumRuleInfo(nullptr)
    , nOldDefListLvl(rWrt.m_nDefListLvl)
    , nOldDirection(rWrt.m_nDirection)
    , bOldOutHeader(rWrt.m_bOutHeader)
    , bOldOutFooter(rWrt.m_bOutFooter)
    , bOldOutFlyFrame(rWrt.m_bOutFlyFrame)
{
    bOldWriteAll = rWrt.bWriteAll;

    rWrt.pCurPam = Writer::NewSwPaM( *rWrt.pDoc, nStt, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if( nStt != rWrt.pCurPam->GetMark()->nNode.GetIndex() )
    {
        const SwNode *pNd = rWrt.pDoc->GetNodes()[ nStt ];
        if( pNd->IsTableNode() || pNd->IsSectionNode() )
            rWrt.pCurPam->GetMark()->nNode = nStt;
    }

    rWrt.SetEndPaM( rWrt.pCurPam );
    rWrt.pCurPam->Exchange( );
    rWrt.bWriteAll = true;
    rWrt.m_nDefListLvl = 0;
    rWrt.m_bOutHeader = rWrt.m_bOutFooter = false;

    // Ggf. die aktuelle Numerierungs-Info merken, damit sie wieder
    // neu aufgenommen werden kann. Nur dann belibt auch die Numerierungs-
    // Info des nachsten Absatz gueltig.
    if( bSaveNum )
    {
        pOldNumRuleInfo = new SwHTMLNumRuleInfo( rWrt.GetNumInfo() );
        pOldNextNumRuleInfo = rWrt.GetNextNumInfo();
        rWrt.SetNextNumInfo( nullptr );
    }
    else
    {
        rWrt.ClearNextNumInfo();
    }

    // Die Numerierung wird in jedem Fall unterbrochen.
    rWrt.GetNumInfo().Clear();

    if( pFrameFormat )
        rWrt.m_nDirection = rWrt.GetHTMLDirection( pFrameFormat->GetAttrSet() );
}

HTMLSaveData::~HTMLSaveData()
{
    delete rWrt.pCurPam;                    // Pam wieder loeschen

    rWrt.pCurPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.bWriteAll = bOldWriteAll;
    rWrt.m_nBkmkTabPos = bOldWriteAll ? rWrt.FindPos_Bkmk( *pOldPam->GetPoint() ) : -1;
    rWrt.m_nLastParaToken = 0;
    rWrt.m_nDefListLvl = nOldDefListLvl;
    rWrt.m_nDirection = nOldDirection;
    rWrt.m_bOutHeader = bOldOutHeader;
    rWrt.m_bOutFooter = bOldOutFooter;
    rWrt.m_bOutFlyFrame = bOldOutFlyFrame;

    // Ggf. die Numerierung von vor der Section fortsetzen. Die Numerierung
    // des naecshten Absatz wird in jedem Fall ungueltig.
    if( pOldNumRuleInfo )
    {
        rWrt.GetNumInfo().Set( *pOldNumRuleInfo );
        delete pOldNumRuleInfo;
        rWrt.SetNextNumInfo( pOldNextNumRuleInfo );
    }
    else
    {
        rWrt.GetNumInfo().Clear();
        rWrt.ClearNextNumInfo();
    }
}

void GetHTMLWriter( const OUString&, const OUString& rBaseURL, WriterRef& xRet )
{
    xRet = new SwHTMLWriter( rBaseURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
