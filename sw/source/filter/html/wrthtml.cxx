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
#include <mdiexp.hxx>       // ...Percent()
#include <fltini.hxx>
#include <viewopt.hxx>
#include <IMark.hxx>        // fuer SwBookmark ...
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
#include <statstr.hrc>      // ResId fuer Statusleiste
#include <swerror.h>
#include <rtl/strbuf.hxx>

#define MAX_INDENT_LEVEL 20

#if defined(UNX)
const sal_Char SwHTMLWriter::sNewLine = '\012';
#else
const sal_Char SwHTMLWriter::sNewLine[] = "\015\012";
#endif

static sal_Char sIndentTabs[MAX_INDENT_LEVEL+2] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

SwHTMLWriter::SwHTMLWriter( const String& rBaseURL )
    : bCfgOutStyles( sal_False )
    , bCfgPreferStyles( sal_False )
    , bCfgFormFeed( sal_False )
    , bCfgStarBasic( sal_False )
    , bCfgCpyLinkedGrfs( sal_False )
    , bFirstLine( sal_False )
    , bTagOn( sal_False )
    , bTxtAttr( sal_False )
    , bOutOpts( sal_False )
    , bOutTable( sal_False )
    , bOutHeader( sal_False )
    , bOutFooter( sal_False )
    , bOutFlyFrame( sal_False )
    , bFirstCSS1Rule( sal_False )
    , bFirstCSS1Property( sal_False )
    , bPoolCollTextModified( sal_False )
    , bCSS1IgnoreFirstPageDesc( sal_False )
    , bNoAlign( sal_False )
    , bClearLeft( sal_False )
    , bClearRight( sal_False )
    , bLFPossible( sal_False )
    , bPreserveForm( sal_False )
    , bCfgNetscape4( sal_False )

{
    SetBaseURL( rBaseURL );
    bFirstLine = sal_True;
    nBkmkTabPos = -1;
    pDfltColor = 0;
    nImgMapCnt = 1;
    pStartNdIdx = 0;
    pTemplate = 0;
    pNumRuleInfo = new SwHTMLNumRuleInfo;
    pNextNumRuleInfo = 0;
    pFootEndNotes = 0;
    pFmtFtn = 0;
    eDestEnc = RTL_TEXTENCODING_MS_1252;
    nDirection = FRMDIR_HORI_LEFT_TOP;
}


SwHTMLWriter::~SwHTMLWriter()
{
    delete pNumRuleInfo;
}

sal_uLong SwHTMLWriter::WriteStream()
{
    // neue Konfiguration setzen
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();

    // die Fontgroessen 1-7
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
    if( HTML_CFG_WRITER==nExportMode ||
        HTML_CFG_NS40==nExportMode )
        nHTMLMode |= HTMLMODE_BLOCK_SPACER;

    if( HTML_CFG_WRITER==nExportMode || HTML_CFG_MSIE==nExportMode )
        nHTMLMode |= (HTMLMODE_FLOAT_FRAME | HTMLMODE_LSPACE_IN_NUMBUL);

    if( HTML_CFG_MSIE==nExportMode )
        nHTMLMode |= HTMLMODE_NBSP_IN_TABLES;

    if( HTML_CFG_WRITER==nExportMode || HTML_CFG_NS40==nExportMode ||
        HTML_CFG_MSIE==nExportMode )
        nHTMLMode |= HTMLMODE_ABS_POS_FLY|HTMLMODE_ABS_POS_DRAW;

    if( HTML_CFG_WRITER==nExportMode )
        nHTMLMode |= HTMLMODE_FLY_MARGINS;

    if( HTML_CFG_NS40==nExportMode )
        nHTMLMode |= HTMLMODE_BORDER_NONE;

    nHTMLMode |= HTMLMODE_FONT_GENERIC;

    if( HTML_CFG_NS40==nExportMode )
        nHTMLMode |= HTMLMODE_NO_CONTROL_CENTERING;

    bCfgOutStyles = IsHTMLMode(HTMLMODE_SOME_STYLES |
                               HTMLMODE_FULL_STYLES);
    bCfgNetscape4 = (HTML_CFG_NS40==nExportMode);

    if( IsHTMLMode(HTMLMODE_SOME_STYLES | HTMLMODE_FULL_STYLES) )
        nHTMLMode |= HTMLMODE_PRINT_EXT;

    const sal_Char *pHelpHack = getenv( "HelpEx" );
    if( pHelpHack )
    {
        OString aTmp(pHelpHack);
        if (aTmp.equalsIgnoreAsciiCase("Hilfe"))
            nHTMLMode |= HTMLMODE_NO_BR_AT_PAREND;
    }

    eCSS1Unit = (FieldUnit)SW_MOD()->GetMetric( pDoc->get(IDocumentSettingAccess::HTML_MODE) );

    sal_Bool bWriteUTF8 = bWriteClipboardDoc;
    eDestEnc = bWriteUTF8 ? RTL_TEXTENCODING_UTF8
                          : rHtmlOptions.GetTextEncoding();
    const sal_Char *pCharSet =
        rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );
    eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    // Nur noch fuer den MS-IE ziehen wir den Export von Styles vor.
    bCfgPreferStyles = HTML_CFG_MSIE==nExportMode;

    bCfgStarBasic = rHtmlOptions.IsStarBasic();

    bCfgFormFeed = !IsHTMLMode(HTMLMODE_PRINT_EXT);
    bCfgCpyLinkedGrfs = rHtmlOptions.IsSaveGraphicsLocal();

    // die HTML-Vorlage holen
    sal_Bool bOldHTMLMode = sal_False;
    sal_uInt16 nOldTxtFmtCollCnt = 0, nOldCharFmtCnt = 0;

    OSL_ENSURE( !pTemplate, "Wo kommt denn die HTML-Vorlage hier her?" );
    pTemplate = ((HTMLReader*)ReadHTML)->GetTemplateDoc();
    if( pTemplate )
    {
        pTemplate->acquire();
        bOldHTMLMode = pTemplate->get(IDocumentSettingAccess::HTML_MODE);
        pTemplate->set(IDocumentSettingAccess::HTML_MODE, true);

        nOldTxtFmtCollCnt = pTemplate->GetTxtFmtColls()->size();
        nOldCharFmtCnt = pTemplate->GetCharFmts()->size();
    }

    if( bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, pDoc->GetNodes().Count(),
                         pDoc->GetDocShell());

    pDfltColor = 0;
    pFootEndNotes = 0;
    pFmtFtn = 0;
    bOutTable = bOutHeader = bOutFooter = bOutFlyFrame = sal_False;
    pxFormComps = 0;
    nFormCntrlCnt = 0;
    bPreserveForm = sal_False;
    bClearLeft = bClearRight = sal_False;
    bLFPossible = sal_False;

    nLeftMargin = nDfltLeftMargin = nDfltRightMargin = 0;
    nDfltTopMargin = nDfltBottomMargin = 0;
    nFirstLineIndent = nDfltFirstLineIndent = 0;
    bPoolCollTextModified = sal_False;
    bFirstCSS1Property = bFirstCSS1Rule = sal_False;
    bCSS1IgnoreFirstPageDesc = sal_False;
    nIndentLvl = 0;
    nWhishLineLen = 70;
    nLastLFPos = 0;
    nDefListLvl = 0;
    nDefListMargin = ((pTemplate && !bCfgOutStyles) ? pTemplate : pDoc)
        ->GetTxtCollFromPool( RES_POOLCOLL_HTML_DD, false )
        ->GetLRSpace().GetTxtLeft();
    nHeaderFooterSpace = 0;
    nTxtAttrsToIgnore = 0;
    nCSS1OutMode = 0;
    sal_uInt16 nScript = SvtLanguageOptions::GetScriptTypeOfLanguage( GetAppLanguage() );
    switch( nScript )
    {
    case SCRIPTTYPE_ASIAN:
        nCSS1Script = CSS1_OUTMODE_CJK;
        break;
    case SCRIPTTYPE_COMPLEX:
        nCSS1Script = CSS1_OUTMODE_CTL;
        break;
    default:
        nCSS1Script = CSS1_OUTMODE_WESTERN;
        break;
    }
    eLang = ((const SvxLanguageItem&)pDoc
            ->GetDefault(GetLangWhichIdFromScript(nCSS1Script))).GetLanguage();

    nFootNote = nEndNote = 0;

    nWarn = 0;
    GetNumInfo().Clear();
    pNextNumRuleInfo = 0;

    OString aStartTags;

    // Tabellen und Bereiche am Doc.-Anfang beachten
    {
        SwTableNode * pTNd = pCurPam->GetNode()->FindTableNode();
        if( pTNd && bWriteAll )
        {
            // mit dem Tabellen-Node anfangen !!
            pCurPam->GetPoint()->nNode = *pTNd;

            if( bWriteOnlyFirstTable )
                pCurPam->GetMark()->nNode = *pTNd->EndOfSectionNode();
        }

        // erster Node (der einen Seitenumbruch enthalten darf)
        pStartNdIdx = new SwNodeIndex( pCurPam->GetPoint()->nNode );

        SwSectionNode * pSNd = pCurPam->GetNode()->FindSectionNode();
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

    bLFPossible = sal_True;

    // Formulare, die nur HiddenControls enthalten ausgeben.
    OutHiddenForms();

    if( !aStartTags.isEmpty() )
        Strm() << aStartTags.getStr();

    const SfxPoolItem *pItem;
    const SfxItemSet& rPageItemSet = pCurrPageDesc->GetMaster().GetAttrSet();
    if( !bWriteClipboardDoc && pDoc->GetDocShell() &&
         (!pDoc->get(IDocumentSettingAccess::HTML_MODE) &&
          !pDoc->get(IDocumentSettingAccess::BROWSE_MODE)) &&
        SFX_ITEM_SET == rPageItemSet.GetItemState( RES_HEADER, sal_True, &pItem) )
    {
        const SwFrmFmt *pHeaderFmt =
            ((const SwFmtHeader *)pItem)->GetHeaderFmt();
        if( pHeaderFmt )
            OutHTML_HeaderFooter( *this, *pHeaderFmt, sal_True );
    }

    nTxtAttrsToIgnore = nHeaderAttrs;
    Out_SwDoc( pOrigPam );
    nTxtAttrsToIgnore = 0;

    if( pxFormComps && pxFormComps->is() )
        OutForm( sal_False, *pxFormComps );

    if( pFootEndNotes )
        OutFootEndNotes();

    if( !bWriteClipboardDoc && pDoc->GetDocShell() &&
        (!pDoc->get(IDocumentSettingAccess::HTML_MODE) && !pDoc->get(IDocumentSettingAccess::BROWSE_MODE))  &&
        SFX_ITEM_SET == rPageItemSet.GetItemState( RES_FOOTER, sal_True, &pItem) )
    {
        const SwFrmFmt *pFooterFmt =
            ((const SwFmtFooter *)pItem)->GetFooterFmt();
        if( pFooterFmt )
            OutHTML_HeaderFooter( *this, *pFooterFmt, sal_False );
    }

    if( bLFPossible )
        OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_body, sal_False );
    OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_html, sal_False );

    // loesche die Tabelle mit den freifliegenden Rahmen
    sal_uInt16 i;
    OSL_ENSURE( !pHTMLPosFlyFrms, "Wurden nicht alle Rahmen ausgegeben" );
    if( pHTMLPosFlyFrms )
    {
        pHTMLPosFlyFrms->DeleteAndDestroyAll();
        delete pHTMLPosFlyFrms;
        pHTMLPosFlyFrms = 0;
    }

    aHTMLControls.DeleteAndDestroyAll();

    if( !aChrFmtInfos.empty() )
        aChrFmtInfos.clear();

    if( !aTxtCollInfos.empty() )
        aTxtCollInfos.clear();

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
            "SwHTMLWriter::Write: Ftns nicht durch OutFootEndNotes geloescht" );

    pCurrPageDesc = 0;

    ClearNextNumInfo();

    for( i=0; i<MAXLEVEL; i++ )
        aBulletGrfs[i].Erase();

    aNonConvertableCharacters.Erase();

    if( bShowProgress )
        ::EndProgress( pDoc->GetDocShell() );

    if( pTemplate )
    {
        // Waehrend des Exports angelegte Zeichen- und Abastzvorlagen
        // loeschen
        sal_uInt16 nTxtFmtCollCnt = pTemplate->GetTxtFmtColls()->size();
        while( nTxtFmtCollCnt > nOldTxtFmtCollCnt )
            pTemplate->DelTxtFmtColl( --nTxtFmtCollCnt );
        OSL_ENSURE( pTemplate->GetTxtFmtColls()->size() == nOldTxtFmtCollCnt,
                "falsche Anzahl TxtFmtColls geloescht" );

        sal_uInt16 nCharFmtCnt = pTemplate->GetCharFmts()->size();
        while( nCharFmtCnt > nOldCharFmtCnt )
            pTemplate->DelCharFmt( --nCharFmtCnt );
        OSL_ENSURE( pTemplate->GetCharFmts()->size() == nOldCharFmtCnt,
                "falsche Anzahl CharFmts geloescht" );

        // HTML-Modus wieder restaurieren
        pTemplate->set(IDocumentSettingAccess::HTML_MODE, bOldHTMLMode);

        if( 0 == pTemplate->release() )
            delete pTemplate;

        pTemplate = 0;
    }

    return nWarn;
}

static const SwFmtCol *lcl_html_GetFmtCol( const SwSection& rSection,
                                       const SwSectionFmt& rFmt )
{
    const SwFmtCol *pCol = 0;

    const SfxPoolItem* pItem;
    if( FILE_LINK_SECTION != rSection.GetType() &&
        SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(RES_COL,sal_False,&pItem) &&
        ((const SwFmtCol *)pItem)->GetNumCols() > 1 )
    {
        pCol = (const SwFmtCol *)pItem;
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
        const SwSectionFmt *pFmt = rSection.GetFmt();
        if( pFmt && lcl_html_GetFmtCol( rSection, *pFmt ) )
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
                                     const SwSectionFmt& rFmt,
                                  const SwFmtCol *pCol,
                                  bool bContinued=false )
{
    OSL_ENSURE( pCol || !bContinued, "Continuation of DIV" );

    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_division);

    const String& rName = rSection.GetSectionName();
    if( rName.Len() && !bContinued )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_id).
            append(RTL_CONSTASCII_STRINGPARAM("=\""));
        rHTMLWrt.Strm() << sOut.makeStringAndClear().getStr();
        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), rName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut.append('\"');
    }

    sal_uInt16 nDir = rHTMLWrt.GetHTMLDirection( rFmt.GetAttrSet() );
    rHTMLWrt.Strm() << sOut.makeStringAndClear().getStr();
    rHTMLWrt.OutDirection( nDir );

    if( FILE_LINK_SECTION == rSection.GetType() )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_href).
            append(RTL_CONSTASCII_STRINGPARAM("=\""));
        rHTMLWrt.Strm() << sOut.makeStringAndClear().getStr();

        const String& aFName = rSection.GetLinkFileName();
        String aURL( aFName.GetToken(0,sfx2::cTokenSeparator) );
        String aFilter( aFName.GetToken(1,sfx2::cTokenSeparator) );
        String aSection( aFName.GetToken(2,sfx2::cTokenSeparator) );

        String aEncURL( URIHelper::simpleNormalizedMakeRelative(rHTMLWrt.GetBaseURL(), aURL ) );
        sal_Unicode cDelim = 255U;
        bool bURLContainsDelim =
            (STRING_NOTFOUND != aEncURL.Search( cDelim ) );

        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aEncURL,
                                  rHTMLWrt.eDestEnc,
                                  &rHTMLWrt.aNonConvertableCharacters );
        const sal_Char *pDelim = "&#255;";
        if( aFilter.Len() || aSection.Len() || bURLContainsDelim )
            rHTMLWrt.Strm() << pDelim;
        if( aFilter.Len() )
            HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aFilter,
                                      rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        if( aSection.Len() || bURLContainsDelim  )
                rHTMLWrt.Strm() << pDelim;
        if( aSection.Len() )
        {
            xub_StrLen nPos = aSection.Search( '%' );
            while( STRING_NOTFOUND != nPos )
            {
                aSection.Erase( nPos, 1 );
                aSection.InsertAscii( "%25", nPos );
                nPos = aSection.Search( '%', nPos+3 );
            }
            nPos = aSection.Search( cDelim );
            while( STRING_NOTFOUND != nPos )
            {
                aSection.Erase( nPos, 1 );
                aSection.InsertAscii( "%FF", nPos );
                nPos = aSection.Search( cDelim, nPos+3 );
            }
            HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), aSection,
                                      rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        }
        sOut.append('\"');
    }
    else if( pCol )
    {
        // minumum gutter width
        sal_uInt16 nGutter = pCol->GetGutterWidth( sal_True );
        if( nGutter!=USHRT_MAX )
        {
            if( nGutter && Application::GetDefaultDevice() )
            {
                nGutter = (sal_uInt16)Application::GetDefaultDevice()
                                ->LogicToPixel( Size(nGutter,0),
                                                MapMode(MAP_TWIP) ).Width();
            }
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_gutter).
                append('=').append(static_cast<sal_Int32>(nGutter));
        }
    }

    rHTMLWrt.Strm() << sOut.makeStringAndClear().getStr();
    if( rHTMLWrt.IsHTMLMode( rHTMLWrt.bCfgOutStyles ) )
        rHTMLWrt.OutCSS1_SectionFmtOptions( rFmt, pCol );

    rHTMLWrt.Strm() << '>';

    rHTMLWrt.bLFPossible = sal_True;
    if( rName.Len() && !bContinued )
        rHTMLWrt.OutImplicitMark( rName, pMarkToRegion );

    rHTMLWrt.IncIndentLevel();
}

static void lcl_html_OutSectionEndTag( SwHTMLWriter& rHTMLWrt )
{
    rHTMLWrt.DecIndentLevel();
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), OOO_STRING_SVTOOLS_HTML_division, sal_False );
    rHTMLWrt.bLFPossible = sal_True;
}

static Writer& OutHTML_Section( Writer& rWrt, const SwSectionNode& rSectNd )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    // End <PRE> and any <DL>, because a definition list's level may
    // change inside the section.
    rHTMLWrt.ChangeParaToken( 0 );
    rHTMLWrt.OutAndSetDefList( 0 );

    const SwSection& rSection = rSectNd.GetSection();
    const SwSectionFmt *pFmt = rSection.GetFmt();
    OSL_ENSURE( pFmt, "Section without a format?" );

    bool bStartTag = true;
    bool bEndTag = true;
    const SwSectionFmt *pSurrFmt = 0;
    const SwSectionNode *pSurrSectNd = 0;
    const SwSection *pSurrSection = 0;
    const SwFmtCol *pSurrCol = 0;

    sal_uInt32 nSectSttIdx = rSectNd.GetIndex();
    sal_uInt32 nSectEndIdx = rSectNd.EndOfSectionIndex();
    const SwFmtCol *pCol = lcl_html_GetFmtCol( rSection, *pFmt );
    if( pCol )
    {
        // If the next node is a columned section node, too, don't export
        // an empty section.
        if( lcl_html_IsMultiColStart( rHTMLWrt, nSectSttIdx+1 ) )
            bStartTag = false;

        // The same applies if the section end with another columned section.
        if( lcl_html_IsMultiColEnd( rHTMLWrt, nSectEndIdx-1 ) )
            bEndTag = false;

        //.is there a columned section arround this one?
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
                    pSurrFmt = pSurrSection->GetFmt();
                    if( pSurrFmt )
                        pSurrCol = lcl_html_GetFmtCol( *pSurrSection,
                                                       *pSurrFmt );
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
        lcl_html_OutSectionStartTag( rHTMLWrt, rSection, *pFmt, pCol );

    {
        HTMLSaveData aSaveData( rHTMLWrt,
            rHTMLWrt.pCurPam->GetPoint()->nNode.GetIndex()+1,
            rSectNd.EndOfSectionIndex(),
            sal_False, pFmt );
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
        lcl_html_OutSectionStartTag( rHTMLWrt, *pSurrSection, *pSurrFmt,
                                     pSurrCol, true );

    return rWrt;
}

void SwHTMLWriter::Out_SwDoc( SwPaM* pPam )
{
    sal_Bool bSaveWriteAll = bWriteAll;     // sichern

    // suche die naechste text::Bookmark-Position aus der text::Bookmark-Tabelle
    nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;

    // gebe alle Bereiche des Pams in das HTML-File aus.
    do {
        bWriteAll = bSaveWriteAll;
        bFirstLine = sal_True;

        // suche den ersten am Pam-auszugebenen FlyFrame
        // fehlt noch:

        while( pCurPam->GetPoint()->nNode.GetIndex() < pCurPam->GetMark()->nNode.GetIndex() ||
              (pCurPam->GetPoint()->nNode.GetIndex() == pCurPam->GetMark()->nNode.GetIndex() &&
               pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
        {
            SwNode * pNd = pCurPam->GetNode();

            OSL_ENSURE( !(pNd->IsGrfNode() || pNd->IsOLENode()),
                    "Grf- oder OLE-Node hier unerwartet" );
            if( pNd->IsTxtNode() )
            {
                SwTxtNode* pTxtNd = pNd->GetTxtNode();

                if( !bFirstLine )
                    pCurPam->GetPoint()->nContent.Assign( pTxtNd, 0 );

                OutHTML_SwTxtNode( *this, *pTxtNd );
            }
            else if( pNd->IsTableNode() )
            {
                OutHTML_SwTblNode( *this, *pNd->GetTableNode(), 0 );
                nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;
            }
            else if( pNd->IsSectionNode() )
            {
                OutHTML_Section( *this, *pNd->GetSectionNode() );
                nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : -1;
            }
            else if( pNd == &pDoc->GetNodes().GetEndOfContent() )
                break;

            pCurPam->GetPoint()->nNode++;   // Bewegen
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
            bFirstLine = sal_False;
            bOutFooter = sal_False; // Nach einem Node keine Fusszeile mehr
        }

        ChangeParaToken( 0 ); // MIB 8.7.97: Machen wir jetzt hier und nicht
                              // beim Aufrufer
        OutAndSetDefList( 0 );

    } while( CopyNextPam( &pPam ) );        // bis alle PaM's bearbeitet

    bWriteAll = bSaveWriteAll;          // wieder auf alten Wert zurueck
}


// schreibe die StyleTabelle, algemeine Angaben,Header/Footer/Footnotes
static void OutBodyColor( const sal_Char *pTag, const SwFmt *pFmt,
                          SwHTMLWriter& rHWrt )
{
    const SwFmt *pRefFmt = 0;

    if( rHWrt.pTemplate )
        pRefFmt = SwHTMLWriter::GetTemplateFmt( pFmt->GetPoolFmtId(),
                                                rHWrt.pTemplate );

    const SvxColorItem *pColorItem = 0;

    const SfxItemSet& rItemSet = pFmt->GetAttrSet();
    const SfxPoolItem *pRefItem = 0, *pItem = 0;
    bool bItemSet = SFX_ITEM_SET == rItemSet.GetItemState( RES_CHRATR_COLOR,
                                                           sal_True, &pItem);
    bool bRefItemSet = pRefFmt &&
        SFX_ITEM_SET == pRefFmt->GetAttrSet().GetItemState( RES_CHRATR_COLOR,
                                                            sal_True, &pRefItem);
    if( bItemSet )
    {
        // wenn das Item nur in der Vorlage des aktuellen Doks gesetzt
        // ist oder einen anderen Wert hat, als in der HTML-Vorlage,
        // wird es gesetzt
        const SvxColorItem *pCItem = (const SvxColorItem*)pItem;

        if( !bRefItemSet )
        {
            pColorItem = pCItem;
        }
        else
        {
            Color aColor( pCItem->GetValue() );
            if( COL_AUTO == aColor.GetColor() )
                aColor.SetColor( COL_BLACK );

            Color aRefColor( ((const SvxColorItem*)pRefItem)->GetValue() );
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
        pColorItem = (const SvxColorItem*)&rItemSet.GetPool()
                                        ->GetDefaultItem( RES_CHRATR_COLOR );
    }

    if( pColorItem )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(pTag).append('=');
        rHWrt.Strm() << sOut.makeStringAndClear().getStr();
        Color aColor( pColorItem->GetValue() );
        if( COL_AUTO == aColor.GetColor() )
            aColor.SetColor( COL_BLACK );
        HTMLOutFuncs::Out_Color( rHWrt.Strm(), aColor, rHWrt.eDestEnc );
        if( RES_POOLCOLL_STANDARD==pFmt->GetPoolFmtId() )
            rHWrt.pDfltColor = new Color( aColor );
    }
}

sal_uInt16 SwHTMLWriter::OutHeaderAttrs()
{
    sal_uLong nIdx = pCurPam->GetPoint()->nNode.GetIndex();
    sal_uLong nEndIdx = pCurPam->GetMark()->nNode.GetIndex();

    SwTxtNode *pTxtNd = 0;
    while( nIdx<=nEndIdx &&
            0==(pTxtNd=pDoc->GetNodes()[nIdx]->GetTxtNode()) )
        nIdx++;

    OSL_ENSURE( pTxtNd, "Kein Text-Node gefunden" );
    if( !pTxtNd || !pTxtNd->HasHints() )
        return 0;

    sal_uInt16 nAttrs = 0;
    sal_uInt16 nCntAttr = pTxtNd->GetSwpHints().Count();
    xub_StrLen nOldPos = 0;
    for( sal_uInt16 i=0; i<nCntAttr; i++ )
    {
        const SwTxtAttr *pHt = pTxtNd->GetSwpHints()[i];
        if( !pHt->GetEnd() )
        {
            xub_StrLen nPos = *pHt->GetStart();
            if( nPos-nOldPos > 1 || RES_TXTATR_FIELD != pHt->Which() )
                break;

            sal_uInt16 nFldWhich = ((const SwFmtFld&)pHt->GetAttr()).GetFld()
                                                     ->GetTyp()->Which();
            if( RES_POSTITFLD!=nFldWhich &&
                RES_SCRIPTFLD!=nFldWhich )
                break;

            OutNewLine();
            OutHTML_SwFmtFld( *this, pHt->GetAttr() );
            nOldPos = nPos;
            nAttrs++;
        }
    }

    return nAttrs;
}

const SwPageDesc *SwHTMLWriter::MakeHeader( sal_uInt16 &rHeaderAttrs )
{
    OStringBuffer sOut;
    sOut.append(OOO_STRING_SVTOOLS_HTML_doctype).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_doctype40);
    HTMLOutFuncs::Out_AsciiTag( Strm(), sOut.makeStringAndClear().getStr() );

    // baue den Vorspann
    OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_html );

    OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_head );

    IncIndentLevel();   // Inhalt von <HEAD> einruecken

    // DokumentInfo
    OString sIndent = GetIndentString();
    using namespace ::com::sun::star;
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

    const SwPageDesc *pPageDesc = 0;

    // In Nicht-HTML-Dokumenten wird die erste gesetzte Seitenvorlage
    // exportiert und wenn keine gesetzt ist die Standard-Vorlage
    sal_uLong nNodeIdx = pCurPam->GetPoint()->nNode.GetIndex();

    while( nNodeIdx < pDoc->GetNodes().Count() )
    {
        SwNode *pNd = pDoc->GetNodes()[ nNodeIdx ];
        if( pNd->IsCntntNode() )
        {
            pPageDesc = ((const SwFmtPageDesc &)pNd->GetCntntNode()
                ->GetAttr(RES_PAGEDESC)).GetPageDesc();
            break;
        }
        else if( pNd->IsTableNode() )
        {
            pPageDesc = pNd->GetTableNode()->GetTable().GetFrmFmt()
                           ->GetPageDesc().GetPageDesc();
            break;
        }

        nNodeIdx++;
    }

    if( !pPageDesc )
        pPageDesc = &pDoc->GetPageDesc( 0 );

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
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_head, sal_False );

    // der Body wird nicht eingerueckt, weil sonst alles eingerueckt waere!
    OutNewLine();
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_body);
    Strm() << sOut.makeStringAndClear().getStr();

    // language
    OutLanguage( eLang );

    // Textfarbe ausgeben, wenn sie an der Standard-Vorlage gesetzt ist
    // und sich geaendert hat.
    OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_text,
                  pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false ),
                  *this );

    // Farben fuer (un)besuchte Links
    OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_link,
                  pDoc->GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL ),
                  *this );
    OutBodyColor( OOO_STRING_SVTOOLS_HTML_O_vlink,
                  pDoc->GetCharFmtFromPool( RES_POOLCHR_INET_VISIT ),
                  *this );

    const SfxItemSet& rItemSet = pPageDesc->GetMaster().GetAttrSet();

    OutBackground( rItemSet, sal_True );

    nDirection = GetHTMLDirection( rItemSet );
    OutDirection( nDirection );

    if( bCfgOutStyles )
        OutCSS1_BodyTagStyleOpt( *this, rItemSet );

    // Events anhaengen
    if( pDoc->GetDocShell() )   // nur mit DocShell ist Basic moeglich
        OutBasicBodyEvents();

    Strm() << '>';

    return pPageDesc;
}

void SwHTMLWriter::OutAnchor( const String& rName )
{
    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_anchor).append(' ')
        .append(OOO_STRING_SVTOOLS_HTML_O_name).append("=\"");
    Strm() << sOut.makeStringAndClear().getStr();
    HTMLOutFuncs::Out_String( Strm(), rName, eDestEnc, &aNonConvertableCharacters ) << "\">";
    HTMLOutFuncs::Out_AsciiTag( Strm(), OOO_STRING_SVTOOLS_HTML_anchor, sal_False );
}

void SwHTMLWriter::OutBookmarks()
{
    // hole das aktuelle Bookmark
    const ::sw::mark::IMark* pBookmark = NULL;
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    if(nBkmkTabPos != -1)
        pBookmark = (pMarkAccess->getMarksBegin() + nBkmkTabPos)->get();
    // Ausgabe aller Bookmarks in diesem Absatz. Die Content-Position
    // wird vorerst nicht beruecksichtigt!
    sal_uInt32 nNode = pCurPam->GetPoint()->nNode.GetIndex();
    while( nBkmkTabPos != -1 &&
        pBookmark->GetMarkPos().nNode.GetIndex() == nNode )
    {
        // Der Bereich derBookmark wird erstam ignoriert, da er von uns
        // auch nicht eingelesen wird.

        // erst die SWG spezifischen Daten:
        if(dynamic_cast< const ::sw::mark::IBookmark* >(pBookmark) && !pBookmark->GetName().isEmpty() )
            OutAnchor( pBookmark->GetName() );

        if( ++nBkmkTabPos >= pMarkAccess->getMarksCount() )
            nBkmkTabPos = -1;
        else
            pBookmark = (pMarkAccess->getMarksBegin() + nBkmkTabPos)->get();
    }

    sal_uInt32 nPos;
    for( nPos = 0; nPos < aOutlineMarkPoss.size() &&
                   aOutlineMarkPoss[nPos] < nNode; nPos++ )
        ;

    while( nPos < aOutlineMarkPoss.size() && aOutlineMarkPoss[nPos] == nNode )
    {
        String sMark( aOutlineMarks[nPos] );
        sMark.SearchAndReplaceAll( '?', '_' );  // '?' causes problems in IE/Netscape 5
        OutAnchor( sMark );
        aOutlineMarkPoss.erase( aOutlineMarkPoss.begin()+nPos );
        aOutlineMarks.erase( aOutlineMarks.begin() + nPos );
    }
}

void SwHTMLWriter::OutImplicitMark( const String& rMark,
                                    const sal_Char *pMarkType )
{
    if( rMark.Len() && !aImplicitMarks.empty() )
    {
        String sMark( rMark );
        sMark.Append( cMarkSeparator );
        sMark.AppendAscii( pMarkType );
        if( 0 != aImplicitMarks.erase( sMark ) )
        {
            sMark.SearchAndReplaceAll( '?', '_' );  // '?' causes problems in IE/Netscape 5
            OutAnchor( sMark );
        }
    }
}

void SwHTMLWriter::OutHyperlinkHRefValue( const String& rURL )
{
    String sURL( rURL );
    xub_StrLen nPos = sURL.SearchBackward( cMarkSeparator );
    if( STRING_NOTFOUND != nPos )
    {
        String sCmp(comphelper::string::remove(sURL.Copy(nPos+1), ' '));
        if( sCmp.Len() )
        {
            sCmp.ToLowerAscii();
            if( sCmp.EqualsAscii( pMarkToRegion ) ||
                sCmp.EqualsAscii( pMarkToFrame ) ||
                sCmp.EqualsAscii( pMarkToGraphic ) ||
                sCmp.EqualsAscii( pMarkToOLE ) ||
                sCmp.EqualsAscii( pMarkToTable ) ||
                sCmp.EqualsAscii( pMarkToOutline ) ||
                sCmp.EqualsAscii( pMarkToText ) )
            {
                sURL.SearchAndReplaceAll( '?', '_' );   // '?' causes problems in IE/Netscape 5
            }
        }
    }

    sURL = URIHelper::simpleNormalizedMakeRelative( GetBaseURL(), sURL);
    HTMLOutFuncs::Out_String( Strm(), sURL, eDestEnc,
                              &aNonConvertableCharacters );
}

void SwHTMLWriter::OutBackground( const SvxBrushItem *pBrushItem, sal_Bool bGraphic )
{
    const Color &rBackColor = pBrushItem->GetColor();
    /// check, if background color is not "no fill"/"auto fill", instead of
    /// only checking, if transparency is not set.
    if( rBackColor.GetColor() != COL_TRANSPARENT )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_bgcolor).append('=');
        Strm() << sOut.makeStringAndClear().getStr();
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
        Strm() << " " OOO_STRING_SVTOOLS_HTML_O_background "=\"";
        Strm() << OOO_STRING_SVTOOLS_HTML_O_data ":";
        HTMLOutFuncs::Out_String( Strm(), aGraphicInBase64, eDestEnc, &aNonConvertableCharacters ) << '\"';
    }
}

void SwHTMLWriter::OutBackground( const SfxItemSet& rItemSet, sal_Bool bGraphic )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False,
                                               &pItem ))
    {
        OutBackground( ((const SvxBrushItem*)pItem), bGraphic );
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
        Strm() << sOut.makeStringAndClear().getStr();
        HTMLOutFuncs::Out_String( Strm(), LanguageTag::convertToBcp47(nLang),
                                  eDestEnc, &aNonConvertableCharacters ) << '"';
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
    const sal_Char *pValue = 0;
    switch( nDir )
    {
    case FRMDIR_HORI_LEFT_TOP:
    case FRMDIR_VERT_TOP_LEFT:
        pValue = "LTR";
        break;
    case FRMDIR_HORI_RIGHT_TOP:
    case FRMDIR_VERT_TOP_RIGHT:
        pValue = "RTL";
        break;
    }
    if( pValue != 0 )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_dir)
            .append("=\"").append(pValue).append('\"');
        Strm() << sOut.makeStringAndClear().getStr();
    }
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

void SwHTMLWriter::OutNewLine( sal_Bool bCheck )
{
    if( !bCheck || (Strm().Tell()-nLastLFPos) > nIndentLvl )
    {
        Strm() << sNewLine;
        nLastLFPos = Strm().Tell();
    }

    if( nIndentLvl && nIndentLvl <= MAX_INDENT_LEVEL)
    {
        sIndentTabs[nIndentLvl] = 0;
        Strm() << sIndentTabs;
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

// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
HTMLSaveData::HTMLSaveData( SwHTMLWriter& rWriter, sal_uLong nStt,
                            sal_uLong nEnd, sal_Bool bSaveNum,
                                const SwFrmFmt *pFrmFmt ) :
    rWrt( rWriter ),
    pOldPam( rWrt.pCurPam ),
    pOldEnd( rWrt.GetEndPaM() ),
    pOldNumRuleInfo( 0 ),
    pOldNextNumRuleInfo( 0 ),
    nOldDefListLvl( rWrt.nDefListLvl ),
    nOldDirection( rWrt.nDirection ),
    bOldOutHeader( rWrt.bOutHeader ),
    bOldOutFooter( rWrt.bOutFooter ),
    bOldOutFlyFrame( rWrt.bOutFlyFrame )
{
    bOldWriteAll = rWrt.bWriteAll;

    rWrt.pCurPam = rWrt.NewSwPaM( *rWrt.pDoc, nStt, nEnd );

    // Tabelle in Sonderbereichen erkennen
    if( nStt != rWrt.pCurPam->GetMark()->nNode.GetIndex() )
    {
        const SwNode *pNd = rWrt.pDoc->GetNodes()[ nStt ];
        if( pNd->IsTableNode() || pNd->IsSectionNode() )
            rWrt.pCurPam->GetMark()->nNode = nStt;
    }

    rWrt.SetEndPaM( rWrt.pCurPam );
    rWrt.pCurPam->Exchange( );
    rWrt.bWriteAll = sal_True;
    rWrt.nDefListLvl = 0;
    rWrt.bOutHeader = rWrt.bOutFooter = sal_False;

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

    if( pFrmFmt )
        rWrt.nDirection = rWrt.GetHTMLDirection( pFrmFmt->GetAttrSet() );
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


void GetHTMLWriter( const String&, const String& rBaseURL, WriterRef& xRet )
{
    xRet = new SwHTMLWriter( rBaseURL );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
