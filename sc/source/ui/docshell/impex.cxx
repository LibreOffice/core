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

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/unicode.hxx>
#include <sot/formats.hxx>
#include <sfx2/mieclip.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>
#include <osl/module.hxx>

#include <global.hxx>
#include <docsh.hxx>
#include <undoblk.hxx>
#include <rangenam.hxx>
#include <tabvwsh.hxx>
#include <filter.hxx>
#include <asciiopt.hxx>
#include <formulacell.hxx>
#include <cellform.hxx>
#include <progress.hxx>
#include <scitems.hxx>
#include <editable.hxx>
#include <compiler.hxx>
#include <warnbox.hxx>
#include <clipparam.hxx>
#include <impex.hxx>
#include <editutil.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <stringutil.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <documentimport.hxx>
#include <refundo.hxx>
#include <mtvelements.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <o3tl/safeint.hxx>
#include <tools/svlibrary.h>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <editeng/editobj.hxx>
#include <svl/numformat.hxx>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <sax/tools/converter.hxx>

#include <memory>
#include <string_view>

#include <osl/endian.h>

// We don't want to end up with 2GB read in one line just because of malformed
// multiline fields, so chop it _somewhere_, which is twice supported columns
// times arbitrary maximum cell content length, 2*1024*64K=128M, and because
// it's sal_Unicode that's 256MB. If it's 2GB of data without LF we're out of
// luck anyway.
constexpr sal_Int32 nArbitraryCellLengthLimit = SAL_MAX_UINT16;
constexpr sal_Int32 nArbitraryLineLengthLimit = 2 * MAXCOLCOUNT * nArbitraryCellLengthLimit;

namespace
{
    const char SYLK_LF[]  = "\x1b :";

    bool lcl_IsEndianSwap( const SvStream& rStrm )
    {
    #ifdef OSL_BIGENDIAN
        return rStrm.GetEndian() != SvStreamEndian::BIG;
    #else
        return rStrm.GetEndian() != SvStreamEndian::LITTLE;
    #endif
    }
}

namespace {

enum class SylkVersion
{
    SCALC3,    // Wrote wrongly quoted strings and unescaped semicolons.
    OOO32,     // Correct strings, plus multiline content.
    OWN,       // Place our new versions, if any, before this value.
    OTHER      // Assume that aliens wrote correct strings.
};

}

// Whole document without Undo
ScImportExport::ScImportExport( ScDocument& r )
    : pDocSh( dynamic_cast< ScDocShell* >(r.GetDocumentShell()) ), rDoc( r ),
      nSizeLimit( 0 ), nMaxImportRow(!utl::ConfigManager::IsFuzzing() ? rDoc.MaxRow() : SCROWS32K),
      cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( true ), bSingle( true ), bUndo( false ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mbImportBroadcast(false), mbOverwriting( false )
{
    pUndoDoc = nullptr;
    pExtOptions = nullptr;
}

// Insert am current cell without range(es)
ScImportExport::ScImportExport( ScDocument& r, const ScAddress& rPt )
    : pDocSh( dynamic_cast< ScDocShell* >(r.GetDocumentShell()) ), rDoc( r ),
      aRange( rPt ),
      nSizeLimit( 0 ), nMaxImportRow(!utl::ConfigManager::IsFuzzing() ? rDoc.MaxRow() : SCROWS32K),
      cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( false ), bSingle( true ), bUndo( pDocSh != nullptr ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mbImportBroadcast(false), mbOverwriting( false )
{
    pUndoDoc = nullptr;
    pExtOptions = nullptr;
}

//  ctor with a range is only used for export
//! ctor with a string (and bSingle=true) is also used for DdeSetData
ScImportExport::ScImportExport( ScDocument& r, const ScRange& rRange )
    : pDocSh( dynamic_cast<ScDocShell* >(r.GetDocumentShell()) ), rDoc( r ),
      aRange( rRange ),
      nSizeLimit( 0 ), nMaxImportRow(!utl::ConfigManager::IsFuzzing() ? rDoc.MaxRow() : SCROWS32K),
      cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( false ), bSingle( false ), bUndo( pDocSh != nullptr ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mbImportBroadcast(false), mbOverwriting( false )
{
    pUndoDoc = nullptr;
    pExtOptions = nullptr;
    // Only one sheet (table) supported
    aRange.aEnd.SetTab( aRange.aStart.Tab() );
}

// Evaluate input string - either range, cell or the whole document (when error)
// If a View exists, the TabNo of the view will be used.
ScImportExport::ScImportExport( ScDocument& r, const OUString& rPos )
    : pDocSh( dynamic_cast< ScDocShell* >(r.GetDocumentShell()) ), rDoc( r ),
      nSizeLimit( 0 ), nMaxImportRow(!utl::ConfigManager::IsFuzzing() ? rDoc.MaxRow() : SCROWS32K),
      cSep( '\t' ), cStr( '"' ),
      bFormulas( false ), bIncludeFiltered( true ),
      bAll( false ), bSingle( true ), bUndo( pDocSh != nullptr ),
      bOverflowRow( false ), bOverflowCol( false ), bOverflowCell( false ),
      mbApi( true ), mbImportBroadcast(false), mbOverwriting( false )
{
    pUndoDoc = nullptr;
    pExtOptions = nullptr;

    SCTAB nTab = ScDocShell::GetCurTab();
    aRange.aStart.SetTab( nTab );
    OUString aPos( rPos );
    // Named range?
    ScRangeName* pRange = rDoc.GetRangeName();
    if (pRange)
    {
        const ScRangeData* pData = pRange->findByUpperName(ScGlobal::getCharClass().uppercase(aPos));
        if (pData)
        {
            if( pData->HasType( ScRangeData::Type::RefArea )
                || pData->HasType( ScRangeData::Type::AbsArea )
                || pData->HasType( ScRangeData::Type::AbsPos ) )
            {
                aPos = pData->GetSymbol();
            }
        }
    }
    formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
    // Range?
    if (aRange.Parse(aPos, rDoc, eConv) & ScRefFlags::VALID)
        bSingle = false;
    // Cell?
    else if (aRange.aStart.Parse(aPos, rDoc, eConv) & ScRefFlags::VALID)
        aRange.aEnd = aRange.aStart;
    else
        bAll = true;
}

ScImportExport::~ScImportExport() COVERITY_NOEXCEPT_FALSE
{
    pUndoDoc.reset();
    pExtOptions.reset();
}

void ScImportExport::SetExtOptions( const ScAsciiOptions& rOpt )
{
    if ( pExtOptions )
        *pExtOptions = rOpt;
    else
        pExtOptions.reset(new ScAsciiOptions( rOpt ));

    //  "normal" Options

    cSep = ScAsciiOptions::GetWeightedFieldSep( rOpt.GetFieldSeps(), false);
    cStr = rOpt.GetTextSep();
}

void ScImportExport::SetFilterOptions(const OUString& rFilterOptions)
{
    maFilterOptions = rFilterOptions;
}

bool ScImportExport::IsFormatSupported( SotClipboardFormatId nFormat )
{
    return nFormat == SotClipboardFormatId::STRING
              || nFormat == SotClipboardFormatId::STRING_TSVC
              || nFormat == SotClipboardFormatId::SYLK
              || nFormat == SotClipboardFormatId::LINK
              || nFormat == SotClipboardFormatId::HTML
              || nFormat == SotClipboardFormatId::HTML_SIMPLE
              || nFormat == SotClipboardFormatId::DIF;
}

// Prepare for Undo
bool ScImportExport::StartPaste()
{
    if ( !bAll )
    {
        ScEditableTester aTester( rDoc, aRange );
        if ( !aTester.IsEditable() )
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(ScDocShell::GetActiveDialogParent(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          ScResId(aTester.GetMessageId())));
            xInfoBox->run();
            return false;
        }
    }
    if( bUndo && pDocSh && rDoc.IsUndoEnabled())
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( rDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
        rDoc.CopyToDocument(aRange, InsertDeleteFlags::ALL | InsertDeleteFlags::NOCAPTIONS, false, *pUndoDoc);
    }
    return true;
}

// Create Undo/Redo actions, Invalidate/Repaint
void ScImportExport::EndPaste(bool bAutoRowHeight)
{
    bool bHeight = bAutoRowHeight && pDocSh && pDocSh->AdjustRowHeight(
                    aRange.aStart.Row(), aRange.aEnd.Row(), aRange.aStart.Tab() );

    if( pUndoDoc && rDoc.IsUndoEnabled() && pDocSh )
    {
        ScDocumentUniquePtr pRedoDoc(new ScDocument( SCDOCMODE_UNDO ));
        pRedoDoc->InitUndo( rDoc, aRange.aStart.Tab(), aRange.aEnd.Tab() );
        rDoc.CopyToDocument(aRange, InsertDeleteFlags::ALL | InsertDeleteFlags::NOCAPTIONS, false, *pRedoDoc);
        ScMarkData aDestMark(pRedoDoc->GetSheetLimits());
        aDestMark.SetMarkArea(aRange);
        pDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoPaste>(pDocSh, aRange, aDestMark, std::move(pUndoDoc), std::move(pRedoDoc), InsertDeleteFlags::ALL, nullptr));
    }
    pUndoDoc.reset();
    if( pDocSh )
    {
        if (!bHeight)
            pDocSh->PostPaint( aRange, PaintPartFlags::Grid );
        pDocSh->SetDocumentModified();
    }
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        pViewSh->UpdateInputHandler();

}

bool ScImportExport::ExportData( const OUString& rMimeType,
                                 css::uno::Any & rValue )
{
    SvMemoryStream aStrm;
    // mba: no BaseURL for data exchange
    if( ExportStream( aStrm, OUString(),
                SotExchange::GetFormatIdFromMimeType( rMimeType ) ))
    {
        aStrm.WriteUChar( 0 );
        rValue <<= css::uno::Sequence< sal_Int8 >(
                                        static_cast<sal_Int8 const *>(aStrm.GetData()),
                                        aStrm.TellEnd() );
        return true;
    }
    return false;
}

bool ScImportExport::ImportString( const OUString& rText, SotClipboardFormatId nFmt )
{
    switch ( nFmt )
    {
        // formats supporting unicode
        case SotClipboardFormatId::STRING :
        case SotClipboardFormatId::STRING_TSVC :
        {
            ScImportStringStream aStrm( rText);
            return ImportStream( aStrm, OUString(), nFmt );
            // ImportStream must handle RTL_TEXTENCODING_UNICODE
        }
        default:
        {
            rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
            OString aTmp( rText.getStr(), rText.getLength(), eEnc );
            SvMemoryStream aStrm( const_cast<char *>(aTmp.getStr()), aTmp.getLength() * sizeof(char), StreamMode::READ );
            aStrm.SetStreamCharSet( eEnc );
            SetNoEndianSwap( aStrm );       //! no swapping in memory
            return ImportStream( aStrm, OUString(), nFmt );
        }
    }
}

bool ScImportExport::ExportString( OUString& rText, SotClipboardFormatId nFmt )
{
    if ( nFmt != SotClipboardFormatId::STRING && nFmt != SotClipboardFormatId::STRING_TSVC )
    {
        SAL_WARN("sc.ui", "ScImportExport::ExportString: Unicode not supported for other formats than SotClipboardFormatId::STRING[_TSV]");
        rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
        OString aTmp;
        bool bOk = ExportByteString( aTmp, eEnc, nFmt );
        rText = OStringToOUString( aTmp, eEnc );
        return bOk;
    }
    //  nSizeLimit not needed for OUString

    SvMemoryStream aStrm;
    aStrm.SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
    SetNoEndianSwap( aStrm );       //! no swapping in memory
    // mba: no BaseURL for data exc
    if( ExportStream( aStrm, OUString(), nFmt ) )
    {
        aStrm.WriteUInt16( 0 );
        rText = OUString( static_cast<const sal_Unicode*>(aStrm.GetData()) );
        return true;
    }
    rText.clear();
    return false;

    // ExportStream must handle RTL_TEXTENCODING_UNICODE
}

bool ScImportExport::ExportByteString( OString& rText, rtl_TextEncoding eEnc, SotClipboardFormatId nFmt )
{
    OSL_ENSURE( eEnc != RTL_TEXTENCODING_UNICODE, "ScImportExport::ExportByteString: Unicode not supported" );
    if ( eEnc == RTL_TEXTENCODING_UNICODE )
        eEnc = osl_getThreadTextEncoding();

    if (!nSizeLimit)
        nSizeLimit = SAL_MAX_UINT16;

    SvMemoryStream aStrm;
    aStrm.SetStreamCharSet( eEnc );
    SetNoEndianSwap( aStrm );       //! no swapping in memory
    // mba: no BaseURL for data exchange
    if( ExportStream( aStrm, OUString(), nFmt ) )
    {
        aStrm.WriteChar( 0 );
        if( aStrm.TellEnd() <= nSizeLimit )
        {
            rText = static_cast<const char*>(aStrm.GetData());
            return true;
        }
    }
    rText.clear();
    return false;
}

bool ScImportExport::ImportStream( SvStream& rStrm, const OUString& rBaseURL, SotClipboardFormatId nFmt )
{
    if( nFmt == SotClipboardFormatId::STRING || nFmt == SotClipboardFormatId::STRING_TSVC )
    {
        if( ExtText2Doc( rStrm ) )      // evaluate pExtOptions
            return true;
    }
    if( nFmt == SotClipboardFormatId::SYLK )
    {
        if( Sylk2Doc( rStrm ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::DIF )
    {
        if( Dif2Doc( rStrm ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::RTF || nFmt == SotClipboardFormatId::RICHTEXT )
    {
        if( RTF2Doc( rStrm, rBaseURL ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::LINK )
        return true;            // Link-Import?
    if ( nFmt == SotClipboardFormatId::HTML )
    {
        if( HTML2Doc( rStrm, rBaseURL ) )
            return true;
    }
    if ( nFmt == SotClipboardFormatId::HTML_SIMPLE )
    {
        MSE40HTMLClipFormatObj aMSE40ClpObj;                // needed to skip the header data
        SvStream* pHTML = aMSE40ClpObj.IsValid( rStrm );
        if ( pHTML && HTML2Doc( *pHTML, rBaseURL ) )
            return true;
    }

    return false;
}

bool ScImportExport::ExportStream( SvStream& rStrm, const OUString& rBaseURL, SotClipboardFormatId nFmt )
{
    if( nFmt == SotClipboardFormatId::STRING || nFmt == SotClipboardFormatId::STRING_TSVC )
    {
        if( Doc2Text( rStrm ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::SYLK )
    {
        if( Doc2Sylk( rStrm ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::DIF )
    {
        if( Doc2Dif( rStrm ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::LINK && !bAll )
    {
        OUString aDocName;
        if ( rDoc.IsClipboard() )
            aDocName = ScGlobal::GetClipDocName();
        else
        {
            SfxObjectShell* pShell = rDoc.GetDocumentShell();
            if (pShell)
                aDocName = pShell->GetTitle( SFX_TITLE_FULLNAME );
        }

        OSL_ENSURE( !aDocName.isEmpty(), "ClipBoard document has no name! :-/" );
        if( !aDocName.isEmpty() )
        {
            // Always use Calc A1 syntax for paste link.
            OUString aRefName;
            ScRefFlags nFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D;
            if( bSingle )
                aRefName = aRange.aStart.Format(nFlags, &rDoc, formula::FormulaGrammar::CONV_OOO);
            else
            {
                if( aRange.aStart.Tab() != aRange.aEnd.Tab() )
                    nFlags |= ScRefFlags::TAB2_3D;
                aRefName = aRange.Format(rDoc, nFlags, formula::FormulaGrammar::CONV_OOO);
            }
            OUString aAppName = Application::GetAppName();

            // extra bits are used to tell the client to prefer external
            // reference link.

            WriteUnicodeOrByteString( rStrm, aAppName, true );
            WriteUnicodeOrByteString( rStrm, aDocName, true );
            WriteUnicodeOrByteString( rStrm, aRefName, true );
            WriteUnicodeOrByteString( rStrm, "calc:extref", true );
            if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
                rStrm.WriteUInt16( 0 );
            else
                rStrm.WriteChar( 0 );
            return rStrm.GetError() == ERRCODE_NONE;
        }
    }
    if( nFmt == SotClipboardFormatId::HTML )
    {
        if( Doc2HTML( rStrm, rBaseURL ) )
            return true;
    }
    if( nFmt == SotClipboardFormatId::RTF || nFmt == SotClipboardFormatId::RICHTEXT )
    {
        if( Doc2RTF( rStrm ) )
            return true;
    }

    return false;
}

void ScImportExport::WriteUnicodeOrByteString( SvStream& rStrm, const OUString& rString, bool bZero )
{
    rtl_TextEncoding eEnc = rStrm.GetStreamCharSet();
    if ( eEnc == RTL_TEXTENCODING_UNICODE )
    {
        if ( !lcl_IsEndianSwap( rStrm ) )
            rStrm.WriteBytes(rString.getStr(), rString.getLength() * sizeof(sal_Unicode));
        else
        {
            const sal_Unicode* p = rString.getStr();
            const sal_Unicode* const pStop = p + rString.getLength();
            while ( p < pStop )
            {
                rStrm.WriteUInt16( *p );
            }
        }
        if ( bZero )
            rStrm.WriteUInt16( 0 );
    }
    else
    {
        OString aByteStr(OUStringToOString(rString, eEnc));
        rStrm.WriteOString( aByteStr );
        if ( bZero )
            rStrm.WriteChar( 0 );
    }
}

// This function could be replaced by endlub()
void ScImportExport::WriteUnicodeOrByteEndl( SvStream& rStrm )
{
    if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
    {   // same as endl() but unicode
        switch ( rStrm.GetLineDelimiter() )
        {
            case LINEEND_CR :
                rStrm.WriteUInt16( '\r' );
            break;
            case LINEEND_LF :
                rStrm.WriteUInt16( '\n' );
            break;
            default:
                rStrm.WriteUInt16( '\r' ).WriteUInt16( '\n' );
        }
    }
    else
        endl( rStrm );
}

sal_Int32 ScImportExport::CountVisualWidth(const OUString& rStr, sal_Int32& nIdx, sal_Int32 nMaxWidth)
{
    sal_Int32 nWidth = 0;
    while(nIdx < rStr.getLength() && nWidth < nMaxWidth)
    {
        sal_uInt32 nCode = rStr.iterateCodePoints(&nIdx);

        if (unicode::isCJKIVSCharacter(nCode) || (nCode >= 0x3000 && nCode <= 0x303F))
            nWidth += 2;
        else if (!unicode::isIVSSelector(nCode))
            nWidth += 1;
    }

    if (nIdx < rStr.getLength())
    {
        sal_Int32 nTmpIdx = nIdx;
        sal_uInt32 nCode = rStr.iterateCodePoints(&nTmpIdx);

        if (unicode::isIVSSelector(nCode))
            nIdx = nTmpIdx;
    }
    return nWidth;
}

sal_Int32 ScImportExport::CountVisualWidth(const OUString& rStr)
{
    sal_Int32 nIdx = 0;
    return CountVisualWidth(rStr, nIdx, SAL_MAX_INT32);
}

void ScImportExport::SetNoEndianSwap( SvStream& rStrm )
{
#ifdef OSL_BIGENDIAN
    rStrm.SetEndian( SvStreamEndian::BIG );
#else
    rStrm.SetEndian( SvStreamEndian::LITTLE );
#endif
}

namespace {

enum QuoteType
{
    FIELDSTART_QUOTE,
    FIRST_QUOTE,
    SECOND_QUOTE,
    FIELDEND_QUOTE,
    DONTKNOW_QUOTE
};

}

/** Determine if *p is a quote that ends a quoted field.

    Precondition: we are parsing a quoted field already and *p is a quote.

    @return
        FIELDEND_QUOTE if end of field quote
        DONTKNOW_QUOTE anything else
 */
static QuoteType lcl_isFieldEndQuote( const sal_Unicode* p, const sal_Unicode* pSeps, sal_Unicode& rcDetectSep )
{
    // Due to broken CSV generators that don't double embedded quotes check if
    // a field separator immediately or with trailing spaces follows the quote,
    // only then end the field, or at end of string.
    constexpr sal_Unicode cBlank = ' ';
    if (p[1] == cBlank && ScGlobal::UnicodeStrChr( pSeps, cBlank))
        return FIELDEND_QUOTE;
    // Detect a possible blank separator if it's not already in the list (which
    // was checked right above for p[1]==cBlank).
    const bool bBlankSep = (p[1] == cBlank && !rcDetectSep && p[2] && p[2] != cBlank);
    while (p[1] == cBlank)
        ++p;
    if (!p[1] || ScGlobal::UnicodeStrChr( pSeps, p[1]))
        return FIELDEND_QUOTE;
    // Extended separator detection after a closing quote (with or without
    // blanks). Note that nQuotes is incremented *after* the call so is not yet
    // even here, and that with separator detection we reach here only if
    // lcl_isEscapedOrFieldEndQuote() did not already detect FIRST_QUOTE or
    // SECOND_QUOTE for an escaped embedded quote, thus nQuotes does not have
    // to be checked.
    if (!rcDetectSep)
    {
        constexpr sal_Unicode vSep[] = { ',', '\t', ';' };
        for (const sal_Unicode c : vSep)
        {
            if (p[1] == c)
            {
                rcDetectSep = c;
                return FIELDEND_QUOTE;
            }
        }
    }
    // Blank separator is least significant, after others.
    if (bBlankSep)
    {
        rcDetectSep = cBlank;
        return FIELDEND_QUOTE;
    }
    return DONTKNOW_QUOTE;
}

/** Determine if *p is a quote that is escaped by being doubled or ends a
    quoted field.

    Precondition: *p is a quote.

    @param nQuotes
        Quote characters encountered so far.
        Odd (after opening quote) means either no embedded quotes or only quote
        pairs so far.
        Even means either not in a quoted field or already one quote
        encountered, the first of a pair.

    @return
        FIELDSTART_QUOTE if first quote in a field, either starting content or
                            embedded so caller should check beforehand.
        FIRST_QUOTE      if first of a doubled quote
        SECOND_QUOTE     if second of a doubled quote
        FIELDEND_QUOTE   if end of field quote
        DONTKNOW_QUOTE   if an unescaped quote we don't consider as end of field,
                            do not increment nQuotes in caller then!
 */
static QuoteType lcl_isEscapedOrFieldEndQuote( sal_Int32 nQuotes, const sal_Unicode* p,
        const sal_Unicode* pSeps, sal_Unicode cStr, sal_Unicode& rcDetectSep )
{
    if ((nQuotes & 1) == 0)
    {
        if (p[-1] == cStr)
            return SECOND_QUOTE;
        else
        {
            SAL_WARN( "sc", "lcl_isEscapedOrFieldEndQuote: really want a FIELDSTART_QUOTE?");
            return FIELDSTART_QUOTE;
        }
    }
    if (p[1] == cStr)
        return FIRST_QUOTE;
    return lcl_isFieldEndQuote( p, pSeps, rcDetectSep);
}

/** Append characters of [p1,p2) to rField.

    @returns TRUE if ok; FALSE if data overflow, truncated
 */
static bool lcl_appendLineData( OUString& rField, const sal_Unicode* p1, const sal_Unicode* p2 )
{
    if (rField.getLength() + (p2 - p1) <= nArbitraryCellLengthLimit)
    {
        rField += std::u16string_view( p1, p2 - p1 );
        return true;
    }
    else
    {
        SAL_WARN( "sc", "lcl_appendLineData: data overflow");
        rField += std::u16string_view( p1, nArbitraryCellLengthLimit - rField.getLength() );
        return false;
    }
}

namespace {

enum class DoubledQuoteMode
{
    KEEP_ALL,   // both are taken, additionally start and end quote are included in string
    ESCAPE,     // escaped quote, one is taken, one ignored
};

}

static const sal_Unicode* lcl_ScanString( const sal_Unicode* p, OUString& rString,
            const sal_Unicode* pSeps, sal_Unicode cStr, DoubledQuoteMode eMode, bool& rbOverflowCell )
{
    if (eMode != DoubledQuoteMode::KEEP_ALL)
        p++;    //! jump over opening quote
    bool bCont;
    do
    {
        bCont = false;
        const sal_Unicode* p0 = p;
        for( ;; )
        {
            if( !*p )
                break;
            if( *p == cStr )
            {
                if ( *++p != cStr )
                {
                    // break or continue for loop
                    if (eMode == DoubledQuoteMode::ESCAPE)
                    {
                        sal_Unicode cDetectSep = 0xffff;    // No separator detection here.
                        if (lcl_isFieldEndQuote( p-1, pSeps, cDetectSep) == FIELDEND_QUOTE)
                            break;
                        else
                            continue;
                    }
                    else
                        break;
                }
                // doubled quote char
                switch ( eMode )
                {
                    case DoubledQuoteMode::KEEP_ALL :
                        p++;            // both for us (not breaking for-loop)
                    break;
                    case DoubledQuoteMode::ESCAPE :
                        p++;            // one for us (breaking for-loop)
                        bCont = true;   // and more
                    break;
                }
                if ( eMode == DoubledQuoteMode::ESCAPE )
                    break;
            }
            else
                p++;
        }
        if ( p0 < p )
        {
            if (!lcl_appendLineData( rString, p0, ((eMode != DoubledQuoteMode::KEEP_ALL && (*p || *(p-1) == cStr)) ? p-1 : p)))
                rbOverflowCell = true;
        }
    } while ( bCont );
    return p;
}

static void lcl_UnescapeSylk( OUString & rString, SylkVersion eVersion )
{
    // Older versions didn't escape the semicolon.
    // Older versions quoted the string and doubled embedded quotes, but not
    // the semicolons, which was plain wrong.
    if (eVersion >= SylkVersion::OOO32)
        rString = rString.replaceAll(";;", ";");
    else
        rString = rString.replaceAll("\"\"", "\"");

    rString = rString.replaceAll(SYLK_LF, "\n");
}

static const sal_Unicode* lcl_ScanSylkString( const sal_Unicode* p,
        OUString& rString, SylkVersion eVersion )
{
    const sal_Unicode* pStartQuote = p;
    const sal_Unicode* pEndQuote = nullptr;
    while( *(++p) )
    {
        if( *p == '"' )
        {
            pEndQuote = p;
            if (eVersion >= SylkVersion::OOO32)
            {
                if (*(p+1) == ';')
                {
                    if (*(p+2) == ';')
                    {
                        p += 2;     // escaped ';'
                        pEndQuote = nullptr;
                    }
                    else
                        break;      // end field
                }
            }
            else
            {
                if (*(p+1) == '"')
                {
                    ++p;            // escaped '"'
                    pEndQuote = nullptr;
                }
                else if (*(p+1) == ';')
                    break;          // end field
            }
        }
    }
    if (!pEndQuote)
        pEndQuote = p;  // Take all data as string.
    rString += std::u16string_view(pStartQuote + 1, pEndQuote - pStartQuote - 1 );
    lcl_UnescapeSylk( rString, eVersion);
    return p;
}

static const sal_Unicode* lcl_ScanSylkFormula( const sal_Unicode* p,
        OUString& rString, SylkVersion eVersion )
{
    const sal_Unicode* pStart = p;
    if (eVersion >= SylkVersion::OOO32)
    {
        while (*p)
        {
            if (*p == ';')
            {
                if (*(p+1) == ';')
                    ++p;        // escaped ';'
                else
                    break;      // end field
            }
            ++p;
        }
        rString += std::u16string_view( pStart, p - pStart);
        lcl_UnescapeSylk( rString, eVersion);
    }
    else
    {
        // Nasty. If in old versions the formula contained a semicolon, it was
        // quoted and embedded quotes were doubled, but semicolons were not. If
        // there was no semicolon, it could still contain quotes and doubled
        // embedded quotes if it was something like ="a""b", which was saved as
        // E"a""b" as is and has to be preserved, even if older versions
        // couldn't even load it correctly. However, theoretically another
        // field might follow and thus the line contain a semicolon again, such
        // as ...;E"a""b";...
        bool bQuoted = false;
        if (*p == '"')
        {
            // May be a quoted expression or just a string constant expression
            // with quotes.
            while (*(++p))
            {
                if (*p == '"')
                {
                    if (*(p+1) == '"')
                        ++p;            // escaped '"'
                    else
                        break;          // closing '"', had no ';' yet
                }
                else if (*p == ';')
                {
                    bQuoted = true;     // ';' within quoted expression
                    break;
                }
            }
            p = pStart;
        }
        if (bQuoted)
            p = lcl_ScanSylkString( p, rString, eVersion);
        else
        {
            while (*p && *p != ';')
                ++p;
            rString += std::u16string_view( pStart, p - pStart);
        }
    }
    return p;
}

static void lcl_DoubleEscapeChar( OUString& rString, sal_Unicode cStr )
{
    sal_Int32 n = 0;
    while( ( n = rString.indexOf( cStr, n ) ) != -1 )
    {
        rString = rString.replaceAt( n, 0, rtl::OUStringChar(cStr) );
        n += 2;
    }
}

static void lcl_WriteString( SvStream& rStrm, OUString& rString, sal_Unicode cQuote, sal_Unicode cEsc )
{
    if (cEsc)
        lcl_DoubleEscapeChar( rString, cEsc );

    if (cQuote)
    {
        rString = OUStringChar(cQuote) + rString + OUStringChar(cQuote);
    }

    ScImportExport::WriteUnicodeOrByteString( rStrm, rString );
}

static void lcl_WriteSimpleString( SvStream& rStrm, const OUString& rString )
{
    ScImportExport::WriteUnicodeOrByteString( rStrm, rString );
}

bool ScImportExport::Text2Doc( SvStream& rStrm )
{
    bool bOk = true;

    sal_Unicode pSeps[2];
    pSeps[0] = cSep;
    pSeps[1] = 0;

    ScSetStringParam aSetStringParam;
    aSetStringParam.mbCheckLinkFormula = true;

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    sal_uInt64 nOldPos = rStrm.Tell();
    rStrm.StartReadingUnicodeText( rStrm.GetStreamCharSet() );
    bool   bData = !bSingle;
    if( !bSingle)
        bOk = StartPaste();

    while( bOk )
    {
        OUString aLine;
        OUString aCell;
        SCROW nRow = nStartRow;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            rStrm.ReadUniOrByteStringLine( aLine, rStrm.GetStreamCharSet(), nArbitraryLineLengthLimit );
            // tdf#125440 When inserting tab separated string, consider quotes as field markers
            DoubledQuoteMode mode = aLine.indexOf("\t") >= 0 ? DoubledQuoteMode::ESCAPE : DoubledQuoteMode::KEEP_ALL;
            if( rStrm.eof() )
                break;
            SCCOL nCol = nStartCol;
            const sal_Unicode* p = aLine.getStr();
            while( *p )
            {
                aCell.clear();
                const sal_Unicode* q = p;
                while (*p && *p != cSep)
                {
                    // Always look for a pairing quote and ignore separator in between.
                    while (*p && *p == cStr)
                        q = p = lcl_ScanString( p, aCell, pSeps, cStr, mode, bOverflowCell );
                    // All until next separator or quote.
                    while (*p && *p != cSep && *p != cStr)
                        ++p;
                    if (!lcl_appendLineData( aCell, q, p))
                        bOverflowCell = true;   // display warning on import
                    q = p;
                }
                if (*p)
                    ++p;
                if (rDoc.ValidCol(nCol) && rDoc.ValidRow(nRow) )
                {
                    if( bSingle )
                    {
                        if (nCol>nEndCol) nEndCol = nCol;
                        if (nRow>nEndRow) nEndRow = nRow;
                    }
                    if( bData && nCol <= nEndCol && nRow <= nEndRow )
                        rDoc.SetString( nCol, nRow, aRange.aStart.Tab(), aCell, &aSetStringParam );
                }
                else                            // too many columns/rows
                {
                    if (!rDoc.ValidRow(nRow))
                        bOverflowRow = true;    // display warning on import
                    if (!rDoc.ValidCol(nCol))
                        bOverflowCol = true;    // display warning on import
                }
                ++nCol;
            }
            ++nRow;
        }

        if( !bData )
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nEndRow );
            bOk = StartPaste();
            bData = true;
        }
        else
            break;
    }

    EndPaste();
    if (bOk && mbImportBroadcast)
    {
        rDoc.BroadcastCells(aRange, SfxHintId::ScDataChanged);
        pDocSh->PostDataChanged();
    }

    return bOk;
}

//  Extended Ascii-Import

static bool lcl_PutString(
    ScDocumentImport& rDocImport, bool bUseDocImport,
    SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rStr, sal_uInt8 nColFormat,
    SvNumberFormatter* pFormatter, bool bDetectNumFormat, bool bEvaluateFormulas, bool bSkipEmptyCells,
    const ::utl::TransliterationWrapper& rTransliteration, CalendarWrapper& rCalendar,
    const ::utl::TransliterationWrapper* pSecondTransliteration, CalendarWrapper* pSecondCalendar )
{
    ScDocument& rDoc = rDocImport.getDoc();
    bool bMultiLine = false;
    if ( nColFormat == SC_COL_SKIP || !rDoc.ValidCol(nCol) || !rDoc.ValidRow(nRow) )
        return bMultiLine;
    if ( rStr.isEmpty() )
    {
        if ( !bSkipEmptyCells )
        {   // delete destination cell
            if ( bUseDocImport )
                rDocImport.setAutoInput(ScAddress(nCol, nRow, nTab), rStr );
            else
                rDoc.SetString( nCol, nRow, nTab, rStr );
        }
        return false;
    }

    const bool bForceFormulaText = (!bEvaluateFormulas && rStr[0] == '=');
    if (nColFormat == SC_COL_TEXT || bForceFormulaText)
    {
        if ( bUseDocImport )
        {
            double fDummy;
            sal_uInt32 nIndex = 0;
            if (bForceFormulaText || rDoc.GetFormatTable()->IsNumberFormat(rStr, nIndex, fDummy))
            {
                // Set the format of this cell to Text.
                // This is only necessary for ScDocumentImport,
                // ScDocument::SetTextCell() forces it by ScSetStringParam.
                sal_uInt32 nFormat = rDoc.GetFormatTable()->GetStandardFormat(SvNumFormatType::TEXT);
                ScPatternAttr aNewAttrs(rDoc.GetPool());
                SfxItemSet& rSet = aNewAttrs.GetItemSet();
                rSet.Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat) );
                rDoc.ApplyPattern(nCol, nRow, nTab, aNewAttrs);
            }
            if (ScStringUtil::isMultiline(rStr))
            {
                ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
                rEngine.SetTextCurrentDefaults(rStr);
                rDocImport.setEditCell(ScAddress(nCol, nRow, nTab), rEngine.CreateTextObject());
                return true;
            }
            else
            {
                rDocImport.setStringCell(ScAddress(nCol, nRow, nTab), rStr);
                return false;
            }
        }
        else
        {
            rDoc.SetTextCell(ScAddress(nCol, nRow, nTab), rStr);
            return bMultiLine;
        }
    }

    if ( nColFormat == SC_COL_ENGLISH )
    {
        //! SetString with Extra-Flag ???

        SvNumberFormatter* pDocFormatter = rDoc.GetFormatTable();
        sal_uInt32 nEnglish = pDocFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
        double fVal;
        if ( pDocFormatter->IsNumberFormat( rStr, nEnglish, fVal ) )
        {
            // Numberformat will not be set to English
            if ( bUseDocImport )
                rDocImport.setNumericCell( ScAddress( nCol, nRow, nTab ), fVal );
            else
                rDoc.SetValue( nCol, nRow, nTab, fVal );
            return bMultiLine;
        }
        // else, continue with SetString
    }
    else if ( nColFormat != SC_COL_STANDARD ) // Datumformats
    {
        const sal_uInt16 nMaxNumberParts = 7; // Y-M-D h:m:s.t
        const sal_Int32 nLen = rStr.getLength();
        sal_Int32 nStart[nMaxNumberParts];
        sal_Int32 nEnd[nMaxNumberParts];

        bool bIso;
        sal_uInt16 nDP, nMP, nYP;
        switch ( nColFormat )
        {
            case SC_COL_YMD: nDP = 2; nMP = 1; nYP = 0; bIso = true; break;
            case SC_COL_MDY: nDP = 1; nMP = 0; nYP = 2; bIso = false; break;
            case SC_COL_DMY:
            default:         nDP = 0; nMP = 1; nYP = 2; bIso = false; break;
        }

        sal_uInt16 nFound = 0;
        bool bInNum = false;
        for (sal_Int32 nPos = 0; nPos < nLen && (bInNum || nFound < nMaxNumberParts); ++nPos)
        {
            bool bLetter = false;
            if (rtl::isAsciiDigit(rStr[nPos]) ||
                    (((!bInNum && nFound==nMP) || (bInNum && nFound==nMP+1))
                     && (bLetter = ScGlobal::getCharClass().isLetterNumeric( rStr, nPos))))
            {
                if (!bInNum)
                {
                    bInNum = true;
                    nStart[nFound] = nPos;
                    ++nFound;
                }
                nEnd[nFound-1] = nPos;
                if (bIso && (bLetter || (2 <= nFound && nFound <= 6 && nPos > nStart[nFound-1] + 1)))
                    // Each M,D,h,m,s at most 2 digits.
                    bIso = false;
            }
            else
            {
                bInNum = false;
                if (bIso)
                {
                    // ([+-])YYYY-MM-DD([T ]hh:mm(:ss(.fff)))(([+-])TZ)
                    // XXX NOTE: timezone is accepted here, but number
                    // formatter parser will not, so the end result will be
                    // type Text to preserve timezone information.
                    switch (rStr[nPos])
                    {
                        case '+':
                            if (nFound >= 5 && nPos == nEnd[nFound-1] + 1)
                                // Accept timezone offset.
                                ;
                            else if (nPos > 0)
                                // Accept one leading sign.
                                bIso = false;
                        break;
                        case '-':
                            if (nFound >= 5 && nPos == nEnd[nFound-1] + 1)
                                // Accept timezone offset.
                                ;
                            else if (nFound == 0 && nPos > 0)
                                // Accept one leading sign.
                                bIso = false;
                            else if (nFound < 1 || 2 < nFound || nPos != nEnd[nFound-1] + 1)
                                // Not immediately after 1 or 1-2
                                bIso = false;
                        break;
                        case 'T':
                        case ' ':
                            if (nFound != 3 || nPos != nEnd[nFound-1] + 1)
                                // Not immediately after 1-2-3
                                bIso = false;
                        break;
                        case ':':
                            if (nFound < 4 || 5 < nFound || nPos != nEnd[nFound-1] + 1)
                                // Not at 1-2-3T4:5:
                                bIso = false;
                        break;
                        case '.':
                        case ',':
                            if (nFound != 6 || nPos != nEnd[nFound-1] + 1)
                                // Not at 1-2-3T4:5:6.
                                bIso = false;
                        break;
                        case 'Z':
                            if (nFound >= 5 && nPos == nEnd[nFound-1] + 1)
                                // Accept Zero timezone.
                                ;
                            else
                                bIso = false;
                        break;
                        default:
                            bIso = false;
                    }
                }
            }
        }

        if (nFound < 3)
            bIso = false;

        if (bIso)
        {
            // Leave conversion and detection of various possible number
            // formats to the number formatter. ISO is recognized in any locale
            // so we can directly use the document's formatter.
            sal_uInt32 nFormat = 0;
            double fVal = 0.0;
            SvNumberFormatter* pDocFormatter = rDoc.GetFormatTable();
            if (pDocFormatter->IsNumberFormat( rStr, nFormat, fVal))
            {
                if (pDocFormatter->GetType(nFormat) & SvNumFormatType::DATE)
                {
                    ScAddress aPos(nCol,nRow,nTab);
                    if (bUseDocImport)
                        rDocImport.setNumericCell(aPos, fVal);
                    else
                        rDoc.SetValue(aPos, fVal);
                    rDoc.SetNumberFormat(aPos, nFormat);

                    return bMultiLine;     // success
                }
            }
            // If we reach here it is type Text (e.g. timezone or trailing
            // characters). Handled below.
        }

        if ( nFound == 1 )
        {
            //  try to break one number (without separators) into date fields

            sal_Int32 nDateStart = nStart[0];
            sal_Int32 nDateLen = nEnd[0] + 1 - nDateStart;

            if ( nDateLen >= 5 && nDateLen <= 8 &&
                    ScGlobal::getCharClass().isNumeric( rStr.copy( nDateStart, nDateLen ) ) )
            {
                //  6 digits: 2 each for day, month, year
                //  8 digits: 4 for year, 2 each for day and month
                //  5 or 7 digits: first field is shortened by 1

                bool bLongYear = ( nDateLen >= 7 );
                bool bShortFirst = ( nDateLen == 5 || nDateLen == 7 );

                sal_uInt16 nFieldStart = nDateStart;
                for (sal_uInt16 nPos=0; nPos<3; nPos++)
                {
                    sal_uInt16 nFieldEnd = nFieldStart + 1;     // default: 2 digits
                    if ( bLongYear && nPos == nYP )
                        nFieldEnd += 2;                     // 2 extra digits for long year
                    if ( bShortFirst && nPos == 0 )
                        --nFieldEnd;                        // first field shortened?

                    nStart[nPos] = nFieldStart;
                    nEnd[nPos]   = nFieldEnd;
                    nFieldStart  = nFieldEnd + 1;
                }
                nFound = 3;
            }
        }

        if (!bIso && nFound >= 3)
        {
            using namespace ::com::sun::star;
            bool bSecondCal = false;
            sal_uInt16 nDay  = static_cast<sal_uInt16>(rStr.copy( nStart[nDP], nEnd[nDP]+1-nStart[nDP] ).toInt32());
            sal_uInt16 nYear = static_cast<sal_uInt16>(rStr.copy( nStart[nYP], nEnd[nYP]+1-nStart[nYP] ).toInt32());
            OUString aMStr = rStr.copy( nStart[nMP], nEnd[nMP]+1-nStart[nMP] );
            sal_Int16 nMonth = static_cast<sal_Int16>(aMStr.toInt32());
            if (!nMonth)
            {
                static constexpr OUStringLiteral aSepShortened = u"SEP";
                uno::Sequence< i18n::CalendarItem2 > xMonths;
                sal_Int32 i, nMonthCount;
                //  first test all month names from local international
                xMonths = rCalendar.getMonths();
                nMonthCount = xMonths.getLength();
                for (i=0; i<nMonthCount && !nMonth; i++)
                {
                    if ( rTransliteration.isEqual( aMStr, xMonths[i].FullName ) ||
                         rTransliteration.isEqual( aMStr, xMonths[i].AbbrevName ) )
                        nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                    else if ( i == 8 && rTransliteration.isEqual( "SEPT",
                                xMonths[i].AbbrevName ) &&
                            rTransliteration.isEqual( aMStr, aSepShortened ) )
                    {   // correct English abbreviation is SEPT,
                        // but data mostly contains SEP only
                        nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                    }
                }
                //  if none found, then test english month names
                if ( !nMonth && pSecondCalendar && pSecondTransliteration )
                {
                    xMonths = pSecondCalendar->getMonths();
                    nMonthCount = xMonths.getLength();
                    for (i=0; i<nMonthCount && !nMonth; i++)
                    {
                        if ( pSecondTransliteration->isEqual( aMStr, xMonths[i].FullName ) ||
                             pSecondTransliteration->isEqual( aMStr, xMonths[i].AbbrevName ) )
                        {
                            nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                            bSecondCal = true;
                        }
                        else if ( i == 8 && pSecondTransliteration->isEqual(
                                    aMStr, aSepShortened ) )
                        {   // correct English abbreviation is SEPT,
                            // but data mostly contains SEP only
                            nMonth = sal::static_int_cast<sal_Int16>( i+1 );
                            bSecondCal = true;
                        }
                    }
                }
            }

            SvNumberFormatter* pDocFormatter = rDoc.GetFormatTable();
            if ( nYear < 100 )
                nYear = pDocFormatter->ExpandTwoDigitYear( nYear );

            CalendarWrapper* pCalendar = (bSecondCal ? pSecondCalendar : &rCalendar);
            sal_Int16 nNumMonths = pCalendar->getNumberOfMonthsInYear();
            if ( nDay && nMonth && nDay<=31 && nMonth<=nNumMonths )
            {
                --nMonth;
                pCalendar->setValue( i18n::CalendarFieldIndex::DAY_OF_MONTH, nDay );
                pCalendar->setValue( i18n::CalendarFieldIndex::MONTH, nMonth );
                pCalendar->setValue( i18n::CalendarFieldIndex::YEAR, nYear );
                sal_Int16 nHour, nMinute, nSecond;
                // #i14974# The imported value should have no fractional value, so set the
                // time fields to zero (ICU calendar instance defaults to current date/time)
                nHour = nMinute = nSecond = 0;
                if (nFound > 3)
                    nHour = static_cast<sal_Int16>(rStr.copy( nStart[3], nEnd[3]+1-nStart[3]).toInt32());
                if (nFound > 4)
                    nMinute = static_cast<sal_Int16>(rStr.copy( nStart[4], nEnd[4]+1-nStart[4]).toInt32());
                if (nFound > 5)
                    nSecond = static_cast<sal_Int16>(rStr.copy( nStart[5], nEnd[5]+1-nStart[5]).toInt32());
                // do not use calendar's milliseconds, to avoid fractional part truncation
                double fFrac = 0.0;
                if (nFound > 6)
                {
                    sal_Unicode cDec = '.';
                    OUString aT = OUStringChar(cDec) + rStr.subView( nStart[6], nEnd[6]+1-nStart[6]);
                    rtl_math_ConversionStatus eStatus;
                    double fV = rtl::math::stringToDouble( aT, cDec, 0, &eStatus );
                    if (eStatus == rtl_math_ConversionStatus_Ok)
                        fFrac = fV / 86400.0;
                }
                sal_Int32 nPos;
                if (nFound > 3 && 1 <= nHour && nHour <= 12  // nHour 0 and >=13 can't be AM/PM
                        && (nPos = nEnd[nFound-1] + 1) < nLen)
                {
                    // Dreaded AM/PM may be following.
                    while (nPos < nLen && rStr[nPos] == ' ')
                        ++nPos;
                    if (nPos < nLen)
                    {
                        sal_Int32 nStop = nPos;
                        while (nStop < nLen && rStr[nStop] != ' ')
                            ++nStop;
                        OUString aAmPm = rStr.copy( nPos, nStop - nPos);
                        // For AM only 12 needs to be treated, whereas for PM
                        // it must not. Check both, locale and second/English
                        // strings.
                        if (nHour == 12 &&
                                (rTransliteration.isEqual( aAmPm, pFormatter->GetLocaleData()->getTimeAM()) ||
                                 (pSecondTransliteration && pSecondTransliteration->isEqual( aAmPm, "AM"))))
                        {
                            nHour = 0;
                        }
                        else if (nHour < 12 &&
                                (rTransliteration.isEqual( aAmPm, pFormatter->GetLocaleData()->getTimePM()) ||
                                 (pSecondTransliteration && pSecondTransliteration->isEqual( aAmPm, "PM"))))
                        {
                            nHour += 12;
                        }
                    }
                }
                pCalendar->setValue( i18n::CalendarFieldIndex::HOUR, nHour );
                pCalendar->setValue( i18n::CalendarFieldIndex::MINUTE, nMinute );
                pCalendar->setValue( i18n::CalendarFieldIndex::SECOND, nSecond );
                pCalendar->setValue( i18n::CalendarFieldIndex::MILLISECOND, 0 );
                if ( pCalendar->isValid() )
                {
                    double fDiff = DateTime(pDocFormatter->GetNullDate()) -
                        pCalendar->getEpochStart();
                    // #i14974# must use getLocalDateTime to get the same
                    // date values as set above
                    double fDays = pCalendar->getLocalDateTime() + fFrac;
                    fDays -= fDiff;

                    LanguageType eLatin, eCjk, eCtl;
                    rDoc.GetLanguage( eLatin, eCjk, eCtl );
                    LanguageType eDocLang = eLatin;     //! which language for date formats?

                    SvNumFormatType nType = (nFound > 3 ? SvNumFormatType::DATETIME : SvNumFormatType::DATE);
                    sal_uLong nFormat = pDocFormatter->GetStandardFormat( nType, eDocLang );
                    // maybe there is a special format including seconds or milliseconds
                    if (nFound > 5)
                        nFormat = pDocFormatter->GetStandardFormat( fDays, nFormat, nType, eDocLang);

                    ScAddress aPos(nCol,nRow,nTab);
                    if ( bUseDocImport )
                        rDocImport.setNumericCell(aPos, fDays);
                    else
                        rDoc.SetValue( aPos, fDays );
                    rDoc.SetNumberFormat(aPos, nFormat);

                    return bMultiLine;     // success
                }
            }
        }
    }

    // Standard or date not determined -> SetString / EditCell
    if( rStr.indexOf( '\n' ) == -1 )
    {
        if (!bDetectNumFormat && nColFormat == SC_COL_STANDARD)
        {
            // Import a strict ISO 8601 date(+time) string even without
            // "Detect special numbers" or "Date (YMD)".
            do
            {
                // Simple pre-check before calling more expensive parser.
                // ([+-])(Y)YYYY-MM-DD
                if (rStr.getLength() < 10)
                    break;
                const sal_Int32 n1 = rStr.indexOf('-', 1);
                if (n1 < 4)
                    break;
                const sal_Int32 n2 = rStr.indexOf('-', n1 + 1);
                if (n2 < 7 || n1 + 3 < n2)
                    break;

                css::util::DateTime aDateTime;
                if (!sax::Converter::parseDateTime( aDateTime, rStr))
                    break;

                sal_uInt32 nFormat = 0;
                double fVal = 0.0;
                SvNumberFormatter* pDocFormatter = rDoc.GetFormatTable();
                if (pDocFormatter->IsNumberFormat( rStr, nFormat, fVal))
                {
                    if (pDocFormatter->GetType(nFormat) & SvNumFormatType::DATE)
                    {
                        ScAddress aPos(nCol,nRow,nTab);
                        if (bUseDocImport)
                            rDocImport.setNumericCell(aPos, fVal);
                        else
                            rDoc.SetValue(aPos, fVal);
                        rDoc.SetNumberFormat(aPos, nFormat);

                        return bMultiLine;     // success
                    }
                }
            }
            while(false);
        }

        ScSetStringParam aParam;
        aParam.mpNumFormatter = pFormatter;
        aParam.mbDetectNumberFormat = bDetectNumFormat;
        aParam.meSetTextNumFormat = ScSetStringParam::SpecialNumberOnly;
        aParam.mbHandleApostrophe = false;
        aParam.mbCheckLinkFormula = true;
        if ( bUseDocImport )
            rDocImport.setAutoInput(ScAddress(nCol, nRow, nTab), rStr, &aParam);
        else
            rDoc.SetString( nCol, nRow, nTab, rStr, &aParam );
    }
    else
    {
        bMultiLine = true;
        ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
        rEngine.SetTextCurrentDefaults(rStr);
        if ( bUseDocImport )
            rDocImport.setEditCell(ScAddress(nCol, nRow, nTab), rEngine.CreateTextObject());
        else
            rDoc.SetEditText( ScAddress( nCol, nRow, nTab ), rEngine.CreateTextObject() );
    }
    return bMultiLine;
}

static OUString lcl_GetFixed( const OUString& rLine, sal_Int32 nStart, sal_Int32 nNext,
                     bool& rbIsQuoted, bool& rbOverflowCell )
{
    sal_Int32 nLen = rLine.getLength();
    if (nNext > nLen)
        nNext = nLen;
    if ( nNext <= nStart )
        return OUString();

    const sal_Unicode* pStr = rLine.getStr();

    sal_Int32 nSpace = nNext;
    while ( nSpace > nStart && pStr[nSpace-1] == ' ' )
        --nSpace;

    rbIsQuoted = (pStr[nStart] == '"' && pStr[nSpace-1] == '"');
    if (rbIsQuoted)
    {
        bool bFits = (nSpace - nStart - 3 <= nArbitraryCellLengthLimit);
        if (bFits)
            return rLine.copy(nStart+1, std::max< sal_Int32 >(0, nSpace-nStart-2));
        else
        {
            SAL_WARN( "sc", "lcl_GetFixed: line doesn't fit into data");
            rbOverflowCell = true;
            return rLine.copy(nStart+1, nArbitraryCellLengthLimit);
        }
    }
    else
    {
        bool bFits = (nSpace - nStart <= nArbitraryCellLengthLimit);
        if (bFits)
            return rLine.copy(nStart, nSpace-nStart);
        else
        {
            SAL_WARN( "sc", "lcl_GetFixed: line doesn't fit into data");
            rbOverflowCell = true;
            return rLine.copy(nStart, nArbitraryCellLengthLimit);
        }
    }
}

bool ScImportExport::ExtText2Doc( SvStream& rStrm )
{
    if (!pExtOptions)
        return Text2Doc( rStrm );

    sal_uInt64 const nOldPos = rStrm.Tell();
    sal_uInt64 const nRemaining = rStrm.remainingSize();
    std::unique_ptr<ScProgress> xProgress( new ScProgress( pDocSh,
            ScResId( STR_LOAD_DOC ), nRemaining, true ));
    rStrm.StartReadingUnicodeText( rStrm.GetStreamCharSet() );

    SCCOL nStartCol = aRange.aStart.Col();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nStartRow = aRange.aStart.Row();
    const SCTAB nTab = aRange.aStart.Tab();

    bool    bFixed              = pExtOptions->IsFixedLen();
    OUString aSeps              = pExtOptions->GetFieldSeps();  // Need non-const for ReadCsvLine(),
    const sal_Unicode* pSeps    = aSeps.getStr();               // but it will be const anyway (asserted below).
    bool    bMerge              = pExtOptions->IsMergeSeps();
    bool    bRemoveSpace        = pExtOptions->IsRemoveSpace();
    sal_uInt16  nInfoCount      = pExtOptions->GetInfoCount();
    const sal_Int32* pColStart  = pExtOptions->GetColStart();
    const sal_uInt8* pColFormat = pExtOptions->GetColFormat();
    tools::Long nSkipLines             = pExtOptions->GetStartRow();

    LanguageType eDocLang = pExtOptions->GetLanguage();
    SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eDocLang);
    bool bDetectNumFormat = pExtOptions->IsDetectSpecialNumber();
    bool bEvaluateFormulas = pExtOptions->IsEvaluateFormulas();
    bool bSkipEmptyCells = pExtOptions->IsSkipEmptyCells();

    // For date recognition
    ::utl::TransliterationWrapper aTransliteration(
        comphelper::getProcessComponentContext(), TransliterationFlags::IGNORE_CASE );
    aTransliteration.loadModuleIfNeeded( eDocLang );
    CalendarWrapper aCalendar( comphelper::getProcessComponentContext() );
    aCalendar.loadDefaultCalendar(
        LanguageTag::convertToLocale( eDocLang ) );
    std::unique_ptr< ::utl::TransliterationWrapper > pEnglishTransliteration;
    std::unique_ptr< CalendarWrapper > pEnglishCalendar;
    if ( eDocLang != LANGUAGE_ENGLISH_US )
    {
        pEnglishTransliteration.reset(new ::utl::TransliterationWrapper (
            comphelper::getProcessComponentContext(), TransliterationFlags::IGNORE_CASE ));
        aTransliteration.loadModuleIfNeeded( LANGUAGE_ENGLISH_US );
        pEnglishCalendar.reset(new CalendarWrapper ( comphelper::getProcessComponentContext() ));
        pEnglishCalendar->loadDefaultCalendar(
            LanguageTag::convertToLocale( LANGUAGE_ENGLISH_US ) );
    }

    OUString aLine;
    OUString aCell;
    sal_uInt16 i;
    SCROW nRow = nStartRow;
    sal_Unicode cDetectSep = 0xffff;    // No separator detection here.

    while(--nSkipLines>0)
    {
        aLine = ReadCsvLine(rStrm, !bFixed, aSeps, cStr, cDetectSep); // content is ignored
        if ( rStrm.eof() )
            break;
    }

    // Determine range for Undo.
    // We don't need this during import of a file to a new sheet or document...
    bool bDetermineRange = bUndo;
    bool bColumnsAreDetermined = false;

    // Row heights don't need to be adjusted on the fly if EndPaste() is called
    // afterwards, which happens only if bDetermineRange. This variable also
    // survives the toggle of bDetermineRange down at the end of the do{} loop.
    bool bRangeIsDetermined = bDetermineRange;

    bool bQuotedAsText = pExtOptions && pExtOptions->IsQuotedAsText();

    sal_uInt64 nOriginalStreamPos = rStrm.Tell();

    SCROW nFirstUpdateRowHeight = SCROW_MAX;
    SCROW nLastUpdateRowHeight = -1;

    ScDocumentImport aDocImport(rDoc);
    do
    {
        for( ;; )
        {
            aLine = ReadCsvLine(rStrm, !bFixed, aSeps, cStr, cDetectSep);
            if ( rStrm.eof() && aLine.isEmpty() )
                break;

            assert(pSeps == aSeps.getStr());

            if ( nRow > rDoc.MaxRow() )
            {
                bOverflowRow = true;    // display warning on import
                break;  // for
            }

            if (!bDetermineRange)
                EmbeddedNullTreatment( aLine);

            sal_Int32 nLineLen = aLine.getLength();
            SCCOL nCol = nStartCol;
            bool bMultiLine = false;
            if ( bFixed ) //  Fixed line length
            {
                if (bDetermineRange)
                {
                    if (!bColumnsAreDetermined)
                    {
                        // Yes, the check is nCol<=rDoc.MaxCol()+1, +1 because it
                        // is only an overflow if there is really data following to
                        // be put behind the last column, which doesn't happen if
                        // info is SC_COL_SKIP.
                        for (i=0; i < nInfoCount && nCol <= rDoc.MaxCol()+1; ++i)
                        {
                            const sal_uInt8 nFmt = pColFormat[i];
                            if (nFmt != SC_COL_SKIP)        // otherwise don't increment nCol either
                            {
                                if (nCol > rDoc.MaxCol())
                                    bOverflowCol = true;    // display warning on import
                                ++nCol;
                            }
                        }
                        bColumnsAreDetermined = true;
                    }
                }
                else
                {
                    sal_Int32 nStartIdx = 0;
                    // Same maxcol+1 check reason as above.
                    for (i=0; i < nInfoCount && nCol <= rDoc.MaxCol()+1; ++i)
                    {
                        sal_Int32 nNextIdx = nStartIdx;
                        if (i + 1 < nInfoCount)
                            CountVisualWidth( aLine, nNextIdx, pColStart[i+1] - pColStart[i] );
                        else
                            nNextIdx = nLineLen;
                        sal_uInt8 nFmt = pColFormat[i];
                        if (nFmt != SC_COL_SKIP)        // otherwise don't increment nCol either
                        {
                            if (nCol > rDoc.MaxCol())
                                bOverflowCol = true;    // display warning on import
                            else
                            {
                                bool bIsQuoted = false;
                                aCell = lcl_GetFixed( aLine, nStartIdx, nNextIdx, bIsQuoted, bOverflowCell );
                                if (bIsQuoted && bQuotedAsText)
                                    nFmt = SC_COL_TEXT;

                                bMultiLine |= lcl_PutString(
                                        aDocImport, !mbOverwriting, nCol, nRow, nTab, aCell, nFmt,
                                        &aNumFormatter, bDetectNumFormat, bEvaluateFormulas, bSkipEmptyCells,
                                        aTransliteration, aCalendar,
                                        pEnglishTransliteration.get(), pEnglishCalendar.get());
                            }
                            ++nCol;
                        }
                        nStartIdx = nNextIdx;
                    }
                }
            }
            else // Search for the separator
            {
                SCCOL nSourceCol = 0;
                sal_uInt16 nInfoStart = 0;
                const sal_Unicode* p = aLine.getStr();
                // Yes, the check is nCol<=rDoc.MaxCol()+1, +1 because it is only an
                // overflow if there is really data following to be put behind
                // the last column, which doesn't happen if info is
                // SC_COL_SKIP.
                while (*p && nCol <= rDoc.MaxCol()+1)
                {
                    bool bIsQuoted = false;
                    p = ScImportExport::ScanNextFieldFromString( p, aCell,
                            cStr, pSeps, bMerge, bIsQuoted, bOverflowCell, bRemoveSpace );

                    sal_uInt8 nFmt = SC_COL_STANDARD;
                    for ( i=nInfoStart; i<nInfoCount; i++ )
                    {
                        if ( pColStart[i] == nSourceCol + 1 )       // pColStart is 1-based
                        {
                            nFmt = pColFormat[i];
                            nInfoStart = i + 1;     // ColInfos are in succession
                            break;  // for
                        }
                    }
                    if ( nFmt != SC_COL_SKIP )
                    {
                        if (nCol > rDoc.MaxCol())
                            bOverflowCol = true;    // display warning on import
                        else if (!bDetermineRange)
                        {
                            if (bIsQuoted && bQuotedAsText)
                                nFmt = SC_COL_TEXT;

                            bMultiLine |= lcl_PutString(
                                aDocImport, !mbOverwriting, nCol, nRow, nTab, aCell, nFmt,
                                &aNumFormatter, bDetectNumFormat, bEvaluateFormulas, bSkipEmptyCells,
                                aTransliteration, aCalendar,
                                pEnglishTransliteration.get(), pEnglishCalendar.get());
                        }
                        ++nCol;
                    }

                    ++nSourceCol;
                }
            }
            if (nEndCol < nCol)
                nEndCol = nCol;     //! points to the next free or even rDoc.MaxCol()+2

            if (!bDetermineRange)
            {
                if (bMultiLine && !bRangeIsDetermined && pDocSh)
                {   // Adjust just once at the end for a whole range.
                    nFirstUpdateRowHeight = std::min( nFirstUpdateRowHeight, nRow );
                    nLastUpdateRowHeight = std::max( nLastUpdateRowHeight, nRow );
                }
                xProgress->SetStateOnPercent( rStrm.Tell() - nOldPos );
            }
            ++nRow;
        }
        // so far nRow/nEndCol pointed to the next free
        if (nRow > nStartRow)
            --nRow;
        if (nEndCol > nStartCol)
            nEndCol = ::std::min( static_cast<SCCOL>(nEndCol - 1), rDoc.MaxCol());

        if (bDetermineRange)
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nRow );

            if ( !mbApi && nStartCol != nEndCol &&
                 !rDoc.IsBlockEmpty( nTab, nStartCol + 1, nStartRow, nEndCol, nRow ) )
            {
                ScReplaceWarnBox aBox(ScDocShell::GetActiveDialogParent());
                if (aBox.run() != RET_YES)
                {
                    return false;
                }
            }

            rStrm.Seek( nOriginalStreamPos );
            nRow = nStartRow;
            if (!StartPaste())
            {
                EndPaste(false);
                return false;
            }
        }

        bDetermineRange = !bDetermineRange;     // toggle
    } while (!bDetermineRange);

    if ( !mbOverwriting )
        aDocImport.finalize();

    xProgress.reset();    // make room for AdjustRowHeight progress

    if( nFirstUpdateRowHeight < nLastUpdateRowHeight && pDocSh )
        pDocSh->AdjustRowHeight( nFirstUpdateRowHeight, nLastUpdateRowHeight, nTab);

    if (bRangeIsDetermined)
        EndPaste(false);

    if (mbImportBroadcast && !mbOverwriting)
    {
        rDoc.BroadcastCells(aRange, SfxHintId::ScDataChanged);
        pDocSh->PostDataChanged();
    }
    return true;
}

void ScImportExport::EmbeddedNullTreatment( OUString & rStr )
{
    // A nasty workaround for data with embedded NULL characters. As long as we
    // can't handle them properly as cell content (things assume 0-terminated
    // strings at too many places) simply strip all NULL characters from raw
    // data. Excel does the same. See fdo#57841 for sample data.

    // The normal case is no embedded NULL, check first before de-/allocating
    // ustring stuff.
    sal_Unicode cNull = 0;
    if (rStr.indexOf( cNull) >= 0)
    {
        rStr = rStr.replaceAll( std::u16string_view( &cNull, 1), "");
    }
}

const sal_Unicode* ScImportExport::ScanNextFieldFromString( const sal_Unicode* p,
        OUString& rField, sal_Unicode cStr, const sal_Unicode* pSeps, bool bMergeSeps, bool& rbIsQuoted,
        bool& rbOverflowCell, bool bRemoveSpace )
{
    rbIsQuoted = false;
    rField.clear();
    const sal_Unicode cBlank = ' ';
    if (cStr && !ScGlobal::UnicodeStrChr(pSeps, cBlank))
    {
        // Cope with broken generators that put leading blanks before a quoted
        // field, like "field1", "field2", "..."
        // NOTE: this is not in conformance with http://tools.ietf.org/html/rfc4180
        const sal_Unicode* pb = p;
        while (*pb == cBlank)
            ++pb;
        if (*pb == cStr)
            p = pb;
    }
    if (cStr && *p == cStr) // String in quotes
    {
        rbIsQuoted = true;
        const sal_Unicode* p1;
        p1 = p = lcl_ScanString( p, rField, pSeps, cStr, DoubledQuoteMode::ESCAPE, rbOverflowCell );
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        // Append remaining unquoted and undelimited data (dirty, dirty) to
        // this field.
        if (p > p1)
        {
            const sal_Unicode* ptrim_f = p;
            if ( bRemoveSpace )
            {
                while ( ptrim_f > p1  && ( *(ptrim_f - 1) == cBlank ) )
                    --ptrim_f;
            }
            if (!lcl_appendLineData( rField, p1, ptrim_f))
                rbOverflowCell = true;
        }
        if( *p )
            p++;
    }
    else                        // up to delimiter
    {
        const sal_Unicode* p0 = p;
        while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
        const sal_Unicode* ptrim_i = p0;
        const sal_Unicode* ptrim_f = p;  // [ptrim_i,ptrim_f) is cell data after trimming
        if ( bRemoveSpace )
        {
            while ( ptrim_i < ptrim_f && *ptrim_i == cBlank )
                ++ptrim_i;
            while ( ptrim_f > ptrim_i && ( *(ptrim_f - 1) == cBlank ) )
                --ptrim_f;
        }
        if (!lcl_appendLineData( rField, ptrim_i, ptrim_f))
            rbOverflowCell = true;
        if( *p )
            p++;
    }
    if ( bMergeSeps )           // skip following delimiters
    {
        while ( *p && ScGlobal::UnicodeStrChr( pSeps, *p ) )
            p++;
    }
    return p;
}

namespace {

/**
 * Check if a given string has any line break characters or separators.
 *
 * @param rStr string to inspect.
 * @param cSep separator character.
 */
bool hasLineBreaksOrSeps( const OUString& rStr, sal_Unicode cSep )
{
    const sal_Unicode* p = rStr.getStr();
    for (sal_Int32 i = 0, n = rStr.getLength(); i < n; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (c == cSep)
            // separator found.
            return true;

        switch (c)
        {
            case '\n':
            case '\r':
                // line break found.
                return true;
            default:
                ;
        }
    }
    return false;
}

}

bool ScImportExport::Doc2Text( SvStream& rStrm )
{
    SCCOL nCol;
    SCROW nRow;
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nStartTab = aRange.aStart.Tab();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    SCTAB nEndTab = aRange.aEnd.Tab();

    if (!rDoc.GetClipParam().isMultiRange() && nStartTab == nEndTab)
        if (!rDoc.ShrinkToDataArea( nStartTab, nStartCol, nStartRow, nEndCol, nEndRow ))
            return false;

    OUString aCellStr;

    bool bConvertLF = (GetSystemLineEnd() != LINEEND_LF);

    // We need to cache sc::ColumnBlockPosition per each column, tab is always nStartTab.
    std::vector< sc::ColumnBlockPosition > blockPos( nEndCol - nStartCol + 1 );
    for( SCCOL i = nStartCol; i <= nEndCol; ++i )
        rDoc.InitColumnBlockPosition( blockPos[ i - nStartCol ], nStartTab, i );
    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        if (bIncludeFiltered || !rDoc.RowFiltered( nRow, nStartTab ))
        {
            for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            {
                ScAddress aPos(nCol, nRow, nStartTab);
                sal_uInt32 nNumFmt = rDoc.GetNumberFormat(aPos);
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();

                ScRefCellValue aCell(rDoc, aPos, blockPos[ nCol - nStartCol ]);
                switch (aCell.meType)
                {
                    case CELLTYPE_FORMULA:
                    {
                        if (bFormulas)
                        {
                            aCellStr = aCell.mpFormula->GetFormula();
                            if( aCellStr.indexOf( cSep ) != -1 )
                                lcl_WriteString( rStrm, aCellStr, cStr, cStr );
                            else
                                lcl_WriteSimpleString( rStrm, aCellStr );
                        }
                        else
                        {
                            const Color* pColor;
                            aCellStr = ScCellFormat::GetString(aCell, nNumFmt, &pColor, *pFormatter, rDoc);

                            bool bMultiLineText = ( aCellStr.indexOf( '\n' ) != -1 );
                            if( bMultiLineText )
                            {
                                if( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSpace )
                                    aCellStr = aCellStr.replaceAll( "\n", " " );
                                else if ( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSystem && bConvertLF )
                                    aCellStr = convertLineEnd(aCellStr, GetSystemLineEnd());
                            }

                            if( mExportTextOptions.mcSeparatorConvertTo && cSep )
                                aCellStr = aCellStr.replaceAll( OUStringChar(cSep), OUStringChar(mExportTextOptions.mcSeparatorConvertTo) );

                            if( mExportTextOptions.mbAddQuotes && ( aCellStr.indexOf( cSep ) != -1 ) )
                                lcl_WriteString( rStrm, aCellStr, cStr, cStr );
                            else
                                lcl_WriteSimpleString( rStrm, aCellStr );
                        }
                    }
                    break;
                    case CELLTYPE_VALUE:
                    {
                        const Color* pColor;
                        aCellStr = ScCellFormat::GetString(aCell, nNumFmt, &pColor, *pFormatter, rDoc);
                        lcl_WriteSimpleString( rStrm, aCellStr );
                    }
                    break;
                    case CELLTYPE_NONE:
                    break;
                    default:
                    {
                        const Color* pColor;
                        aCellStr = ScCellFormat::GetString(aCell, nNumFmt, &pColor, *pFormatter, rDoc);

                        bool bMultiLineText = ( aCellStr.indexOf( '\n' ) != -1 );
                        if( bMultiLineText )
                        {
                            if( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSpace )
                                aCellStr = aCellStr.replaceAll( "\n", " " );
                            else if ( mExportTextOptions.meNewlineConversion == ScExportTextOptions::ToSystem && bConvertLF )
                                aCellStr = convertLineEnd(aCellStr, GetSystemLineEnd());
                        }

                        if( mExportTextOptions.mcSeparatorConvertTo && cSep )
                            aCellStr = aCellStr.replaceAll( OUStringChar(cSep), OUStringChar(mExportTextOptions.mcSeparatorConvertTo) );

                        if( mExportTextOptions.mbAddQuotes && hasLineBreaksOrSeps(aCellStr, cSep) )
                            lcl_WriteString( rStrm, aCellStr, cStr, cStr );
                        else
                            lcl_WriteSimpleString( rStrm, aCellStr );
                    }
                }
                if( nCol < nEndCol )
                    lcl_WriteSimpleString( rStrm, OUString(cSep) );
            }
            // Do not append a line feed for one single cell.
            // NOTE: this Doc2Text() is only called for clipboard via
            // ScImportExport::ExportStream().
            if (nStartRow != nEndRow || nStartCol != nEndCol)
                WriteUnicodeOrByteEndl( rStrm );
            if( rStrm.GetError() != ERRCODE_NONE )
                break;
            if( nSizeLimit && rStrm.Tell() > nSizeLimit )
                break;
        }
    }

    return rStrm.GetError() == ERRCODE_NONE;
}

bool ScImportExport::Sylk2Doc( SvStream& rStrm )
{
    bool bOk = true;
    bool bMyDoc = false;
    SylkVersion eVersion = SylkVersion::OTHER;

    // US-English separators for StringToDouble
    sal_Unicode const cDecSep = '.';
    sal_Unicode const cGrpSep = ',';

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    sal_uInt64 nOldPos = rStrm.Tell();
    bool bData = !bSingle;
    ::std::vector< sal_uInt32 > aFormats;

    if( !bSingle)
        bOk = StartPaste();

    while( bOk )
    {
        OUString aLine;
        OUString aText;
        OString aByteLine;
        SCCOL nCol = nStartCol;
        SCROW nRow = nStartRow;
        SCCOL nRefCol = nCol;
        SCROW nRefRow = nRow;
        rStrm.Seek( nOldPos );
        for( ;; )
        {
            //! allow unicode
            rStrm.ReadLine( aByteLine );
            aLine = OStringToOUString(aByteLine, rStrm.GetStreamCharSet());
            if( rStrm.eof() )
                break;
            bool bInvalidCol = false;
            bool bInvalidRow = false;
            const sal_Unicode* p = aLine.getStr();
            sal_Unicode cTag = *p++;
            if( cTag == 'C' )       // Content
            {
                if( *p++ != ';' )
                    return false;

                bool bInvalidRefCol = false;
                bool bInvalidRefRow = false;
                while( *p )
                {
                    sal_Unicode ch = *p++;
                    ch = ScGlobal::ToUpperAlpha( ch );
                    switch( ch )
                    {
                        case 'X':
                        {
                            bInvalidCol = false;
                            bool bFail = o3tl::checked_add<SCCOL>(OUString(p).toInt32(), nStartCol - 1, nCol);
                            if (bFail || nCol < 0 || rDoc.MaxCol() < nCol)
                            {
                                SAL_WARN("sc.ui","ScImportExport::Sylk2Doc - ;X invalid nCol=" << nCol);
                                nCol = std::clamp<SCCOL>(nCol, 0, rDoc.MaxCol());
                                bInvalidCol = bOverflowCol = true;
                            }
                            break;
                        }
                        case 'Y':
                        {
                            bInvalidRow = false;
                            bool bFail = o3tl::checked_add(OUString(p).toInt32(), nStartRow - 1, nRow);
                            if (bFail || nRow < 0 || nMaxImportRow < nRow)
                            {
                                SAL_WARN("sc.ui","ScImportExport::Sylk2Doc - ;Y invalid nRow=" << nRow);
                                nRow = std::clamp<SCROW>(nRow, 0, nMaxImportRow);
                                bInvalidRow = bOverflowRow = true;
                            }
                            break;
                        }
                        case 'C':
                        {
                            bInvalidRefCol = false;
                            bool bFail = o3tl::checked_add<SCCOL>(OUString(p).toInt32(), nStartCol - 1, nRefCol);
                            if (bFail || nRefCol < 0 || rDoc.MaxCol() < nRefCol)
                            {
                                SAL_WARN("sc.ui","ScImportExport::Sylk2Doc - ;C invalid nRefCol=" << nRefCol);
                                nRefCol = std::clamp<SCCOL>(nRefCol, 0, rDoc.MaxCol());
                                bInvalidRefCol = bOverflowCol = true;
                            }
                            break;
                        }
                        case 'R':
                        {
                            bInvalidRefRow = false;
                            bool bFail = o3tl::checked_add(OUString(p).toInt32(), nStartRow - 1, nRefRow);
                            if (bFail || nRefRow < 0 || nMaxImportRow < nRefRow)
                            {
                                SAL_WARN("sc.ui","ScImportExport::Sylk2Doc - ;R invalid nRefRow=" << nRefRow);
                                nRefRow = std::clamp<SCROW>(nRefRow, 0, nMaxImportRow);
                                bInvalidRefRow = bOverflowRow = true;
                            }
                            break;
                        }
                        case 'K':
                        {
                            if( !bSingle &&
                                    ( nCol < nStartCol || nCol > nEndCol
                                      || nRow < nStartRow || nRow > nEndRow
                                      || nCol > rDoc.MaxCol() || nRow > nMaxImportRow
                                      || bInvalidCol || bInvalidRow ) )
                                break;
                            if( !bData )
                            {
                                if( nRow > nEndRow )
                                    nEndRow = nRow;
                                if( nCol > nEndCol )
                                    nEndCol = nCol;
                                break;
                            }
                            bool bText;
                            if( *p == '"' )
                            {
                                bText = true;
                                aText.clear();
                                p = lcl_ScanSylkString( p, aText, eVersion);
                            }
                            else
                                bText = false;
                            const sal_Unicode* q = p;
                            while( *q && *q != ';' )
                                q++;
                            if ( (*q != ';' || *(q+1) != 'I') && !bInvalidCol && !bInvalidRow )
                            {   // don't ignore value
                                if( bText )
                                {
                                    rDoc.EnsureTable(aRange.aStart.Tab());
                                    rDoc.SetTextCell(
                                        ScAddress(nCol, nRow, aRange.aStart.Tab()), aText);
                                }
                                else
                                {
                                    double fVal = rtl_math_uStringToDouble( p,
                                            aLine.getStr() + aLine.getLength(),
                                            cDecSep, cGrpSep, nullptr, nullptr );
                                    rDoc.SetValue( nCol, nRow, aRange.aStart.Tab(), fVal );
                                }
                            }
                        }
                        break;
                        case 'E':
                        case 'M':
                        {
                            if ( ch == 'M' )
                            {
                                if ( nRefCol < nCol )
                                    nRefCol = nCol;
                                if ( nRefRow < nRow )
                                    nRefRow = nRow;
                                if ( !bData )
                                {
                                    if( nRefRow > nEndRow )
                                        nEndRow = nRefRow;
                                    if( nRefCol > nEndCol )
                                        nEndCol = nRefCol;
                                }
                            }
                            if( !bMyDoc || !bData )
                                break;
                            aText = "=";
                            p = lcl_ScanSylkFormula( p, aText, eVersion);

                            if (bInvalidCol || bInvalidRow || (ch == 'M' && (bInvalidRefCol || bInvalidRefRow)))
                                break;

                            ScAddress aPos( nCol, nRow, aRange.aStart.Tab() );
                            /* FIXME: do we want GRAM_ODFF_A1 instead? At the
                             * end it probably should be GRAM_ODFF_R1C1, since
                             * R1C1 is what Excel writes in SYLK, or even
                             * better GRAM_ENGLISH_XL_R1C1. */
                            const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_PODF_A1;
                            ScCompiler aComp(rDoc, aPos, eGrammar);
                            std::unique_ptr<ScTokenArray> xCode(aComp.CompileString(aText)); // ctor/InsertMatrixFormula did copy TokenArray
                            rDoc.CheckLinkFormulaNeedingCheck(*xCode);
                            if ( ch == 'M' )
                            {
                                ScMarkData aMark(rDoc.GetSheetLimits());
                                aMark.SelectTable( aPos.Tab(), true );
                                rDoc.InsertMatrixFormula( nCol, nRow, nRefCol,
                                    nRefRow, aMark, OUString(), xCode.get() );
                            }
                            else
                            {
                                ScFormulaCell* pFCell = new ScFormulaCell(
                                        rDoc, aPos, *xCode, eGrammar, ScMatrixMode::NONE);
                                rDoc.SetFormulaCell(aPos, pFCell);
                            }
                        }
                        break;
                    }
                    while( *p && *p != ';' )
                        p++;
                    if( *p )
                        p++;
                }
            }
            else if( cTag == 'F' )      // Format
            {
                if( *p++ != ';' )
                    return false;
                sal_Int32 nFormat = -1;
                while( *p )
                {
                    sal_Unicode ch = *p++;
                    ch = ScGlobal::ToUpperAlpha( ch );
                    switch( ch )
                    {
                        case 'X':
                        {
                            bInvalidCol = false;
                            bool bFail = o3tl::checked_add<SCCOL>(OUString(p).toInt32(), nStartCol - 1, nCol);
                            if (bFail || nCol < 0 || rDoc.MaxCol() < nCol)
                            {
                                SAL_WARN("sc.ui","ScImportExport::Sylk2Doc - ;X invalid nCol=" << nCol);
                                nCol = std::clamp<SCCOL>(nCol, 0, rDoc.MaxCol());
                                bInvalidCol = bOverflowCol = true;
                            }
                            break;
                        }
                        case 'Y':
                        {
                            bInvalidRow = false;
                            bool bFail = o3tl::checked_add(OUString(p).toInt32(), nStartRow - 1, nRow);
                            if (bFail || nRow < 0 || nMaxImportRow < nRow)
                            {
                                SAL_WARN("sc.ui","ScImportExport::Sylk2Doc - ;Y invalid nRow=" << nRow);
                                nRow = std::clamp<SCROW>(nRow, 0, nMaxImportRow);
                                bInvalidRow = bOverflowRow = true;
                            }
                            break;
                        }
                        case 'P' :
                            if ( bData )
                            {
                                // F;P<n> sets format code of P;P<code> at
                                // current position, or at ;X;Y if specified.
                                // Note that ;X;Y may appear after ;P
                                const sal_Unicode* p0 = p;
                                while( *p && *p != ';' )
                                    p++;
                                OUString aNumber(p0, p - p0);
                                nFormat = aNumber.toInt32();
                            }
                            break;
                    }
                    while( *p && *p != ';' )
                        p++;
                    if( *p )
                        p++;
                }
                if ( !bData )
                {
                    if( nRow > nEndRow )
                        nEndRow = nRow;
                    if( nCol > nEndCol )
                        nEndCol = nCol;
                }
                if ( 0 <= nFormat && nFormat < static_cast<sal_Int32>(aFormats.size()) && !bInvalidCol && !bInvalidRow )
                {
                    sal_uInt32 nKey = aFormats[nFormat];
                    rDoc.ApplyAttr( nCol, nRow, aRange.aStart.Tab(),
                            SfxUInt32Item( ATTR_VALUE_FORMAT, nKey ) );
                }
            }
            else if( cTag == 'P' )
            {
                if ( bData && *p == ';' && *(p+1) == 'P' )
                {
                    OUString aCode( p+2 );

                    sal_uInt32 nKey;
                    sal_Int32 nCheckPos;

                    if (aCode.getLength() > 2048 && utl::ConfigManager::IsFuzzing())
                    {
                        // consider an excessive length as a failure when fuzzing
                        nCheckPos = 1;
                    }
                    else
                    {
                        // unescape doubled semicolons
                        aCode = aCode.replaceAll(";;", ";");
                        // get rid of Xcl escape characters
                        aCode = aCode.replaceAll("\x1b", "");
                        SvNumFormatType nType;
                        rDoc.GetFormatTable()->PutandConvertEntry( aCode, nCheckPos, nType, nKey,
                                                                    LANGUAGE_ENGLISH_US, ScGlobal::eLnge, false);
                    }

                    if ( nCheckPos )
                        nKey = 0;

                    aFormats.push_back( nKey );
                }
            }
            else if( cTag == 'I' && *p == 'D' )
            {
                aLine = aLine.copy(4);
                if (aLine == "CALCOOO32")
                    eVersion = SylkVersion::OOO32;
                else if (aLine == "SCALC3")
                    eVersion = SylkVersion::SCALC3;
                bMyDoc = (eVersion <= SylkVersion::OWN);
            }
            else if( cTag == 'E' )                      // End
                break;
        }
        if( !bData )
        {
            aRange.aEnd.SetCol( nEndCol );
            aRange.aEnd.SetRow( nEndRow );
            bOk = StartPaste();
            bData = true;
        }
        else
            break;
    }

    EndPaste();
    return bOk;
}

bool ScImportExport::Doc2Sylk( SvStream& rStrm )
{
    SCCOL nCol;
    SCROW nRow;
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    OUString aCellStr;
    OUString aValStr;
    lcl_WriteSimpleString( rStrm, "ID;PCALCOOO32" );
    WriteUnicodeOrByteEndl( rStrm );

    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
        {
            OUString aBufStr;
            double nVal;
            bool bForm = false;
            SCROW r = nRow - nStartRow + 1;
            SCCOL c = nCol - nStartCol + 1;
            ScRefCellValue aCell(rDoc, ScAddress(nCol, nRow, aRange.aStart.Tab()));
            CellType eType = aCell.meType;
            switch( eType )
            {
                case CELLTYPE_FORMULA:
                    bForm = bFormulas;
                    if( rDoc.HasValueData( nCol, nRow, aRange.aStart.Tab()) )
                        goto hasvalue;
                    else
                        goto hasstring;

                case CELLTYPE_VALUE:
                hasvalue:
                    nVal = rDoc.GetValue( nCol, nRow, aRange.aStart.Tab() );

                    aValStr = ::rtl::math::doubleToUString( nVal,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true );

                    aBufStr = "C;X"
                            + OUString::number( c )
                            + ";Y"
                            + OUString::number( r )
                            + ";K"
                            + aValStr;
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    goto checkformula;

                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                hasstring:
                    aCellStr = rDoc.GetString(nCol, nRow, aRange.aStart.Tab());
                    aCellStr = aCellStr.replaceAll("\n", SYLK_LF);

                    aBufStr = "C;X"
                            + OUString::number( c )
                            + ";Y"
                            + OUString::number( r )
                            + ";K";
                    lcl_WriteSimpleString( rStrm, aBufStr );
                    lcl_WriteString( rStrm, aCellStr, '"', ';' );

                checkformula:
                    if( bForm )
                    {
                        const ScFormulaCell* pFCell = aCell.mpFormula;
                        switch ( pFCell->GetMatrixFlag() )
                        {
                            case ScMatrixMode::Reference :
                                aCellStr.clear();
                            break;
                            default:
                                aCellStr = pFCell->GetFormula( formula::FormulaGrammar::GRAM_PODF_A1);
                                /* FIXME: do we want GRAM_ODFF_A1 instead? At
                                 * the end it probably should be
                                 * GRAM_ODFF_R1C1, since R1C1 is what Excel
                                 * writes in SYLK, or even better
                                 * GRAM_ENGLISH_XL_R1C1. */
                        }
                        if ( pFCell->GetMatrixFlag() != ScMatrixMode::NONE &&
                                aCellStr.startsWith("{") &&
                                aCellStr.endsWith("}") )
                        {   // cut off matrix {} characters
                            aCellStr = aCellStr.copy(1, aCellStr.getLength()-2);
                        }
                        if ( aCellStr[0] == '=' )
                            aCellStr = aCellStr.copy(1);
                        OUString aPrefix;
                        switch ( pFCell->GetMatrixFlag() )
                        {
                            case ScMatrixMode::Formula :
                            {   // diff expression with 'M' M$-extension
                                SCCOL nC;
                                SCROW nR;
                                pFCell->GetMatColsRows( nC, nR );
                                nC += c - 1;
                                nR += r - 1;
                                aPrefix = ";R"
                                        + OUString::number( nR )
                                        + ";C"
                                        + OUString::number( nC )
                                        + ";M";
                            }
                            break;
                            case ScMatrixMode::Reference :
                            {   // diff expression with 'I' M$-extension
                                ScAddress aPos;
                                (void)pFCell->GetMatrixOrigin( rDoc, aPos );
                                aPrefix = ";I;R"
                                        + OUString::number( aPos.Row() - nStartRow + 1 )
                                        + ";C"
                                        + OUString::number( aPos.Col() - nStartCol + 1 );
                            }
                            break;
                            default:
                                // formula Expression
                                aPrefix = ";E";
                        }
                        lcl_WriteSimpleString( rStrm, aPrefix );
                        if ( !aCellStr.isEmpty() )
                            lcl_WriteString( rStrm, aCellStr, 0, ';' );
                    }
                    WriteUnicodeOrByteEndl( rStrm );
                    break;

                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }
    lcl_WriteSimpleString( rStrm, OUString( 'E' ) );
    WriteUnicodeOrByteEndl( rStrm );
    return rStrm.GetError() == ERRCODE_NONE;
}

bool ScImportExport::Doc2HTML( SvStream& rStrm, const OUString& rBaseURL )
{
    // rtl_TextEncoding is ignored in ScExportHTML, read from Load/Save HTML options
    ScFormatFilter::Get().ScExportHTML( rStrm, rBaseURL, &rDoc, aRange, RTL_TEXTENCODING_DONTKNOW, bAll,
        aStreamPath, aNonConvertibleChars, maFilterOptions );
    return rStrm.GetError() == ERRCODE_NONE;
}

bool ScImportExport::Doc2RTF( SvStream& rStrm )
{
    //  rtl_TextEncoding is ignored in ScExportRTF
    ScFormatFilter::Get().ScExportRTF( rStrm, &rDoc, aRange, RTL_TEXTENCODING_DONTKNOW );
    return rStrm.GetError() == ERRCODE_NONE;
}

bool ScImportExport::Doc2Dif( SvStream& rStrm )
{
    // for DIF in the clipboard, IBM_850 is always used
    ScFormatFilter::Get().ScExportDif( rStrm, &rDoc, aRange, RTL_TEXTENCODING_IBM_850 );
    return true;
}

bool ScImportExport::Dif2Doc( SvStream& rStrm )
{
    SCTAB nTab = aRange.aStart.Tab();
    ScDocumentUniquePtr pImportDoc( new ScDocument( SCDOCMODE_UNDO ) );
    pImportDoc->InitUndo( rDoc, nTab, nTab );

    // for DIF in the clipboard, IBM_850 is always used
    ScFormatFilter::Get().ScImportDif( rStrm, pImportDoc.get(), aRange.aStart, RTL_TEXTENCODING_IBM_850 );

    SCCOL nEndCol;
    SCROW nEndRow;
    pImportDoc->GetCellArea( nTab, nEndCol, nEndRow );
    // if there are no cells in the imported content, nEndCol/nEndRow may be before the start
    if ( nEndCol < aRange.aStart.Col() )
        nEndCol = aRange.aStart.Col();
    if ( nEndRow < aRange.aStart.Row() )
        nEndRow = aRange.aStart.Row();
    aRange.aEnd = ScAddress( nEndCol, nEndRow, nTab );

    bool bOk = StartPaste();
    if (bOk)
    {
        InsertDeleteFlags nFlags = InsertDeleteFlags::ALL & ~InsertDeleteFlags::STYLES;
        rDoc.DeleteAreaTab( aRange, nFlags );
        pImportDoc->CopyToDocument(aRange, nFlags, false, rDoc);
        EndPaste();
    }

    return bOk;
}

bool ScImportExport::RTF2Doc( SvStream& rStrm, const OUString& rBaseURL )
{
    std::unique_ptr<ScEEAbsImport> pImp = ScFormatFilter::Get().CreateRTFImport( &rDoc, aRange );
    if (!pImp)
        return false;
    pImp->Read( rStrm, rBaseURL );
    aRange = pImp->GetRange();

    bool bOk = StartPaste();
    if (bOk)
    {
        InsertDeleteFlags const nFlags = InsertDeleteFlags::ALL & ~InsertDeleteFlags::STYLES;
        rDoc.DeleteAreaTab( aRange, nFlags );
        pImp->WriteToDocument();
        EndPaste();
    }
    return bOk;
}

bool ScImportExport::HTML2Doc( SvStream& rStrm, const OUString& rBaseURL )
{
    std::unique_ptr<ScEEAbsImport> pImp = ScFormatFilter::Get().CreateHTMLImport( &rDoc, rBaseURL, aRange);
    if (!pImp)
        return false;
    pImp->Read( rStrm, rBaseURL );
    aRange = pImp->GetRange();

    bool bOk = StartPaste();
    if (bOk)
    {
        // ScHTMLImport may call ScDocument::InitDrawLayer, resulting in
        // a Draw Layer but no Draw View -> create Draw Layer and View here
        if (pDocSh)
            pDocSh->MakeDrawLayer();

        InsertDeleteFlags const nFlags = InsertDeleteFlags::ALL & ~InsertDeleteFlags::STYLES;
        rDoc.DeleteAreaTab( aRange, nFlags );

        if (pExtOptions)
        {
            // Pick up import options if available.
            LanguageType eLang = pExtOptions->GetLanguage();
            SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eLang);
            bool bSpecialNumber = pExtOptions->IsDetectSpecialNumber();
            pImp->WriteToDocument(false, 1.0, &aNumFormatter, bSpecialNumber);
        }
        else
            // Regular import, with no options.
            pImp->WriteToDocument();

        EndPaste();
    }
    return bOk;
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

#else

extern "C" {
ScFormatFilterPlugin* ScFilterCreate();
}

#endif

typedef ScFormatFilterPlugin * (*FilterFn)();
ScFormatFilterPlugin &ScFormatFilter::Get()
{
    static ScFormatFilterPlugin *plugin = []()
    {
#ifndef DISABLE_DYNLOADING
        OUString sFilterLib(SVLIBRARY("scfilt"));
        static ::osl::Module aModule;
        bool bLoaded = aModule.is();
        if (!bLoaded)
            bLoaded = aModule.loadRelative(&thisModule, sFilterLib);
        if (!bLoaded)
            bLoaded = aModule.load(sFilterLib);
        if (bLoaded)
        {
            oslGenericFunction fn = aModule.getFunctionSymbol( "ScFilterCreate" );
            if (fn != nullptr)
                return reinterpret_cast<FilterFn>(fn)();
        }
        assert(false);
        return static_cast<ScFormatFilterPlugin*>(nullptr);
#else
        return ScFilterCreate();
#endif
    }();

    return *plugin;
}

// Precondition: pStr is guaranteed to be non-NULL and points to a 0-terminated
// array.
static const sal_Unicode* lcl_UnicodeStrChr( const sal_Unicode* pStr,
        sal_Unicode c )
{
    while (*pStr)
    {
        if (*pStr == c)
            return pStr;
        ++pStr;
    }
    return nullptr;
}

ScImportStringStream::ScImportStringStream( const OUString& rStr )
    : SvMemoryStream( const_cast<sal_Unicode *>(rStr.getStr()),
            rStr.getLength() * sizeof(sal_Unicode), StreamMode::READ)
{
    SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
#ifdef OSL_BIGENDIAN
    SetEndian(SvStreamEndian::BIG);
#else
    SetEndian(SvStreamEndian::LITTLE);
#endif
}

OUString ReadCsvLine( SvStream &rStream, bool bEmbeddedLineBreak,
        OUString& rFieldSeparators, sal_Unicode cFieldQuote, sal_Unicode& rcDetectSep, sal_uInt32 nMaxSourceLines )
{
    enum RetryState
    {
        FORBID,
        ALLOW,
        RETRY,
        RETRIED
    } eRetryState = (bEmbeddedLineBreak && rcDetectSep == 0 ? RetryState::ALLOW : RetryState::FORBID);

    sal_uInt64 nStreamPos = (eRetryState == RetryState::ALLOW ? rStream.Tell() : 0);

Label_RetryWithNewSep:

    if (eRetryState == RetryState::RETRY)
    {
        eRetryState = RetryState::RETRIED;
        rStream.Seek( nStreamPos);
    }

    OUString aStr;
    rStream.ReadUniOrByteStringLine(aStr, rStream.GetStreamCharSet(), nArbitraryLineLengthLimit);

    if (bEmbeddedLineBreak)
    {
        sal_uInt32 nLine = 0;

        const sal_Unicode* pSeps = rFieldSeparators.getStr();

        QuoteType eQuoteState = FIELDEND_QUOTE;
        bool bFieldStart = true;

        sal_Int32 nLastOffset = 0;
        sal_Int32 nQuotes = 0;
        while (!rStream.eof() && aStr.getLength() < nArbitraryLineLengthLimit)
        {
            const sal_Unicode * p = aStr.getStr() + nLastOffset;
            const sal_Unicode * const pStop = aStr.getStr() + aStr.getLength();
            while (p < pStop)
            {
                if (!*p)
                {
                    // Skip embedded null-characters. They don't change
                    // anything and are handled at a higher level.
                    ++p;
                    continue;
                }

                if (nQuotes)
                {
                    if (*p == cFieldQuote)
                    {
                        if (bFieldStart)
                        {
                            ++nQuotes;
                            bFieldStart = false;
                            eQuoteState = FIELDSTART_QUOTE;
                        }
                        // Do not detect a FIELDSTART_QUOTE if not in
                        // bFieldStart mode, in which case for unquoted content
                        // we are in FIELDEND_QUOTE state.
                        else if (eQuoteState != FIELDEND_QUOTE)
                        {
                            eQuoteState = lcl_isEscapedOrFieldEndQuote( nQuotes, p, pSeps, cFieldQuote, rcDetectSep);

                            if (eRetryState == RetryState::ALLOW && rcDetectSep)
                            {
                                eRetryState = RetryState::RETRY;
                                rFieldSeparators += OUStringChar(rcDetectSep);
                                pSeps = rFieldSeparators.getStr();
                                goto Label_RetryWithNewSep;
                            }

                            // DONTKNOW_QUOTE is an embedded unescaped quote we
                            // don't count for pairing.
                            if (eQuoteState != DONTKNOW_QUOTE)
                                ++nQuotes;
                        }
                    }
                    else if (eQuoteState == FIELDEND_QUOTE)
                    {
                        if (bFieldStart)
                            // If blank is a separator it starts a field, if it
                            // is not and thus maybe leading before quote we
                            // are still at start of field regarding quotes.
                            bFieldStart = (*p == ' ' || lcl_UnicodeStrChr( pSeps, *p) != nullptr);
                        else
                            bFieldStart = (lcl_UnicodeStrChr( pSeps, *p) != nullptr);
                    }
                }
                else
                {
                    if (*p == cFieldQuote && bFieldStart)
                    {
                        nQuotes = 1;
                        eQuoteState = FIELDSTART_QUOTE;
                        bFieldStart = false;
                    }
                    else if (eQuoteState == FIELDEND_QUOTE)
                    {
                        // This also skips leading blanks at beginning of line
                        // if followed by a quote. It's debatable whether we
                        // actually want that or not, but congruent with what
                        // ScanNextFieldFromString() does.
                        if (bFieldStart)
                            bFieldStart = (*p == ' ' || lcl_UnicodeStrChr( pSeps, *p) != nullptr);
                        else
                            bFieldStart = (lcl_UnicodeStrChr( pSeps, *p) != nullptr);
                    }
                }
                // A quote character inside a field content does not start
                // a quote.
                ++p;
            }

            if ((nQuotes & 1) == 0)
                // We still have a (theoretical?) problem here if due to
                // nArbitraryLineLengthLimit (or nMaxSourceLines below) we
                // split a string right between a doubled quote pair.
                break;
            else if (++nLine >= nMaxSourceLines && nMaxSourceLines > 0)
                // Unconditionally increment nLine even if nMaxSourceLines==0
                // so it can be observed in debugger.
                break;
            else
            {
                nLastOffset = aStr.getLength();
                OUString aNext;
                rStream.ReadUniOrByteStringLine(aNext, rStream.GetStreamCharSet(), nArbitraryLineLengthLimit);
                aStr += "\n" + aNext;
            }
        }
    }
    return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
