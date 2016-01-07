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

#include "xicontent.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/itemset.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include "stringutil.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "formulacell.hxx"
#include "validat.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "rangenam.hxx"
#include "arealink.hxx"
#include "stlsheet.hxx"
#include "scextopt.hxx"
#include "xlformula.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xistyle.hxx"
#include "xiescher.hxx"
#include "xiname.hxx"

#include "excform.hxx"
#include "tabprotection.hxx"
#include "documentimport.hxx"

#include <memory>
#include <utility>
#include <o3tl/make_unique.hxx>

using ::com::sun::star::uno::Sequence;
using ::std::unique_ptr;

// Shared string table ========================================================

XclImpSst::XclImpSst( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpSst::ReadSst( XclImpStream& rStrm )
{
    rStrm.Ignore( 4 );
    sal_uInt32 nStrCount = rStrm.ReaduInt32();
    auto nBytesAvailable = rStrm.GetRecLeft();
    if (nStrCount > nBytesAvailable)
    {
        SAL_WARN("sc.filter", "xls claimed to have " << nStrCount << " strings, but only " << nBytesAvailable << " bytes available, truncating");
        nStrCount = nBytesAvailable;
    }
    maStrings.clear();
    maStrings.reserve(nStrCount);
    while( (nStrCount > 0) && rStrm.IsValid() )
    {
        XclImpString aString;
        aString.Read( rStrm );
        maStrings.push_back( aString );
        --nStrCount;
    }
}

const XclImpString* XclImpSst::GetString( sal_uInt32 nSstIndex ) const
{
    return (nSstIndex < maStrings.size()) ? &maStrings[ nSstIndex ] : nullptr;
}

// Hyperlinks =================================================================

namespace {

/** Reads character array and stores it into rString.
    @param nChars  Number of following characters (not byte count!).
    @param b16Bit  true = 16-bit characters, false = 8-bit characters. */
void lclAppendString32( OUString& rString, XclImpStream& rStrm, sal_uInt32 nChars, bool b16Bit )
{
    sal_uInt16 nReadChars = ulimit_cast< sal_uInt16 >( nChars );
    rString += rStrm.ReadRawUniString( nReadChars, b16Bit );
    // ignore remaining chars
    sal_Size nIgnore = nChars - nReadChars;
    if( b16Bit )
        nIgnore *= 2;
    rStrm.Ignore( nIgnore );
}

/** Reads 32-bit string length and the character array and stores it into rString.
    @param b16Bit  true = 16-bit characters, false = 8-bit characters. */
void lclAppendString32( OUString& rString, XclImpStream& rStrm, bool b16Bit )
{
    lclAppendString32( rString, rStrm, rStrm.ReaduInt32(), b16Bit );
}

/** Reads 32-bit string length and ignores following character array.
    @param b16Bit  true = 16-bit characters, false = 8-bit characters. */
void lclIgnoreString32( XclImpStream& rStrm, bool b16Bit )
{
    sal_uInt32 nChars(0);
    nChars = rStrm.ReaduInt32();
    if( b16Bit )
        nChars *= 2;
    rStrm.Ignore( nChars );
}

/** Converts a path to an absolute path.
    @param rPath  The source path. The resulting path is returned here.
    @param nLevel  Number of parent directories to add in front of the path. */
void lclGetAbsPath( OUString& rPath, sal_uInt16 nLevel, SfxObjectShell* pDocShell )
{
    OUStringBuffer aTmpStr;
    while( nLevel )
    {
        aTmpStr.append( "../" );
        --nLevel;
    }
    aTmpStr.append( rPath );

    if( pDocShell )
    {
        bool bWasAbs = false;
        rPath = pDocShell->GetMedium()->GetURLObject().smartRel2Abs( aTmpStr.makeStringAndClear(), bWasAbs ).GetMainURL( INetURLObject::NO_DECODE );
        // full path as stored in SvxURLField must be encoded
    }
    else
        rPath = aTmpStr.makeStringAndClear();
}

/** Inserts the URL into a text cell. Does not modify value or formula cells. */
void lclInsertUrl( XclImpRoot& rRoot, const OUString& rUrl, SCCOL nScCol, SCROW nScRow, SCTAB nScTab )
{
    ScDocumentImport& rDoc = rRoot.GetDocImport();
    ScAddress aScPos( nScCol, nScRow, nScTab );
    CellType eCellType = rDoc.getDoc().GetCellType(aScPos);
    switch( eCellType )
    {
        // #i54261# hyperlinks in string cells
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
        {
            OUString aDisplText = rDoc.getDoc().GetString(nScCol, nScRow, nScTab);
            if (aDisplText.isEmpty())
                aDisplText = rUrl;

            ScEditEngineDefaulter& rEE = rRoot.GetEditEngine();
            SvxURLField aUrlField( rUrl, aDisplText, SVXURLFORMAT_APPDEFAULT );

            const EditTextObject* pEditObj = rDoc.getDoc().GetEditText(aScPos);
            if( pEditObj )
            {
                rEE.SetText( *pEditObj );
                rEE.QuickInsertField( SvxFieldItem( aUrlField, EE_FEATURE_FIELD ), ESelection( 0, 0, EE_PARA_ALL, 0 ) );
            }
            else
            {
                rEE.SetText( EMPTY_OUSTRING );
                rEE.QuickInsertField( SvxFieldItem( aUrlField, EE_FEATURE_FIELD ), ESelection() );
                if( const ScPatternAttr* pPattern = rDoc.getDoc().GetPattern( aScPos.Col(), aScPos.Row(), nScTab ) )
                {
                    SfxItemSet aItemSet( rEE.GetEmptyItemSet() );
                    pPattern->FillEditItemSet( &aItemSet );
                    rEE.QuickSetAttribs( aItemSet, ESelection( 0, 0, EE_PARA_ALL, 0 ) );
                }
            }

            // The cell will own the text object instance.
            rDoc.setEditCell(aScPos, rEE.CreateTextObject());
        }
        break;

        default:
        // Handle other cell types e.g. formulas ( and ? ) that have associated
        // hyperlinks.
        // Ideally all hyperlinks should be treated  as below. For the moment,
        // given the current absence of ods support lets just handle what we
        // previously didn't handle the new way.
        // Unfortunately we won't be able to preserve such hyperlinks when
        // saving to ods. Note: when we are able to save such hyperlinks to ods
        // we should handle *all* imported hyperlinks as below ( e.g. as cell
        // attribute ) for better interoperability.
        {
            SfxStringItem aItem( ATTR_HYPERLINK, rUrl );
            rDoc.getDoc().ApplyAttr(nScCol, nScRow, nScTab, aItem);
            break;
        }
    }
}

} // namespace

void XclImpHyperlink::ReadHlink( XclImpStream& rStrm )
{
    XclRange aXclRange( ScAddress::UNINITIALIZED );
    rStrm >> aXclRange;
    // #i80006# Excel silently ignores invalid hi-byte of column index (TODO: everywhere?)
    aXclRange.maFirst.mnCol &= 0xFF;
    aXclRange.maLast.mnCol &= 0xFF;
    OUString aString = ReadEmbeddedData( rStrm );
    if ( !aString.isEmpty() )
        rStrm.GetRoot().GetXFRangeBuffer().SetHyperlink( aXclRange, aString );
}

OUString XclImpHyperlink::ReadEmbeddedData( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    SfxObjectShell* pDocShell = rRoot.GetDocShell();

    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    XclGuid aGuid;
    rStrm >> aGuid;
    rStrm.Ignore( 4 );
    sal_uInt32 nFlags(0);
    nFlags = rStrm.ReaduInt32();

    OSL_ENSURE( aGuid == XclTools::maGuidStdLink, "XclImpHyperlink::ReadEmbeddedData - unknown header GUID" );

    ::std::unique_ptr< OUString > xLongName;    // link / file name
    ::std::unique_ptr< OUString > xShortName;   // 8.3-representation of file name
    ::std::unique_ptr< OUString > xTextMark;    // text mark

    // description (ignore)
    if( ::get_flag( nFlags, EXC_HLINK_DESCR ) )
        lclIgnoreString32( rStrm, true );
    // target frame (ignore) !! DESCR/FRAME - is this the right order? (never seen them together)
    if( ::get_flag( nFlags, EXC_HLINK_FRAME ) )
        lclIgnoreString32( rStrm, true );

    // URL fields are zero-terminated - do not let the stream replace them
    // in the lclAppendString32() with the '?' character.
    rStrm.SetNulSubstChar( '\0' );

    // UNC path
    if( ::get_flag( nFlags, EXC_HLINK_UNC ) )
    {
        xLongName.reset( new OUString );
        lclAppendString32( *xLongName, rStrm, true );
        lclGetAbsPath( *xLongName, 0, pDocShell );
    }
    // file link or URL
    else if( ::get_flag( nFlags, EXC_HLINK_BODY ) )
    {
        rStrm >> aGuid;

        if( aGuid == XclTools::maGuidFileMoniker )
        {
            sal_uInt16 nLevel = 0; // counter for level to climb down in path
            nLevel = rStrm.ReaduInt16();
            xShortName.reset( new OUString );
            lclAppendString32( *xShortName, rStrm, false );
            rStrm.Ignore( 24 );

            sal_uInt32 nStrLen = 0;
            nStrLen = rStrm.ReaduInt32();
            if( nStrLen )
            {
                nStrLen = rStrm.ReaduInt32();
                nStrLen /= 2;       // it's byte count here...
                rStrm.Ignore( 2 );
                xLongName.reset( new OUString );
                lclAppendString32( *xLongName, rStrm, nStrLen, true );
                lclGetAbsPath( *xLongName, nLevel, pDocShell );
            }
            else
                lclGetAbsPath( *xShortName, nLevel, pDocShell );
        }
        else if( aGuid == XclTools::maGuidUrlMoniker )
        {
            sal_uInt32 nStrLen(0);
            nStrLen = rStrm.ReaduInt32();
            nStrLen /= 2;       // it's byte count here...
            xLongName.reset( new OUString );
            lclAppendString32( *xLongName, rStrm, nStrLen, true );
            if( !::get_flag( nFlags, EXC_HLINK_ABS ) )
                lclGetAbsPath( *xLongName, 0, pDocShell );
        }
        else
        {
            OSL_FAIL( "XclImpHyperlink::ReadEmbeddedData - unknown content GUID" );
        }
    }

    // text mark
    if( ::get_flag( nFlags, EXC_HLINK_MARK ) )
    {
        xTextMark.reset( new OUString );
        lclAppendString32( *xTextMark, rStrm, true );
    }

    rStrm.SetNulSubstChar();    // back to default

    OSL_ENSURE( rStrm.GetRecLeft() == 0, "XclImpHyperlink::ReadEmbeddedData - record size mismatch" );

    if( !xLongName.get() && xShortName.get() )
        xLongName = std::move(xShortName);
    else if( !xLongName.get() && xTextMark.get() )
        xLongName.reset( new OUString );

    if( xLongName.get() )
    {
        if( xTextMark.get() )
        {
            if( xLongName->isEmpty() )
            {
                sal_Int32 nSepPos = xTextMark->lastIndexOf( '!' );
                if( nSepPos > 0 )
                {
                    // Do not attempt to blindly convert '#SheetName!A1' to
                    // '#SheetName.A1', it can be #SheetName!R1C1 as well.
                    // Hyperlink handler has to handle all, but prefer
                    // '#SheetName.A1' if possible.
                    if (nSepPos < xTextMark->getLength() - 1)
                    {
                        ScRange aRange;
                        if ((aRange.ParseAny( xTextMark->copy( nSepPos + 1 ), nullptr,
                                        formula::FormulaGrammar::CONV_XL_R1C1) & SCA_VALID) != SCA_VALID)
                            xTextMark.reset( new OUString( xTextMark->replaceAt( nSepPos, 1, OUString( '.' ))));
                    }
                }
            }
            xLongName.reset( new OUString( *xLongName + "#" + *xTextMark ) );
        }
        return( *xLongName );
    }
    return( OUString() );
}

void XclImpHyperlink::ConvertToValidTabName(OUString& rUrl)
{
    sal_Int32 n = rUrl.getLength();
    if (n < 4)
        // Needs at least 4 characters.
        return;

    if (rUrl[0] != '#')
        // the 1st character must be '#'.
        return;

    OUString aNewUrl('#'), aTabName;

    bool bInQuote = false;
    bool bQuoteTabName = false;
    for( sal_Int32 i = 1; i < n; ++i )
    {
        sal_Unicode c = rUrl[i];
        if (c == '\'')
        {
            if (bInQuote && i+1 < n && rUrl[i+1] == '\'')
            {
                // Two consecutive single quotes ('') signify a single literal
                // quite.  When this occurs, the whole table name needs to be
                // quoted.
                bQuoteTabName = true;
                aTabName += OUString(c);
                aTabName += OUString(c);
                ++i;
                continue;
            }

            bInQuote = !bInQuote;
            if (!bInQuote && !aTabName.isEmpty())
            {
                if (bQuoteTabName)
                    aNewUrl += "'";
                aNewUrl += aTabName;
                if (bQuoteTabName)
                    aNewUrl += "'";
            }
        }
        else if (bInQuote)
            aTabName += OUString(c);
        else
            aNewUrl += OUString(c);
    }

    if (bInQuote)
        // It should be outside the quotes!
        return;

    // All is good.  Pass the new URL.
    rUrl = aNewUrl;
}

void XclImpHyperlink::InsertUrl( XclImpRoot& rRoot, const XclRange& rXclRange, const OUString& rUrl )
{
    OUString aUrl(rUrl);
    ConvertToValidTabName(aUrl);

    SCTAB nScTab = rRoot.GetCurrScTab();
    ScRange aScRange( ScAddress::UNINITIALIZED );
    if( rRoot.GetAddressConverter().ConvertRange( aScRange, rXclRange, nScTab, nScTab, true ) )
    {
        SCCOL nScCol1, nScCol2;
        SCROW nScRow1, nScRow2;
        aScRange.GetVars( nScCol1, nScRow1, nScTab, nScCol2, nScRow2, nScTab );
        for( SCCOL nScCol = nScCol1; nScCol <= nScCol2; ++nScCol )
            for( SCROW nScRow = nScRow1; nScRow <= nScRow2; ++nScRow )
                lclInsertUrl( rRoot, aUrl, nScCol, nScRow, nScTab );
    }
}

// Label ranges ===============================================================

void XclImpLabelranges::ReadLabelranges( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    ScDocument& rDoc = rRoot.GetDoc();
    SCTAB nScTab = rRoot.GetCurrScTab();
    XclImpAddressConverter& rAddrConv = rRoot.GetAddressConverter();
    ScRangePairListRef xLabelRangesRef;
    const ScRange* pScRange = nullptr;

    XclRangeList aRowXclRanges, aColXclRanges;
    rStrm >> aRowXclRanges >> aColXclRanges;

    // row label ranges
    ScRangeList aRowScRanges;
    rAddrConv.ConvertRangeList( aRowScRanges, aRowXclRanges, nScTab, false );
    xLabelRangesRef = rDoc.GetRowNameRangesRef();
    for ( size_t i = 0, nRanges = aRowScRanges.size(); i < nRanges; ++i )
    {
        pScRange = aRowScRanges[ i ];
        ScRange aDataRange( *pScRange );
        if( aDataRange.aEnd.Col() < MAXCOL )
        {
            aDataRange.aStart.SetCol( aDataRange.aEnd.Col() + 1 );
            aDataRange.aEnd.SetCol( MAXCOL );
        }
        else if( aDataRange.aStart.Col() > 0 )
        {
            aDataRange.aEnd.SetCol( aDataRange.aStart.Col() - 1 );
            aDataRange.aStart.SetCol( 0 );
        }
        xLabelRangesRef->Append( ScRangePair( *pScRange, aDataRange ) );
    }

    // column label ranges
    ScRangeList aColScRanges;
    rAddrConv.ConvertRangeList( aColScRanges, aColXclRanges, nScTab, false );
    xLabelRangesRef = rDoc.GetColNameRangesRef();

    for ( size_t i = 0, nRanges = aColScRanges.size(); i < nRanges; ++i )
    {
        pScRange = aColScRanges[ i ];
        ScRange aDataRange( *pScRange );
        if( aDataRange.aEnd.Row() < MAXROW )
        {
            aDataRange.aStart.SetRow( aDataRange.aEnd.Row() + 1 );
            aDataRange.aEnd.SetRow( MAXROW );
        }
        else if( aDataRange.aStart.Row() > 0 )
        {
            aDataRange.aEnd.SetRow( aDataRange.aStart.Row() - 1 );
            aDataRange.aStart.SetRow( 0 );
        }
        xLabelRangesRef->Append( ScRangePair( *pScRange, aDataRange ) );
    }
}

// Conditional formatting =====================================================

XclImpCondFormat::XclImpCondFormat( const XclImpRoot& rRoot, sal_uInt32 nFormatIndex ) :
    XclImpRoot( rRoot ),
    mnFormatIndex( nFormatIndex ),
    mnCondCount( 0 ),
    mnCondIndex( 0 )
{
}

XclImpCondFormat::~XclImpCondFormat()
{
}

void XclImpCondFormat::ReadCondfmt( XclImpStream& rStrm )
{
    OSL_ENSURE( !mnCondCount, "XclImpCondFormat::ReadCondfmt - already initialized" );
    XclRangeList aXclRanges;
    mnCondCount = rStrm.ReaduInt16();
    rStrm.Ignore( 10 );
    rStrm >> aXclRanges;
    GetAddressConverter().ConvertRangeList( maRanges, aXclRanges, GetCurrScTab(), true );
}

void XclImpCondFormat::ReadCF( XclImpStream& rStrm )
{
    if( mnCondIndex >= mnCondCount )
    {
        OSL_FAIL( "XclImpCondFormat::ReadCF - CF without leading CONDFMT" );
        return;
    }

    // entire conditional format outside of valid range?
    if( maRanges.empty() )
        return;

    sal_uInt8 nType = rStrm.ReaduInt8();
    sal_uInt8 nOperator = rStrm.ReaduInt8();
    sal_uInt16 nFmlaSize1 = rStrm.ReaduInt16();
    sal_uInt16 nFmlaSize2 = rStrm.ReaduInt16();
    sal_uInt32 nFlags = rStrm.ReaduInt32();
    rStrm.Ignore( 2 ); //nFlagsExtended

    // *** mode and comparison operator ***

    ScConditionMode eMode = SC_COND_NONE;
    switch( nType )
    {
        case EXC_CF_TYPE_CELL:
        {
            switch( nOperator )
            {
                case EXC_CF_CMP_BETWEEN:        eMode = SC_COND_BETWEEN;    break;
                case EXC_CF_CMP_NOT_BETWEEN:    eMode = SC_COND_NOTBETWEEN; break;
                case EXC_CF_CMP_EQUAL:          eMode = SC_COND_EQUAL;      break;
                case EXC_CF_CMP_NOT_EQUAL:      eMode = SC_COND_NOTEQUAL;   break;
                case EXC_CF_CMP_GREATER:        eMode = SC_COND_GREATER;    break;
                case EXC_CF_CMP_LESS:           eMode = SC_COND_LESS;       break;
                case EXC_CF_CMP_GREATER_EQUAL:  eMode = SC_COND_EQGREATER;  break;
                case EXC_CF_CMP_LESS_EQUAL:     eMode = SC_COND_EQLESS;     break;
                default:
                    SAL_INFO(
                        "sc.filter", "unknown CF comparison " << nOperator);
            }
        }
        break;

        case EXC_CF_TYPE_FMLA:
            eMode = SC_COND_DIRECT;
        break;

        default:
            SAL_INFO("sc.filter", "unknown CF mode " << nType);
            return;
    }

    // *** create style sheet ***

    OUString aStyleName( XclTools::GetCondFormatStyleName( GetCurrScTab(), mnFormatIndex, mnCondIndex ) );
    SfxItemSet& rStyleItemSet = ScfTools::MakeCellStyleSheet( GetStyleSheetPool(), aStyleName, true ).GetItemSet();

    const XclImpPalette& rPalette = GetPalette();

    // number format

    if( get_flag( nFlags, EXC_CF_BLOCK_NUMFMT ) )
    {
        XclImpNumFmtBuffer& rNumFmtBuffer = GetRoot().GetNumFmtBuffer();
        bool bIFmt = get_flag( nFlags, EXC_CF_IFMT_USER );
        sal_uInt16 nFormat = rNumFmtBuffer.ReadCFFormat( rStrm, bIFmt );
        rNumFmtBuffer.FillToItemSet( rStyleItemSet, nFormat );
    }

    // *** font block ***

    if( ::get_flag( nFlags, EXC_CF_BLOCK_FONT ) )
    {
        XclImpFont aFont( GetRoot() );
        aFont.ReadCFFontBlock( rStrm );
        aFont.FillToItemSet( rStyleItemSet, EXC_FONTITEM_CELL );
    }

    // alignment
    if( get_flag( nFlags, EXC_CF_BLOCK_ALIGNMENT ) )
    {
        XclImpCellAlign aAlign;
        sal_uInt16 nAlign(0);
        sal_uInt16 nAlignMisc(0);
        nAlign = rStrm.ReaduInt16();
        nAlignMisc = rStrm.ReaduInt16();
        aAlign.FillFromCF( nAlign, nAlignMisc );
        aAlign.FillToItemSet( rStyleItemSet, nullptr );
        rStrm.Ignore(4);
    }

    // *** border block ***

    if( ::get_flag( nFlags, EXC_CF_BLOCK_BORDER ) )
    {
        sal_uInt16 nLineStyle(0);
        sal_uInt32 nLineColor(0);
        nLineStyle = rStrm.ReaduInt16();
        nLineColor = rStrm.ReaduInt32();
        rStrm.Ignore( 2 );

        XclImpCellBorder aBorder;
        aBorder.FillFromCF8( nLineStyle, nLineColor, nFlags );
        aBorder.FillToItemSet( rStyleItemSet, rPalette );
    }

    // *** pattern block ***

    if( ::get_flag( nFlags, EXC_CF_BLOCK_AREA ) )
    {
        sal_uInt16 nPattern(0), nColor(0);
        nPattern = rStrm.ReaduInt16();
        nColor = rStrm.ReaduInt16();

        XclImpCellArea aArea;
        aArea.FillFromCF8( nPattern, nColor, nFlags );
        aArea.FillToItemSet( rStyleItemSet, rPalette );
    }

    if( get_flag( nFlags, EXC_CF_BLOCK_PROTECTION ) )
    {
        sal_uInt16 nCellProt;
        nCellProt = rStrm.ReaduInt16();
        XclImpCellProt aCellProt;
        aCellProt.FillFromXF3(nCellProt);
        aCellProt.FillToItemSet( rStyleItemSet );
    }

    // *** formulas ***

    const ScAddress& rPos = maRanges.front()->aStart;    // assured above that maRanges is not empty
    ExcelToSc& rFmlaConv = GetOldFmlaConverter();

    ::std::unique_ptr< ScTokenArray > xTokArr1;
    if( nFmlaSize1 > 0 )
    {
        const ScTokenArray* pTokArr = nullptr;
        rFmlaConv.Reset( rPos );
        rFmlaConv.Convert( pTokArr, rStrm, nFmlaSize1, false, FT_CondFormat );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            xTokArr1.reset( pTokArr->Clone() );
    }

    ::std::unique_ptr< ScTokenArray > pTokArr2;
    if( nFmlaSize2 > 0 )
    {
        const ScTokenArray* pTokArr = nullptr;
        rFmlaConv.Reset( rPos );
        rFmlaConv.Convert( pTokArr, rStrm, nFmlaSize2, false, FT_CondFormat );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            pTokArr2.reset( pTokArr->Clone() );
    }

    // *** create the Calc conditional formatting ***

    if( !mxScCondFmt.get() )
    {
        sal_uLong nKey = 0;
        mxScCondFmt.reset( new ScConditionalFormat( nKey, &GetDocRef() ) );
        if(maRanges.size() > 1)
            maRanges.Join(*maRanges[0], true);
        mxScCondFmt->SetRange(maRanges);
    }

    ScCondFormatEntry* pEntry = new ScCondFormatEntry( eMode, xTokArr1.get(), pTokArr2.get(), &GetDocRef(), rPos, aStyleName );
    mxScCondFmt->AddEntry( pEntry );
    ++mnCondIndex;
}

void XclImpCondFormat::Apply()
{
    if( mxScCondFmt.get() )
    {
        ScDocument& rDoc = GetDoc();

        SCTAB nTab = maRanges.front()->aStart.Tab();
        sal_uLong nKey = rDoc.AddCondFormat( mxScCondFmt->Clone(), nTab );

        rDoc.AddCondFormatData( maRanges, nTab, nKey );
    }
}

XclImpCondFormatManager::XclImpCondFormatManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpCondFormatManager::ReadCondfmt( XclImpStream& rStrm )
{
    XclImpCondFormat* pFmt = new XclImpCondFormat( GetRoot(), maCondFmtList.size() );
    pFmt->ReadCondfmt( rStrm );
    maCondFmtList.push_back( std::unique_ptr<XclImpCondFormat>(pFmt) );
}

void XclImpCondFormatManager::ReadCF( XclImpStream& rStrm )
{
    OSL_ENSURE( !maCondFmtList.empty(), "XclImpCondFormatManager::ReadCF - CF without leading CONDFMT" );
    if( !maCondFmtList.empty() )
        maCondFmtList.back()->ReadCF( rStrm );
}

void XclImpCondFormatManager::Apply()
{
    for( XclImpCondFmtList::iterator itFmt = maCondFmtList.begin(); itFmt != maCondFmtList.end(); ++itFmt )
        (*itFmt)->Apply();
    maCondFmtList.clear();
}

// Data Validation ============================================================

XclImpValidationManager::DVItem::DVItem( const ScRangeList& rRanges, const ScValidationData& rValidData ) :
    maRanges(rRanges), maValidData(rValidData) {}

XclImpValidationManager::XclImpValidationManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpValidationManager::ReadDval( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    sal_uInt32 nObjId(0);
    rStrm.Ignore( 10 );
    nObjId = rStrm.ReaduInt32();
    if( nObjId != EXC_DVAL_NOOBJ )
    {
        OSL_ENSURE( nObjId <= 0xFFFF, "XclImpValidation::ReadDval - invalid object ID" );
        rRoot.GetCurrSheetDrawing().SetSkipObj( static_cast< sal_uInt16 >( nObjId ) );
    }
}

void XclImpValidationManager::ReadDV( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    OSL_ENSURE_BIFF( rRoot.GetBiff() == EXC_BIFF8 );

    ScDocument& rDoc = rRoot.GetDoc();
    SCTAB nScTab = rRoot.GetCurrScTab();
    ExcelToSc& rFmlaConv = rRoot.GetOldFmlaConverter();

    // flags
    sal_uInt32 nFlags(0);
    nFlags = rStrm.ReaduInt32();

    // message strings
    /*  Empty strings are single NUL characters in Excel (string length is 1).
        -> Do not let the stream replace them with '?' characters. */
    rStrm.SetNulSubstChar( '\0' );
    OUString aPromptTitle(   rStrm.ReadUniString() );
    OUString aErrorTitle(    rStrm.ReadUniString() );
    OUString aPromptMessage( rStrm.ReadUniString() );
    OUString aErrorMessage(  rStrm.ReadUniString() );
    rStrm.SetNulSubstChar();    // back to default

    // formula(s)
    if ( rStrm.GetRecLeft() <= 8 )
        // Not enough bytes left in the record.  Bail out.
        return;

    // first formula
    // string list is single tStr token with NUL separators -> replace them with LF
    rStrm.SetNulSubstChar( '\n' );
    ::std::unique_ptr< ScTokenArray > xTokArr1;

    sal_uInt16 nLen = 0;
    nLen = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    if( nLen > 0 )
    {
        const ScTokenArray* pTokArr = nullptr;
        rFmlaConv.Reset();
            rFmlaConv.Convert( pTokArr, rStrm, nLen, false, FT_CondFormat );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            xTokArr1.reset( pTokArr->Clone() );
    }
    rStrm.SetNulSubstChar();    // back to default

    // second formula
    ::std::unique_ptr< ScTokenArray > xTokArr2;

    nLen = rStrm.ReaduInt16();
    rStrm.Ignore( 2 );
    if( nLen > 0 )
    {
        const ScTokenArray* pTokArr = nullptr;
        rFmlaConv.Reset();
            rFmlaConv.Convert( pTokArr, rStrm, nLen, false, FT_CondFormat );
        // formula converter owns pTokArr -> create a copy of the token array
        if( pTokArr )
            xTokArr2.reset( pTokArr->Clone() );
    }

    // read all cell ranges
    XclRangeList aXclRanges;
    rStrm >> aXclRanges;

    // convert to Calc range list
    ScRangeList aScRanges;
    rRoot.GetAddressConverter().ConvertRangeList( aScRanges, aXclRanges, nScTab, true );

    // only continue if there are valid ranges
    if ( aScRanges.empty() )
        return;

    bool bIsValid = true;   // valid settings in flags field

    ScValidationMode eValMode = SC_VALID_ANY;
    switch( nFlags & EXC_DV_MODE_MASK )
    {
        case EXC_DV_MODE_ANY:       eValMode = SC_VALID_ANY;        break;
        case EXC_DV_MODE_WHOLE:     eValMode = SC_VALID_WHOLE;      break;
        case EXC_DV_MODE_DECIMAL:   eValMode = SC_VALID_DECIMAL;    break;
        case EXC_DV_MODE_LIST:      eValMode = SC_VALID_LIST;       break;
        case EXC_DV_MODE_DATE:      eValMode = SC_VALID_DATE;       break;
        case EXC_DV_MODE_TIME:      eValMode = SC_VALID_TIME;       break;
        case EXC_DV_MODE_TEXTLEN:   eValMode = SC_VALID_TEXTLEN;    break;
        case EXC_DV_MODE_CUSTOM:    eValMode = SC_VALID_CUSTOM;     break;
        default:                    bIsValid = false;
    }
    rRoot.GetTracer().TraceDVType(eValMode == SC_VALID_CUSTOM);

    ScConditionMode eCondMode = SC_COND_BETWEEN;
    switch( nFlags & EXC_DV_COND_MASK )
    {
        case EXC_DV_COND_BETWEEN:   eCondMode = SC_COND_BETWEEN;    break;
        case EXC_DV_COND_NOTBETWEEN:eCondMode = SC_COND_NOTBETWEEN; break;
        case EXC_DV_COND_EQUAL:     eCondMode = SC_COND_EQUAL;      break;
        case EXC_DV_COND_NOTEQUAL:  eCondMode = SC_COND_NOTEQUAL;   break;
        case EXC_DV_COND_GREATER:   eCondMode = SC_COND_GREATER;    break;
        case EXC_DV_COND_LESS:      eCondMode = SC_COND_LESS;       break;
        case EXC_DV_COND_EQGREATER: eCondMode = SC_COND_EQGREATER;  break;
        case EXC_DV_COND_EQLESS:    eCondMode = SC_COND_EQLESS;     break;
        default:                    bIsValid = false;
    }

    if ( !bIsValid )
        // No valid validation found.  Bail out.
        return;

    // first range for base address for relative references
    const ScRange& rScRange = *aScRanges.front();    // aScRanges is not empty

    // process string list of a list validity (convert to list of string tokens)
    if( xTokArr1.get() && (eValMode == SC_VALID_LIST) && ::get_flag( nFlags, EXC_DV_STRINGLIST ) )
        XclTokenArrayHelper::ConvertStringToList(*xTokArr1, rDoc.GetSharedStringPool(), '\n', true);

    maDVItems.push_back(
        o3tl::make_unique<DVItem>(aScRanges, ScValidationData(eValMode, eCondMode, xTokArr1.get(), xTokArr2.get(), &rDoc, rScRange.aStart)));
    DVItem& rItem = *maDVItems.back().get();

    rItem.maValidData.SetIgnoreBlank( ::get_flag( nFlags, EXC_DV_IGNOREBLANK ) );
    rItem.maValidData.SetListType( ::get_flagvalue( nFlags, EXC_DV_SUPPRESSDROPDOWN, css::sheet::TableValidationVisibility::INVISIBLE, css::sheet::TableValidationVisibility::UNSORTED ) );

    // *** prompt box ***
    if( !aPromptTitle.isEmpty() || !aPromptMessage.isEmpty() )
    {
        // set any text stored in the record
        rItem.maValidData.SetInput( aPromptTitle, aPromptMessage );
        if( !::get_flag( nFlags, EXC_DV_SHOWPROMPT ) )
            rItem.maValidData.ResetInput();
    }

    // *** error box ***
    ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
    switch( nFlags & EXC_DV_ERROR_MASK )
    {
        case EXC_DV_ERROR_WARNING:  eErrStyle = SC_VALERR_WARNING;  break;
        case EXC_DV_ERROR_INFO:     eErrStyle = SC_VALERR_INFO;     break;
    }
    // set texts and error style
    rItem.maValidData.SetError( aErrorTitle, aErrorMessage, eErrStyle );
    if( !::get_flag( nFlags, EXC_DV_SHOWERROR ) )
        rItem.maValidData.ResetError();
}

void XclImpValidationManager::Apply()
{
    ScDocument& rDoc = GetRoot().GetDoc();
    DVItemList::iterator itr = maDVItems.begin(), itrEnd = maDVItems.end();
    for (; itr != itrEnd; ++itr)
    {
        DVItem& rItem = *itr->get();
        // set the handle ID
        sal_uLong nHandle = rDoc.AddValidationEntry( rItem.maValidData );
        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nHandle ) );

        // apply all ranges
        for ( size_t i = 0, nRanges = rItem.maRanges.size(); i < nRanges; ++i )
        {
            const ScRange* pScRange = rItem.maRanges[ i ];
            rDoc.ApplyPatternAreaTab( pScRange->aStart.Col(), pScRange->aStart.Row(),
                pScRange->aEnd.Col(), pScRange->aEnd.Row(), pScRange->aStart.Tab(), aPattern );
        }
    }
    maDVItems.clear();
}

// Web queries ================================================================

XclImpWebQuery::XclImpWebQuery( const ScRange& rDestRange ) :
    maDestRange( rDestRange ),
    meMode( xlWQUnknown ),
    mnRefresh( 0 )
{
}

void XclImpWebQuery::ReadParamqry( XclImpStream& rStrm )
{
    sal_uInt16 nFlags = rStrm.ReaduInt16();
    sal_uInt16 nType = ::extract_value< sal_uInt16 >( nFlags, 0, 3 );
    if( (nType == EXC_PQRYTYPE_WEBQUERY) && ::get_flag( nFlags, EXC_PQRY_WEBQUERY ) )
    {
        if( ::get_flag( nFlags, EXC_PQRY_TABLES ) )
        {
            meMode = xlWQAllTables;
            maTables = ScfTools::GetHTMLTablesName();
        }
        else
        {
            meMode = xlWQDocument;
            maTables = ScfTools::GetHTMLDocName();
        }
    }
}

void XclImpWebQuery::ReadWqstring( XclImpStream& rStrm )
{
    maURL = rStrm.ReadUniString();
}

void XclImpWebQuery::ReadWqsettings( XclImpStream& rStrm )
{
    rStrm.Ignore( 10 );
    sal_uInt16 nFlags(0);
    nFlags = rStrm.ReaduInt16();
    rStrm.Ignore( 10 );
    mnRefresh = rStrm.ReaduInt16();

    if( ::get_flag( nFlags, EXC_WQSETT_SPECTABLES ) && (meMode == xlWQAllTables) )
        meMode = xlWQSpecTables;
}

void XclImpWebQuery::ReadWqtables( XclImpStream& rStrm )
{
    if( meMode == xlWQSpecTables )
    {
        rStrm.Ignore( 4 );
        OUString aTables( rStrm.ReadUniString() );

        const sal_Unicode cSep = ';';
        OUString aQuotedPairs( "\"\"" );
        sal_Int32 nTokenCnt = ScStringUtil::GetQuotedTokenCount( aTables, aQuotedPairs, ',' );
        maTables.clear();
        sal_Int32 nStringIx = 0;
        for( sal_Int32 nToken = 0; nToken < nTokenCnt; ++nToken )
        {
            OUString aToken( ScStringUtil::GetQuotedToken( aTables, 0, aQuotedPairs, ',', nStringIx ) );
            sal_Int32 nTabNum = CharClass::isAsciiNumeric( aToken ) ? aToken.toInt32() : 0;
            if( nTabNum > 0 )
                maTables = ScGlobal::addToken( maTables, ScfTools::GetNameFromHTMLIndex( static_cast< sal_uInt32 >( nTabNum ) ), cSep );
            else
            {
                ScGlobal::EraseQuotes( aToken, '"', false );
                if( !aToken.isEmpty() )
                    maTables = ScGlobal::addToken( maTables, ScfTools::GetNameFromHTMLName( aToken ), cSep );
            }
        }
    }
}

void XclImpWebQuery::Apply( ScDocument& rDoc, const OUString& rFilterName )
{
    if( !maURL.isEmpty() && (meMode != xlWQUnknown) && rDoc.GetDocumentShell() )
    {
        ScAreaLink* pLink = new ScAreaLink( rDoc.GetDocumentShell(),
            maURL, rFilterName, EMPTY_OUSTRING, maTables, maDestRange, mnRefresh * 60UL );
        rDoc.GetLinkManager()->InsertFileLink( *pLink, OBJECT_CLIENT_FILE,
            maURL, &rFilterName, &maTables );
    }
}

XclImpWebQueryBuffer::XclImpWebQueryBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpWebQueryBuffer::ReadQsi( XclImpStream& rStrm )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        rStrm.Ignore( 10 );
        OUString aXclName( rStrm.ReadUniString() );

        // #i64794# Excel replaces spaces with underscores
        aXclName = aXclName.replaceAll( " ", "_" );

        // find the defined name used in Calc
        if( const XclImpName* pName = GetNameManager().FindName( aXclName, GetCurrScTab() ) )
        {
            if( const ScRangeData* pRangeData = pName->GetScRangeData() )
            {
                ScRange aRange;
                if( pRangeData->IsReference( aRange ) )
                    maWQList.push_back( XclImpWebQuery( aRange ) );
            }
        }
    }
    else
    {
        DBG_ERROR_BIFF();
    }
}

void XclImpWebQueryBuffer::ReadParamqry( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadParamqry( rStrm );
}

void XclImpWebQueryBuffer::ReadWqstring( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadWqstring( rStrm );
}

void XclImpWebQueryBuffer::ReadWqsettings( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadWqsettings( rStrm );
}

void XclImpWebQueryBuffer::ReadWqtables( XclImpStream& rStrm )
{
    if (!maWQList.empty())
        maWQList.back().ReadWqtables( rStrm );
}

void XclImpWebQueryBuffer::Apply()
{
    ScDocument& rDoc = GetDoc();
    OUString aFilterName( EXC_WEBQRY_FILTER );
    for( XclImpWebQueryList::iterator itQuery = maWQList.begin(); itQuery != maWQList.end(); ++itQuery )
        itQuery->Apply( rDoc, aFilterName );
}

// Decryption =================================================================

namespace {

XclImpDecrypterRef lclReadFilepass5( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;
    OSL_ENSURE( rStrm.GetRecLeft() == 4, "lclReadFilepass5 - wrong record size" );
    if( rStrm.GetRecLeft() == 4 )
    {
        sal_uInt16 nKey(0), nHash(0);
        nKey = rStrm.ReaduInt16();
        nHash = rStrm.ReaduInt16();
        xDecr.reset( new XclImpBiff5Decrypter( nKey, nHash ) );
    }
    return xDecr;
}

XclImpDecrypterRef lclReadFilepass8_Standard( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;
    OSL_ENSURE( rStrm.GetRecLeft() == 48, "lclReadFilepass8 - wrong record size" );
    if( rStrm.GetRecLeft() == 48 )
    {
        sal_uInt8 pnSalt[ 16 ];
        sal_uInt8 pnVerifier[ 16 ];
        sal_uInt8 pnVerifierHash[ 16 ];
        rStrm.Read( pnSalt, 16 );
        rStrm.Read( pnVerifier, 16 );
        rStrm.Read( pnVerifierHash, 16 );
        xDecr.reset( new XclImpBiff8Decrypter( pnSalt, pnVerifier, pnVerifierHash ) );
    }
    return xDecr;
}

XclImpDecrypterRef lclReadFilepass8_Strong( XclImpStream& /*rStrm*/ )
{
    // not supported
    return XclImpDecrypterRef();
}

XclImpDecrypterRef lclReadFilepass8( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;

    sal_uInt16 nMode(0);
    nMode = rStrm.ReaduInt16();
    switch( nMode )
    {
        case EXC_FILEPASS_BIFF5:
            xDecr = lclReadFilepass5( rStrm );
        break;

        case EXC_FILEPASS_BIFF8:
        {
            rStrm.Ignore( 2 );
            sal_uInt16 nSubMode(0);
            nSubMode = rStrm.ReaduInt16();
            switch( nSubMode )
            {
                case EXC_FILEPASS_BIFF8_STD:
                    xDecr = lclReadFilepass8_Standard( rStrm );
                break;
                case EXC_FILEPASS_BIFF8_STRONG:
                    xDecr = lclReadFilepass8_Strong( rStrm );
                break;
                default:
                    OSL_FAIL( "lclReadFilepass8 - unknown BIFF8 encryption sub mode" );
            }
        }
        break;

        default:
            OSL_FAIL( "lclReadFilepass8 - unknown encryption mode" );
    }

    return xDecr;
}

} // namespace

ErrCode XclImpDecryptHelper::ReadFilepass( XclImpStream& rStrm )
{
    XclImpDecrypterRef xDecr;
    rStrm.DisableDecryption();

    // read the FILEPASS record and create a new decrypter object
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5: xDecr = lclReadFilepass5( rStrm );  break;
        case EXC_BIFF8: xDecr = lclReadFilepass8( rStrm );  break;
        default:        DBG_ERROR_BIFF();
    };

    // set decrypter at import stream
    rStrm.SetDecrypter( xDecr );

    // request and verify a password (decrypter implements IDocPasswordVerifier)
    if( xDecr )
        rStrm.GetRoot().RequestEncryptionData( *xDecr );

    // return error code (success, wrong password, etc.)
    return xDecr ? xDecr->GetError() : EXC_ENCR_ERROR_UNSUPP_CRYPT;
}

// Document protection ========================================================

XclImpDocProtectBuffer::XclImpDocProtectBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mnPassHash(0x0000),
    mbDocProtect(false),
    mbWinProtect(false)
{
}

void XclImpDocProtectBuffer::ReadDocProtect( XclImpStream& rStrm )
{
    mbDocProtect = rStrm.ReaduInt16() != 0;
}

void XclImpDocProtectBuffer::ReadWinProtect( XclImpStream& rStrm )
{
    mbWinProtect = rStrm.ReaduInt16() != 0;
}

void XclImpDocProtectBuffer::ReadPasswordHash( XclImpStream& rStrm )
{
    rStrm.EnableDecryption();
    mnPassHash = rStrm.ReaduInt16();
}

void XclImpDocProtectBuffer::Apply() const
{
    if (!mbDocProtect && !mbWinProtect)
        // Excel requires either the structure or windows protection is set.
        // If neither is set then the document is not protected at all.
        return;

    unique_ptr<ScDocProtection> pProtect(new ScDocProtection);
    pProtect->setProtected(true);

    if (mnPassHash)
    {
        // 16-bit password hash.
        Sequence<sal_Int8> aPass(2);
        aPass[0] = (mnPassHash >> 8) & 0xFF;
        aPass[1] = mnPassHash & 0xFF;
        pProtect->setPasswordHash(aPass, PASSHASH_XL);
    }

    // document protection options
    pProtect->setOption(ScDocProtection::STRUCTURE, mbDocProtect);
    pProtect->setOption(ScDocProtection::WINDOWS,   mbWinProtect);

    GetDoc().SetDocProtection(pProtect.get());
}

// Sheet Protection ===========================================================

XclImpSheetProtectBuffer::Sheet::Sheet() :
    mbProtected(false),
    mnPasswordHash(0x0000),
    mnOptions(0x4400)
{
}

XclImpSheetProtectBuffer::Sheet::Sheet(const Sheet& r) :
    mbProtected(r.mbProtected),
    mnPasswordHash(r.mnPasswordHash),
    mnOptions(r.mnOptions)
{
}

XclImpSheetProtectBuffer::XclImpSheetProtectBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpSheetProtectBuffer::ReadProtect( XclImpStream& rStrm, SCTAB nTab )
{
    if ( rStrm.ReaduInt16() )
    {
        Sheet* pSheet = GetSheetItem(nTab);
        if (pSheet)
            pSheet->mbProtected = true;
    }
}

void XclImpSheetProtectBuffer::ReadOptions( XclImpStream& rStrm, SCTAB nTab )
{
    // The flag size specifies the size of bytes that follows that stores
    // feature data.  If -1 it depends on the feature type imported earlier.
    // For enhanced protection data, the size is always 4.  For the most xls
    // documents out there this value is almost always -1.
    sal_Int32 nFlagSize(0);
    nFlagSize = rStrm.ReadInt32();
    if (nFlagSize != -1)
        return;

    // There are actually 4 bytes to read, but the upper 2 bytes currently
    // don't store any bits.
    sal_uInt16 nOptions(0);
    nOptions = rStrm.ReaduInt16();

    Sheet* pSheet = GetSheetItem(nTab);
    if (pSheet)
        pSheet->mnOptions = nOptions;
}

void XclImpSheetProtectBuffer::AppendEnhancedProtection( const ScEnhancedProtection & rProt, SCTAB nTab )
{
    Sheet* pSheet = GetSheetItem(nTab);
    if (pSheet)
        pSheet->maEnhancedProtections.push_back( rProt);
}

void XclImpSheetProtectBuffer::ReadPasswordHash( XclImpStream& rStrm, SCTAB nTab )
{
    sal_uInt16 nHash(0);
    nHash = rStrm.ReaduInt16();
    Sheet* pSheet = GetSheetItem(nTab);
    if (pSheet)
        pSheet->mnPasswordHash = nHash;
}

void XclImpSheetProtectBuffer::Apply() const
{
    for (ProtectedSheetMap::const_iterator itr = maProtectedSheets.begin(), itrEnd = maProtectedSheets.end();
         itr != itrEnd; ++itr)
    {
        if (!itr->second.mbProtected)
            // This sheet is (for whatever reason) not protected.
            continue;

        unique_ptr<ScTableProtection> pProtect(new ScTableProtection);
        pProtect->setProtected(true);

        // 16-bit hash password
        const sal_uInt16 nHash = itr->second.mnPasswordHash;
        if (nHash)
        {
            Sequence<sal_Int8> aPass(2);
            aPass[0] = (nHash >> 8) & 0xFF;
            aPass[1] = nHash & 0xFF;
            pProtect->setPasswordHash(aPass, PASSHASH_XL);
        }

        // sheet protection options
        const sal_uInt16 nOptions = itr->second.mnOptions;
        pProtect->setOption( ScTableProtection::OBJECTS,               (nOptions & 0x0001) );
        pProtect->setOption( ScTableProtection::SCENARIOS,             (nOptions & 0x0002) );
        pProtect->setOption( ScTableProtection::FORMAT_CELLS,          (nOptions & 0x0004) );
        pProtect->setOption( ScTableProtection::FORMAT_COLUMNS,        (nOptions & 0x0008) );
        pProtect->setOption( ScTableProtection::FORMAT_ROWS,           (nOptions & 0x0010) );
        pProtect->setOption( ScTableProtection::INSERT_COLUMNS,        (nOptions & 0x0020) );
        pProtect->setOption( ScTableProtection::INSERT_ROWS,           (nOptions & 0x0040) );
        pProtect->setOption( ScTableProtection::INSERT_HYPERLINKS,     (nOptions & 0x0080) );
        pProtect->setOption( ScTableProtection::DELETE_COLUMNS,        (nOptions & 0x0100) );
        pProtect->setOption( ScTableProtection::DELETE_ROWS,           (nOptions & 0x0200) );
        pProtect->setOption( ScTableProtection::SELECT_LOCKED_CELLS,   (nOptions & 0x0400) );
        pProtect->setOption( ScTableProtection::SORT,                  (nOptions & 0x0800) );
        pProtect->setOption( ScTableProtection::AUTOFILTER,            (nOptions & 0x1000) );
        pProtect->setOption( ScTableProtection::PIVOT_TABLES,          (nOptions & 0x2000) );
        pProtect->setOption( ScTableProtection::SELECT_UNLOCKED_CELLS, (nOptions & 0x4000) );

        // Enhanced protection containing editable ranges and permissions.
        pProtect->setEnhancedProtection( itr->second.maEnhancedProtections);

        // all done.  now commit.
        GetDoc().SetTabProtection(itr->first, pProtect.get());
    }
}

XclImpSheetProtectBuffer::Sheet* XclImpSheetProtectBuffer::GetSheetItem( SCTAB nTab )
{
    ProtectedSheetMap::iterator itr = maProtectedSheets.find(nTab);
    if (itr == maProtectedSheets.end())
    {
        // new sheet
        if ( !maProtectedSheets.insert( ProtectedSheetMap::value_type(nTab, Sheet()) ).second )
            return nullptr;

        itr = maProtectedSheets.find(nTab);
    }

    return &itr->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
