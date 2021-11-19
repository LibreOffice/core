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
#include <svl/languageoptions.hxx>
#include <rtl/tencinfo.h>
#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>

#include <svtools/htmlcfg.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <vcl/svapp.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/frmdiritem.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XModel.hpp>
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
#include "wrthtml.hxx"
#include "htmlnum.hxx"
#include "htmlfly.hxx"
#include <swmodule.hxx>
#include <strings.hrc>
#include <swerror.h>
#include <rtl/strbuf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <xmloff/odffields.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/sequenceashashmap.hxx>

#define MAX_INDENT_LEVEL 20

using namespace css;

static char sIndentTabs[MAX_INDENT_LEVEL+2] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

SwHTMLWriter::SwHTMLWriter( const OUString& rBaseURL, const OUString& rFilterOptions )
    : m_pNumRuleInfo(new SwHTMLNumRuleInfo)
    , m_nHTMLMode(0)
    , m_eCSS1Unit(FieldUnit::NONE)
    , mxFormComps()
    , m_pStartNdIdx(nullptr)
    , m_pCurrPageDesc(nullptr)
    , m_pFormatFootnote(nullptr)
    , m_nWarn(0)
    , m_nLastLFPos(0)
    , m_nLastParaToken(HtmlTokenId::NONE)
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
    , m_nWishLineLen(70)
    , m_nDefListLvl(0)
    , m_nDefListMargin(0)
    , m_nHeaderFooterSpace(0)
    , m_nTextAttrsToIgnore(0)
    , m_nExportMode(0)
    , m_nCSS1OutMode(0)
    , m_nCSS1Script(CSS1_OUTMODE_WESTERN)
    , m_nDirection(SvxFrameDirection::Horizontal_LR_TB)
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
    , m_bCSS1IgnoreFirstPageDesc( false )
    , m_bNoAlign( false )
    , m_bClearLeft( false )
    , m_bClearRight( false )
    , m_bLFPossible( false )
    , m_bPreserveForm( false )
    , m_bCfgNetscape4( false )
    , mbSkipImages(false)
    , mbSkipHeaderFooter(false)
    , mbEmbedImages(false)
    , m_bCfgPrintLayout( false )
    , m_bParaDotLeaders( false )
{
    SetBaseURL(rBaseURL);

    if (rBaseURL.isEmpty())
    {
        // Paste: set base URL to a tempfile, so images are not lost.
        mpTempBaseURL.reset(new utl::TempFile());
        mpTempBaseURL->EnableKillingFile();
        SetBaseURL(mpTempBaseURL->GetURL());
    }

    SetupFilterOptions(rFilterOptions);

    if (mbXHTML)
    {
        m_bNoAlign = true;
    }
}

SwHTMLWriter::~SwHTMLWriter()
{
}

std::unique_ptr<SwHTMLNumRuleInfo> SwHTMLWriter::ReleaseNextNumInfo()
{
    return std::move(m_pNextNumRuleInfo);
}

void SwHTMLWriter::SetupFilterOptions(SfxMedium& rMedium)
{
    const SfxItemSet* pSet = rMedium.GetItemSet();
    if (pSet == nullptr)
        return;

    const SfxPoolItem* pItem;
    if (pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) != SfxItemState::SET)
        return;


    const OUString sFilterOptions = static_cast<const SfxStringItem*>(pItem)->GetValue();
    SetupFilterOptions(sFilterOptions);

    comphelper::SequenceAsHashMap aStoreMap(rMedium.GetArgs());
    auto it = aStoreMap.find("RTFOLEMimeType");
    if (it == aStoreMap.end())
    {
        return;
    }

    it->second >>= m_aRTFOLEMimeType;
}

void SwHTMLWriter::SetupFilterOptions(const OUString& rFilterOptions)
{
    if (rFilterOptions.indexOf("SkipImages") >= 0)
    {
        mbSkipImages = true;
    }
    else if (rFilterOptions.indexOf("SkipHeaderFooter") >= 0)
    {
        mbSkipHeaderFooter = true;
    }
    else if (rFilterOptions.indexOf("EmbedImages") >= 0)
    {
        mbEmbedImages = true;
    }

    // this option can be "on" together with any of above
    if (rFilterOptions.indexOf("NoLineLimit") >= 0)
    {
        m_nWishLineLen = -1;
    }

    const uno::Sequence<OUString> aOptionSeq = comphelper::string::convertCommaSeparated(rFilterOptions);
    const OUString aXhtmlNsKey("xhtmlns=");
    for (const auto& rOption : aOptionSeq)
    {
        if (rOption == "XHTML")
            mbXHTML = true;
        else if (rOption.startsWith(aXhtmlNsKey))
        {
            maNamespace = rOption.copy(aXhtmlNsKey.getLength()).toUtf8();
            if (maNamespace == "reqif-xhtml")
            {
                mbReqIF = true;
                // XHTML is always just a fragment inside ReqIF.
                mbSkipHeaderFooter = true;
            }
            // XHTML namespace implies XHTML.
            mbXHTML = true;
        }
    }
}

ErrCode SwHTMLWriter::WriteStream()
{
    // Intercept paste output if requested.
    char* pPasteEnv = getenv("SW_DEBUG_HTML_PASTE_TO");
    std::unique_ptr<SvStream> pPasteStream;
    SvStream* pOldPasteStream = nullptr;
    if (pPasteEnv)
    {
        OUString aPasteStr;
        if (osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pPasteEnv), aPasteStr)
                   == osl::FileBase::E_None)
        {
            pPasteStream.reset(new SvFileStream(aPasteStr, StreamMode::WRITE));
            pOldPasteStream = &Strm();
            SetStream(pPasteStream.get());
        }
    }
    comphelper::ScopeGuard g([this, pOldPasteStream] { this->SetStream(pOldPasteStream); });

    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();

    // font heights 1-7
    m_aFontHeights[0] = rHtmlOptions.GetFontSize( 0 ) * 20;
    m_aFontHeights[1] = rHtmlOptions.GetFontSize( 1 ) * 20;
    m_aFontHeights[2] = rHtmlOptions.GetFontSize( 2 ) * 20;
    m_aFontHeights[3] = rHtmlOptions.GetFontSize( 3 ) * 20;
    m_aFontHeights[4] = rHtmlOptions.GetFontSize( 4 ) * 20;
    m_aFontHeights[5] = rHtmlOptions.GetFontSize( 5 ) * 20;
    m_aFontHeights[6] = rHtmlOptions.GetFontSize( 6 ) * 20;

    // output styles anyway
    // (then also top and bottom paragraph spacing)
    m_nExportMode = rHtmlOptions.GetExportMode();
    m_nHTMLMode = GetHtmlMode(nullptr);

    if( HTML_CFG_WRITER == m_nExportMode || HTML_CFG_NS40 == m_nExportMode )
        m_nHTMLMode |= HTMLMODE_BLOCK_SPACER;

    if( HTML_CFG_WRITER == m_nExportMode || HTML_CFG_MSIE == m_nExportMode )
        m_nHTMLMode |= (HTMLMODE_FLOAT_FRAME | HTMLMODE_LSPACE_IN_NUMBER_BULLET);

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

    m_eCSS1Unit = SW_MOD()->GetMetric( m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) );

    bool bWriteUTF8 = m_bWriteClipboardDoc;
    m_eDestEnc = bWriteUTF8 ? RTL_TEXTENCODING_UTF8 : rHtmlOptions.GetTextEncoding();
    const char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( m_eDestEnc );
    m_eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    // Only for the MS-IE we favour the export of styles.
    m_bCfgPreferStyles = HTML_CFG_MSIE == m_nExportMode;

    m_bCfgStarBasic = rHtmlOptions.IsStarBasic();

    m_bCfgFormFeed = !IsHTMLMode(HTMLMODE_PRINT_EXT);
    m_bCfgCpyLinkedGrfs = rHtmlOptions.IsSaveGraphicsLocal();

    m_bCfgPrintLayout = rHtmlOptions.IsPrintLayoutExtension();

    // get HTML template
    bool bOldHTMLMode = false;
    SwTextFormatColls::size_type nOldTextFormatCollCnt = 0;
    SwCharFormats::size_type nOldCharFormatCnt = 0;

    OSL_ENSURE( !m_xTemplate.is(), "Where is the HTML template coming from?" );
    m_xTemplate = static_cast<HTMLReader*>(ReadHTML)->GetTemplateDoc(*m_pDoc);
    if( m_xTemplate.is() )
    {
        bOldHTMLMode = m_xTemplate->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE);
        m_xTemplate->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, true);

        nOldTextFormatCollCnt = m_xTemplate->GetTextFormatColls()->size();
        nOldCharFormatCnt = m_xTemplate->GetCharFormats()->size();
    }

    if( m_bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, m_pDoc->GetNodes().Count(),
                         m_pDoc->GetDocShell());

    m_xDfltColor.reset();
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
    m_bFirstCSS1Property = m_bFirstCSS1Rule = false;
    m_bCSS1IgnoreFirstPageDesc = false;
    m_nIndentLvl = 0;
    m_nLastLFPos = 0;
    m_nDefListLvl = 0;
    m_nDefListMargin = ((m_xTemplate.is() && !m_bCfgOutStyles) ? m_xTemplate.get() : m_pDoc)
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
    m_eLang = static_cast<const SvxLanguageItem&>(m_pDoc
            ->GetDefault(GetLangWhichIdFromScript(m_nCSS1Script))).GetLanguage();

    m_nFootNote = m_nEndNote = 0;

    m_nWarn = ERRCODE_NONE;
    GetNumInfo().Clear();
    m_pNextNumRuleInfo = nullptr;

    OString aStartTags;

    // respect table and section at document beginning
    {
        SwTableNode * pTNd = m_pCurrentPam->GetNode().FindTableNode();
        if( pTNd && m_bWriteAll )
        {
            // start with table node !!
            m_pCurrentPam->GetPoint()->nNode = *pTNd;

            if( m_bWriteOnlyFirstTable )
                m_pCurrentPam->GetMark()->nNode = *pTNd->EndOfSectionNode();
        }

        // first node (with can contain a page break)
        m_pStartNdIdx = new SwNodeIndex( m_pCurrentPam->GetPoint()->nNode );

        SwSectionNode * pSNd = m_pCurrentPam->GetNode().FindSectionNode();
        while( pSNd )
        {
            if( m_bWriteAll )
            {
                // start with section node !!
                m_pCurrentPam->GetPoint()->nNode = *pSNd;
            }
            else
            {
                OSL_ENSURE( SectionType::FileLink != pSNd->GetSection().GetType(),
                        "Export linked areas at document beginning is not implemented" );

                // save only the tag of section
                OString aName = HTMLOutFuncs::ConvertStringToHTML(
                    pSNd->GetSection().GetSectionName(), m_eDestEnc,
                    &m_aNonConvertableCharacters );

                aStartTags =
                    "<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_division
                    " " OOO_STRING_SVTOOLS_HTML_O_id
                    "=\"" + aName + "\">" +
                    aStartTags;
            }
            // FindSectionNode() on a SectionNode return the same!
            pSNd = pSNd->StartOfSectionNode()->FindSectionNode();
        }
    }

    // create table of the floating frames, but only when the whole
    // document is saved
    m_pHTMLPosFlyFrames = nullptr;
    CollectFlyFrames();
    m_nLastParaToken = HtmlTokenId::NONE;
    GetControls();
    CollectLinkTargets();

    sal_uInt16 nHeaderAttrs = 0;
    m_pCurrPageDesc = MakeHeader( nHeaderAttrs );

    m_bLFPossible = true;

    // output forms which contain only HiddenControls
    OutHiddenForms();

    if( !aStartTags.isEmpty() )
        Strm().WriteOString( aStartTags );

    const SfxPoolItem *pItem;
    const SfxItemSet& rPageItemSet = m_pCurrPageDesc->GetMaster().GetAttrSet();
    if( !m_bWriteClipboardDoc && m_pDoc->GetDocShell() &&
         (!m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) &&
          !m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE)) &&
        SfxItemState::SET == rPageItemSet.GetItemState( RES_HEADER, true, &pItem) )
    {
        const SwFrameFormat *pHeaderFormat =
            static_cast<const SwFormatHeader *>(pItem)->GetHeaderFormat();
        if( pHeaderFormat )
            OutHTML_HeaderFooter( *this, *pHeaderFormat, true );
    }

    m_nTextAttrsToIgnore = nHeaderAttrs;
    Out_SwDoc( m_pOrigPam );
    m_nTextAttrsToIgnore = 0;

    if( mxFormComps.is() )
        OutForm( false, mxFormComps );

    if( m_pFootEndNotes )
        OutFootEndNotes();

    if( !m_bWriteClipboardDoc && m_pDoc->GetDocShell() &&
        (!m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) && !m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE))  &&
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
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_body, false );
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_html, false );
    }
    else if (mbReqIF)
    {
        // ReqIF: end xhtml.BlkStruct.class.
        HTMLOutFuncs::Out_AsciiTag(Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_division, false);
    }

    // delete the table with floating frames
    OSL_ENSURE( !m_pHTMLPosFlyFrames, "Were not all frames output?" );
    m_pHTMLPosFlyFrames.reset();

    m_aHTMLControls.clear();

    m_CharFormatInfos.clear();
    m_TextCollInfos.clear();
    m_aImgMapNames.clear();
    m_aImplicitMarks.clear();
    m_aOutlineMarks.clear();
    m_aOutlineMarkPoss.clear();
    m_aNumRuleNames.clear();
    m_aScriptParaStyles.clear();
    m_aScriptTextStyles.clear();

    m_xDfltColor.reset();

    delete m_pStartNdIdx;
    m_pStartNdIdx = nullptr;

    mxFormComps.clear();

    OSL_ENSURE( !m_pFootEndNotes,
            "SwHTMLWriter::Write: Footnotes not deleted by OutFootEndNotes" );

    m_pCurrPageDesc = nullptr;

    ClearNextNumInfo();

    for(OUString & s : m_aBulletGrfs)
        s.clear();

    m_aNonConvertableCharacters.clear();

    if( m_bShowProgress )
        ::EndProgress( m_pDoc->GetDocShell() );

    if( m_xTemplate.is() )
    {
        // delete character and paragraph templates created during export
        auto nTextFormatCollCnt = m_xTemplate->GetTextFormatColls()->size();
        while( nTextFormatCollCnt > nOldTextFormatCollCnt )
            m_xTemplate->DelTextFormatColl( --nTextFormatCollCnt );
        OSL_ENSURE( m_xTemplate->GetTextFormatColls()->size() == nOldTextFormatCollCnt,
                "wrong number of TextFormatColls deleted" );

        auto nCharFormatCnt = m_xTemplate->GetCharFormats()->size();
        while( nCharFormatCnt > nOldCharFormatCnt )
            m_xTemplate->DelCharFormat( --nCharFormatCnt );
        OSL_ENSURE( m_xTemplate->GetCharFormats()->size() == nOldCharFormatCnt,
                "wrong number of CharFormats deleted" );

        // restore HTML mode
        m_xTemplate->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, bOldHTMLMode);

        m_xTemplate.clear();
    }

    return m_nWarn;
}

static const SwFormatCol *lcl_html_GetFormatCol( const SwSection& rSection,
                                       const SwSectionFormat& rFormat )
{
    const SwFormatCol *pCol = nullptr;

    const SfxPoolItem* pItem;
    if( SectionType::FileLink != rSection.GetType() &&
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
        rHTMLWrt.m_pDoc->GetNodes()[nIndex]->GetSectionNode();
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
    const SwEndNode *pEndNd = rHTMLWrt.m_pDoc->GetNodes()[nIndex]->GetEndNode();
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
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_id "=\"");
        rHTMLWrt.Strm().WriteOString( sOut.makeStringAndClear() );
        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), rName, rHTMLWrt.m_eDestEnc, &rHTMLWrt.m_aNonConvertableCharacters );
        sOut.append('\"');
    }

    SvxFrameDirection nDir = rHTMLWrt.GetHTMLDirection( rFormat.GetAttrSet() );
    rHTMLWrt.Strm().WriteOString( sOut.makeStringAndClear() );
    rHTMLWrt.OutDirection( nDir );

    if( SectionType::FileLink == rSection.GetType() )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_href "=\"");
        rHTMLWrt.Strm().WriteOString( sOut.makeStringAndClear() );

        const OUString& aFName = rSection.GetLinkFileName();
        sal_Int32 nIdx{ 0 };
        OUString aURL( aFName.getToken(0, sfx2::cTokenSeparator, nIdx) );
        OUString aFilter( aFName.getToken(0, sfx2::cTokenSeparator, nIdx) );
        OUString aSection( aFName.getToken(0, sfx2::cTokenSeparator, nIdx) );

        OUString aEncURL( URIHelper::simpleNormalizedMakeRelative(rHTMLWrt.GetBaseURL(), aURL ) );
        sal_Unicode cDelim = 255U;
        bool bURLContainsDelim = (-1 != aEncURL.indexOf( cDelim ) );

        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aEncURL,
                                  rHTMLWrt.m_eDestEnc,
                                  &rHTMLWrt.m_aNonConvertableCharacters );
        const char* const pDelim = "&#255;";
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
                nGutter = static_cast<sal_uInt16>(Application::GetDefaultDevice()
                                ->LogicToPixel( Size(nGutter, 0), MapMode(MapUnit::MapTwip) ).Width());
            }
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_gutter "=\"" + OString::number(nGutter) + "\"");
        }
    }

    rHTMLWrt.Strm().WriteOString( sOut.makeStringAndClear() );
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
    HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_division, false );
    rHTMLWrt.m_bLFPossible = true;
}

static Writer& OutHTML_Section( Writer& rWrt, const SwSectionNode& rSectNd )
{
    SwHTMLWriter& rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);

    // End <PRE> and any <DL>, because a definition list's level may
    // change inside the section.
    rHTMLWrt.ChangeParaToken( HtmlTokenId::NONE );
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

        // is there a columned section around this one?
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
    // opened, except that it start immediately before the current one or
    // another end immediately before the current one
    if( pSurrCol && nSectSttIdx - pSurrSectNd->GetIndex() > 1 &&
        !lcl_html_IsMultiColEnd( rHTMLWrt, nSectSttIdx-1 ) )
        lcl_html_OutSectionEndTag( rHTMLWrt );

    if( bStartTag )
        lcl_html_OutSectionStartTag( rHTMLWrt, rSection, *pFormat, pCol );

    {
        HTMLSaveData aSaveData( rHTMLWrt,
            rHTMLWrt.m_pCurrentPam->GetPoint()->nNode.GetIndex()+1,
            rSectNd.EndOfSectionIndex(),
            false, pFormat );
        rHTMLWrt.Out_SwDoc( rHTMLWrt.m_pCurrentPam.get() );
    }

    rHTMLWrt.m_pCurrentPam->GetPoint()->nNode = *rSectNd.EndOfSectionNode();

    if( bEndTag )
        lcl_html_OutSectionEndTag( rHTMLWrt );

    // The surrounding section must be started again, except that it ends
    // immediately behind the current one.
    if( pSurrCol &&
        pSurrSectNd->EndOfSectionIndex() - nSectEndIdx > 1 &&
        !lcl_html_IsMultiColStart( rHTMLWrt, nSectEndIdx+1 ) )
        lcl_html_OutSectionStartTag( rHTMLWrt, *pSurrSection, *pSurrFormat,
                                     pSurrCol, true );

    return rWrt;
}

void SwHTMLWriter::Out_SwDoc( SwPaM* pPam )
{
    bool bSaveWriteAll = m_bWriteAll;     // save

    // search next text::Bookmark position from text::Bookmark table
    m_nBkmkTabPos = m_bWriteAll ? FindPos_Bkmk( *m_pCurrentPam->GetPoint() ) : -1;

    // output all areas of PaM's in the HTML file
    do {
        m_bWriteAll = bSaveWriteAll;
        m_bFirstLine = true;

        // search for first on PaM created FlyFrame
        // still missing:

        while( m_pCurrentPam->GetPoint()->nNode.GetIndex() < m_pCurrentPam->GetMark()->nNode.GetIndex() ||
              (m_pCurrentPam->GetPoint()->nNode.GetIndex() == m_pCurrentPam->GetMark()->nNode.GetIndex() &&
               m_pCurrentPam->GetPoint()->nContent.GetIndex() <= m_pCurrentPam->GetMark()->nContent.GetIndex()) )
        {
            SwNode&  rNd = m_pCurrentPam->GetNode();

            OSL_ENSURE( !(rNd.IsGrfNode() || rNd.IsOLENode()),
                    "Unexpected Grf- or OLE-Node here" );
            if( rNd.IsTextNode() )
            {
                SwTextNode* pTextNd = rNd.GetTextNode();

                if( !m_bFirstLine )
                    m_pCurrentPam->GetPoint()->nContent.Assign( pTextNd, 0 );

                OutHTML_SwTextNode( *this, *pTextNd );
            }
            else if( rNd.IsTableNode() )
            {
                OutHTML_SwTableNode( *this, *rNd.GetTableNode(), nullptr );
                m_nBkmkTabPos = m_bWriteAll ? FindPos_Bkmk( *m_pCurrentPam->GetPoint() ) : -1;
            }
            else if( rNd.IsSectionNode() )
            {
                OutHTML_Section( *this, *rNd.GetSectionNode() );
                m_nBkmkTabPos = m_bWriteAll ? FindPos_Bkmk( *m_pCurrentPam->GetPoint() ) : -1;
            }
            else if( &rNd == &m_pDoc->GetNodes().GetEndOfContent() )
                break;

            ++m_pCurrentPam->GetPoint()->nNode;   // move
            sal_uInt32 nPos = m_pCurrentPam->GetPoint()->nNode.GetIndex();

            if( m_bShowProgress )
                ::SetProgressState( nPos, m_pDoc->GetDocShell() );   // How far ?

            /* If only the selected area should be saved, so only the complete
             * nodes should be saved, this means the first and n-th node
             * partly, the 2nd till n-1 node complete. (complete means with
             * all formats!)
             */
            m_bWriteAll = bSaveWriteAll ||
                        nPos != m_pCurrentPam->GetMark()->nNode.GetIndex();
            m_bFirstLine = false;
            m_bOutFooter = false; // after one node no footer anymore
        }

        ChangeParaToken( HtmlTokenId::NONE ); // MIB 8.7.97: We're doing it here and not at the caller
        OutAndSetDefList( 0 );

    } while( CopyNextPam( &pPam ) );        // until all PaM's processed

    m_bWriteAll = bSaveWriteAll;          // reset to old values
}

// write the StyleTable, general data, header/footer/footnotes
static void OutBodyColor( const char* pTag, const SwFormat *pFormat,
                          SwHTMLWriter& rHWrt )
{
    const SwFormat *pRefFormat = nullptr;

    if( rHWrt.m_xTemplate.is() )
        pRefFormat = SwHTMLWriter::GetTemplateFormat( pFormat->GetPoolFormatId(),
                                                &rHWrt.m_xTemplate->getIDocumentStylePoolAccess() );

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
        // only when the item is set in the template of the current document
        // or has a different value as the in HTML template, it will be set
        const SvxColorItem *pCItem = static_cast<const SvxColorItem*>(pItem);

        if( !bRefItemSet )
        {
            pColorItem = pCItem;
        }
        else
        {
            Color aColor( pCItem->GetValue() );
            if( COL_AUTO == aColor )
                aColor = COL_BLACK;

            Color aRefColor( static_cast<const SvxColorItem*>(pRefItem)->GetValue() );
            if( COL_AUTO == aRefColor )
                aRefColor = COL_BLACK;

            if( !aColor.IsRGBEqual( aRefColor ) )
                pColorItem = pCItem;
        }
    }
    else if( bRefItemSet )
    {
        // The item was still set in the HTML template so we output the default
        pColorItem = &rItemSet.GetPool()->GetDefaultItem( RES_CHRATR_COLOR );
    }

    if( pColorItem )
    {
        OStringBuffer sOut;
        sOut.append(OStringLiteral(" ") + pTag + "=");
        rHWrt.Strm().WriteOString( sOut.makeStringAndClear() );
        Color aColor( pColorItem->GetValue() );
        if( COL_AUTO == aColor )
            aColor = COL_BLACK;
        HTMLOutFuncs::Out_Color( rHWrt.Strm(), aColor );
        if( RES_POOLCOLL_STANDARD==pFormat->GetPoolFormatId() )
            rHWrt.m_xDfltColor = aColor;
    }
}

sal_uInt16 SwHTMLWriter::OutHeaderAttrs()
{
    sal_uLong nIdx = m_pCurrentPam->GetPoint()->nNode.GetIndex();
    sal_uLong nEndIdx = m_pCurrentPam->GetMark()->nNode.GetIndex();

    SwTextNode *pTextNd = nullptr;
    while( nIdx<=nEndIdx &&
        nullptr==(pTextNd=m_pDoc->GetNodes()[nIdx]->GetTextNode()) )
        nIdx++;

    OSL_ENSURE( pTextNd, "No Text-Node found" );
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

            const SwFieldIds nFieldWhich =
                static_cast<const SwFormatField&>(pHt->GetAttr()).GetField()->GetTyp()->Which();
            if( SwFieldIds::Postit!=nFieldWhich &&
                SwFieldIds::Script!=nFieldWhich )
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
        if (mbXHTML)
            sOut.append(OOO_STRING_SVTOOLS_HTML_doctype " " OOO_STRING_SVTOOLS_XHTML_doctype11);
        else
            sOut.append(OOO_STRING_SVTOOLS_HTML_doctype " " OOO_STRING_SVTOOLS_HTML_doctype40);
        HTMLOutFuncs::Out_AsciiTag( Strm(), sOut.makeStringAndClear().getStr() ); // No GetNamespace() here.

        // build prelude
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_html );

        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_head );

        IncIndentLevel();   // indent content of <HEAD>

        // DocumentInfo
        OString sIndent = GetIndentString();

        uno::Reference<document::XDocumentProperties> xDocProps;
        SwDocShell *pDocShell(m_pDoc->GetDocShell());
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

        // comments and meta-tags of first paragraph
        rHeaderAttrs = OutHeaderAttrs();

        OutFootEndNoteInfo();
    }

    const SwPageDesc *pPageDesc = nullptr;

    // In none HTML documents the first set template will be exported
    // and if none is set the default template
    sal_uLong nNodeIdx = m_pCurrentPam->GetPoint()->nNode.GetIndex();

    while( nNodeIdx < m_pDoc->GetNodes().Count() )
    {
        SwNode *pNd = m_pDoc->GetNodes()[ nNodeIdx ];
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
        pPageDesc = &m_pDoc->GetPageDesc( 0 );

    if (!mbSkipHeaderFooter)
    {
        // and now ... the style sheet!!!
        if( m_bCfgOutStyles )
        {
            OutStyleSheet( *pPageDesc );
        }

        // and now ... the BASIC and JavaScript!
        if( m_pDoc->GetDocShell() )   // BASIC is possible only in case we have a DocShell
            OutBasic(*this);

        DecIndentLevel();   // indent content of <HEAD>
        OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_head, false );

        // the body won't be indented, because then everything would be indented!
        OutNewLine();
        sOut.append("<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_body);
        Strm().WriteOString( sOut.makeStringAndClear() );

        // language
        OutLanguage( m_eLang );

        // output text colour, when it was set in the default template or was changed
        OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_text,
                      m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD, false ),
                      *this );

        // colour of (un)visited links
        OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_link,
                      m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( RES_POOLCHR_INET_NORMAL ),
                      *this );
        OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_vlink,
                      m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( RES_POOLCHR_INET_VISIT ),
                      *this );

        const SfxItemSet& rItemSet = pPageDesc->GetMaster().GetAttrSet();

        // fdo#86857 page styles now contain the XATTR_*, not RES_BACKGROUND
        std::unique_ptr<SvxBrushItem> const aBrushItem(getSvxBrushItemFromSourceSet(rItemSet, RES_BACKGROUND));
        OutBackground(aBrushItem.get(), true);

        m_nDirection = GetHTMLDirection( rItemSet );
        OutDirection( m_nDirection );

        if( m_bCfgOutStyles )
        {
            OutCSS1_BodyTagStyleOpt( *this, rItemSet );
        }
        // append events
        if( m_pDoc->GetDocShell() )   // BASIC is possible only in case we have a DocShell
            OutBasicBodyEvents();

        Strm().WriteChar( '>' );
    }
    else if (mbReqIF)
        // ReqIF: start xhtml.BlkStruct.class.
        HTMLOutFuncs::Out_AsciiTag(Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_division);

    return pPageDesc;
}

void SwHTMLWriter::OutAnchor( const OUString& rName )
{
    OStringBuffer sOut;
    sOut.append("<" + GetNamespace() + OOO_STRING_SVTOOLS_HTML_anchor " ");
    if (!mbXHTML)
    {
        sOut.append(OOO_STRING_SVTOOLS_HTML_O_name "=\"");
        Strm().WriteOString( sOut.makeStringAndClear() );
        HTMLOutFuncs::Out_String( Strm(), rName, m_eDestEnc, &m_aNonConvertableCharacters ).WriteCharPtr( "\">" );
    }
    else
    {
        // XHTML wants 'id' instead of 'name', also the value can't contain
        // spaces.
        sOut.append(OOO_STRING_SVTOOLS_HTML_O_id "=\"");
        Strm().WriteOString( sOut.makeStringAndClear() );
        HTMLOutFuncs::Out_String( Strm(), rName.replace(' ', '_'), m_eDestEnc, &m_aNonConvertableCharacters ).WriteCharPtr( "\">" );
    }
    HTMLOutFuncs::Out_AsciiTag( Strm(), GetNamespace() + OOO_STRING_SVTOOLS_HTML_anchor, false );
}

void SwHTMLWriter::OutBookmarks()
{
    // fetch current bookmark
    const ::sw::mark::IMark* pBookmark = nullptr;
    IDocumentMarkAccess* const pMarkAccess = m_pDoc->getIDocumentMarkAccess();
    if(m_nBkmkTabPos != -1)
        pBookmark = pMarkAccess->getAllMarksBegin()[m_nBkmkTabPos];
    // Output all bookmarks in this paragraph. The content position
    // for the moment isn't considered!
    sal_uInt32 nNode = m_pCurrentPam->GetPoint()->nNode.GetIndex();
    while( m_nBkmkTabPos != -1
           && pBookmark->GetMarkPos().nNode.GetIndex() == nNode )
    {
        // The area of bookmarks is first ignored, because it's not read.

        // first the SWG specific data:
        if ( dynamic_cast< const ::sw::mark::IBookmark* >(pBookmark) && !pBookmark->GetName().isEmpty() )
        {
            OutAnchor( pBookmark->GetName() );
        }

        if( ++m_nBkmkTabPos >= pMarkAccess->getAllMarksCount() )
            m_nBkmkTabPos = -1;
        else
            pBookmark = pMarkAccess->getAllMarksBegin()[m_nBkmkTabPos];
    }

    decltype(m_aOutlineMarkPoss)::size_type nPos;
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

    const IDocumentMarkAccess* pMarkAccess = m_pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return;

    const sw::mark::IFieldmark* pMark = pMarkAccess->getFieldmarkAt(rPos);
    if (!pMark)
        return;

    if (pMark->GetFieldname() != ODF_FORMCHECKBOX)
        return;

    const sw::mark::ICheckboxFieldmark* pCheckBox =
        dynamic_cast<const sw::mark::ICheckboxFieldmark*>(pMark);

    if (!pCheckBox)
        return;

    OString aOut("<"
        OOO_STRING_SVTOOLS_HTML_input
        " "
        OOO_STRING_SVTOOLS_HTML_O_type
        "=\""
        OOO_STRING_SVTOOLS_HTML_IT_checkbox
        "\"");

    if (pCheckBox->IsChecked())
    {
        aOut += " "
            OOO_STRING_SVTOOLS_HTML_O_checked
            "=\""
            OOO_STRING_SVTOOLS_HTML_O_checked
            "\"";
    }

    aOut += "/>";
    Strm().WriteOString(aOut);

    // TODO : Handle other single-point fieldmark types here (if any).
}

void SwHTMLWriter::OutImplicitMark( const OUString& rMark,
                                    const char *pMarkType )
{
    if( !rMark.isEmpty() && !m_aImplicitMarks.empty() )
    {
        OUString sMark(rMark + OUStringChar(cMarkSeparator) + OUString::createFromAscii(pMarkType));
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
        OUString sCompare = sURL.copy(nPos + 1).replaceAll(" ", "");
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
        sURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
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
    if( rBackColor != COL_TRANSPARENT )
    {
        Strm().WriteOString( " " OOO_STRING_SVTOOLS_HTML_O_bgcolor "=" );
        HTMLOutFuncs::Out_Color( Strm(), rBackColor);
    }

    if( !bGraphic )
        return;

    const Graphic* pGrf = pBrushItem->GetGraphic();
    OUString GraphicURL = pBrushItem->GetGraphicLink();
    if( mbEmbedImages || GraphicURL.isEmpty())
    {
        if( pGrf )
        {
            OUString aGraphicInBase64;
            if( !XOutBitmap::GraphicToBase64(*pGrf, aGraphicInBase64) )
            {
                m_nWarn = WARN_SWG_POOR_LOAD;
            }
            Strm().WriteCharPtr( " " OOO_STRING_SVTOOLS_HTML_O_background "=\"" );
            Strm().WriteCharPtr( OOO_STRING_SVTOOLS_HTML_O_data ":" );
            HTMLOutFuncs::Out_String( Strm(), aGraphicInBase64, m_eDestEnc, &m_aNonConvertableCharacters ).WriteChar( '\"' );
        }
    }
    else
    {
        if( m_bCfgCpyLinkedGrfs )
        {
            CopyLocalFileToINet( GraphicURL );
        }
        OUString s( URIHelper::simpleNormalizedMakeRelative( GetBaseURL(), GraphicURL));
        Strm().WriteCharPtr(" " OOO_STRING_SVTOOLS_HTML_O_background "=\"" );
        HTMLOutFuncs::Out_String( Strm(), s, m_eDestEnc, &m_aNonConvertableCharacters );
        Strm().WriteCharPtr("\"");

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
    // ReqIF mode: consumers would ignore language anyway.
    if (!(LANGUAGE_DONTKNOW != nLang && !mbReqIF))
        return;

    OStringBuffer sOut;
    sOut.append(' ');
    if (mbXHTML)
        sOut.append(OOO_STRING_SVTOOLS_XHTML_O_lang);
    else
        sOut.append(OOO_STRING_SVTOOLS_HTML_O_lang);
    sOut.append("=\"");
    Strm().WriteOString( sOut.makeStringAndClear() );
    HTMLOutFuncs::Out_String( Strm(), LanguageTag::convertToBcp47(nLang),
                              m_eDestEnc, &m_aNonConvertableCharacters ).WriteChar( '"' );
}

SvxFrameDirection SwHTMLWriter::GetHTMLDirection( const SfxItemSet& rItemSet ) const
{
    return GetHTMLDirection( rItemSet.Get( RES_FRAMEDIR ).GetValue() );
}

SvxFrameDirection SwHTMLWriter::GetHTMLDirection( SvxFrameDirection nDir ) const
{
    switch( nDir )
    {
    case SvxFrameDirection::Vertical_LR_TB:
        nDir = SvxFrameDirection::Horizontal_LR_TB;
        break;
    case SvxFrameDirection::Vertical_RL_TB:
        nDir = SvxFrameDirection::Horizontal_RL_TB;
        break;
    case SvxFrameDirection::Environment:
        nDir = m_nDirection;
        break;
    default: break;
    }

    return nDir;
}

void SwHTMLWriter::OutDirection( SvxFrameDirection nDir )
{
    OString sConverted = convertDirection(nDir);
    if (!sConverted.isEmpty())
    {
        OString sOut =
            " " OOO_STRING_SVTOOLS_HTML_O_dir
            "=\"" + sConverted + "\"";
        Strm().WriteOString( sOut );
    }
}

OString SwHTMLWriter::convertDirection(SvxFrameDirection nDir)
{
    OString sConverted;
    switch (nDir)
    {
    case SvxFrameDirection::Horizontal_LR_TB:
    case SvxFrameDirection::Vertical_LR_TB:
        sConverted = "ltr";
        break;
    case SvxFrameDirection::Horizontal_RL_TB:
    case SvxFrameDirection::Vertical_RL_TB:
        sConverted = "rtl";
        break;
    default: break;
    }
    return sConverted;
}

OString SwHTMLWriter::GetIndentString(sal_uInt16 nIncLvl)
{
    OString sRet;

    // somewhat cumbersome, but we have only one indent string!
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

OString SwHTMLWriter::GetNamespace() const
{
    if (maNamespace.isEmpty())
        return OString();

    return maNamespace + ":";
}

// Structure caches the current data of the writer to output a
// other part of the document, like e.g. header/footer
HTMLSaveData::HTMLSaveData(SwHTMLWriter& rWriter, sal_uLong nStt,
                           sal_uLong nEnd, bool bSaveNum,
                           const SwFrameFormat *pFrameFormat)
    : rWrt(rWriter)
    , pOldPam(rWrt.m_pCurrentPam)
    , pOldEnd(rWrt.GetEndPaM())
    , nOldDefListLvl(rWrt.m_nDefListLvl)
    , nOldDirection(rWrt.m_nDirection)
    , bOldOutHeader(rWrt.m_bOutHeader)
    , bOldOutFooter(rWrt.m_bOutFooter)
    , bOldOutFlyFrame(rWrt.m_bOutFlyFrame)
{
    bOldWriteAll = rWrt.m_bWriteAll;

    rWrt.m_pCurrentPam = Writer::NewUnoCursor(*rWrt.m_pDoc, nStt, nEnd);

    // recognize table in special areas
    if( nStt != rWrt.m_pCurrentPam->GetMark()->nNode.GetIndex() )
    {
        const SwNode *pNd = rWrt.m_pDoc->GetNodes()[ nStt ];
        if( pNd->IsTableNode() || pNd->IsSectionNode() )
            rWrt.m_pCurrentPam->GetMark()->nNode = nStt;
    }

    rWrt.SetEndPaM( rWrt.m_pCurrentPam.get() );
    rWrt.m_pCurrentPam->Exchange( );
    rWrt.m_bWriteAll = true;
    rWrt.m_nDefListLvl = 0;
    rWrt.m_bOutHeader = rWrt.m_bOutFooter = false;

    // Maybe save the current numbering information, so that it can be started again.
    // Only then also the numbering information of the next paragraph will be valid.
    if( bSaveNum )
    {
        pOldNumRuleInfo.reset( new SwHTMLNumRuleInfo( rWrt.GetNumInfo() ) );
        pOldNextNumRuleInfo = rWrt.ReleaseNextNumInfo();
    }
    else
    {
        rWrt.ClearNextNumInfo();
    }

    // The numbering will be in any case interrupted.
    rWrt.GetNumInfo().Clear();

    if( pFrameFormat )
        rWrt.m_nDirection = rWrt.GetHTMLDirection( pFrameFormat->GetAttrSet() );
}

HTMLSaveData::~HTMLSaveData()
{
    rWrt.m_pCurrentPam.reset(); // delete PaM again

    rWrt.m_pCurrentPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.m_bWriteAll = bOldWriteAll;
    rWrt.m_nBkmkTabPos = bOldWriteAll ? rWrt.FindPos_Bkmk( *pOldPam->GetPoint() ) : -1;
    rWrt.m_nLastParaToken = HtmlTokenId::NONE;
    rWrt.m_nDefListLvl = nOldDefListLvl;
    rWrt.m_nDirection = nOldDirection;
    rWrt.m_bOutHeader = bOldOutHeader;
    rWrt.m_bOutFooter = bOldOutFooter;
    rWrt.m_bOutFlyFrame = bOldOutFlyFrame;

    // Maybe continue the numbering from before section. The numbering
    // of the next paragraph will be invalid in any case.
    if( pOldNumRuleInfo )
    {
        rWrt.GetNumInfo().Set( *pOldNumRuleInfo );
        pOldNumRuleInfo.reset();
        rWrt.SetNextNumInfo( std::move(pOldNextNumRuleInfo) );
    }
    else
    {
        rWrt.GetNumInfo().Clear();
        rWrt.ClearNextNumInfo();
    }
}

void GetHTMLWriter( const OUString& rFilterOptions, const OUString& rBaseURL, WriterRef& xRet )
{
    xRet = new SwHTMLWriter( rBaseURL, rFilterOptions );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
