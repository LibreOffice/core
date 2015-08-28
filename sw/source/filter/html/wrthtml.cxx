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
    : pHTMLPosFlyFrms(NULL)
    , pNumRuleInfo(new SwHTMLNumRuleInfo)
    , pNextNumRuleInfo(NULL)
    , nHTMLMode(0)
    , eCSS1Unit(FUNIT_NONE)
    , pFootEndNotes(NULL)
    , pxFormComps(NULL)
    , pTemplate(NULL)
    , pDfltColor(NULL)
    , pStartNdIdx(NULL)
    , pCurrPageDesc(NULL)
    , pFormatFootnote(NULL)
    , nWarn(0)
    , nLastLFPos(0)
    , nLastParaToken(0)
    , nBkmkTabPos(-1)
    , nImgMapCnt(1)
    , nFormCntrlCnt(0)
    , nEndNote(0)
    , nFootNote(0)
    , nLeftMargin(0)
    , nDfltLeftMargin(0)
    , nDfltRightMargin(0)
    , nFirstLineIndent(0)
    , nDfltFirstLineIndent(0)
    , nDfltTopMargin(0)
    , nDfltBottomMargin(0)
    , nIndentLvl(0)
    , nWhishLineLen(0)
    , nDefListLvl(0)
    , nDefListMargin(0)
    , nHeaderFooterSpace(0)
    , nTextAttrsToIgnore(0)
    , nExportMode(0)
    , nCSS1OutMode(0)
    , nCSS1Script(CSS1_OUTMODE_WESTERN)
    , nDirection(FRMDIR_HORI_LEFT_TOP)
    , eDestEnc(RTL_TEXTENCODING_MS_1252)
    , eLang(LANGUAGE_DONTKNOW)
    , bCfgOutStyles( false )
    , bCfgPreferStyles( false )
    , bCfgFormFeed( false )
    , bCfgStarBasic( false )
    , bCfgCpyLinkedGrfs( false )
    , bFirstLine(true)
    , bTagOn( false )
    , bTextAttr( false )
    , bOutOpts( false )
    , bOutTable( false )
    , bOutHeader( false )
    , bOutFooter( false )
    , bOutFlyFrame( false )
    , bFirstCSS1Rule( false )
    , bFirstCSS1Property( false )
    , bPoolCollTextModified( false )
    , bCSS1IgnoreFirstPageDesc( false )
    , bNoAlign( false )
    , bClearLeft( false )
    , bClearRight( false )
    , bLFPossible( false )
    , bPreserveForm( false )
    , bCfgNetscape4( false )
    , mbSkipImages(false)
    , mbSkipHeaderFooter(false)
    , bCfgPrintLayout( false )
    , bParaDotLeaders( false )
{
    SetBaseURL(rBaseURL);
}

SwHTMLWriter::~SwHTMLWriter()
{
    delete pNumRuleInfo;
}

void SwHTMLWriter::SetupFilterOptions(SfxMedium& rMedium)
{
    const SfxItemSet* pSet = rMedium.GetItemSet();
    if (pSet == NULL)
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
    aFontHeights[0] = rHtmlOptions.GetFontSize( 0 ) * 20;
    aFontHeights[1] = rHtmlOptions.GetFontSize( 1 ) * 20;
    aFontHeights[2] = rHtmlOptions.GetFontSize( 2 ) * 20;
    aFontHeights[3] = rHtmlOptions.GetFontSize( 3 ) * 20;
    aFontHeights[4] = rHtmlOptions.GetFontSize( 4 ) * 20;
    aFontHeights[5] = rHtmlOptions.GetFontSize( 5 ) * 20;
    aFontHeights[6] = rHtmlOptions.GetFontSize( 6 ) * 20;

    // ueberhaupt Styles ausgeben
    // (dann auch obere und untere Absatz-Abstaende)
    nExportMode = rHtmlOptions.GetExportMode();
    nHTMLMode = GetHtmlMode(0);

    if( HTML_CFG_WRITER == nExportMode || HTML_CFG_NS40 == nExportMode )
        nHTMLMode |= HTMLMODE_BLOCK_SPACER;

    if( HTML_CFG_WRITER == nExportMode || HTML_CFG_MSIE == nExportMode )
        nHTMLMode |= (HTMLMODE_FLOAT_FRAME | HTMLMODE_LSPACE_IN_NUMBUL);

    if( HTML_CFG_MSIE == nExportMode )
        nHTMLMode |= HTMLMODE_NBSP_IN_TABLES;

    if( HTML_CFG_WRITER == nExportMode || HTML_CFG_NS40 == nExportMode || HTML_CFG_MSIE == nExportMode )
        nHTMLMode |= HTMLMODE_ABS_POS_FLY | HTMLMODE_ABS_POS_DRAW;

    if( HTML_CFG_WRITER == nExportMode )
        nHTMLMode |= HTMLMODE_FLY_MARGINS;

    if( HTML_CFG_NS40 == nExportMode )
        nHTMLMode |= HTMLMODE_BORDER_NONE;

    nHTMLMode |= HTMLMODE_FONT_GENERIC;

    if( HTML_CFG_NS40==nExportMode )
        nHTMLMode |= HTMLMODE_NO_CONTROL_CENTERING;

    bCfgOutStyles = IsHTMLMode(HTMLMODE_SOME_STYLES | HTMLMODE_FULL_STYLES);
    bCfgNetscape4 = (HTML_CFG_NS40 == nExportMode);

    if( IsHTMLMode(HTMLMODE_SOME_STYLES | HTMLMODE_FULL_STYLES) )
        nHTMLMode |= HTMLMODE_PRINT_EXT;

    const sal_Char *pHelpHack = getenv( "HelpEx" );
    if( pHelpHack )
    {
        OString aTmp(pHelpHack);
        if (aTmp.equalsIgnoreAsciiCase("Hilfe"))
            nHTMLMode |= HTMLMODE_NO_BR_AT_PAREND;
    }

    eCSS1Unit = (FieldUnit)SW_MOD()->GetMetric( pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) );

    bool bWriteUTF8 = bWriteClipboardDoc;
    eDestEnc = bWriteUTF8 ? RTL_TEXTENCODING_UTF8 : rHtmlOptions.GetTextEncoding();
    const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );
    eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    // Nur noch fuer den MS-IE ziehen wir den Export von Styles vor.
    bCfgPreferStyles = HTML_CFG_MSIE == nExportMode;

    bCfgStarBasic = rHtmlOptions.IsStarBasic();

    bCfgFormFeed = !IsHTMLMode(HTMLMODE_PRINT_EXT);
    bCfgCpyLinkedGrfs = rHtmlOptions.IsSaveGraphicsLocal();

    bCfgPrintLayout = rHtmlOptions.IsPrintLayoutExtension();

    // die HTML-Vorlage holen
    bool bOldHTMLMode = false;
    SwTextFormatColls::size_type nOldTextFormatCollCnt = 0;
    SwCharFormats::size_type nOldCharFormatCnt = 0;

    OSL_ENSURE( !pTemplate, "Wo kommt denn die HTML-Vorlage hier her?" );
    pTemplate = static_cast<HTMLReader*>(ReadHTML)->GetTemplateDoc();
    if( pTemplate )
    {
        pTemplate->acquire();
        bOldHTMLMode = pTemplate->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE);
        pTemplate->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, true);

        nOldTextFormatCollCnt = pTemplate->GetTextFormatColls()->size();
        nOldCharFormatCnt = pTemplate->GetCharFormats()->size();
    }

    if( bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, pDoc->GetNodes().Count(),
                         pDoc->GetDocShell());

    pDfltColor = 0;
    pFootEndNotes = 0;
    pFormatFootnote = 0;
    bOutTable = bOutHeader = bOutFooter = bOutFlyFrame = false;
    pxFormComps = 0;
    nFormCntrlCnt = 0;
    bPreserveForm = false;
    bClearLeft = bClearRight = false;
    bLFPossible = false;

    nLeftMargin = nDfltLeftMargin = nDfltRightMargin = 0;
    nDfltTopMargin = nDfltBottomMargin = 0;
    nFirstLineIndent = nDfltFirstLineIndent = 0;
    bPoolCollTextModified = false;
    bFirstCSS1Property = bFirstCSS1Rule = false;
    bCSS1IgnoreFirstPageDesc = false;
    nIndentLvl = 0;
    nWhishLineLen = 70;
    nLastLFPos = 0;
    nDefListLvl = 0;
    nDefListMargin = ((pTemplate && !bCfgOutStyles) ? pTemplate : pDoc)
        ->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_HTML_DD, false )
        ->GetLRSpace().GetTextLeft();
    nHeaderFooterSpace = 0;
    nTextAttrsToIgnore = 0;
    nCSS1OutMode = 0;
    SvtScriptType nScript = SvtLanguageOptions::GetScriptTypeOfLanguage( GetAppLanguage() );
    switch( nScript )
    {
    case SvtScriptType::ASIAN:
        nCSS1Script = CSS1_OUTMODE_CJK;
        break;
    case SvtScriptType::COMPLEX:
        nCSS1Script = CSS1_OUTMODE_CTL;
        break;
    default:
        nCSS1Script = CSS1_OUTMODE_WESTERN;
        break;
    }
    eLang = static_cast<const SvxLanguageItem&>(pDoc
            ->GetDefault(GetLangWhichIdFromScript(nCSS1Script))).GetLanguage();

    nFootNote = nEndNote = 0;

    nWarn = 0;
    GetNumInfo().Clear();
    pNextNumRuleInfo = 0;

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
        pStartNdIdx = new SwNodeIndex( pCurPam->GetPoint()->nNode );

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
                    pSNd->GetSection().GetSectionName(), eDestEnc,
                    &aNonConvertableCharacters );

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
    pHTMLPosFlyFrms = 0;
    CollectFlyFrms();
    nLastParaToken = 0;
    GetControls();
    CollectLinkTargets();

    sal_uInt16 nHeaderAttrs = 0;
    pCurrPageDesc = MakeHeader( nHeaderAttrs );

    bLFPossible = true;

    // Formulare, die nur HiddenControls enthalten ausgeben.
    OutHiddenForms();

    if( !aStartTags.isEmpty() )
        Strm().WriteCharPtr( aStartTags.getStr() );

    const SfxPoolItem *pItem;
    const SfxItemSet& rPageItemSet = pCurrPageDesc->GetMaster().GetAttrSet();
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

    nTextAttrsToIgnore = nHeaderAttrs;
    Out_SwDoc( pOrigPam );
    nTextAttrsToIgnore = 0;

    if( pxFormComps && pxFormComps->is() )
        OutForm( false, *pxFormComps );

    if( pFootEndNotes )
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

    if( bLFPossible )
        OutNewLine();
    if (!mbSkipHeaderFooter)
    {
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_body, false );
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_html, false );
    }

    // loesche die Tabelle mit den freifliegenden Rahmen
    OSL_ENSURE( !pHTMLPosFlyFrms, "Wurden nicht alle Rahmen ausgegeben" );
    if( pHTMLPosFlyFrms )
    {
        pHTMLPosFlyFrms->DeleteAndDestroyAll();
        delete pHTMLPosFlyFrms;
        pHTMLPosFlyFrms = 0;
    }

    aHTMLControls.DeleteAndDestroyAll();

    if (!m_CharFormatInfos.empty())
        m_CharFormatInfos.clear();

    if (!m_TextCollInfos.empty())
        m_TextCollInfos.clear();

    if(!aImgMapNames.empty())
        aImgMapNames.clear();

    aImplicitMarks.clear();

    aOutlineMarks.clear();

    aOutlineMarkPoss.clear();

    aNumRuleNames.clear();

    aScriptParaStyles.clear();
    aScriptTextStyles.clear();

    delete pDfltColor;
    pDfltColor = 0;

    delete pStartNdIdx;
    pStartNdIdx = 0;

    delete pxFormComps;
    pxFormComps = 0;

    OSL_ENSURE( !pFootEndNotes,
            "SwHTMLWriter::Write: Footnotes nicht durch OutFootEndNotes geloescht" );

    pCurrPageDesc = 0;

    ClearNextNumInfo();

    for( int i=0; i<MAXLEVEL; ++i )
        aBulletGrfs[i].clear();

    aNonConvertableCharacters.clear();

    if( bShowProgress )
        ::EndProgress( pDoc->GetDocShell() );

    if( pTemplate )
    {
        // Waehrend des Exports angelegte Zeichen- und Abastzvorlagen
        // loeschen
        auto nTextFormatCollCnt = pTemplate->GetTextFormatColls()->size();
        while( nTextFormatCollCnt > nOldTextFormatCollCnt )
            pTemplate->DelTextFormatColl( --nTextFormatCollCnt );
        OSL_ENSURE( pTemplate->GetTextFormatColls()->size() == nOldTextFormatCollCnt,
                "falsche Anzahl TextFormatColls geloescht" );

        auto nCharFormatCnt = pTemplate->GetCharFormats()->size();
        while( nCharFormatCnt > nOldCharFormatCnt )
            pTemplate->DelCharFormat( --nCharFormatCnt );
        OSL_ENSURE( pTemplate->GetCharFormats()->size() == nOldCharFormatCnt,
                "falsche Anzahl CharFormats geloescht" );

        // HTML-Modus wieder restaurieren
        pTemplate->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, bOldHTMLMode);

        if( 0 == pTemplate->release() )
            delete pTemplate;

        pTemplate = 0;
    }

    return nWarn;
}

static const SwFormatCol *lcl_html_GetFormatCol( const SwSection& rSection,
                                       const SwSectionFormat& rFormat )
{
    const SwFormatCol *pCol = 0;

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

    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_division);

    const OUString& rName = rSection.GetSectionName();
    if( !rName.isEmpty() && !bContinued )
    {
        sOut.append(" " + OString(OOO_STRING_SVTOOLS_HTML_O_id) + "=\"");
        rHTMLWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), rName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
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
                                  rHTMLWrt.eDestEnc,
                                  &rHTMLWrt.aNonConvertableCharacters );
        const sal_Char* pDelim = "&#255;";
        if( !aFilter.isEmpty() || !aSection.isEmpty() || bURLContainsDelim )
            rHTMLWrt.Strm().WriteCharPtr( pDelim );
        if( !aFilter.isEmpty() )
            HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aFilter, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
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
                                      rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
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
    if( rHTMLWrt.IsHTMLMode( rHTMLWrt.bCfgOutStyles ? HTMLMODE_ON : 0 ) )
        rHTMLWrt.OutCSS1_SectionFormatOptions( rFormat, pCol );

    rHTMLWrt.Strm().WriteChar( '>' );

    rHTMLWrt.bLFPossible = true;
    if( !rName.isEmpty() && !bContinued )
        rHTMLWrt.OutImplicitMark( rName, "region" );

    rHTMLWrt.IncIndentLevel();
}

static void lcl_html_OutSectionEndTag( SwHTMLWriter& rHTMLWrt )
{
    rHTMLWrt.DecIndentLevel();
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), OOO_STRING_SVTOOLS_HTML_division, false );
    rHTMLWrt.bLFPossible = true;
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
    const SwSectionFormat *pSurrFormat = 0;
    const SwSectionNode *pSurrSectNd = 0;
    const SwSection *pSurrSection = 0;
    const SwFormatCol *pSurrCol = 0;

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
    nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;

    // gebe alle Bereiche des Pams in das HTML-File aus.
    do {
        bWriteAll = bSaveWriteAll;
        bFirstLine = true;

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

                if( !bFirstLine )
                    pCurPam->GetPoint()->nContent.Assign( pTextNd, 0 );

                OutHTML_SwTextNode( *this, *pTextNd );
            }
            else if( rNd.IsTableNode() )
            {
                OutHTML_SwTableNode( *this, *rNd.GetTableNode(), 0 );
                nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;
            }
            else if( rNd.IsSectionNode() )
            {
                OutHTML_Section( *this, *rNd.GetSectionNode() );
                nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;
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
            bFirstLine = false;
            bOutFooter = false; // Nach einem Node keine Fusszeile mehr
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
    const SwFormat *pRefFormat = 0;

    if( rHWrt.pTemplate )
        pRefFormat = SwHTMLWriter::GetTemplateFormat( pFormat->GetPoolFormatId(),
                                                &rHWrt.pTemplate->getIDocumentStylePoolAccess() );

    const SvxColorItem *pColorItem = 0;

    const SfxItemSet& rItemSet = pFormat->GetAttrSet();
    const SfxPoolItem *pRefItem = 0, *pItem = 0;
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
        HTMLOutFuncs::Out_Color( rHWrt.Strm(), aColor, rHWrt.eDestEnc );
        if( RES_POOLCOLL_STANDARD==pFormat->GetPoolFormatId() )
            rHWrt.pDfltColor = new Color( aColor );
    }
}

sal_uInt16 SwHTMLWriter::OutHeaderAttrs()
{
    sal_uLong nIdx = pCurPam->GetPoint()->nNode.GetIndex();
    sal_uLong nEndIdx = pCurPam->GetMark()->nNode.GetIndex();

    SwTextNode *pTextNd = 0;
    while( nIdx<=nEndIdx &&
        0==(pTextNd=pDoc->GetNodes()[nIdx]->GetTextNode()) )
        nIdx++;

    OSL_ENSURE( pTextNd, "Kein Text-Node gefunden" );
    if( !pTextNd || !pTextNd->HasHints() )
        return 0;

    sal_uInt16 nAttrs = 0;
    const size_t nCntAttr = pTextNd->GetSwpHints().Count();
    sal_Int32 nOldPos = 0;
    for( size_t i=0; i<nCntAttr; ++i )
    {
        const SwTextAttr *pHt = pTextNd->GetSwpHints()[i];
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
                                         sIndent.getStr(), eDestEnc,
                                         &aNonConvertableCharacters );

        // Kommentare und Meta-Tags des ersten Absatzes
        rHeaderAttrs = OutHeaderAttrs();

        OutFootEndNoteInfo();
    }

    const SwPageDesc *pPageDesc = 0;

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
        if( bCfgOutStyles )
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
        OutLanguage( eLang );

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

        nDirection = GetHTMLDirection( rItemSet );
        OutDirection( nDirection );

        if( bCfgOutStyles )
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
    HTMLOutFuncs::Out_String( Strm(), rName, eDestEnc, &aNonConvertableCharacters ).WriteCharPtr( "\">" );
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_anchor, false );
}

void SwHTMLWriter::OutBookmarks()
{
    // hole das aktuelle Bookmark
    const ::sw::mark::IMark* pBookmark = NULL;
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    if(nBkmkTabPos != -1)
        pBookmark = (pMarkAccess->getAllMarksBegin() + nBkmkTabPos)->get();
    // Ausgabe aller Bookmarks in diesem Absatz. Die Content-Position
    // wird vorerst nicht beruecksichtigt!
    sal_uInt32 nNode = pCurPam->GetPoint()->nNode.GetIndex();
    while( nBkmkTabPos != -1
           && pBookmark->GetMarkPos().nNode.GetIndex() == nNode )
    {
        // Der Bereich derBookmark wird erstam ignoriert, da er von uns
        // auch nicht eingelesen wird.

        // erst die SWG spezifischen Daten:
        if ( dynamic_cast< const ::sw::mark::IBookmark* >(pBookmark) && !pBookmark->GetName().isEmpty() )
        {
            OutAnchor( pBookmark->GetName() );
        }

        if( ++nBkmkTabPos >= pMarkAccess->getAllMarksCount() )
            nBkmkTabPos = -1;
        else
            pBookmark = (pMarkAccess->getAllMarksBegin() + nBkmkTabPos)->get();
    }

    sal_uInt32 nPos;
    for( nPos = 0; nPos < aOutlineMarkPoss.size() &&
                   aOutlineMarkPoss[nPos] < nNode; nPos++ )
        ;

    while( nPos < aOutlineMarkPoss.size() && aOutlineMarkPoss[nPos] == nNode )
    {
        OUString sMark( aOutlineMarks[nPos] );
        OutAnchor( sMark.replace('?', '_') ); // '?' causes problems in IE/Netscape 5
        aOutlineMarkPoss.erase( aOutlineMarkPoss.begin()+nPos );
        aOutlineMarks.erase( aOutlineMarks.begin() + nPos );
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
    if( !rMark.isEmpty() && !aImplicitMarks.empty() )
    {
        OUString sMark(rMark + OUStringLiteral1<cMarkSeparator>() + OUString::createFromAscii(pMarkType));
        if( 0 != aImplicitMarks.erase( sMark ) )
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
    HTMLOutFuncs::Out_String( Strm(), sURL, eDestEnc, &aNonConvertableCharacters );
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
        HTMLOutFuncs::Out_Color( Strm(), rBackColor, eDestEnc);
    }

    if( !bGraphic )
        return;

    OUString aGraphicInBase64;
    const Graphic* pGrf = pBrushItem->GetGraphic();
    if( pGrf )
    {
        sal_uLong nErr = XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64);
        if( nErr )
        {
            nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
        }
        Strm().WriteCharPtr( " " OOO_STRING_SVTOOLS_HTML_O_background "=\"" );
        Strm().WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_data ":" );
        HTMLOutFuncs::Out_String( Strm(), aGraphicInBase64, eDestEnc, &aNonConvertableCharacters ).WriteChar( '\"' );
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
                                  eDestEnc, &aNonConvertableCharacters ).WriteChar( '"' );
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
        nDir = nDirection;
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
    sal_uInt16 nLevel = nIndentLvl + nIncLvl;

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
    if( !bCheck || (Strm().Tell()-nLastLFPos) > nIndentLvl )
    {
        Strm().WriteCharPtr( SAL_NEWLINE_STRING );
        nLastLFPos = Strm().Tell();
    }

    if( nIndentLvl && nIndentLvl <= MAX_INDENT_LEVEL)
    {
        sIndentTabs[nIndentLvl] = 0;
        Strm().WriteCharPtr( sIndentTabs );
        sIndentTabs[nIndentLvl] = '\t';
    }
}

sal_uInt16 SwHTMLWriter::GetHTMLFontSize( sal_uInt32 nHeight ) const
{
    sal_uInt16 nSize = 1;
    for( sal_uInt16 i=6; i>0; i-- )
    {
        if( nHeight > (aFontHeights[i] + aFontHeights[i-1])/2 )
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
    if (bCfgPrintLayout && ((nPoolId >= RES_POOLCOLL_TOX_CNTNT1 && nPoolId <= RES_POOLCOLL_TOX_CNTNT5) ||
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
    , pOldNumRuleInfo(0)
    , pOldNextNumRuleInfo(0)
    , nOldDefListLvl(rWrt.nDefListLvl)
    , nOldDirection(rWrt.nDirection)
    , bOldOutHeader(rWrt.bOutHeader)
    , bOldOutFooter(rWrt.bOutFooter)
    , bOldOutFlyFrame(rWrt.bOutFlyFrame)
    , pOldFlyFormat(NULL)
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
    rWrt.nDefListLvl = 0;
    rWrt.bOutHeader = rWrt.bOutFooter = false;

    // Ggf. die aktuelle Numerierungs-Info merken, damit sie wieder
    // neu aufgenommen werden kann. Nur dann belibt auch die Numerierungs-
    // Info des nachsten Absatz gueltig.
    if( bSaveNum )
    {
        pOldNumRuleInfo = new SwHTMLNumRuleInfo( rWrt.GetNumInfo() );
        pOldNextNumRuleInfo = rWrt.GetNextNumInfo();
        rWrt.SetNextNumInfo( 0 );
    }
    else
    {
        rWrt.ClearNextNumInfo();
    }

    // Die Numerierung wird in jedem Fall unterbrochen.
    rWrt.GetNumInfo().Clear();

    if( pFrameFormat )
        rWrt.nDirection = rWrt.GetHTMLDirection( pFrameFormat->GetAttrSet() );
}

HTMLSaveData::~HTMLSaveData()
{
    delete rWrt.pCurPam;                    // Pam wieder loeschen

    rWrt.pCurPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.bWriteAll = bOldWriteAll;
    rWrt.nBkmkTabPos = bOldWriteAll ? rWrt.FindPos_Bkmk( *pOldPam->GetPoint() ) : -1;
    rWrt.nLastParaToken = 0;
    rWrt.nDefListLvl = nOldDefListLvl;
    rWrt.nDirection = nOldDirection;
    rWrt.bOutHeader = bOldOutHeader;
    rWrt.bOutFooter = bOldOutFooter;
    rWrt.bOutFlyFrame = bOldOutFlyFrame;

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
