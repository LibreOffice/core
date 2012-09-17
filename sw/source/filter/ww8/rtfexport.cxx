/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Miklos Vajna.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <swtable.hxx>
#include <frmatr.hxx>
#include <ftninfo.hxx>
#include <fmthdft.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/protitem.hxx>

#include <docary.hxx>
#include <numrule.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>

#include "ww8par.hxx"

#include <comphelper/string.hxx>
#include <rtl/ustring.hxx>
#include <svtools/rtfkeywd.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <unotools/configmgr.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#endif

using ::editeng::SvxBorderLine;
using namespace ::comphelper;
using namespace ::com::sun::star;

using sw::mark::IMark;

#if defined(UNX)
const sal_Char RtfExport::sNewLine = '\012';
#else
const sal_Char* const RtfExport::sNewLine = "\015\012";
#endif

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

bool RtfExport::CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich )
{
    // FIXME is this actually true for rtf? - this is copied from DOCX
    if ( nScript == i18n::ScriptType::ASIAN )
    {
        // for asian in ww8, there is only one fontsize
        // and one fontstyle (posture/weight)
        switch ( nWhich )
        {
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
                return false;
            default:
                break;
        }
    }
    else if ( nScript != i18n::ScriptType::COMPLEX )
    {
        // for western in ww8, there is only one fontsize
        // and one fontstyle (posture/weight)
        switch ( nWhich )
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

void RtfExport::AppendBookmarks( const SwTxtNode& rNode, xub_StrLen nAktPos, xub_StrLen nLen )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    std::vector< OUString > aStarts;
    std::vector< OUString > aEnds;

    IMarkVector aMarks;
    if ( GetBookmarks( rNode, nAktPos, nAktPos + nLen, aMarks ) )
    {
        for ( IMarkVector::const_iterator it = aMarks.begin(), end = aMarks.end();
                it != end; ++it )
        {
            IMark* pMark = (*it);
            xub_StrLen nStart = pMark->GetMarkStart().nContent.GetIndex();
            xub_StrLen nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if ( nStart == nAktPos )
                aStarts.push_back( pMark->GetName() );

            if ( nEnd == nAktPos )
                aEnds.push_back( pMark->GetName() );
        }
    }

    m_pAttrOutput->WriteBookmarks_Impl( aStarts, aEnds );
}

void RtfExport::AppendBookmark( const OUString& rName, bool /*bSkip*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    std::vector<OUString> aStarts;
    std::vector<OUString> aEnds;

    aStarts.push_back(rName);
    aEnds.push_back(rName);

    m_pAttrOutput->WriteBookmarks_Impl(aStarts, aEnds);
}

void RtfExport::WriteChar( sal_Unicode )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* WriteChar() has nothing to do for rtf. */
}

static bool IsExportNumRule( const SwNumRule& rRule, sal_uInt8* pEnd = 0 )
{
    sal_uInt8 nEnd = MAXLEVEL;
    while( nEnd-- && !rRule.GetNumFmt( nEnd ))
        ;
    ++nEnd;

    const SwNumFmt* pNFmt;
    sal_uInt8 nLvl;

    for( nLvl = 0; nLvl < nEnd; ++nLvl )
        if( SVX_NUM_NUMBER_NONE != ( pNFmt = &rRule.Get( nLvl ))
                ->GetNumberingType() || !pNFmt->GetPrefix().isEmpty() ||
                (!pNFmt->GetSuffix().isEmpty() && !pNFmt->GetSuffix().equals(aDotStr)) )
            break;

    if( pEnd )
        *pEnd = nEnd;
    return nLvl != nEnd;
}

void RtfExport::BuildNumbering()
{
    const SwNumRuleTbl& rListTbl = pDoc->GetNumRuleTbl();

    for( sal_uInt16 n = rListTbl.size()+1; n; )
    {
        SwNumRule* pRule;
        --n;
        if( n == rListTbl.size() )
            pRule = (SwNumRule*)pDoc->GetOutlineNumRule();
        else
        {
            pRule = rListTbl[ n ];
            if( !pDoc->IsUsed( *pRule ))
                continue;
        }

        if( IsExportNumRule( *pRule ))
            GetId( *pRule );
    }
}

void RtfExport::WriteNumbering()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    if ( !pUsedNumTbl )
        return; // no numbering is used

    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_LISTTABLE;
    AbstractNumberingDefinitions();
    Strm() << '}';

    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_LISTOVERRIDETABLE;
    NumberingDefinitions();
    Strm() << '}';

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteRevTab()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    int nRevAuthors = pDoc->GetRedlineTbl().size();

    if (nRevAuthors < 1)
        return;

    // RTF always seems to use Unknown as the default first entry
    String sUnknown(RTL_CONSTASCII_USTRINGPARAM("Unknown"));
    GetRedline(sUnknown);

    for( sal_uInt16 i = 0; i < pDoc->GetRedlineTbl().size(); ++i )
    {
        const SwRedline* pRedl = pDoc->GetRedlineTbl()[ i ];

        GetRedline(SW_MOD()->GetRedlineAuthor(pRedl->GetAuthor()));
    }

    // Now write the table
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_REVTBL << ' ';
    for(sal_uInt16 i = 0; i < m_aRedlineTbl.size(); ++i)
    {
        const String* pAuthor = GetRedline(i);
        Strm() << '{';
        if (pAuthor)
            Strm() << msfilter::rtfutil::OutString(*pAuthor, eDefaultEncoding).getStr();
        Strm() << ";}";
    }
    Strm() << '}' << sNewLine;
}

void RtfExport::WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
        const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, sal_uInt8 /*nBreakCode*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // headers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_EVEN )
        WriteHeaderFooter( rLeftFmt, true, OOO_STRING_SVTOOLS_RTF_HEADERL );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_ODD )
        WriteHeaderFooter( rFmt, true, OOO_STRING_SVTOOLS_RTF_HEADER );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_HEADER_FIRST )
        WriteHeaderFooter( rFirstPageFmt, true, OOO_STRING_SVTOOLS_RTF_HEADERF, true );

    // footers
    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_EVEN )
        WriteHeaderFooter( rLeftFmt, false, OOO_STRING_SVTOOLS_RTF_FOOTERL );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_ODD )
        WriteHeaderFooter( rFmt, false, OOO_STRING_SVTOOLS_RTF_FOOTER );

    if ( nHeadFootFlags & nsHdFtFlags::WW8_FOOTER_FIRST )
        WriteHeaderFooter( rFirstPageFmt, false, OOO_STRING_SVTOOLS_RTF_FOOTERF, true );
}

void RtfExport::OutputField( const SwField* pFld, ww::eField eFldType, const String& rFldCmd, sal_uInt8 nMode )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_pAttrOutput->WriteField_Impl( pFld, eFldType, rFldCmd, nMode );
}

void RtfExport::WriteFormData( const ::sw::mark::IFieldmark& /*rFieldmark*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfExport::WriteHyperlinkData( const ::sw::mark::IFieldmark& /*rFieldmark*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfExport::DoComboBox(const rtl::OUString& /*rName*/,
                             const rtl::OUString& /*rHelp*/,
                             const rtl::OUString& /*rToolTip*/,
                             const rtl::OUString& /*rSelected*/,
                             uno::Sequence<rtl::OUString>& /*rListItems*/)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // this is handled in RtfAttributeOutput::OutputFlyFrame_Impl
}

void RtfExport::DoFormText(const SwInputField* pFld )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    ::rtl::OUString sResult = pFld->ExpandField(pDoc->IsClipBoard());
    ::rtl::OUString sHelp( pFld->GetHelp() );
    ::rtl::OUString sName = pFld->GetPar2();
    ::rtl::OUString sStatus = pFld->GetToolTip();
    m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_FIELD "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST "{ FORMTEXT }");
    m_pAttrOutput->RunText().append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD " {" OOO_STRING_SVTOOLS_RTF_FFTYPE "0" );
    if( !sHelp.isEmpty() )
        m_pAttrOutput->RunText().append( OOO_STRING_SVTOOLS_RTF_FFOWNHELP );
    if( !sStatus.isEmpty() )
        m_pAttrOutput->RunText().append( OOO_STRING_SVTOOLS_RTF_FFOWNSTAT );
    m_pAttrOutput->RunText().append( OOO_STRING_SVTOOLS_RTF_FFTYPETXT  "0" );

    if( !sName.isEmpty() )
        m_pAttrOutput->RunText().append( "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFNAME " ").append( msfilter::rtfutil::OutString( sName, eDefaultEncoding )).append( "}" );
    if( !sHelp.isEmpty() )
        m_pAttrOutput->RunText().append( "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ").append( msfilter::rtfutil::OutString( sHelp, eDefaultEncoding )).append( "}" );
    m_pAttrOutput->RunText().append( "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFDEFTEXT " ").append( msfilter::rtfutil::OutString( sResult, eDefaultEncoding )).append( "}" );
    if( !sStatus.isEmpty() )
        m_pAttrOutput->RunText().append( "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ").append( msfilter::rtfutil::OutString( sStatus, eDefaultEncoding )).append( "}");
    m_pAttrOutput->RunText().append( "}}}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " " );
    m_pAttrOutput->RunText().append( msfilter::rtfutil::OutString( sResult, eDefaultEncoding )).append( "}}" );
}

sal_uLong RtfExport::ReplaceCr( sal_uInt8 )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // Completely unused for Rtf export... only here for code sharing
    // purpose with binary export

    return 0;
}

void RtfExport::WriteFonts()
{
    Strm() << sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_FONTTBL;
    maFontHelper.WriteFontTable( *m_pAttrOutput );
    Strm() << '}';
}

void RtfExport::WriteStyles()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");
    pStyles->OutputStylesTable();
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteMainText()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");
    pCurPam->GetPoint()->nNode = pDoc->GetNodes().GetEndOfContent().StartOfSectionNode()->GetIndex();
    WriteText();
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteInfo()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_INFO;

    SwDocShell *pDocShell(pDoc->GetDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pDocShell->GetModel(), uno::UNO_QUERY);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    if (xDocProps.is()) {
        OutUnicode(OOO_STRING_SVTOOLS_RTF_TITLE, xDocProps->getTitle());
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

    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_COMMENT << " ";
    Strm() << OUStringToOString( utl::ConfigManager::getProductName(), eCurrentEncoding).getStr() << "}{" << OOO_STRING_SVTOOLS_RTF_VERN;
    OutULong( SUPD*10 ) << '}';
    Strm() << '}';
}

void RtfExport::WritePageDescTable()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // Write page descriptions (page styles)
    sal_uInt16 nSize = pDoc->GetPageDescCnt();
    if( !nSize )
        return;

    Strm() << sNewLine;        // a separator
    bOutPageDescs = sal_True;
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_PGDSCTBL;
    for( sal_uInt16 n = 0; n < nSize; ++n )
    {
        const SwPageDesc& rPageDesc = pDoc->GetPageDesc( n );

        Strm() << sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_PGDSC;
        OutULong( n ) << OOO_STRING_SVTOOLS_RTF_PGDSCUSE;
        OutULong( rPageDesc.ReadUseOn() );

        OutPageDescription( rPageDesc, sal_False, sal_False );

        // search for the next page description
        sal_uInt16 i = nSize;
        while( i  )
            if( rPageDesc.GetFollow() == &pDoc->GetPageDesc( --i ) )
                break;
        Strm() << OOO_STRING_SVTOOLS_RTF_PGDSCNXT;
        OutULong( i ) << ' ';
        Strm() << msfilter::rtfutil::OutString( rPageDesc.GetName(), eDefaultEncoding).getStr() << ";}";
    }
    Strm() << '}' << sNewLine;
    bOutPageDescs = sal_False;

    // reset table infos, otherwise the depth of the cells will be incorrect,
    // in case the page style (header or footer) had tables
    mpTableInfo = ww8::WW8TableInfo::Pointer_t(new ww8::WW8TableInfo());
}

void RtfExport::ExportDocument_Impl()
{
#if OSL_DEBUG_LEVEL > 1
    // MSWordExportBase::WriteText and others write debug messages to std::clog
    // which is not interesting while debugging RtfExport
    std::ostringstream aOss;
    std::streambuf *pOldBuf = std::clog.rdbuf(aOss.rdbuf());
#endif

    // Make the header
    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_RTF << '1'
        << OOO_STRING_SVTOOLS_RTF_ANSI;
    Strm() << OOO_STRING_SVTOOLS_RTF_DEFF;
    OutULong( maFontHelper.GetId( (SvxFontItem&)pDoc->GetAttrPool().GetDefaultItem(
                    RES_CHRATR_FONT ) ));
    // If this not exist, MS don't understand our ansi characters (0x80-0xff).
    Strm() << "\\adeflang1025";

    // Font table
    WriteFonts();

    pStyles = new MSWordStyles( *this );
    // Color and stylesheet table
    WriteStyles();

    // List table
    BuildNumbering();
    WriteNumbering();

    WriteRevTab();

    WriteInfo();
    // Default TabSize
    Strm() << m_pAttrOutput->m_aTabStop.makeStringAndClear().getStr() << sNewLine;
    // Zoom
    ViewShell *pViewShell(pDoc->GetCurrentViewShell());
    if (pViewShell && pViewShell->GetViewOptions()->GetZoomType() == SVX_ZOOM_PERCENT)
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_VIEWSCALE;
        OutULong(pViewShell->GetViewOptions()->GetZoom());
    }
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
                Strm() << OOO_STRING_SVTOOLS_RTF_FORMPROT;
                break;
            }
        }
    }

    // enable form field shading
    Strm() << OOO_STRING_SVTOOLS_RTF_FORMSHADE;

    // size and empty margins of the page
    if( pDoc->GetPageDescCnt() )
    {
        // Seeking the first SwFmtPageDesc. If no set, the default is valid
        const SwFmtPageDesc* pSttPgDsc = 0;
        {
            const SwNode& rSttNd = *pDoc->GetNodes()[
                        pDoc->GetNodes().GetEndOfExtras().GetIndex() + 2 ];
            const SfxItemSet* pSet = 0;

            if( rSttNd.IsCntntNode() )
                pSet = &rSttNd.GetCntntNode()->GetSwAttrSet();
            else if( rSttNd.IsTableNode() )
                pSet = &rSttNd.GetTableNode()->GetTable().
                            GetFrmFmt()->GetAttrSet();
            else if( rSttNd.IsSectionNode() )
                pSet = &rSttNd.GetSectionNode()->GetSection().
                            GetFmt()->GetAttrSet();

            if( pSet )
            {
                sal_uInt16 nPosInDoc;
                pSttPgDsc = (SwFmtPageDesc*)&pSet->Get( RES_PAGEDESC );
                if( !pSttPgDsc->GetPageDesc() )
                    pSttPgDsc = 0;
                else if( pDoc->FindPageDescByName( pSttPgDsc->
                                    GetPageDesc()->GetName(), &nPosInDoc ))
                {
                    Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_PGDSCNO;
                    OutULong( nPosInDoc ) << '}';
                }
            }
        }
        const SwPageDesc& rPageDesc = pSttPgDsc ? *pSttPgDsc->GetPageDesc()
            : pDoc->GetPageDesc( 0 );
        const SwFrmFmt &rFmtPage = rPageDesc.GetMaster();

        {
            if( rPageDesc.GetLandscape() )
                Strm() << OOO_STRING_SVTOOLS_RTF_LANDSCAPE;

            const SwFmtFrmSize& rSz = rFmtPage.GetFrmSize();
            // Clipboard document is always created without a printer, then
            // the size will be always LONG_MAX! Solution then is to use A4
            if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
            {
                Strm() << OOO_STRING_SVTOOLS_RTF_PAPERH;
                Size a4 = SvxPaperInfo::GetPaperSize(PAPER_A4);
                OutULong( a4.Height() ) << OOO_STRING_SVTOOLS_RTF_PAPERW;
                OutULong( a4.Width() );
            }
            else
            {
                Strm() << OOO_STRING_SVTOOLS_RTF_PAPERH;
                OutULong( rSz.GetHeight() ) << OOO_STRING_SVTOOLS_RTF_PAPERW;
                OutULong( rSz.GetWidth() );
            }
        }

        {
            const SvxLRSpaceItem& rLR = rFmtPage.GetLRSpace();
            Strm() << OOO_STRING_SVTOOLS_RTF_MARGL;
            OutLong( rLR.GetLeft() ) << OOO_STRING_SVTOOLS_RTF_MARGR;
            OutLong( rLR.GetRight() );
        }

        {
            const SvxULSpaceItem& rUL = rFmtPage.GetULSpace();
            Strm() << OOO_STRING_SVTOOLS_RTF_MARGT;
            OutLong( rUL.GetUpper() ) << OOO_STRING_SVTOOLS_RTF_MARGB;
            OutLong( rUL.GetLower() );
        }

        Strm() << OOO_STRING_SVTOOLS_RTF_SECTD << OOO_STRING_SVTOOLS_RTF_SBKNONE;
        // All sections are unlocked by default
        Strm() << OOO_STRING_SVTOOLS_RTF_SECTUNLOCKED;
        OutLong(1);
        OutPageDescription( rPageDesc, sal_False, sal_True );   // Changed bCheckForFirstPage to sal_True so headers
                                                            // following title page are correctly added - i13107
        if( pSttPgDsc )
        {
            pAktPageDesc = &rPageDesc;
        }
    }

    // line numbering
    const SwLineNumberInfo& rLnNumInfo = pDoc->GetLineNumberInfo();
    if ( rLnNumInfo.IsPaintLineNumbers() )
        AttrOutput().SectionLineNumbering( 0, rLnNumInfo );

    {
        // write the footnotes and endnotes-out Info
        const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();

        const char* pOut = FTNPOS_CHAPTER == rFtnInfo.ePos
                            ? OOO_STRING_SVTOOLS_RTF_ENDDOC
                            : OOO_STRING_SVTOOLS_RTF_FTNBJ;
        Strm() << pOut << OOO_STRING_SVTOOLS_RTF_FTNSTART;
        OutLong( rFtnInfo.nFtnOffset + 1 );

        switch( rFtnInfo.eNum )
        {
            case FTNNUM_PAGE:       pOut = OOO_STRING_SVTOOLS_RTF_FTNRSTPG; break;
            case FTNNUM_DOC:        pOut = OOO_STRING_SVTOOLS_RTF_FTNRSTCONT;   break;
            default:                pOut = OOO_STRING_SVTOOLS_RTF_FTNRESTART;   break;
        }
        Strm() << pOut;

        switch( rFtnInfo.aFmt.GetNumberingType() )
        {
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_FTNNALC;  break;
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_UPPER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_FTNNAUC;  break;
            case SVX_NUM_ROMAN_LOWER:           pOut = OOO_STRING_SVTOOLS_RTF_FTNNRLC;  break;
            case SVX_NUM_ROMAN_UPPER:           pOut = OOO_STRING_SVTOOLS_RTF_FTNNRUC;  break;
            case SVX_NUM_CHAR_SPECIAL:          pOut = OOO_STRING_SVTOOLS_RTF_FTNNCHI;  break;
            default:                    pOut = OOO_STRING_SVTOOLS_RTF_FTNNAR;       break;
        }
        Strm() << pOut;


        const SwEndNoteInfo& rEndNoteInfo = pDoc->GetEndNoteInfo();

        Strm() << OOO_STRING_SVTOOLS_RTF_AENDDOC << OOO_STRING_SVTOOLS_RTF_AFTNRSTCONT
               << OOO_STRING_SVTOOLS_RTF_AFTNSTART;
        OutLong( rEndNoteInfo.nFtnOffset + 1 );

        switch( rEndNoteInfo.aFmt.GetNumberingType() )
        {
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_AFTNNALC; break;
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_UPPER_LETTER_N:  pOut = OOO_STRING_SVTOOLS_RTF_AFTNNAUC; break;
            case SVX_NUM_ROMAN_LOWER:           pOut = OOO_STRING_SVTOOLS_RTF_AFTNNRLC; break;
            case SVX_NUM_ROMAN_UPPER:           pOut = OOO_STRING_SVTOOLS_RTF_AFTNNRUC; break;
            case SVX_NUM_CHAR_SPECIAL:          pOut = OOO_STRING_SVTOOLS_RTF_AFTNNCHI; break;
            default:                    pOut = OOO_STRING_SVTOOLS_RTF_AFTNNAR;  break;
        }
        Strm() << pOut;
    }

    Strm() << sNewLine;

    // Init sections
    m_pSections = new MSWordSections( *this );

    WriteMainText();

    Strm() << '}';

#if OSL_DEBUG_LEVEL > 1
    std::clog.rdbuf(pOldBuf);
#endif
}

void RtfExport::PrepareNewPageDesc( const SfxItemSet* pSet,
        const SwNode& rNd, const SwFmtPageDesc* pNewPgDescFmt,
        const SwPageDesc* pNewPgDesc )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    const SwSectionFmt* pFmt = GetSectionFormat( rNd );
    const sal_uLong nLnNm = GetSectionLineNo( pSet, rNd );

    OSL_ENSURE( pNewPgDescFmt || pNewPgDesc, "Neither page desc format nor page desc provided." );

    if ( pNewPgDescFmt )
        m_pSections->AppendSection( *pNewPgDescFmt, rNd, pFmt, nLnNm );
    else if ( pNewPgDesc )
        m_pSections->AppendSection( pNewPgDesc, rNd, pFmt, nLnNm );

    // Don't insert a page break, when we're changing page style just because the next page has to be a different one.
    if (!m_pAttrOutput->m_pPrevPageDesc || m_pAttrOutput->m_pPrevPageDesc->GetFollow() != pNewPgDesc)
        AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo() );
}

bool RtfExport::DisallowInheritingOutlineNumbering( const SwFmt& rFmt )
{
    bool bRet( false );

    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (SFX_ITEM_SET != rFmt.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFmt *pParent = rFmt.DerivedFrom())
        {
            if (((const SwTxtFmtColl*)pParent)->IsAssignedToListLevelOfOutlineStyle())
            {
                // Level 9 disables the outline
                Strm() << OOO_STRING_SVTOOLS_RTF_LEVEL << sal_Int32(9);

                bRet = true;
            }
        }
    }

    return bRet;
}

void RtfExport::OutputGrfNode( const SwGrfNode& )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* noop, see RtfAttributeOutput::FlyFrameGraphic */
}

void RtfExport::OutputOLENode( const SwOLENode& )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* noop, see RtfAttributeOutput::FlyFrameOLE */
}

void RtfExport::OutputLinkedOLE( const rtl::OUString& )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
}

void RtfExport::OutputTextNode( const SwTxtNode& rNode )
{
    if ( !m_bOutOutlineOnly || rNode.IsOutline( ) )
        MSWordExportBase::OutputTextNode( rNode );
}

void RtfExport::AppendSection( const SwPageDesc* pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_pSections->AppendSection( pPageDesc, pFmt, nLnNum );
    AttrOutput().SectionBreak( msword::PageBreak, m_pSections->CurrentSectionInfo() );
}

RtfExport::RtfExport( RtfExportFilter *pFilter, SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam, Writer* pWriter, bool bOutOutlineOnly )
    : MSWordExportBase( pDocument, pCurrentPam, pOriginalPam ),
      m_pFilter( pFilter ),
      m_pWriter( pWriter ),
      m_pAttrOutput( NULL ),
      m_pSections( NULL ),
      m_pSdrExport( NULL ),
      m_bOutOutlineOnly( bOutOutlineOnly ),
      eDefaultEncoding(
              rtl_getTextEncodingFromWindowsCharset(
                  sw::ms::rtl_TextEncodingToWinCharset(DEF_ENCODING))),
      eCurrentEncoding(eDefaultEncoding),
      bRTFFlySyntax(false)
{
    mbExportModeRTF = true;
    // the attribute output for the document
    m_pAttrOutput = new RtfAttributeOutput( *this );
    // that just causes problems for RTF
    bSubstituteBullets = false;
    // needed to have a complete font table
    maFontHelper.bLoadAllFonts = true;
    // the related SdrExport
    m_pSdrExport = new RtfSdrExport( *this );

    if (!m_pWriter)
        m_pWriter = &m_pFilter->m_aWriter;
}

RtfExport::~RtfExport()
{
    delete m_pAttrOutput, m_pAttrOutput = NULL;
    delete m_pSdrExport, m_pSdrExport = NULL;
}

SvStream& RtfExport::Strm()
{
    return m_pWriter->Strm();
}

SvStream& RtfExport::OutULong( sal_uLong nVal )
{
    return m_pWriter->OutULong( Strm(), nVal );
}

SvStream& RtfExport::OutLong( long nVal )
{
    return m_pWriter->OutLong( Strm(), nVal );
}

void RtfExport::OutUnicode(const sal_Char *pToken, const String &rContent)
{
    if (rContent.Len())
    {
        Strm() << '{' << pToken << ' ';
        Strm() << msfilter::rtfutil::OutString( rContent, eCurrentEncoding ).getStr();
        Strm() << '}';
    }
}

void RtfExport::OutDateTime(const sal_Char* pStr, const util::DateTime& rDT )
{
    Strm() << '{' << pStr << OOO_STRING_SVTOOLS_RTF_YR;
    OutULong( rDT.Year ) << OOO_STRING_SVTOOLS_RTF_MO;
    OutULong( rDT.Month ) << OOO_STRING_SVTOOLS_RTF_DY;
    OutULong( rDT.Day ) << OOO_STRING_SVTOOLS_RTF_HR;
    OutULong( rDT.Hours ) << OOO_STRING_SVTOOLS_RTF_MIN;
    OutULong( rDT.Minutes ) << '}';
}

sal_uInt16 RtfExport::GetColor( const Color& rColor ) const
{
    for (RtfColorTbl::const_iterator it=m_aColTbl.begin() ; it != m_aColTbl.end(); ++it )
        if ((*it).second == rColor) {
            SAL_INFO("sw.rtf", OSL_THIS_FUNC << " returning " << (*it).first << " (" << rColor.GetRed() << "," << rColor.GetGreen() << "," << rColor.GetBlue() << ")");
            return (*it).first;
        }
    OSL_FAIL( "No such Color in m_aColTbl!" );
    return 0;
}

void RtfExport::InsColor( const Color& rCol )
{
    sal_uInt16 n;
    bool bContainsAuto = false;
    for (RtfColorTbl::iterator it=m_aColTbl.begin() ; it != m_aColTbl.end(); ++it )
        if ((*it).second == rCol)
            return; // Already in the table
        else if ((*it).second == COL_AUTO)
            bContainsAuto = true;
    if (rCol.GetColor() == COL_AUTO)
        n = 0;
    else
    {
        n = m_aColTbl.size();
        // Fix for the case when first a !COL_AUTO gets inserted as #0, then
        // gets overwritten by COL_AUTO
        if (!n)
            n++;

        // Fix the case where the table doesn't contain the COL_AUTO at #0
        else if (!bContainsAuto)
            n++;
    }
    m_aColTbl.insert(std::pair<sal_uInt16,Color>( n, rCol ));
}

void RtfExport::InsColorLine( const SvxBoxItem& rBox )
{
    const SvxBorderLine* pLine = 0;

    if( rBox.GetTop() )
        InsColor( (pLine = rBox.GetTop())->GetColor() );
    if( rBox.GetBottom() && pLine != rBox.GetBottom() )
        InsColor( (pLine = rBox.GetBottom())->GetColor() );
    if( rBox.GetLeft() && pLine != rBox.GetLeft()  )
        InsColor( (pLine = rBox.GetLeft())->GetColor() );
    if( rBox.GetRight() && pLine != rBox.GetRight()  )
        InsColor( rBox.GetRight()->GetColor() );
}
void RtfExport::OutColorTable()
{
    // Build the table from rPool since the colors provided to
    // RtfAttributeOutput callbacks are too late.
    sal_uInt32 nMaxItem;
    const SfxItemPool& rPool = pDoc->GetAttrPool();

    // char color
    {
        const SvxColorItem* pCol = (const SvxColorItem*)GetDfltAttr(
                                                RES_CHRATR_COLOR );
        InsColor( pCol->GetValue() );
        if( 0 != ( pCol = (const SvxColorItem*)rPool.GetPoolDefaultItem(
                RES_CHRATR_COLOR ) ))
            InsColor( pCol->GetValue() );
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_COLOR);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if( 0 != (pCol = (const SvxColorItem*)rPool.GetItem2(
                RES_CHRATR_COLOR, n ) ) )
                InsColor( pCol->GetValue() );
        }

        const SvxUnderlineItem* pUnder = (const SvxUnderlineItem*)GetDfltAttr( RES_CHRATR_UNDERLINE );
        InsColor( pUnder->GetColor() );
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_UNDERLINE);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if( 0 != (pUnder = (const SvxUnderlineItem*)rPool.GetItem2( RES_CHRATR_UNDERLINE, n ) ) )
                InsColor( pUnder->GetColor() );

        }

        const SvxOverlineItem* pOver = (const SvxOverlineItem*)GetDfltAttr( RES_CHRATR_OVERLINE );
        InsColor( pOver->GetColor() );
        nMaxItem = rPool.GetItemCount2(RES_CHRATR_OVERLINE);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if( 0 != (pOver = (const SvxOverlineItem*)rPool.GetItem2( RES_CHRATR_OVERLINE, n ) ) )
                InsColor( pOver->GetColor() );

        }

    }

    // background color
    static const sal_uInt16 aBrushIds[] = {
                                RES_BACKGROUND, RES_CHRATR_BACKGROUND, 0 };

    for( const sal_uInt16* pIds = aBrushIds; *pIds; ++pIds )
    {
        const SvxBrushItem* pBkgrd = (const SvxBrushItem*)GetDfltAttr( *pIds );
        InsColor( pBkgrd->GetColor() );
        if( 0 != ( pBkgrd = (const SvxBrushItem*)rPool.GetPoolDefaultItem(
                        *pIds ) ))
        {
            InsColor( pBkgrd->GetColor() );
        }
        nMaxItem = rPool.GetItemCount2( *pIds );
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if( 0 != (pBkgrd = (const SvxBrushItem*)rPool.GetItem2(
                    *pIds , n ) ))
            {
                InsColor( pBkgrd->GetColor() );
            }
    }
    }

    // shadow color
    {
        const SvxShadowItem* pShadow = (const SvxShadowItem*)GetDfltAttr(
                                                            RES_SHADOW );
        InsColor( pShadow->GetColor() );
        if( 0 != ( pShadow = (const SvxShadowItem*)rPool.GetPoolDefaultItem(
                        RES_SHADOW ) ))
        {
            InsColor( pShadow->GetColor() );
        }
        nMaxItem = rPool.GetItemCount2(RES_SHADOW);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if( 0 != (pShadow = (const SvxShadowItem*)rPool.GetItem2(
                RES_SHADOW, n ) ) )
            {
                InsColor( pShadow->GetColor() );
            }
    }
    }

    // frame border color
    {
        const SvxBoxItem* pBox;
        if( 0 != ( pBox = (const SvxBoxItem*)rPool.GetPoolDefaultItem(
                        RES_BOX ) ))
            InsColorLine( *pBox );
        nMaxItem = rPool.GetItemCount2(RES_BOX);
        for (sal_uInt32 n = 0; n < nMaxItem; ++n)
        {
            if( 0 != (pBox = (const SvxBoxItem*)rPool.GetItem2( RES_BOX, n ) ))
                InsColorLine( *pBox );
    }
    }

    for (size_t n = 0; n < m_aColTbl.size(); ++n)
    {
        const Color& rCol = m_aColTbl[ n ];
        if( n || COL_AUTO != rCol.GetColor() )
        {
            Strm() << OOO_STRING_SVTOOLS_RTF_RED;
            OutULong( rCol.GetRed() ) << OOO_STRING_SVTOOLS_RTF_GREEN;
            OutULong( rCol.GetGreen() ) << OOO_STRING_SVTOOLS_RTF_BLUE;
            OutULong( rCol.GetBlue() );
        }
        Strm() << ';';
    }
}

void RtfExport::InsStyle( sal_uInt16 nId, const OString& rStyle )
{
    m_aStyTbl.insert(std::pair<sal_uInt16,OString>(nId, rStyle) );
}

OString* RtfExport::GetStyle( sal_uInt16 nId )
{
    std::map<sal_uInt16,OString>::iterator i = m_aStyTbl.find(nId);
    if (i != m_aStyTbl.end())
        return &i->second;
    return NULL;
}

sal_uInt16 RtfExport::GetRedline( const String& rAuthor )
{
    std::map<String,sal_uInt16>::iterator i = m_aRedlineTbl.find(rAuthor);
    if (i != m_aRedlineTbl.end())
        return i->second;
    else
    {
        int nId = m_aRedlineTbl.size();
        m_aRedlineTbl.insert(std::pair<String,sal_uInt16>(rAuthor,nId));
        return nId;
    }
}

const String* RtfExport::GetRedline( sal_uInt16 nId )
{
    for(std::map<String,sal_uInt16>::iterator aIter = m_aRedlineTbl.begin(); aIter != m_aRedlineTbl.end(); ++aIter)
        if ((*aIter).second == nId)
            return &(*aIter).first;
    return NULL;
}

void RtfExport::OutPageDescription( const SwPageDesc& rPgDsc, sal_Bool bWriteReset, sal_Bool bCheckForFirstPage )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");
    const SwPageDesc *pSave = pAktPageDesc;

    pAktPageDesc = &rPgDsc;
    if( bCheckForFirstPage && pAktPageDesc->GetFollow() &&
            pAktPageDesc->GetFollow() != pAktPageDesc )
        pAktPageDesc = pAktPageDesc->GetFollow();

    if( bWriteReset )
    {
        if( pCurPam->GetPoint()->nNode == pOrigPam->Start()->nNode )
            Strm() << OOO_STRING_SVTOOLS_RTF_SECTD << OOO_STRING_SVTOOLS_RTF_SBKNONE;
        else
            Strm() << OOO_STRING_SVTOOLS_RTF_SECT << OOO_STRING_SVTOOLS_RTF_SECTD;
    }

    if( pAktPageDesc->GetLandscape() )
        Strm() << OOO_STRING_SVTOOLS_RTF_LNDSCPSXN;

    const SwFmt *pFmt = &pAktPageDesc->GetMaster(); //GetLeft();
    bOutPageDescs = true;
    OutputFormat(*pFmt, true, false);
    bOutPageDescs = false;

    // normal header / footer (without a style)
    const SfxPoolItem* pItem;
    if( pAktPageDesc->GetLeft().GetAttrSet().GetItemState( RES_HEADER, sal_False,
                &pItem ) == SFX_ITEM_SET)
        WriteHeaderFooter(*pItem, true);
    if( pAktPageDesc->GetLeft().GetAttrSet().GetItemState( RES_FOOTER, sal_False,
                &pItem ) == SFX_ITEM_SET)
        WriteHeaderFooter(*pItem, false);

    // title page
    if( pAktPageDesc != &rPgDsc )
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_TITLEPG;
        pAktPageDesc = &rPgDsc;
        if( pAktPageDesc->GetMaster().GetAttrSet().GetItemState( RES_HEADER,
                    sal_False, &pItem ) == SFX_ITEM_SET )
            WriteHeaderFooter(*pItem, true);
        if( pAktPageDesc->GetMaster().GetAttrSet().GetItemState( RES_FOOTER,
                    sal_False, &pItem ) == SFX_ITEM_SET )
            WriteHeaderFooter(*pItem, false);
    }

    // numbering type
    AttrOutput().SectionPageNumbering(pAktPageDesc->GetNumType().GetNumberingType(), 0);

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
    if( pAktPageDesc->GetFollow() && pAktPageDesc->GetFollow() != pAktPageDesc )
    {
        Strm() << OOO_STRING_SVTOOLS_RTF_TITLEPG;
        pStr = (bHeader ? OOO_STRING_SVTOOLS_RTF_HEADERF : OOO_STRING_SVTOOLS_RTF_FOOTERF);
    }
    Strm() << '{' << pStr;
    WriteHeaderFooterText(pAktPageDesc->GetMaster(), bHeader);
    Strm() << '}';

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfExport::WriteHeaderFooter(const SwFrmFmt& rFmt, bool bHeader, const sal_Char* pStr, bool bTitlepg)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    m_pAttrOutput->WriteHeaderFooter_Impl( rFmt, bHeader, pStr, bTitlepg );

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

/// Glue class to call RtfExport as an internal filter, needed by copy&paste support.
class SwRTFWriter : public Writer
{
    private:
        bool m_bOutOutlineOnly;

    public:
        SwRTFWriter( const String& rFilterName, const String& rBaseURL );
        virtual ~SwRTFWriter();
               virtual sal_uLong WriteStream();
};

SwRTFWriter::SwRTFWriter( const String& rFltName, const String & rBaseURL )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    SetBaseURL( rBaseURL );
    // export outline nodes, only (send outline to clipboard/presentation)
    m_bOutOutlineOnly = 'O' == rFltName.GetChar( 0 );
}

SwRTFWriter::~SwRTFWriter()
{}

sal_uLong SwRTFWriter::WriteStream()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    RtfExport aExport( NULL, pDoc, new SwPaM( *pCurPam->End(), *pCurPam->Start() ), pCurPam, this, m_bOutOutlineOnly );
    aExport.ExportDocument( true );
    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL ExportRTF( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    xRet = new SwRTFWriter( rFltName, rBaseURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
