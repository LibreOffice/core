/*************************************************************************
 *
 *  $RCSfile: wrthtml.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 10:15:21 $
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

#include <stdlib.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _LINKMGR_HXX //autogen
#include <so3/linkmgr.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _FRMHTMLW_HXX //autogen
#include <sfx2/frmhtmlw.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif


#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLERLISTENER_HPP_
#include <com/sun/star/form/XFormControllerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // ...Percent()
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>      // fuer SwBookmark ...
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTHTML_HXX
#include <wrthtml.hxx>
#endif
#ifndef _HTMLNUM_HXX
#include <htmlnum.hxx>
#endif
#ifndef _HTMLFLY_HXX
#include <htmlfly.hxx>
#endif

#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>      // ResId fuer Statusleiste
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

#define MAX_INDENT_LEVEL 20

#if defined(MAC)
const sal_Char SwHTMLWriter::sNewLine = '\015';
#elif defined(UNX)
const sal_Char SwHTMLWriter::sNewLine = '\012';
#else
const sal_Char __FAR_DATA SwHTMLWriter::sNewLine[] = "\015\012";
#endif

static sal_Char __FAR_DATA sIndentTabs[MAX_INDENT_LEVEL+2] =
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

SwHTMLWriter::SwHTMLWriter()
{
    bFirstLine = sal_True;
    nBkmkTabPos = USHRT_MAX;
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


__EXPORT SwHTMLWriter::~SwHTMLWriter()
{
    delete pNumRuleInfo;
}

sal_uInt32 SwHTMLWriter::WriteStream()
{
    // neue Konfiguration setzen
    OfaHtmlOptions* pHtmlOptions = OFF_APP()->GetHtmlOptions();

    // die Fontgroessen 1-7
    aFontHeights[0] = pHtmlOptions->GetFontSize( 0 ) * 20;
    aFontHeights[1] = pHtmlOptions->GetFontSize( 1 ) * 20;
    aFontHeights[2] = pHtmlOptions->GetFontSize( 2 ) * 20;
    aFontHeights[3] = pHtmlOptions->GetFontSize( 3 ) * 20;
    aFontHeights[4] = pHtmlOptions->GetFontSize( 4 ) * 20;
    aFontHeights[5] = pHtmlOptions->GetFontSize( 5 ) * 20;
    aFontHeights[6] = pHtmlOptions->GetFontSize( 6 ) * 20;

    // ueberhaupt Styles ausgeben
    // (dann auch obere und untere Absatz-Abstaende)
    nExportMode = pHtmlOptions->GetExportMode();
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
        nHTMLMode |= HTMLMODE_ABS_POS_FLY;

    if( HTML_CFG_WRITER==nExportMode || HTML_CFG_MSIE==nExportMode )
        nHTMLMode |= HTMLMODE_ABS_POS_DRAW;

    if( HTML_CFG_WRITER==nExportMode )
//      nHTMLMode |= HTMLMODE_FLY_MARGINS | HTMLMODE_FRSTLINE_IN_NUMBUL;
        nHTMLMode |= HTMLMODE_FLY_MARGINS;

    if( HTML_CFG_NS40==nExportMode )
        nHTMLMode |= HTMLMODE_BORDER_NONE;

    if( HTML_CFG_HTML32!=nExportMode )
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
        ByteString aTmp( pHelpHack );
        if( aTmp.EqualsIgnoreCaseAscii( "Hilfe" ) )
            nHTMLMode |= HTMLMODE_NO_BR_AT_PAREND;
    }

    eCSS1Unit = (FieldUnit)SW_MOD()->GetMetric( pDoc->IsHTMLMode() );

    sal_Bool bWriteUTF8 = bWriteClipboardDoc;
    eDestEnc = bWriteUTF8 ? RTL_TEXTENCODING_UTF8
                          : pHtmlOptions->GetTextEncoding();
    const sal_Char *pCharSet =
        rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );
    eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    // fuer Netscape optimieren heisst Spacer- und Multicol ausgeben
//  bCfgMultiCol = pHtmlOptions->IsNetscape3();
//  bCfgSpacer = pHtmlOptions->IsNetscape3();

    // wenn Styles exportiert werden, wird ein Style einem HTML-Tag manchmal
    // vorgezogen, wenn nicht fuer Netscape exportiert wird
    // bCfgPreferStyles = bCfgOutStyles; // && !pHtmlOptions->IsNetscape3();

    // Nur noch fuer den MS-IE ziehen wir den Export von Styles vor.
    bCfgPreferStyles = HTML_CFG_MSIE==nExportMode;

    bCfgStarBasic = pHtmlOptions->IsStarBasic();

    bCfgFormFeed = !IsHTMLMode(HTMLMODE_PRINT_EXT);
    bCfgCpyLinkedGrfs = pHtmlOptions->IsSaveGraphicsLocal();

    // die HTML-Vorlage holen
    sal_Bool bOldHTMLMode;
    sal_uInt16 nOldTxtFmtCollCnt, nOldCharFmtCnt;

    ASSERT( !pTemplate, "Wo kommt denn die HTML-Vorlage hier her?" );
    pTemplate = ((HTMLReader*)ReadHTML)->GetTemplateDoc();
    if( pTemplate )
    {
        pTemplate->AddLink();
        bOldHTMLMode = pTemplate->IsHTMLMode();
        pTemplate->SetHTMLMode( sal_True );

        nOldTxtFmtCollCnt = pTemplate->GetTxtFmtColls()->Count();
        nOldCharFmtCnt = pTemplate->GetCharFmts()->Count();
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
        ->GetTxtCollFromPoolSimple( RES_POOLCOLL_HTML_DD, FALSE )
        ->GetLRSpace().GetTxtLeft();
    nHeaderFooterSpace = 0;
    nTxtAttrsToIgnore = 0;
    nCSS1OutMode = 0;
    sal_uInt16 nScript = SvtLanguageOptions::GetScriptTypeOfLanguage(
            static_cast< LanguageType >( GetAppLanguage() ) );
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

    ByteString aStartTags;

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
                ASSERT( FILE_LINK_SECTION != pSNd->GetSection().GetType(),
                        "Export gelinkter Bereiche am Dok-Anfang ist nicht implemntiert" );

                // nur das Tag fuer die Section merken
                ByteString aName;
                HTMLOutFuncs::ConvertStringToHTML( pSNd->GetSection().GetName(),
                                                   aName, eDestEnc, &aNonConvertableCharacters );

                ByteString sOut( '<' );
                (((((((sOut += sHTML_division)
                    += ' ') += sHTML_O_id) += "=\"")
                    += aName) += '\"')
                    += '>') += aStartTags;

                aStartTags = sOut;
            }
            // FindSectionNode() an einem SectionNode liefert den selben!
            pSNd = pSNd->FindStartNode()->FindSectionNode();
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

    if( aStartTags.Len() )
        Strm() << aStartTags.GetBuffer();

    const SfxPoolItem *pItem;
    const SfxItemSet& rPageItemSet = pCurrPageDesc->GetMaster().GetAttrSet();
    if( !bWriteClipboardDoc && pDoc->GetDocShell() &&
        ( pDoc->IsHeadInBrowse() ||
          (!pDoc->IsHTMLMode() && !pDoc->IsBrowseMode()) ) &&
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
        ( pDoc->IsFootInBrowse() ||
          (!pDoc->IsHTMLMode() && !pDoc->IsBrowseMode()) ) &&
        SFX_ITEM_SET == rPageItemSet.GetItemState( RES_FOOTER, sal_True, &pItem) )
    {
        const SwFrmFmt *pFooterFmt =
            ((const SwFmtFooter *)pItem)->GetFooterFmt();
        if( pFooterFmt )
            OutHTML_HeaderFooter( *this, *pFooterFmt, sal_False );
    }

    if( bLFPossible )
        OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_body, sal_False );
    OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_html, sal_False );

    if( aNonConvertableCharacters.Len() )
    {
        nWarn = *new StringErrorInfo( WARN_UNCONVERTABLE_CHARS,
                                      aNonConvertableCharacters,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }

    // loesche die Tabelle mit den freifliegenden Rahmen
    sal_uInt16 i;
    ASSERT( !pHTMLPosFlyFrms, "Wurden nicht alle Rahmen ausgegeben" );
    if( pHTMLPosFlyFrms )
    {
        pHTMLPosFlyFrms->DeleteAndDestroy( 0, pHTMLPosFlyFrms->Count() );
        delete pHTMLPosFlyFrms;
        pHTMLPosFlyFrms = 0;
    }

    if( aHTMLControls.Count() )
        aHTMLControls.DeleteAndDestroy( sal_uInt16(0), aHTMLControls.Count() );

    if( aChrFmtInfos.Count() )
        aChrFmtInfos.DeleteAndDestroy( sal_uInt16(0), aChrFmtInfos.Count() );

    if( aTxtCollInfos.Count() )
        aTxtCollInfos.DeleteAndDestroy( sal_uInt16(0), aTxtCollInfos.Count() );

    if( aImgMapNames.Count() )
        aImgMapNames.DeleteAndDestroy( sal_uInt16(0), aImgMapNames.Count() );

    if( aImplicitMarks.Count() )
        aImplicitMarks.DeleteAndDestroy( sal_uInt16(0), aImplicitMarks.Count() );

    if( aOutlineMarks.Count() )
        aOutlineMarks.DeleteAndDestroy( sal_uInt16(0), aOutlineMarks.Count() );

    if( aOutlineMarkPoss.Count() )
        aOutlineMarkPoss.Remove( sal_uInt16(0), aOutlineMarkPoss.Count() );

    if( aNumRuleNames.Count() )
        aNumRuleNames.DeleteAndDestroy( sal_uInt16(0), aNumRuleNames.Count() );

    if( aScriptParaStyles.Count() )
        aScriptParaStyles.DeleteAndDestroy( sal_uInt16(0), aScriptParaStyles.Count() );
    if( aScriptTextStyles.Count() )
        aScriptTextStyles.DeleteAndDestroy( sal_uInt16(0), aScriptTextStyles.Count() );

    delete pDfltColor;
    pDfltColor = 0;

    delete pStartNdIdx;
    pStartNdIdx = 0;

    delete pxFormComps;
    pxFormComps = 0;

    ASSERT( !pFootEndNotes,
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
        sal_uInt16 nTxtFmtCollCnt = pTemplate->GetTxtFmtColls()->Count();
        while( nTxtFmtCollCnt > nOldTxtFmtCollCnt )
            pTemplate->DelTxtFmtColl( --nTxtFmtCollCnt );
        ASSERT( pTemplate->GetTxtFmtColls()->Count() == nOldTxtFmtCollCnt,
                "falsche Anzahl TxtFmtColls geloescht" );

        sal_uInt16 nCharFmtCnt = pTemplate->GetCharFmts()->Count();
        while( nCharFmtCnt > nOldCharFmtCnt )
            pTemplate->DelCharFmt( --nCharFmtCnt );
        ASSERT( pTemplate->GetCharFmts()->Count() == nOldCharFmtCnt,
                "falsche Anzahl CharFmts geloescht" );

        // HTML-Modus wieder restaurieren
        pTemplate->SetHTMLMode( bOldHTMLMode );

        if( 0 == pTemplate->RemoveLink() )
            delete pTemplate;

        pTemplate = 0;
    }

    return nWarn;
}

const SwFmtCol *lcl_html_GetFmtCol( const SwHTMLWriter& rHTMLWrt,
                                       const SwSection& rSection,
                                       const SwSectionFmt& rFmt )
{
    const SwFmtCol *pCol = 0;

    const SfxPoolItem* pItem;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_FRM_COLUMNS ) &&
        FILE_LINK_SECTION != rSection.GetType() &&
        SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(RES_COL,sal_False,&pItem) &&
        ((const SwFmtCol *)pItem)->GetNumCols() > 1 )
    {
        pCol = (const SwFmtCol *)pItem;
    }

    return pCol;
}

sal_Bool lcl_html_IsMultiColStart( const SwHTMLWriter& rHTMLWrt, sal_uInt32 nIndex )
{
    sal_Bool bRet = sal_False;
    const SwSectionNode *pSectNd =
        rHTMLWrt.pDoc->GetNodes()[nIndex]->GetSectionNode();
    if( pSectNd )
    {
        const SwSection& rSection = pSectNd->GetSection();
        const SwSectionFmt *pFmt = rSection.GetFmt();
        if( pFmt && lcl_html_GetFmtCol( rHTMLWrt, rSection, *pFmt ) )
            bRet = sal_True;
    }

    return bRet;
}

sal_Bool lcl_html_IsMultiColEnd( const SwHTMLWriter& rHTMLWrt, sal_uInt32 nIndex )
{
    sal_Bool bRet = sal_False;
    const SwEndNode *pEndNd = rHTMLWrt.pDoc->GetNodes()[nIndex]->GetEndNode();
    if( pEndNd )
        bRet = lcl_html_IsMultiColStart( rHTMLWrt,
                                         pEndNd->StartOfSectionIndex() );

    return bRet;
}


void lcl_html_OutSectionStartTag( SwHTMLWriter& rHTMLWrt,
                                     const SwSection& rSection,
                                     const SwSectionFmt& rFmt,
                                  const SwFmtCol *pCol,
                                  sal_Bool bContinued=sal_False )
{
    ASSERT( pCol || !bContinued, "Continuation of DIV" );

    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();

    const sal_Char *pTag = pCol ? sHTML_multicol : sHTML_division;

    ByteString sOut( '<' );
    sOut += pTag;

    const String& rName = rSection.GetName();
    if( rName.Len() && !bContinued )
    {
        ((sOut += ' ') += sHTML_O_id) += "=\"";
        rHTMLWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rHTMLWrt.Strm(), rName, rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );
        sOut = '\"';
    }

    sal_uInt16 nDir = rHTMLWrt.GetHTMLDirection( rFmt.GetAttrSet() );
    rHTMLWrt.Strm() << sOut.GetBuffer();
    sOut.Erase();
    rHTMLWrt.OutDirection( nDir );

    if( FILE_LINK_SECTION == rSection.GetType() )
    {
        ((sOut += ' ') += sHTML_O_href) += "=\"";
        rHTMLWrt.Strm() << sOut.GetBuffer();

        const String& aFName = rSection.GetLinkFileName();
        String aURL( aFName.GetToken(0,so3::cTokenSeperator) );
        String aFilter( aFName.GetToken(1,so3::cTokenSeperator) );
        String aSection( aFName.GetToken(2,so3::cTokenSeperator) );

        String aEncURL( INetURLObject::AbsToRel(aURL,
                                          INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS ) );
        sal_Unicode cDelim = 255U;
        sal_Bool bURLContainsDelim =
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
        sOut = '\"';
    }
    else if( pCol )
    {
        (((sOut += ' ') += sHTML_O_cols) += '=')
            += ByteString::CreateFromInt32( pCol->GetNumCols() );

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
            (((sOut += ' ') += sHTML_O_gutter) += '=')
                += ByteString::CreateFromInt32( nGutter );
        }
    }

    rHTMLWrt.Strm() << sOut.GetBuffer();
    if( rHTMLWrt.IsHTMLMode( rHTMLWrt.bCfgOutStyles ) )
        rHTMLWrt.OutCSS1_SectionFmtOptions( rFmt );

    rHTMLWrt.Strm() << '>';

    rHTMLWrt.bLFPossible = sal_True;
    if( rName.Len() && !bContinued )
        rHTMLWrt.OutImplicitMark( rName, pMarkToRegion );

    rHTMLWrt.IncIndentLevel();
}

void lcl_html_OutSectionEndTag( SwHTMLWriter& rHTMLWrt,
                                const SwFmtCol *pCol )
{
    const sal_Char *pTag = pCol ? sHTML_multicol : sHTML_division;

    rHTMLWrt.DecIndentLevel();
    if( rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), pTag, sal_False );
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
    ASSERT( pFmt, "Section without a format?" );

    sal_Bool bStartTag = sal_True;
    sal_Bool bEndTag = sal_True;
    const SwSectionFmt *pSurrFmt = 0;
    const SwSectionNode *pSurrSectNd = 0;
    const SwSection *pSurrSection = 0;
    const SwFmtCol *pSurrCol = 0;

    sal_uInt32 nSectSttIdx = rSectNd.GetIndex();
    sal_uInt32 nSectEndIdx = rSectNd.EndOfSectionIndex();
    const SwFmtCol *pCol = lcl_html_GetFmtCol( rHTMLWrt, rSection, *pFmt );
    if( pCol )
    {
        // If the next node is a columned section node, too, don't export
        // an empty section.
        if( lcl_html_IsMultiColStart( rHTMLWrt, nSectSttIdx+1 ) )
            bStartTag = sal_False;

        // The same applies if the section end with another columned section.
        if( lcl_html_IsMultiColEnd( rHTMLWrt, nSectEndIdx-1 ) )
            bEndTag = sal_False;

        //.is there a columned section arround this one?
        const SwStartNode *pSttNd = rSectNd.FindStartNode();
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
                        pSurrCol = lcl_html_GetFmtCol( rHTMLWrt, *pSurrSection,
                                                       *pSurrFmt );
                }
            }
        }
    }

    // The surrounding section must be closed before the current one is
    // opended, except that it start immediatly before the current one or
    // another end immediately before the current one
    if( pSurrCol && nSectSttIdx - pSurrSectNd->GetIndex() > 1 &&
        !lcl_html_IsMultiColEnd( rHTMLWrt, nSectSttIdx-1 ) )
        lcl_html_OutSectionEndTag( rHTMLWrt, pSurrCol );

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
        lcl_html_OutSectionEndTag( rHTMLWrt, pCol );

    // The surrounding section must be started again, except that it ends
    // immeditaly behind the current one.
    if( pSurrCol &&
        pSurrSectNd->EndOfSectionIndex() - nSectEndIdx > 1 &&
        !lcl_html_IsMultiColStart( rHTMLWrt, nSectEndIdx+1 ) )
        lcl_html_OutSectionStartTag( rHTMLWrt, *pSurrSection, *pSurrFmt,
                                     pSurrCol, sal_True );

    return rWrt;
}

void SwHTMLWriter::Out_SwDoc( SwPaM* pPam )
{
    sal_Bool bSaveWriteAll = bWriteAll;     // sichern


    // suche die naechste ::com::sun::star::text::Bookmark-Position aus der ::com::sun::star::text::Bookmark-Tabelle
    nBkmkTabPos = bWriteAll ? FindPos_Bkmk( *pCurPam->GetPoint() ) : USHRT_MAX;

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

            ASSERT( !(pNd->IsGrfNode() || pNd->IsOLENode()),
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
            }
            else if( pNd->IsSectionNode() )
            {
                OutHTML_Section( *this, *pNd->GetSectionNode() );
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
    const SfxPoolItem *pRefItem, *pItem;
    sal_Bool bItemSet = SFX_ITEM_SET == rItemSet.GetItemState( RES_CHRATR_COLOR,
                                                           sal_True, &pItem);
    sal_Bool bRefItemSet = pRefFmt &&
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
        ByteString sOut( ' ' );
        (sOut += pTag) += '=';
        rHWrt.Strm() << sOut.GetBuffer();
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
    sal_uInt32 nIdx = pCurPam->GetPoint()->nNode.GetIndex();
    sal_uInt32 nEndIdx = pCurPam->GetMark()->nNode.GetIndex();

    SwTxtNode *pTxtNd = 0;
    while( nIdx<=nEndIdx &&
            0==(pTxtNd=pDoc->GetNodes()[nIdx]->GetTxtNode()) )
        nIdx++;

    ASSERT( pTxtNd, "Kein Text-Node gefunden" );
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
    ByteString sOut( sHTML_doctype );
    (sOut += ' ') +=
        (HTML_CFG_HTML32==nExportMode ? sHTML_doctype32
                                       : sHTML_doctype40);
    HTMLOutFuncs::Out_AsciiTag( Strm(), sOut.GetBuffer() );

    // baue den Vorspann
    OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_html );

    OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_head );

    IncIndentLevel();   // Inhalt von <HEAD> einruecken

    // DokumentInfo
    ByteString sIndent;
    GetIndentString( sIndent );
//  OutNewLine();
    SfxFrameHTMLWriter::Out_DocInfo( Strm(), pDoc->GetpInfo(),
                                     sIndent.GetBuffer(), eDestEnc,
                                       &aNonConvertableCharacters );

    // Kommentare und Meta-Tags des ersten Absatzes
    rHeaderAttrs = OutHeaderAttrs();

    OutFootEndNoteInfo();

    const SwPageDesc *pPageDesc = 0;
    //if( !pDoc->IsHTMLMode() )
    //{
        // In Nicht-HTML-Dokumenten wird die erste gesetzte Seitenvorlage
        // exportiert und wenn keine gesetzt ist die Standard-Vorlage
        sal_uInt32 nNodeIdx = pCurPam->GetPoint()->nNode.GetIndex();

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
    //}
    //else
    //{
        // In HTML-Dokumenten nehmen wir immer die HTML-Vorlage
    //  pPageDesc = pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML );
    //}

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
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_head, sal_False );

    // der Body wird nicht eingerueckt, weil sonst alles eingerueckt waere!
    OutNewLine();
    sOut = '<';
    sOut += sHTML_body;
    Strm() << sOut.GetBuffer();
    sOut.Erase();

    // language
    OutLanguage( eLang );

    // Textfarbe ausgeben, wenn sie an der Standard-Vorlage gesetzt ist
    // und sich geaendert hat.
    OutBodyColor( sHTML_O_text,
                  pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD,
                                                  FALSE ),
                  *this );

    // Farben fuer (un)besuchte Links
    OutBodyColor( sHTML_O_link,
                  pDoc->GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL ),
                  *this );
    OutBodyColor( sHTML_O_vlink,
                  pDoc->GetCharFmtFromPool( RES_POOLCHR_INET_VISIT ),
                  *this );

    const SfxItemSet& rItemSet = pPageDesc->GetMaster().GetAttrSet();

    String aEmbGrfName;
    OutBackground( rItemSet, aEmbGrfName, sal_True );

    nDirection = GetHTMLDirection( rItemSet );
    OutDirection( nDirection );

    if( bCfgOutStyles )
        OutCSS1_BodyTagStyleOpt( *this, rItemSet, aEmbGrfName );

    // Events anhaengen
    if( pDoc->GetDocShell() )   // nur mit DocShell ist Basic moeglich
        OutBasicBodyEvents();

    Strm() << '>';

    return pPageDesc;
}

void SwHTMLWriter::OutAnchor( const String& rName )
{
    ByteString sOut( '<' );
    (((sOut += sHTML_anchor) += ' ') += sHTML_O_name) += "=\"";
    Strm() << sOut.GetBuffer();
    HTMLOutFuncs::Out_String( Strm(), rName, eDestEnc, &aNonConvertableCharacters ) << "\">";
    HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_anchor, sal_False );
}

void SwHTMLWriter::OutBookmarks()
{
    // hole das aktuelle Bookmark
    const SwBookmark* pBookmark = USHRT_MAX != nBkmkTabPos ?
                            pDoc->GetBookmarks()[ nBkmkTabPos ] : 0;
    // Ausgabe aller Bookmarks in diesem Absatz. Die Content-Position
    // wird vorerst nicht beruecksichtigt!
    sal_uInt32 nNode = pCurPam->GetPoint()->nNode.GetIndex();
    while( USHRT_MAX != nBkmkTabPos &&
        pBookmark->GetPos().nNode.GetIndex() == nNode )
    {
        // Der Bereich derBookmark wird erstam ignoriert, da er von uns
        // auch nicht eingelesen wird.

        // erst die SWG spezifischen Daten:
        if( pBookmark->IsBookMark() && pBookmark->GetName().Len() )
            OutAnchor( pBookmark->GetName() );

        if( ++nBkmkTabPos >= pDoc->GetBookmarks().Count() )
            nBkmkTabPos = USHRT_MAX;
        else
            pBookmark = pDoc->GetBookmarks()[ nBkmkTabPos ];
    }

    sal_uInt16 nPos;
    for( nPos = 0; nPos < aOutlineMarkPoss.Count() &&
                   aOutlineMarkPoss[nPos] < nNode; nPos++ )
        ;

    while( nPos < aOutlineMarkPoss.Count() && aOutlineMarkPoss[nPos] == nNode )
    {
        String sMark( *aOutlineMarks[nPos] );
        sMark.SearchAndReplaceAll( '?', '_' );  // '?' causes problems in IE/Netscape 5
        OutAnchor( sMark );
        aOutlineMarkPoss.Remove( nPos, 1 );
        aOutlineMarks.DeleteAndDestroy( nPos, 1 );
    }
}

void SwHTMLWriter::OutImplicitMark( const String& rMark,
                                    const sal_Char *pMarkType )
{
    if( rMark.Len() && aImplicitMarks.Count() )
    {
        String sMark( rMark );
        sMark.Append( cMarkSeperator );
        sMark.AppendAscii( pMarkType );
        sal_uInt16 nPos;
        if( aImplicitMarks.Seek_Entry( &sMark, &nPos ) )
        {
            sMark.SearchAndReplaceAll( '?', '_' );  // '?' causes problems in IE/Netscape 5
            OutAnchor( sMark );
            aImplicitMarks.DeleteAndDestroy( nPos, 1 );
        }
    }
}

void SwHTMLWriter::OutHyperlinkHRefValue( const String& rURL )
{
    String sURL( rURL );
    xub_StrLen nPos = sURL.SearchBackward( cMarkSeperator );
    if( STRING_NOTFOUND != nPos )
    {
        String sCmp( sURL.Copy( nPos+1 ) );
        sCmp.EraseAllChars();
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

    sURL = INetURLObject::AbsToRel( sURL, INetURLObject::WAS_ENCODED,
                                    INetURLObject::DECODE_UNAMBIGUOUS);
    HTMLOutFuncs::Out_String( Strm(), sURL, eDestEnc,
                              &aNonConvertableCharacters );
}

void SwHTMLWriter::OutBackground( const SvxBrushItem *pBrushItem,
                                  String& rEmbGrfNm, sal_Bool bGraphic )
{
    const Color &rBackColor = pBrushItem->GetColor();
    /// OD 02.09.2002 #99657#
    /// check, if background color is not "no fill"/"auto fill", instead of
    /// only checking, if transparency is not set.
    if( rBackColor.GetColor() != COL_TRANSPARENT )
    {
        ByteString sOut( ' ' );
        (sOut += sHTML_O_bgcolor) += '=';
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_Color( Strm(), rBackColor, eDestEnc);
    }

    if( !bGraphic )
        return;

    const String *pLink = pBrushItem->GetGraphicLink();

    // embeddete Grafik -> WriteEmbedded schreiben
    if( !pLink )
    {
        const Graphic* pGrf = pBrushItem->GetGraphic();
        if( pGrf )
        {
            // Grafik als (JPG-)File speichern
            const String* pTempFileName = GetOrigFileName();
            if(pTempFileName)
                rEmbGrfNm = *pTempFileName;
            sal_uInt16 nErr = XOutBitmap::WriteGraphic( *pGrf, rEmbGrfNm,
                    String::CreateFromAscii( "JPG" ),
                    XOUTBMP_USE_NATIVE_IF_POSSIBLE );
            if( !nErr )     // fehlerhaft, da ist nichts auszugeben
            {
                rEmbGrfNm = URIHelper::SmartRelToAbs( rEmbGrfNm );
                pLink = &rEmbGrfNm;
            }
            else
            {
                nWarn = WARN_SWG_POOR_LOAD | WARN_SW_WRITE_BASE;
            }
        }
    }
    else
    {
        rEmbGrfNm = *pLink;
        if( bCfgCpyLinkedGrfs )
        {
            CopyLocalFileToINet( rEmbGrfNm  );
            pLink = &rEmbGrfNm;
        }
    }

    if( pLink )
    {
        ByteString sOut( ' ' );
        String s( INetURLObject::AbsToRel( *pLink,
                                        INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_UNAMBIGUOUS));
        (sOut += sHTML_O_background) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), s, eDestEnc, &aNonConvertableCharacters ) << '\"';
    }
}

void SwHTMLWriter::OutBackground( const SfxItemSet& rItemSet,
                                  String& rEmbGrfNm, sal_Bool bGraphic )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False,
                                               &pItem ))
    {
        OutBackground( ((const SvxBrushItem*)pItem), rEmbGrfNm, bGraphic );
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

void SwHTMLWriter::OutLanguage( LanguageType eLang )
{
    if( LANGUAGE_DONTKNOW != eLang )
    {
        ByteString sOut( ' ' );
        (sOut += sHTML_O_lang) += "=\"";
        Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( Strm(), ConvertLanguageToIsoString(eLang),
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
        ByteString sOut( ' ' );
        (((sOut += sHTML_O_dir) += "=\"") += pValue) += '\"';
        Strm() << sOut.GetBuffer();
    }
}

void SwHTMLWriter::GetIndentString( ByteString& rStr, sal_uInt16 nIncLvl )
{
    // etwas umstaendlich, aber wir haben nur einen Indent-String!
    sal_uInt16 nLevel = nIndentLvl + nIncLvl;

    if( nLevel && nLevel <= MAX_INDENT_LEVEL)
    {
        sIndentTabs[nLevel] = 0;
        rStr = sIndentTabs;
        sIndentTabs[nLevel] = '\t';
    }
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
HTMLSaveData::HTMLSaveData( SwHTMLWriter& rWriter, sal_uInt32 nStt,
                            sal_uInt32 nEnd, sal_Bool bSaveNum,
                                const SwFrmFmt *pFrmFmt )
    : rWrt( rWriter ),
    pOldPam( rWrt.pCurPam ), pOldEnd( rWrt.GetEndPaM() ),
    pOldNumRuleInfo( 0 ),
    pOldNextNumRuleInfo( 0 ),
    nOldDirection( rWrt.nDirection ),
    nOldDefListLvl( rWrt.nDefListLvl ),
    bOldOutHeader( rWrt.bOutHeader ), bOldOutFooter( rWrt.bOutFooter ),
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


void GetHTMLWriter( const String&, WriterRef& xRet )
{
    xRet = new SwHTMLWriter;
}


