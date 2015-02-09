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

#include "rtfexportfilter.hxx"
#include "rtfsdrexport.hxx"
#include "rtfattributeoutput.hxx"
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <ndtxt.hxx>
#include <fmtpdsc.hxx>
#include <section.hxx>
#include <pagedesc.hxx>
#include <ftninfo.hxx>
#include <fmthdft.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/protitem.hxx>
#include <numrule.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>
#include <IDocumentLayoutAccess.hxx>
#include "ww8par.hxx"
#include <comphelper/string.hxx>
#include <svtools/rtfkeywd.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <unotools/docinfohelper.hxx>
#include <docary.hxx>
#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#endif
#include <svx/xflclit.hxx>

using ::editeng::SvxBorderLine;
using namespace ::comphelper;
using namespace ::com::sun::star;

using sw::mark::IMark;

// the default text encoding for the export, if it doesn't fit unicode will
// be used
#define DEF_ENCODING            RTL_TEXTENCODING_ASCII_US

AttributeOutputBase& RtfExport::AttrOutput() const
{
    return *m_pAttrOutput;
}

MSWordSections& RtfExport::Sections() const
{
    return *m_pSections;
}

RtfSdrExport& RtfExport::SdrExporter() const
{
    return *m_pSdrExport;
}

bool RtfExport::CollapseScriptsforWordOk(sal_uInt16 nScript, sal_uInt16 nWhich)
{
    // FIXME is this actually true for rtf? - this is copied from DOCX
    if (nScript == i18n::ScriptType::ASIAN)
    {
        // for asian in ww8, there is only one fontsize
        // and one fontstyle (posture/weight)
        switch (nWhich)
        {
        case RES_CHRATR_FONTSIZE:
        case RES_CHRATR_POSTURE:
        case RES_CHRATR_WEIGHT:
            return false;
        default:
            break;
        }
    }
    else if (nScript != i18n::ScriptType::COMPLEX)
    {
        // for western in ww8, there is only one fontsize
        // and one fontstyle (posture/weight)
        switch (nWhich)
        {
        case RES_CHRATR_CJK_FONTSIZE:
        case RES_CHRATR_CJK_POSTURE:
        case RES_CHRATR_CJK_WEIGHT:
            return false;
        default:
            break;
        }
    }
    return true;
}

void RtfExport::AppendBookmarks(const SwTxtNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen)
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if (GetBookmarks(rNode, nAktPos, nAktPos + nLen, aMarks))
    {
        for (IMarkVector::const_iterator it = aMarks.begin(), end = aMarks.end();
                it != end; ++it)
        {
            IMark* pMark = (*it);
            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if (nStart == nAktPos)
                aStarts.push_back(pMark->GetName());

            if (nEnd == nAktPos)
                aEnds.push_back(pMark->GetName());
        }
    }

    m_pAttrOutput->WriteBookmarks_Impl(aStarts, aEnds);
}

void RtfExport::AppendBookmark(const OUString& rName, bool /*bSkip*/)
{
    std::vector<OUString> aStarts;
    std::vector<OUString> aEnds;

    aStarts.push_back(rName);
    aEnds.push_back(rName);

    m_pAttrOutput->WriteBookmarks_Impl(aStarts, aEnds);
}

void RtfExport::AppendAnnotationMarks(const SwTxtNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen)
{
    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if (GetAnnotationMarks(rNode, nAktPos, nAktPos + nLen, aMarks))
    {
        for (IMarkVector::const_iterator it = aMarks.begin(), end = aMarks.end();
                it != end; ++it)
        {
            IMark* pMark = (*it);
            const sal_Int32 nStart = pMark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if (nStart == nAktPos)
                aStarts.push_back(pMark->GetName());

            if (nEnd == nAktPos)
                aEnds.push_back(pMark->GetName());
        }
    }

    m_pAttrOutput->WriteAnnotationMarks_Impl(aStarts, aEnds);
}

//For i120928,to export graphic of bullet for RTF filter
void RtfExport::ExportGrfBullet(const SwTxtNode&)
{
    // Noop, would be too late, see WriteNumbering() instead.
}

void RtfExport::WriteChar(sal_Unicode)
{
    /* WriteChar() has nothing to do for rtf. */
}

static bool IsExportNumRule(const SwNumRule& rRule, sal_uInt8* pEnd = 0)
{
    sal_uInt8 nEnd = MAXLEVEL;
    while (nEnd-- && !rRule.GetNumFmt(nEnd))
        ;
    ++nEnd;

    const SwNumFmt* pNFmt;
    sal_uInt8 nLvl;

    for (nLvl = 0; nLvl < nEnd; ++nLvl)
        if (SVX_NUM_NUMBER_NONE != (pNFmt = &rRule.Get(nLvl))
                ->GetNumberingType() || !pNFmt->GetPrefix().isEmpty() ||
                (!pNFmt->GetSuffix().isEmpty() && !pNFmt->GetSuffix().equals(".")))
            break;

    if (pEnd)
        *pEnd = nEnd;
    return nLvl != nEnd;
}

void RtfExport::BuildNumbering()
{
    const SwNumRuleTbl& rListTbl = pDoc->GetNumRuleTbl();

    for (sal_uInt16 n = rListTbl.size()+1; n;)
    {
        SwNumRule* pRule;
        --n;
        if (n == rListTbl.size())
            pRule = (SwNumRule*)pDoc->GetOutlineNumRule();
        else
        {
            pRule = rListTbl[ n ];
            if (!pDoc->IsUsed(*pRule))
                continue;
        }

        if (IsExportNumRule(*pRule))
            GetId(*pRule);
    }
}

void RtfExport::WriteNumbering()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    if (!pUsedNumTbl)
        return; // no numbering is used

    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTTABLE);

    CollectGrfsOfBullets();
    if (!m_vecBulletPic.empty())
        Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE).WriteCharPtr(LO_STRING_SVTOOLS_RTF_LISTPICTURE);
    BulletDefinitions();
    if (!m_vecBulletPic.empty())
        Strm().WriteChar('}');

    AbstractNumberingDefinitions();
    Strm().WriteChar('}');

    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LISTOVERRIDETABLE);
    NumberingDefinitions();
    Strm().WriteChar('}');

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteRevTab()
{
    int nRevAuthors = pDoc->getIDocumentRedlineAccess().GetRedlineTbl().size();

    if (nRevAuthors < 1)
        return;

    // RTF always seems to use Unknown as the default first entry
    GetRedline(OUString("Unknown"));

    for (sal_uInt16 i = 0; i < pDoc->getIDocumentRedlineAccess().GetRedlineTbl().size(); ++i)
    {
        const SwRangeRedline* pRedl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl()[ i ];

        GetRedline(SW_MOD()->GetRedlineAuthor(pRedl->GetAuthor()));
    }

    // Now write the table
    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_REVTBL).WriteChar(' ');
    for (sal_uInt16 i = 0; i < m_aRedlineTbl.size(); ++i)
    {
        const OUString* pAuthor = GetRedline(i);
        Strm().WriteChar('{');
        if (pAuthor)
            Strm().WriteCharPtr(msfilter::rtfutil::OutString(*pAuthor, eDefaultEncoding).getStr());
        Strm().WriteCharPtr(";}");
    }
    Strm().WriteChar('}').WriteCharPtr(SAL_NEWLINE_STRING);
}

void RtfExport::WriteHeadersFooters(sal_uInt8 nHeadFootFlags,
                                    const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, sal_uInt8 /*nBreakCode*/)
{
    // headers
    if (nHeadFootFlags & nsHdFtFlags::WW8_HEADER_EVEN)
        WriteHeaderFooter(rLeftFmt, true, OOO_STRING_SVTOOLS_RTF_HEADERL);

    if (nHeadFootFlags & nsHdFtFlags::WW8_HEADER_ODD)
        WriteHeaderFooter(rFmt, true, OOO_STRING_SVTOOLS_RTF_HEADER);

    if (nHeadFootFlags & nsHdFtFlags::WW8_HEADER_FIRST)
        WriteHeaderFooter(rFirstPageFmt, true, OOO_STRING_SVTOOLS_RTF_HEADERF, true);

    // footers
    if (nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_EVEN)
        WriteHeaderFooter(rLeftFmt, false, OOO_STRING_SVTOOLS_RTF_FOOTERL);

    if (nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_ODD)
        WriteHeaderFooter(rFmt, false, OOO_STRING_SVTOOLS_RTF_FOOTER);

    if (nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_FIRST)
        WriteHeaderFooter(rFirstPageFmt, false, OOO_STRING_SVTOOLS_RTF_FOOTERF, true);
}

void RtfExport::OutputField(const SwField* pFld, ww::eField eFldType, const OUString& rFldCmd, sal_uInt8 nMode)
{
    m_pAttrOutput->WriteField_Impl(pFld, eFldType, rFldCmd, nMode);
}

void RtfExport::WriteFormData(const ::sw::mark::IFieldmark& /*rFieldmark*/)
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfExport::WriteHyperlinkData(const ::sw::mark::IFieldmark& /*rFieldmark*/)
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfExport::DoComboBox(const OUString& /*rName*/,
                           const OUString& /*rHelp*/,
                           const OUString& /*rToolTip*/,
                           const OUString& /*rSelected*/,
                           uno::Sequence<OUString>& /*rListItems*/)
{
    // this is handled in RtfAttributeOutput::OutputFlyFrame_Impl
}

void RtfExport::DoFormText(const SwInputField* pFld)
{
    OUString sResult = pFld->ExpandField(true);
    OUString sHelp(pFld->GetHelp());
    OUString sName = pFld->GetPar2();
    OUString sStatus = pFld->GetToolTip();
    m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_FIELD "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST "{ FORMTEXT }");
    m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD " {" OOO_STRING_SVTOOLS_RTF_FFTYPE "0");
    if (!sHelp.isEmpty())
        m_pAttrOutput->RunText().append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
    if (!sStatus.isEmpty())
        m_pAttrOutput->RunText().append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
    m_pAttrOutput->RunText().append(OOO_STRING_SVTOOLS_RTF_FFTYPETXT  "0");

    if (!sName.isEmpty())
        m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFNAME " ").append(msfilter::rtfutil::OutString(sName, eDefaultEncoding)).append("}");
    if (!sHelp.isEmpty())
        m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ").append(msfilter::rtfutil::OutString(sHelp, eDefaultEncoding)).append("}");
    m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFDEFTEXT " ").append(msfilter::rtfutil::OutString(sResult, eDefaultEncoding)).append("}");
    if (!sStatus.isEmpty())
        m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ").append(msfilter::rtfutil::OutString(sStatus, eDefaultEncoding)).append("}");
    m_pAttrOutput->RunText().append("}}}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
    m_pAttrOutput->RunText().append(msfilter::rtfutil::OutString(sResult, eDefaultEncoding)).append("}}");
}

sal_uLong RtfExport::ReplaceCr(sal_uInt8)
{
    // Completely unused for Rtf export... only here for code sharing
    // purpose with binary export

    return 0;
}

void RtfExport::WriteFonts()
{
    Strm().WriteCharPtr(SAL_NEWLINE_STRING).WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FONTTBL);
    maFontHelper.WriteFontTable(*m_pAttrOutput);
    Strm().WriteChar('}');
}

void RtfExport::WriteStyles()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");
    pStyles->OutputStylesTable();
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteFootnoteSettings()
{
    const SwPageFtnInfo& rFtnInfo = pDoc->GetPageDesc(0).GetFtnInfo();
    // Request a separator only in case the width is larger than zero.
    bool bSeparator = double(rFtnInfo.GetWidth()) > 0;

    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FTNSEP);
    if (bSeparator)
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_CHFTNSEP);
    Strm().WriteChar('}');
}

void RtfExport::WriteMainText()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    SwTableNode* pTableNode = pCurPam->GetNode().FindTableNode();
    if (m_pWriter && m_pWriter->bWriteOnlyFirstTable
            && pTableNode != 0)
    {
        pCurPam->GetPoint()->nNode = *pTableNode;
        pCurPam->GetMark()->nNode = *(pTableNode->EndOfSectionNode());
    }
    else
    {
        pCurPam->GetPoint()->nNode = pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();
    }

    WriteText();

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteInfo()
{
    OString aGenerator = OUStringToOString(utl::DocInfoHelper::GetGeneratorString(), RTL_TEXTENCODING_UTF8);
    Strm().WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_IGNORE LO_STRING_SVTOOLS_RTF_GENERATOR " ").WriteCharPtr(aGenerator.getStr()).WriteChar('}');
    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_INFO);

    SwDocShell* pDocShell(pDoc->GetDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;
    if (pDocShell)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(), uno::UNO_QUERY);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    if (xDocProps.is())
    {
        OutUnicode(OOO_STRING_SVTOOLS_RTF_TITLE, xDocProps->getTitle(), true);
        OutUnicode(OOO_STRING_SVTOOLS_RTF_SUBJECT, xDocProps->getSubject());

        OutUnicode(OOO_STRING_SVTOOLS_RTF_KEYWORDS,
                   ::comphelper::string::convertCommaSeparated(xDocProps->getKeywords()));
        OutUnicode(OOO_STRING_SVTOOLS_RTF_DOCCOMM, xDocProps->getDescription());

        OutUnicode(OOO_STRING_SVTOOLS_RTF_AUTHOR, xDocProps->getAuthor());
        OutDateTime(OOO_STRING_SVTOOLS_RTF_CREATIM, xDocProps->getCreationDate());

        OutUnicode(OOO_STRING_SVTOOLS_RTF_AUTHOR,xDocProps->getModifiedBy());
        OutDateTime(OOO_STRING_SVTOOLS_RTF_REVTIM, xDocProps->getModificationDate());

        OutDateTime(OOO_STRING_SVTOOLS_RTF_PRINTIM, xDocProps->getPrintDate());
    }

    Strm().WriteChar('}');
}

void RtfExport::WritePageDescTable()
{
    // Write page descriptions (page styles)
    sal_uInt16 nSize = pDoc->GetPageDescCnt();
    if (!nSize)
        return;

    Strm().WriteCharPtr(SAL_NEWLINE_STRING);
    bOutPageDescs = true;
    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PGDSCTBL);
    for (sal_uInt16 n = 0; n < nSize; ++n)
    {
        const SwPageDesc& rPageDesc = pDoc->GetPageDesc(n);

        Strm().WriteCharPtr(SAL_NEWLINE_STRING).WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PGDSC);
        OutULong(n).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PGDSCUSE);
        OutULong(rPageDesc.ReadUseOn());

        OutPageDescription(rPageDesc, false, false);

        // search for the next page description
        sal_uInt16 i = nSize;
        while (i)
            if (rPageDesc.GetFollow() == &pDoc->GetPageDesc(--i))
                break;
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PGDSCNXT);
        OutULong(i).WriteChar(' ');
        Strm().WriteCharPtr(msfilter::rtfutil::OutString(rPageDesc.GetName(), eDefaultEncoding).getStr()).WriteCharPtr(";}");
    }
    Strm().WriteChar('}').WriteCharPtr(SAL_NEWLINE_STRING);
    bOutPageDescs = false;

    // reset table infos, otherwise the depth of the cells will be incorrect,
    // in case the page style (header or footer) had tables
    mpTableInfo = ww8::WW8TableInfo::Pointer_t(new ww8::WW8TableInfo());
}

void RtfExport::ExportDocument_Impl()
{
    // Make the header
    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_RTF).WriteChar('1')
    .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_ANSI);
    Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_DEFF);
    OutULong(maFontHelper.GetId((SvxFontItem&)pDoc->GetAttrPool().GetDefaultItem(RES_CHRATR_FONT)));
    // If this not exist, MS don't understand our ansi characters (0x80-0xff).
    Strm().WriteCharPtr("\\adeflang1025");

    // Font table
    WriteFonts();

    pStyles = new MSWordStyles(*this);
    // Color and stylesheet table
    WriteStyles();

    // List table
    BuildNumbering();
    WriteNumbering();

    WriteRevTab();

    WriteInfo();
    // Default TabSize
    Strm().WriteCharPtr(m_pAttrOutput->m_aTabStop.makeStringAndClear().getStr()).WriteCharPtr(SAL_NEWLINE_STRING);
    // Zoom
    SwViewShell* pViewShell(pDoc->getIDocumentLayoutAccess().GetCurrentViewShell());
    if (pViewShell && pViewShell->GetViewOptions()->GetZoomType() == SVX_ZOOM_PERCENT)
    {
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_VIEWSCALE);
        OutULong(pViewShell->GetViewOptions()->GetZoom());
    }
    // Record changes?
    if (nsRedlineMode_t::REDLINE_ON & mnRedlineMode)
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_REVISIONS);
    // Init sections
    m_pSections = new MSWordSections(*this);

    // Page description
    WritePageDescTable();

    // Enable form protection by default if needed, as there is no switch to
    // enable it on a per-section basis. OTOH don't always enable it as it
    // breaks moving of drawings - so write it only in case there is really a
    // protected section in the document.
    {
        const SfxItemPool& rPool = pDoc->GetAttrPool();
        sal_uInt32 const nMaxItem = rPool.GetItemCount2(RES_PROTECT);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            const SvxProtectItem* pProtect = (const SvxProtectItem*)rPool.GetItem2(RES_PROTECT, n);
            if (pProtect && pProtect->IsCntntProtected())
            {
                Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FORMPROT);
                break;
            }
        }
    }

    // enable form field shading
    Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FORMSHADE);

    // size and empty margins of the page
    if (pDoc->GetPageDescCnt())
    {
        // Seeking the first SwFmtPageDesc. If no set, the default is valid
        const SwFmtPageDesc* pSttPgDsc = 0;
        {
            const SwNode& rSttNd = *pDoc->GetNodes()[
                                       pDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 ];
            const SfxItemSet* pSet = 0;

            if (rSttNd.IsCntntNode())
                pSet = &rSttNd.GetCntntNode()->GetSwAttrSet();
            else if (rSttNd.IsTableNode())
                pSet = &rSttNd.GetTableNode()->GetTable().
                       GetFrmFmt()->GetAttrSet();
            else if (rSttNd.IsSectionNode())
                pSet = &rSttNd.GetSectionNode()->GetSection().
                       GetFmt()->GetAttrSet();

            if (pSet)
            {
                sal_uInt16 nPosInDoc;
                pSttPgDsc = (SwFmtPageDesc*)&pSet->Get(RES_PAGEDESC);
                if (!pSttPgDsc->GetPageDesc())
                    pSttPgDsc = 0;
                else if (pDoc->FindPageDesc(pSttPgDsc->GetPageDesc()->GetName(), &nPosInDoc))
                {
                    Strm().WriteChar('{').WriteCharPtr(OOO_STRING_SVTOOLS_RTF_IGNORE).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PGDSCNO);
                    OutULong(nPosInDoc).WriteChar('}');
                }
            }
        }
        const SwPageDesc& rPageDesc = pSttPgDsc ? *pSttPgDsc->GetPageDesc()
                                      : pDoc->GetPageDesc(0);
        const SwFrmFmt& rFmtPage = rPageDesc.GetMaster();

        {
            if (rPageDesc.GetLandscape())
                Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LANDSCAPE);

            const SwFmtFrmSize& rSz = rFmtPage.GetFrmSize();
            // Clipboard document is always created without a printer, then
            // the size will be always LONG_MAX! Solution then is to use A4
            if (LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth())
            {
                Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PAPERH);
                Size a4 = SvxPaperInfo::GetPaperSize(PAPER_A4);
                OutULong(a4.Height()).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PAPERW);
                OutULong(a4.Width());
            }
            else
            {
                Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PAPERH);
                OutULong(rSz.GetHeight()).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PAPERW);
                OutULong(rSz.GetWidth());
            }
        }

        {
            const SvxLRSpaceItem& rLR = rFmtPage.GetLRSpace();
            Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_MARGL);
            OutLong(rLR.GetLeft()).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_MARGR);
            OutLong(rLR.GetRight());
        }

        {
            const SvxULSpaceItem& rUL = rFmtPage.GetULSpace();
            Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_MARGT);
            OutLong(rUL.GetUpper()).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_MARGB);
            OutLong(rUL.GetLower());
        }

        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SECTD).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SBKNONE);
        // All sections are unlocked by default
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SECTUNLOCKED);
        OutLong(1);
        OutPageDescription(rPageDesc, false, true);     // Changed bCheckForFirstPage to true so headers
        // following title page are correctly added - i13107
        if (pSttPgDsc)
        {
            pAktPageDesc = &rPageDesc;
        }
    }

    // line numbering
    const SwLineNumberInfo& rLnNumInfo = pDoc->GetLineNumberInfo();
    if (rLnNumInfo.IsPaintLineNumbers())
        AttrOutput().SectionLineNumbering(0, rLnNumInfo);

    {
        // write the footnotes and endnotes-out Info
        const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();

        const char* pOut = FTNPOS_CHAPTER == rFtnInfo.ePos
                           ? OOO_STRING_SVTOOLS_RTF_ENDDOC
                           : OOO_STRING_SVTOOLS_RTF_FTNBJ;
        Strm().WriteCharPtr(pOut).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_FTNSTART);
        OutLong(rFtnInfo.nFtnOffset + 1);

        switch (rFtnInfo.eNum)
        {
        case FTNNUM_PAGE:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNRSTPG;
            break;
        case FTNNUM_DOC:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNRSTCONT;
            break;
        default:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNRESTART;
            break;
        }
        Strm().WriteCharPtr(pOut);

        switch (rFtnInfo.aFmt.GetNumberingType())
        {
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNNALC;
            break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNNAUC;
            break;
        case SVX_NUM_ROMAN_LOWER:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNNRLC;
            break;
        case SVX_NUM_ROMAN_UPPER:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNNRUC;
            break;
        case SVX_NUM_CHAR_SPECIAL:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNNCHI;
            break;
        default:
            pOut = OOO_STRING_SVTOOLS_RTF_FTNNAR;
            break;
        }
        Strm().WriteCharPtr(pOut);

        const SwEndNoteInfo& rEndNoteInfo = pDoc->GetEndNoteInfo();

        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_AENDDOC).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_AFTNRSTCONT)
        .WriteCharPtr(OOO_STRING_SVTOOLS_RTF_AFTNSTART);
        OutLong(rEndNoteInfo.nFtnOffset + 1);

        switch (rEndNoteInfo.aFmt.GetNumberingType())
        {
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            pOut = OOO_STRING_SVTOOLS_RTF_AFTNNALC;
            break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            pOut = OOO_STRING_SVTOOLS_RTF_AFTNNAUC;
            break;
        case SVX_NUM_ROMAN_LOWER:
            pOut = OOO_STRING_SVTOOLS_RTF_AFTNNRLC;
            break;
        case SVX_NUM_ROMAN_UPPER:
            pOut = OOO_STRING_SVTOOLS_RTF_AFTNNRUC;
            break;
        case SVX_NUM_CHAR_SPECIAL:
            pOut = OOO_STRING_SVTOOLS_RTF_AFTNNCHI;
            break;
        default:
            pOut = OOO_STRING_SVTOOLS_RTF_AFTNNAR;
            break;
        }
        Strm().WriteCharPtr(pOut);
    }

    Strm().WriteCharPtr(SAL_NEWLINE_STRING);

    WriteFootnoteSettings();

    WriteMainText();

    Strm().WriteChar('}');
}

void RtfExport::PrepareNewPageDesc(const SfxItemSet* pSet,
                                   const SwNode& rNd, const SwFmtPageDesc* pNewPgDescFmt,
                                   const SwPageDesc* pNewPgDesc)
{
    const SwSectionFmt* pFmt = GetSectionFormat(rNd);
    const sal_uLong nLnNm = GetSectionLineNo(pSet, rNd);

    OSL_ENSURE(pNewPgDescFmt || pNewPgDesc, "Neither page desc format nor page desc provided.");

    if (pNewPgDescFmt)
        m_pSections->AppendSection(*pNewPgDescFmt, rNd, pFmt, nLnNm);
    else if (pNewPgDesc)
        m_pSections->AppendSection(pNewPgDesc, rNd, pFmt, nLnNm);

    // Don't insert a page break, when we're changing page style just because the next page has to be a different one.
    if (!m_pAttrOutput->m_pPrevPageDesc || m_pAttrOutput->m_pPrevPageDesc->GetFollow() != pNewPgDesc)
        AttrOutput().SectionBreak(msword::PageBreak, m_pSections->CurrentSectionInfo());
}

bool RtfExport::DisallowInheritingOutlineNumbering(const SwFmt& rFmt)
{
    bool bRet(false);

    if (SfxItemState::SET != rFmt.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFmt* pParent = rFmt.DerivedFrom())
        {
            if (((const SwTxtFmtColl*)pParent)->IsAssignedToListLevelOfOutlineStyle())
            {
                // Level 9 disables the outline
                Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LEVEL).WriteInt32(9);

                bRet = true;
            }
        }
    }

    return bRet;
}

void RtfExport::OutputGrfNode(const SwGrfNode&)
{
    /* noop, see RtfAttributeOutput::FlyFrameGraphic */
}

void RtfExport::OutputOLENode(const SwOLENode&)
{
    /* noop, see RtfAttributeOutput::FlyFrameOLE */
}

void RtfExport::OutputLinkedOLE(const OUString&)
{
}

void RtfExport::OutputTextNode(const SwTxtNode& rNode)
{
    m_nCurrentNodeIndex = rNode.GetIndex();
    if (!m_bOutOutlineOnly || rNode.IsOutline())
        MSWordExportBase::OutputTextNode(rNode);
    m_nCurrentNodeIndex = 0;
}

void RtfExport::AppendSection(const SwPageDesc* pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum)
{
    m_pSections->AppendSection(pPageDesc, pFmt, nLnNum);
    AttrOutput().SectionBreak(msword::PageBreak, m_pSections->CurrentSectionInfo());
}

RtfExport::RtfExport(RtfExportFilter* pFilter, SwDoc* pDocument, SwPaM* pCurrentPam, SwPaM* pOriginalPam, Writer* pWriter, bool bOutOutlineOnly)
    : MSWordExportBase(pDocument, pCurrentPam, pOriginalPam),
      m_pFilter(pFilter),
      m_pWriter(pWriter),
      m_pAttrOutput(),
      m_pSections(NULL),
      m_pSdrExport(),
      m_bOutOutlineOnly(bOutOutlineOnly),
      eDefaultEncoding(rtl_getTextEncodingFromWindowsCharset(sw::ms::rtl_TextEncodingToWinCharset(DEF_ENCODING))),
      eCurrentEncoding(eDefaultEncoding),
      bRTFFlySyntax(false),
      m_nCurrentNodeIndex(0)
{
    mbExportModeRTF = true;
    // the attribute output for the document
    m_pAttrOutput.reset(new RtfAttributeOutput(*this));
    // that just causes problems for RTF
    bSubstituteBullets = false;
    // needed to have a complete font table
    maFontHelper.bLoadAllFonts = true;
    // the related SdrExport
    m_pSdrExport.reset(new RtfSdrExport(*this));

    if (!m_pWriter)
        m_pWriter = &m_pFilter->m_aWriter;
}

RtfExport::~RtfExport()
{
}

SvStream& RtfExport::Strm()
{
    if (m_pStream)
        return *m_pStream;
    else
        return m_pWriter->Strm();
}

void RtfExport::setStream()
{
    m_pStream.reset(new SvMemoryStream());
}

OString RtfExport::getStream()
{
    OString aRet;

    if (m_pStream)
        aRet = OString(static_cast<const sal_Char*>(m_pStream->GetData()), m_pStream->Tell());

    return aRet;
}

void RtfExport::resetStream()
{
    m_pStream.reset();
}

SvStream& RtfExport::OutULong(sal_uLong nVal)
{
    return m_pWriter->OutULong(Strm(), nVal);
}

SvStream& RtfExport::OutLong(long nVal)
{
    return m_pWriter->OutLong(Strm(), nVal);
}

void RtfExport::OutUnicode(const sal_Char* pToken, const OUString& rContent, bool bUpr)
{
    if (!rContent.isEmpty())
    {
        if (!bUpr)
        {
            Strm().WriteChar('{').WriteCharPtr(pToken).WriteChar(' ');
            Strm().WriteCharPtr(msfilter::rtfutil::OutString(rContent, eCurrentEncoding).getStr());
            Strm().WriteChar('}');
        }
        else
            Strm().WriteCharPtr(msfilter::rtfutil::OutStringUpr(pToken, rContent, eCurrentEncoding).getStr());
    }
}

void RtfExport::OutDateTime(const sal_Char* pStr, const util::DateTime& rDT)
{
    Strm().WriteChar('{').WriteCharPtr(pStr).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_YR);
    OutULong(rDT.Year).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_MO);
    OutULong(rDT.Month).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_DY);
    OutULong(rDT.Day).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_HR);
    OutULong(rDT.Hours).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_MIN);
    OutULong(rDT.Minutes).WriteChar('}');
}

sal_uInt16 RtfExport::GetColor(const Color& rColor) const
{
    for (RtfColorTbl::const_iterator it=m_aColTbl.begin() ; it != m_aColTbl.end(); ++it)
        if ((*it).second == rColor)
        {
            SAL_INFO("sw.rtf", OSL_THIS_FUNC << " returning " << (*it).first << " (" << rColor.GetRed() << "," << rColor.GetGreen() << "," << rColor.GetBlue() << ")");
            return (*it).first;
        }
    OSL_FAIL("No such Color in m_aColTbl!");
    return 0;
}

void RtfExport::InsColor(const Color& rCol)
{
    sal_uInt16 n;
    bool bAutoColorInTable = false;
    for (RtfColorTbl::iterator it=m_aColTbl.begin() ; it != m_aColTbl.end(); ++it)
        if ((*it).second == rCol)
            return; // Already in the table
        else if ((*it).second == COL_AUTO)
            bAutoColorInTable = true;
    if (rCol.GetColor() == COL_AUTO)
        // COL_AUTO gets value 0
        n = 0;
    else
    {
        // other colors get values >0
        n = m_aColTbl.size();
        if (!bAutoColorInTable)
            // reserve value "0" for COL_AUTO (if COL_AUTO wasn't inserted until now)
            n++;
    }
    m_aColTbl.insert(std::pair<sal_uInt16,Color>(n, rCol));
}

void RtfExport::InsColorLine(const SvxBoxItem& rBox)
{
    const SvxBorderLine* pLine = 0;

    if (rBox.GetTop())
        InsColor((pLine = rBox.GetTop())->GetColor());
    if (rBox.GetBottom() && pLine != rBox.GetBottom())
        InsColor((pLine = rBox.GetBottom())->GetColor());
    if (rBox.GetLeft() && pLine != rBox.GetLeft())
        InsColor((pLine = rBox.GetLeft())->GetColor());
    if (rBox.GetRight() && pLine != rBox.GetRight())
        InsColor(rBox.GetRight()->GetColor());
}
void RtfExport::OutColorTable()
{
    // Build the table from rPool since the colors provided to
    // RtfAttributeOutput callbacks are too late.
    sal_uInt32 nMaxItem;
    const SfxItemPool& rPool = pDoc->GetAttrPool();

    // char color
    {
        const SvxColorItem* pCol = (const SvxColorItem*)GetDfltAttr(RES_CHRATR_COLOR);
        InsColor(pCol->GetValue());
        if (0 != (pCol = (const SvxColorItem*)rPool.GetPoolDefaultItem(RES_CHRATR_COLOR)))
            InsColor(pCol->GetValue());
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_COLOR);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pCol = (const SvxColorItem*)rPool.GetItem2(RES_CHRATR_COLOR, n)))
                InsColor(pCol->GetValue());
        }

        const SvxUnderlineItem* pUnder = (const SvxUnderlineItem*)GetDfltAttr(RES_CHRATR_UNDERLINE);
        InsColor(pUnder->GetColor());
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_UNDERLINE);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pUnder = (const SvxUnderlineItem*)rPool.GetItem2(RES_CHRATR_UNDERLINE, n)))
                InsColor(pUnder->GetColor());

        }

        const SvxOverlineItem* pOver = (const SvxOverlineItem*)GetDfltAttr(RES_CHRATR_OVERLINE);
        InsColor(pOver->GetColor());
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_OVERLINE);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pOver = (const SvxOverlineItem*)rPool.GetItem2(RES_CHRATR_OVERLINE, n)))
                InsColor(pOver->GetColor());

        }

    }

    // background color
    static const sal_uInt16 aBrushIds[] =
    {
        RES_BACKGROUND, RES_CHRATR_BACKGROUND, 0
    };

    for (const sal_uInt16* pIds = aBrushIds; *pIds; ++pIds)
    {
        const SvxBrushItem* pBkgrd = (const SvxBrushItem*)GetDfltAttr(*pIds);
        InsColor(pBkgrd->GetColor());
        if (0 != (pBkgrd = (const SvxBrushItem*)rPool.GetPoolDefaultItem(*pIds)))
        {
            InsColor(pBkgrd->GetColor());
        }
        nMaxItem = rPool.GetItemCount2(*pIds);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pBkgrd = (const SvxBrushItem*)rPool.GetItem2(*pIds , n)))
            {
                InsColor(pBkgrd->GetColor());
            }
        }
    }

    // shadow color
    {
        const SvxShadowItem* pShadow = (const SvxShadowItem*)GetDfltAttr(RES_SHADOW);
        InsColor(pShadow->GetColor());
        if (0 != (pShadow = (const SvxShadowItem*)rPool.GetPoolDefaultItem(RES_SHADOW)))
        {
            InsColor(pShadow->GetColor());
        }
        nMaxItem = rPool.GetItemCount2(RES_SHADOW);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pShadow = (const SvxShadowItem*)rPool.GetItem2(RES_SHADOW, n)))
            {
                InsColor(pShadow->GetColor());
            }
        }
    }

    // frame border color
    {
        const SvxBoxItem* pBox;
        if (0 != (pBox = (const SvxBoxItem*)rPool.GetPoolDefaultItem(RES_BOX)))
            InsColorLine(*pBox);
        nMaxItem = rPool.GetItemCount2(RES_BOX);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pBox = (const SvxBoxItem*)rPool.GetItem2(RES_BOX, n)))
                InsColorLine(*pBox);
        }
    }

    {
        const SvxBoxItem* pCharBox;
        if (0 != (pCharBox = (const SvxBoxItem*)rPool.GetPoolDefaultItem(RES_CHRATR_BOX)))
            InsColorLine(*pCharBox);
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_BOX);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if (0 != (pCharBox = (const SvxBoxItem*)rPool.GetItem2(RES_CHRATR_BOX, n)))
                InsColorLine(*pCharBox);
        }
    }

    // TextFrame or paragraph background solid fill.
    nMaxItem = rPool.GetItemCount2(XATTR_FILLCOLOR);
    for (sal_uInt32 i = 0; i < nMaxItem; ++i)
    {
        if (const XFillColorItem* pItem = static_cast<const XFillColorItem*>(rPool.GetItem2(XATTR_FILLCOLOR, i)))
            InsColor(pItem->GetColorValue());
    }

    for (size_t n = 0; n < m_aColTbl.size(); ++n)
    {
        const Color& rCol = m_aColTbl[ n ];
        if (n || COL_AUTO != rCol.GetColor())
        {
            Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_RED);
            OutULong(rCol.GetRed()).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_GREEN);
            OutULong(rCol.GetGreen()).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_BLUE);
            OutULong(rCol.GetBlue());
        }
        Strm().WriteChar(';');
    }
}

void RtfExport::InsStyle(sal_uInt16 nId, const OString& rStyle)
{
    m_aStyTbl.insert(std::pair<sal_uInt16,OString>(nId, rStyle));
}

OString* RtfExport::GetStyle(sal_uInt16 nId)
{
    std::map<sal_uInt16,OString>::iterator i = m_aStyTbl.find(nId);
    if (i != m_aStyTbl.end())
        return &i->second;
    return NULL;
}

sal_uInt16 RtfExport::GetRedline(const OUString& rAuthor)
{
    std::map<OUString,sal_uInt16>::iterator i = m_aRedlineTbl.find(rAuthor);
    if (i != m_aRedlineTbl.end())
        return i->second;
    else
    {
        int nId = m_aRedlineTbl.size();
        m_aRedlineTbl.insert(std::pair<OUString,sal_uInt16>(rAuthor,nId));
        return nId;
    }
}

const OUString* RtfExport::GetRedline(sal_uInt16 nId)
{
    for (std::map<OUString,sal_uInt16>::iterator aIter = m_aRedlineTbl.begin(); aIter != m_aRedlineTbl.end(); ++aIter)
        if ((*aIter).second == nId)
            return &(*aIter).first;
    return NULL;
}

void RtfExport::OutPageDescription(const SwPageDesc& rPgDsc, bool bWriteReset, bool bCheckForFirstPage)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");
    const SwPageDesc* pSave = pAktPageDesc;

    pAktPageDesc = &rPgDsc;
    if (bCheckForFirstPage && pAktPageDesc->GetFollow() &&
            pAktPageDesc->GetFollow() != pAktPageDesc)
        pAktPageDesc = pAktPageDesc->GetFollow();

    if (bWriteReset)
    {
        if (pCurPam->GetPoint()->nNode == pOrigPam->Start()->nNode)
            Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SECTD).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SBKNONE);
        else
            Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SECT).WriteCharPtr(OOO_STRING_SVTOOLS_RTF_SECTD);
    }

    if (pAktPageDesc->GetLandscape())
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_LNDSCPSXN);

    const SwFmt* pFmt = &pAktPageDesc->GetMaster(); //GetLeft();
    bOutPageDescs = true;
    OutputFormat(*pFmt, true, false);
    bOutPageDescs = false;

    // normal header / footer (without a style)
    const SfxPoolItem* pItem;
    if (pAktPageDesc->GetLeft().GetAttrSet().GetItemState(RES_HEADER, false,
            &pItem) == SfxItemState::SET)
        WriteHeaderFooter(*pItem, true);
    if (pAktPageDesc->GetLeft().GetAttrSet().GetItemState(RES_FOOTER, false,
            &pItem) == SfxItemState::SET)
        WriteHeaderFooter(*pItem, false);

    // title page
    if (pAktPageDesc != &rPgDsc)
    {
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_TITLEPG);
        pAktPageDesc = &rPgDsc;
        if (pAktPageDesc->GetMaster().GetAttrSet().GetItemState(RES_HEADER,
                false, &pItem) == SfxItemState::SET)
            WriteHeaderFooter(*pItem, true);
        if (pAktPageDesc->GetMaster().GetAttrSet().GetItemState(RES_FOOTER,
                false, &pItem) == SfxItemState::SET)
            WriteHeaderFooter(*pItem, false);
    }

    // numbering type
    AttrOutput().SectionPageNumbering(pAktPageDesc->GetNumType().GetNumberingType(), boost::none);

    pAktPageDesc = pSave;
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteHeaderFooter(const SfxPoolItem& rItem, bool bHeader)
{
    if (bHeader)
    {
        const SwFmtHeader& rHeader = (const SwFmtHeader&)rItem;
        if (!rHeader.IsActive())
            return;
    }
    else
    {
        const SwFmtFooter& rFooter = (const SwFmtFooter&)rItem;
        if (!rFooter.IsActive())
            return;
    }

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    const sal_Char* pStr = (bHeader ? OOO_STRING_SVTOOLS_RTF_HEADER : OOO_STRING_SVTOOLS_RTF_FOOTER);
    /* is this a title page? */
    if (pAktPageDesc->GetFollow() && pAktPageDesc->GetFollow() != pAktPageDesc)
    {
        Strm().WriteCharPtr(OOO_STRING_SVTOOLS_RTF_TITLEPG);
        pStr = (bHeader ? OOO_STRING_SVTOOLS_RTF_HEADERF : OOO_STRING_SVTOOLS_RTF_FOOTERF);
    }
    Strm().WriteChar('{').WriteCharPtr(pStr);
    WriteHeaderFooterText(pAktPageDesc->GetMaster(), bHeader);
    Strm().WriteChar('}');

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteHeaderFooter(const SwFrmFmt& rFmt, bool bHeader, const sal_Char* pStr, bool bTitlepg)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    m_pAttrOutput->WriteHeaderFooter_Impl(rFmt, bHeader, pStr, bTitlepg);

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

/// Glue class to call RtfExport as an internal filter, needed by copy&paste support.
class SwRTFWriter : public Writer
{
private:
    bool m_bOutOutlineOnly;

public:
    SwRTFWriter(const OUString& rFilterName, const OUString& rBaseURL);
    virtual ~SwRTFWriter();
    virtual sal_uLong WriteStream() SAL_OVERRIDE;
};

SwRTFWriter::SwRTFWriter(const OUString& rFltName, const OUString& rBaseURL)
{
    SetBaseURL(rBaseURL);
    // export outline nodes, only (send outline to clipboard/presentation)
    m_bOutOutlineOnly = rFltName.startsWith("O");
}

SwRTFWriter::~SwRTFWriter()
{}

sal_uLong SwRTFWriter::WriteStream()
{
    SwPaM aPam(*pCurPam->End(), *pCurPam->Start());
    RtfExport aExport(NULL, pDoc, &aPam, pCurPam, this, m_bOutOutlineOnly);
    aExport.ExportDocument(true);
    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL ExportRTF(const OUString& rFltName, const OUString& rBaseURL, WriterRef& xRet)
{
    xRet = new SwRTFWriter(rFltName, rBaseURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
